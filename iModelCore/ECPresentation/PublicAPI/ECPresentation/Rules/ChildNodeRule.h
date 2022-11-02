/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ECPresentation/Rules/PresentationRule.h>
#include <ECPresentation/Rules/PresentationRulesTypes.h>
#include <ECPresentation/Rules/CustomizationRules.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

typedef bvector<ChildNodeSpecificationP> ChildNodeSpecificationList;
typedef bvector<SubConditionP>           SubConditionList;
typedef bvector<CustomizationRuleP>      ChildNodeCustomizationRuleList;

/*---------------------------------------------------------------------------------**//**
SubCondition can be used in ChildNodeRule or RootNodeRule in order to separate
specifications by using sub-conditions.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct SubCondition : PresentationKey
{
    DEFINE_T_SUPER(PresentationKey)

private:
    Utf8String m_condition;
    RequiredSchemaSpecificationsList m_requiredSchemas;
    SubConditionList m_subConditions;
    ChildNodeSpecificationList m_specifications;

protected:
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName() const override;
    ECPRESENTATION_EXPORT bool _ReadXml(BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml(BeXmlNodeP xmlNode) const override;

    Utf8CP _GetJsonElementTypeAttributeName() const override {return nullptr;}
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR) const override;

public:
    //! Constructor. It is used to initialize the rule with default settings.
    ECPRESENTATION_EXPORT SubCondition ();

    //! Constructor.
    ECPRESENTATION_EXPORT SubCondition (Utf8StringCR condition);

    //! Copy constructor.
    ECPRESENTATION_EXPORT SubCondition(SubConditionCR);

    //! Destructor.
    ECPRESENTATION_EXPORT ~SubCondition (void);

    //! Returns sub-condition string.
    ECPRESENTATION_EXPORT Utf8StringCR GetCondition (void);

    RequiredSchemaSpecificationsList const& GetRequiredSchemaSpecifications() const {return m_requiredSchemas;}
    ECPRESENTATION_EXPORT void ClearRequiredSchemaSpecifications();
    ECPRESENTATION_EXPORT void AddRequiredSchemaSpecification(RequiredSchemaSpecificationR spec);

    //! Collection of sub-conditions that can be used to separate specifications.
    ECPRESENTATION_EXPORT SubConditionList const& GetSubConditions (void) const;

    //! Add sub-condition.
    ECPRESENTATION_EXPORT void AddSubCondition(SubConditionR subCondition);

    //! Collection ChildNodeSpecifications that will be used to provide child/root nodes.
    ECPRESENTATION_EXPORT ChildNodeSpecificationList const& GetSpecifications (void) const;

    //! Add ChildNodeSpecification.
    ECPRESENTATION_EXPORT void AddSpecification(ChildNodeSpecificationR specification);
};

/*---------------------------------------------------------------------------------**//**
ChildNodeRule defines rules for generating child nodes.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ChildNodeRule : ConditionalPresentationRule
{
    DEFINE_T_SUPER(ConditionalPresentationRule)

private:
    RuleTargetTree             m_targetTree;
    bool                       m_stopFurtherProcessing;
    SubConditionList           m_subConditions;
    ChildNodeSpecificationList m_specifications;
    ChildNodeCustomizationRuleList m_customizationRules;

protected:
    ECPRESENTATION_EXPORT virtual bool _ShallowEqual(PresentationKeyCR other) const override;

    ECPRESENTATION_EXPORT virtual Utf8CP _GetXmlElementName() const override;
    ECPRESENTATION_EXPORT virtual bool _ReadXml(BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT virtual void _WriteXml(BeXmlNodeP xmlNode) const override;

    ECPRESENTATION_EXPORT virtual Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT virtual bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT virtual void _WriteJson(JsonValueR json) const override;

    //! Computes rule hash.
    ECPRESENTATION_EXPORT virtual MD5 _ComputeHash() const override;

public:
    //! Constructor. It is used to initialize the rule with default settings.
    ECPRESENTATION_EXPORT ChildNodeRule ();

    //! Constructor.
    ECPRESENTATION_EXPORT ChildNodeRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled, RuleTargetTree targetTree = TargetTree_Both);

    //! Copy constructor.
    ECPRESENTATION_EXPORT ChildNodeRule(ChildNodeRuleCR);

    //! Destructor.
    ECPRESENTATION_EXPORT virtual ~ChildNodeRule (void);

    //! Returns target tree for which rule should be applied.
    ECPRESENTATION_EXPORT RuleTargetTree                 GetTargetTree (void) const;

    //! Collection of sub-conditions that can be used to separate specifications.
    ECPRESENTATION_EXPORT SubConditionList const&        GetSubConditions (void) const;

    //! Add sub-condition.
    ECPRESENTATION_EXPORT void AddSubCondition(SubConditionR subCondition);

    //! Collection ChildNodeSpecifications that will be used to provide child/root nodes.
    ECPRESENTATION_EXPORT ChildNodeSpecificationList const&  GetSpecifications (void) const;

    //! Add ChildNodesSpecification.
    ECPRESENTATION_EXPORT void AddSpecification(ChildNodeSpecificationR specification);

    //! Nested customization rules applied on nodes created by this rule
    ECPRESENTATION_EXPORT ChildNodeCustomizationRuleList const&   GetCustomizationRules (void) const;

    //! Add customization rule.
    ECPRESENTATION_EXPORT void AddCustomizationRule(CustomizationRuleR customizationRule);

    //! If this flag is set, this rule will stop any further processing of rules.
    //! This helps in cases when recursion suppression is needed.
    //! Note: such rules should not contain any SubConditions or specifications,
    //! because they will not be applied.
    ECPRESENTATION_EXPORT void                           SetStopFurtherProcessing (bool stopFurtherProcessing);

    //! If this flag is set, this rule will stop any further processing of rules.
    ECPRESENTATION_EXPORT bool                           GetStopFurtherProcessing (void) const;
};

/*---------------------------------------------------------------------------------**//**
RootNodeRule defines rules for generating root nodes.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE RootNodeRule : ChildNodeRule
{
    DEFINE_T_SUPER(ChildNodeRule)

private:
    bool m_autoExpand;

protected:
    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;

    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
    ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

    //! Computes rule hash.
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

public:
    //! Constructor. It is used to initialize the rule with default settings.
    ECPRESENTATION_EXPORT RootNodeRule ();

    //! Constructor.
    ECPRESENTATION_EXPORT RootNodeRule (Utf8StringCR condition, int priority, bool onlyIfNotHandled,
        RuleTargetTree targetTree = TargetTree_Both, bool autoExpand = false);

    //! Returns flag which determines if nodes have to be automatically expanded.
    ECPRESENTATION_EXPORT bool                           GetAutoExpand (void) const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
