/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ECPresentation/Rules/PresentationRuleSet.h>
#include <ECPresentation/Rules/RelationshipPathSpecification.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Specification that creates content ECQueries for predefined relationships and/or
related ECClasses of the selected node.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ContentRelatedInstancesSpecification : ContentSpecification
{
    DEFINE_T_SUPER(ContentSpecification)

private:
    int                        m_skipRelatedLevel;
    bool                       m_isRecursive;
    Utf8String                 m_instanceFilter;
    RequiredRelationDirection  m_requiredDirection;
    Utf8String                 m_relationshipClassNames;
    Utf8String                 m_relatedClassNames;
    bvector<RepeatableRelationshipPathSpecification*> m_relationshipPaths;

protected:
    //! Allows the visitor to visit this specification.
    ECPRESENTATION_EXPORT void _Accept(PresentationRuleSpecificationVisitor& visitor) const override;

    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
    ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

    //! Clones this content specification.
    ContentSpecification* _Clone() const override {return new ContentRelatedInstancesSpecification(*this);}

    //! Computes specification hash.
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;

public:
    //! Constructor. It is used to initialize the rule with default settings.
    ECPRESENTATION_EXPORT ContentRelatedInstancesSpecification();
    ECPRESENTATION_EXPORT ContentRelatedInstancesSpecification(ContentRelatedInstancesSpecification const&);
    ECPRESENTATION_EXPORT ContentRelatedInstancesSpecification(ContentRelatedInstancesSpecification&&);
    ECPRESENTATION_EXPORT ContentRelatedInstancesSpecification(int priority, int skipRelatedLevel, bool isRecursive,
        Utf8String instanceFilter, RequiredRelationDirection requiredDirection, Utf8String relationshipClassNames, Utf8String relatedClassNames);
    //! @deprecated Use ContentRelatedInstancesSpecification (int, bool, Utf8String, bvector<RepeatableRelationshipPathSpecification*>)
    ContentRelatedInstancesSpecification(int priority, Utf8String instanceFilter, bvector<RepeatableRelationshipPathSpecification*> relationshipPaths)
        : ContentRelatedInstancesSpecification(priority, false, instanceFilter, relationshipPaths)
        {}
    ContentRelatedInstancesSpecification(int priority, bool onlyIfNotHandled, Utf8String instanceFilter, bvector<RepeatableRelationshipPathSpecification*> relationshipPaths)
        : ContentSpecification(priority, false, onlyIfNotHandled), m_instanceFilter(instanceFilter), m_relationshipPaths(relationshipPaths), m_skipRelatedLevel(0),
        m_isRecursive(false), m_requiredDirection(RequiredRelationDirection_Both)
        {}
    ECPRESENTATION_EXPORT ~ContentRelatedInstancesSpecification();

    //! InstanceFiler is specially formated string that represents WhereCriteria in
    //! ECQuery that is used to filter query results.
    Utf8StringCR GetInstanceFilter() const { return m_instanceFilter; }
    void SetInstanceFilter(Utf8String value) { InvalidateHash(); m_instanceFilter = value; }

    //! Returns level of related instances to skip.
    int GetSkipRelatedLevel() const { return m_skipRelatedLevel; }
    void SetSkipRelatedLevel(int value) { InvalidateHash(); m_skipRelatedLevel = value; }

    //! Should the relationship be followed recursively
    //! @note The followed relationship should be recursive (e.g. A to A)
    bool IsRecursive() const { return m_isRecursive; }
    void SetIsRecursive(bool value) { InvalidateHash(); m_isRecursive = value; }

    //! Returns direction of relationship that should be selected in the query.
    RequiredRelationDirection GetRequiredRelationDirection() const { return m_requiredDirection; }
    void SetRequiredRelationDirection(RequiredRelationDirection value) { InvalidateHash(); m_requiredDirection = value; }

    //! Relationship class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
    Utf8StringCR GetRelationshipClassNames() const { return m_relationshipClassNames; }
    void SetRelationshipClassNames(Utf8String value) { InvalidateHash(); m_relationshipClassNames = value; }

    //! Related class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
    Utf8StringCR GetRelatedClassNames() const { return m_relatedClassNames; }
    void SetRelatedClassNames(Utf8String value) { InvalidateHash();  m_relatedClassNames = value; }

    //! Get specification of the relationship path to the related instance
    bvector<RepeatableRelationshipPathSpecification*> const& GetRelationshipPaths() const { return m_relationshipPaths; }
    ECPRESENTATION_EXPORT void AddRelationshipPath(RepeatableRelationshipPathSpecification& relationship);
    ECPRESENTATION_EXPORT void ClearRelationshipPaths();
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
