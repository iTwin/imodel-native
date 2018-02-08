/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/CustomizationRule.cpp $
|
|   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ConditionalCustomizationRule::GetCondition() const { return m_condition; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ConditionalCustomizationRule::SetCondition(Utf8String value) { m_condition = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConditionalCustomizationRule::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_condition, PRESENTATION_RULE_XML_ATTRIBUTE_CONDITION))
        m_condition = "";

    return CustomizationRule::_ReadXml(xmlNode);;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void ConditionalCustomizationRule::_WriteXml(BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeStringValue(PRESENTATION_RULE_XML_ATTRIBUTE_CONDITION, m_condition.c_str());
    CustomizationRule::_WriteXml(xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ConditionalCustomizationRule::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = CustomizationRule::_ComputeHash(parentHash);
    md5.Add(m_condition.c_str(), m_condition.size());
    return md5;
    }