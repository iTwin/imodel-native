/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ECPresentation/Rules/ChildNodeSpecification.h>
#include <ECPresentation/Rules/PresentationRuleSet.h>
#include <ECPresentation/Rules/RelationshipPathSpecification.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
This specification returns related items of defined relationship/related classes for
the given parent node. This specification does not return any nodes if parent node is
not ECInstance node!
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE RelatedInstanceNodesSpecification : ChildNodeSpecification
{
    DEFINE_T_SUPER(ChildNodeSpecification)

private:
    bool                       m_groupByClass;
    bool                       m_groupByRelationship;
    bool                       m_groupByLabel;
    bool                       m_showEmptyGroups;
    int                        m_skipRelatedLevel;
    Utf8String                 m_instanceFilter;
    RequiredRelationDirection  m_requiredDirection;
    Utf8String                 m_supportedSchemas;
    Utf8String                 m_relationshipClassNames;
    Utf8String                 m_relatedClassNames;
    bvector<RepeatableRelationshipPathSpecification*> m_relationshipPaths;

protected:
    //! Allows the visitor to visit this specification.
    ECPRESENTATION_EXPORT void _Accept(PresentationRuleSpecificationVisitor& visitor) const override;

    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;

    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
    ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

    //! Clones this specification.
    ChildNodeSpecification* _Clone() const override {return new RelatedInstanceNodesSpecification(*this);}

    //! Compute specification hash.
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

public:
    ECPRESENTATION_EXPORT RelatedInstanceNodesSpecification ();
    ECPRESENTATION_EXPORT RelatedInstanceNodesSpecification(RelatedInstanceNodesSpecification const&);
    ECPRESENTATION_EXPORT RelatedInstanceNodesSpecification(RelatedInstanceNodesSpecification&&);
    ECPRESENTATION_EXPORT RelatedInstanceNodesSpecification(int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy,
        bool hideIfNoChildren, bool groupByClass, bool groupByRelationship, bool groupByLabel, bool showEmptyGroups, int skipRelatedLevel,
        Utf8String instanceFilter, RequiredRelationDirection requiredDirection, Utf8String supportedSchemas, Utf8String relationshipClassNames,
        Utf8String relatedClassNames);
    ECPRESENTATION_EXPORT RelatedInstanceNodesSpecification(int priority, ChildrenHint hasChildren,
        bool hideNodesInHierarchy, bool hideIfNoChildren, bool groupByClass, bool groupByLabel, int skipRelatedLevel,
        Utf8String instanceFilter, RequiredRelationDirection requiredDirection, Utf8String supportedSchemas,
        Utf8String relationshipClassNames, Utf8String relatedClassNames);
    RelatedInstanceNodesSpecification(int priority, ChildrenHint hasChildren, bool hideNodesInHierarchy, bool hideIfNoChildren,
        bool groupByClass, bool groupByLabel, Utf8String instanceFilter, bvector<RepeatableRelationshipPathSpecification*> relationshipPaths)
        : ChildNodeSpecification(priority, hasChildren, hideNodesInHierarchy, hideIfNoChildren), m_groupByClass(groupByClass),
        m_groupByRelationship(false), m_groupByLabel(groupByLabel), m_relationshipPaths(relationshipPaths), m_instanceFilter(instanceFilter),
        m_skipRelatedLevel(0), m_showEmptyGroups(false), m_requiredDirection(RequiredRelationDirection_Both)
        {}
    ECPRESENTATION_EXPORT ~RelatedInstanceNodesSpecification();

    //! Returns true if grouping by class should be applied.
    bool GetGroupByClass() const {return m_groupByClass;}
    void SetGroupByClass(bool value) {m_groupByClass = value; InvalidateHash();}

    //! Returns true if grouping by relationship should be applied.
    //! @deprecated
    bool GetGroupByRelationship() const {return m_groupByRelationship;}
    void SetGroupByRelationship(bool value) {m_groupByRelationship = value; InvalidateHash();}

    //! Returns true if grouping by label should be applied.
    bool GetGroupByLabel() const {return m_groupByLabel;}
    void SetGroupByLabel(bool value) {m_groupByLabel = value; InvalidateHash();}

    //! Returns true if class grouping nodes should be shown even if there are no
    //! ECInstances of those classes. Grouping nodes will be generated for all listed classes.
    //! @deprecated
    bool GetShowEmptyGroups() const {return m_showEmptyGroups;}
    void SetShowEmptyGroups(bool value) {m_showEmptyGroups = value; InvalidateHash();}

    //! InstanceFiler is specially formated string that represents WhereCriteria in
    //! ECQuery that is used to filter query results (ChildNodes).
    Utf8StringCR GetInstanceFilter() const {return m_instanceFilter;}
    void SetInstanceFilter(Utf8String value) {m_instanceFilter = value; InvalidateHash();}

    //! Returns level of related instances to skip.
    //! @deprecated
    int GetSkipRelatedLevel() const {return m_skipRelatedLevel;}
    void SetSkipRelatedLevel(int value) {m_skipRelatedLevel = value; InvalidateHash();}

    //! Returns direction of relationship that should be selected in the query.
    //! @deprecated
    RequiredRelationDirection GetRequiredRelationDirection() const {return m_requiredDirection;}
    void SetRequiredRelationDirection(RequiredRelationDirection value) {m_requiredDirection = value; InvalidateHash();}

    //! Returns supported schemas that should be used by this specification.
    //! @deprecated
    Utf8StringCR GetSupportedSchemas() const {return m_supportedSchemas;}
    void SetSupportedSchemas(Utf8String value) {m_supportedSchemas = value; InvalidateHash();}

    //! Relationship class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
    //! @deprecated
    Utf8StringCR GetRelationshipClassNames() const {return m_relationshipClassNames;}
    void SetRelationshipClassNames(Utf8String value) {m_relationshipClassNames = value; InvalidateHash();}

    //! Related class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
    //! @deprecated
    Utf8StringCR GetRelatedClassNames() const {return m_relatedClassNames;}
    void SetRelatedClassNames(Utf8String value) {m_relatedClassNames = value; InvalidateHash();}

    //! Get specification of the relationship path to the related instance
    bvector<RepeatableRelationshipPathSpecification*> const& GetRelationshipPaths() const {return m_relationshipPaths;}
    ECPRESENTATION_EXPORT void SetRelationshipPaths(bvector<RepeatableRelationshipPathSpecification*> paths);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
