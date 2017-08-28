/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/ContentModifier.cpp $
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentModifier::ContentModifier() : m_schemaName(""), m_className("") {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentModifier::ContentModifier(Utf8String schemaName, Utf8String className)
    : m_schemaName(schemaName), m_className(className)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentModifier::ContentModifier(ContentModifierCR other)
    : m_className(other.m_className), m_schemaName(other.m_schemaName)
    {
    CommonTools::CopyRules(m_relatedProperties, other.m_relatedProperties);
    CommonTools::CopyRules(m_propertiesDisplaySpecification, other.m_propertiesDisplaySpecification);
    CommonTools::CopyRules(m_calculatedProperties, other.m_calculatedProperties);
    CommonTools::CopyRules(m_propertyEditors, other.m_propertyEditors);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentModifier::~ContentModifier()
    {
    CommonTools::FreePresentationRules(m_relatedProperties);
    CommonTools::FreePresentationRules(m_propertiesDisplaySpecification);
    CommonTools::FreePresentationRules(m_calculatedProperties);
    CommonTools::FreePresentationRules(m_propertyEditors);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CharCP ContentModifier::_GetXmlElementName() const {return CONTENTMODIEFIER_XML_NODE_NAME;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentModifier::_ReadXml(BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_schemaName, CONTENTMODIEFIER_XML_ATTRIBUTE_SCHEMANAME))
        m_schemaName = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue(m_className, CONTENTMODIEFIER_XML_ATTRIBUTE_CLASSNAME))
        m_className = "";

    CommonTools::LoadSpecificationsFromXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList>(xmlNode, m_relatedProperties, RELATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
    CommonTools::LoadSpecificationsFromXmlNode<PropertiesDisplaySpecification, PropertiesDisplaySpecificationList>(xmlNode, m_propertiesDisplaySpecification, HIDDEN_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
    CommonTools::LoadSpecificationsFromXmlNode<PropertiesDisplaySpecification, PropertiesDisplaySpecificationList>(xmlNode, m_propertiesDisplaySpecification, DISPLAYED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
    BeXmlNodeP xmlPropertyNode = xmlNode->SelectSingleNode(CALCULATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
    if (xmlPropertyNode)
        CommonTools::LoadSpecificationsFromXmlNode<CalculatedPropertiesSpecification, CalculatedPropertiesSpecificationList>(xmlPropertyNode, m_calculatedProperties, CALCULATED_PROPERTIES_SPECIFICATION_XML_CHILD_NAME);
    xmlPropertyNode = xmlNode->SelectSingleNode(PROPERTY_EDITORS_SPECIFICATION_XML_NODE_NAME);
    if (xmlPropertyNode)
        CommonTools::LoadSpecificationsFromXmlNode<PropertyEditorsSpecification, PropertyEditorsSpecificationList>(xmlPropertyNode, m_propertyEditors, PROPERTY_EDITORS_SPECIFICATION_XML_CHILD_NAME);
    
    return true;
    }   

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifier::_WriteXml(BeXmlNodeP xmlNode) const
    {
    xmlNode->AddAttributeStringValue(CONTENTMODIEFIER_XML_ATTRIBUTE_CLASSNAME, m_className.c_str());
    xmlNode->AddAttributeStringValue(CONTENTMODIEFIER_XML_ATTRIBUTE_SCHEMANAME, m_schemaName.c_str());

    CommonTools::WriteRulesToXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList>(xmlNode, m_relatedProperties);
    CommonTools::WriteRulesToXmlNode<PropertiesDisplaySpecification, PropertiesDisplaySpecificationList>(xmlNode, m_propertiesDisplaySpecification);
    if (!m_calculatedProperties.empty())
        {
        BeXmlNodeP calculatedPropertiesNode = xmlNode->AddEmptyElement(CALCULATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
        CommonTools::WriteRulesToXmlNode<CalculatedPropertiesSpecification, CalculatedPropertiesSpecificationList>(calculatedPropertiesNode, m_calculatedProperties);
        }
    if (!m_propertyEditors.empty())
        {
        BeXmlNodeP propertyEditorsNode = xmlNode->AddEmptyElement(PROPERTY_EDITORS_SPECIFICATION_XML_NODE_NAME);
        CommonTools::WriteRulesToXmlNode<PropertyEditorsSpecification, PropertyEditorsSpecificationList>(propertyEditorsNode, m_propertyEditors);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ContentModifier::GetClassName() const {return m_className;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ContentModifier::GetSchemaName() const {return m_schemaName;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecificationList const& ContentModifier::GetRelatedProperties() const {return m_relatedProperties;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecificationList&  ContentModifier::GetRelatedPropertiesR()  {return m_relatedProperties;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                 07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PropertiesDisplaySpecificationList const& ContentModifier::GetPropertiesDisplaySpecifications() const {return m_propertiesDisplaySpecification;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                 07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PropertiesDisplaySpecificationList&  ContentModifier::GetPropertiesDisplaySpecificationsR() {return m_propertiesDisplaySpecification;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CalculatedPropertiesSpecificationList const& ContentModifier::GetCalculatedProperties() const {return m_calculatedProperties;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CalculatedPropertiesSpecificationList& ContentModifier::GetCalculatedPropertiesR() {return m_calculatedProperties;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                 07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyEditorsSpecificationList const& ContentModifier::GetPropertyEditors() const {return m_propertyEditors;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                 07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyEditorsSpecificationList& ContentModifier::GetPropertyEditorsR() {return m_propertyEditors;}

