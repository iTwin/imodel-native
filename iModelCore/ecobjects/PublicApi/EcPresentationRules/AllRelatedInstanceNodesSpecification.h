/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/AllRelatedInstanceNodesSpecification.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_START__*/

/*---------------------------------------------------------------------------------**//**
This specification returns all instance nodes available in the repository.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct AllRelatedInstanceNodesSpecification : public ChildNodeSpecification
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        bool     m_groupByClass;
        bool     m_groupByRelationship;
        bool     m_groupByLabel;
        int      m_skipRelatedLevel;
        WString  m_supportedSchemas;

    protected:
        /*__PUBLISH_SECTION_START__*/
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP               _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode);

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT AllRelatedInstanceNodesSpecification ()
            : ChildNodeSpecification (), m_groupByClass (true), m_groupByRelationship (false), m_groupByLabel (true), m_skipRelatedLevel (0), m_supportedSchemas (L"")
            {
            }

        //! Constructor.
        ECOBJECTS_EXPORT AllRelatedInstanceNodesSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, bool hideIfNoChildren,
                                              bool groupByClass, bool groupByRelationship, bool groupByLabel, int skipRelatedLevel, WStringCR supportedSchemas)
            : ChildNodeSpecification (priority, alwaysReturnsChildren, hideNodesInHierarchy, hideIfNoChildren), 
            m_groupByClass (groupByClass), m_groupByRelationship (groupByRelationship), m_groupByLabel (groupByLabel),
            m_skipRelatedLevel (skipRelatedLevel), m_supportedSchemas (supportedSchemas)
            {
            }

    
        //! Returns true if grouping by class should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByClass (void) const        { return m_groupByClass; }

        //! Returns true if grouping by relationship should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByRelationship (void) const { return m_groupByRelationship; }

        //! Returns true if grouping by label should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByLabel (void) const        { return m_groupByLabel; }

        //! Returns level of related instances to skip.
        ECOBJECTS_EXPORT int                          GetSkipRelatedLevel (void) const    { return m_skipRelatedLevel; }

        //! Returns supported schemas that should be used by this specification.
        ECOBJECTS_EXPORT WStringCR                    GetSupportedSchemas (void) const    { return m_supportedSchemas; }

    };

END_BENTLEY_ECOBJECT_NAMESPACE