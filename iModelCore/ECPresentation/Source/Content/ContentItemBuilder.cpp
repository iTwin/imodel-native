/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <regex>
#include "ContentItemBuilder.h"
#include "../Shared/ValueHelpers.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION

#define NULL_FORMATTED_VALUE_PRECONDITION(sqlValue) \
    if (sqlValue.IsNull()) \
        return rapidjson::Document();

#define NULL_FORMATTED_PRIMITIVE_VALUE_PRECONDITION(sqlValue, primitiveType) \
    NULL_FORMATTED_VALUE_PRECONDITION(sqlValue)

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentValuesFormatter::GetFallbackPrimitiveValue(ECPropertyCR prop, PrimitiveType type, IECSqlValue const& value, rapidjson::MemoryPoolAllocator<>* allocator)
    {
    NULL_FORMATTED_PRIMITIVE_VALUE_PRECONDITION(value, type);
    rapidjson::Document json(allocator);
    ECValue v = ValueHelpers::GetECValueFromSqlValue(type, value);
    Utf8String stringValue;
    if (v.ConvertPrimitiveToString(stringValue))
        json.SetString(stringValue.c_str(), json.GetAllocator());
    else
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_ERROR, Utf8PrintfString("Failed to convert ECValue to string - returning empty string. Value: '%s'", v.ToString().c_str()));
        json.SetString("");
        }
    return json;
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentValuesFormatter::GetFormattedPrimitiveValue(ECPropertyCR prop, PrimitiveType type, IECSqlValue const& value, rapidjson::MemoryPoolAllocator<>* allocator) const
    {
    NULL_FORMATTED_PRIMITIVE_VALUE_PRECONDITION(value, type);
    rapidjson::Document json(allocator);
    Utf8String formattedValue;
    if (!m_propertyFormatter || SUCCESS != m_propertyFormatter->GetFormattedPropertyValue(formattedValue, prop, ValueHelpers::GetECValueFromSqlValue(type, value), m_unitSystem))
        return GetFallbackPrimitiveValue(prop, type, value, allocator);

    json.SetString(formattedValue.c_str(), json.GetAllocator());
    return json;
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentValuesFormatter::GetFallbackStructValue(IECSqlValue const& structValue, rapidjson::MemoryPoolAllocator<>* allocator)
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
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentValuesFormatter::GetFormattedStructValue(IECSqlValue const& structValue, rapidjson::MemoryPoolAllocator<>* allocator) const
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
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentValuesFormatter::GetFallbackArrayValue(ArrayECPropertyCR prop, IECSqlValue const& arrayValue, rapidjson::MemoryPoolAllocator<>* allocator)
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
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Unknown type of ArrayECProperty");
            }
        }
    return json;
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentValuesFormatter::GetFormattedArrayValue(ArrayECPropertyCR prop, IECSqlValue const& arrayValue, rapidjson::MemoryPoolAllocator<>* allocator) const
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
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Unknown type of ArrayECProperty");
            }
        }
    return json;
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentValuesFormatter::GetFormattedValue(ECPropertyCR prop, IECSqlValue const& value, rapidjson::MemoryPoolAllocator<>* allocator) const
    {
    if (prop.GetIsPrimitive())
        return GetFormattedPrimitiveValue(prop, prop.GetAsPrimitiveProperty()->GetType(), value, allocator);
    if (prop.GetIsArray())
        return GetFormattedArrayValue(*prop.GetAsArrayProperty(), value, allocator);
    if (prop.GetIsStruct())
        return GetFormattedStructValue(value, allocator);
    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Unexpected ECProperty type");
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentValuesFormatter::GetFallbackValue(ECPropertyCR prop, IECSqlValue const& value, rapidjson::MemoryPoolAllocator<>* allocator)
    {
    if (prop.GetIsPrimitive())
        return GetFallbackPrimitiveValue(prop, prop.GetAsPrimitiveProperty()->GetType(), value, allocator);
    if (prop.GetIsArray())
        return GetFallbackArrayValue(*prop.GetAsArrayProperty(), value, allocator);
    if (prop.GetIsStruct())
        return GetFallbackStructValue(value, allocator);
    DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, "Unexpected ECProperty type");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bpair<LabelDefinitionCPtr, ECInstanceKey> ContentValueHelpers::ParseNavigationPropertyValue(IECSqlValue const& value)
    {
    if (value.IsNull())
        return bpair<LabelDefinitionCPtr, ECInstanceKey>();

    rapidjson::Document::AllocatorType jsonAllocator(1024); // 1kB should be enough (default is 64kB)
    rapidjson::Document json(&jsonAllocator);
    json.Parse(value.GetText());
    auto label = LabelDefinition::FromInternalJson(json["label"]);
    ECInstanceKey key = ValueHelpers::GetECInstanceKeyFromJson(json["key"]);
    return bpair<LabelDefinitionCPtr, ECInstanceKey>(label, key);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentItemBuilder::ItemReadAction ContentItemBuilder::_GetActionForPrimaryKey(ECInstanceKeyCR key) const
    {
    if (ContainerHelpers::Contains(GetPrimaryKeys(), key))
        return ItemReadAction::ReadNestedFields;

    if (m_context->readInstanceKeys.end() != m_context->readInstanceKeys.find(key))
        return ItemReadAction::Skip;

    m_context->readInstanceKeys.insert(key);

    if (GetPrimaryKeys().empty())
        return ItemReadAction::ReadAllFields;

    return ItemReadAction::CreateNewItem;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ContentItemBuilder::GetRecordClass() const
    {
    if (!m_determinedPrimaryInstanceClass)
        {
        ECClassId id;
        for (ECInstanceKeyCR key : m_primaryKeys)
            {
            if (!id.IsValid())
                id = key.GetClassId();
            else if (id != key.GetClassId())
                {
                id.Invalidate();
                break;
                }
            }
        m_primaryInstanceClass = id.IsValid() ? m_schemaManager.GetClass(id) : nullptr;
        m_determinedPrimaryInstanceClass = true;
        }
    return m_primaryInstanceClass;
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentItemBuilder::_AddValue(Utf8CP name, rapidjson::Value&& value, rapidjson::Value&& displayValue, ECPropertyCP)
    {
    m_values.AddMember(rapidjson::Value(name, m_values.GetAllocator()), value, m_values.GetAllocator());
    m_displayValues.AddMember(rapidjson::Value(name, m_displayValues.GetAllocator()), displayValue, m_displayValues.GetAllocator());
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentItemBuilder::AddNull(Utf8CP name, ECPropertyCP prop)
    {
    if (BeforeAddValueStatus::Skip == _OnBeforeAddValue(name))
        return;

    _AddValue(name, rapidjson::Value(), rapidjson::Value(), prop);
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentItemBuilder::AddValue(Utf8CP name, Utf8CP rawAndDisplayValue, ECPropertyCP prop)
    {
    if (BeforeAddValueStatus::Skip == _OnBeforeAddValue(name))
        return;

    if (!rawAndDisplayValue)
        AddNull(name, prop);
    else
        _AddValue(name, rapidjson::Value(rawAndDisplayValue, m_values.GetAllocator()), rapidjson::Value(rawAndDisplayValue, m_displayValues.GetAllocator()), prop);
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentItemBuilder::AddValue(Utf8CP name, ECPropertyCR ecProperty, IECSqlValue const& value)
    {
    if (BeforeAddValueStatus::Skip == _OnBeforeAddValue(name))
        return;

    if (value.IsNull())
        {
        AddNull(name, &ecProperty);
        return;
        }

    if (ecProperty.GetIsPrimitive())
        {
        PrimitiveECPropertyCR primitiveProperty = *ecProperty.GetAsPrimitiveProperty();
        _AddValue(name,
            ValueHelpers::GetJsonFromPrimitiveValue(primitiveProperty.GetType(), value, &m_values.GetAllocator()),
            m_formatter.GetFormattedValue(primitiveProperty, value, &m_displayValues.GetAllocator()), &ecProperty);
        }
    else if (ecProperty.GetIsStruct())
        {
        StructECPropertyCR structProperty = *ecProperty.GetAsStructProperty();
        _AddValue(name,
            ValueHelpers::GetJsonFromStructValue(structProperty.GetType(), value, &m_values.GetAllocator()),
            m_formatter.GetFormattedValue(structProperty, value, &m_displayValues.GetAllocator()), &ecProperty);
        }
    else if (ecProperty.GetIsArray())
        {
        ArrayECPropertyCR arrayProperty = *ecProperty.GetAsArrayProperty();
        _AddValue(name,
            ValueHelpers::GetJsonFromArrayValue(value, &m_values.GetAllocator()),
            m_formatter.GetFormattedValue(arrayProperty, value, &m_displayValues.GetAllocator()), &ecProperty);
        }
    else if (ecProperty.GetIsNavigation())
        {
        auto parsedValue = ContentValueHelpers::ParseNavigationPropertyValue(value);
        if (parsedValue.first.IsNull())
            {
            AddNull(name, &ecProperty);
            }
        else
            {
            ECPresentationSerializerContext ctx;
            ECClassInstanceKey classInstanceKey(m_schemaManager.GetClass(parsedValue.second.GetClassId()), parsedValue.second.GetInstanceId());
            _AddValue(name, ECPresentationManager::GetSerializer().AsJson(ctx, classInstanceKey, &m_values.GetAllocator()),
                rapidjson::Value(parsedValue.first->GetDisplayValue().c_str(), m_displayValues.GetAllocator()), &ecProperty);
            }
        }
    else
        {
        AddNull(name, &ecProperty);
        }
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentItemBuilder::_AddEmptyNestedContentValue(Utf8CP name)
    {
    auto iter = m_nestedContentValues.find(name);
    if (m_nestedContentValues.end() == iter)
        m_nestedContentValues.insert(std::make_pair(name, NestedContentValues()));
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentItemBuilder& ContentItemBuilder::_AddNestedContentValue(Utf8CP name, ECInstanceKey const& key)
    {
    auto iter = m_nestedContentValues.find(name);
    if (m_nestedContentValues.end() == iter)
        iter = m_nestedContentValues.insert(std::make_pair(name, NestedContentValues())).first;
    iter->second.values.push_back(_CreateNestedContentItemBuilder());
    iter->second.values.back()->SetPrimaryKey(key);
    iter->second.values.back()->GetExtendedData().MergeWith(GetExtendedData());
    return *iter->second.values.back();
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentItemBuilder* ContentItemBuilder::_GetNestedContentValue(Utf8CP name, ECInstanceKey const& key) const
    {
    auto iter = m_nestedContentValues.find(name);
    if (m_nestedContentValues.end() == iter)
        return nullptr;

    for (auto const& item : iter->second.values)
        {
        if (ContainerHelpers::Contains(item->GetPrimaryKeys(), key))
            return item.get();
        }
    return nullptr;
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentItemBuilder::_CaptureNestedContentValueCounts()
    {
    for (auto& entry : m_nestedContentValues)
        {
        DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Content, entry.second.capturedCount.IsNull() || entry.second.capturedCount == entry.second.values.size(), "Expecting captured count of nested content values to either be unset or equal to values count");
        entry.second.capturedCount = entry.second.values.size();
        }
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSetItemPtr ContentItemBuilder::BuildItem()
    {
    static Utf8PrintfString const s_variesStr(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);

    _CaptureNestedContentValueCounts();

    bvector<ECClassInstanceKey> primaryKeys = ContainerHelpers::TransformContainer<bvector<ECClassInstanceKey>>(m_primaryKeys, [&](ECInstanceKeyCR key)
        {
        return ValueHelpers::GetECClassInstanceKey(m_schemaManager, key);
        });
    bvector<ECClassInstanceKey> inputKeys = ContainerHelpers::TransformContainer<bvector<ECClassInstanceKey>>(m_inputKeys, [&](ECInstanceKeyCR key)
        {
        return ValueHelpers::GetECClassInstanceKey(m_schemaManager, key);
        });

    bmap<Utf8String, bvector<ContentSetItemPtr>> nestedContentItems;
    for (auto const& entry : m_nestedContentValues)
        {
        if (GetMergedFieldNames().end() != GetMergedFieldNames().find(entry.first))
            {
            GetValues().AddMember(rapidjson::Value(entry.first.c_str(), GetValues().GetAllocator()), rapidjson::Value(), GetValues().GetAllocator());
            GetDisplayValues().AddMember(rapidjson::Value(entry.first.c_str(), GetDisplayValues().GetAllocator()), rapidjson::Value(s_variesStr.c_str(), GetDisplayValues().GetAllocator()), GetDisplayValues().GetAllocator());
            continue;
            }
        nestedContentItems.Insert(entry.first, ContainerHelpers::TransformContainer<bvector<ContentSetItemPtr>>(entry.second.values, [](auto const& itemBuilder) {return itemBuilder->BuildItem(); }));
        }

    LabelDefinitionCPtr label = m_displayLabel;
    if (label.IsNull())
        {
        static LabelDefinitionPtr const s_emptyLabel = LabelDefinition::Create();
        label = s_emptyLabel.get();
        }

    auto item = ContentSetItem::Create(inputKeys, primaryKeys, *label, m_imageId, nestedContentItems,
        std::move(m_values), std::move(m_displayValues), ContainerHelpers::TransformContainer<bvector<Utf8String>>(m_mergedFieldNames),
        ContentSetItem::FieldPropertyInstanceKeyMap()); // wip_field_instance_keys
    item->SetClass(GetRecordClass());
    ContentSetItemExtendedData(*item).MergeWith(m_extendedData);
    return item;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NoopContentItemBuilder : ContentItemBuilder
{
protected:
    ItemReadAction _GetActionForPrimaryKey(ECInstanceKeyCR) const override { return ItemReadAction::Skip; }
    void _SetPrimaryKey(ECInstanceKey const&) override {}
    void _SetInputKey(ECInstanceKey const&) override {}
    void _SetLabel(LabelDefinitionCR) override {}
    void _SetImageId(Utf8String) override {}
    void _SetRelatedInstanceKeys(bvector<ContentSetItemExtendedData::RelatedInstanceKey> const&) override {}
    BeforeAddValueStatus _OnBeforeAddValue(Utf8CP) override { return BeforeAddValueStatus::Skip; }
    void _AddValue(Utf8CP, rapidjson::Value&&, rapidjson::Value&&, ECPropertyCP) override {}
    std::unique_ptr<ContentItemBuilder> _CreateNestedContentItemBuilder() const override { return std::make_unique<NoopContentItemBuilder>(GetSchemaManager(), GetValuesFormatter()); }
    void _AddEmptyNestedContentValue(Utf8CP) override {}
    ContentItemBuilder& _AddNestedContentValue(Utf8CP name, ECInstanceKey const&) override { return *this; }
    void _CaptureNestedContentValueCounts() override {}

public:
    NoopContentItemBuilder(SchemaManagerCR schemaManager, ContentValuesFormatter formatter)
        : ContentItemBuilder(nullptr, schemaManager, formatter)
        {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentItemBuilder::ItemReadAction MergingContentItemBuilder::_GetActionForPrimaryKey(ECInstanceKeyCR key) const
    {
    if (!ContainerHelpers::Contains(GetPrimaryKeys(), key))
        return ItemReadAction::ReadAllFields;
    return ItemReadAction::ReadNestedFields;
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MergingContentItemBuilder::_SetPrimaryKey(ECInstanceKey const& key)
    {
    if (!GetPrimaryKeys().empty())
        _CaptureNestedContentValueCounts();
    GetPrimaryKeysR().push_back(key);
    InvalidateInstanceClass();
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MergingContentItemBuilder::_SetInputKey(ECInstanceKey const& key)
    {
    GetInputKeysR().push_back(key);
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MergingContentItemBuilder::_SetLabel(LabelDefinitionCR definition)
    {
    if (GetLabel().IsValid() && (*GetLabel()) != definition)
        {
        ContentItemBuilder::_SetLabel(*LabelDefinition::Create(CommonStrings::RULESENGINE_MULTIPLEINSTANCES));
        }
    else
        ContentItemBuilder::_SetLabel(definition);
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MergingContentItemBuilder::_SetImageId(Utf8String id)
    {
    if (GetImageId() != id)
        ContentItemBuilder::_SetImageId("");
    else
        ContentItemBuilder::_SetImageId(id);
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MergingContentItemBuilder::_SetRelatedInstanceKeys(bvector<ContentSetItemExtendedData::RelatedInstanceKey> const& keys)
    {
    if (!GetExtendedData().GetRelatedInstanceKeys().empty())
        ContentItemBuilder::_SetRelatedInstanceKeys(bvector<ContentSetItemExtendedData::RelatedInstanceKey>());
    else
        ContentItemBuilder::_SetRelatedInstanceKeys(keys);
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentItemBuilder::BeforeAddValueStatus MergingContentItemBuilder::_OnBeforeAddValue(Utf8CP name)
    {
    if (GetMergedFieldNames().end() != GetMergedFieldNames().find(name))
        return BeforeAddValueStatus::Skip;
    return BeforeAddValueStatus::Continue;
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MergingContentItemBuilder::_AddValue(Utf8CP name, rapidjson::Value&& value, rapidjson::Value&& displayValue, ECPropertyCP prop)
    {
    if (!GetValues().HasMember(name))
        {
        ContentItemBuilder::_AddValue(name, std::move(value), std::move(displayValue), prop);
        return;
        }

    if (!GetValues().HasMember(name) || !GetDisplayValues().HasMember(name))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, Utf8PrintfString("Attempting to merge value, but target is not set. Property name: '%s'", name));

    RapidJsonValueR existingValue = GetValues()[name];
    RapidJsonValueR existingDisplayValue = GetDisplayValues()[name];

    if (prop && prop->GetIsNavigation())
        {
        if (value == existingValue)
            return;
        }
    else if (displayValue == existingDisplayValue)
        {
        // note: the same display value might be a result of different raw values - need to
        // somehow "merge" these raw values when we detect the same display value
        return;
        }

    existingValue.SetNull();
    static Utf8PrintfString const s_variesStr(CONTENTRECORD_MERGED_VALUE_FORMAT, CommonStrings::RULESENGINE_VARIES);
    existingDisplayValue.SetString(s_variesStr.c_str(), GetDisplayValues().GetAllocator());
    GetMergedFieldNames().insert(name);
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MergingContentItemBuilder::SetNestedContentValuesAsMerged(Utf8CP name, NestedContentValues& values)
    {
    values.values.clear();
    values.capturedCount = (size_t)0;
    GetMergedFieldNames().insert(name);
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MergingContentItemBuilder::_AddEmptyNestedContentValue(Utf8CP name)
    {
    if (BeforeAddValueStatus::Skip == _OnBeforeAddValue(name))
        return;

    m_nestedContentValueCounts[name] = 0;

    auto iter = GetNestedContentValues().find(name);
    if (GetNestedContentValues().end() != iter && !iter->second.values.empty())
        {
        // adding empty value on top of non-empty value means the number of related items is
        // different, which means we should set the value as merged
        SetNestedContentValuesAsMerged(name, iter->second);
        return;
        }

    ContentItemBuilder::_AddEmptyNestedContentValue(name);
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentItemBuilder& MergingContentItemBuilder::_AddNestedContentValue(Utf8CP name, ECInstanceKey const& key)
    {
    static NoopContentItemBuilder s_noop(GetSchemaManager(), GetValuesFormatter());
    if (BeforeAddValueStatus::Skip == _OnBeforeAddValue(name))
        return s_noop;

    auto iter = GetNestedContentValues().find(name);
    bool hasNestedContentValue = (GetNestedContentValues().end() != iter);
    Nullable<size_t> capturedValuesCount(hasNestedContentValue ? iter->second.capturedCount : nullptr);
    size_t currValuesCount = ++m_nestedContentValueCounts[name];

    if (capturedValuesCount.IsValid() && (capturedValuesCount.Value() < currValuesCount || capturedValuesCount.Value() > 1))
        {
        // set the whole nested content item as merged
        SetNestedContentValuesAsMerged(name, iter->second);
        m_nestedContentValueCounts[name] = 0;
        return s_noop;
        }

    if (capturedValuesCount.IsValid() && !iter->second.values.empty())
        {
        // merge current item with existing one
        ContentItemBuilder& nestedItemBuilder = *iter->second.values.back();
        nestedItemBuilder.SetPrimaryKey(key);
        return nestedItemBuilder;
        }

    // append a new nested content item
    return ContentItemBuilder::_AddNestedContentValue(name, key);
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentItemBuilder* MergingContentItemBuilder::_GetNestedContentValue(Utf8CP name, ECInstanceKey const& key) const
    {
    auto result = ContentItemBuilder::_GetNestedContentValue(name, key);
    if (nullptr != result)
        m_nestedContentValueCounts[name] = result->GetPrimaryKeys().size();
    return result;
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void MergingContentItemBuilder::_CaptureNestedContentValueCounts()
    {
    for (auto& entry : GetNestedContentValues())
        {
        if (GetMergedFieldNames().end() != GetMergedFieldNames().find(entry.first))
            {
            // nothing to do it the value is already set as merged
            DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Content, entry.second.values.empty(), Utf8PrintfString("Field is merged, but still has values. Field name: '%s'", entry.first.c_str()));
            continue;
            }

        auto valuesCountIter = m_nestedContentValueCounts.find(entry.first);
        size_t currValuesCount = (m_nestedContentValueCounts.end() != valuesCountIter) ? valuesCountIter->second : 0;

        if (entry.second.capturedCount.IsValid() && (entry.second.capturedCount != currValuesCount || currValuesCount > 1))
            {
            // set as merged if the number of related items is different or is larger
            // than 1 (we can't compare items individually as their order is undefined)
            SetNestedContentValuesAsMerged(entry.first.c_str(), entry.second);
            }
        }
    m_nestedContentValueCounts.clear();
    ContentItemBuilder::_CaptureNestedContentValueCounts();
    }
