/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/RulesPreprocessor.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/RulesDriven/RuleSetLocater.h>
#include <ECPresentation/RulesDriven/UserSettings.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include "ECExpressionContextsProvider.h"

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
struct RulesPreprocessor
{
    //===================================================================================
    //! Base class for all rules preprocessor parameter classes.
    // @bsiclass                                    Grigas.Petraitis            04/2016
    //===================================================================================
    struct PreprocessorParameters
    {
    private:
        IConnectionManagerCR m_connections;
        IConnectionCR m_connection;
        PresentationRuleSetCR m_ruleset;
        Utf8String m_locale;
        IUserSettings const& m_userSettings;
        IUsedUserSettingsListener* m_usedSettingsListener;
        ECExpressionsCache& m_ecexpressionsCache;
    public:
        //! Constructor.
        //! @param[in] connections The connections manager.
        //! @param[in] connection The connection used for evaluating ECDb-based ECExpressions
        //! @param[in] ruleset The ruleset that contains the presentation rules.
        //! @param[in] locale Locale to use for preprocessing
        //! @param[in] settings The user settings object.
        //! @param[in] ecexpressionsCache ECExpressions cache that should be used by preprocessor.
        PreprocessorParameters(IConnectionManagerCR connections, IConnectionCR connection, PresentationRuleSetCR ruleset, Utf8String locale,
            IUserSettings const& settings, IUsedUserSettingsListener* settingsListener, ECExpressionsCache& ecexpressionsCache)
            : m_connections(connections), m_connection(connection), m_ruleset(ruleset), m_locale(locale), m_userSettings(settings),
            m_usedSettingsListener(settingsListener), m_ecexpressionsCache(ecexpressionsCache)
            {}
        //! Get the connections manager.
        IConnectionManagerCR GetConnections() const {return m_connections;}
        //! Get the connection.
        IConnectionCR GetConnection() const {return m_connection;}
        //! Get the ruleset.
        PresentationRuleSetCR GetRuleset() const {return m_ruleset;}
        //! Get locale
        Utf8StringCR GetLocale() const {return m_locale;}
        //! Get the user settings.
        IUserSettings const& GetUserSettings() const {return m_userSettings;}
        //! Get used user settings listener.
        IUsedUserSettingsListener* GetUsedUserSettingsListener() const {return m_usedSettingsListener;}
        //! Get ECExpressions cache.
        ECExpressionsCache& GetECExpressionsCache() const {return m_ecexpressionsCache;}
    };

    //===================================================================================
    //! Parameters for finding root node rules.
    // @bsiclass                                    Grigas.Petraitis            04/2016
    //===================================================================================
    struct RootNodeRuleParameters : PreprocessorParameters
    {
    private:
        RuleTargetTree m_tree;
    public:
        //! Constructor.
        //! @param[in] connections The connections manager.
        //! @param[in] connection The connection used for evaluating ECDb-based ECExpressions
        //! @param[in] ruleset The ruleset that contains the presentation rules.
        //! @param[in] tree The target tree to find the rules for.
        //! @param[in] locale Locale to use for preprocessing
        //! @param[in] settings The user settings object.
        //! @param[in] ecexpressionsCache ECExpressions cache that should be used by preprocessor.
        RootNodeRuleParameters(IConnectionManagerCR connections, IConnectionCR connection, PresentationRuleSetCR ruleset, RuleTargetTree tree,
            Utf8String locale, IUserSettings const& settings, IUsedUserSettingsListener* settingsListener, ECExpressionsCache& ecexpressionsCache)
            : PreprocessorParameters(connections, connection, ruleset, locale, settings, settingsListener, ecexpressionsCache), m_tree(tree)
            {}
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
        //! @param[in] connections The connections manager.
        //! @param[in] connection The connection used for evaluating ECDb-based ECExpressions
        //! @param[in] parentNode The parent node to find the children rules for.
        //! @param[in] ruleset The ruleset that contains the presentation rules.
        //! @param[in] tree The target tree to find the rules for.
        //! @param[in] locale Locale to use for preprocessing
        //! @param[in] settings The user settings object.
        //! @param[in] ecexpressionsCache ECExpressions cache that should be used by preprocessor.
        ChildNodeRuleParameters(IConnectionManagerCR connections, IConnectionCR connection, NavNodeCR parentNode,
            PresentationRuleSetCR ruleset, RuleTargetTree tree, Utf8String locale,
            IUserSettings const& settings, IUsedUserSettingsListener* settingsListener, ECExpressionsCache& ecexpressionsCache)
            : RootNodeRuleParameters(connections, connection, ruleset, tree, locale, settings, settingsListener, ecexpressionsCache), m_parentNode(parentNode)
            {}
        //! Get the parent node.
        NavNodeCR GetParentNode() const {return m_parentNode;}
    };

