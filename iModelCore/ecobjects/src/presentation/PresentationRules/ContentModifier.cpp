/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/ContentModifier.cpp $
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentModifier::ContentModifier() : m_schemaName(""), m_className("") {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentModifier::ContentModifier(Utf8String schemaName, Utf8String className)
    : m_schemaName(schemaName), m_className(className), m_relatedProperties(), m_hiddenProperties(), m_calculatedProperties() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentModifier::ContentModifier(ContentModifierCR other)
    : CustomizationRule(other), m_className(other.m_className), m_schemaName(other.m_schemaName)
    {
    CommonTools::CopyRules(m_relatedProperties, other.m_relatedProperties);
    CommonTools::CopyRules(m_hiddenProperties, other.m_hiddenProperties);
    CommonTools::CopyRules(m_calculatedProperties, other.m_calculatedProperties);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentModifier::~ContentModifier()
    {
    CommonTools::FreePresentationRules(m_relatedProperties);
    CommonTools::FreePresentationRules(m_hiddenProperties);
    CommonTools::FreePresentationRules(m_calculatedProperties);
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
      CommonTools::LoadSpecificationsFromXmlNode<HiddenPropertiesSpecification, HiddenPropertiesSpecificationList>(xmlNode, m_hiddenProperties, HIDDEN_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
      BeXmlNodeP xmlPropertyNode = xmlNode->SelectSingleNode(CALCULATED_PROPERTIES_SPECIFICATION_XML_CHILD_NAME);
      if (xmlPropertyNode)
          CommonTools::LoadSpecificationsFromXmlNode<CalculatedPropertiesSpecification, CalculatedPropertiesSpecificationList>(xmlPropertyNode, m_calculatedProperties, CALCULATED_PROPERTIES_SPECIFICATION_XML_CHILD_NAME);

      return PresentationRule::_ReadXml(xmlNode);
    }   

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifier::_WriteXml(BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP contentModifierNode = parentXmlNode->AddEmptyElement(CONTENTMODIEFIER_XML_NODE_NAME);
    contentModifierNode->AddAttributeStringValue(CONTENTMODIEFIER_XML_ATTRIBUTE_CLASSNAME, m_className.c_str());
    contentModifierNode->AddAttributeStringValue(CONTENTMODIEFIER_XML_ATTRIBUTE_SCHEMANAME, m_schemaName.c_str());

    CommonTools::WriteRulesToXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList>(contentModifierNode, m_relatedProperties);
    CommonTools::WriteRulesToXmlNode<HiddenPropertiesSpecification, HiddenPropertiesSpecificationList>(contentModifierNode, m_hiddenProperties);
    if (!m_calculatedProperties.empty())
        {
        BeXmlNodeP calculatedPropertiesNode = contentModifierNode->AddEmptyElement(CALCULATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);
        CommonTools::WriteRulesToXmlNode<CalculatedPropertiesSpecification, CalculatedPropertiesSpecificationList>(calculatedPropertiesNode, m_calculatedProperties);
        }

    PresentationRule::_WriteXml(contentModifierNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool ContentModifier::ReadXml(BeXmlNodeP xmlNode){return _ReadXml(xmlNode);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentModifier::WriteXml(BeXmlNodeP parentXmlNode) const {_WriteXml(parentXmlNode);}

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
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
HiddenPropertiesSpecificationList const& ContentModifier::GetHiddenProperties() const {return m_hiddenProperties;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
HiddenPropertiesSpecificationList&  ContentModifier::GetHiddenPropertiesR() {return m_hiddenProperties;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CalculatedPropertiesSpecificationList const& ContentModifier::GetCalculatedProperties() const {return m_calculatedProperties;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                 05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CalculatedPropertiesSpecificationList& ContentModifier::GetCalculatedPropertiesR() {return m_calculatedProperties;}

