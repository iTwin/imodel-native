/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/IRulesPreprocessor.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/RulesDriven/RuleSetLocater.h>
#include <ECPresentation/RulesDriven/UserSettings.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//=======================================================================================
//! Holds a pair of navigation rule and specification in that rule.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass                                    Grigas.Petraitis                07/2015
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
// @bsiclass                                    Grigas.Petraitis                04/2016
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

    //! Get the rule.
    ContentRuleCR GetRule() const {BeAssert(nullptr != m_rule); return *m_rule;}

    //! Get the list of selected node keys.
    NavNodeKeyListCR GetMatchingNodeKeys() const {return m_matchingNodeKeys;}
    //! Get the list of selected node keys.
    NavNodeKeyListR GetMatchingNodeKeys() {return m_matchingNodeKeys;}

    //! Get rule priority.
    int GetPriority() const {return nullptr == m_rule ? -1 : m_rule->GetPriority();}
};
typedef bset<ContentRuleInputKeys> ContentRuleInputKeysList;

//=======================================================================================
//! Holds a content rule and list of ECIntance keys objects that apply for
//! that rule.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass                                    Saulius.Skliutas                01/2018
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

    //! Get the rule.
    ContentRuleCR GetRule() const {BeAssert(nullptr != m_rule); return *m_rule;}

    //! Get the list of instance keys.
    bvector<ECInstanceKey> const& GetInstanceKeys() const {return m_instanceKeys;}

    //! Get rule priority.
    int GetPriority() const {return nullptr == m_rule ? -1 : m_rule->GetPriority();}
};
typedef bset<ContentRuleInstanceKeys> ContentRuleInstanceKeysList;

//=======================================================================================
//! A class responsible for finding appropriate presentation rules based on supplied
//! parameters.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass                                    Grigas.Petraitis                03/2015
//=======================================================================================
struct IRulesPreprocessor
{
    //===================================================================================
    //! Parameters for finding root node rules.
    // @bsiclass                                    Grigas.Petraitis            04/2016
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
    // @bsiclass                                    Grigas.Petraitis            04/2016
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
    //! Parameters for finding customization rules.
    // @bsiclass                                    Grigas.Petraitis            04/2016
    //===================================================================================
    struct CustomizationRuleParameters
    {
    private:
        NavNodeCR m_node;
        NavNodeCPtr m_parentNode;
    public:
        //! Constructor.
        //! @param[in] node The node to find the rules for.
        //! @param[in] parentNode The parent node of the @p node.
        CustomizationRuleParameters(NavNodeCR node, NavNodeCP parentNode) : m_node(node), m_parentNode(parentNode) {}
        //! Get the node.
        NavNodeCR GetNode() const {return m_node;}
        //! Get the parent node.
        NavNodeCP GetParentNode() const {return m_parentNode.get();}
    };

    //===================================================================================
    //! Parameters for finding aggregate customization rules (grouping and sorting).
    // @bsiclass                                    Grigas.Petraitis            04/2016
    //===================================================================================
    struct AggregateCustomizationRuleParameters
    {
    private:
        NavNodeCP m_parentNode;
        Utf8StringCR m_specificationHash;
    public:
        //! Constructor.
        //! @param[in] parentNode The parent node whose children the rules will be applied to.
        //! @param[in] specificationHash The Hash of specification which nests customization rule
        AggregateCustomizationRuleParameters(NavNodeCP parentNode, Utf8StringCR specificationHash)
        : m_parentNode(parentNode), m_specificationHash(specificationHash)
            {}
        //! Get the parent node.
        NavNodeCP GetParentNode() const {return m_parentNode;}
        //! Get specification Id
        Utf8StringCR GetSpecificationHash() const {return m_specificationHash;}
    };

