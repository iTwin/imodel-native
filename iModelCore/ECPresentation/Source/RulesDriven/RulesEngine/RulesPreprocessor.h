/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/RulesPreprocessor.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
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
struct ContentRuleSpecification
{
protected:
    ContentRuleCP m_rule;
    NavNodeKeyList m_matchingSelectedNodeKeys;
public:
    //! Constructor. Creates invalid instance.
    ContentRuleSpecification() : m_rule(nullptr) {}

    //! Copy constructor.
    ContentRuleSpecification(ContentRuleSpecification const& other) : m_rule(other.m_rule), m_matchingSelectedNodeKeys(other.m_matchingSelectedNodeKeys) {}

    //! Constructor.
    //! @param[in] rule The content rule.
    //! @param[in] matchingSelectedNodeKeys The list of @ref NavNodeKey objects that apply for the rule.
    ContentRuleSpecification(ContentRuleCR rule, NavNodeKeyList matchingSelectedNodeKeys = NavNodeKeyList()) : m_rule(&rule), m_matchingSelectedNodeKeys(matchingSelectedNodeKeys) {}

    //! Compare operator.
    bool operator<(ContentRuleSpecification const& rhs) const {return m_rule < rhs.m_rule;}

    //! Get the rule.
    ContentRuleCR GetRule() const {BeAssert(nullptr != m_rule); return *m_rule;}

    //! Get the list of selected node keys.
    NavNodeKeyListCR GetMatchingSelectedNodeKeys() const {return m_matchingSelectedNodeKeys;}
    //! Get the list of selected node keys.
    NavNodeKeyListR GetMatchingSelectedNodeKeys() {return m_matchingSelectedNodeKeys;}

