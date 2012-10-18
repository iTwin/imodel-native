/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/ChildNodeSpecification.h $
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
Base class for all ChildNodeSpecifications.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ChildNodeSpecification /*__PUBLISH_ABSTRACT__*/
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        int                m_priority;
        int                m_id;
        bool               m_alwaysReturnsChildren;
        bool               m_hideNodesInHierarchy;
        ChildNodeRuleList  m_nestedRules;

        static int GetNewSpecificationId ();

    protected:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT ChildNodeSpecification ();
        ECOBJECTS_EXPORT ChildNodeSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy);

        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode) = 0;
        //ECOBJECTS_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode) = 0;

    public:
        //! Destructor.
        ECOBJECTS_EXPORT                              ~ChildNodeSpecification (void);

        //! Reads specification from XML.
        ECOBJECTS_EXPORT bool                         ReadXml (BeXmlNodeP xmlNode);

        //! Priority of the specification, defines the order in which specifications are evaluated and executed.
        ECOBJECTS_EXPORT int                          GetPriority (void) const               { return m_priority; }

        //! ID of the specification.
        ECOBJECTS_EXPORT int                          GetId (void) const                     { return m_id; }

        //! Returns true if specification always returns nodes. This allows to optimize node expand performance.
        ECOBJECTS_EXPORT bool                         GetAlwaysReturnsChildren (void) const  { return m_alwaysReturnsChildren; }

        //! f this property is set it will not show nodes of this specification in the hierarchy, instead it will 
        //! use those nodes to get children that will be actually returned.
        ECOBJECTS_EXPORT bool                         GetHideNodesInHierarchy (void) const   { return m_hideNodesInHierarchy; }

        //! Collection ChildNodeSpecifications that will be used to provide child/root nodes.
        ECOBJECTS_EXPORT ChildNodeRuleList&           GetNestedRules (void)                  { return m_nestedRules; }

    };

END_BENTLEY_ECOBJECT_NAMESPACE