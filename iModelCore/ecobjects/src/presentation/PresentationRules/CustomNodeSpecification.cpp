/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/CustomNodeSpecification.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>
#include <ECPresentationRules/SpecificationVisitor.h>

USING_NAMESPACE_BENTLEY_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CustomNodeSpecification::CustomNodeSpecification ()
    : ChildNodeSpecification (), m_type (L""), m_label (L""), m_description (L""), m_imageId (L"")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CustomNodeSpecification::CustomNodeSpecification (int priority, bool hideIfNoChildren, Utf8StringCR type, Utf8StringCR label, Utf8StringCR description, Utf8StringCR imageId)
    : ChildNodeSpecification (priority, true, false, hideIfNoChildren), 
    m_type (type), m_label (label), m_description (description), m_imageId (imageId)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodeSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP CustomNodeSpecification::_GetXmlElementName ()
    {
    return CUSTOM_NODE_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool CustomNodeSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_type, CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_TYPE))
        m_type = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_label, CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_LABEL))
        m_label = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_description, CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_DESCRIPTION))
        m_description = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_imageId, CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_IMAGEID))
        m_imageId = "";

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodeSpecification::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeStringValue (CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_TYPE, m_type.c_str ());
    xmlNode->AddAttributeStringValue (CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_LABEL, m_label.c_str ());
    xmlNode->AddAttributeStringValue (CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_DESCRIPTION, m_description.c_str ());
    xmlNode->AddAttributeStringValue (CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_IMAGEID, m_imageId.c_str ());
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