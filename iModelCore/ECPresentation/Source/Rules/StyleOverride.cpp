/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "PresentationRuleJsonConstants.h"
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
Utf8CP StyleOverride::_GetJsonElementType() const
    {
    return STYLE_OVERRIDE_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool StyleOverride::_ReadJson(BeJsConst json)
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
void StyleOverride::_WriteJson(BeJsValue json) const
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
