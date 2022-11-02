/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/RuleSetLocater.h>
#include <ECPresentation/UserSettings.h>
#include <ECPresentation/Rules/PresentationRules.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//=======================================================================================
//! Holds a pair of navigation rule and specification in that rule.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass
//=======================================================================================
template<typename RuleType>
struct NavigationRuleSpecification
{
protected:
    ChildNodeSpecificationCP m_specification;
    RuleType const* m_rule;

public:
    //! Constructor. Creates an in valid instance.
    NavigationRuleSpecification() : m_specification(nullptr), m_rule(nullptr) {}

    //! Copy Constructor.
    NavigationRuleSpecification(NavigationRuleSpecification const& other) : m_specification(other.m_specification), m_rule(other.m_rule) {}

    //! Constructor.
    NavigationRuleSpecification(ChildNodeSpecificationCR specification, RuleType const& rule) : m_specification(&specification), m_rule(&rule) {}

    //! Get the specification.
    ChildNodeSpecificationCR GetSpecification() const {BeAssert(nullptr != m_specification); return *m_specification;}

    //! Get the rule.
    RuleType const& GetRule() const {BeAssert(nullptr != m_rule); return *m_rule;}

    //! Get specificaton priority.
    int GetPriority() const {return nullptr == m_specification ? -1 : m_specification->GetPriority();}
};
typedef NavigationRuleSpecification<ChildNodeRule> ChildNodeRuleSpecification;
typedef NavigationRuleSpecification<RootNodeRule> RootNodeRuleSpecification;
typedef bvector<ChildNodeRuleSpecification> ChildNodeRuleSpecificationsList;
typedef bvector<RootNodeRuleSpecification> RootNodeRuleSpecificationsList;

//=======================================================================================
//! Holds a content rule and list of selected @ref NavNodeKey objects that apply for
//! that rule.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass
//=======================================================================================
struct ContentRuleInputKeys
{
protected:
    ContentRuleCP m_rule;
    NavNodeKeyList m_matchingNodeKeys;
public:
    //! Constructor. Creates invalid instance.
    ContentRuleInputKeys() : m_rule(nullptr) {}

    //! Copy constructor.
    ContentRuleInputKeys(ContentRuleInputKeys const& other) : m_rule(other.m_rule), m_matchingNodeKeys(other.m_matchingNodeKeys) {}

    //! Constructor.
    //! @param[in] rule The content rule.
    //! @param[in] matchingNodeKeys The list of @ref NavNodeKey objects that apply for the rule.
    ContentRuleInputKeys(ContentRuleCR rule, NavNodeKeyList matchingNodeKeys = NavNodeKeyList()) : m_rule(&rule), m_matchingNodeKeys(matchingNodeKeys) {}

    //! Compare operator.
    bool operator<(ContentRuleInputKeys const& rhs) const {return m_rule < rhs.m_rule;}

    //! Equality operator. ContentRuleInputKeys are considered equal if their underlying rules are equal.
    bool operator==(ContentRuleInputKeys const& rhs) const { return m_rule == rhs.m_rule; }

    //! Inequality operator.
    bool operator!=(ContentRuleInputKeys const& rhs) { return !(*this == rhs); }

    //! Get the rule.
    ContentRuleCR GetRule() const {BeAssert(nullptr != m_rule); return *m_rule;}

    //! Get the list of selected node keys.
    NavNodeKeyListCR GetMatchingNodeKeys() const {return m_matchingNodeKeys;}
    //! Get the list of selected node keys.
    NavNodeKeyListR GetMatchingNodeKeys() {return m_matchingNodeKeys;}

    //! Get rule priority.
    int GetPriority() const {return nullptr == m_rule ? -1 : m_rule->GetPriority();}
};
typedef VectorSet<ContentRuleInputKeys> ContentRuleInputKeysContainer;

//=======================================================================================
//! Holds a content rule and list of ECIntance keys objects that apply for
//! that rule.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass
//=======================================================================================
struct ContentRuleInstanceKeys
{
protected:
    ContentRuleCP m_rule;
    bvector<ECInstanceKey> m_instanceKeys;
public:
    //! Constructor. Creates invalid instance.
    ContentRuleInstanceKeys() : m_rule(nullptr) {}

