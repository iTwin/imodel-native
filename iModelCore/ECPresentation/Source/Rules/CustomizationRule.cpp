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
CustomizationRuleP CustomizationRule::Create(BeJsConst json)
    {
    Utf8CP type = json[COMMON_JSON_ATTRIBUTE_RULETYPE].asCString("");
    CustomizationRuleP spec = nullptr;
    if (0 == strcmp(IMAGE_ID_OVERRIDE_JSON_TYPE, type))
        spec = new ImageIdOverride();
    else if (0 == strcmp(LABEL_OVERRIDE_JSON_TYPE, type))
        spec = new LabelOverride();
    else if (0 == strcmp(INSTANCE_LABEL_OVERRIDE_JSON_TYPE, type))
        spec = new InstanceLabelOverride();
    else if (0 == strcmp(STYLE_OVERRIDE_JSON_TYPE, type))
        spec = new StyleOverride();
    else if (0 == strcmp(CHECKBOX_RULE_JSON_TYPE, type))
        spec = new CheckBoxRule();
    else if (0 == strcmp(SORTING_RULE_DISABLEDSORTING_JSON_TYPE, type) || 0 == strcmp(SORTING_RULE_PROPERTYSORTING_JSON_TYPE, type))
        spec = new SortingRule();
    else if (0 == strcmp(GROUPING_RULE_JSON_TYPE, type))
        spec = new GroupingRule();
    else if (0 == strcmp(EXTENDED_DATA_RULE_JSON_TYPE, type))
        spec = new ExtendedDataRule();
    else if (0 == strcmp(NODE_ARTIFACTS_RULE_JSON_TYPE, type))
        spec = new NodeArtifactsRule();
    else
        {
        Utf8String msg = json.isMember(COMMON_JSON_ATTRIBUTE_RULETYPE)
            ? Utf8PrintfString("Invalid `" COMMON_JSON_ATTRIBUTE_RULETYPE "` attribute value: `%s`", type)
            : Utf8String("Missing required attribute: `" COMMON_JSON_ATTRIBUTE_RULETYPE "`");
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_TRACE, LOG_ERROR, msg);
        }
    if (!spec || !spec->ReadJson(json))
        DELETE_AND_CLEAR(spec);
    return spec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ConditionalCustomizationRule::GetCondition() const { return m_condition; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConditionalCustomizationRule::SetCondition(Utf8String value) { m_condition = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConditionalCustomizationRule::_ReadJson(BeJsConst json)
    {
    if (!CustomizationRule::_ReadJson(json))
        return false;

    m_condition = json[PRESENTATION_RULE_JSON_ATTRIBUTE_CONDITION].asString("");
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConditionalCustomizationRule::_WriteJson(BeJsValue json) const
    {
    CustomizationRule::_WriteJson(json);
    if (!m_condition.empty())
        json[PRESENTATION_RULE_JSON_ATTRIBUTE_CONDITION] = m_condition;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 ConditionalCustomizationRule::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_condition.empty())
        ADD_STR_VALUE_TO_HASH(md5, PRESENTATION_RULE_JSON_ATTRIBUTE_CONDITION, m_condition);
    return md5;
    }
