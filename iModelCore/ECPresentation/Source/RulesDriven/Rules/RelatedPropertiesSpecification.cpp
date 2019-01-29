/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/RelatedPropertiesSpecification.cpp $
|
|   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecification::RelatedPropertiesSpecification ()
    : m_requiredDirection (RequiredRelationDirection_Both), m_relationshipMeaning(RelationshipMeaning::RelatedInstance), m_polymorphic(false)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecification::RelatedPropertiesSpecification 
(
RequiredRelationDirection  requiredDirection,
Utf8String                 relationshipClassNames,
Utf8String                 relatedClassNames,
Utf8String                 propertyNames,
RelationshipMeaning        relationshipMeaning,
bool                       polymorphic
) : m_requiredDirection (requiredDirection), 
    m_relationshipClassNames (relationshipClassNames),
    m_relatedClassNames (relatedClassNames),
    m_propertyNames (propertyNames),
    m_relationshipMeaning (relationshipMeaning),
    m_polymorphic(polymorphic)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecification::RelatedPropertiesSpecification(RelatedPropertiesSpecification const& other)
    : m_requiredDirection(other.m_requiredDirection), m_relationshipClassNames(other.m_relationshipClassNames), 
    m_relatedClassNames(other.m_relatedClassNames), m_propertyNames(other.m_propertyNames), 
    m_relationshipMeaning(other.m_relationshipMeaning), m_polymorphic(other.m_polymorphic)
    {
    CommonToolsInternal::CopyRules(m_nestedRelatedPropertiesSpecification, other.m_nestedRelatedPropertiesSpecification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecification::~RelatedPropertiesSpecification ()
    {
    CommonToolsInternal::FreePresentationRules (m_nestedRelatedPropertiesSpecification);
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
        m_requiredDirection = CommonToolsInternal::ParseRequiredDirectionString (requiredDirectionString.c_str ());

    Utf8String relationshipMeaningString = "";
    if (BEXML_Success != xmlNode->GetAttributeStringValue(relationshipMeaningString, COMMON_XML_ATTRIBUTE_RELATIONSHIPMEANING))
        relationshipMeaningString = "";
    else
        m_relationshipMeaning = CommonToolsInternal::ParseRelationshipMeaningString(relationshipMeaningString.c_str());
    
    if (BEXML_Success != xmlNode->GetAttributeBooleanValue(m_polymorphic, COMMON_XML_ATTRIBUTE_ISPOLYMORPHIC))
        m_polymorphic = false;

    CommonToolsInternal::LoadSpecificationsFromXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList> (xmlNode, m_nestedRelatedPropertiesSpecification, RELATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME);

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
    relatedPropertiesNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION, CommonToolsInternal::FormatRequiredDirectionString (m_requiredDirection));
    relatedPropertiesNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_RELATIONSHIPMEANING, CommonToolsInternal::FormatRelationshipMeaningString(m_relationshipMeaning));
    relatedPropertiesNode->AddAttributeBooleanValue(COMMON_XML_ATTRIBUTE_ISPOLYMORPHIC, m_polymorphic);

    CommonToolsInternal::WriteRulesToXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList> (relatedPropertiesNode, m_nestedRelatedPropertiesSpecification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedPropertiesSpecification::ReadJson(JsonValueCR json)
    {
    m_relationshipClassNames = CommonToolsInternal::SchemaAndClassNamesToString(json[COMMON_JSON_ATTRIBUTE_RELATIONSHIPS]);
    m_relatedClassNames = CommonToolsInternal::SchemaAndClassNamesToString(json[COMMON_JSON_ATTRIBUTE_RELATEDCLASSES]);
    m_polymorphic = json[COMMON_JSON_ATTRIBUTE_ISPOLYMORPHIC].asBool(false);
    m_relationshipMeaning = CommonToolsInternal::ParseRelationshipMeaningString(json[COMMON_JSON_ATTRIBUTE_RELATIONSHIPMEANING].asCString(""));
    m_requiredDirection = CommonToolsInternal::ParseRequiredDirectionString(json[COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION].asCString(""));
    
    JsonValueCR propertyNamesJson = json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAMES];
    if (propertyNamesJson.isString() && 0 == BeStringUtilities::Strnicmp(propertyNamesJson.asCString(), "_none_", 6))
        {
        m_propertyNames = propertyNamesJson.asCString();
        }
    else if (propertyNamesJson.isArray())
        {
        for (Json::ArrayIndex i = 0; i < propertyNamesJson.size(); ++i)
            {
            if (!m_propertyNames.empty())
                m_propertyNames.append(",");
            m_propertyNames.append(propertyNamesJson[i].asCString());
            }
        }

    CommonToolsInternal::LoadFromJson(json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_NESTEDRELATEDPROPERTIES], 
        m_nestedRelatedPropertiesSpecification, CommonToolsInternal::LoadRuleFromJson<RelatedPropertiesSpecification>);
    
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value RelatedPropertiesSpecification::WriteJson() const
    {
    Json::Value json(Json::objectValue);
    if (m_polymorphic)
        json[COMMON_JSON_ATTRIBUTE_ISPOLYMORPHIC] = m_polymorphic;
    if (RelationshipMeaning::RelatedInstance != m_relationshipMeaning)
        json[COMMON_JSON_ATTRIBUTE_RELATIONSHIPMEANING] = CommonToolsInternal::FormatRelationshipMeaningString(m_relationshipMeaning);
    if (RequiredRelationDirection_Both != m_requiredDirection)
        json[COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION] = CommonToolsInternal::FormatRequiredDirectionString(m_requiredDirection);
    if (!m_relationshipClassNames.empty())
        json[COMMON_JSON_ATTRIBUTE_RELATIONSHIPS] = CommonToolsInternal::SchemaAndClassNamesToJson(m_relationshipClassNames);
    if (!m_relatedClassNames.empty())
        json[COMMON_JSON_ATTRIBUTE_RELATEDCLASSES] = CommonToolsInternal::SchemaAndClassNamesToJson(m_relatedClassNames);

    if (m_propertyNames.EqualsI("_none_"))
        {
        json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAMES] = m_propertyNames;
        }
    else
        {
        bvector<Utf8String> propertyNames;
        BeStringUtilities::Split(m_propertyNames.c_str(), ",", propertyNames);
        for (Utf8StringR propertyName : propertyNames)
            json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAMES].append(propertyName.Trim());
        }

    if (!m_nestedRelatedPropertiesSpecification.empty())
        {
        CommonToolsInternal::WriteRulesToJson<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList>
            (json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_NESTEDRELATEDPROPERTIES], m_nestedRelatedPropertiesSpecification);
        }

    return json;
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
    md5.Add(&m_polymorphic, sizeof(m_polymorphic));
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
