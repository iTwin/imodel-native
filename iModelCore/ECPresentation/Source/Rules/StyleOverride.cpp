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
StyleOverride::StyleOverride() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StyleOverride::StyleOverride (Utf8StringCR condition, int priority, Utf8StringCR foreColor, Utf8StringCR backColor, Utf8StringCR fontStyle)
    : ConditionalCustomizationRule(condition, priority, false), m_foreColor (foreColor), m_backColor (backColor), m_fontStyle (fontStyle)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP StyleOverride::_GetXmlElementName () const
    {
    return STYLE_OVERRIDE_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void StyleOverride::_WriteXml (BeXmlNodeP xmlNode) const
    {
    ConditionalCustomizationRule::_WriteXml (xmlNode);
    xmlNode->AddAttributeStringValue (STYLE_OVERRIDE_XML_ATTRIBUTE_FORECOLOR, m_foreColor.c_str ());
    xmlNode->AddAttributeStringValue (STYLE_OVERRIDE_XML_ATTRIBUTE_BACKCOLOR, m_backColor.c_str ());
    xmlNode->AddAttributeStringValue (STYLE_OVERRIDE_XML_ATTRIBUTE_FONTSTYLE, m_fontStyle.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP StyleOverride::_GetJsonElementType() const
    {
    return STYLE_OVERRIDE_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR StyleOverride::GetForeColor (void) const { return m_foreColor; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void StyleOverride::SetForeColor (Utf8String value) { m_foreColor = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR StyleOverride::GetBackColor (void) const { return m_backColor; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR StyleOverride::GetFontStyle (void) const { return m_fontStyle; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void StyleOverride::_Accept(CustomizationRuleVisitor& visitor) const { visitor._Visit(*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 StyleOverride::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_foreColor.empty())
        ADD_STR_VALUE_TO_HASH(md5, STYLE_OVERRIDE_JSON_ATTRIBUTE_FORECOLOR, m_foreColor);
    if (!m_backColor.empty())
        ADD_STR_VALUE_TO_HASH(md5, STYLE_OVERRIDE_JSON_ATTRIBUTE_BACKCOLOR, m_backColor);
    if (!m_fontStyle.empty())
        ADD_STR_VALUE_TO_HASH(md5, STYLE_OVERRIDE_JSON_ATTRIBUTE_FONTSTYLE, m_fontStyle);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool StyleOverride::_ShallowEqual(PresentationKeyCR other) const
    {
    if (!ConditionalCustomizationRule::_ShallowEqual(other))
        return false;

    StyleOverride const* otherRule = dynamic_cast<StyleOverride const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_foreColor == otherRule->m_foreColor
        && m_backColor == otherRule->m_backColor
        && m_fontStyle == otherRule->m_fontStyle;
    }