    //! Copy constructor.
    ContentRuleInstanceKeys(ContentRuleInstanceKeys const& other) : m_rule(other.m_rule), m_instanceKeys(other.m_instanceKeys) {}

    //! Constructor.
    //! @param[in] rule The content rule.
    //! @param[in] instanceKeys The list of ECIntance keys that apply for the rule.
    ContentRuleInstanceKeys(ContentRuleCR rule, bvector<ECInstanceKey> instanceKeys = bvector<ECInstanceKey>()) : m_rule(&rule), m_instanceKeys(instanceKeys) {}

    //! Compare operator.
    bool operator<(ContentRuleInstanceKeys const& rhs) const {return m_rule < rhs.m_rule;}

    //! Equality operator. ContentRuleInstanceKeys are considered equal if their underlying rules are equal.
    bool operator==(ContentRuleInstanceKeys const& rhs) const { return m_rule == rhs.m_rule; }

    //! Inequality operator.
    bool operator!=(ContentRuleInstanceKeys const& rhs) { return !(*this == rhs); }

    //! Get the rule.
    ContentRuleCR GetRule() const {BeAssert(nullptr != m_rule); return *m_rule;}

    //! Get the list of instance keys.
    bvector<ECInstanceKey> const& GetInstanceKeys() const {return m_instanceKeys;}

    //! Get rule priority.
    int GetPriority() const {return nullptr == m_rule ? -1 : m_rule->GetPriority();}
};
typedef VectorSet<ContentRuleInstanceKeys> ContentRuleInstanceKeysContainer;

//=======================================================================================
//! A class responsible for finding appropriate presentation rules based on supplied
//! parameters.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass
//=======================================================================================
struct IRulesPreprocessor
{
    //===================================================================================
    //! Parameters for finding root node rules.
    // @bsiclass
    //===================================================================================
    struct RootNodeRuleParameters
    {
    private:
        RuleTargetTree m_tree;
    public:
        //! Constructor.
        //! @param[in] tree The target tree to find the rules for.
        RootNodeRuleParameters(RuleTargetTree tree) : m_tree(tree) {}
        //! Get the target tree.
        RuleTargetTree GetTargetTree() const {return m_tree;}
    };

    //===================================================================================
    //! Parameters for finding the child node rules.
    // @bsiclass
    //===================================================================================
    struct ChildNodeRuleParameters : RootNodeRuleParameters
    {
    private:
        NavNodeCR m_parentNode;
    public:
        //! Constructor.
        //! @param[in] parentNode The parent node to find the children rules for.
        //! @param[in] tree The target tree to find the rules for.
        ChildNodeRuleParameters(NavNodeCR parentNode, RuleTargetTree tree) : RootNodeRuleParameters(tree), m_parentNode(parentNode) {}
        //! Get the parent node.
        NavNodeCR GetParentNode() const {return m_parentNode;}
    };

    //===================================================================================
    //! Parameters for finding customization rules by node and parent node.
    // @bsiclass
    //===================================================================================
    struct CustomizationRuleByNodeParameters
    {
    private:
        NavNodeCR m_node;
        NavNodeCPtr m_parentNode;
    public:
        //! Constructor.
        //! @param[in] node The node to find the rules for.
        //! @param[in] parentNode The parent node of the @p node.
        CustomizationRuleByNodeParameters(NavNodeCR node, NavNodeCP parentNode) : m_node(node), m_parentNode(parentNode) {}
        //! Get the node.
        NavNodeCR GetNode() const {return m_node;}
        //! Get the parent node.
        NavNodeCP GetParentNode() const {return m_parentNode.get();}
    };

    //===================================================================================
    //! Parameters for finding customization rules by specification hash.
    // @bsiclass
    //===================================================================================
    struct CustomizationRuleBySpecParameters
    {
    private:
        Utf8StringCR m_specificationHash;
    public:
        //! Constructor.
        //! @param[in] spec The specification to find rules for.
        CustomizationRuleBySpecParameters(ChildNodeSpecificationCR spec) : m_specificationHash(spec.GetHash()) {}
        //! Constructor.
        //! @param[in] specHash Hash of specification to find rules for.
        CustomizationRuleBySpecParameters(Utf8StringCR specHash) : m_specificationHash(specHash) {}
        //! Get the specification.
        Utf8StringCR GetSpecificationHash() const {return m_specificationHash;}
    };

