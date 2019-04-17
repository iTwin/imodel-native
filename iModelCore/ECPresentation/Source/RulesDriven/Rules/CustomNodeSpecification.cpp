/*--------------------------------------------------------------------------------------+
|
|   Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CustomNodeSpecification::CustomNodeSpecification()
    : ChildNodeSpecification()
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CustomNodeSpecification::CustomNodeSpecification (int priority, bool hideIfNoChildren, Utf8StringCR type, Utf8StringCR label, Utf8StringCR description, Utf8StringCR imageId)
    : ChildNodeSpecification (priority, ChildrenHint::Always, false, hideIfNoChildren), 
    m_type(type), m_label(label), m_description(description), m_imageId(imageId)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodeSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP CustomNodeSpecification::_GetXmlElementName () const
    {
    return CUSTOM_NODE_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool CustomNodeSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    if (!ChildNodeSpecification::_ReadXml(xmlNode))
        return false;

    //Required:
    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_type, CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_TYPE))
        return false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_label, CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_LABEL))
        return false;
    
    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_description, CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_DESCRIPTION))
        m_description = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_imageId, CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_IMAGEID))
        m_imageId = "";

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodeSpecification::_WriteXml (BeXmlNodeP xmlNode) const
    {
    ChildNodeSpecification::_WriteXml(xmlNode);
    xmlNode->AddAttributeStringValue (CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_TYPE, m_type.c_str ());
    xmlNode->AddAttributeStringValue (CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_LABEL, m_label.c_str ());
    xmlNode->AddAttributeStringValue (CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_DESCRIPTION, m_description.c_str ());
    xmlNode->AddAttributeStringValue (CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_IMAGEID, m_imageId.c_str ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP CustomNodeSpecification::_GetJsonElementType() const
    {
    return CUSTOM_NODE_SPECIFICATION_JSON_TYPE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool CustomNodeSpecification::_ReadJson(JsonValueCR json)
    {
    if (!ChildNodeSpecification::_ReadJson(json))
        return false;

    m_type = json[CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_TYPE].asCString("");
    if (m_type.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "CustomNodeSpecification", CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_TYPE);
        return false;
        }

    m_label = json[CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_LABEL].asCString("");
    if (m_label.empty())
        {
        ECPRENSETATION_RULES_LOG.errorv(INVALID_JSON, "CustomNodeSpecification", CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_LABEL);
        return false;
        }

    m_description = json[CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_DESCRIPTION].asCString("");
    m_imageId = json[CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_IMAGEID].asCString("");
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodeSpecification::_WriteJson(JsonValueR json) const
    {
    ChildNodeSpecification::_WriteJson(json);
    json[CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_TYPE] = m_type;
    json[CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_LABEL] = m_label;
    if (!m_description.empty())
        json[CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_DESCRIPTION] = m_description;
    if (!m_imageId.empty())
        json[CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_IMAGEID] = m_imageId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR CustomNodeSpecification::GetNodeType (void) const { return m_type; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodeSpecification::SetNodeType (Utf8StringCR value) { m_type = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR CustomNodeSpecification::GetLabel (void) const { return m_label; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodeSpecification::SetLabel (Utf8StringCR value) { m_label = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR CustomNodeSpecification::GetDescription (void) const { return m_description; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodeSpecification::SetDescription (Utf8StringCR value) { m_description = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR CustomNodeSpecification::GetImageId (void) const { return m_imageId; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodeSpecification::SetImageId (Utf8StringCR value) { m_imageId = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 CustomNodeSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5 = ChildNodeSpecification::_ComputeHash(parentHash);
    md5.Add(m_type.c_str(), m_type.size());
    md5.Add(m_label.c_str(), m_label.size());
    md5.Add(m_description.c_str(), m_description.size());
    md5.Add(m_imageId.c_str(), m_imageId.size());
    return md5;
    }