    //! Get rule priority.
    int GetPriority() const {return nullptr == m_rule ? -1 : m_rule->GetPriority();}
};
typedef bset<ContentRuleSpecification> ContentRuleSpecificationsList;

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
        BeSQLite::EC::ECDbCR m_connection;
        PresentationRuleSetCR m_ruleset;
        IUserSettings const& m_userSettings;
        IUsedUserSettingsListener* m_usedSettingsListener;
        ECExpressionsCache& m_ecexpressionsCache;
    public:
        //! Constructor.
        //! @param[in] connection The connection used for evaluating ECDb-based ECExpressions
        //! @param[in] ruleset The ruleset that contains the presentation rules.
        //! @param[in] settings The user settings object.
        //! @param[in] ecexpressionsCache ECExpressions cache that should be used by preprocessor.
        PreprocessorParameters(BeSQLite::EC::ECDbCR connection, PresentationRuleSetCR ruleset, 
            IUserSettings const& settings, IUsedUserSettingsListener* settingsListener, ECExpressionsCache& ecexpressionsCache)
            : m_connection(connection), m_ruleset(ruleset), m_userSettings(settings), m_usedSettingsListener(settingsListener), m_ecexpressionsCache(ecexpressionsCache)
            {}
        //! Get the connection.
        BeSQLite::EC::ECDbCR GetConnection() const {return m_connection;}
        //! Get the ruleset.
        PresentationRuleSetCR GetRuleset() const {return m_ruleset;}
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
        //! @param[in] connection The connection used for evaluating ECDb-based ECExpressions
        //! @param[in] ruleset The ruleset that contains the presentation rules.
        //! @param[in] tree The target tree to find the rules for.
        //! @param[in] settings The user settings object.
        //! @param[in] ecexpressionsCache ECExpressions cache that should be used by preprocessor.
        RootNodeRuleParameters(BeSQLite::EC::ECDbCR connection, PresentationRuleSetCR ruleset, RuleTargetTree tree, 
            IUserSettings const& settings, IUsedUserSettingsListener* settingsListener, ECExpressionsCache& ecexpressionsCache)
            : PreprocessorParameters(connection, ruleset, settings, settingsListener, ecexpressionsCache), m_tree(tree)
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
        //! @param[in] connection The connection used for evaluating ECDb-based ECExpressions
        //! @param[in] parentNode The parent node to find the children rules for.
        //! @param[in] ruleset The ruleset that contains the presentation rules.
        //! @param[in] tree The target tree to find the rules for.
        //! @param[in] settings The user settings object.
        //! @param[in] ecexpressionsCache ECExpressions cache that should be used by preprocessor.
        ChildNodeRuleParameters(BeSQLite::EC::ECDbCR connection, NavNodeCR parentNode, PresentationRuleSetCR ruleset, RuleTargetTree tree, 
            IUserSettings const& settings, IUsedUserSettingsListener* settingsListener, ECExpressionsCache& ecexpressionsCache)
            : RootNodeRuleParameters(connection, ruleset, tree, settings, settingsListener, ecexpressionsCache), m_parentNode(parentNode)
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
        //! @param[in] connection The connection used for evaluating ECDb-based ECExpressions
        //! @param[in] node The node to find the rules for.
        //! @param[in] parentNode The parent node of the @p node.
        //! @param[in] ruleset The ruleset that contains the presentation rules.
        //! @param[in] settings The user settings object.
        //! @param[in] ecexpressionsCache ECExpressions cache that should be used by preprocessor.
        CustomizationRuleParameters(BeSQLite::EC::ECDbCR connection, NavNodeCR node, NavNodeCP parentNode, PresentationRuleSetCR ruleset, 
            IUserSettings const& settings, IUsedUserSettingsListener* settingsListener, ECExpressionsCache& ecexpressionsCache)
            : PreprocessorParameters(connection, ruleset, settings, settingsListener, ecexpressionsCache), m_node(node), m_parentNode(parentNode)
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
        int m_specificationId;
    public:
        //! Constructor.
        //! @param[in] parentNode The parent node whose children the rules will be applied to.
        //! @param[in] specificationId The Id of specification which nests customization rule
        //! @param[in] connection The connection used for evaluating ECDb-based ECExpressions
        //! @param[in] ruleset The ruleset that contains the presentation rules.
        //! @param[in] settings The user settings object.
        //! @param[in] ecexpressionsCache ECExpressions cache that should be used by preprocessor.
        AggregateCustomizationRuleParameters(NavNodeCP parentNode, int specificationId, BeSQLite::EC::ECDbCR connection, PresentationRuleSetCR ruleset, 
            IUserSettings const& settings, IUsedUserSettingsListener* settingsListener, ECExpressionsCache& ecexpressionsCache)
            : PreprocessorParameters(connection, ruleset, settings, settingsListener, ecexpressionsCache), m_parentNode(parentNode), m_specificationId(specificationId)
            {}
        //! Get the parent node.
        NavNodeCP GetParentNode() const {return m_parentNode;}
        //! Get specification Id
        int GetSpecificationId() const {return m_specificationId;}
    };

    //===================================================================================
    //! Parameters for finding content rules.
    // @bsiclass                                    Grigas.Petraitis            04/2016
    //===================================================================================
    struct ContentRuleParameters : PreprocessorParameters
    {
    private:
        INavNodeLocaterCR m_nodeLocater;
        INavNodeKeysContainerCPtr m_selectedNodeKeys;
        Utf8StringCR m_preferredContentDisplayType;
        Utf8StringCR m_selectionProviderName;
        bool m_isSubSelection;
    public:
        //! Constructor.
        //! @param[in] connection The connection used for evaluating ECDb-based ECExpressions
        //! @param[in] selectedNodeKeys A container of selected nodes.
        //! @param[in] preferredContentDisplayType Type of content display that the content is going to be displayed in.
        //! @param[in] selectionProviderName Name of the last selection source.
        //! @param[in] isSubSelection Did the last selection event happen in sub-selection.
        //! @param[in] ruleset The ruleset that contains the presentation rules.
        //! @param[in] settings The user settings object.
        //! @param[in] ecexpressionsCache ECExpressions cache that should be used by preprocessor.
        //! @param[in] nodeLocater Nodes locater.
        ContentRuleParameters(BeSQLite::EC::ECDbCR connection, INavNodeKeysContainerCR selectedNodeKeys, Utf8StringCR preferredContentDisplayType,
            Utf8StringCR selectionProviderName, bool isSubSelection, PresentationRuleSetCR ruleset, IUserSettings const& settings, IUsedUserSettingsListener* settingsListener,
            ECExpressionsCache& ecexpressionsCache, INavNodeLocaterCR nodeLocater)
            : PreprocessorParameters(connection, ruleset, settings, settingsListener, ecexpressionsCache), m_selectedNodeKeys(&selectedNodeKeys), 
            m_preferredContentDisplayType(preferredContentDisplayType), m_selectionProviderName(selectionProviderName), m_nodeLocater(nodeLocater)
            {
            m_isSubSelection = isSubSelection;
            }
        //! Do these parameters contain any selection.
        bool HasSelectionInfo() const {return m_selectedNodeKeys.IsValid();}
        //! Get the nodes locater.
        INavNodeLocaterCR GetNodeLocater() const {return m_nodeLocater;}
        //! Get selected node keys.
        INavNodeKeysContainerCR GetSelectedNodeKeys() const {return *m_selectedNodeKeys;}
        //! Get preferred display type.
        Utf8StringCR GetPreferredDisplayType() const {return m_preferredContentDisplayType;}
        //! Get the name of the last selection source.
        Utf8StringCR GetSelectionProviderName() const {return m_selectionProviderName;}
        //! Did the last selection event happen in sub-selection.
        bool IsSubSelection() const {return m_isSubSelection;}
    };
    
    typedef RootNodeRuleParameters const&               RootNodeRuleParametersCR;
    typedef ChildNodeRuleParameters const&              ChildNodeRuleParametersCR;
    typedef CustomizationRuleParameters const&          CustomizationRuleParametersCR;
    typedef AggregateCustomizationRuleParameters const& AggregateCustomizationRuleParametersCR;
    typedef ContentRuleParameters const&                ContentRuleParametersCR;

