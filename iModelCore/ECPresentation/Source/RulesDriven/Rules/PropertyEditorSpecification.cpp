/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
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
    if (!spec || !spec->ReadJson(json))
        DELETE_AND_CLEAR(spec);
    return spec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorSpecification::ReadXml(BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_name, PROPERTY_EDITORS_SPECIFICATION_XML_ATTRIBUTE_EDITORNAME))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_XML, PROPERTY_EDITORS_SPECIFICATION_XML_NODE_NAME, PROPERTY_EDITORS_SPECIFICATION_XML_ATTRIBUTE_EDITORNAME);
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
* @bsimethod                                    Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorSpecification::WriteXml(BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP editorNode = parentXmlNode->AddEmptyElement(PROPERTY_EDITORS_SPECIFICATION_XML_CHILD_NAME);
    editorNode->AddAttributeStringValue(PROPERTY_EDITORS_SPECIFICATION_XML_ATTRIBUTE_EDITORNAME, m_name.c_str());
    CommonToolsInternal::WriteRulesToXmlNode<PropertyEditorParametersSpecification, PropertyEditorParametersList>(editorNode, m_parameters);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorSpecification::ReadJson(JsonValueCR json)
    {
    //Required
    m_name = json[PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_EDITORNAME].asCString("");
    if (m_name.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "PropertyEditorsSpecification", PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_EDITORNAME);
        return false;
        }

    CommonToolsInternal::LoadFromJson(json[PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_PARAMETERS],
        m_parameters, PropertyEditorParametersSpecification::Create, this);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PropertyEditorSpecification::WriteJson() const
    {
    Json::Value json(Json::objectValue);
    json[PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_EDITORNAME] = m_name;
    if (!m_parameters.empty())
        {
        CommonToolsInternal::WriteRulesToJson<PropertyEditorParametersSpecification, PropertyEditorParametersList>
            (json[PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_PARAMETERS], m_parameters);
        }
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorSpecification::AddParameter(PropertyEditorParametersSpecificationR specification)
    {
    ADD_HASHABLE_CHILD(m_parameters, specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PropertyEditorSpecification::_ComputeHash() const
    {
    MD5 md5;
    md5.Add(m_name.c_str(), m_name.size());
    ADD_RULES_TO_HASH(md5, m_parameters);
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PropertyEditorParametersSpecification::_ComputeHash() const
    {
    MD5 md5;
    Utf8CP name = _GetXmlElementName();
    md5.Add(name, strlen(name));
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorParametersSpecification::WriteXml(BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP paramsNode = parentXmlNode->AddEmptyElement(_GetXmlElementName());
    _WriteXml(paramsNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorParametersSpecification::ReadJson(JsonValueCR json)
    {
    if (!json.isMember(COMMON_JSON_ATTRIBUTE_PARAMSTYPE) || !json[COMMON_JSON_ATTRIBUTE_PARAMSTYPE].isString())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "PropertyEditorParametersSpecification", COMMON_JSON_ATTRIBUTE_PARAMSTYPE);
        return false;
        }
    if (0 != strcmp(json[COMMON_JSON_ATTRIBUTE_PARAMSTYPE].asCString(), _GetJsonElementType()))
        return false;

    return _ReadJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PropertyEditorParametersSpecification::WriteJson() const
    {
    Json::Value json(Json::objectValue);
    json[COMMON_JSON_ATTRIBUTE_PARAMSTYPE] = _GetJsonElementType();
    _WriteJson(json);
    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyEditorJsonParameters::_GetXmlElementName() const {return PROPERTY_EDITOR_JSON_PARAMETERS_XML_NODE_NAME;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorJsonParameters::_ReadXml(BeXmlNodeP xmlNode)
    {
    Utf8String content;
    xmlNode->GetContent(content);
    if (!Json::Reader::Parse(content, m_json))
        BeAssert(false);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorJsonParameters::_WriteXml(BeXmlNodeP xmlNode) const
    {
    xmlNode->SetContent(WString(m_json.ToString().c_str(), BentleyCharEncoding::Utf8).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyEditorJsonParameters::_GetJsonElementType() const
    {
    return PROPERTY_EDITOR_JSON_PARAMETERS_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorJsonParameters::_ReadJson(JsonValueCR json)
    {
    m_json = json[PROPERTY_EDITOR_JSON_PARAMETERS_JSON_ATTRIBUTE_JSON];
    if (m_json.isNull())
        BeAssert(false);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorJsonParameters::_WriteJson(JsonValueR json) const
    {
    json[PROPERTY_EDITOR_JSON_PARAMETERS_JSON_ATTRIBUTE_JSON] = m_json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PropertyEditorJsonParameters::_ComputeHash() const
    {
    MD5 md5 = PropertyEditorParametersSpecification::_ComputeHash();
    Utf8String jsonString = m_json.ToString();
    md5.Add(jsonString.c_str(), jsonString.size());
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyEditorMultilineParameters::_GetXmlElementName() const {return PROPERTY_EDITOR_MULTILINE_PARAMETERS_XML_NODE_NAME;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorMultilineParameters::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeUInt32Value(m_height, PROPERTY_EDITOR_MULTILINE_PARAMETERS_ATTRIBUTE_HEIGHT))
        m_height = 1;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorMultilineParameters::_WriteXml(BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeUInt32Value(PROPERTY_EDITOR_MULTILINE_PARAMETERS_ATTRIBUTE_HEIGHT, m_height);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyEditorMultilineParameters::_GetJsonElementType() const
    {
    return PROPERTY_EDITOR_MULTILINE_PARAMETERS_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorMultilineParameters::_ReadJson(JsonValueCR json)
    {
    m_height = json[PROPERTY_EDITOR_MULTILINE_PARAMETERS_JSON_ATTRIBUTE_HEIGHT].asUInt(1);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorMultilineParameters::_WriteJson(JsonValueR json) const
    {
    json[PROPERTY_EDITOR_MULTILINE_PARAMETERS_JSON_ATTRIBUTE_HEIGHT] = m_height;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PropertyEditorMultilineParameters::_ComputeHash() const
    {
    MD5 md5 = PropertyEditorParametersSpecification::_ComputeHash();
    md5.Add(&m_height, sizeof(m_height));
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyEditorRangeParameters::_GetXmlElementName() const {return PROPERTY_EDITOR_RANGE_PARAMETERS_XML_NODE_NAME;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorRangeParameters::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (BEXML_Success == xmlNode->GetAttributeDoubleValue(m_min, PROPERTY_EDITOR_RANGE_PARAMETERS_XML_ATTRIBUTE_MINIMUM))
        m_isMinSet = true;
    if (BEXML_Success == xmlNode->GetAttributeDoubleValue(m_max, PROPERTY_EDITOR_RANGE_PARAMETERS_XML_ATTRIBUTE_MAXIMUM))
        m_isMaxSet = true;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorRangeParameters::_WriteXml(BeXmlNodeP xmlNode) const
    {
    if (m_isMinSet)
        xmlNode->AddAttributeDoubleValue(PROPERTY_EDITOR_RANGE_PARAMETERS_XML_ATTRIBUTE_MINIMUM, m_min);
    if (m_isMaxSet)
        xmlNode->AddAttributeDoubleValue(PROPERTY_EDITOR_RANGE_PARAMETERS_XML_ATTRIBUTE_MAXIMUM, m_max);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyEditorRangeParameters::_GetJsonElementType() const
    {
    return PROPERTY_EDITOR_RANGE_PARAMETERS_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorRangeParameters::_ReadJson(JsonValueCR json)
    {
    JsonValueCR minJson = json[PROPERTY_EDITOR_RANGE_PARAMETERS_JSON_ATTRIBUTE_MINIMUM];
    if (!minJson.isNull() && minJson.isConvertibleTo(Json::ValueType::realValue))
        {
        m_min = minJson.asDouble();
        m_isMinSet = true;
        }

    JsonValueCR maxJson = json[PROPERTY_EDITOR_RANGE_PARAMETERS_JSON_ATTRIBUTE_MAXIMUM];
    if (!maxJson.isNull() && maxJson.isConvertibleTo(Json::ValueType::realValue))
        {
        m_max = maxJson.asDouble();
        m_isMaxSet = true;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorRangeParameters::_WriteJson(JsonValueR json) const
    {
    if (m_isMinSet)
        json[PROPERTY_EDITOR_RANGE_PARAMETERS_JSON_ATTRIBUTE_MINIMUM] = m_min;
    if (m_isMaxSet)
        json[PROPERTY_EDITOR_RANGE_PARAMETERS_JSON_ATTRIBUTE_MAXIMUM] = m_max;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PropertyEditorRangeParameters::_ComputeHash() const
    {
    MD5 md5 = PropertyEditorParametersSpecification::_ComputeHash();
    md5.Add(&m_min, sizeof(m_min));
    md5.Add(&m_max, sizeof(m_max));
    md5.Add(&m_isMinSet, sizeof(m_isMinSet));
    md5.Add(&m_isMaxSet, sizeof(m_isMaxSet));
    return md5;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyEditorSliderParameters::_GetXmlElementName() const {return PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_NODE_NAME;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorSliderParameters::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeDoubleValue(m_min, PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_MINIMUM))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_XML, PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_NODE_NAME, PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_MINIMUM);
        return false;
        }
    if (BEXML_Success != xmlNode->GetAttributeDoubleValue(m_max, PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_MAXIMUM))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_XML, PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_NODE_NAME, PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_MAXIMUM);
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
* @bsimethod                                    Grigas.Petraitis                10/2017
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
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP PropertyEditorSliderParameters::_GetJsonElementType() const
    {
    return PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorSliderParameters::_ReadJson(JsonValueCR json)
    {
    //Required
    JsonValueCR minJson = json[PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_MINIMUM];
    if (minJson.isNull() || !minJson.isConvertibleTo(Json::ValueType::realValue))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "PropertyEditorSliderParameters", PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_MINIMUM);
        return false;
        }
    m_min = minJson.asDouble();

    JsonValueCR maxJson = json[PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_MAXIMUM];
    if (maxJson.isNull() || !maxJson.isConvertibleTo(Json::ValueType::realValue))
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "PropertyEditorSliderParameters", PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_MAXIMUM);
        return false;
        }
    m_max = maxJson.asDouble();

    //Optional
    m_intervalsCount = json[PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_INTERVALS].asUInt(1);
    m_isVertical = json[PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_VERTICAL].asBool(false);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorSliderParameters::_WriteJson(JsonValueR json) const
    {
    json[PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_MINIMUM] = m_min;
    json[PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_MAXIMUM] = m_max;
    json[PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_INTERVALS] = m_intervalsCount;
    json[PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_VERTICAL] = m_isVertical;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 PropertyEditorSliderParameters::_ComputeHash() const
    {
    MD5 md5 = PropertyEditorParametersSpecification::_ComputeHash();
    md5.Add(&m_min, sizeof(m_min));
    md5.Add(&m_max, sizeof(m_max));
    md5.Add(&m_intervalsCount, sizeof(m_intervalsCount));
    md5.Add(&m_valueFactor, sizeof(m_valueFactor));
    md5.Add(&m_isVertical, sizeof(m_isVertical));
    return md5;
    }
