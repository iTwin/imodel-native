/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>
#include <ECPresentation/RulesDriven/Rules/RelationshipPathSpecification.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Specification that creates content ECQueries for predefined relationships and/or 
related ECClasses of the selected node.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ContentRelatedInstancesSpecification : public ContentSpecification
{
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
    ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

public:
    //! Constructor. It is used to initialize the rule with default settings.
    ECPRESENTATION_EXPORT ContentRelatedInstancesSpecification();
    ECPRESENTATION_EXPORT ContentRelatedInstancesSpecification(ContentRelatedInstancesSpecification const&);
    ECPRESENTATION_EXPORT ContentRelatedInstancesSpecification(ContentRelatedInstancesSpecification&&);
    ECPRESENTATION_EXPORT ContentRelatedInstancesSpecification(int priority, int skipRelatedLevel, bool isRecursive, 
        Utf8String instanceFilter, RequiredRelationDirection requiredDirection, Utf8String relationshipClassNames, Utf8String relatedClassNames);
    ContentRelatedInstancesSpecification(int priority, Utf8String instanceFilter, bvector<RepeatableRelationshipPathSpecification*> relationshipPaths)
        : ContentSpecification(priority), m_instanceFilter(instanceFilter), m_relationshipPaths(relationshipPaths), m_skipRelatedLevel(0),
        m_isRecursive(false), m_requiredDirection(RequiredRelationDirection_Both)
        {}
    ECPRESENTATION_EXPORT ~ContentRelatedInstancesSpecification();

    //! InstanceFiler is specially formated string that represents WhereCriteria in
    //! ECQuery that is used to filter query results.
    Utf8StringCR GetInstanceFilter() const { return m_instanceFilter; }
    void SetInstanceFilter(Utf8String value) { m_instanceFilter = value; }

    //! Returns level of related instances to skip.
    int GetSkipRelatedLevel() const { return m_skipRelatedLevel; }
    void SetSkipRelatedLevel(int value) { m_skipRelatedLevel = value; }

    //! Should the relationship be followed recursively
    //! @note The followed relationship should be recursive (e.g. A to A)
    bool IsRecursive() const {return m_isRecursive;}
    void SetIsRecursive(bool value) {m_isRecursive = value;}

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
    bvector<RepeatableRelationshipPathSpecification*> const& GetRelationshipPaths() const { return m_relationshipPaths; }
    bvector<RepeatableRelationshipPathSpecification*>& GetRelationshipPaths() { return m_relationshipPaths; }
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
