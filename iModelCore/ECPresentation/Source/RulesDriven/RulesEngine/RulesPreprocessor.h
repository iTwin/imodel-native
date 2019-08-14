/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/RulesDriven/IRulesPreprocessor.h>
#include "ECExpressionContextsProvider.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//=======================================================================================
// @bsiclass                                    Grigas.Petraitis                03/2015
//=======================================================================================
struct RulesPreprocessor : RefCounted<IRulesPreprocessor>
{
private:
    IConnectionManagerCR m_connections;
    IConnectionCR m_connection;
    PresentationRuleSetCR m_ruleset;
    Utf8String m_locale;
    IUserSettings const& m_userSettings;
    IUsedUserSettingsListener* m_usedSettingsListener;
    ECExpressionsCache& m_ecexpressionsCache;
    ECSqlStatementCache const& m_statementCache;

private:
    static bool VerifyCondition(Utf8CP, ECExpressionsCache&, OptimizedExpressionsParameters const*, std::function<ExpressionContextPtr()>);
    static void AddSpecificationsByHierarchy(PresentationRuleSetCR, Utf8CP specificationHash, bool requested, RuleTargetTree, ECExpressionsCache&, ChildNodeRuleSpecificationsList& specs, bool& handled, bool& stopProcessing, OptimizedExpressionsParameters const*, std::function<ExpressionContextPtr()>);
    template<typename RuleType> static bool AddSpecificationsByHierarchy(bvector<RuleType*> const& rules, Utf8CP specificationHash, bool requested, RuleTargetTree, ECExpressionsCache&, unsigned depth, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled, bool& stopProcessing, OptimizedExpressionsParameters const*, std::function<ExpressionContextPtr()>);
    template<typename RuleType> static bool AddMatchingSpecifications(bvector<RuleType*> const& rules, RuleTargetTree, ECExpressionsCache&, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled, OptimizedExpressionsParameters const*, std::function<ExpressionContextPtr()>);
    template<typename RuleType> static void ProcessSubConditions(RuleType const& rule, SubConditionList const&, ECExpressionsCache&, bvector<NavigationRuleSpecification<RuleType>>& specs, OptimizedExpressionsParameters const*, std::function<ExpressionContextPtr()>);
    template<typename RuleType> static bool ProcessSpecificationsByHash(RuleType const& rule, ChildNodeSpecificationList const& searchIn, Utf8CP specificationHash, bool requested, RuleTargetTree, ECExpressionsCache&, unsigned depth, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled, bool& stopProcessing, OptimizedExpressionsParameters const*, std::function<ExpressionContextPtr()>);
    template<typename RuleType> static bool ProcessSpecificationsByHash(RuleType const& rule, SubConditionList const&, Utf8CP specificationHash, bool requested, RuleTargetTree, ECExpressionsCache&, unsigned depth, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled, bool& stopProcessing, OptimizedExpressionsParameters const*, std::function<ExpressionContextPtr()>);
    ECPRESENTATION_EXPORT static bvector<CustomizationRuleCP> GetCustomizationRulesForSpecificationInternal(PresentationRuleSetCR, ChildNodeSpecificationCR, bvector<CustomizationRuleCP> const&);

protected:
    // IRulesPreprocesssor: Navigation rules
    ECPRESENTATION_EXPORT RootNodeRuleSpecificationsList _GetRootNodeSpecifications(RootNodeRuleParametersCR params) override;
    ECPRESENTATION_EXPORT ChildNodeRuleSpecificationsList _GetChildNodeSpecifications(ChildNodeRuleParametersCR params) override;

    // IRulesPreprocessor: Customization rules
    ECPRESENTATION_EXPORT LabelOverrideCP _GetLabelOverride(CustomizationRuleParametersCR params) override;
    ECPRESENTATION_EXPORT ImageIdOverrideCP _GetImageIdOverride(CustomizationRuleParametersCR params) override;
    ECPRESENTATION_EXPORT StyleOverrideCP _GetStyleOverride(CustomizationRuleParametersCR params) override;
    ECPRESENTATION_EXPORT CheckBoxRuleCP _GetCheckboxRule(CustomizationRuleParametersCR params) override;
    ECPRESENTATION_EXPORT bvector<ExtendedDataRuleCP> _GetExtendedDataRules(CustomizationRuleParametersCR params) override;
    ECPRESENTATION_EXPORT bvector<NodeArtifactsRuleCP> _GetNodeArtifactRules(CustomizationRuleParametersCR params) override;
    ECPRESENTATION_EXPORT bvector<GroupingRuleCP> _GetGroupingRules(AggregateCustomizationRuleParametersCR params) override;
    ECPRESENTATION_EXPORT bvector<SortingRuleCP> _GetSortingRules(AggregateCustomizationRuleParametersCR params) override;

    // IRulesPreprocessor: Content rules
    ECPRESENTATION_EXPORT ContentRuleInputKeysList _GetContentSpecifications(ContentRuleParametersCR params) override;

public:
    RulesPreprocessor(IConnectionManagerCR connections, IConnectionCR connection, PresentationRuleSetCR ruleset, Utf8String locale,
        IUserSettings const& userSettings, IUsedUserSettingsListener* usedSettingsListener, ECExpressionsCache& ecexpressionsCache,
        ECSqlStatementCache const& statementCache)
        : m_connections(connections), m_connection(connection), m_ruleset(ruleset), m_locale(locale), m_userSettings(userSettings),
        m_usedSettingsListener(usedSettingsListener), m_ecexpressionsCache(ecexpressionsCache), m_statementCache(statementCache)
        {}

/** @name Rule sets */
/** @{ */
    //! Get the presentation rule set.
    //! @param[in] locaters Ruleset locater manager which holds all available ruleset locaters.
    //! @param[in] connection The connection to check whether the ruleset is supported.
    //! @param[in] rulesetId ID of the ruleset to find. Returns the first available ruleset if nullptr.
    ECPRESENTATION_EXPORT static PresentationRuleSetPtr GetPresentationRuleSet(IRulesetLocaterManager const& locaters,
        IConnectionCR connection, Utf8CP rulesetId = nullptr);
/** @} */

/** @name Customization rules */
/** @{ */
    //! Get matching localization resource definitions.
    //! @param[in] id Localization resource key definition ID.
    //! @param[in] ruleset The ruleset search in.
    ECPRESENTATION_EXPORT static LocalizationResourceKeyDefinitionCP GetLocalizationResourceKeyDefinition(Utf8StringCR id, PresentationRuleSetCR ruleset);

    //! Get all customization rules that may apply to nodes created by the specified specification
    template<typename TRule>
    static bvector<CustomizationRuleCP> GetCustomizationRulesForSpecification(PresentationRuleSetCR ruleset, ChildNodeSpecificationCR spec, bvector<TRule*> const& (PresentationRuleSet::*rootRulesGetter)() const = nullptr)
        {
        bvector<CustomizationRuleCP> rootRules;
        if (rootRulesGetter)
            std::move((ruleset.*rootRulesGetter)().begin(), (ruleset.*rootRulesGetter)().end(), std::back_inserter(rootRules));
        return GetCustomizationRulesForSpecificationInternal(ruleset, spec, rootRules);
        }
/** @} */
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
