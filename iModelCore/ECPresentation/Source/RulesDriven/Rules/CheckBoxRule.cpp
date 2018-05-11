/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/CheckBoxRule.cpp $
|
|   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CheckBoxRule::CheckBoxRule ()
    : m_propertyName (""), m_useInversedPropertyValue (false), m_defaultValue (false), m_isEnabled ("")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   11/2012
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
* @bsimethod                                    Andrius.Zonys                   11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP CheckBoxRule::_GetXmlElementName () const
    {
    return CHECKBOX_RULE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool CheckBoxRule::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue  (m_propertyName, COMMON_XML_ATTRIBUTE_PROPERTYNAME))
        m_propertyName = "";

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_useInversedPropertyValue, CHECKBOX_RULE_XML_ATTRIBUTE_USEINVERSEDPROPERTYVALUE))
        m_useInversedPropertyValue = false;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_defaultValue, CHECKBOX_RULE_XML_ATTRIBUTE_DEFAULTVALUE))
        m_defaultValue = false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_isEnabled, CHECKBOX_RULE_XML_ATTRIBUTE_ISENABLED))
        m_isEnabled = "";

    return ConditionalCustomizationRule::_ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool CheckBoxRule::_ReadJson(JsonValueCR json)
    {
    m_propertyName = json[CHECKBOX_RULE_JSON_ATTRIBUTE_PROPERTYNAME].asCString("");
    m_useInversedPropertyValue = json[CHECKBOX_RULE_JSON_ATTRIBUTE_USEINVERSEDPROPERTYVALUE].asBool(false);
    m_isEnabled = json[CHECKBOX_RULE_JSON_ATTRIBUTE_ISENABLED].asCString("");
    m_defaultValue = json[CHECKBOX_RULE_JSON_ATTRIBUTE_DEFAULTVALUE].asBool(false);

    return ConditionalCustomizationRule::_ReadJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void CheckBoxRule::_WriteXml (BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_PROPERTYNAME, m_propertyName.c_str ());
    xmlNode->AddAttributeBooleanValue (CHECKBOX_RULE_XML_ATTRIBUTE_USEINVERSEDPROPERTYVALUE, m_useInversedPropertyValue);
    xmlNode->AddAttributeBooleanValue (CHECKBOX_RULE_XML_ATTRIBUTE_DEFAULTVALUE, m_defaultValue);
    xmlNode->AddAttributeStringValue (CHECKBOX_RULE_XML_ATTRIBUTE_ISENABLED, m_isEnabled.c_str());
    ConditionalCustomizationRule::_WriteXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR CheckBoxRule::GetPropertyName (void) const { return m_propertyName; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool CheckBoxRule::GetUseInversedPropertyValue (void) const { return m_useInversedPropertyValue; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool CheckBoxRule::GetDefaultValue (void) const { return m_defaultValue; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR CheckBoxRule::GetIsEnabled(void) const { return m_isEnabled; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void CheckBoxRule::_Accept(CustomizationRuleVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 CheckBoxRule::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = ConditionalCustomizationRule::_ComputeHash(parentHash);
    md5.Add(m_propertyName.c_str(), m_propertyName.size());
    md5.Add(&m_useInversedPropertyValue, sizeof(m_useInversedPropertyValue));
    md5.Add(&m_defaultValue, sizeof(m_defaultValue));
    md5.Add(m_isEnabled.c_str(), m_isEnabled.size());
    return md5;
    }
