/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/ContentInstancesOfSpecificClassesSpecification.cpp $
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
ContentInstancesOfSpecificClassesSpecification::ContentInstancesOfSpecificClassesSpecification () 
    : ContentSpecification (), m_instanceFilter (""), m_classNames (""), m_arePolymorphic (false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentInstancesOfSpecificClassesSpecification::ContentInstancesOfSpecificClassesSpecification (int priority, Utf8StringCR instanceFilter, Utf8StringCR classNames, bool arePolymorphic) 
    : ContentSpecification (priority), m_instanceFilter (instanceFilter), m_classNames (classNames), m_arePolymorphic (arePolymorphic)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentInstancesOfSpecificClassesSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

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
        LOG.errorv ("Invalid XML: %s element must contain a %s attribute", CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME, COMMON_XML_ATTRIBUTE_CLASSNAMES);
        return false;
        }

    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue (m_arePolymorphic, COMMON_XML_ATTRIBUTE_AREPOLYMORPHIC))
        m_arePolymorphic = false;

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_instanceFilter, COMMON_XML_ATTRIBUTE_INSTANCEFILTER))
        m_instanceFilter = "";

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ContentInstancesOfSpecificClassesSpecification::GetClassNames (void) const { return m_classNames; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentInstancesOfSpecificClassesSpecification::SetClassNames (Utf8StringCR value) { m_classNames = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentInstancesOfSpecificClassesSpecification::GetArePolymorphic (void) const { return m_arePolymorphic; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentInstancesOfSpecificClassesSpecification::SetArePolymorphic (bool value) { m_arePolymorphic = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ContentInstancesOfSpecificClassesSpecification::GetInstanceFilter (void) const { return m_instanceFilter; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentInstancesOfSpecificClassesSpecification::SetInstanceFilter (Utf8StringCR value) { m_instanceFilter = value; }