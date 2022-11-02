/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "ContentQueryResultsReader.h"
#include "ContentQueryContracts.h"
#include "../Shared/ImageHelper.h"
#include "../Shared/ValueHelpers.h"

#ifdef wip_field_instance_keys
//=======================================================================================
// @bsiclass
//=======================================================================================
struct FieldValueInstanceKeyReader
{
private:
    ECDbCR m_db;
    bool m_trackKeys;
    bmap<ContentDescriptor::ECPropertiesField const*, bvector<ECClassInstanceKey>> m_relatedFieldKeys;
    bmap<ContentDescriptor::ECPropertiesField const*, int> m_fieldProperties;
    bvector<ECClassInstanceKey> const& m_primaryKeys;

public:
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    FieldValueInstanceKeyReader(ECDbCR db, bvector<ECClassInstanceKey> const& primaryKeys, bool trackKeys)
        : m_db(db), m_primaryKeys(primaryKeys), m_trackKeys(trackKeys)
        {}

    /*---------------------------------------------------------------------------------**//**
    // @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ReadFieldKeys(ContentDescriptor::ECInstanceKeyField const& field, ECSqlStatementCR statement, int columnIndex)
        {
        if (!m_trackKeys)
            return;

        if (statement.IsValueNull(columnIndex))
            return;

        bvector<ECInstanceKey> keys = ValueHelpers::GetECInstanceKeysFromJsonString(statement.GetValueText(columnIndex));
        for (ECInstanceKeyCR key : keys)
            {
            ECClassCP ecClass = m_db.Schemas().GetClass(key.GetClassId());
            for (ContentDescriptor::ECPropertiesField const* field : field.GetKeyFields())
                m_relatedFieldKeys[field].push_back(ECClassInstanceKey(ecClass, key.GetInstanceId()));
            }
        }

    /*---------------------------------------------------------------------------------**//**
    // @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetFieldProperty(ContentDescriptor::ECPropertiesField const& field, ContentDescriptor::Property const& prop, bool isMerged)
        {
        if (!m_trackKeys)
            return;

        if (isMerged)
            {
            m_fieldProperties[&field] = -1;
            return;
            }

        size_t propertyIndex;
        for (propertyIndex = 0; propertyIndex < field.GetProperties().size(); ++propertyIndex)
            {
            if (&field.GetProperties()[propertyIndex] == &prop)
                break;
            }

        if (propertyIndex >= field.GetProperties().size())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Field property was not found");

        m_fieldProperties[&field] = (int)propertyIndex;
        }

    /*---------------------------------------------------------------------------------**//**
    // @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentSetItem::FieldPropertyInstanceKeyMap GetKeys() const
        {
        ContentSetItem::FieldPropertyInstanceKeyMap keys;
        for (auto pair : m_fieldProperties)
            {
            if (-1 == pair.second)
                {
                // note: -1 means any property - in this case we iterate through all field properties and
                // see whether any ECInstaceKey matches that property
                bvector<ECClassInstanceKey> const* sourceVector = &m_primaryKeys;
                auto relatedFieldKeyIter = m_relatedFieldKeys.find(pair.first);
                if (m_relatedFieldKeys.end() != relatedFieldKeyIter)
                    sourceVector = &relatedFieldKeyIter->second;

                size_t propertyIndex = 0;
                for (ContentDescriptor::Property const& p : pair.first->GetProperties())
                    {
                    ContentSetItem::FieldPropertyIdentifier fieldProperty(*pair.first, propertyIndex++);
                    for (ECClassInstanceKeyCR key : *sourceVector)
                        {
                        if (key.GetClass()->Is(&p.GetPropertyClass()))
                            keys[fieldProperty].push_back(key);
                        }
                    }
                }
            else
                {
                // here we know exactly which property was changed, so just copy all ECInstanceKeys in that field
                // to the output map
                ContentSetItem::FieldPropertyIdentifier fieldProperty(*pair.first, pair.second);
                bvector<ECClassInstanceKey> const* sourceVector = &m_primaryKeys;
                auto relatedFieldKeyIter = m_relatedFieldKeys.find(pair.first);
                if (m_relatedFieldKeys.end() != relatedFieldKeyIter)
                    sourceVector = &relatedFieldKeyIter->second;
                std::copy(sourceVector->begin(), sourceVector->end(), std::back_inserter(keys[fieldProperty]));
                }
            }
        return keys;
        }
};
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentItemBuilder& ContentReader::GetItemInProgress()
    {
    if (!m_inProgress)
        {
        ContentDescriptorCR descriptor = m_contracts.GetContract()->GetDescriptor();
        if (descriptor.HasContentFlag(ContentFlags::MergeResults))
            m_inProgress = std::make_unique<MergingContentItemBuilder>(m_schemaManager, GetPropertyFormatter(), GetUnitSystem());
        else
            m_inProgress = std::make_unique<ContentItemBuilder>(m_builderContext, m_schemaManager, GetPropertyFormatter(), GetUnitSystem());
        }
    return *m_inProgress;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void SkipValues(int& sqlColumnIndex, bvector<ContentDescriptor::Field*> const& fields, ContentQueryContractCR contract)
    {
    for (ContentDescriptor::Field const* field : fields)
        {
        if (field->IsNestedContentField() && field->AsNestedContentField()->AsRelatedContentField())
            {
            ContentDescriptor::RelatedContentField const& relatedContentField = *field->AsNestedContentField()->AsRelatedContentField();
            if (!contract.ShouldHandleRelatedContentField(relatedContentField))
                continue; // don't increase sqlColumnIndex even for this field

            SkipValues(sqlColumnIndex, relatedContentField.GetFields(), contract);
            }

        if (contract.ShouldSkipCompositePropertyFields() && field->IsPropertiesField() && field->AsPropertiesField()->IsCompositePropertiesField())
            continue;

        ++sqlColumnIndex;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<ContentSetItemExtendedData::RelatedInstanceKey> ReadRelatedInstanceKeys(Utf8CP serializedJsonArray)
    {
    rapidjson::Document json;
    json.Parse(serializedJsonArray);
    return ContentSetItemExtendedData::RelatedInstanceKey::FromJsonArray(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void ReadValues(ContentItemBuilder& item, int& sqlColumnIndex, ECSqlStatementCR statement,
    bvector<ContentDescriptor::Field*> const& fields, ContentQueryContractCR contract, bool readLabels, bool shouldReadDirectValues)
    {
    for (ContentDescriptor::Field const* field : fields)
        {
        if (field->IsDisplayLabelField() && !readLabels)
            {
            // `readLabels == false` means the display label field was not included in the query - just skip it completely
            // and don't even increase the `sqlColumnIndex`
            continue;
            }

        Utf8StringCR fieldName = field->GetUniqueName();

        if (field->IsNestedContentField() && field->AsNestedContentField()->AsRelatedContentField())
            {
            ContentDescriptor::RelatedContentField const& relatedContentField = *field->AsNestedContentField()->AsRelatedContentField();
            if (contract.ShouldHandleRelatedContentField(relatedContentField))
                {
                if (statement.IsValueNull(sqlColumnIndex))
                    {
                    item.AddEmptyNestedContentValue(fieldName.c_str());
                    SkipValues(++sqlColumnIndex, relatedContentField.GetFields(), contract);
                    }
                else
                    {
                    ECInstanceKey relatedInstanceKey = ValueHelpers::GetECInstanceKeyFromJsonString(statement.GetValueText(sqlColumnIndex++));
                    ContentItemBuilder* existingNestedContentItem = item.GetNestedContentValue(fieldName.c_str(), relatedInstanceKey);
                    ReadValues(existingNestedContentItem ? *existingNestedContentItem : item.AddNestedContentValue(fieldName.c_str(), relatedInstanceKey), sqlColumnIndex, statement,
                        relatedContentField.GetFields(), contract, readLabels, nullptr == existingNestedContentItem);
                    }
                }
            continue;
            }

        if (contract.ShouldSkipCompositePropertyFields() && field->IsPropertiesField() && field->AsPropertiesField()->IsCompositePropertiesField())
            continue;

        if (!shouldReadDirectValues)
            {
            ++sqlColumnIndex;
            continue;
            }

        if (field->IsDisplayLabelField())
            {
            // if this is a display label field, set the display label definition
            item.SetLabel(*LabelDefinition::FromString(statement.GetValueText(sqlColumnIndex)));
            }
        else if (field->IsCalculatedPropertyField())
            {
            // if this is a calculated field, just append the value
            Utf8CP value = statement.GetValueText(sqlColumnIndex);
            item.AddValue(fieldName.c_str(), value, nullptr);
            }
        else if (field->IsPropertiesField())
            {
            ContentDescriptor::ECPropertiesField const& propertiesField = *field->AsPropertiesField();
            if (ContentDescriptor::Property const* matchingProperty = contract.FindMatchingProperty(propertiesField, item.GetRecordClass()))
                item.AddValue(fieldName.c_str(), matchingProperty->GetProperty(), statement.GetValue(sqlColumnIndex));
            else
                item.AddNull(fieldName.c_str(), nullptr);
            }
        ++sqlColumnIndex;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
QueryResultReaderStatus ContentReader::_ReadRecord(ContentSetItemPtr& record, ECSqlStatementCR statement)
    {
    record = nullptr;

    ContentQueryContractCPtr contract = m_contracts.GetContract();
    ContentDescriptorCR descriptor = contract->GetDescriptor();
    int columnIndex = 0;
    uint64_t contractId = 0;
    bool skipRead = false;
    bool shouldReadDirectValues = true;

#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
    if (descriptor.OnlyDistinctValues())
        {
        if (m_inProgress != nullptr)
            {
            record = GetItemInProgress().BuildItem();
            m_inProgress = nullptr;
            }
        }
    else
        {
#endif
        contractId = statement.GetValueUInt64(columnIndex++);
        ECInstanceKey instanceKey = ValueHelpers::GetECInstanceKeyFromJsonString(statement.GetValueText(columnIndex++));
        switch (GetItemInProgress().GetActionForPrimaryKey(instanceKey))
            {
            case ContentItemBuilder::ItemReadAction::Skip:
                {
                skipRead = true;
                break;
                }
            case ContentItemBuilder::ItemReadAction::ReadNestedFields:
                {
                // primary key matches the in-progress item which means all direct item values are already read
                shouldReadDirectValues = false;
                break;
                }
            case ContentItemBuilder::ItemReadAction::CreateNewItem:
                {
                // we found different primary key than what we have in progress, which means
                // the in-progress item is complete and we should start creating a new one
                record = GetItemInProgress().BuildItem();
                for (auto const& primaryKey : record->GetKeys())
                    m_createdItems.insert(make_bpair(primaryKey, record));
                m_inProgress = nullptr;
                // fall through
                }
            case ContentItemBuilder::ItemReadAction::ReadAllFields:
                {
                GetItemInProgress().SetPrimaryKey(instanceKey);
                GetItemInProgress().GetExtendedData().SetContractId(contractId);
                if (Mode::Skip == m_mode)
                    skipRead = true;
                break;
                }
            }

        if (Mode::Skip != m_mode)
            {
            ECInstanceKey inputInstanceKey = ValueHelpers::GetECInstanceKeyFromJsonString(statement.GetValueText(columnIndex++));
            if (inputInstanceKey.IsValid())
                {
                auto createdItemIter = m_createdItems.find(ValueHelpers::GetECClassInstanceKey(m_schemaManager, instanceKey));
                if (m_createdItems.end() != createdItemIter)
                    createdItemIter->second->GetInputKeys().push_back(ValueHelpers::GetECClassInstanceKey(m_schemaManager, inputInstanceKey));
                else
                    GetItemInProgress().SetInputKey(inputInstanceKey);
                }
            }
        else
            {
            ++columnIndex;
            }
#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
        }
#endif

    if (!descriptor.HasContentFlag(ContentFlags::KeysOnly) && !skipRead)
        {
#ifdef wip_field_instance_keys
        // TODO: associate fields with instance keys
        bool needFieldValueKeys = (0 == (descriptor.GetContentFlags() & (int)ContentFlags::ExcludeEditingData));
        FieldValueInstanceKeyReader fieldValueInstanceKeyReader(*statement.GetECDb(), primaryRecordKeys, needFieldValueKeys);
#endif

        ECClassCP recordClass = GetItemInProgress().GetRecordClass();
        if (recordClass && descriptor.ShowImages())
            GetItemInProgress().SetImageId(ImageHelper::GetImageId(*recordClass, true, false));

        GetItemInProgress().SetRelatedInstanceKeys(ReadRelatedInstanceKeys(statement.GetValueText(columnIndex++)));

        ReadValues(GetItemInProgress(), columnIndex, statement, descriptor.GetAllFields(),
            *m_contracts.GetContract(contractId), descriptor.ShowLabels(), shouldReadDirectValues);
        }

    // if the record is set, it means we finished previous in-progress item and started a new one - return
    // 'row' status as the record is complete. otherwise we're still working on the in-progress item and should
    // return 'skip' for the executor to step again
    return record.IsValid() ? QueryResultReaderStatus::Row : QueryResultReaderStatus::Skip;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentReader::_Complete(ContentSetItemPtr& record, ECSqlStatementCR)
    {
    bool result = false;
    if (Mode::Skip == m_mode)
        {
        result = true;
        }
    else if (m_inProgress != nullptr)
        {
        record = GetItemInProgress().BuildItem();
        result = true;
        }
    m_inProgress = nullptr;
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayValueGroupR DistinctValuesAccumulator::GetOrCreateDisplayValueGroup(Utf8StringCR displayValue)
    {
    auto iter = m_values.find(displayValue);
    return iter != m_values.end() ? *(iter->second) : *(m_values.emplace(displayValue, new DisplayValueGroup(displayValue)).first->second);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DistinctValuesAccumulator::ReadNavigationPropertyRecord(ContentDescriptor::ECPropertiesField const& propertiesField, ECSqlStatementCR statement)
    {
    auto value = ContentValueHelpers::ParseNavigationPropertyValue(statement.GetValue(0));
    if (value.first.IsNull())
        {
        DisplayValueGroupR group = GetOrCreateDisplayValueGroup("");
        group.GetRawValues().emplace_back(rapidjson::kNullType);
        }
    else
        {
        DisplayValueGroupR group = GetOrCreateDisplayValueGroup(value.first->GetDisplayValue());
        ECPresentationSerializerContext ctx;
        ECClassInstanceKey classInstanceKey(m_schemaManager.GetClass(value.second.GetClassId()), value.second.GetInstanceId());
        group.GetRawValues().push_back(ECPresentationManager::GetSerializer().AsJson(ctx, classInstanceKey, &group.GetRawValuesAllocator()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* note: we have to handle each property in the field separately just because formatter
* might format same raw values differently for different properties
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DistinctValuesAccumulator::ReadPrimitivePropertyRecord(ContentDescriptor::ECPropertiesField const& propertiesField, ECSqlStatementCR statement)
    {
    ContentValuesFormatter formatter(m_propertyFormatter, m_unitSystem);
    IECSqlValue const& value = statement.GetValue(0);
    bmap<Utf8String, bvector<ECValue>> currentRecords;
    for (ContentDescriptor::Property const& prop : propertiesField.GetProperties())
        {
        if (!prop.GetProperty().GetIsPrimitive())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Expecting property to be primitive, but it's not");

        rapidjson::Document formattedJson = formatter.GetFormattedValue(prop.GetProperty(), value, nullptr);
        if (!formattedJson.IsString() && !formattedJson.IsNull())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, Utf8PrintfString("Expecting formatter value to be string, but it's not. Actual: '%s'", BeRapidJsonUtilities::ToString(formattedJson).c_str()));

        Utf8String displayValue = formattedJson.IsString() ? formattedJson.GetString() : "";
        ECValue rawValue = ValueHelpers::GetECValueFromSqlValue(prop.GetProperty().GetAsPrimitiveProperty()->GetType(), value);

        auto it = currentRecords.find(displayValue);
        if (it == currentRecords.end())
            currentRecords.emplace(displayValue, bvector<ECValue>({rawValue}));
        else if (!ContainerHelpers::Contains(it->second, rawValue))
            it->second.push_back(rawValue);
        }

    for (auto pair : currentRecords)
        {
        DisplayValueGroupR group = GetOrCreateDisplayValueGroup(pair.first);
        for (auto valueInGroup : pair.second)
            group.GetRawValues().push_back(ValueHelpers::GetJsonFromECValue(valueInGroup, &group.GetRawValuesAllocator()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
QueryResultAccumulatorStatus DistinctValuesAccumulator::_ReadRow(ECSqlStatementCR statement)
    {
    if (m_field.IsDisplayLabelField())
        {
        // if this is a display label field, set the display label definition
        auto label = LabelDefinition::FromString(statement.GetValueText(0));
        DisplayValueGroupR group = GetOrCreateDisplayValueGroup(label->GetDisplayValue());
        group.GetRawValues().emplace_back(label->GetDisplayValue().c_str(), group.GetRawValuesAllocator());
        return QueryResultAccumulatorStatus::Continue;
        }

    if (m_field.IsCalculatedPropertyField())
        {
        // if this is a calculated field, just append the value
        Utf8CP value = statement.GetValueText(0);
        DisplayValueGroupR group = GetOrCreateDisplayValueGroup(value);
        group.GetRawValues().emplace_back(value, group.GetRawValuesAllocator());
        return QueryResultAccumulatorStatus::Continue;
        }

    if (m_field.IsPropertiesField())
        {
        ContentDescriptor::ECPropertiesField const& propertiesField = *m_field.AsPropertiesField();
        if (propertiesField.IsCompositePropertiesField())
            {
            // can't create a display value for arrays / structs
            return QueryResultAccumulatorStatus::Continue;
            }

        if (propertiesField.GetTypeDescription().GetTypeName().Equals("navigation"))
            ReadNavigationPropertyRecord(propertiesField, statement);
        else
            ReadPrimitivePropertyRecord(propertiesField, statement);

        return QueryResultAccumulatorStatus::Continue;
        }

    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Unexpected field type");
    }
