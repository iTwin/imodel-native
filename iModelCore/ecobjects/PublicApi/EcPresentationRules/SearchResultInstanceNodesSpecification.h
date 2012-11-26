/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/SearchResultInstanceNodesSpecification.h $
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
This specification returns search results instance nodes. Nodes are returned only if 
parent node is SearchNodes.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct SearchResultInstanceNodesSpecification : public ChildNodeSpecification
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        bool     m_groupByClass;
        bool     m_groupByLabel;

    protected:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT virtual CharCP               _GetXmlElementName ();
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode);
        ECOBJECTS_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode);

    public:
        ECOBJECTS_EXPORT SearchResultInstanceNodesSpecification ()
            : ChildNodeSpecification (), m_groupByClass (true), m_groupByLabel (true)
            {
            }

        ECOBJECTS_EXPORT SearchResultInstanceNodesSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, bool hideIfNoChildren,
                                                bool groupByClass, bool groupByLabel)
            : ChildNodeSpecification (priority, alwaysReturnsChildren, hideNodesInHierarchy, hideIfNoChildren), 
              m_groupByClass (groupByClass), m_groupByLabel (groupByLabel)
            {
            }

        //! Returns true if grouping by class should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByClass (void) const       { return m_groupByClass; }

        //! Returns true if grouping by label should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByLabel (void) const       { return m_groupByLabel; }

    };

END_BENTLEY_ECOBJECT_NAMESPACE