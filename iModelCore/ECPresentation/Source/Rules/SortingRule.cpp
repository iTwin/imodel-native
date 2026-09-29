/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SortingRule::SortingRule()
    : m_sortAscending(true), m_doNotSort(false), m_isPolymorphic(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SortingRule::SortingRule(Utf8StringCR condition, int priority, Utf8StringCR schemaName, Utf8StringCR className, Utf8StringCR propertyName, bool sortAscending, bool doNotSort, bool isPolymorphic)
    : ConditionalCustomizationRule(condition, priority, false),
    m_schemaName(schemaName), m_className(className), m_propertyName(propertyName), m_sortAscending(sortAscending), m_doNotSort(doNotSort), m_isPolymorphic(isPolymorphic)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP SortingRule::_GetJsonElementType() const
    {
    // note: SortingRule handles JSON element type itself
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SortingRule::_ReadJson(BeJsConst json)
    {
    if (!ConditionalCustomizationRule::_ReadJson(json))
        return false;

    // required:
    Utf8CP identifiedRuleType = nullptr;
    Utf8CP ruleType = json[COMMON_JSON_ATTRIBUTE_RULETYPE].asCString("");
    if (0 == strcmp(ruleType, SORTING_RULE_PROPERTYSORTING_JSON_TYPE))
        {
        m_propertyName = json[COMMON_JSON_ATTRIBUTE_PROPERTYNAME].asCString("");
        m_sortAscending = json[SORTING_RULE_JSON_ATTRIBUTE_SORTASCENDING].asBool(true);
        identifiedRuleType = SORTING_RULE_PROPERTYSORTING_JSON_TYPE;
        }
    else if (0 == strcmp(ruleType, SORTING_RULE_DISABLEDSORTING_JSON_TYPE))
        {
        m_doNotSort = true;
        identifiedRuleType = SORTING_RULE_DISABLEDSORTING_JSON_TYPE;
        }

    if (nullptr == identifiedRuleType)
        return false;

    // optional:
    if (json.isMember(COMMON_JSON_ATTRIBUTE_CLASS))
        CommonToolsInternal::ParseSchemaAndClassName(m_schemaName, m_className, json[COMMON_JSON_ATTRIBUTE_CLASS], Utf8PrintfString("%s.%s", identifiedRuleType, COMMON_JSON_ATTRIBUTE_CLASS).c_str());
    m_isPolymorphic = json[COMMON_JSON_ATTRIBUTE_ISPOLYMORPHIC].asBool(false);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SortingRule::_WriteJson(BeJsValue json) const
    {
    ConditionalCustomizationRule::_WriteJson(json);

    if (!m_schemaName.empty() && !m_className.empty())
        CommonToolsInternal::WriteSchemaAndClassNameToJson(json[COMMON_JSON_ATTRIBUTE_CLASS], m_schemaName, m_className);
    if (m_isPolymorphic)
        json[COMMON_JSON_ATTRIBUTE_ISPOLYMORPHIC] = m_isPolymorphic;

    if (m_doNotSort)
        {
        json[COMMON_JSON_ATTRIBUTE_RULETYPE] = SORTING_RULE_DISABLEDSORTING_JSON_TYPE;
        }
    else
        {
        json[COMMON_JSON_ATTRIBUTE_RULETYPE] = SORTING_RULE_PROPERTYSORTING_JSON_TYPE;
        json[COMMON_JSON_ATTRIBUTE_PROPERTYNAME] = m_propertyName;
        if (!m_sortAscending)
            json[SORTING_RULE_JSON_ATTRIBUTE_SORTASCENDING] = m_sortAscending;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SortingRule::GetSchemaName(void) const { return m_schemaName; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SortingRule::GetClassName(void) const { return m_className; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SortingRule::GetPropertyName(void) const { return m_propertyName; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SortingRule::GetSortAscending(void) const { return m_sortAscending; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SortingRule::GetDoNotSort(void) const { return m_doNotSort; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SortingRule::GetIsPolymorphic(void) const { return m_isPolymorphic; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SortingRule::_Accept(CustomizationRuleVisitor& visitor) const { visitor._Visit(*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 SortingRule::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_schemaName.empty())
        ADD_STR_VALUE_TO_HASH(md5, SCHEMA_CLASS_SPECIFICATION_SCHEMANAME, m_schemaName);
    if (!m_className.empty())
        ADD_STR_VALUE_TO_HASH(md5, SINGLE_SCHEMA_CLASS_SPECIFICATION_CLASSNAME, m_className);
    if (!m_propertyName.empty())
        ADD_STR_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_PROPERTYNAME, m_propertyName);
    if (!m_sortAscending)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, SORTING_RULE_JSON_ATTRIBUTE_SORTASCENDING, m_sortAscending);
    if (m_doNotSort)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, SORTING_RULE_DISABLEDSORTING_JSON_TYPE, m_doNotSort);
    if (m_isPolymorphic)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, COMMON_JSON_ATTRIBUTE_ISPOLYMORPHIC, m_isPolymorphic);
    return md5;
    }
