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
        WString                    m_relationshipClassNames;
        WString                    m_relatedClassNames;

    protected:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT virtual CharCP               _GetXmlElementName ();
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode);
        ECOBJECTS_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode);

    public:
        ECOBJECTS_EXPORT ContentRelatedInstancesSpecification () 
            : ContentSpecification (), m_skipRelatedLevel (0), m_instanceFilter (L""), 
            m_requiredDirection (RequiredRelationDirection_Both), m_relationshipClassNames (L""), m_relatedClassNames (L"")
            {
            }

        ECOBJECTS_EXPORT ContentRelatedInstancesSpecification 
                                             (
                                              int                        priority,
                                              int                        skipRelatedLevel,
                                              WString                    instanceFilter,
                                              RequiredRelationDirection  requiredDirection,
                                              WString                    relationshipClassNames,
                                              WString                    relatedClassNames
                                             ) 
            : ContentSpecification (priority), m_skipRelatedLevel (skipRelatedLevel), 
              m_instanceFilter (instanceFilter), m_requiredDirection (requiredDirection),
              m_relationshipClassNames (relationshipClassNames), m_relatedClassNames (relatedClassNames)
            {
            }

        //! Returns level of related instances to skip.
        ECOBJECTS_EXPORT int                          GetSkipRelatedLevel (void) const           { return m_skipRelatedLevel; }

        //! InstanceFiler is spacially formated string that represents WhereCriteria in 
        //! ECQuery that is used to filter query results.
        ECOBJECTS_EXPORT WStringCR                    GetInstanceFilter (void) const             { return m_instanceFilter; }

        //! Returns direction of relationship that should be selected in the query.
        ECOBJECTS_EXPORT RequiredRelationDirection    GetRequiredRelationDirection (void) const  { return m_requiredDirection; }

        //! Relationship class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECOBJECTS_EXPORT WStringCR                    GetRelationshipClassNames (void) const     { return m_relationshipClassNames; }

        //! Related class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECOBJECTS_EXPORT WStringCR                    GetRelatedClassNames (void) const          { return m_relatedClassNames; }

    };

END_BENTLEY_ECOBJECT_NAMESPACE