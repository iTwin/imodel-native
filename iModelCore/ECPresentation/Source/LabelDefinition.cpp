/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/LabelDefinition.h>
#include <ECPresentation/NavNode.h>
#include "ValueHelpers.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document LabelDefinition::SimpleRawValue::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool LabelDefinition::SimpleRawValue::_Equals(LabelDefinition::RawValueBase const& other) const
    {
    SimpleRawValue const* simpleValue = other.AsSimpleValue();
    if (nullptr == simpleValue)
        return false;

    return m_value == simpleValue->m_value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document LabelDefinition::SimpleRawValue::_ToInternalJson(rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document doc(allocator);
    doc.CopyFrom(m_value, doc.GetAllocator());
    return doc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document LabelDefinition::CompositeRawValue::_AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool LabelDefinition::CompositeRawValue::_Equals(LabelDefinition::RawValueBase const& other) const
    {
    CompositeRawValue const* compositeValue = other.AsCompositeValue();
    if (nullptr == compositeValue)
        return false;

    if (m_separator != compositeValue->m_separator || m_values.size() != compositeValue->m_values.size())
        return false;

    for (size_t i = 0; i < m_values.size(); ++i)
        {
        if (*m_values[i] != *compositeValue->m_values[i])
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document LabelDefinition::CompositeRawValue::_ToInternalJson(rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document doc(allocator);
    doc.SetObject();
    doc.AddMember(NAVNODE_LABEL_DEFINITION_COMPOSITE_VALUE_Separator, rapidjson::Value(m_separator.c_str(), doc.GetAllocator()), doc.GetAllocator());
    rapidjson::Value values(rapidjson::kArrayType);
    for (LabelDefinitionCPtr value : m_values)
        values.PushBack(value->ToInternalJson(&doc.GetAllocator()), doc.GetAllocator());

    doc.AddMember(NAVNODE_LABEL_DEFINITION_COMPOSITE_VALUE_Values, values, doc.GetAllocator());
    return doc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<LabelDefinition::CompositeRawValue> LabelDefinition::CompositeRawValue::FromInternalJson(RapidJsonValueCR json)
    {
    Utf8CP separator = json[NAVNODE_LABEL_DEFINITION_COMPOSITE_VALUE_Separator].GetString();
    bvector<LabelDefinitionCPtr> values;
    RapidJsonValueCR valuesJson = json[NAVNODE_LABEL_DEFINITION_COMPOSITE_VALUE_Values];
    for (RapidJsonValueCR value : valuesJson.GetArray())
        values.push_back(LabelDefinition::FromInternalJson(value));

    return std::make_unique<CompositeRawValue>(separator, values);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
LabelDefinition::LabelDefinition(LabelDefinitionCR other)
    : m_displayValue(other.m_displayValue), m_typeName(other.m_typeName), m_rawValue(nullptr)
    {
    if (nullptr != other.m_rawValue)
        m_rawValue = other.m_rawValue->Clone();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool LabelDefinition::operator==(LabelDefinitionCR other) const
    {
    if (m_displayValue != other.m_displayValue)
        return false;
    if (m_typeName != other.m_typeName)
        return false;
    if ((nullptr == m_rawValue && nullptr != other.m_rawValue) || (nullptr != m_rawValue && nullptr == other.m_rawValue))
        return false;
    return (nullptr == m_rawValue && nullptr == other.m_rawValue) || *m_rawValue == *other.m_rawValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool LabelDefinition::operator!=(LabelDefinitionCR other) const
    {
    return !(*this == other);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
LabelDefinition const& LabelDefinition::SetStringValue(Utf8CP value, Utf8CP displayValue)
    {
    m_displayValue = nullptr != displayValue ? displayValue : value;
    m_typeName = "string";
    m_rawValue = std::make_unique<SimpleRawValue>(value);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
LabelDefinition const& LabelDefinition::SetECValue(ECValueCR value, Utf8CP displayValue)
    {
    m_displayValue = nullptr != displayValue ? displayValue : value.ToString().c_str();
    m_typeName = ValueHelpers::GetECValueTypeName(value);
    m_rawValue = std::make_unique<SimpleRawValue>(ValueHelpers::GetJsonFromECValue(value, nullptr));
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
LabelDefinition const& LabelDefinition::SetECPropertyValue(ECPropertyCR ecProperty, DbValue const& dbValue, Utf8CP displayValue)
    {
    if (ecProperty.GetIsPrimitive())
        {
        ECValue value = ValueHelpers::GetECValueFromSqlValue(ecProperty.GetAsPrimitiveProperty()->GetType(), dbValue);
        return SetECValue(value, displayValue);
        }

    return SetStringValue(displayValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
LabelDefinition const& LabelDefinition::SetJsonValue(Utf8CP displayValue, Utf8CP typeName, RapidJsonValueCR value)
    {
    m_displayValue = displayValue;
    m_typeName = typeName;
    m_rawValue = std::make_unique<SimpleRawValue>(value);
    return *this; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
LabelDefinitionCR LabelDefinition::SetCompositeValue(Utf8CP displayValue, std::unique_ptr<CompositeRawValue> value)
    {
    m_displayValue = displayValue;
    m_typeName = "composite";
    m_rawValue = std::move(value);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document LabelDefinition::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document LabelDefinition::ToInternalJson(rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document doc(allocator);
    doc.SetObject();
    doc.AddMember(NAVNODE_LABEL_DEFINITION_DisplayValue, rapidjson::Value(m_displayValue.c_str(), doc.GetAllocator()), doc.GetAllocator());
    doc.AddMember(NAVNODE_LABEL_DEFINITION_TypeName, rapidjson::Value(m_typeName.c_str(), doc.GetAllocator()), doc.GetAllocator());
    if (nullptr != m_rawValue)
        doc.AddMember(NAVNODE_LABEL_DEFINITION_RawValue, m_rawValue->ToInternalJson(&doc.GetAllocator()), doc.GetAllocator());
    return doc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String LabelDefinition::ToJsonString() const
    {
    rapidjson::MemoryPoolAllocator<> allocator(1024U);
    return BeRapidJsonUtilities::ToString(ToInternalJson(&allocator));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
LabelDefinitionPtr LabelDefinition::FromInternalJson(RapidJsonValueCR json)
    {
    Utf8CP displayValue = json[NAVNODE_LABEL_DEFINITION_DisplayValue].GetString();
    Utf8CP typeName = json[NAVNODE_LABEL_DEFINITION_TypeName].GetString();
    RapidJsonValueCR rawValueJson = json[NAVNODE_LABEL_DEFINITION_RawValue];
    std::unique_ptr<RawValueBase> rawValue = nullptr;
    if (rawValueJson.IsObject() && rawValueJson.HasMember(NAVNODE_LABEL_DEFINITION_COMPOSITE_VALUE_Separator))
        rawValue = LabelDefinition::CompositeRawValue::FromInternalJson(rawValueJson);
    else
        rawValue = LabelDefinition::SimpleRawValue::FromInternalJson(rawValueJson);

    return LabelDefinition::Create(displayValue, typeName, std::move(rawValue));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
LabelDefinitionPtr LabelDefinition::FromString(Utf8CP value)
    {
    if (nullptr == value || 0 == *value)
        return LabelDefinition::Create();

    rapidjson::MemoryPoolAllocator<> allocator(1024U);
    rapidjson::Document json(&allocator);
    json.Parse(value);
    if (json.IsObject())
        return LabelDefinition::FromInternalJson(json);
    return LabelDefinition::Create(value);
    }