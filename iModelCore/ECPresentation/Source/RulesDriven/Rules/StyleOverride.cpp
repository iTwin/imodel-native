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
StyleOverride::StyleOverride() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StyleOverride::StyleOverride (Utf8StringCR condition, int priority, Utf8StringCR foreColor, Utf8StringCR backColor, Utf8StringCR fontStyle)
    : ConditionalCustomizationRule(condition, priority, false), m_foreColor (foreColor), m_backColor (backColor), m_fontStyle (fontStyle)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP StyleOverride::_GetXmlElementName () const
    {
    return STYLE_OVERRIDE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool StyleOverride::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!ConditionalCustomizationRule::_ReadXml(xmlNode))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_foreColor, STYLE_OVERRIDE_XML_ATTRIBUTE_FORECOLOR))
        m_foreColor = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_backColor, STYLE_OVERRIDE_XML_ATTRIBUTE_BACKCOLOR))
        m_backColor = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_fontStyle, STYLE_OVERRIDE_XML_ATTRIBUTE_FONTSTYLE))
        m_fontStyle = "";

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void StyleOverride::_WriteXml (BeXmlNodeP xmlNode) const
    {
    ConditionalCustomizationRule::_WriteXml (xmlNode);
    xmlNode->AddAttributeStringValue (STYLE_OVERRIDE_XML_ATTRIBUTE_FORECOLOR, m_foreColor.c_str ());
    xmlNode->AddAttributeStringValue (STYLE_OVERRIDE_XML_ATTRIBUTE_BACKCOLOR, m_backColor.c_str ());
    xmlNode->AddAttributeStringValue (STYLE_OVERRIDE_XML_ATTRIBUTE_FONTSTYLE, m_fontStyle.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP StyleOverride::_GetJsonElementType() const
    {
    return STYLE_OVERRIDE_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool StyleOverride::_ReadJson(JsonValueCR json)
    {
    if (!ConditionalCustomizationRule::_ReadJson(json))
        return false;

    m_foreColor = json[STYLE_OVERRIDE_JSON_ATTRIBUTE_FORECOLOR].asCString("");
    m_backColor = json[STYLE_OVERRIDE_JSON_ATTRIBUTE_BACKCOLOR].asCString("");
    m_fontStyle = json[STYLE_OVERRIDE_JSON_ATTRIBUTE_FONTSTYLE].asCString("");
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void StyleOverride::_WriteJson(JsonValueR json) const
    {
    ConditionalCustomizationRule::_WriteJson(json);
    if (!m_foreColor.empty())
        json[STYLE_OVERRIDE_JSON_ATTRIBUTE_FORECOLOR] = m_foreColor;
    if (!m_backColor.empty())
        json[STYLE_OVERRIDE_JSON_ATTRIBUTE_BACKCOLOR] = m_backColor;
    if (!m_fontStyle.empty())
        json[STYLE_OVERRIDE_JSON_ATTRIBUTE_FONTSTYLE] = m_fontStyle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR StyleOverride::GetForeColor (void) const { return m_foreColor; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Tom.Amon                        03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void StyleOverride::SetForeColor (Utf8String value) { m_foreColor = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR StyleOverride::GetBackColor (void) const { return m_backColor; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR StyleOverride::GetFontStyle (void) const { return m_fontStyle; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void StyleOverride::_Accept(CustomizationRuleVisitor& visitor) const { visitor._Visit(*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 StyleOverride::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = ConditionalCustomizationRule::_ComputeHash(parentHash);
    md5.Add(m_foreColor.c_str(), m_foreColor.size());
    md5.Add(m_backColor.c_str(), m_backColor.size());
    md5.Add(m_fontStyle.c_str(), m_fontStyle.size());
    return md5;
    }
