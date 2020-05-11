/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <regex>
#include "ContentQueryResultsReader.h"
#include "ImageHelper.h"

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsValueMerged(Utf8CP value)
    {
    static std::once_flag s_formatOnceFlag;
    static Utf8String s_format;
    std::call_once(s_formatOnceFlag, []()
        {
        s_format = "^" CONTENTRECORD_MERGED_VALUE_FORMAT "$";
        s_format.ReplaceAll("*", "\\*");
        s_format.ReplaceAll("%s", ".*?");
        });
    static std::regex s_regex(s_format.c_str());
    return std::regex_search(value, s_regex);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsNullQuotedDouble(PrimitiveType primitiveType, IECSqlValue const& value)
    {
    return primitiveType == PRIMITIVETYPE_Double
        && value.GetText() && 0 == strcmp("NULL", value.GetText());
    }

#define NULL_FORMATTED_VALUE_PRECONDITION(sqlValue) \
    if (sqlValue.IsNull()) \
        return rapidjson::Document();

#define NULL_FORMATTED_PRIMITIVE_VALUE_PRECONDITION(sqlValue, primitiveType) \
    NULL_FORMATTED_VALUE_PRECONDITION(sqlValue) \
    if (IsNullQuotedDouble(primitiveType, sqlValue)) \
        return rapidjson::Document();

//=======================================================================================
// @bsiclass                                     Grigas.Petraitis                10/2016
//=======================================================================================
struct ContentValuesFormatter
{
private:
    IECPropertyFormatter const* m_propertyFormatter;
    ECPresentation::UnitSystem m_unitSystem;

private:
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                09/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static rapidjson::Document GetFallbackValue(ECPropertyCR prop, IECSqlValue const& value,
        rapidjson::MemoryPoolAllocator<>* allocator);

    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                09/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static rapidjson::Document GetFallbackPrimitiveValue(ECPropertyCR prop, PrimitiveType type, IECSqlValue const& value, rapidjson::MemoryPoolAllocator<>* allocator)
        {
        NULL_FORMATTED_PRIMITIVE_VALUE_PRECONDITION(value, type);
        rapidjson::Document json(allocator);
        ECValue v = ValueHelpers::GetECValueFromSqlValue(type, value);
        Utf8String stringValue;
        if (v.ConvertPrimitiveToString(stringValue))
            json.SetString(stringValue.c_str(), json.GetAllocator());
        else
            {
            BeAssert(false);
            json.SetString("");
            }
        return json;
        }

    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                10/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Document GetFormattedPrimitiveValue(ECPropertyCR prop, PrimitiveType type, IECSqlValue const& value, rapidjson::MemoryPoolAllocator<>* allocator) const
        {
        NULL_FORMATTED_PRIMITIVE_VALUE_PRECONDITION(value, type);
        rapidjson::Document json(allocator);
        Utf8String formattedValue;
        if (!m_propertyFormatter || SUCCESS != m_propertyFormatter->GetFormattedPropertyValue(formattedValue, prop, ValueHelpers::GetECValueFromSqlValue(type, value), nullptr, m_unitSystem))
            return GetFallbackPrimitiveValue(prop, type, value, allocator);

        json.SetString(formattedValue.c_str(), json.GetAllocator());
        return json;
        }

    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                09/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static rapidjson::Document GetFallbackStructValue(IECSqlValue const& structValue, rapidjson::MemoryPoolAllocator<>* allocator)
        {
        NULL_FORMATTED_VALUE_PRECONDITION(structValue);
        rapidjson::Document json(allocator);
        json.SetObject();
        for (IECSqlValue const& value : structValue.GetStructIterable())
            {
            ECPropertyCP memberProperty = value.GetColumnInfo().GetProperty();
            json.AddMember(rapidjson::Value(memberProperty->GetName().c_str(), json.GetAllocator()),
                GetFallbackValue(*memberProperty, value, &json.GetAllocator()), json.GetAllocator());
            }
        return json;
        }

    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                09/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Document GetFormattedStructValue(IECSqlValue const& structValue, rapidjson::MemoryPoolAllocator<>* allocator) const
        {
        NULL_FORMATTED_VALUE_PRECONDITION(structValue);
        rapidjson::Document json(allocator);
        json.SetObject();
        for (IECSqlValue const& value : structValue.GetStructIterable())
            {
            ECPropertyCP memberProperty = value.GetColumnInfo().GetProperty();
            json.AddMember(rapidjson::Value(memberProperty->GetName().c_str(), json.GetAllocator()),
                GetFormattedValue(*memberProperty, value, &json.GetAllocator()), json.GetAllocator());
            }
        return json;
        }

    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                09/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static rapidjson::Document GetFallbackArrayValue(ArrayECPropertyCR prop, IECSqlValue const& arrayValue, rapidjson::MemoryPoolAllocator<>* allocator)
        {
        NULL_FORMATTED_VALUE_PRECONDITION(arrayValue);
        rapidjson::Document json(allocator);
        json.SetArray();
        for (IECSqlValue const& value : arrayValue.GetArrayIterable())
            {
            if (prop.GetIsStructArray())
                {
                json.PushBack(GetFallbackStructValue(value, &json.GetAllocator()), json.GetAllocator());
                }
            else if (prop.GetIsPrimitiveArray())
                {
                PrimitiveType primitiveType = prop.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();
                json.PushBack(GetFallbackPrimitiveValue(prop, primitiveType, value, &json.GetAllocator()), json.GetAllocator());
                }
            else
                {
                BeAssert(false);
                break;
                }
            }
        return json;
        }

    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                09/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    rapidjson::Document GetFormattedArrayValue(ArrayECPropertyCR prop, IECSqlValue const& arrayValue, rapidjson::MemoryPoolAllocator<>* allocator) const
        {
        NULL_FORMATTED_VALUE_PRECONDITION(arrayValue);
        rapidjson::Document json(allocator);
        json.SetArray();
        for (IECSqlValue const& value : arrayValue.GetArrayIterable())
            {
            if (prop.GetIsStructArray())
                {
                json.PushBack(GetFormattedStructValue(value, &json.GetAllocator()), json.GetAllocator());
                }
            else if (prop.GetIsPrimitiveArray())
                {
                PrimitiveType primitiveType = prop.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();
                json.PushBack(GetFormattedPrimitiveValue(prop, primitiveType, value, &json.GetAllocator()), json.GetAllocator());
                }
            else
                {
                BeAssert(false);
                break;
                }
            }
        return json;
        }

public:
    ContentValuesFormatter(IECPropertyFormatter const* formatter, ECPresentation::UnitSystem unitSystem)
        : m_propertyFormatter(formatter), m_unitSystem(unitSystem)
        {}
    rapidjson::Document GetFormattedValue(ECPropertyCR prop, IECSqlValue const& value, rapidjson::MemoryPoolAllocator<>* allocator) const;
};

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentValuesFormatter::GetFormattedValue(ECPropertyCR prop, IECSqlValue const& value, rapidjson::MemoryPoolAllocator<>* allocator) const
    {
    if (prop.GetIsPrimitive())
        return GetFormattedPrimitiveValue(prop, prop.GetAsPrimitiveProperty()->GetType(), value, allocator);
    if (prop.GetIsArray())
        return GetFormattedArrayValue(*prop.GetAsArrayProperty(), value, allocator);
    if (prop.GetIsStruct())
        return GetFormattedStructValue(value, allocator);
    BeAssert(false && "Unexpected property type");
    return rapidjson::Document(allocator);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentValuesFormatter::GetFallbackValue(ECPropertyCR prop, IECSqlValue const& value, rapidjson::MemoryPoolAllocator<>* allocator)
    {
    if (prop.GetIsPrimitive())
        return GetFallbackPrimitiveValue(prop, prop.GetAsPrimitiveProperty()->GetType(), value, allocator);
    if (prop.GetIsArray())
        return GetFallbackArrayValue(*prop.GetAsArrayProperty(), value, allocator);
    if (prop.GetIsStruct())
        return GetFallbackStructValue(value, allocator);
    BeAssert(false && "Unexpected property type");
    return rapidjson::Document(allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
static bpair<LabelDefinitionCPtr, ECInstanceKey> ParseNavigationPropertyValue(IECSqlValue const& value)
    {
    if (value.IsNull())
        return bpair<LabelDefinitionCPtr, ECInstanceKey>();

    rapidjson::Document json;
    json.Parse(value.GetText());
    auto label = LabelDefinition::FromInternalJson(json["label"]);
    ECInstanceKey key = ValueHelpers::GetECInstanceKeyFromJson(json["key"]);
    return bpair<LabelDefinitionCPtr, ECInstanceKey>(label, key);
    }

//=======================================================================================
// @bsiclass                                     Grigas.Petraitis                10/2016
//=======================================================================================
struct ContentValueAppender
{
    enum ValueToAdd
        {
        ADD_Value = 1,
        ADD_DisplayValue = 2,
        ADD_Both = ADD_Value | ADD_DisplayValue,
        };

private:
    ContentValuesFormatter m_formatter;
    rapidjson::Document m_values;
    rapidjson::Document m_displayValues;
    bvector<Utf8String> m_mergedFieldNames;

public:
    ContentValueAppender(IECPropertyFormatter const* propertyFormatter, ECPresentation::UnitSystem unitSystem)
        : m_formatter(propertyFormatter, unitSystem)
        {
        m_values.SetObject();
        m_displayValues.SetObject();
        }
    rapidjson::Document&& GetValues() {return std::move(m_values);}
    rapidjson::Document& GetValuesR() {return m_values;}
    rapidjson::Document&& GetDisplayValues() {return std::move(m_displayValues);}
    rapidjson::Document& GetDisplayValuesR() {return m_displayValues;}
    bvector<Utf8String> GetMergedFieldNames() const {return m_mergedFieldNames;}
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddValue(Utf8CP name, rapidjson::Value&& value, rapidjson::Value&& displayValue, int add = ADD_Both)
        {
        if (0 != (ADD_Value & add))
            m_values.AddMember(rapidjson::Value(name, m_values.GetAllocator()), value, m_values.GetAllocator());
        if (0 != (ADD_DisplayValue & add))
            m_displayValues.AddMember(rapidjson::Value(name, m_displayValues.GetAllocator()), displayValue, m_displayValues.GetAllocator());
        }
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddNull(Utf8CP name, int add = ADD_Both)
        {
        AddValue(name, rapidjson::Value(), rapidjson::Value(), add);
        }
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddValue(Utf8CP name, Utf8CP value, Utf8CP displayValue, bool isMerged, int add = ADD_Both)
        {
        if (nullptr == value)
            AddNull(name, add);
        else
            AddValue(name, rapidjson::Value(value, m_values.GetAllocator()), rapidjson::Value(displayValue, m_displayValues.GetAllocator()), add);

        if (isMerged)
            m_mergedFieldNames.push_back(name);
        }
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddValue(Utf8CP name, ECPropertyCR ecProperty, IECSqlValue const& value, bool possiblyMerged)
        {
        if (value.IsNull())
            {
            AddNull(name);
            return;
            }

        if (possiblyMerged && IsValueMerged(value.GetText()))
            {
            if (ecProperty.GetIsNavigation())
                AddValue(name, rapidjson::Value(), rapidjson::Value(value.GetText(), m_displayValues.GetAllocator()));
            else
                AddValue(name, rapidjson::Value(value.GetText(), m_values.GetAllocator()), rapidjson::Value(value.GetText(), m_displayValues.GetAllocator()));
            m_mergedFieldNames.push_back(name);
            }
        else if (ecProperty.GetIsPrimitive())
            {
            PrimitiveECPropertyCR primitiveProperty = *ecProperty.GetAsPrimitiveProperty();
            if (IsNullQuotedDouble(primitiveProperty.GetType(), value))
                {
                AddNull(name);
                }
            else
                {
                AddValue(name,
                    ValueHelpers::GetJsonFromPrimitiveValue(primitiveProperty.GetType(), value, &m_values.GetAllocator()),
                    m_formatter.GetFormattedValue(primitiveProperty, value, &m_displayValues.GetAllocator()));
                }
            }
        else if (ecProperty.GetIsStruct())
            {
            StructECPropertyCR structProperty = *ecProperty.GetAsStructProperty();
            AddValue(name,
                ValueHelpers::GetJsonFromStructValue(structProperty.GetType(), value, &m_values.GetAllocator()),
                m_formatter.GetFormattedValue(structProperty, value, &m_displayValues.GetAllocator()));
            }
        else if (ecProperty.GetIsArray())
            {
            ArrayECPropertyCR arrayProperty = *ecProperty.GetAsArrayProperty();
            AddValue(name,
                ValueHelpers::GetJsonFromArrayValue(value, &m_values.GetAllocator()),
                m_formatter.GetFormattedValue(arrayProperty, value, &m_displayValues.GetAllocator()));
            }
        else if (ecProperty.GetIsNavigation())
            {
            auto parsedValue = ParseNavigationPropertyValue(value);
            if (parsedValue.first.IsNull())
                {
                AddNull(name);
                }
            else
                {
                AddValue(name, rapidjson::Value(parsedValue.second.GetInstanceId().GetValue()),
                    rapidjson::Value(parsedValue.first->GetDisplayValue().c_str(), m_displayValues.GetAllocator()));
                }
            }
        else
            {
            // if the property is not primitive, append NULL
            AddNull(name);
            }
        }
};

//=======================================================================================
// @bsiclass                                     Grigas.Petraitis               06/2017
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
    // @bsimethod                                    Grigas.Petraitis               06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    FieldValueInstanceKeyReader(ECDbCR db, bvector<ECClassInstanceKey> const& primaryKeys, bool trackKeys)
        : m_db(db), m_primaryKeys(primaryKeys), m_trackKeys(trackKeys)
        {}

    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis               06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ReadFieldKeys(ContentDescriptor::ECInstanceKeyField const& field, ECSqlStatementCR statement, int columnIndex)
        {
        if (!m_trackKeys)
            return;

        if (statement.IsValueNull(columnIndex))
            return;

        bvector<ECInstanceKey> keys = ValueHelpers::GetECInstanceKeysFromSerializedJson(statement.GetValueText(columnIndex));
        for (ECInstanceKeyCR key : keys)
            {
            ECClassCP ecClass = m_db.Schemas().GetClass(key.GetClassId());
            for (ContentDescriptor::ECPropertiesField const* field : field.GetKeyFields())
                m_relatedFieldKeys[field].push_back(ECClassInstanceKey(ecClass, key.GetInstanceId()));
            }
        }

    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis               06/2017
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
        BeAssert(propertyIndex < field.GetProperties().size());
        m_fieldProperties[&field] = (int)propertyIndex;
        }

    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis               06/2017
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
                    ContentSetItem::FieldProperty fieldProperty(*pair.first, propertyIndex++);
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
                ContentSetItem::FieldProperty fieldProperty(*pair.first, pair.second);
                bvector<ECClassInstanceKey> const* sourceVector = &m_primaryKeys;
                auto relatedFieldKeyIter = m_relatedFieldKeys.find(&fieldProperty.GetField());
                if (m_relatedFieldKeys.end() != relatedFieldKeyIter)
                    sourceVector = &relatedFieldKeyIter->second;
                std::copy(sourceVector->begin(), sourceVector->end(), std::back_inserter(keys[fieldProperty]));
                }
            }
        return keys;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
QueryResultReaderStatus ContentReader::_ReadRecord(ContentSetItemPtr& record, ECSqlStatementCR statement)
    {
    ContentQueryContractCPtr contract = m_query.GetContract();
    ContentDescriptorCR descriptor = contract->GetDescriptor();
    bool resultsMerged = descriptor.HasContentFlag(ContentFlags::MergeResults);
    int columnIndex = 0;
    uint64_t contractId = 0;
    Utf8CP relatedInstanceInfo = nullptr;
    bvector<ECClassInstanceKey> primaryRecordKeys;

#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
    if (!descriptor.OnlyDistinctValues())
        {
#endif
        contractId = statement.GetValueUInt64(columnIndex++);
        bvector<ECInstanceKey> ecInstanceKeys = ValueHelpers::GetECInstanceKeysFromSerializedJson(statement.GetValueText(columnIndex++));
        BeAssert(1 == ecInstanceKeys.size() || resultsMerged);
        if (!resultsMerged)
            {
            auto readKeysIter = m_readKeys.find(ecInstanceKeys.front());
            if (m_readKeys.end() != readKeysIter)
                return QueryResultReaderStatus::Skip;

            m_readKeys.insert(readKeysIter, ecInstanceKeys.front());
            }
        for (ECInstanceKeyCR key : ecInstanceKeys)
            {
            ECClassCP keyClass = statement.GetECDb()->Schemas().GetClass(key.GetClassId());
            primaryRecordKeys.push_back(ECClassInstanceKey(keyClass, key.GetInstanceId()));
            }
#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
        }
#endif

    bool needFieldValueKeys = (0 == (descriptor.GetContentFlags() & (int)ContentFlags::ExcludeEditingData));
    FieldValueInstanceKeyReader fieldValueInstanceKeyReader(*statement.GetECDb(), primaryRecordKeys, needFieldValueKeys);

    ContentValueAppender values(m_propertyFormatter, m_unitSystem);
    Utf8String imageId;
    LabelDefinitionPtr displayLabelDefinition = LabelDefinition::Create();
    ECClassCP recordClass = nullptr;
    if (0 == ((int)ContentFlags::KeysOnly & descriptor.GetContentFlags()))
        {
        for (ECClassInstanceKeyCR primaryKey : primaryRecordKeys)
            {
            if (nullptr == recordClass)
                recordClass = primaryKey.GetClass();
            else if (recordClass != primaryKey.GetClass())
                {
                recordClass = nullptr;
                break;
                }
            }

        bool possiblyMerged = (resultsMerged && primaryRecordKeys.size() > 1);
        // note: the record consists of multiple merged ECInstance values - not clear
        // which one's related instance info should be used for customizing
        if (!possiblyMerged)
            relatedInstanceInfo = statement.GetValueText(columnIndex);
        columnIndex++;

        ContentQueryContractCPtr fieldsContract = m_query.GetContract(contractId);
        for (ContentDescriptor::Field const* field : descriptor.GetAllFields())
            {
            Utf8StringCR fieldName = field->GetUniqueName();
            if (field->IsDisplayLabelField() && descriptor.ShowLabels())
                {
                // if this is a display label field, set the display label definition
                displayLabelDefinition = LabelDefinition::FromString(statement.GetValueText(columnIndex));
                }
            else if (field->IsCalculatedPropertyField())
                {
                // if this is a calculated field, just append the value
                Utf8CP value = statement.GetValueText(columnIndex);
                values.AddValue(fieldName.c_str(), value, value, (possiblyMerged && IsValueMerged(value)));
                }
            else if (field->IsPropertiesField())
                {
                ContentDescriptor::ECPropertiesField const& propertiesField = *field->AsPropertiesField();
                if (contract->ShouldSkipCompositePropertyFields() && propertiesField.IsCompositePropertiesField())
                    continue; // do not increase columnIndex if we didn't use the field

                ContentDescriptor::Property const* matchingProperty = nullptr;
                if (resultsMerged)
                    {
                    if (!propertiesField.GetProperties().empty())
                        matchingProperty = &propertiesField.GetProperties().front();
                    }
                else
                    {
                    // find the field property that's appropriate for this instance
                    matchingProperty = fieldsContract->FindMatchingProperty(propertiesField, recordClass);
                    }

                if (nullptr != matchingProperty)
                    {
                    values.AddValue(fieldName.c_str(), matchingProperty->GetProperty(), statement.GetValue(columnIndex), possiblyMerged);
                    fieldValueInstanceKeyReader.SetFieldProperty(propertiesField, *matchingProperty, possiblyMerged);
                    }
                else
                    {
                    // if the property was not found, append NULL
                    values.AddNull(fieldName.c_str());
                    }
                }
            else if (field->IsSystemField() && field->AsSystemField()->IsECInstanceKeyField())
                {
                fieldValueInstanceKeyReader.ReadFieldKeys(*field->AsSystemField()->AsECInstanceKeyField(), statement, columnIndex);
                }
            else
                {
                // do not increase columnIndex if we didn't use the field
                continue;
                }
            columnIndex++;
            }

        if (descriptor.ShowImages() && nullptr != recordClass)
            imageId = ImageHelper::GetImageId(*recordClass, true, false);
        }

    record = ContentSetItem::Create(primaryRecordKeys, *displayLabelDefinition, imageId,
        values.GetValues(), values.GetDisplayValues(), values.GetMergedFieldNames(), fieldValueInstanceKeyReader.GetKeys());
    record->SetClass(recordClass);
    ContentSetItemExtendedData(*record).SetContractId(contractId);
    ContentSetItemExtendedData(*record).SetRelatedInstanceKeys(relatedInstanceInfo);
    return QueryResultReaderStatus::Row;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
QueryResultReaderStatus DistinctValuesReader::PopRecord(bpair<Utf8String, ECValue>& record)
    {
    if (m_currRecords.empty())
        return QueryResultReaderStatus::Skip;

    record = m_currRecords.front();
    m_currRecords.pop_front();
    return m_currRecords.empty() ? QueryResultReaderStatus::Row : QueryResultReaderStatus::RowAndRepeat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void DistinctValuesReader::ReadNavigationPropertyRecord(ContentDescriptor::ECPropertiesField const& propertiesField, ECSqlStatementCR statement)
    {
    auto value = ParseNavigationPropertyValue(statement.GetValue(0));
    if (value.first.IsNull())
        {
        ECValue nullValue;
        nullValue.SetIsNull(true);
        m_currRecords.push_back(bpair<Utf8String, ECValue>("", nullValue));
        }
    else
        {
        m_currRecords.push_back(bpair<Utf8String, ECValue>(value.first->GetDisplayValue(), ECValue(value.second.GetInstanceId())));
        }
    }

/*---------------------------------------------------------------------------------**//**
* note: we have to handle each property in the field separately just because formatter
* might format same raw values differently for different properties
* @bsimethod                                    Grigas.Petraitis                05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void DistinctValuesReader::ReadPrimitivePropertyRecord(ContentDescriptor::ECPropertiesField const& propertiesField, ECSqlStatementCR statement)
    {
    ContentValuesFormatter formatter(m_propertyFormatter, m_unitSystem);
    IECSqlValue const& value = statement.GetValue(0);
    for (ContentDescriptor::Property const& prop : propertiesField.GetProperties())
        {
        if (!prop.GetProperty().GetIsPrimitive())
            {
            BeAssert(false);
            continue;
            }

        rapidjson::Document formattedJson = formatter.GetFormattedValue(prop.GetProperty(), value, nullptr);
        if (!formattedJson.IsString() && !formattedJson.IsNull())
            {
            BeAssert(false);
            continue;
            }

        Utf8String displayValue = formattedJson.IsString() ? formattedJson.GetString() : "";
        ECValue rawValue = ValueHelpers::GetECValueFromSqlValue(prop.GetProperty().GetAsPrimitiveProperty()->GetType(), value);

        bool exists = false;
        for (auto const& existingEntry : m_currRecords)
            {
            if (existingEntry.first == displayValue && existingEntry.second == rawValue)
                {
                exists = true;
                break;
                }
            }
        if (!exists)
            m_currRecords.push_back(bpair<Utf8String, ECValue>(displayValue, rawValue));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2020
+---------------+---------------+---------------+---------------+---------------+------*/
QueryResultReaderStatus DistinctValuesReader::_ReadRecord(bpair<Utf8String, ECValue>& record, ECSqlStatementCR statement)
    {
    if (!m_currRecords.empty())
        return PopRecord(record);

    if (m_field.IsDisplayLabelField())
        {
        // if this is a display label field, set the display label definition
        auto label = LabelDefinition::FromString(statement.GetValueText(0));
        record = bpair<Utf8String, ECValue>(label->GetDisplayValue(), ECValue(label->GetDisplayValue().c_str()));
        return QueryResultReaderStatus::Row;
        }

    if (m_field.IsCalculatedPropertyField())
        {
        // if this is a calculated field, just append the value
        Utf8CP value = statement.GetValueText(0);
        record = bpair<Utf8String, ECValue>(value, ECValue(value));
        return QueryResultReaderStatus::Row;
        }

    if (m_field.IsPropertiesField())
        {
        ContentDescriptor::ECPropertiesField const& propertiesField = *m_field.AsPropertiesField();
        if (propertiesField.IsCompositePropertiesField())
            {
            // can't create a display value for arrays / structs
            return QueryResultReaderStatus::Skip;
            }

        if (propertiesField.GetTypeDescription().GetTypeName().Equals("navigation"))
            ReadNavigationPropertyRecord(propertiesField, statement);
        else
            ReadPrimitivePropertyRecord(propertiesField, statement);

        return PopRecord(record);
        }

    BeAssert(false);
    return QueryResultReaderStatus::Error;
    }
