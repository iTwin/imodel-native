/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/RelatedPropertiesSpecification.cpp $
|
|   $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "PresentationRuleXmlConstants.h"
#include <ECPresentationRules/CommonTools.h>
#include <ECPresentationRules/PresentationRules.h>
#include <ECPresentationRules/SpecificationVisitor.h>

USING_NAMESPACE_BENTLEY_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecification::RelatedPropertiesSpecification ()
    : m_requiredDirection (RequiredRelationDirection_Both), m_relationshipClassNames (""), m_relatedClassNames (""), m_propertyNames ("")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecification::RelatedPropertiesSpecification 
(
RequiredRelationDirection  requiredDirection,
Utf8String                 relationshipClassNames,
Utf8String                 relatedClassNames,
Utf8String                 propertyNames
) : m_requiredDirection (requiredDirection), 
    m_relationshipClassNames (relationshipClassNames),
    m_relatedClassNames (relatedClassNames),
    m_propertyNames (propertyNames)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedPropertiesSpecification::_Accept(PresentationRuleSpecificationVisitor& visitor) const {visitor._Visit(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecification::~RelatedPropertiesSpecification ()
    {
    CommonTools::FreePresentationRules (m_nestedRelatedPropertiesSpecification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedPropertiesSpecification::ReadXml (BeXmlNodeP xmlNode)
    {
    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_relationshipClassNames, COMMON_XML_ATTRIBUTE_RELATIONSHIPCLASSNAMES))
        m_relationshipClassNames = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_relatedClassNames, COMMON_XML_ATTRIBUTE_RELATEDCLASSNAMES))
        m_relatedClassNames = "";

    if (BEXML_Success != xmlNode->GetAttributeStringValue (m_propertyNames, COMMON_XML_ATTRIBUTE_PROPERTYNAMES))
        m_propertyNames = "";

    Utf8String requiredDirectionString = "";
    if (BEXML_Success != xmlNode->GetAttributeStringValue (requiredDirectionString, COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION))
        requiredDirectionString = "";
    else
        m_requiredDirection = CommonTools::ParseRequiredDirectionString (requiredDirectionString.c_str ());

    CommonTools::LoadSpecificationsFromXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList> (xmlNode, m_nestedRelatedPropertiesSpecification, RELATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedPropertiesSpecification::WriteXml (BeXmlNodeP parentXmlNode) const
    {
    BeXmlNodeP relatedPropertiesNode = parentXmlNode->AddEmptyElement (RELATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);

    relatedPropertiesNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_RELATIONSHIPCLASSNAMES, m_relationshipClassNames.c_str ());
    relatedPropertiesNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_RELATEDCLASSNAMES, m_relatedClassNames.c_str ());
    relatedPropertiesNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_PROPERTYNAMES, m_propertyNames.c_str ());
    relatedPropertiesNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION, CommonTools::FormatRequiredDirectionString (m_requiredDirection));

    CommonTools::WriteRulesToXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList> (relatedPropertiesNode, m_nestedRelatedPropertiesSpecification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RequiredRelationDirection RelatedPropertiesSpecification::GetRequiredRelationDirection (void) const { return m_requiredDirection; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR RelatedPropertiesSpecification::GetRelationshipClassNames (void) const { return m_relationshipClassNames; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR RelatedPropertiesSpecification::GetRelatedClassNames (void) const { return m_relatedClassNames; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR RelatedPropertiesSpecification::GetPropertyNames (void) const { return m_propertyNames; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecificationList const& RelatedPropertiesSpecification::GetNestedRelatedProperties() const { return m_nestedRelatedPropertiesSpecification; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecificationList& RelatedPropertiesSpecification::GetNestedRelatedPropertiesR() { return m_nestedRelatedPropertiesSpecification; }