    //===================================================================================
    //! Parameters for finding aggregate customization rules (grouping and sorting).
    // @bsiclass
    //===================================================================================
    struct AggregateCustomizationRuleParameters
    {
    private:
        NavNodeCP m_parentNode;
        bvector<Utf8String> m_specificationHashes;
    public:
        //! Constructor.
        //! @param[in] parentNode The parent node whose children the rules will be applied to.
        //! @param[in] specificationHash The hash of specification which nests customization rule
        AggregateCustomizationRuleParameters(NavNodeCP parentNode, Utf8StringCR specificationHash)
            : m_parentNode(parentNode), m_specificationHashes({ specificationHash })
            {}
        //! Constructor.
        //! @param[in] parentNode The parent node whose children the rules will be applied to.
        //! @param[in] specificationHashes The hash of specificationes which nest customization rules
        AggregateCustomizationRuleParameters(NavNodeCP parentNode, bvector<Utf8String> specificationHashes)
            : m_parentNode(parentNode), m_specificationHashes(std::move(specificationHashes))
            {}
        //! Get the parent node.
        NavNodeCP GetParentNode() const {return m_parentNode;}
        //! Get specification Id
        bvector<Utf8String> const& GetSpecificationHashes() const {return m_specificationHashes;}
    };

    //===================================================================================
    //! Parameters for finding content rules.
    // @bsiclass
    //===================================================================================
    struct ContentRuleParameters
    {
    private:
        INavNodeLocaterCP m_nodeLocater;
        INavNodeKeysContainerCPtr m_inputNodeKeys;
        Utf8StringCR m_preferredContentDisplayType;
        SelectionInfo const* m_selectionInfo;
        INodeLabelCalculator const& m_nodeLabelCalculator;
    public:
        //! Constructor.
        //! @param[in] inputNodeKeys A container of input nodes.
        //! @param[in] preferredContentDisplayType Type of content display that the content is going to be displayed in.
        //! @param[in] selectionInfo Info about last selection.
        //! @param[in] nodeLocater (optional) Nodes locater.
        ContentRuleParameters(INavNodeKeysContainerCR inputNodeKeys, Utf8StringCR preferredContentDisplayType, SelectionInfo const* selectionInfo, INodeLabelCalculator const& nodeLabelCalculator, INavNodeLocaterCP nodeLocater = nullptr)
            : m_inputNodeKeys(&inputNodeKeys), m_preferredContentDisplayType(preferredContentDisplayType), m_selectionInfo(selectionInfo), m_nodeLocater(nodeLocater), m_nodeLabelCalculator(nodeLabelCalculator)
            {}
        //! Do these parameters contain selection info.
        bool HasSelectionInfo() const {return nullptr != m_selectionInfo;}
        //! Get the nodes locater.
        INavNodeLocaterCP GetNodeLocater() const {return m_nodeLocater;}
        //! Get selected node keys.
        INavNodeKeysContainerCR GetInputNodeKeys() const {return *m_inputNodeKeys;}
        //! Get preferred display type.
        Utf8StringCR GetPreferredDisplayType() const {return m_preferredContentDisplayType;}
        //! Get the name of the last selection source.
        Utf8CP GetSelectionProviderName() const {return HasSelectionInfo() ? m_selectionInfo->GetSelectionProviderName().c_str() : nullptr;}
        //! Did the last selection event happen in sub-selection.
        bool IsSubSelection() const {return HasSelectionInfo() ? m_selectionInfo->IsSubSelection() : false;}
        //! Get the node label calculator
        INodeLabelCalculator const& GetNodeLabelCalculator() const {return m_nodeLabelCalculator;}
    };

