/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/ContentSpecification.h>
#include <ECPresentation/RulesDriven/Rules/RelatedInstanceNodesSpecification.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define INCLUDE_NO_PROPERTIES_SPEC "_none_"
#define INCLUDE_ALL_PROPERTIES_SPEC "*"

/*---------------------------------------------------------------------------------**//**
Specifies how should related instance properties appear in presentation controls.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
enum class RelationshipMeaning
    {
    //! Properties belong to the same instance.
    SameInstance = 0,
    //! Properties belong to the related instance.
    RelatedInstance = 1,
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
enum class IncludedProperties
    {
    //! No properties are included
    None,
    //! All properties are included
    All,
    // Specific properties are included
    Specified,
    };

/*---------------------------------------------------------------------------------**//**
Related properties specification. It allow to extend a content ECQuery to include
properties of related classes.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelatedPropertiesSpecification : HashableBase
{
private:
    RequiredRelationDirection m_requiredDirection;
    Utf8String m_relationshipClassNames;
    Utf8String m_relatedClassNames;
    RelationshipPathSpecification* m_propertiesSourceSpecification;
    IncludedProperties m_includedProperties;
    PropertySpecificationsList m_properties;
    RelatedPropertiesSpecificationList m_nestedRelatedPropertiesSpecification;
    RelationshipMeaning m_relationshipMeaning;
    bool m_polymorphic;
    bool m_autoExpand;

protected:
    //! Computes specification hash.
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const;

public:
    ECPRESENTATION_EXPORT RelatedPropertiesSpecification();
    ECPRESENTATION_EXPORT RelatedPropertiesSpecification(RelatedPropertiesSpecification const&);
    ECPRESENTATION_EXPORT RelatedPropertiesSpecification(RelatedPropertiesSpecification&&);
    ECPRESENTATION_EXPORT RelatedPropertiesSpecification(RequiredRelationDirection requiredDirection,
        Utf8String relationshipClassNames, Utf8String relatedClassNames, Utf8String propertyNames,
        RelationshipMeaning relationshipMeaning, bool polymorphic = false, bool autoExpand = false);
    ECPRESENTATION_EXPORT RelatedPropertiesSpecification(RequiredRelationDirection requiredDirection,
        Utf8String relationshipClassNames, Utf8String relatedClassNames, PropertySpecificationsList properties,
        RelationshipMeaning relationshipMeaning, bool polymorphic = false, bool autoExpand = false);
    RelatedPropertiesSpecification(RelationshipPathSpecification& propertiesSource, PropertySpecificationsList properties,
        RelationshipMeaning relationshipMeaning, bool polymorphic = false, bool autoExpand = false)
        : m_propertiesSourceSpecification(&propertiesSource), m_properties(properties), m_relationshipMeaning(relationshipMeaning),
        m_polymorphic(polymorphic), m_autoExpand(autoExpand), m_requiredDirection(RequiredRelationDirection_Both),
        m_includedProperties(properties.empty() ? IncludedProperties::All : IncludedProperties::Specified)
        {}

    //! Destructor.
    ECPRESENTATION_EXPORT ~RelatedPropertiesSpecification (void);

    //! Reads specification from XML.
    ECPRESENTATION_EXPORT bool ReadXml (BeXmlNodeP xmlNode);

    //! Writes related properties to xml node.
    ECPRESENTATION_EXPORT void WriteXml (BeXmlNodeP parentXmlNode) const;

    //! Reads rule information from Json, returns true if it can read it successfully.
    ECPRESENTATION_EXPORT bool ReadJson(JsonValueCR json);

    //! Writes rule information to json.
    ECPRESENTATION_EXPORT Json::Value WriteJson() const;

    //! Returns direction of relationship that should be selected in the query.
    RequiredRelationDirection GetRequiredRelationDirection() const { return m_requiredDirection; }
    void SetRequiredRelationDirection(RequiredRelationDirection value) { m_requiredDirection = value; }

    //! Relationship class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
    Utf8StringCR GetRelationshipClassNames() const { return m_relationshipClassNames; }
    void SetRelationshipClassNames(Utf8String value) { m_relationshipClassNames = value; }

    //! Related class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
    Utf8StringCR GetRelatedClassNames() const { return m_relatedClassNames; }
    void SetRelatedClassNames(Utf8String value) { m_relatedClassNames = value; }

    //! Get specification of the relationship path to the related instance
    RelationshipPathSpecification const* GetPropertiesSource() const { return m_propertiesSourceSpecification; }
    RelationshipPathSpecification* GetPropertiesSource() { return m_propertiesSourceSpecification; }
    void SetPropertiesSource(RelationshipPathSpecification* spec) { DELETE_AND_CLEAR(m_propertiesSourceSpecification); m_propertiesSourceSpecification = spec; }

    //! Property names separated by comma. RelatedClasses are required if related properties are specified.
    PropertySpecificationsList const& GetProperties() const {return m_properties;}
    ECPRESENTATION_EXPORT void AddProperty(PropertySpecificationR spec);

    bool GetIncludeAllProperties() const {return m_includedProperties == IncludedProperties::All;}
    ECPRESENTATION_EXPORT void SetIncludeAllProperties();

    bool GetIncludeNoProperties() const {return m_includedProperties == IncludedProperties::None;}
    ECPRESENTATION_EXPORT void SetIncludeNoProperties();

    //! Meaning of properties returned by this specification.
    RelationshipMeaning GetRelationshipMeaning() const {return m_relationshipMeaning;}
    void SetRelationshipMeaning(RelationshipMeaning value) {m_relationshipMeaning = value;}

    //! Should related classes be handled polymorphically
    bool IsPolymorphic() const {return m_polymorphic;}
    void SetIsPolymorphic(bool value) {m_polymorphic = value;}

    //! Nested related properties, that will be shown next to ECInstance proerties (the same row for example).
    ECPRESENTATION_EXPORT RelatedPropertiesSpecificationList const& GetNestedRelatedProperties() const;
    ECPRESENTATION_EXPORT void AddNestedRelatedProperty(RelatedPropertiesSpecificationR specification);

    //! Should properties be automatically expand
    bool ShouldAutoExpand() const {return m_autoExpand;}
    void SetAutoExpand(bool value) {m_autoExpand = value;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
