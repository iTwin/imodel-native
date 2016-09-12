/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/ChildNodeSpecification.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

typedef bvector<struct ChildNodeRule*> ChildNodeRuleList;

/*---------------------------------------------------------------------------------**//**
Base class for all ChildNodeSpecifications.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ChildNodeSpecification : PresentationRuleSpecification
    {
    private:
        int                m_priority;
        int                m_id;
        bool               m_alwaysReturnsChildren;
        bool               m_hideNodesInHierarchy;
        bool               m_hideIfNoChildren;
        bool               m_doNotSort;
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

        //! Sets the priority of the specification, can be an int.
        ECOBJECTS_EXPORT void                         SetPriority (int value);
        
        //! ID of the specification.
        ECOBJECTS_EXPORT void                         SetId(int id);

        //! ID of the specification.
        ECOBJECTS_EXPORT int                          GetId (void) const;

        //! Returns true if specification always returns nodes. This allows to optimize node expand performance.
        ECOBJECTS_EXPORT bool                         GetAlwaysReturnsChildren (void) const;

        //! Sets the AlwaysReturnsChildren value. Can be boolean.
        ECOBJECTS_EXPORT void                         SetAlwaysReturnsChildren (bool value);

        //! If this property is set it will not show nodes of this specification in the hierarchy, instead it will 
        //! use those nodes to get children that will be actually returned.
        ECOBJECTS_EXPORT bool                         GetHideNodesInHierarchy (void) const;

        //! Sets the HideNodesInHierarchy value. Can be boolean.
        ECOBJECTS_EXPORT void                         SetHideNodesInHierarchy (bool value);

        //! If this property is set, it will not show nodes in the hierarchy that doesn't contain children.
        //! Important: Setting this to true may affect tree performance.
        ECOBJECTS_EXPORT bool                         GetHideIfNoChildren (void) const;

         //! Sets the HideIfNoChildren value. Can be boolean.
        ECOBJECTS_EXPORT void                         SetHideIfNoChildren (bool value);

        //! Returns a string that represents extended data that will be passed to ECQuery for this particular specification.
        ECOBJECTS_EXPORT WStringCR                    GetExtendedData (void) const;

        //! Sets a string that represents extended data that will be passed to ECQuery for this particular specification.
        ECOBJECTS_EXPORT void                         SetExtendedData (WStringCR extendedData);

        //! Identifies whether ECInstances sort or not returned by specification. If true, then ECInstances will be listed
        //! in the order they were stored, or the order PersistenceProvider returns them.
        ECOBJECTS_EXPORT bool                         GetDoNotSort (void) const;

        //! Identifies whether ECInstances sort or not returned by specification. If true, then ECInstances will be listed
        //! in the order they were stored, or the order PersistenceProvider returns them.
        ECOBJECTS_EXPORT void                         SetDoNotSort (bool doNotSort);

        //! Collection ChildNodeSpecifications that will be used to provide child/root nodes.
        ECOBJECTS_EXPORT ChildNodeRuleList&           GetNestedRules (void);
    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
