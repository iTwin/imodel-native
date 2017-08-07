/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/ContentSpecification.cpp $
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
ContentSpecification::ContentSpecification() : m_priority(1000), m_showImages(false) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecification::ContentSpecification(int priority, bool showImages)
    : m_priority(priority), m_showImages(showImages)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecification::ContentSpecification(ContentSpecificationCR other)
    : m_priority(other.m_priority), m_showImages(other.m_showImages)
    {
    CommonTools::CopyRules(m_relatedPropertiesSpecification, other.m_relatedPropertiesSpecification);
    CommonTools::CopyRules(m_propertiesDisplaySpecification, other.m_propertiesDisplaySpecification);
    CommonTools::CopyRules(m_displayRelatedItemsSpecification, other.m_displayRelatedItemsSpecification);
    CommonTools::CopyRules(m_calculatedPropertiesSpecification, other.m_calculatedPropertiesSpecification);
    CommonTools::CopyRules(m_propertyEditorsSpecification, other.m_propertyEditorsSpecification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSpecification::~ContentSpecification ()
    {
    CommonTools::FreePresentationRules (m_relatedPropertiesSpecification);
    CommonTools::FreePresentationRules (m_propertiesDisplaySpecification);
    CommonTools::FreePresentationRules (m_displayRelatedItemsSpecification);
    CommonTools::FreePresentationRules (m_calculatedPropertiesSpecification);
    CommonTools::FreePresentationRules (m_propertyEditorsSpecification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentSpecification::ReadXml (BeXmlNodeP xmlNode)
    {
    //Optional:
    if (BEXML_Success != xmlNode->GetAttributeInt32Value(m_priority, COMMON_XML_ATTRIBUTE_PRIORITY))
        m_priority = 1000;

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue(m_showImages, CONTENT_SPECIFICATION_XML_ATTRIBUTE_SHOWIMAGES))
        m_showImages = false;

    CommonTools::LoadSpecificationsFromXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList> (xmlNode, m_relatedPropertiesSpecification, RELATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
    CommonTools::LoadSpecificationsFromXmlNode<PropertiesDisplaySpecification, PropertiesDisplaySpecificationList> (xmlNode, m_propertiesDisplaySpecification, HIDDEN_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
    CommonTools::LoadSpecificationsFromXmlNode<PropertiesDisplaySpecification, PropertiesDisplaySpecificationList> (xmlNode, m_propertiesDisplaySpecification, DISPLAYED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
    CommonTools::LoadSpecificationsFromXmlNode<DisplayRelatedItemsSpecification, DisplayRelatedItemsSpecificationList> (xmlNode, m_displayRelatedItemsSpecification, DISPLAYRELATEDITEMS_SPECIFICATION_XML_NODE_NAME);
    BeXmlNodeP xmlPropertyNode = xmlNode->SelectSingleNode(CALCULATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
    if (xmlPropertyNode)
        CommonTools::LoadSpecificationsFromXmlNode<CalculatedPropertiesSpecification, CalculatedPropertiesSpecificationList> (xmlPropertyNode, m_calculatedPropertiesSpecification, CALCULATED_PROPERTIES_SPECIFICATION_XML_CHILD_NAME);
    xmlPropertyNode = xmlNode->SelectSingleNode(PROPERTY_EDITORS_SPECIFICATION_XML_NODE_NAME);
    if (xmlPropertyNode)
        CommonTools::LoadSpecificationsFromXmlNode<PropertyEditorsSpecification, PropertyEditorsSpecificationList>(xmlPropertyNode, m_propertyEditorsSpecification, PROPERTY_EDITORS_SPECIFICATION_XML_CHILD_NAME);

    return _ReadXml (xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentSpecification::WriteXml (BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP specificationNode = parentXmlNode->AddEmptyElement (_GetXmlElementName ());

    specificationNode->AddAttributeInt32Value(COMMON_XML_ATTRIBUTE_PRIORITY, m_priority);
    specificationNode->AddAttributeBooleanValue(CONTENT_SPECIFICATION_XML_ATTRIBUTE_SHOWIMAGES, m_showImages);

    CommonTools::WriteRulesToXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList> (specificationNode, m_relatedPropertiesSpecification);
    CommonTools::WriteRulesToXmlNode<PropertiesDisplaySpecification, PropertiesDisplaySpecificationList> (specificationNode, m_propertiesDisplaySpecification);
    CommonTools::WriteRulesToXmlNode<DisplayRelatedItemsSpecification, DisplayRelatedItemsSpecificationList> (specificationNode, m_displayRelatedItemsSpecification);
    if (!m_calculatedPropertiesSpecification.empty())
        {
        BeXmlNodeP calculatedPropertiesNode = specificationNode->AddEmptyElement(CALCULATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
        CommonTools::WriteRulesToXmlNode<CalculatedPropertiesSpecification, CalculatedPropertiesSpecificationList>(calculatedPropertiesNode, m_calculatedPropertiesSpecification);
        }
    if (!m_propertyEditorsSpecification.empty())
        {
        BeXmlNodeP propertyEditorsNode = specificationNode->AddEmptyElement(PROPERTY_EDITORS_SPECIFICATION_XML_NODE_NAME);
        CommonTools::WriteRulesToXmlNode<PropertyEditorsSpecification, PropertyEditorsSpecificationList>(propertyEditorsNode, m_propertyEditorsSpecification);
        }

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
RelatedPropertiesSpecificationList& ContentSpecification::GetRelatedPropertiesR() {return m_relatedPropertiesSpecification;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecificationList const& ContentSpecification::GetRelatedProperties() const {return m_relatedPropertiesSpecification;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayRelatedItemsSpecificationList& ContentSpecification::GetDisplayRelatedItems (void) { return m_displayRelatedItemsSpecification; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrew.Menzies                          07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayRelatedItemsSpecificationList const& ContentSpecification::GetDisplayRelatedItems(void) const { return m_displayRelatedItemsSpecification; }
