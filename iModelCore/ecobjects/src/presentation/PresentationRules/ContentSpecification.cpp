/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/ContentSpecification.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/CommonTools.h>
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecification::ContentSpecification () : m_priority (1000)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecification::ContentSpecification (int priority) : m_priority (priority)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecification::~ContentSpecification ()
    {
    CommonTools::FreePresentationRules (m_relatedPropertiesSpecification);
    CommonTools::FreePresentationRules (m_displayRelatedItemsSpecification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentSpecification::ReadXml (BeXmlNodeP xmlNode)
    {
    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeInt32Value (m_priority, COMMON_XML_ATTRIBUTE_PRIORITY))
        m_priority = 1000;

    CommonTools::LoadSpecificationsFromXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList> (xmlNode, m_relatedPropertiesSpecification, RELATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
    CommonTools::LoadSpecificationsFromXmlNode<DisplayRelatedItemsSpecification, DisplayRelatedItemsSpecificationList> (xmlNode, m_displayRelatedItemsSpecification, DISPLAYRELATEDITEMS_SPECIFICATION_XML_NODE_NAME);

    return _ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecification::WriteXml (BeXmlNodeP parentXmlNode)
    {
    BeXmlNodeP specificationNode = parentXmlNode->AddEmptyElement (_GetXmlElementName ());

    specificationNode->AddAttributeInt32Value   (COMMON_XML_ATTRIBUTE_PRIORITY, m_priority);

    CommonTools::WriteRulesToXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList> (specificationNode, m_relatedPropertiesSpecification);
    CommonTools::WriteRulesToXmlNode<DisplayRelatedItemsSpecification, DisplayRelatedItemsSpecificationList> (specificationNode, m_displayRelatedItemsSpecification);

    //Make sure we call protected override
    _WriteXml (specificationNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int ContentSpecification::GetPriority (void) const { return m_priority; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Kelly.Shiptoski                 06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecification::SetPriority (int value) { m_priority = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecificationList& ContentSpecification::GetRelatedProperties (void) { return m_relatedPropertiesSpecification; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayRelatedItemsSpecificationList& ContentSpecification::GetDisplayRelatedItems (void) { return m_displayRelatedItemsSpecification; }
