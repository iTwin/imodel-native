/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/PropertyEditorsSpecification.cpp $
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/CommonTools.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyEditorsSpecification::ReadXml(BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_propertyName, PROPERTY_EDITORS_SPECIFICATION_XML_ATTRIBUTE_PROPERTYNAME))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_editorName, PROPERTY_EDITORS_SPECIFICATION_XML_ATTRIBUTE_EDITORNAME))
        return false;
    
    for (BeXmlNodeP child = xmlNode->GetFirstChild(BEXMLNODE_Element); nullptr != child; child = child->GetNextSibling(BEXMLNODE_Element))
        {
        if (0 == BeStringUtilities::Stricmp(child->GetName(), PROPERTY_EDITOR_JSON_PARAMETERS_XML_NODE_NAME))
            CommonTools::LoadSpecificationFromXmlNode<PropertyEditorJsonParameters, PropertyEditorParametersList>(child, m_parameters);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), PROPERTY_EDITOR_MULTILINE_PARAMETERS_XML_NODE_NAME))
            CommonTools::LoadSpecificationFromXmlNode<PropertyEditorMultilineParameters, PropertyEditorParametersList>(child, m_parameters);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), PROPERTY_EDITOR_RANGE_PARAMETERS_XML_NODE_NAME))
            CommonTools::LoadSpecificationFromXmlNode<PropertyEditorRangeParameters, PropertyEditorParametersList>(child, m_parameters);
        else if (0 == BeStringUtilities::Stricmp(child->GetName(), PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_NODE_NAME))
            CommonTools::LoadSpecificationFromXmlNode<PropertyEditorSliderParameters, PropertyEditorParametersList>(child, m_parameters);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PropertyEditorsSpecification::WriteXml(BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP editorNode = parentXmlNode->AddEmptyElement(PROPERTY_EDITORS_SPECIFICATION_XML_CHILD_NAME);
    editorNode->AddAttributeStringValue(PROPERTY_EDITORS_SPECIFICATION_XML_ATTRIBUTE_PROPERTYNAME, m_propertyName.c_str());
    editorNode->AddAttributeStringValue(PROPERTY_EDITORS_SPECIFICATION_XML_ATTRIBUTE_EDITORNAME, m_editorName.c_str());
    CommonTools::WriteRulesToXmlNode<PropertyEditorParametersSpecification, PropertyEditorParametersList>(editorNode, m_parameters);
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
void PropertyEditorJsonParameters::_WriteXml(BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP paramsNode = parentXmlNode->AddEmptyElement(_GetXmlElementName());
    paramsNode->SetContent(WString(m_json.ToString().c_str(), BentleyCharEncoding::Utf8).c_str());
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
void PropertyEditorMultilineParameters::_WriteXml(BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP paramsNode = parentXmlNode->AddEmptyElement(_GetXmlElementName());
    paramsNode->AddAttributeUInt32Value(PROPERTY_EDITOR_MULTILINE_PARAMETERS_ATTRIBUTE_HEIGHT, m_height);
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
void PropertyEditorRangeParameters::_WriteXml(BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP paramsNode = parentXmlNode->AddEmptyElement(_GetXmlElementName());
    if (m_isMinSet)
        paramsNode->AddAttributeDoubleValue(PROPERTY_EDITOR_RANGE_PARAMETERS_XML_ATTRIBUTE_MINIMUM, m_min);
    if (m_isMaxSet)
        paramsNode->AddAttributeDoubleValue(PROPERTY_EDITOR_RANGE_PARAMETERS_XML_ATTRIBUTE_MAXIMUM, m_max);
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
        return false;
    if (BEXML_Success != xmlNode->GetAttributeDoubleValue(m_max, PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_MAXIMUM))
        return false;
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
void PropertyEditorSliderParameters::_WriteXml(BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP paramsNode = parentXmlNode->AddEmptyElement(_GetXmlElementName());
    paramsNode->AddAttributeDoubleValue(PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_MINIMUM, m_min);
    paramsNode->AddAttributeDoubleValue(PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_MAXIMUM, m_max);
    paramsNode->AddAttributeUInt32Value(PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_INTERVALS, m_intervalsCount);
    paramsNode->AddAttributeUInt32Value(PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_VALUEFACTOR, m_valueFactor);
    paramsNode->AddAttributeBooleanValue(PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_VERTICAL, m_isVertical);
    }