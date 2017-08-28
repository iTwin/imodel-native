/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/StyleOverride.cpp $
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StyleOverride::StyleOverride ()
    : m_foreColor (""), m_backColor (""), m_fontStyle ("")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StyleOverride::StyleOverride (Utf8StringCR condition, int priority, Utf8StringCR foreColor, Utf8StringCR backColor, Utf8StringCR fontStyle)
    : CustomizationRule(condition, priority, false),
        m_foreColor (foreColor), m_backColor (backColor), m_fontStyle (fontStyle)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP StyleOverride::_GetXmlElementName () const
    {
    return STYLE_OVERRIDE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool StyleOverride::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_foreColor, STYLE_OVERRIDE_XML_ATTRIBUTE_FORECOLOR))
        m_foreColor = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_backColor, STYLE_OVERRIDE_XML_ATTRIBUTE_BACKCOLOR))
        m_backColor = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_fontStyle, STYLE_OVERRIDE_XML_ATTRIBUTE_FONTSTYLE))
        m_fontStyle = "";

    return PresentationRule::_ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void StyleOverride::_WriteXml (BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeStringValue (STYLE_OVERRIDE_XML_ATTRIBUTE_FORECOLOR, m_foreColor.c_str ());
    xmlNode->AddAttributeStringValue (STYLE_OVERRIDE_XML_ATTRIBUTE_BACKCOLOR, m_backColor.c_str ());
    xmlNode->AddAttributeStringValue (STYLE_OVERRIDE_XML_ATTRIBUTE_FONTSTYLE, m_fontStyle.c_str ());

    PresentationRule::_WriteXml (xmlNode);
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