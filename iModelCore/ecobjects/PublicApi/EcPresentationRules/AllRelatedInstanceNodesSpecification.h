/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/AllRelatedInstanceNodesSpecification.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#include <ECPresentationRules/ChildNodeSpecification.h>
#include <ECPresentationRules/PresentationRuleSet.h>
#include <ECPresentationRules/RelatedInstanceNodesSpecification.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
This specification returns all instance nodes available in the repository.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct AllRelatedInstanceNodesSpecification : public ChildNodeSpecification
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        bool                       m_groupByClass;
        bool                       m_groupByRelationship;
        bool                       m_groupByLabel;
        int                        m_skipRelatedLevel;
        Utf8String                 m_supportedSchemas;
        RequiredRelationDirection  m_requiredDirection;

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
        ECOBJECTS_EXPORT AllRelatedInstanceNodesSpecification ();

        //! Constructor.
        ECOBJECTS_EXPORT AllRelatedInstanceNodesSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, bool hideIfNoChildren,
                                                               bool groupByClass, bool groupByRelationship, bool groupByLabel, int skipRelatedLevel, Utf8StringCR supportedSchemas);

        //! Returns true if grouping by class should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByClass (void) const;

        //! Returns true if grouping by relationship should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByRelationship (void) const;

        //! Returns true if grouping by label should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByLabel (void) const;

        //! Returns level of related instances to skip.
        ECOBJECTS_EXPORT int                          GetSkipRelatedLevel (void) const;

        //! Returns supported schemas that should be used by this specification.
        ECOBJECTS_EXPORT Utf8StringCR                 GetSupportedSchemas (void) const;

        //! Returns direction of relationship that should be selected in the query.
        ECOBJECTS_EXPORT RequiredRelationDirection    GetRequiredRelationDirection (void) const;

        //! Sets direction of relationship that should be selected in the query.
        ECOBJECTS_EXPORT void                         SetRequiredRelationDirection (RequiredRelationDirection requiredDirection);
    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
