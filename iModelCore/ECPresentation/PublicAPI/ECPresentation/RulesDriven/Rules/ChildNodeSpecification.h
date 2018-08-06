/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/ChildNodeSpecification.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

typedef bvector<struct ChildNodeRule*>          ChildNodeRuleList;

/*---------------------------------------------------------------------------------**//**
Base class for all ChildNodeSpecifications.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ChildNodeSpecification : PresentationRuleSpecification
    {
    private:
        int                m_priority;
        bool               m_alwaysReturnsChildren;
        bool               m_hideNodesInHierarchy;
        bool               m_hideIfNoChildren;
        bool               m_doNotSort;
        Utf8String            m_extendedData;
        RelatedInstanceSpecificationList m_relatedInstances;
        ChildNodeRuleList  m_nestedRules;

    protected:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT ChildNodeSpecification ();

        //! Constructor.
        ECPRESENTATION_EXPORT ChildNodeSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, bool hideIfNoChildren);
        
        //! Copy constructor.
        ECPRESENTATION_EXPORT ChildNodeSpecification(ChildNodeSpecificationCR);

        ECPRESENTATION_EXPORT virtual bool _ReadXml(BeXmlNodeP xmlNode) override;
        ECPRESENTATION_EXPORT virtual void _WriteXml(BeXmlNodeP xmlNode) const override;

        ECPRESENTATION_EXPORT virtual bool _ReadJson(JsonValueCR json) override;
        ECPRESENTATION_EXPORT virtual void _WriteJson(JsonValueR json) const override;
        
        //! Clones this specification.
        virtual ChildNodeSpecification* _Clone() const = 0;

        //! Computes specification hash.
        ECPRESENTATION_EXPORT virtual MD5 _ComputeHash(Utf8CP parentHash) const override;

    public:
        ECPRESENTATION_EXPORT static ChildNodeSpecificationP Create(JsonValueCR);

        //! Destructor.
        ECPRESENTATION_EXPORT virtual ~ChildNodeSpecification (void);
        
        //! Clones this specification.
        ChildNodeSpecification* Clone() const {return _Clone();}

        //! Priority of the specification, defines the order in which specifications are evaluated and executed.
        ECPRESENTATION_EXPORT int                          GetPriority (void) const;

        //! Sets the priority of the specification, can be an int.
        ECPRESENTATION_EXPORT void                         SetPriority (int value);

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
        ECPRESENTATION_EXPORT void AddRelatedInstance(RelatedInstanceSpecificationR relatedInstance);

        //! A const list of related instance specifications.
        RelatedInstanceSpecificationList const& GetRelatedInstances() const {return m_relatedInstances;}

        //! Collection ChildNodeSpecifications that will be used to provide child/root nodes.
        ECPRESENTATION_EXPORT ChildNodeRuleList const&           GetNestedRules (void);

        //! Add nested rule.
        ECPRESENTATION_EXPORT void AddNestedRule(ChildNodeRuleR rule);
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
