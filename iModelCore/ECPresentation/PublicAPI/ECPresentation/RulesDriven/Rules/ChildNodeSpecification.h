/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/ChildNodeSpecification.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

typedef bvector<struct ChildNodeRule*>          ChildNodeRuleList;
typedef bvector<RelatedInstanceSpecificationP>  RelatedInstanceSpecificationList;

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
        Utf8String            m_extendedData;
        RelatedInstanceSpecificationList m_relatedInstances;
        ChildNodeRuleList  m_nestedRules;

        static int GetNewSpecificationId ();

    protected:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT ChildNodeSpecification ();

        //! Constructor.
        ECPRESENTATION_EXPORT ChildNodeSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, bool hideIfNoChildren);
        
        //! Copy constructor.
        ECPRESENTATION_EXPORT ChildNodeSpecification(ChildNodeSpecificationCR);

        //! Returns XmlElement name that is used to read/save this rule information.
        ECPRESENTATION_EXPORT virtual CharCP               _GetXmlElementName () const = 0;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode) = 0;

        //! Writes rule information to given XmlNode.
        ECPRESENTATION_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode) const = 0;
        
        //! Clones this specification.
        virtual ChildNodeSpecification* _Clone() const = 0;

    public:
        //! Destructor.
        ECPRESENTATION_EXPORT virtual                      ~ChildNodeSpecification (void);

        //! Reads specification from XML.
        ECPRESENTATION_EXPORT bool                         ReadXml (BeXmlNodeP xmlNode);

        //! Writes specification to xml node.
        ECPRESENTATION_EXPORT void                         WriteXml (BeXmlNodeP parentXmlNode) const;
        
        //! Clones this specification.
        ChildNodeSpecification* Clone() const {return _Clone();}

        //! Priority of the specification, defines the order in which specifications are evaluated and executed.
        ECPRESENTATION_EXPORT int                          GetPriority (void) const;

        //! Sets the priority of the specification, can be an int.
        ECPRESENTATION_EXPORT void                         SetPriority (int value);
        
        //! ID of the specification.
        ECPRESENTATION_EXPORT void                         SetId(int id);

        //! ID of the specification.
        ECPRESENTATION_EXPORT int                          GetId (void) const;

        //! Returns true if specification always returns nodes. This allows to optimize node expand performance.
        ECPRESENTATION_EXPORT bool                         GetAlwaysReturnsChildren (void) const;

        //! Sets the AlwaysReturnsChildren value. Can be boolean.
        ECPRESENTATION_EXPORT void                         SetAlwaysReturnsChildren (bool value);

        //! If this property is set it will not show nodes of this specification in the hierarchy, instead it will 
        //! use those nodes to get children that will be actually returned.
        ECPRESENTATION_EXPORT bool                         GetHideNodesInHierarchy (void) const;

        //! Sets the HideNodesInHierarchy value. Can be boolean.
        ECPRESENTATION_EXPORT void                         SetHideNodesInHierarchy (bool value);

        //! If this property is set, it will not show nodes in the hierarchy that doesn't contain children.
        //! Important: Setting this to true may affect tree performance.
        ECPRESENTATION_EXPORT bool                         GetHideIfNoChildren (void) const;

         //! Sets the HideIfNoChildren value. Can be boolean.
        ECPRESENTATION_EXPORT void                         SetHideIfNoChildren (bool value);

        //! Returns a string that represents extended data that will be passed to ECQuery for this particular specification.
        ECPRESENTATION_EXPORT Utf8StringCR                 GetExtendedData (void) const;

        //! Sets a string that represents extended data that will be passed to ECQuery for this particular specification.
        ECPRESENTATION_EXPORT void                         SetExtendedData (Utf8StringCR extendedData);

        //! Identifies whether ECInstances sort or not returned by specification. If true, then ECInstances will be listed
        //! in the order they were stored, or the order PersistenceProvider returns them.
        ECPRESENTATION_EXPORT bool                         GetDoNotSort (void) const;

        //! Identifies whether ECInstances sort or not returned by specification. If true, then ECInstances will be listed
        //! in the order they were stored, or the order PersistenceProvider returns them.
        ECPRESENTATION_EXPORT void                         SetDoNotSort (bool doNotSort);

        //! A writable list of related instance specifications.
        RelatedInstanceSpecificationList& GetRelatedInstances() {return m_relatedInstances;}

        //! A const list of related instance specifications.
        RelatedInstanceSpecificationList const& GetRelatedInstances() const {return m_relatedInstances;}

        //! Collection ChildNodeSpecifications that will be used to provide child/root nodes.
        ECPRESENTATION_EXPORT ChildNodeRuleList&           GetNestedRules (void);
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