private:
    RulesPreprocessor() {}
    static bool VerifyCondition(Utf8CP condition, ExpressionContextR, ECExpressionsCache&);
    static void AddSpecificationsByHierarchy(PresentationRuleSetCR, int specificationId, bool requested, RuleTargetTree, ExpressionContextR, ECExpressionsCache&, ChildNodeRuleSpecificationsList& specs, bool& handled, bool& stopProcessing);
    template<typename RuleType> static bool AddSpecificationsByHierarchy(bvector<RuleType*> const& rules, int specificationId, bool requested, RuleTargetTree, ExpressionContextR, ECExpressionsCache&, unsigned depth, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled, bool& stopProcessing);
    template<typename RuleType> static bool AddMatchingSpecifications(bvector<RuleType*> const& rules, RuleTargetTree, ExpressionContextR, ECExpressionsCache&, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled);
    template<typename RuleType> static void ProcessSubConditions(RuleType const& rule, SubConditionList const&, ExpressionContextR, ECExpressionsCache&, bvector<NavigationRuleSpecification<RuleType>>& specs);
    template<typename RuleType> static bool ProcessSpecificationsById(RuleType const& rule, ChildNodeSpecificationList const& searchIn, int specificationId, bool requested, RuleTargetTree, ExpressionContextR, ECExpressionsCache&, unsigned depth, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled, bool& stopProcessing);
    template<typename RuleType> static bool ProcessSpecificationsById(RuleType const& rule, SubConditionList const&, int specificationId, bool requested, RuleTargetTree, ExpressionContextR, ECExpressionsCache&, unsigned depth, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled, bool& stopProcessing);

public:
/** @name Rule sets */
/** @{ */
    //! Get the presentation rule set.
    //! @param[in] locaters Ruleset locater manager which holds all available ruleset locaters.
    //! @param[in] connection The connection to check whether the ruleset is supported.
    //! @param[in] rulesetId ID of the ruleset to find. Returns the first available ruleset if nullptr.
    ECPRESENTATION_EXPORT static PresentationRuleSetPtr GetPresentationRuleSet(RuleSetLocaterManager const& locaters, 
        ECDbCR connection, Utf8CP rulesetId = nullptr);
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
    ECPRESENTATION_EXPORT static ContentRuleSpecificationsList GetContentSpecifications(ContentRuleParametersCR params);
/** @} */
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
