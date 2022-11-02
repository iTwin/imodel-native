/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ECPresentation/Rules/ContentSpecification.h>
#include <ECPresentation/Rules/RelatedInstanceNodesSpecification.h>
#include <ECPresentation/Rules/PresentationRuleSet.h>

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
Related properties specification. It allow to extend a content ECQuery to include
properties of related classes.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelatedPropertiesSpecification : PresentationKey
{
    DEFINE_T_SUPER(PresentationKey)

private:
    RequiredRelationDirection m_requiredDirection;
    Utf8String m_relationshipClassNames;
    Utf8String m_relatedClassNames;
    RelationshipPathSpecification* m_propertiesSourceSpecification;
    Utf8String m_instanceFilter;
    PropertySpecificationsList m_properties;
    PropertySpecificationsList m_relationshipProperties;
    RelatedPropertiesSpecificationList m_nestedRelatedPropertiesSpecification;
    RelationshipMeaning m_relationshipMeaning;
    bool m_polymorphic;
    bool m_autoExpand;
    bool m_shouldSkipIfDuplicate;
    bool m_forceCreateRelationshipCategory;

private:
    ECPRESENTATION_EXPORT void ParsePropertiesFromPropertyJson(PropertySpecificationsList&, JsonValueCR, Utf8CP);

protected:
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName() const override;
    ECPRESENTATION_EXPORT bool _ReadXml(BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml(BeXmlNodeP xmlNode) const override;

    Utf8CP _GetJsonElementTypeAttributeName() const override {return nullptr;}
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR) const override;

public:
    ECPRESENTATION_EXPORT RelatedPropertiesSpecification();
    ECPRESENTATION_EXPORT RelatedPropertiesSpecification(RelatedPropertiesSpecification const&);
    ECPRESENTATION_EXPORT RelatedPropertiesSpecification(RelatedPropertiesSpecification&&);
    /** @deprecated */
    ECPRESENTATION_EXPORT RelatedPropertiesSpecification(RequiredRelationDirection requiredDirection,
        Utf8String relationshipClassNames, Utf8String relatedClassNames, Utf8String propertyNames,
        RelationshipMeaning relationshipMeaning, bool polymorphic = false, bool autoExpand = false);
    /** @deprecated */
    ECPRESENTATION_EXPORT RelatedPropertiesSpecification(RequiredRelationDirection requiredDirection,
        Utf8String relationshipClassNames, Utf8String relatedClassNames, PropertySpecificationsList properties,
        RelationshipMeaning relationshipMeaning, bool polymorphic = false, bool autoExpand = false);
    ECPRESENTATION_EXPORT RelatedPropertiesSpecification(RelationshipPathSpecification& propertiesSource, PropertySpecificationsList properties,
        RelationshipMeaning relationshipMeaning, bool polymorphic = false, bool autoExpand = false, bool skipIfDuplicate = false,
        PropertySpecificationsList relationshipProperties = {}, bool forceCreateRelationshipCategory = false);

    //! Destructor.
    ECPRESENTATION_EXPORT ~RelatedPropertiesSpecification(void);

    //! Returns direction of relationship that should be selected in the query.
    RequiredRelationDirection GetRequiredRelationDirection() const {return m_requiredDirection;}
    void SetRequiredRelationDirection(RequiredRelationDirection value) {m_requiredDirection = value; InvalidateHash();}

    //! Relationship class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
    Utf8StringCR GetRelationshipClassNames() const {return m_relationshipClassNames;}
    void SetRelationshipClassNames(Utf8String value) {m_relationshipClassNames = value; InvalidateHash();}

    //! Related class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
    Utf8StringCR GetRelatedClassNames() const {return m_relatedClassNames;}
    void SetRelatedClassNames(Utf8String value) {m_relatedClassNames = value; InvalidateHash();}

    //! Get specification of the relationship path to the related instance
    RelationshipPathSpecification const* GetPropertiesSource() const { return m_propertiesSourceSpecification; }
    RelationshipPathSpecification* GetPropertiesSource() { return m_propertiesSourceSpecification; }
    void SetPropertiesSource(RelationshipPathSpecification* spec) { DELETE_AND_CLEAR(m_propertiesSourceSpecification); m_propertiesSourceSpecification = spec;  InvalidateHash();}

    //! Filter for related property instances
    Utf8StringCR GetInstanceFilter() const {return m_instanceFilter;}
    void SetInstanceFilter(Utf8String value) {m_instanceFilter = value; InvalidateHash();}

    //! List of property specifications. RelatedClasses are required if related properties are specified.
    PropertySpecificationsList const& GetProperties() const {return m_properties;}
    ECPRESENTATION_EXPORT void SetProperties(PropertySpecificationsList properties);
    ECPRESENTATION_EXPORT void AddProperty(PropertySpecificationR spec);

    //! List of relationship property specifications.
    PropertySpecificationsList const& GetRelationshipProperties() const {return m_relationshipProperties;}
    ECPRESENTATION_EXPORT void SetRelationshipProperties(PropertySpecificationsList properties);
    ECPRESENTATION_EXPORT void AddRelationshipProperty(PropertySpecificationR spec);

    //! Meaning of properties returned by this specification.
    RelationshipMeaning GetRelationshipMeaning() const {return m_relationshipMeaning;}
    void SetRelationshipMeaning(RelationshipMeaning value) {m_relationshipMeaning = value; InvalidateHash();}

    //! Should related classes be handled polymorphically
    bool IsPolymorphic() const {return m_polymorphic;}
    void SetIsPolymorphic(bool value) {m_polymorphic = value; InvalidateHash();}

    //! Nested related properties, that will be shown next to ECInstance properties (the same row for example).
    ECPRESENTATION_EXPORT RelatedPropertiesSpecificationList const& GetNestedRelatedProperties() const;
    ECPRESENTATION_EXPORT void AddNestedRelatedProperty(RelatedPropertiesSpecificationR specification);

    //! Should properties be automatically expand
    bool ShouldAutoExpand() const {return m_autoExpand;}
    void SetAutoExpand(bool value) {m_autoExpand = value; InvalidateHash();}

    //! Should this specification be ignored if there's another, higher priority specification
    //! for the same properties source.
    bool ShouldSkipIfDuplicate() const {return m_shouldSkipIfDuplicate;}
    void SetSkipIfDuplicate(bool value) {m_shouldSkipIfDuplicate = value; InvalidateHash();}

    //! Should relationship category be created regardless of whether relationship properties were added.
    bool ShouldForceCreateRelationshipCategory() const {return m_forceCreateRelationshipCategory;}
    void SetForceCreateRelationshipCategory(bool value) {m_forceCreateRelationshipCategory = value; InvalidateHash();}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
