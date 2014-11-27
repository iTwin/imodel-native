/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/ContentRelatedInstancesSpecification.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

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
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP               _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode);

    /*__PUBLISH_SECTION_START__*/
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

        //! InstanceFiler is spacially formated string that represents WhereCriteria in 
        //! ECQuery that is used to filter query results.
        ECOBJECTS_EXPORT WStringCR                    GetInstanceFilter (void) const;

        //! Returns direction of relationship that should be selected in the query.
        ECOBJECTS_EXPORT RequiredRelationDirection    GetRequiredRelationDirection (void) const;

        //! Relationship class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECOBJECTS_EXPORT WStringCR                    GetRelationshipClassNames (void) const;

        //! Related class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECOBJECTS_EXPORT WStringCR                    GetRelatedClassNames (void) const;

    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
