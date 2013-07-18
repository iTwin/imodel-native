/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/StyleOverride.cpp $
|
|   $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StyleOverride::StyleOverride ()
    : PresentationRule (), m_foreColor (L""), m_backColor (L""), m_fontStyle (L"")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StyleOverride::StyleOverride (WStringCR condition, int priority, WStringCR foreColor, WStringCR backColor, WStringCR fontStyle)
    : PresentationRule (condition, priority, false), 
        m_foreColor (foreColor), m_backColor (backColor), m_fontStyle (fontStyle)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP StyleOverride::_GetXmlElementName ()
    {
    return STYLE_OVERRIDE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool StyleOverride::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_foreColor, STYLE_OVERRIDE_XML_ATTRIBUTE_FORECOLOR))
        m_foreColor = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_backColor, STYLE_OVERRIDE_XML_ATTRIBUTE_BACKCOLOR))
        m_backColor = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_fontStyle, STYLE_OVERRIDE_XML_ATTRIBUTE_FONTSTYLE))
        m_fontStyle = L"";

    return PresentationRule::_ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void StyleOverride::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeStringValue (STYLE_OVERRIDE_XML_ATTRIBUTE_FORECOLOR, m_foreColor.c_str ());
    xmlNode->AddAttributeStringValue (STYLE_OVERRIDE_XML_ATTRIBUTE_BACKCOLOR, m_backColor.c_str ());
    xmlNode->AddAttributeStringValue (STYLE_OVERRIDE_XML_ATTRIBUTE_FONTSTYLE, m_fontStyle.c_str ());

    PresentationRule::_WriteXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR StyleOverride::GetForeColor (void) const { return m_foreColor; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR StyleOverride::GetBackColor (void) const { return m_backColor; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR StyleOverride::GetFontStyle (void) const { return m_fontStyle; }