    typedef RootNodeRuleParameters const&               RootNodeRuleParametersCR;
    typedef ChildNodeRuleParameters const&              ChildNodeRuleParametersCR;
    typedef CustomizationRuleByNodeParameters const&    CustomizationRuleByNodeParametersCR;
    typedef CustomizationRuleBySpecParameters const&    CustomizationRuleBySpecParametersCR;
    typedef AggregateCustomizationRuleParameters const& AggregateCustomizationRuleByNodeParametersCR;
    typedef ContentRuleParameters const&                ContentRuleParametersCR;

protected:
/** @name Navigation rules */
/** @{ */
    virtual RootNodeRuleSpecificationsList _GetRootNodeSpecifications(RootNodeRuleParametersCR params) = 0;
    virtual ChildNodeRuleSpecificationsList _GetChildNodeSpecifications(ChildNodeRuleParametersCR params) = 0;
    virtual ChildNodeSpecificationCP _FindChildNodeSpecification(Utf8StringCR specificationHash) = 0;
/** @} */

/** @name Customization rules */
/** @{ */
    //! @see GetInstanceLabelOverrides
    virtual bvector<InstanceLabelOverrideCP> _GetInstanceLabelOverrides(CustomizationRuleBySpecParametersCR) = 0;
    virtual bvector<InstanceLabelOverrideCP> _GetInstanceLabelOverrides() = 0;
    //! @see GetLabelOverride
    virtual LabelOverrideCP _GetLabelOverride(CustomizationRuleByNodeParametersCR params) = 0;
    virtual bvector<LabelOverrideCP> _GetLabelOverrides() = 0;
    //! @see GetImageIdOverride
    virtual ImageIdOverrideCP _GetImageIdOverride(CustomizationRuleByNodeParametersCR params) = 0;
    virtual bvector<ImageIdOverrideCP> _GetImageIdOverrides() = 0;
    //! @see GetStyleOverride
    virtual StyleOverrideCP _GetStyleOverride(CustomizationRuleByNodeParametersCR params) = 0;
    virtual bvector<StyleOverrideCP> _GetStyleOverrides() = 0;
    //! @see GetCheckboxRule
    virtual CheckBoxRuleCP _GetCheckboxRule(CustomizationRuleByNodeParametersCR params) = 0;
    virtual bvector<CheckBoxRuleCP> _GetCheckboxRules() = 0;
    //! @see GetDefaultPropertyCategoryOverride
    virtual DefaultPropertyCategoryOverrideCP _GetDefaultPropertyCategoryOverride() = 0;
    //! @see GetExtendedDataRules
    virtual bvector<ExtendedDataRuleCP> _GetExtendedDataRules(CustomizationRuleByNodeParametersCR params) = 0;
    virtual bvector<ExtendedDataRuleCP> _GetExtendedDataRules() = 0;
    //! @see GetNodeArtifactRules
    virtual bvector<NodeArtifactsRuleCP> _GetNodeArtifactRules(CustomizationRuleByNodeParametersCR params) = 0;
    virtual bvector<NodeArtifactsRuleCP> _GetNodeArtifactRules(CustomizationRuleBySpecParametersCR params) = 0;
    virtual bvector<NodeArtifactsRuleCP> _GetNodeArtifactRules() = 0;
    //! @see GetGroupingRules
    virtual bvector<GroupingRuleCP> _GetGroupingRules(AggregateCustomizationRuleByNodeParametersCR params) = 0;
    virtual bvector<GroupingRuleCP> _GetGroupingRules() = 0;
    //! @see GetSortingRules
    virtual bvector<SortingRuleCP> _GetSortingRules(AggregateCustomizationRuleByNodeParametersCR params) = 0;
    virtual bvector<SortingRuleCP> _GetSortingRules() = 0;
/** @} */

/** @name Content rules */
/** @{ */
    //! @see GetContentRules
    virtual bvector<ContentRuleCP> _GetContentRules() = 0;
    //! @see GetContentSpecifications
    virtual ContentRuleInputKeysContainer _GetContentSpecifications(ContentRuleParametersCR params) = 0;
    //! @see GetContentModifiers
    virtual bvector<ContentModifierCP> _GetContentModifiers() = 0;
/** @} */

public:
    virtual ~IRulesPreprocessor() {}

/** @name Navigation rules */
/** @{ */
    //! Get matching root node specifications.
    //! @param[in] params The request parameters.
    RootNodeRuleSpecificationsList GetRootNodeSpecifications(RootNodeRuleParametersCR params) {return _GetRootNodeSpecifications(params);}

