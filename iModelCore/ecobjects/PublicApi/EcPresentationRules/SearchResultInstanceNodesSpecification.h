/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/SearchResultInstanceNodesSpecification.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
This specification returns search results instance nodes. Nodes are returned only if 
parent node is SearchNodes.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct SearchResultInstanceNodesSpecification : public ChildNodeSpecification
    {
    private:
        bool     m_groupByClass;
        bool     m_groupByLabel;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP               _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode);

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT SearchResultInstanceNodesSpecification ();

        //! Constructor.
        ECOBJECTS_EXPORT SearchResultInstanceNodesSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, 
                                                                 bool hideIfNoChildren, bool groupByClass, bool groupByLabel);

        //! Returns true if grouping by class should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByClass (void) const;

        //! Returns true if grouping by label should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByLabel (void) const;

    };

END_BENTLEY_ECOBJECT_NAMESPACE