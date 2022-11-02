/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CheckBoxRule::CheckBoxRule()
    : m_useInversedPropertyValue(false), m_defaultValue(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CheckBoxRule::CheckBoxRule
(
Utf8StringCR condition,
int       priority,
bool      onlyIfNotHandled,
Utf8StringCR propertyName,
bool      useInversedPropertyValue,
bool      defaultValue,
Utf8StringCR isEnabled
) : ConditionalCustomizationRule (condition, priority, onlyIfNotHandled), m_propertyName (propertyName),
    m_useInversedPropertyValue (useInversedPropertyValue), m_defaultValue (defaultValue), m_isEnabled (isEnabled)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP CheckBoxRule::_GetXmlElementName () const
    {
    return CHECKBOX_RULE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CheckBoxRule::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!ConditionalCustomizationRule::_ReadXml(xmlNode))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue  (m_propertyName, COMMON_XML_ATTRIBUTE_PROPERTYNAME))
        m_propertyName = "";

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_useInversedPropertyValue, CHECKBOX_RULE_XML_ATTRIBUTE_USEINVERSEDPROPERTYVALUE))
        m_useInversedPropertyValue = false;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_defaultValue, CHECKBOX_RULE_XML_ATTRIBUTE_DEFAULTVALUE))
        m_defaultValue = false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_isEnabled, CHECKBOX_RULE_XML_ATTRIBUTE_ISENABLED))
        m_isEnabled = "";

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CheckBoxRule::_WriteXml (BeXmlNodeP xmlNode) const
    {
    ConditionalCustomizationRule::_WriteXml (xmlNode);
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_PROPERTYNAME, m_propertyName.c_str ());
    xmlNode->AddAttributeBooleanValue (CHECKBOX_RULE_XML_ATTRIBUTE_USEINVERSEDPROPERTYVALUE, m_useInversedPropertyValue);
    xmlNode->AddAttributeBooleanValue (CHECKBOX_RULE_XML_ATTRIBUTE_DEFAULTVALUE, m_defaultValue);
    xmlNode->AddAttributeStringValue (CHECKBOX_RULE_XML_ATTRIBUTE_ISENABLED, m_isEnabled.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP CheckBoxRule::_GetJsonElementType() const
    {
    return CHECKBOX_RULE_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetIsEnabledExpression(JsonValueCR json)
    {
    if (json.isBool())
        return json.asBool() ? "true" : "false";
    return json.asCString("");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Json::Value GetIsEnabledJsonValue(Utf8StringCR expr)
    {
    if (expr.EqualsI("true"))
        return Json::Value(true);
    if (expr.EqualsI("false"))
        return Json::Value(false);
    return Json::Value(expr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CheckBoxRule::_ReadJson(JsonValueCR json)
    {
    if (!ConditionalCustomizationRule::_ReadJson(json))
        return false;

    m_propertyName = json[CHECKBOX_RULE_JSON_ATTRIBUTE_PROPERTYNAME].asCString("");
    m_useInversedPropertyValue = json[CHECKBOX_RULE_JSON_ATTRIBUTE_USEINVERSEDPROPERTYVALUE].asBool(false);
    m_isEnabled = GetIsEnabledExpression(json[CHECKBOX_RULE_JSON_ATTRIBUTE_ISENABLED]);
    m_defaultValue = json[CHECKBOX_RULE_JSON_ATTRIBUTE_DEFAULTVALUE].asBool(false);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CheckBoxRule::_WriteJson(JsonValueR json) const
    {
    ConditionalCustomizationRule::_WriteJson(json);
    if (!m_propertyName.empty())
        json[CHECKBOX_RULE_JSON_ATTRIBUTE_PROPERTYNAME] = m_propertyName;
    if (m_useInversedPropertyValue)
        json[CHECKBOX_RULE_JSON_ATTRIBUTE_USEINVERSEDPROPERTYVALUE] = m_useInversedPropertyValue;
    if (!m_isEnabled.empty())
        json[CHECKBOX_RULE_JSON_ATTRIBUTE_ISENABLED] = GetIsEnabledJsonValue(m_isEnabled);
    if (m_defaultValue)
        json[CHECKBOX_RULE_JSON_ATTRIBUTE_DEFAULTVALUE] = m_defaultValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR CheckBoxRule::GetPropertyName (void) const { return m_propertyName; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CheckBoxRule::GetUseInversedPropertyValue (void) const { return m_useInversedPropertyValue; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CheckBoxRule::GetDefaultValue (void) const { return m_defaultValue; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR CheckBoxRule::GetIsEnabled(void) const { return m_isEnabled; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CheckBoxRule::_Accept(CustomizationRuleVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 CheckBoxRule::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_propertyName.empty())
        ADD_STR_VALUE_TO_HASH(md5, CHECKBOX_RULE_JSON_ATTRIBUTE_PROPERTYNAME, m_propertyName);
    if (m_useInversedPropertyValue)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, CHECKBOX_RULE_JSON_ATTRIBUTE_USEINVERSEDPROPERTYVALUE, m_useInversedPropertyValue);
    if (m_defaultValue)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, CHECKBOX_RULE_JSON_ATTRIBUTE_DEFAULTVALUE, m_defaultValue);
    if (!m_isEnabled.empty())
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, CHECKBOX_RULE_JSON_ATTRIBUTE_ISENABLED, m_isEnabled);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CheckBoxRule::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!ConditionalCustomizationRule::_ShallowEqual(other))
        return false;

    CheckBoxRule const* otherRule = dynamic_cast<CheckBoxRule const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_propertyName == otherRule->m_propertyName
        && m_useInversedPropertyValue == otherRule->m_useInversedPropertyValue
        && m_defaultValue == otherRule->m_defaultValue
        && m_isEnabled == otherRule->m_isEnabled;
    }