    //! Get matching child node specifications.
    //! @param[in] params The request parameters.
    ChildNodeRuleSpecificationsList GetChildNodeSpecifications(ChildNodeRuleParametersCR params) {return _GetChildNodeSpecifications(params);}

    //! Find a child node specification by its hash
    ChildNodeSpecificationCP FindChildNodeSpecification(Utf8StringCR specificationHash) {return _FindChildNodeSpecification(specificationHash);}
/** @} */

/** @name Customization rules */
/** @{ */
    //! Get instance label overrides.
    bvector<InstanceLabelOverrideCP> GetInstanceLabelOverrides() { return _GetInstanceLabelOverrides(); }
    bvector<InstanceLabelOverrideCP> GetInstanceLabelOverrides(CustomizationRuleBySpecParametersCR params) {return _GetInstanceLabelOverrides(params);}

    //! Get matching label override.
    LabelOverrideCP GetLabelOverride(CustomizationRuleByNodeParametersCR params) {return _GetLabelOverride(params);}
    bvector<LabelOverrideCP> GetLabelOverrides() {return _GetLabelOverrides();}

    //! Get matching image ID override.
    //! @param[in] params The request parameters.
    ImageIdOverrideCP GetImageIdOverride(CustomizationRuleByNodeParametersCR params) {return _GetImageIdOverride(params);}
    bvector<ImageIdOverrideCP> GetImageIdOverrides() {return _GetImageIdOverrides();}

    //! Get matching style override.
    //! @param[in] params The request parameters.
    StyleOverrideCP GetStyleOverride(CustomizationRuleByNodeParametersCR params) {return _GetStyleOverride(params);}
    bvector<StyleOverrideCP> GetStyleOverrides() {return _GetStyleOverrides();}

    //! Get matching checkbox rule.
    //! @param[in] params The request parameters.
    CheckBoxRuleCP GetCheckboxRule(CustomizationRuleByNodeParametersCR params) {return _GetCheckboxRule(params);}
    bvector<CheckBoxRuleCP> GetCheckboxRules() {return _GetCheckboxRules();}

    //! Get DefaultPropertyCategoryOverride rule with the highest priority.
    DefaultPropertyCategoryOverrideCP GetDefaultPropertyCategoryOverride() {return _GetDefaultPropertyCategoryOverride();}

    //! Get matching extended data rules.
    //! @param[in] params The request parameters.
    bvector<ExtendedDataRuleCP> GetExtendedDataRules(CustomizationRuleByNodeParametersCR params) {return _GetExtendedDataRules(params);}
    bvector<ExtendedDataRuleCP> GetExtendedDataRules() {return _GetExtendedDataRules();}

    //! Get matching node artifacts rules.
    bvector<NodeArtifactsRuleCP> GetNodeArtifactRules(CustomizationRuleByNodeParametersCR params) {return _GetNodeArtifactRules(params);}
    bvector<NodeArtifactsRuleCP> GetNodeArtifactRules(CustomizationRuleBySpecParametersCR params) {return _GetNodeArtifactRules(params);}
    bvector<NodeArtifactsRuleCP> GetNodeArtifactRules() {return _GetNodeArtifactRules();}

    //! Get matching grouping rules.
    //! @param[in] params The request parameters.
    bvector<GroupingRuleCP> GetGroupingRules(AggregateCustomizationRuleByNodeParametersCR params) {return _GetGroupingRules(params);}
    bvector<GroupingRuleCP> GetGroupingRules() {return _GetGroupingRules();}

    //! Get matching sorting rules.
    //! @param[in] params The request parameters.
    bvector<SortingRuleCP> GetSortingRules(AggregateCustomizationRuleByNodeParametersCR params) {return _GetSortingRules(params);}
    bvector<SortingRuleCP> GetSortingRules() {return _GetSortingRules();}
/** @} */

/** @name Content rules */
/** @{ */
    //! Get content rules.
    bvector<ContentRuleCP> GetContentRules() {return _GetContentRules();}

    //! Get matching content rule specifications.
    ContentRuleInputKeysContainer GetContentSpecifications(ContentRuleParametersCR params) {return _GetContentSpecifications(params);}

    //! Get content modifiers.
    bvector<ContentModifierCP> GetContentModifiers() {return _GetContentModifiers();}
/** @} */
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
