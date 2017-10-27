/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/RelatedPropertiesSpecification.cpp $
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleXmlConstants.h"
#include <ECPresentation/RulesDriven/Rules/CommonTools.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecification::RelatedPropertiesSpecification ()
    : m_requiredDirection (RequiredRelationDirection_Both), m_relationshipClassNames (""), m_relatedClassNames (""), m_propertyNames (""),
    m_relationshipMeaning(RelationshipMeaning::RelatedInstance)
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
Utf8String                 propertyNames,
RelationshipMeaning        relationshipMeaning
) : m_requiredDirection (requiredDirection), 
    m_relationshipClassNames (relationshipClassNames),
    m_relatedClassNames (relatedClassNames),
    m_propertyNames (propertyNames),
    m_relationshipMeaning (relationshipMeaning)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecification::RelatedPropertiesSpecification(RelatedPropertiesSpecification const& other)
    : m_requiredDirection(other.m_requiredDirection), m_relationshipClassNames(other.m_relationshipClassNames), 
    m_relatedClassNames(other.m_relatedClassNames), m_propertyNames(other.m_propertyNames), m_relationshipMeaning(other.m_relationshipMeaning)
    {
    CommonTools::CopyRules(m_nestedRelatedPropertiesSpecification, other.m_nestedRelatedPropertiesSpecification);
    }

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

    Utf8String relationshipMeaningString = "";
    if (BEXML_Success != xmlNode->GetAttributeStringValue(relationshipMeaningString, COMMON_XML_ATTRIBUTE_RELATIONSHIPMEANING))
        relationshipMeaningString = "";
    else
        m_relationshipMeaning = CommonTools::ParseRelationshipMeaningString(relationshipMeaningString.c_str());


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
    relatedPropertiesNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_RELATIONSHIPMEANING, CommonTools::FormatRelationshipMeaningString(m_relationshipMeaning));

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
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedPropertiesSpecification::SetRelatedClassNames(Utf8StringCR value) {m_relatedClassNames = value;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR RelatedPropertiesSpecification::GetPropertyNames (void) const { return m_propertyNames; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedPropertiesSpecification::SetPropertyNames(Utf8StringCR value) {m_propertyNames = value;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecificationList const& RelatedPropertiesSpecification::GetNestedRelatedProperties() const { return m_nestedRelatedPropertiesSpecification; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedPropertiesSpecification::AddNestedRelatedProperty(RelatedPropertiesSpecificationR specification)
    {
    InvalidateHash();
    specification.SetParent(this);
    m_nestedRelatedPropertiesSpecification.push_back(&specification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipMeaning RelatedPropertiesSpecification::GetRelationshipMeaning() const { return m_relationshipMeaning; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedPropertiesSpecification::SetRelationshipMeaning(RelationshipMeaning value) { m_relationshipMeaning = value; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 RelatedPropertiesSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5;
    md5.Add(&m_requiredDirection, sizeof(m_requiredDirection));
    md5.Add(m_relationshipClassNames.c_str(), m_relationshipClassNames.size());
    md5.Add(m_relatedClassNames.c_str(), m_relatedClassNames.size());
    md5.Add(m_propertyNames.c_str(), m_propertyNames.size());
    md5.Add(&m_relationshipMeaning, sizeof(m_relationshipMeaning));
    if (nullptr != parentHash)
        md5.Add(parentHash, strlen(parentHash));

    Utf8String currentHash = md5.GetHashString();
    for (RelatedPropertiesSpecificationP spec : m_nestedRelatedPropertiesSpecification)
        {
        Utf8StringCR specHash = spec->GetHash(currentHash.c_str());
        md5.Add(specHash.c_str(), specHash.size());
        }
    return md5;
    }