    //===================================================================================
    //! Parameters for finding customization rules.
    // @bsiclass                                    Grigas.Petraitis            04/2016
    //===================================================================================
    struct CustomizationRuleParameters : PreprocessorParameters
    {
    private:
        NavNodeCR m_node;
        NavNodeCPtr m_parentNode;
    public:
        //! Constructor.
        //! @param[in] connections The connections manager.
        //! @param[in] connection The connection used for evaluating ECDb-based ECExpressions
        //! @param[in] node The node to find the rules for.
        //! @param[in] parentNode The parent node of the @p node.
        //! @param[in] ruleset The ruleset that contains the presentation rules.
        //! @param[in] locale Locale to use for preprocessing
        //! @param[in] settings The user settings object.
        //! @param[in] ecexpressionsCache ECExpressions cache that should be used by preprocessor.
        CustomizationRuleParameters(IConnectionManagerCR connections, IConnectionCR connection, NavNodeCR node, NavNodeCP parentNode,
            PresentationRuleSetCR ruleset, Utf8String locale,
            IUserSettings const& settings, IUsedUserSettingsListener* settingsListener, ECExpressionsCache& ecexpressionsCache)
            : PreprocessorParameters(connections, connection, ruleset, locale, settings, settingsListener, ecexpressionsCache), m_node(node), m_parentNode(parentNode)
            {}
        //! Get the node.
        NavNodeCR GetNode() const {return m_node;}
        //! Get the parent node.
        NavNodeCP GetParentNode() const {return m_parentNode.get();}
    };

    //===================================================================================
    //! Parameters for finding aggregate customization rules (grouping and sorting).
    // @bsiclass                                    Grigas.Petraitis            04/2016
    //===================================================================================
    struct AggregateCustomizationRuleParameters : PreprocessorParameters
    {
    private:
        NavNodeCP m_parentNode;
        Utf8StringCR m_specificationHash;
    public:
        //! Constructor.
        //! @param[in] parentNode The parent node whose children the rules will be applied to.
        //! @param[in] specificationHash The Hash of specification which nests customization rule
        //! @param[in] connections The connections manager.
        //! @param[in] connection The connection used for evaluating ECDb-based ECExpressions
        //! @param[in] ruleset The ruleset that contains the presentation rules.
        //! @param[in] locale Locale to use for preprocessing
        //! @param[in] settings The user settings object.
        //! @param[in] ecexpressionsCache ECExpressions cache that should be used by preprocessor.
        AggregateCustomizationRuleParameters(NavNodeCP parentNode, Utf8StringCR specificationHash, IConnectionManagerCR connections,
            IConnectionCR connection, PresentationRuleSetCR ruleset, Utf8String locale,
            IUserSettings const& settings, IUsedUserSettingsListener* settingsListener, ECExpressionsCache& ecexpressionsCache)
            : PreprocessorParameters(connections, connection, ruleset, locale, settings, settingsListener, ecexpressionsCache),
            m_parentNode(parentNode), m_specificationHash(specificationHash)
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
    struct ContentRuleParameters : PreprocessorParameters
    {
    private:
        INavNodeLocaterCR m_nodeLocater;
        INavNodeKeysContainerCPtr m_inputNodeKeys;
        Utf8StringCR m_preferredContentDisplayType;
        SelectionInfo const* m_selectionInfo;
    public:
        //! Constructor.
        //! @param[in] connections The connections manager.
        //! @param[in] connection The connection used for evaluating ECDb-based ECExpressions
        //! @param[in] inputNodeKeys A container of input nodes.
        //! @param[in] preferredContentDisplayType Type of content display that the content is going to be displayed in.
        //! @param[in] selectionInfo Info about last selection.
        //! @param[in] ruleset The ruleset that contains the presentation rules.
        //! @param[in] locale Locale to use for preprocessing
        //! @param[in] settings The user settings object.
        //! @param[in] ecexpressionsCache ECExpressions cache that should be used by preprocessor.
        //! @param[in] nodeLocater Nodes locater.
        ContentRuleParameters(IConnectionManagerCR connections, IConnectionCR connection, INavNodeKeysContainerCR inputNodeKeys, Utf8StringCR preferredContentDisplayType,
            SelectionInfo const* selectionInfo, PresentationRuleSetCR ruleset, Utf8String locale, IUserSettings const& settings, IUsedUserSettingsListener* settingsListener,
            ECExpressionsCache& ecexpressionsCache, INavNodeLocaterCR nodeLocater)
            : PreprocessorParameters(connections, connection, ruleset, locale, settings, settingsListener, ecexpressionsCache), m_inputNodeKeys(&inputNodeKeys),
            m_preferredContentDisplayType(preferredContentDisplayType), m_selectionInfo(selectionInfo), m_nodeLocater(nodeLocater)
            {
            }
        //! Do these parameters contain selection info.
        bool HasSelectionInfo() const {return nullptr != m_selectionInfo;}
        //! Get the nodes locater.
        INavNodeLocaterCR GetNodeLocater() const {return m_nodeLocater;}
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

private:
    RulesPreprocessor() {}
    static bool VerifyCondition(Utf8CP, ECExpressionsCache&, OptimizedExpressionsParameters const*, std::function<ExpressionContextPtr()>);
    static void AddSpecificationsByHierarchy(PresentationRuleSetCR, Utf8CP specificationHash, bool requested, RuleTargetTree, ECExpressionsCache&, ChildNodeRuleSpecificationsList& specs, bool& handled, bool& stopProcessing, OptimizedExpressionsParameters const*, std::function<ExpressionContextPtr()>);
    template<typename RuleType> static bool AddSpecificationsByHierarchy(bvector<RuleType*> const& rules, Utf8CP specificationHash, bool requested, RuleTargetTree, ECExpressionsCache&, unsigned depth, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled, bool& stopProcessing, OptimizedExpressionsParameters const*, std::function<ExpressionContextPtr()>);
    template<typename RuleType> static bool AddMatchingSpecifications(bvector<RuleType*> const& rules, RuleTargetTree, ECExpressionsCache&, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled, OptimizedExpressionsParameters const*, std::function<ExpressionContextPtr()>);
    template<typename RuleType> static void ProcessSubConditions(RuleType const& rule, SubConditionList const&, ECExpressionsCache&, bvector<NavigationRuleSpecification<RuleType>>& specs, OptimizedExpressionsParameters const*, std::function<ExpressionContextPtr()>);
    template<typename RuleType> static bool ProcessSpecificationsByHash(RuleType const& rule, ChildNodeSpecificationList const& searchIn, Utf8CP specificationHash, bool requested, RuleTargetTree, ECExpressionsCache&, unsigned depth, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled, bool& stopProcessing, OptimizedExpressionsParameters const*, std::function<ExpressionContextPtr()>);
    template<typename RuleType> static bool ProcessSpecificationsByHash(RuleType const& rule, SubConditionList const&, Utf8CP specificationHash, bool requested, RuleTargetTree, ECExpressionsCache&, unsigned depth, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled, bool& stopProcessing, OptimizedExpressionsParameters const*, std::function<ExpressionContextPtr()>);

public:
/** @name Rule sets */
/** @{ */
    //! Get the presentation rule set.
    //! @param[in] locaters Ruleset locater manager which holds all available ruleset locaters.
    //! @param[in] connection The connection to check whether the ruleset is supported.
    //! @param[in] rulesetId ID of the ruleset to find. Returns the first available ruleset if nullptr.
    ECPRESENTATION_EXPORT static PresentationRuleSetPtr GetPresentationRuleSet(IRulesetLocaterManager const& locaters,
        IConnectionCR connection, Utf8CP rulesetId = nullptr);
/** @} */

/** @name Navigation rules */
/** @{ */
    //! Get matching root node specifications.
    //! @param[in] params The request parameters.
    ECPRESENTATION_EXPORT static RootNodeRuleSpecificationsList GetRootNodeSpecifications(RootNodeRuleParametersCR params);

