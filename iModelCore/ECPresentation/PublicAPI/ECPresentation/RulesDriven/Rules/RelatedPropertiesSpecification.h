/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

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
    IncludedProperties m_includedProperties;
    PropertySpecificationsList m_properties;
    RelatedPropertiesSpecificationList m_nestedRelatedPropertiesSpecification;
    RelationshipMeaning m_relationshipMeaning;
    bool m_polymorphic;
    bool m_autoExpand;

protected:
    //! Computes specification hash.
    ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const;
        
public:
    //! Constructor. It is used to initialize the rule with default settings.
    ECPRESENTATION_EXPORT RelatedPropertiesSpecification();
        
    //! Copy constructor.
    ECPRESENTATION_EXPORT RelatedPropertiesSpecification(RelatedPropertiesSpecification const&);

    //! Constructor.
    //! @deprecated
    ECPRESENTATION_EXPORT RelatedPropertiesSpecification(RequiredRelationDirection requiredDirection,
        Utf8String relationshipClassNames, Utf8String relatedClassNames, Utf8String propertyNames,
        RelationshipMeaning relationshipMeaning, bool polymorphic = false, bool autoExpand = false);

    //! Constructor.
    ECPRESENTATION_EXPORT RelatedPropertiesSpecification(RequiredRelationDirection requiredDirection,
        Utf8String relationshipClassNames, Utf8String relatedClassNames, PropertySpecificationsList properties,
        RelationshipMeaning relationshipMeaning, bool polymorphic = false, bool autoExpand = false);

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
    ECPRESENTATION_EXPORT RequiredRelationDirection    GetRequiredRelationDirection (void) const;

    //! Relationship class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
    ECPRESENTATION_EXPORT Utf8StringCR                 GetRelationshipClassNames (void) const;

    //! Related class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
    ECPRESENTATION_EXPORT Utf8StringCR                 GetRelatedClassNames (void) const;

    //! Set related class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
    ECPRESENTATION_EXPORT void                         SetRelatedClassNames(Utf8StringCR);

    //! Property names separated by comma. RelatedClasses are required if related properties are specified.
    PropertySpecificationsList const& GetProperties() const {return m_properties;}
    ECPRESENTATION_EXPORT void AddProperty(PropertySpecificationR spec);

    bool GetIncludeAllProperties() const {return m_includedProperties == IncludedProperties::All;}
    ECPRESENTATION_EXPORT void SetIncludeAllProperties();

    bool GetIncludeNoProperties() const {return m_includedProperties == IncludedProperties::None;}
    ECPRESENTATION_EXPORT void SetIncludeNoProperties();

    //! Meaning of properties returned by this specification.
    ECPRESENTATION_EXPORT RelationshipMeaning          GetRelationshipMeaning(void) const;

    //! Set relationship meaning.
    ECPRESENTATION_EXPORT void                         SetRelationshipMeaning(RelationshipMeaning);
        
    //! Should related classes be handled polymorphically
    bool IsPolymorphic() const {return m_polymorphic;}

    //! Set whether related classes should be handled polymorphically
    void SetIsPolymorphic(bool value) {m_polymorphic = value;}

    //! Nested related properties, that will be shown next to ECInstance proerties (the same row for example).
    ECPRESENTATION_EXPORT RelatedPropertiesSpecificationList const& GetNestedRelatedProperties() const;
        
    //! Add nested related property.
    ECPRESENTATION_EXPORT void AddNestedRelatedProperty(RelatedPropertiesSpecificationR specification);

    //! Should properties be automatically expand
    bool ShouldAutoExpand() const {return m_autoExpand;}

    //! Set whether properties should be automatically expanded
    void SetAutoExpand(bool value) {m_autoExpand = value;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
