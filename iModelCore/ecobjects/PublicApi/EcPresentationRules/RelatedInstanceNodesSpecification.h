/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/RelatedInstanceNodesSpecification.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_START__*/

// This enumerator allows to chose which direction should be honored
// when selecting relationships in the query.
enum RequiredRelationDirection
    {
    //Folows relationships in both directions (default).
    RequiredRelationDirection_Both     = 0,
    //Folows only Forward relationships.
    RequiredRelationDirection_Forward  = 1,
    //Folows only Backward relationships.
    RequiredRelationDirection_Backward = 2
    };

/*---------------------------------------------------------------------------------**//**
This specification returns related items of defined relationship/related classes for 
the given parent node. This specification does not return any nodes if parent node is
not ECInstance node!
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelatedInstanceNodesSpecification : public ChildNodeSpecification
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        bool                       m_groupByClass;
        bool                       m_groupByLabel;
        bool                       m_showEmptyGroups;
        int                        m_skipRelatedLevel;
        WString                    m_instanceFilter;
        RequiredRelationDirection  m_requiredDirection;
        WString                    m_supportedSchemas;
        WString                    m_relationshipSchemaName;
        WString                    m_relationshipClassNames;
        WString                    m_relatedSchemaName;
        WString                    m_relatedClassNames;


    protected:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode);

    public:
        ECOBJECTS_EXPORT RelatedInstanceNodesSpecification ()
            : ChildNodeSpecification (), m_groupByClass (true), m_groupByLabel (true), m_showEmptyGroups (false),
            m_skipRelatedLevel (0), m_instanceFilter (L""), m_requiredDirection (RequiredRelationDirection_Both),
            m_supportedSchemas (L""), m_relationshipSchemaName (L""), m_relationshipClassNames (L""), 
            m_relatedSchemaName (L""), m_relatedClassNames (L"")
            {
            }

        ECOBJECTS_EXPORT RelatedInstanceNodesSpecification 
                                          (
                                           int                        priority, 
                                           bool                       alwaysReturnsChildren, 
                                           bool                       hideNodesInHierarchy, 
                                           bool                       groupByClass,
                                           bool                       groupByLabel,
                                           bool                       showEmptyGroups,
                                           int                        skipRelatedLevel,
                                           WString                    instanceFilter,
                                           RequiredRelationDirection  requiredDirection,
                                           WString                    supportedSchemas,
                                           WString                    relationshipSchemaName,
                                           WString                    relationshipClassNames,
                                           WString                    relatedSchemaName,
                                           WString                    relatedClassNames
                                          )
            : ChildNodeSpecification (priority, alwaysReturnsChildren, hideNodesInHierarchy), 
              m_groupByClass (groupByClass), m_groupByLabel (groupByLabel), m_showEmptyGroups (showEmptyGroups), 
              m_skipRelatedLevel (skipRelatedLevel), m_instanceFilter (instanceFilter), m_requiredDirection (requiredDirection),
              m_supportedSchemas (supportedSchemas), m_relationshipSchemaName (relationshipSchemaName), m_relationshipClassNames (relationshipClassNames), 
              m_relatedSchemaName (relatedSchemaName), m_relatedClassNames (relatedClassNames)
            {
            }

        //! Returns true if grouping by class should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByClass (void) const               { return m_groupByClass; }

        //! Returns true if grouping by label should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByLabel (void) const               { return m_groupByLabel; }

        //! Returns true if class grouping nodes should be shown even if there are no 
        //! ECInstances of those classes. Grouping nodes will be generated for all listed classes.
        ECOBJECTS_EXPORT bool                         GetShowEmptyGroups (void) const            { return m_showEmptyGroups; }

        //! Returns level of related instances to skip.
        ECOBJECTS_EXPORT int                          GetSkipRelatedLevel (void) const           { return m_skipRelatedLevel; }

        //! InstanceFiler is spacially formated string that represents WhereCriteria in 
        //! ECQuery that is used to filter query results (ChildNodes).
        ECOBJECTS_EXPORT WStringCR                    GetInstanceFilter (void) const             { return m_instanceFilter; }

        //! Returns direction of relationship that should be selected in the query.
        ECOBJECTS_EXPORT RequiredRelationDirection    GetRequiredRelationDirection (void) const  { return m_requiredDirection; }

        //! Returns supported schemas that should be used by this specification.
        ECOBJECTS_EXPORT WStringCR                    GetSupportedSchemas (void) const           { return m_supportedSchemas; }

        //! Schema name of specified relationship classes.
        ECOBJECTS_EXPORT WStringCR                    GetRelationshipSchemaName (void) const     { return m_relationshipSchemaName; }

        //! Relationship class names separated by comma that should be used by this specification.
        ECOBJECTS_EXPORT WStringCR                    GetRelationshipClassNames (void) const     { return m_relationshipClassNames; }

        //! Schema name of specified related classes.
        ECOBJECTS_EXPORT WStringCR                    GetRelatedSchemaName (void) const          { return m_relatedSchemaName; }

        //! Related class names separated by comma that should be used by this specification.
        ECOBJECTS_EXPORT WStringCR                    GetRelatedClassNames (void) const          { return m_relatedClassNames; }

    };

END_BENTLEY_ECOBJECT_NAMESPACE