    //! Get matching child node specifications.
    //! @param[in] params The request parameters.
    ECPRESENTATION_EXPORT static ChildNodeRuleSpecificationsList GetChildNodeSpecifications(ChildNodeRuleParametersCR params);
/** @} */

/** @name Customization rules */
/** @{ */
    //! Get matching label override.
    //! @param[in] params The request parameters.
    ECPRESENTATION_EXPORT static LabelOverrideCP GetLabelOverride(CustomizationRuleParametersCR params);

    //! Get matching image ID override.
    //! @param[in] params The request parameters.
    ECPRESENTATION_EXPORT static ImageIdOverrideCP GetImageIdOverride(CustomizationRuleParametersCR params);

    //! Get matching style override.
    //! @param[in] params The request parameters.
    ECPRESENTATION_EXPORT static StyleOverrideCP GetStyleOverride(CustomizationRuleParametersCR params);

    //! Get matching checkbox rule.
    //! @param[in] params The request parameters.
    ECPRESENTATION_EXPORT static CheckBoxRuleCP GetCheckboxRule(CustomizationRuleParametersCR params);

    //! Get matching grouping rules.
    //! @param[in] params The request parameters.
    ECPRESENTATION_EXPORT static bvector<GroupingRuleCP> GetGroupingRules(AggregateCustomizationRuleParametersCR params);

    //! Get matching sorting rules.
    //! @param[in] params The request parameters.
    ECPRESENTATION_EXPORT static bvector<SortingRuleCP> GetSortingRules(AggregateCustomizationRuleParametersCR params);

    //! Get matching localization resource definitions.
    //! @param[in] id Localization resource key definition ID.
    //! @param[in] ruleset The ruleset search in.
    ECPRESENTATION_EXPORT static LocalizationResourceKeyDefinitionCP GetLocalizationResourceKeyDefinition(Utf8StringCR id, PresentationRuleSetCR ruleset);
/** @} */

/** @name Content rules */
/** @{ */
    //! Get matching content rules.
    //! @param[in] params The request parameters.
    ECPRESENTATION_EXPORT static ContentRuleInputKeysList GetContentSpecifications(ContentRuleParametersCR params);
/** @} */
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