    //===================================================================================
    //! Parameters for finding content rules.
    // @bsiclass                                    Grigas.Petraitis            04/2016
    //===================================================================================
    struct ContentRuleParameters
    {
    private:
        INavNodeLocaterCP m_nodeLocater;
        INavNodeKeysContainerCPtr m_inputNodeKeys;
        Utf8StringCR m_preferredContentDisplayType;
        SelectionInfo const* m_selectionInfo;
    public:
        //! Constructor.
        //! @param[in] inputNodeKeys A container of input nodes.
        //! @param[in] preferredContentDisplayType Type of content display that the content is going to be displayed in.
        //! @param[in] selectionInfo Info about last selection.
        //! @param[in] nodeLocater (optional) Nodes locater.
        ContentRuleParameters(INavNodeKeysContainerCR inputNodeKeys, Utf8StringCR preferredContentDisplayType, SelectionInfo const* selectionInfo, INavNodeLocaterCP nodeLocater = nullptr)
            : m_inputNodeKeys(&inputNodeKeys), m_preferredContentDisplayType(preferredContentDisplayType), m_selectionInfo(selectionInfo), m_nodeLocater(nodeLocater)
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
    };

    typedef RootNodeRuleParameters const&               RootNodeRuleParametersCR;
    typedef ChildNodeRuleParameters const&              ChildNodeRuleParametersCR;
    typedef CustomizationRuleParameters const&          CustomizationRuleParametersCR;
    typedef AggregateCustomizationRuleParameters const& AggregateCustomizationRuleParametersCR;
    typedef ContentRuleParameters const&                ContentRuleParametersCR;

protected:
/** @name Navigation rules */
/** @{ */
    //! @see GetRootNodeSpecifications
    virtual RootNodeRuleSpecificationsList _GetRootNodeSpecifications(RootNodeRuleParametersCR params) = 0;
    //! @see GetChildNodeSpecifications
    virtual ChildNodeRuleSpecificationsList _GetChildNodeSpecifications(ChildNodeRuleParametersCR params) = 0;
/** @} */

/** @name Customization rules */
/** @{ */
    //! @see GetLabelOverride
    virtual LabelOverrideCP _GetLabelOverride(CustomizationRuleParametersCR params) = 0;
    //! @see GetImageIdOverride
    virtual ImageIdOverrideCP _GetImageIdOverride(CustomizationRuleParametersCR params) = 0;
    //! @see GetStyleOverride
    virtual StyleOverrideCP _GetStyleOverride(CustomizationRuleParametersCR params) = 0;
    //! @see GetCheckboxRule
    virtual CheckBoxRuleCP _GetCheckboxRule(CustomizationRuleParametersCR params) = 0;
    //! @see GetGroupingRules
    virtual bvector<GroupingRuleCP> _GetGroupingRules(AggregateCustomizationRuleParametersCR params) = 0;
    //! @see GetSortingRules
    virtual bvector<SortingRuleCP> _GetSortingRules(AggregateCustomizationRuleParametersCR params) = 0;
/** @} */

/** @name Content rules */
/** @{ */
    //! @see GetContentSpecifications
    virtual ContentRuleInputKeysList _GetContentSpecifications(ContentRuleParametersCR params) = 0;
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
/** @} */

/** @name Customization rules */
/** @{ */
    //! Get matching label override.
    //! @param[in] params The request parameters.
    LabelOverrideCP GetLabelOverride(CustomizationRuleParametersCR params) {return _GetLabelOverride(params);}

    //! Get matching image ID override.
    //! @param[in] params The request parameters.
    ImageIdOverrideCP GetImageIdOverride(CustomizationRuleParametersCR params) {return _GetImageIdOverride(params);}

    //! Get matching style override.
    //! @param[in] params The request parameters.
    StyleOverrideCP GetStyleOverride(CustomizationRuleParametersCR params) {return _GetStyleOverride(params);}

    //! Get matching checkbox rule.
    //! @param[in] params The request parameters.
    CheckBoxRuleCP GetCheckboxRule(CustomizationRuleParametersCR params) {return _GetCheckboxRule(params);}

    //! Get matching grouping rules.
    //! @param[in] params The request parameters.
    bvector<GroupingRuleCP> GetGroupingRules(AggregateCustomizationRuleParametersCR params) {return _GetGroupingRules(params);}

    //! Get matching sorting rules.
    //! @param[in] params The request parameters.
    bvector<SortingRuleCP> GetSortingRules(AggregateCustomizationRuleParametersCR params) {return _GetSortingRules(params);}
/** @} */

/** @name Content rules */
/** @{ */
    //! Get matching content rules.
    //! @param[in] params The request parameters.
    ContentRuleInputKeysList GetContentSpecifications(ContentRuleParametersCR params) {return _GetContentSpecifications(params);}
/** @} */
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
