/*--------------------------------------------------------------------------------------+
|
|   Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
LabelOverride::LabelOverride() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
LabelOverride::LabelOverride (Utf8StringCR condition, int priority, Utf8StringCR label, Utf8StringCR description)
    : ConditionalCustomizationRule (condition, priority, false), m_label (label), m_description (description)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP LabelOverride::_GetXmlElementName () const
    {
    return LABEL_OVERRIDE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool LabelOverride::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!ConditionalCustomizationRule::_ReadXml(xmlNode))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_label, LABEL_OVERRIDE_XML_ATTRIBUTE_LABEL))
        m_label = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_description, LABEL_OVERRIDE_XML_ATTRIBUTE_DESCRIPTION))
        m_description = "";

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void LabelOverride::_WriteXml (BeXmlNodeP xmlNode) const
    {
    ConditionalCustomizationRule::_WriteXml (xmlNode);
    xmlNode->AddAttributeStringValue (LABEL_OVERRIDE_XML_ATTRIBUTE_LABEL, m_label.c_str ());
    xmlNode->AddAttributeStringValue (LABEL_OVERRIDE_XML_ATTRIBUTE_DESCRIPTION, m_description.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP LabelOverride::_GetJsonElementType() const
    {
    return LABEL_OVERRIDE_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool LabelOverride::_ReadJson(JsonValueCR json)
    {
    if (!ConditionalCustomizationRule::_ReadJson(json))
        return false;
    m_label = json[LABEL_OVERRIDE_JSON_ATTRIBUTE_LABEL].asCString("");
    m_description = json[LABEL_OVERRIDE_JSON_ATTRIBUTE_DESCRIPTION].asCString("");
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void LabelOverride::_WriteJson(JsonValueR json) const
    {
    ConditionalCustomizationRule::_WriteJson(json);
    if (!m_label.empty())
        json[LABEL_OVERRIDE_JSON_ATTRIBUTE_LABEL] = m_label;
    if (!m_description.empty())
        json[LABEL_OVERRIDE_JSON_ATTRIBUTE_DESCRIPTION] = m_description;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR LabelOverride::GetLabel (void) const { return m_label; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Tom.Amon                        03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void LabelOverride::SetLabel (Utf8String value) { m_label = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR LabelOverride::GetDescription (void) const { return m_description; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Tom.Amon                        03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void LabelOverride::SetDescription (Utf8String value) { m_description = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void LabelOverride::_Accept(CustomizationRuleVisitor& visitor) const { visitor._Visit(*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 LabelOverride::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = ConditionalCustomizationRule::_ComputeHash(parentHash);
    md5.Add(m_label.c_str(), m_label.size());
    md5.Add(m_description.c_str(), m_description.size());
    return md5;
    }
