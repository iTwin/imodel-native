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
CustomNodeSpecification::CustomNodeSpecification (int priority, bool hideIfNoChildren, WStringCR type, WStringCR label, WStringCR description, WStringCR imageId)
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
        m_type = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_label, CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_LABEL))
        m_label = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_description, CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_DESCRIPTION))
        m_description = L"";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_imageId, CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_IMAGEID))
        m_imageId = L"";

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
WStringCR CustomNodeSpecification::GetNodeType (void) const { return m_type; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodeSpecification::SetNodeType (WString value) { m_type = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR CustomNodeSpecification::GetLabel (void) const { return m_label; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodeSpecification::SetLabel (WString value) { m_label = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR CustomNodeSpecification::GetDescription (void) const { return m_description; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodeSpecification::SetDescription (WString value) { m_description = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR CustomNodeSpecification::GetImageId (void) const { return m_imageId; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomNodeSpecification::SetImageId (WString value) { m_imageId = value; }