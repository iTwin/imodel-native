/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/RelatedInstanceNodesSpecification.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#include <ECPresentationRules/ChildNodeSpecification.h>
#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//! This enumerator allows to chose which direction should be honored
//! when selecting relationships in the query.
enum RequiredRelationDirection
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
struct RelatedInstanceNodesSpecification : public ChildNodeSpecification
    {
    /*__PUBLISH_SECTION_END__*/
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
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP               _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode);

    /*__PUBLISH_SECTION_START__*/
    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT RelatedInstanceNodesSpecification ();

        //! Constructor.
        ECOBJECTS_EXPORT RelatedInstanceNodesSpecification 
                                          (
                                           int                        priority, 
                                           bool                       alwaysReturnsChildren, 
                                           bool                       hideNodesInHierarchy, 
                                           bool                       hideIfNoChildren,
                                           bool                       groupByClass,
                                           bool                       groupByRelationship,
                                           bool                       groupByLabel,
                                           bool                       showEmptyGroups,
                                           int                        skipRelatedLevel,
                                           Utf8String                 instanceFilter,
                                           RequiredRelationDirection  requiredDirection,
                                           Utf8String                 supportedSchemas,
                                           Utf8String                 relationshipClassNames,
                                           Utf8String                 relatedClassNames
                                          );

        //! Returns true if grouping by class should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByClass (void) const;

        //! Returns true if grouping by relationship should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByRelationship (void) const;

        //! Returns true if grouping by label should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByLabel (void) const;

        //! Returns true if class grouping nodes should be shown even if there are no 
        //! ECInstances of those classes. Grouping nodes will be generated for all listed classes.
        ECOBJECTS_EXPORT bool                         GetShowEmptyGroups (void) const;

        //! Returns level of related instances to skip.
        ECOBJECTS_EXPORT int                          GetSkipRelatedLevel (void) const;

        //! InstanceFiler is spacially formated string that represents WhereCriteria in 
        //! ECQuery that is used to filter query results (ChildNodes).
        ECOBJECTS_EXPORT Utf8StringCR                 GetInstanceFilter (void) const;

        //! Returns direction of relationship that should be selected in the query.
        ECOBJECTS_EXPORT RequiredRelationDirection    GetRequiredRelationDirection (void) const;

        //! Returns supported schemas that should be used by this specification.
        ECOBJECTS_EXPORT Utf8StringCR                 GetSupportedSchemas (void) const;

        //! Relationship class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECOBJECTS_EXPORT Utf8StringCR                 GetRelationshipClassNames (void) const;

        //! Related class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECOBJECTS_EXPORT Utf8StringCR                 GetRelatedClassNames (void) const;

    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
