/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/CheckBoxRule.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP CheckBoxRule::_GetXmlElementName ()
    {
    return CHECKBOX_RULE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool CheckBoxRule::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue  (m_propertyName, COMMON_XML_ATTRIBUTE_PROPERTYNAME))
        m_propertyName = L"";

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_useInversedPropertyValue, CHECKBOX_RULE_XML_ATTRIBUTE_USEINVERSEDPROPERTYVALUE))
        m_useInversedPropertyValue = false;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_defaultValue, CHECKBOX_RULE_XML_ATTRIBUTE_DEFAULTVALUE))
        m_defaultValue = false;

    return PresentationRule::_ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void CheckBoxRule::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_PROPERTYNAME, m_propertyName.c_str ());
    xmlNode->AddAttributeBooleanValue (CHECKBOX_RULE_XML_ATTRIBUTE_USEINVERSEDPROPERTYVALUE, m_useInversedPropertyValue);
    xmlNode->AddAttributeBooleanValue (CHECKBOX_RULE_XML_ATTRIBUTE_DEFAULTVALUE, m_defaultValue);

    PresentationRule::_WriteXml (xmlNode);
    }