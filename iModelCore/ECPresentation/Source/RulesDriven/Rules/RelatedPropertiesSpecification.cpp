/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>

#include "PresentationRuleJsonConstants.h"
#include "PresentationRuleXmlConstants.h"
#include "CommonToolsInternal.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static PropertySpecificationP CreatePropertySpecFromPropertyName(Utf8StringCR name, HashableBase* parent)
    {
    if (name.empty())
        return nullptr;
    PropertySpecificationP spec = new PropertySpecification(name);
    spec->SetParent(parent);
    return spec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static PropertySpecificationsList CreatePropertySpecsFromPropertyNames(Utf8StringCR namesStr, HashableBase* parent)
    {
    PropertySpecificationsList list;
    if (namesStr.EqualsI(INCLUDE_NO_PROPERTIES_SPEC))
        return list;

    bvector<Utf8String> names;
    BeStringUtilities::Split(namesStr.c_str(), ",", names);
    for (Utf8StringCR name : names)
        {
        PropertySpecificationP spec = CreatePropertySpecFromPropertyName(name, parent);
        if (nullptr != spec)
            list.push_back(spec);
        }
    return list;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetPropertyNamesStr(IncludedProperties included, PropertySpecificationsList const& properties)
    {
    switch (included)
        {
        case IncludedProperties::None: return INCLUDE_NO_PROPERTIES_SPEC;
        case IncludedProperties::All: return "";
        }
    Utf8String names;
    for (PropertySpecificationCP spec : properties)
        {
        if (!names.empty())
            names.append(",");
        names.append(spec->GetPropertyName());
        }
    return names;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecification::RelatedPropertiesSpecification ()
    : m_requiredDirection (RequiredRelationDirection_Both), m_relationshipMeaning(RelationshipMeaning::RelatedInstance), m_polymorphic(false), m_autoExpand(false),
    m_includedProperties(IncludedProperties::All), m_propertiesSourceSpecification(nullptr)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecification::RelatedPropertiesSpecification(RequiredRelationDirection requiredDirection, Utf8String relationshipClassNames, 
    Utf8String relatedClassNames, Utf8String propertyNames, RelationshipMeaning relationshipMeaning, bool polymorphic, bool autoExpand) 
    : m_requiredDirection (requiredDirection), m_relationshipClassNames (relationshipClassNames), m_relatedClassNames (relatedClassNames),
    m_relationshipMeaning (relationshipMeaning), m_polymorphic(polymorphic), m_autoExpand(autoExpand), m_propertiesSourceSpecification(nullptr)
    {
    m_includedProperties = propertyNames.empty() ? IncludedProperties::All
        : propertyNames.EqualsI(INCLUDE_NO_PROPERTIES_SPEC) ? IncludedProperties::None : IncludedProperties::Specified;
    m_properties = CreatePropertySpecsFromPropertyNames(propertyNames, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecification::RelatedPropertiesSpecification(RequiredRelationDirection requiredDirection, Utf8String relationshipClassNames, 
    Utf8String relatedClassNames, PropertySpecificationsList properties, RelationshipMeaning relationshipMeaning, bool polymorphic, bool autoExpand) 
    : m_requiredDirection(requiredDirection), m_relationshipClassNames(relationshipClassNames), m_relatedClassNames(relatedClassNames), 
    m_relationshipMeaning(relationshipMeaning), m_properties(properties), m_polymorphic(polymorphic), m_autoExpand(autoExpand), m_propertiesSourceSpecification(nullptr)
    {
    m_includedProperties = properties.empty() ? IncludedProperties::All : IncludedProperties::Specified;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecification::RelatedPropertiesSpecification(RelatedPropertiesSpecification const& other)
    : m_requiredDirection(other.m_requiredDirection), m_relationshipClassNames(other.m_relationshipClassNames), 
    m_relatedClassNames(other.m_relatedClassNames), m_includedProperties(other.m_includedProperties),
    m_relationshipMeaning(other.m_relationshipMeaning), m_polymorphic(other.m_polymorphic), m_autoExpand(other.m_autoExpand), 
    m_propertiesSourceSpecification(nullptr)
    {
    CommonToolsInternal::CopyRules(m_properties, other.m_properties, this);
    CommonToolsInternal::CopyRules(m_nestedRelatedPropertiesSpecification, other.m_nestedRelatedPropertiesSpecification, this);
    if (nullptr != other.m_propertiesSourceSpecification)
        m_propertiesSourceSpecification = new RelationshipPathSpecification(*other.m_propertiesSourceSpecification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecification::RelatedPropertiesSpecification(RelatedPropertiesSpecification&& other)
    : m_requiredDirection(other.m_requiredDirection), m_relationshipClassNames(std::move(other.m_relationshipClassNames)),
    m_relatedClassNames(std::move(other.m_relatedClassNames)), m_includedProperties(other.m_includedProperties),
    m_relationshipMeaning(other.m_relationshipMeaning), m_polymorphic(other.m_polymorphic), m_autoExpand(other.m_autoExpand)
    {
    m_properties.swap(other.m_properties);
    m_nestedRelatedPropertiesSpecification.swap(other.m_nestedRelatedPropertiesSpecification);
    m_propertiesSourceSpecification = other.m_propertiesSourceSpecification;
    other.m_propertiesSourceSpecification = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
RelatedPropertiesSpecification::~RelatedPropertiesSpecification ()
    {
    CommonToolsInternal::FreePresentationRules(m_properties);
    CommonToolsInternal::FreePresentationRules(m_nestedRelatedPropertiesSpecification);
    DELETE_AND_CLEAR(m_propertiesSourceSpecification);
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

    Utf8String propertyNames;
    if (BEXML_Success == xmlNode->GetAttributeStringValue(propertyNames, COMMON_XML_ATTRIBUTE_PROPERTYNAMES))
        {
        m_includedProperties = propertyNames.empty() ? IncludedProperties::All
            : propertyNames.EqualsI(INCLUDE_NO_PROPERTIES_SPEC) ? IncludedProperties::None : IncludedProperties::Specified;
        m_properties = CreatePropertySpecsFromPropertyNames(propertyNames, this);
        }    

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

    if (BEXML_Success != xmlNode->GetAttributeBooleanValue(m_autoExpand, COMMON_XML_ATTRIBUTE_AUTOEXPAND))
        m_autoExpand = false;

    CommonToolsInternal::LoadSpecificationsFromXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList> (xmlNode, m_nestedRelatedPropertiesSpecification, RELATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME, this);

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
    relatedPropertiesNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_PROPERTYNAMES, GetPropertyNamesStr(m_includedProperties, m_properties).c_str());
    relatedPropertiesNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION, CommonToolsInternal::FormatRequiredDirectionString (m_requiredDirection));
    relatedPropertiesNode->AddAttributeStringValue (COMMON_XML_ATTRIBUTE_RELATIONSHIPMEANING, CommonToolsInternal::FormatRelationshipMeaningString(m_relationshipMeaning));
    relatedPropertiesNode->AddAttributeBooleanValue(COMMON_XML_ATTRIBUTE_ISPOLYMORPHIC, m_polymorphic);
    relatedPropertiesNode->AddAttributeBooleanValue(COMMON_XML_ATTRIBUTE_AUTOEXPAND, m_autoExpand);

    CommonToolsInternal::WriteRulesToXmlNode<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList> (relatedPropertiesNode, m_nestedRelatedPropertiesSpecification);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Kilinskas                  04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelatedPropertiesSpecification::ReadJson(JsonValueCR json)
    {
    if (json.isMember(RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIESSOURCE))
        {
        m_propertiesSourceSpecification = CommonToolsInternal::LoadRuleFromJson<RelationshipPathSpecification>(json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIESSOURCE]);
        if (!m_propertiesSourceSpecification)
            return false;
        }
    else
        {
        m_relationshipClassNames = CommonToolsInternal::SchemaAndClassNamesToString(json[COMMON_JSON_ATTRIBUTE_RELATIONSHIPS]);
        m_relatedClassNames = CommonToolsInternal::SchemaAndClassNamesToString(json[COMMON_JSON_ATTRIBUTE_RELATEDCLASSES]);
        m_requiredDirection = CommonToolsInternal::ParseRequiredDirectionString(json[COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION].asCString(""));
        }

    m_polymorphic = json[COMMON_JSON_ATTRIBUTE_ISPOLYMORPHIC].asBool(false);
    m_autoExpand = json[COMMON_JSON_ATTRIBUTE_AUTOEXPAND].asBool(false);
    m_relationshipMeaning = CommonToolsInternal::ParseRelationshipMeaningString(json[COMMON_JSON_ATTRIBUTE_RELATIONSHIPMEANING].asCString(""));
    
    JsonValueCR propertyNamesJson = json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAMES];
    JsonValueCR propertySpecsJson = json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIES];
    if (propertySpecsJson.isString() && 0 == BeStringUtilities::Stricmp(propertySpecsJson.asCString(), INCLUDE_NO_PROPERTIES_SPEC)
        || propertyNamesJson.isString() && 0 == BeStringUtilities::Stricmp(propertyNamesJson.asCString(), INCLUDE_NO_PROPERTIES_SPEC))
        {
        m_includedProperties = IncludedProperties::None;
        m_properties.clear();
        }
    else if (propertySpecsJson.isString() && BeStringUtilities::Stricmp(propertySpecsJson.asCString(), INCLUDE_ALL_PROPERTIES_SPEC)
        || propertyNamesJson.isString() && 0 == BeStringUtilities::Stricmp(propertyNamesJson.asCString(), INCLUDE_ALL_PROPERTIES_SPEC))
        {
        m_includedProperties = IncludedProperties::All;
        m_properties.clear();
        }
    else if (propertySpecsJson.isArray() && !propertySpecsJson.empty())
        {
        m_includedProperties = IncludedProperties::Specified;
        for (Json::ArrayIndex i = 0; i < propertySpecsJson.size(); ++i)
            {
            PropertySpecificationP spec = propertySpecsJson[i].isObject() ? CommonToolsInternal::LoadRuleFromJson<PropertySpecification>(propertySpecsJson[i])
                : propertySpecsJson[i].isString() ? CreatePropertySpecFromPropertyName(propertySpecsJson[i].asCString(), this) : nullptr;
            if (nullptr != spec)
                m_properties.push_back(spec);
            }
        }
    else if (propertyNamesJson.isArray() && !propertyNamesJson.empty())
        {
        m_includedProperties = IncludedProperties::Specified;
        for (Json::ArrayIndex i = 0; i < propertyNamesJson.size(); ++i)
            {
            PropertySpecificationP spec = CreatePropertySpecFromPropertyName(propertyNamesJson[i].asCString(), this);
            if (nullptr != spec)
                m_properties.push_back(spec);
            }
        }
    else
        {
        m_includedProperties = IncludedProperties::All;
        }

    CommonToolsInternal::LoadFromJson(json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_NESTEDRELATEDPROPERTIES], 
        m_nestedRelatedPropertiesSpecification, CommonToolsInternal::LoadRuleFromJson<RelatedPropertiesSpecification>, this);
    
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
    if (m_autoExpand)
        json[COMMON_JSON_ATTRIBUTE_AUTOEXPAND] = m_autoExpand;
    if (RelationshipMeaning::RelatedInstance != m_relationshipMeaning)
        json[COMMON_JSON_ATTRIBUTE_RELATIONSHIPMEANING] = CommonToolsInternal::FormatRelationshipMeaningString(m_relationshipMeaning);
    if (RequiredRelationDirection_Both != m_requiredDirection)
        json[COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION] = CommonToolsInternal::FormatRequiredDirectionString(m_requiredDirection);
    if (!m_relationshipClassNames.empty())
        json[COMMON_JSON_ATTRIBUTE_RELATIONSHIPS] = CommonToolsInternal::SchemaAndClassNamesToJson(m_relationshipClassNames);
    if (!m_relatedClassNames.empty())
        json[COMMON_JSON_ATTRIBUTE_RELATEDCLASSES] = CommonToolsInternal::SchemaAndClassNamesToJson(m_relatedClassNames);
    if (m_propertiesSourceSpecification)
        json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIESSOURCE] = m_propertiesSourceSpecification->WriteJson();

    if (m_includedProperties == IncludedProperties::None)
        {
        json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIES] = INCLUDE_NO_PROPERTIES_SPEC;
        }
    else if (m_includedProperties == IncludedProperties::Specified)
        {
        CommonToolsInternal::WriteRulesToJson<PropertySpecification, PropertySpecificationsList>
            (json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIES], m_properties);
        }

    if (!m_nestedRelatedPropertiesSpecification.empty())
        {
        CommonToolsInternal::WriteRulesToJson<RelatedPropertiesSpecification, RelatedPropertiesSpecificationList>
            (json[RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_NESTEDRELATEDPROPERTIES], m_nestedRelatedPropertiesSpecification);
        }

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedPropertiesSpecification::AddProperty(PropertySpecificationR specification)
    {
    ADD_HASHABLE_CHILD(m_properties, specification);
    m_includedProperties = IncludedProperties::Specified;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedPropertiesSpecification::SetIncludeAllProperties()
    {
    InvalidateHash();
    m_properties.clear();
    m_includedProperties = IncludedProperties::All;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedPropertiesSpecification::SetIncludeNoProperties()
    {
    InvalidateHash();
    m_properties.clear();
    m_includedProperties = IncludedProperties::None;
    }

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
* @bsimethod                                    Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
MD5 RelatedPropertiesSpecification::_ComputeHash(Utf8CP parentHash) const
    {
    MD5 md5;

    if (nullptr != parentHash)
        md5.Add(parentHash, strlen(parentHash));

    md5.Add(&m_requiredDirection, sizeof(m_requiredDirection));
    md5.Add(m_relationshipClassNames.c_str(), m_relationshipClassNames.size());
    md5.Add(m_relatedClassNames.c_str(), m_relatedClassNames.size());
    md5.Add(&m_relationshipMeaning, sizeof(m_relationshipMeaning));
    md5.Add(&m_polymorphic, sizeof(m_polymorphic));
    md5.Add(&m_includedProperties, sizeof(m_includedProperties));

    Utf8String currentHash = md5.GetHashString();

    if (m_propertiesSourceSpecification)
        {
        Utf8StringCR specHash = m_propertiesSourceSpecification->GetHash(currentHash.c_str());
        md5.Add(specHash.c_str(), specHash.size());
        }

    for (PropertySpecificationP spec : m_properties)
        {
        Utf8StringCR specHash = spec->GetHash(currentHash.c_str());
        md5.Add(specHash.c_str(), specHash.size());
        }

    for (RelatedPropertiesSpecificationP spec : m_nestedRelatedPropertiesSpecification)
        {
        Utf8StringCR specHash = spec->GetHash(currentHash.c_str());
        md5.Add(specHash.c_str(), specHash.size());
        }

    return md5;
    }
