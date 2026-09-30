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
LabelOverride::LabelOverride() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LabelOverride::LabelOverride (Utf8StringCR condition, int priority, Utf8StringCR label, Utf8StringCR description)
    : ConditionalCustomizationRule (condition, priority, false), m_label (label), m_description (description)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP LabelOverride::_GetJsonElementType() const
    {
    return LABEL_OVERRIDE_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool LabelOverride::_ReadJson(BeJsConst json)
    {
    if (!ConditionalCustomizationRule::_ReadJson(json))
        return false;

    m_label = json[LABEL_OVERRIDE_JSON_ATTRIBUTE_LABEL].asCString("");
    m_description = json[LABEL_OVERRIDE_JSON_ATTRIBUTE_DESCRIPTION].asCString("");
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LabelOverride::_WriteJson(BeJsValue json) const
    {
    ConditionalCustomizationRule::_WriteJson(json);
    if (!m_label.empty())
        json[LABEL_OVERRIDE_JSON_ATTRIBUTE_LABEL] = m_label;
    if (!m_description.empty())
        json[LABEL_OVERRIDE_JSON_ATTRIBUTE_DESCRIPTION] = m_description;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR LabelOverride::GetLabel (void) const { return m_label; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LabelOverride::SetLabel (Utf8String value) { m_label = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR LabelOverride::GetDescription (void) const { return m_description; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LabelOverride::SetDescription (Utf8String value) { m_description = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LabelOverride::_Accept(CustomizationRuleVisitor& visitor) const { visitor._Visit(*this); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 LabelOverride::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_label.empty())
        ADD_STR_VALUE_TO_HASH(md5, LABEL_OVERRIDE_JSON_ATTRIBUTE_LABEL, m_label);
    if (!m_description.empty())
        ADD_STR_VALUE_TO_HASH(md5, LABEL_OVERRIDE_JSON_ATTRIBUTE_DESCRIPTION, m_description);
    return md5;
    }
