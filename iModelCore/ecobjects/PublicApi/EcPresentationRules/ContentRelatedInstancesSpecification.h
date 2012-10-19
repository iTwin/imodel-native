/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/ContentRelatedInstancesSpecification.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_START__*/

/*---------------------------------------------------------------------------------**//**
Specification that creates content ECQueries for predefined relationships and/or 
related ECClasses of the selected node.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentRelatedInstancesSpecification : public ContentSpecification
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        int                        m_skipRelatedLevel;
        WString                    m_instanceFilter;
        RequiredRelationDirection  m_requiredDirection;
        WString                    m_relationshipSchemaName;
        WString                    m_relationshipClassNames;
        WString                    m_relatedSchemaName;
        WString                    m_relatedClassNames;

    protected:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode);

    public:
        ECOBJECTS_EXPORT ContentRelatedInstancesSpecification () 
            : ContentSpecification (), m_skipRelatedLevel (0), m_instanceFilter (L""), 
            m_requiredDirection (RequiredRelationDirection_Both), m_relationshipSchemaName (L""),
            m_relationshipClassNames (L""), m_relatedSchemaName (L""), m_relatedClassNames (L"")
            {
            }

        ECOBJECTS_EXPORT ContentRelatedInstancesSpecification 
                                             (
                                              int                        priority, 
                                              int                        skipRelatedLevel,
                                              WString                    instanceFilter,
                                              RequiredRelationDirection  requiredDirection,
                                              WString                    relationshipSchemaName,
                                              WString                    relationshipClassNames,
                                              WString                    relatedSchemaName,
                                              WString                    relatedClassNames
                                             ) 
            : ContentSpecification (priority), m_skipRelatedLevel (skipRelatedLevel), 
              m_instanceFilter (instanceFilter), m_requiredDirection (requiredDirection),
              m_relationshipSchemaName (relationshipSchemaName), m_relationshipClassNames (relationshipClassNames), 
              m_relatedSchemaName (relatedSchemaName), m_relatedClassNames (relatedClassNames)
            {
            }

        //! Returns level of related instances to skip.
        ECOBJECTS_EXPORT int                          GetSkipRelatedLevel (void) const           { return m_skipRelatedLevel; }

        //! InstanceFiler is spacially formated string that represents WhereCriteria in 
        //! ECQuery that is used to filter query results.
        ECOBJECTS_EXPORT WStringCR                    GetInstanceFilter (void) const             { return m_instanceFilter; }

        //! Returns direction of relationship that should be selected in the query.
        ECOBJECTS_EXPORT RequiredRelationDirection    GetRequiredRelationDirection (void) const  { return m_requiredDirection; }

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