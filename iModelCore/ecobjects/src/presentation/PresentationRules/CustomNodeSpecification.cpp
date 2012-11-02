/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/CustomNodeSpecification.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_EC

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