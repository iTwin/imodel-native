/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* A performance hint for telling that node always or never returns nodes
* @bsiclass                                     Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
enum class ChildrenHint
    {
    Unknown,
    Never,
    Always,
    };

typedef bvector<struct ChildNodeRule*>          ChildNodeRuleList;

/*---------------------------------------------------------------------------------**//**
Base class for all ChildNodeSpecifications.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ChildNodeSpecification : PresentationRuleSpecification
    {
    private:
        int m_priority;
        bool m_hideNodesInHierarchy;
        bool m_hideIfNoChildren;
        Utf8String m_hideExpression;
        bool m_doNotSort;
        ChildrenHint m_hasChildren;
        Utf8String m_extendedData;
        RelatedInstanceSpecificationList m_relatedInstances;
        ChildNodeRuleList m_nestedRules;

    protected:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT ChildNodeSpecification ();

        //! Constructor.
        ECPRESENTATION_EXPORT ChildNodeSpecification (int priority, ChildrenHint hasChildren, bool hideNodesInHierarchy, bool hideIfNoChildren);

        //! Copy constructor.
        ECPRESENTATION_EXPORT ChildNodeSpecification(ChildNodeSpecificationCR);

        ECPRESENTATION_EXPORT virtual bool _ShallowEqual(PresentationRuleSpecification const& other) const override;

        ECPRESENTATION_EXPORT virtual bool _ReadXml(BeXmlNodeP xmlNode) override;
        ECPRESENTATION_EXPORT virtual void _WriteXml(BeXmlNodeP xmlNode) const override;

        ECPRESENTATION_EXPORT virtual bool _ReadJson(JsonValueCR json) override;
        ECPRESENTATION_EXPORT virtual void _WriteJson(JsonValueR json) const override;

        //! Clones this specification.
        virtual ChildNodeSpecification* _Clone() const = 0;

        //! Computes specification hash.
        ECPRESENTATION_EXPORT virtual MD5 _ComputeHash() const override;

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

        //! Get a hint for whether nodes produced by this specification always or never returns nodes
        ChildrenHint GetHasChildren() const { return m_hasChildren; }

        //! Set a hint for whether nodes produced by this specification always or never returns nodes
        void SetHasChildren(ChildrenHint value) { m_hasChildren = value; }

        //! Returns true if specification always returns nodes. This allows to optimize node expand performance.
        //! @deprecated Use GetHasChildren
        // ECPRESENTATION_EXPORT bool                         GetAlwaysReturnsChildren (void) const;

        //! Sets the AlwaysReturnsChildren value. Can be boolean.
        //! @deprecated Use SetHasChildren
        // ECPRESENTATION_EXPORT void                         SetAlwaysReturnsChildren (bool value);

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

        //! Get an ECExpression whose result is used to determine whether node should be hidden
        Utf8StringCR GetHideExpression() const {return m_hideExpression;}

        //! Set an ECExpression whose result is used to determine whether node should be hidden
        void SetHideExpression(Utf8String expr) {m_hideExpression = expr;}

        //! Returns a string that represents extended data that will be passed to ECQuery for this particular specification.
        //! @deprecated
        ECPRESENTATION_EXPORT Utf8StringCR                 GetExtendedData (void) const;

        //! Sets a string that represents extended data that will be passed to ECQuery for this particular specification.
        //! @deprecated
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
        ECPRESENTATION_EXPORT ChildNodeRuleList const&           GetNestedRules (void) const;

        //! Add nested rule.
        ECPRESENTATION_EXPORT void AddNestedRule(ChildNodeRuleR rule);
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
