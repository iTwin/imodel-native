/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/ChildNodeSpecification.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Base class for all ChildNodeSpecifications.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ChildNodeSpecification
    {
    private:
        int                m_priority;
        int                m_id;
        bool               m_alwaysReturnsChildren;
        bool               m_hideNodesInHierarchy;
        bool               m_hideIfNoChildren;
        WString            m_extendedData;
        ChildNodeRuleList  m_nestedRules;

        static int GetNewSpecificationId ();

    protected:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT ChildNodeSpecification ();

        //! Constructor.
        ECOBJECTS_EXPORT ChildNodeSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, bool hideIfNoChildren);

        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP               _GetXmlElementName () = 0;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode) = 0;

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode) = 0;

    public:
        //! Destructor.
        ECOBJECTS_EXPORT virtual                      ~ChildNodeSpecification (void);

        //! Reads specification from XML.
        ECOBJECTS_EXPORT bool                         ReadXml (BeXmlNodeP xmlNode);

        //! Writes specification to xml node.
        ECOBJECTS_EXPORT void                         WriteXml (BeXmlNodeP parentXmlNode);

        //! Priority of the specification, defines the order in which specifications are evaluated and executed.
        ECOBJECTS_EXPORT int                          GetPriority (void) const;

        //! ID of the specification.
        ECOBJECTS_EXPORT int                          GetId (void) const;

        //! Returns true if specification always returns nodes. This allows to optimize node expand performance.
        ECOBJECTS_EXPORT bool                         GetAlwaysReturnsChildren (void) const;

        //! f this property is set it will not show nodes of this specification in the hierarchy, instead it will 
        //! use those nodes to get children that will be actually returned.
        ECOBJECTS_EXPORT bool                         GetHideNodesInHierarchy (void) const;

        //! If this property is set, it will not show nodes in the hierarchy that doesn't contain children.
        //! Important: Setting this to true may affect tree performance.
        ECOBJECTS_EXPORT bool                         GetHideIfNoChildren (void);

        //! Returns a string that represents extended data that will be passed to ECQuery for this particular specification.
        ECOBJECTS_EXPORT WStringCR                    GetExtendedData (void);

        //! Sets a string that represents extended data that will be passed to ECQuery for this particular specification.
        ECOBJECTS_EXPORT void                         SetExtendedData (WStringCR extendedData);

        //! Collection ChildNodeSpecifications that will be used to provide child/root nodes.
        ECOBJECTS_EXPORT ChildNodeRuleList&           GetNestedRules (void);
    };

END_BENTLEY_ECOBJECT_NAMESPACE