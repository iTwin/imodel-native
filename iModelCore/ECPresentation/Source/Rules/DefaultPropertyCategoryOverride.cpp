/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/DefaultPropertyCategoryOverride.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DefaultPropertyCategoryOverride::DefaultPropertyCategoryOverride(DefaultPropertyCategoryOverride const& other)
    : PresentationRule(other), m_specification(nullptr)
    {
    if (other.m_specification != nullptr)
        m_specification = new PropertyCategorySpecification(*other.m_specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DefaultPropertyCategoryOverride::DefaultPropertyCategoryOverride(DefaultPropertyCategoryOverride&& other)
    : PresentationRule(std::move(other)), m_specification(other.m_specification)
    {
    other.m_specification = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DefaultPropertyCategoryOverride::~DefaultPropertyCategoryOverride()
    {
    DELETE_AND_CLEAR(m_specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP DefaultPropertyCategoryOverride::_GetJsonElementType() const {return DEFAULT_PROPERTY_CATEGORY_OVERRIDE_JSON_TYPE;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool DefaultPropertyCategoryOverride::_ReadJson(JsonValueCR json)
    {
    if (!PresentationRule::_ReadJson(json))
        return false;

    if (CommonToolsInternal::CheckRuleIssue(!json[DEFAULT_PROPERTY_CATEGORY_OVERRIDE_JSON_ATTRIBUTE_SPECIFICATION].isObject(), _GetJsonElementType(), DEFAULT_PROPERTY_CATEGORY_OVERRIDE_JSON_ATTRIBUTE_SPECIFICATION, json[DEFAULT_PROPERTY_CATEGORY_OVERRIDE_JSON_ATTRIBUTE_SPECIFICATION], "object"))
        return false;

    auto spec = CommonToolsInternal::LoadRuleFromJson<PropertyCategorySpecification>(json[DEFAULT_PROPERTY_CATEGORY_OVERRIDE_JSON_ATTRIBUTE_SPECIFICATION]);
    if (CommonToolsInternal::CheckRuleIssue(!spec, _GetJsonElementType(), DEFAULT_PROPERTY_CATEGORY_OVERRIDE_JSON_ATTRIBUTE_SPECIFICATION, json[DEFAULT_PROPERTY_CATEGORY_OVERRIDE_JSON_ATTRIBUTE_SPECIFICATION], "object"))
        return false;

    m_specification = spec;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultPropertyCategoryOverride::_WriteJson(JsonValueR json) const
    {
    PresentationRule::_WriteJson(json);
    json[COMMON_JSON_ATTRIBUTE_RULETYPE] = _GetJsonElementType();
    json[DEFAULT_PROPERTY_CATEGORY_OVERRIDE_JSON_ATTRIBUTE_SPECIFICATION] = m_specification->WriteJson();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 DefaultPropertyCategoryOverride::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (m_specification)
        {
        Utf8StringCR specHash = m_specification->GetHash();
        ADD_STR_VALUE_TO_HASH(md5, DEFAULT_PROPERTY_CATEGORY_OVERRIDE_JSON_ATTRIBUTE_SPECIFICATION, specHash);
        }
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool DefaultPropertyCategoryOverride::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!PresentationRule::_ShallowEqual(other))
        return false;

    DefaultPropertyCategoryOverride const* otherRule = dynamic_cast<DefaultPropertyCategoryOverride const*>(&other);
    if (nullptr == otherRule)
        return false;

    return true;
    }
