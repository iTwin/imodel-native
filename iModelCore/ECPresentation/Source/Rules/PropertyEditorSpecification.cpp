/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyEditorSpecification::PropertyEditorSpecification(PropertyEditorSpecification const& other)
    : PresentationKey(other), m_name(other.m_name)
    {
    CommonToolsInternal::CloneRules(m_parameters, other.m_parameters, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyEditorSpecification::PropertyEditorSpecification(PropertyEditorSpecification&& other)
    : PresentationKey(std::move(other))
    {
    m_name.swap(other.m_name);
    CommonToolsInternal::SwapRules(m_parameters, other.m_parameters, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyEditorSpecification::~PropertyEditorSpecification()
    {
    CommonToolsInternal::FreePresentationRules(m_parameters);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyEditorParametersSpecification* PropertyEditorParametersSpecification::Create(BeJsConst json)
    {
    Utf8CP type = json[COMMON_JSON_ATTRIBUTE_PARAMSTYPE].asCString("");
    PropertyEditorParametersSpecification* spec = nullptr;
    if (0 == strcmp(PROPERTY_EDITOR_JSON_PARAMETERS_JSON_TYPE, type))
        spec = new PropertyEditorJsonParameters();
    else if (0 == strcmp(PROPERTY_EDITOR_MULTILINE_PARAMETERS_JSON_TYPE, type))
        spec = new PropertyEditorMultilineParameters();
    else if (0 == strcmp(PROPERTY_EDITOR_RANGE_PARAMETERS_JSON_TYPE, type))
        spec = new PropertyEditorRangeParameters();
    else if (0 == strcmp(PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_TYPE, type))
        spec = new PropertyEditorSliderParameters();
    else
        {
        Utf8String msg = json.isMember(COMMON_JSON_ATTRIBUTE_PARAMSTYPE)
            ? Utf8PrintfString("Invalid `" COMMON_JSON_ATTRIBUTE_PARAMSTYPE "` attribute value: `%s`", type)
            : Utf8String("Missing required attribute: `" COMMON_JSON_ATTRIBUTE_PARAMSTYPE "`");
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_TRACE, LOG_ERROR, msg);
        }
    if (!spec || !spec->ReadJson(json))
        DELETE_AND_CLEAR(spec);
    return spec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyEditorSpecification::_GetJsonElementType() const {return "PropertyEditorSpecification";}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorSpecification::_ReadJson(BeJsConst json)
    {
    if (!PresentationKey::_ReadJson(json))
        return false;

    // required:
    m_name = json[PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_EDITORNAME].asCString("");
    if (CommonToolsInternal::CheckRuleIssue(m_name.empty(), _GetJsonElementType(), PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_EDITORNAME, json[PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_EDITORNAME], "non-empty string"))
        return false;

    // optional:
    if (json.isMember(PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_PARAMETERS))
        {
        CommonToolsInternal::LoadFromJson(_GetJsonElementType(), PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_PARAMETERS, json[PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_PARAMETERS],
            m_parameters, PropertyEditorParametersSpecification::Create, this);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorSpecification::_WriteJson(BeJsValue json) const
    {
    PresentationKey::_WriteJson(json);
    json[PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_EDITORNAME] = m_name;
    if (!m_parameters.empty())
        {
        CommonToolsInternal::WriteRulesToJson<PropertyEditorParametersSpecification, PropertyEditorParametersList>
            (json[PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_PARAMETERS], m_parameters);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorSpecification::AddParameter(PropertyEditorParametersSpecificationR specification)
    {
    ADD_HASHABLE_CHILD(m_parameters, specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorSpecification::ClearParameters()
    {
    InvalidateHash();
    CommonToolsInternal::FreePresentationRules(m_parameters);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PropertyEditorSpecification::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_name.empty())
        ADD_STR_VALUE_TO_HASH(md5, PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_EDITORNAME, m_name);
    ADD_RULES_TO_HASH(md5, PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_PARAMETERS, m_parameters);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyEditorParametersSpecification::_GetJsonElementTypeAttributeName() const {return COMMON_JSON_ATTRIBUTE_PARAMSTYPE;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyEditorJsonParameters::_GetJsonElementType() const
    {
    return PROPERTY_EDITOR_JSON_PARAMETERS_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorJsonParameters::_ReadJson(BeJsConst json)
    {
    m_json.From(json[PROPERTY_EDITOR_JSON_PARAMETERS_JSON_ATTRIBUTE_JSON]);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorJsonParameters::_WriteJson(BeJsValue json) const
    {
    json[PROPERTY_EDITOR_JSON_PARAMETERS_JSON_ATTRIBUTE_JSON].From(m_json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PropertyEditorJsonParameters::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    Utf8String jsonString = m_json.Stringify();
    ADD_STR_VALUE_TO_HASH(md5, PROPERTY_EDITOR_JSON_PARAMETERS_JSON_ATTRIBUTE_JSON, jsonString);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyEditorMultilineParameters::_GetJsonElementType() const
    {
    return PROPERTY_EDITOR_MULTILINE_PARAMETERS_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorMultilineParameters::_ReadJson(BeJsConst json)
    {
    m_height = json[PROPERTY_EDITOR_MULTILINE_PARAMETERS_JSON_ATTRIBUTE_HEIGHT].asUInt(1);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorMultilineParameters::_WriteJson(BeJsValue json) const
    {
    json[PROPERTY_EDITOR_MULTILINE_PARAMETERS_JSON_ATTRIBUTE_HEIGHT] = m_height;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PropertyEditorMultilineParameters::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (m_height != 1)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, PROPERTY_EDITOR_MULTILINE_PARAMETERS_JSON_ATTRIBUTE_HEIGHT, m_height);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyEditorRangeParameters::_GetJsonElementType() const
    {
    return PROPERTY_EDITOR_RANGE_PARAMETERS_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorRangeParameters::_ReadJson(BeJsConst json)
    {
    BeJsConst minJson = json[PROPERTY_EDITOR_RANGE_PARAMETERS_JSON_ATTRIBUTE_MINIMUM];
    if (!minJson.isNull() && minJson.isNumeric())
        m_min = minJson.GetDouble();

    BeJsConst maxJson = json[PROPERTY_EDITOR_RANGE_PARAMETERS_JSON_ATTRIBUTE_MAXIMUM];
    if (!maxJson.isNull() && minJson.isNumeric())
        m_max = maxJson.GetDouble();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorRangeParameters::_WriteJson(BeJsValue json) const
    {
    if (m_min.IsValid())
        json[PROPERTY_EDITOR_RANGE_PARAMETERS_JSON_ATTRIBUTE_MINIMUM] = m_min.Value();
    if (m_max.IsValid())
        json[PROPERTY_EDITOR_RANGE_PARAMETERS_JSON_ATTRIBUTE_MAXIMUM] = m_max.Value();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PropertyEditorRangeParameters::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (!m_min.IsNull())
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, PROPERTY_EDITOR_RANGE_PARAMETERS_JSON_ATTRIBUTE_MINIMUM, m_min);
    if (!m_max.IsNull())
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, PROPERTY_EDITOR_RANGE_PARAMETERS_JSON_ATTRIBUTE_MAXIMUM, m_max);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyEditorSliderParameters::_GetJsonElementType() const
    {
    return PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorSliderParameters::_ReadJson(BeJsConst json)
    {
    // required:
    BeJsConst minJson = json[PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_MINIMUM];
    BeJsConst maxJson = json[PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_MAXIMUM];

    bool hasIssues = false
        || CommonToolsInternal::CheckRuleIssue(minJson.isNull() || !minJson.isNumeric(), _GetJsonElementType(), PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_MINIMUM, minJson, "floating point value")
        || CommonToolsInternal::CheckRuleIssue(maxJson.isNull() || !maxJson.isNumeric(), _GetJsonElementType(), PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_MAXIMUM, maxJson, "floating point value");
    if (hasIssues)
        return false;

    m_min = minJson.asDouble();
    m_max = maxJson.asDouble();

    // optional:
    m_intervalsCount = json[PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_INTERVALS].asUInt(1);
    m_isVertical = json[PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_VERTICAL].asBool(false);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorSliderParameters::_WriteJson(BeJsValue json) const
    {
    json[PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_MINIMUM] = m_min;
    json[PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_MAXIMUM] = m_max;
    json[PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_INTERVALS] = m_intervalsCount;
    json[PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_VERTICAL] = m_isVertical;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PropertyEditorSliderParameters::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    if (m_min != 0)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_MINIMUM, m_min);
    if (m_max != 0)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_MAXIMUM, m_max);
    if (m_intervalsCount != 1)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_INTERVALS, m_intervalsCount);
    if (m_isVertical)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_VERTICAL, m_isVertical);
    return md5;
    }
