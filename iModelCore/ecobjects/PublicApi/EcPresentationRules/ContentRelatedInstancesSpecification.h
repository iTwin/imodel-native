/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/ContentRelatedInstancesSpecification.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Specification that creates content ECQueries for predefined relationships and/or 
related ECClasses of the selected node.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentRelatedInstancesSpecification : public ContentSpecification
    {
    private:
        int                        m_skipRelatedLevel;
        WString                    m_instanceFilter;
        RequiredRelationDirection  m_requiredDirection;
        WString                    m_relationshipClassNames;
        WString                    m_relatedClassNames;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP               _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode);

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT ContentRelatedInstancesSpecification ();

        //! Constructor.
        ECOBJECTS_EXPORT ContentRelatedInstancesSpecification 
                                             (
                                              int                        priority,
                                              int                        skipRelatedLevel,
                                              WString                    instanceFilter,
                                              RequiredRelationDirection  requiredDirection,
                                              WString                    relationshipClassNames,
                                              WString                    relatedClassNames
                                             );

        //! Returns level of related instances to skip.
        ECOBJECTS_EXPORT int                          GetSkipRelatedLevel (void) const;

        //! Sets the SkipRelatedLevel of the specification.
        ECOBJECTS_EXPORT void                         SetSkipRelatedLevel (int value);

        //! InstanceFiler is spacially formated string that represents WhereCriteria in 
        //! ECQuery that is used to filter query results.
        ECOBJECTS_EXPORT WStringCR                    GetInstanceFilter (void) const;

        //! Sets the instance filter of the specification.
        ECOBJECTS_EXPORT void                         SetInstanceFilter (WStringCR value); 

        //! Returns direction of relationship that should be selected in the query.
        ECOBJECTS_EXPORT RequiredRelationDirection    GetRequiredRelationDirection (void) const;

        //! Sets the required direction of the specification.
        ECOBJECTS_EXPORT void                         SetRequiredRelationDirection (RequiredRelationDirection value);

        //! Relationship class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECOBJECTS_EXPORT WStringCR                    GetRelationshipClassNames (void) const;

        //! Sets the RelationshipClassNames of the specification.
        ECOBJECTS_EXPORT void                         SetRelationshipClassNames (WStringCR value);

        //! Related class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECOBJECTS_EXPORT WStringCR                    GetRelatedClassNames (void) const;

        //! Sets the RelatedClassNames of the specification.
        ECOBJECTS_EXPORT void                         SetRelatedClassNames (WStringCR value);
    };

END_BENTLEY_ECOBJECT_NAMESPACE