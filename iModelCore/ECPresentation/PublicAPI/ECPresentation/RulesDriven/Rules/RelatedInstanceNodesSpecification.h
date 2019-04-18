/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/ChildNodeSpecification.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//! This enumerator allows to chose which direction should be honored
//! when selecting relationships in the query.
enum RequiredRelationDirection : unsigned
    {
    //! Folows relationships in both directions (default).
    RequiredRelationDirection_Both     = 0,
    //! Folows only Forward relationships.
    RequiredRelationDirection_Forward  = 1,
    //! Folows only Backward relationships.
    RequiredRelationDirection_Backward = 2
    };

/*---------------------------------------------------------------------------------**//**
This specification returns related items of defined relationship/related classes for 
the given parent node. This specification does not return any nodes if parent node is
not ECInstance node!
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE RelatedInstanceNodesSpecification : public ChildNodeSpecification
    {
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

    protected:
        //! Allows the visitor to visit this specification.
        ECPRESENTATION_EXPORT void _Accept(PresentationRuleSpecificationVisitor& visitor) const override;
        
        ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
        ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
        ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;
        
        ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
        ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
        ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;
        
        //! Clones this specification.
        ChildNodeSpecification* _Clone() const override {return new RelatedInstanceNodesSpecification(*this);}

        //! Compute specification hash.
        ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT RelatedInstanceNodesSpecification ();
        
        //! Constructor.
        //! @deprecated Use RelatedInstanceNodesSpecification(int, ChildrenHint, bool, bool, bool, bool, int, Utf8String, RequiredRelationDirection, Utf8String, Utf8String, Utf8String)
        ECPRESENTATION_EXPORT RelatedInstanceNodesSpecification(int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, 
            bool hideIfNoChildren, bool groupByClass, bool groupByRelationship, bool groupByLabel, bool showEmptyGroups, int skipRelatedLevel,
            Utf8String instanceFilter, RequiredRelationDirection requiredDirection, Utf8String supportedSchemas, Utf8String relationshipClassNames,
            Utf8String relatedClassNames);

        //! Constructor.
        ECPRESENTATION_EXPORT RelatedInstanceNodesSpecification(int priority, ChildrenHint hasChildren, 
            bool hideNodesInHierarchy, bool hideIfNoChildren, bool groupByClass, bool groupByLabel, int skipRelatedLevel,
            Utf8String instanceFilter, RequiredRelationDirection requiredDirection, Utf8String supportedSchemas,
            Utf8String relationshipClassNames, Utf8String relatedClassNames);

        //! Returns true if grouping by class should be applied.
        ECPRESENTATION_EXPORT bool                         GetGroupByClass (void) const;

        //! Sets GroupByClass value. Can be boolean.
        ECPRESENTATION_EXPORT void                         SetGroupByClass (bool value);

        //! Returns true if grouping by relationship should be applied.
        //! @deprecated
        ECPRESENTATION_EXPORT bool                         GetGroupByRelationship (void) const;

        //! Sets GroupByRelationship value. Can be boolean.
        //! @deprecated
        ECPRESENTATION_EXPORT void                         SetGroupByRelationship (bool value);

        //! Returns true if grouping by label should be applied.
        ECPRESENTATION_EXPORT bool                         GetGroupByLabel (void) const;

        //! Sets GroupByLabel value. Can be boolean.
        ECPRESENTATION_EXPORT void                         SetGroupByLabel (bool value);

        //! Returns true if class grouping nodes should be shown even if there are no 
        //! ECInstances of those classes. Grouping nodes will be generated for all listed classes.
        //! @deprecated
        ECPRESENTATION_EXPORT bool                         GetShowEmptyGroups (void) const;
        
        //! Sets ShowEmptyGroups value. Can be boolean.
        //! @deprecated
        ECPRESENTATION_EXPORT void                         SetShowEmptyGroups (bool value);

        //! Returns level of related instances to skip.
        ECPRESENTATION_EXPORT int                          GetSkipRelatedLevel (void) const;

        //! Sets SkipRelatedLevel value. Can be int.
        ECPRESENTATION_EXPORT void                         SetSkipRelatedLevel (int value);

        //! InstanceFiler is spacially formated string that represents WhereCriteria in 
        //! ECQuery that is used to filter query results (ChildNodes).
        ECPRESENTATION_EXPORT Utf8StringCR                 GetInstanceFilter (void) const;

        //! Sets InstanceFilter value. Can be string.
        ECPRESENTATION_EXPORT void                         SetInstanceFilter (Utf8String value);

        //! Returns direction of relationship that should be selected in the query.
        ECPRESENTATION_EXPORT RequiredRelationDirection    GetRequiredRelationDirection (void) const;

        //! Sets RequiredRelationDirection value. Can be RequiredRelationDirection.
        ECPRESENTATION_EXPORT void                         SetRequiredRelationDirection (RequiredRelationDirection value);

        //! Returns supported schemas that should be used by this specification.
        ECPRESENTATION_EXPORT Utf8StringCR                 GetSupportedSchemas (void) const;

        //! Sets SupportedSchemas value. Can be string.
        ECPRESENTATION_EXPORT void                         SetSupportedSchemas (Utf8String value);

        //! Relationship class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECPRESENTATION_EXPORT Utf8StringCR                 GetRelationshipClassNames (void) const;

        //! Sets RelationshipClassNames value. Can be string.
        ECPRESENTATION_EXPORT void                         SetRelationshipClassNames (Utf8String value);

        //! Related class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECPRESENTATION_EXPORT Utf8StringCR                 GetRelatedClassNames (void) const;

        //! Sets RelatedClassNames value. Can be string.
        ECPRESENTATION_EXPORT void                         SetRelatedClassNames (Utf8String value);
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
