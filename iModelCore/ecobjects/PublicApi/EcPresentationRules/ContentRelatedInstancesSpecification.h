/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/ContentRelatedInstancesSpecification.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
        Utf8String                 m_instanceFilter;
        RequiredRelationDirection  m_requiredDirection;
        Utf8String                 m_relationshipClassNames;
        Utf8String                 m_relatedClassNames;

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
                                              Utf8String                 instanceFilter,
                                              RequiredRelationDirection  requiredDirection,
                                              Utf8String                 relationshipClassNames,
                                              Utf8String                 relatedClassNames
                                             );

        //! Returns level of related instances to skip.
        ECOBJECTS_EXPORT int                          GetSkipRelatedLevel (void) const;

        //! InstanceFiler is spacially formated string that represents WhereCriteria in 
        //! ECQuery that is used to filter query results.
        ECOBJECTS_EXPORT Utf8StringCR                 GetInstanceFilter (void) const;

        //! Returns direction of relationship that should be selected in the query.
        ECOBJECTS_EXPORT RequiredRelationDirection    GetRequiredRelationDirection (void) const;

        //! Relationship class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECOBJECTS_EXPORT Utf8StringCR                 GetRelationshipClassNames (void) const;

        //! Related class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECOBJECTS_EXPORT Utf8StringCR                 GetRelatedClassNames (void) const;

    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
