/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/ContentInstancesOfSpecificClassesSpecification.cpp $
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
CharCP ContentInstancesOfSpecificClassesSpecification::_GetXmlElementName ()
    {
    return CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentInstancesOfSpecificClassesSpecification::_ReadXml (BeXmlNodeP xmlNode)
    {
    //Required:
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_classNames, COMMON_XML_ATTRIBUTE_CLASSNAMES))
        {
        LOG.errorv (L"Invalid XML: %hs element must contain a %hs attribute", CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME, COMMON_XML_ATTRIBUTE_CLASSNAMES);
        return false;
        }

    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_arePolymorphic, COMMON_XML_ATTRIBUTE_AREPOLYMORPHIC))
        m_arePolymorphic = false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_instanceFilter, COMMON_XML_ATTRIBUTE_INSTANCEFILTER))
        m_instanceFilter = L"";

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentInstancesOfSpecificClassesSpecification::_WriteXml (BeXmlNodeP xmlNode)
    {
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_CLASSNAMES, m_classNames.c_str ());
    xmlNode->AddAttributeBooleanValue (COMMON_XML_ATTRIBUTE_AREPOLYMORPHIC, m_arePolymorphic);
    xmlNode->AddAttributeStringValue  (COMMON_XML_ATTRIBUTE_INSTANCEFILTER, m_instanceFilter.c_str ());
    }