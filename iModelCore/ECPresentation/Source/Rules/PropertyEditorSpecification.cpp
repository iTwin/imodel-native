/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
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
PropertyEditorParametersSpecification* PropertyEditorParametersSpecification::Create(JsonValueCR json)
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
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, msg);
        }
    if (!spec || !spec->ReadJson(json))
        DELETE_AND_CLEAR(spec);
    return spec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyEditorSpecification::_GetXmlElementName() const {return PROPERTY_EDITORS_SPECIFICATION_XML_CHILD_NAME;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorSpecification::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (!PresentationKey::_ReadXml(xmlNode))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_name, PROPERTY_EDITORS_SPECIFICATION_XML_ATTRIBUTE_EDITORNAME))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, Utf8PrintfString(INVALID_XML, PROPERTY_EDITORS_SPECIFICATION_XML_NODE_NAME, PROPERTY_EDITORS_SPECIFICATION_XML_ATTRIBUTE_EDITORNAME));
        return false;
        }

    for (BeXmlNodeP child = xmlNode->GetFirstChild(BEXMLNODE_Element); nullptr != child; child = child->GetNextSibling(BEXMLNODE_Element))
        {
        if (0 == BeStringUtilities::Stricmp(child->GetName(), PROPERTY_EDITOR_JSON_PARAMETERS_XML_NODE_NAME))
            CommonToolsInternal::LoadSpecificationFromXmlNode<PropertyEditorJsonParameters, PropertyEditorParametersList>(child, m_parameters, this);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), PROPERTY_EDITOR_MULTILINE_PARAMETERS_XML_NODE_NAME))
            CommonToolsInternal::LoadSpecificationFromXmlNode<PropertyEditorMultilineParameters, PropertyEditorParametersList>(child, m_parameters, this);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), PROPERTY_EDITOR_RANGE_PARAMETERS_XML_NODE_NAME))
            CommonToolsInternal::LoadSpecificationFromXmlNode<PropertyEditorRangeParameters, PropertyEditorParametersList>(child, m_parameters, this);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_NODE_NAME))
            CommonToolsInternal::LoadSpecificationFromXmlNode<PropertyEditorSliderParameters, PropertyEditorParametersList>(child, m_parameters, this);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorSpecification::_WriteXml(BeXmlNodeP xmlNode) const
    {
    PresentationKey::_WriteXml(xmlNode);
    xmlNode->AddAttributeStringValue(PROPERTY_EDITORS_SPECIFICATION_XML_ATTRIBUTE_EDITORNAME, m_name.c_str());
    CommonToolsInternal::WriteRulesToXmlNode<PropertyEditorParametersSpecification, PropertyEditorParametersList>(xmlNode, m_parameters);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyEditorSpecification::_GetJsonElementType() const {return "PropertyEditorSpecification";}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorSpecification::_ReadJson(JsonValueCR json)
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
void PropertyEditorSpecification::_WriteJson(JsonValueR json) const
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
bool PropertyEditorSpecification::_ShallowEqual(PresentationKeyCR other) const
    {
    PropertyEditorSpecification const* otherRule = dynamic_cast<PropertyEditorSpecification const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_name == otherRule->m_name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyEditorParametersSpecification::_GetJsonElementTypeAttributeName() const {return COMMON_JSON_ATTRIBUTE_PARAMSTYPE;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyEditorJsonParameters::_GetXmlElementName() const {return PROPERTY_EDITOR_JSON_PARAMETERS_XML_NODE_NAME;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorJsonParameters::_ReadXml(BeXmlNodeP xmlNode)
    {
    Utf8String content;
    xmlNode->GetContent(content);
    if (!Json::Reader::Parse(content, m_json))
        {
        DIAGNOSTICS_EDITOR_LOG(DiagnosticsCategory::Rules, LOG_ERROR, Utf8PrintfString("Failed to parse property editor JSON parameters: %s", content.c_str()));
        return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorJsonParameters::_WriteXml(BeXmlNodeP xmlNode) const
    {
    xmlNode->SetContent(WString(m_json.ToString().c_str(), BentleyCharEncoding::Utf8).c_str());
    }

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
bool PropertyEditorJsonParameters::_ReadJson(JsonValueCR json)
    {
    m_json = json[PROPERTY_EDITOR_JSON_PARAMETERS_JSON_ATTRIBUTE_JSON];
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorJsonParameters::_WriteJson(JsonValueR json) const
    {
    json[PROPERTY_EDITOR_JSON_PARAMETERS_JSON_ATTRIBUTE_JSON] = m_json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PropertyEditorJsonParameters::_ComputeHash() const
    {
    MD5 md5 = T_Super::_ComputeHash();
    Utf8String jsonString = m_json.ToString();
    ADD_STR_VALUE_TO_HASH(md5, PROPERTY_EDITOR_JSON_PARAMETERS_JSON_ATTRIBUTE_JSON, jsonString);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorJsonParameters::_ShallowEqual(PresentationKeyCR other) const
    {
    PropertyEditorJsonParameters const* otherRule = dynamic_cast<PropertyEditorJsonParameters const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_json == otherRule->m_json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyEditorMultilineParameters::_GetXmlElementName() const {return PROPERTY_EDITOR_MULTILINE_PARAMETERS_XML_NODE_NAME;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorMultilineParameters::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeUInt32Value(m_height, PROPERTY_EDITOR_MULTILINE_PARAMETERS_ATTRIBUTE_HEIGHT))
        m_height = 1;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorMultilineParameters::_WriteXml(BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeUInt32Value(PROPERTY_EDITOR_MULTILINE_PARAMETERS_ATTRIBUTE_HEIGHT, m_height);
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
bool PropertyEditorMultilineParameters::_ReadJson(JsonValueCR json)
    {
    m_height = json[PROPERTY_EDITOR_MULTILINE_PARAMETERS_JSON_ATTRIBUTE_HEIGHT].asUInt(1);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorMultilineParameters::_WriteJson(JsonValueR json) const
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
bool PropertyEditorMultilineParameters::_ShallowEqual(PresentationKeyCR other) const
    {
    PropertyEditorMultilineParameters const* otherRule = dynamic_cast<PropertyEditorMultilineParameters const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_height == otherRule->m_height;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyEditorRangeParameters::_GetXmlElementName() const {return PROPERTY_EDITOR_RANGE_PARAMETERS_XML_NODE_NAME;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorRangeParameters::_ReadXml(BeXmlNodeP xmlNode)
    {
    double value;
    if (BEXML_Success == xmlNode->GetAttributeDoubleValue(value, PROPERTY_EDITOR_RANGE_PARAMETERS_XML_ATTRIBUTE_MINIMUM))
        m_min = value;
    if (BEXML_Success == xmlNode->GetAttributeDoubleValue(value, PROPERTY_EDITOR_RANGE_PARAMETERS_XML_ATTRIBUTE_MAXIMUM))
        m_max = value;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorRangeParameters::_WriteXml(BeXmlNodeP xmlNode) const
    {
    if (m_min.IsValid())
        xmlNode->AddAttributeDoubleValue(PROPERTY_EDITOR_RANGE_PARAMETERS_XML_ATTRIBUTE_MINIMUM, m_min.Value());
    if (m_max.IsValid())
        xmlNode->AddAttributeDoubleValue(PROPERTY_EDITOR_RANGE_PARAMETERS_XML_ATTRIBUTE_MAXIMUM, m_max.Value());
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
bool PropertyEditorRangeParameters::_ReadJson(JsonValueCR json)
    {
    JsonValueCR minJson = json[PROPERTY_EDITOR_RANGE_PARAMETERS_JSON_ATTRIBUTE_MINIMUM];
    if (!minJson.isNull() && minJson.isConvertibleTo(Json::ValueType::realValue))
        m_min = minJson.asDouble();

    JsonValueCR maxJson = json[PROPERTY_EDITOR_RANGE_PARAMETERS_JSON_ATTRIBUTE_MAXIMUM];
    if (!maxJson.isNull() && maxJson.isConvertibleTo(Json::ValueType::realValue))
        m_max = maxJson.asDouble();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorRangeParameters::_WriteJson(JsonValueR json) const
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
bool PropertyEditorRangeParameters::_ShallowEqual(PresentationKeyCR other) const
    {
    PropertyEditorRangeParameters const* otherRule = dynamic_cast<PropertyEditorRangeParameters const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_min == otherRule->m_min
        && m_max == otherRule->m_max;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyEditorSliderParameters::_GetXmlElementName() const {return PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_NODE_NAME;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorSliderParameters::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeDoubleValue(m_min, PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_MINIMUM))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, Utf8PrintfString(INVALID_XML, PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_NODE_NAME, PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_MINIMUM));
        return false;
        }
    if (BEXML_Success != xmlNode->GetAttributeDoubleValue(m_max, PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_MAXIMUM))
        {
        DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_ERROR, Utf8PrintfString(INVALID_XML, PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_NODE_NAME, PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_MAXIMUM));
        return false;
        }
    if (BEXML_Success != xmlNode->GetAttributeUInt32Value(m_intervalsCount, PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_INTERVALS))
        m_intervalsCount = 1;
    if (BEXML_Success != xmlNode->GetAttributeUInt32Value(m_valueFactor, PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_VALUEFACTOR))
        m_valueFactor = 1;
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue(m_isVertical, PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_VERTICAL))
        m_isVertical = false;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorSliderParameters::_WriteXml(BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeDoubleValue(PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_MINIMUM, m_min);
    xmlNode->AddAttributeDoubleValue(PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_MAXIMUM, m_max);
    xmlNode->AddAttributeUInt32Value(PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_INTERVALS, m_intervalsCount);
    xmlNode->AddAttributeUInt32Value(PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_VALUEFACTOR, m_valueFactor);
    xmlNode->AddAttributeBooleanValue(PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_VERTICAL, m_isVertical);
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
bool PropertyEditorSliderParameters::_ReadJson(JsonValueCR json)
    {
    // required:
    JsonValueCR minJson = json[PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_MINIMUM];
    JsonValueCR maxJson = json[PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_MAXIMUM];

    bool hasIssues = false
        || CommonToolsInternal::CheckRuleIssue(minJson.isNull() || !minJson.isConvertibleTo(Json::ValueType::realValue), _GetJsonElementType(), PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_MINIMUM, minJson, "floating point value")
        || CommonToolsInternal::CheckRuleIssue(maxJson.isNull() || !maxJson.isConvertibleTo(Json::ValueType::realValue), _GetJsonElementType(), PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_MAXIMUM, maxJson, "floating point value");
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
void PropertyEditorSliderParameters::_WriteJson(JsonValueR json) const
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
    if (m_valueFactor != 1)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_VALUEFACTOR, m_valueFactor);
    if (m_isVertical)
        ADD_PRIMITIVE_VALUE_TO_HASH(md5, PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_VERTICAL, m_isVertical);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorSliderParameters::_ShallowEqual(PresentationKeyCR other) const
    {
    PropertyEditorSliderParameters const* otherRule = dynamic_cast<PropertyEditorSliderParameters const*>(&other);
    if (nullptr == otherRule)
        return false;

    return m_min == otherRule->m_min
        && m_max == otherRule->m_max
        && m_intervalsCount == otherRule->m_intervalsCount
        && m_valueFactor == otherRule->m_valueFactor
        && m_isVertical == otherRule->m_isVertical;
    }
