/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/CustomNodeSpecification.cpp $
|
|   $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_EC

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
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR CustomNodeSpecification::GetLabel (void) const { return m_label; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR CustomNodeSpecification::GetDescription (void) const { return m_description; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR CustomNodeSpecification::GetImageId (void) const { return m_imageId; }