/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/IRulesPreprocessor.h>
#include "ECExpressions/ECExpressionContextsProvider.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct RulesPreprocessor : IRulesPreprocessor
{
    //=======================================================================================
    // @bsiclass
    //=======================================================================================
    struct SpecificationsLookupContext
        {
        private:
            ECDbCR m_ecdb;
            RuleTargetTree m_targetTree;
            ECExpressionsCache& m_expressionsCache;
            Utf8CP m_specificationHash;
            bool m_requestedSpecification;
            OptimizedExpressionsParameters const* m_optimizedExpressionsParams;
            std::function<ExpressionContextPtr()> m_expressionContextPreparer;


        public:
            SpecificationsLookupContext(ECDbCR ecdb, RuleTargetTree targetTree, ECExpressionsCache& expressionsCache, Utf8CP specificationHash, bool requestedSpecification, 
                OptimizedExpressionsParameters const* optimizedExpressionsParams, std::function<ExpressionContextPtr()> expressionContextPreparer)
                : m_ecdb(ecdb), m_targetTree(targetTree), m_expressionsCache(expressionsCache), m_specificationHash(specificationHash), m_requestedSpecification(requestedSpecification),
                m_optimizedExpressionsParams(optimizedExpressionsParams),m_expressionContextPreparer(expressionContextPreparer)
                {}

            ECDbCR GetECDb() const {return m_ecdb;}
            RuleTargetTree GetRuleTargetTree() const {return m_targetTree;}
            ECExpressionsCache& GetECExpressionsCahce() const {return m_expressionsCache;}
            Utf8CP GetSpecificationHash() const {return m_specificationHash;}
            bool IsRequestedSpecification() const {return m_requestedSpecification;}
            OptimizedExpressionsParameters const* GetOptimizedExpressionsParams() const {return m_optimizedExpressionsParams;}
            std::function<ExpressionContextPtr()> GetExpressionsContextPreparer() const {return m_expressionContextPreparer;}
        };

private:
    IConnectionManagerCR m_connections;
    IConnectionCR m_connection;
    PresentationRuleSetCR m_ruleset;
    RulesetVariables const& m_rulesetVariables;
    IUsedRulesetVariablesListener* m_usedVariablesListener;
    ECExpressionsCache& m_ecexpressionsCache;

private:
    static bool VerifyCondition(Utf8CP, ECExpressionsCache&, OptimizedExpressionsParameters const*, std::function<ExpressionContextPtr()>);
    template<typename RuleType> static void ProcessSubConditions(SpecificationsLookupContext const&, RuleType const& rule, SubConditionList const&, bvector<NavigationRuleSpecification<RuleType>>& specs);
    template<typename RuleType> static bool AddMatchingSpecifications(SpecificationsLookupContext const&, bvector<RuleType*> const& rules, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled);
    template<typename RuleType> static bool AddSpecificationsByHierarchy(SpecificationsLookupContext const&, bvector<RuleType*> const& rules, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled, bool& stopProcessing, unsigned depth);
    template<typename RuleType> static bool ProcessSpecificationsByHash(SpecificationsLookupContext const&, RuleType const& rule, ChildNodeSpecificationList const& searchIn, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled, bool& stopProcessing, unsigned depth);
    template<typename RuleType> static bool ProcessSpecificationsByHash(SpecificationsLookupContext const&, RuleType const& rule, SubConditionList const&, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled, bool& stopProcessing, unsigned depth);

protected:
    // IRulesPreprocesssor: Navigation rules
    ECPRESENTATION_EXPORT RootNodeRuleSpecificationsList _GetRootNodeSpecifications(RootNodeRuleParametersCR params) override;
    ECPRESENTATION_EXPORT ChildNodeRuleSpecificationsList _GetChildNodeSpecifications(ChildNodeRuleParametersCR params) override;
    ECPRESENTATION_EXPORT ChildNodeSpecificationCP _FindChildNodeSpecification(Utf8StringCR specificationHash) override;

    // IRulesPreprocessor: Customization rules
    ECPRESENTATION_EXPORT bvector<InstanceLabelOverrideCP> _GetInstanceLabelOverrides() override;
    ECPRESENTATION_EXPORT bvector<InstanceLabelOverrideCP> _GetInstanceLabelOverrides(CustomizationRuleBySpecParametersCR) override;
    ECPRESENTATION_EXPORT LabelOverrideCP _GetLabelOverride(CustomizationRuleByNodeParametersCR params) override;
    ECPRESENTATION_EXPORT bvector<LabelOverrideCP> _GetLabelOverrides() override;
    ECPRESENTATION_EXPORT ImageIdOverrideCP _GetImageIdOverride(CustomizationRuleByNodeParametersCR params) override;
    ECPRESENTATION_EXPORT bvector<ImageIdOverrideCP> _GetImageIdOverrides() override;
    ECPRESENTATION_EXPORT StyleOverrideCP _GetStyleOverride(CustomizationRuleByNodeParametersCR params) override;
    ECPRESENTATION_EXPORT bvector<StyleOverrideCP> _GetStyleOverrides() override;
    ECPRESENTATION_EXPORT CheckBoxRuleCP _GetCheckboxRule(CustomizationRuleByNodeParametersCR params) override;
    ECPRESENTATION_EXPORT bvector<CheckBoxRuleCP> _GetCheckboxRules() override;
    ECPRESENTATION_EXPORT DefaultPropertyCategoryOverrideCP _GetDefaultPropertyCategoryOverride() override;
    ECPRESENTATION_EXPORT bvector<ExtendedDataRuleCP> _GetExtendedDataRules(CustomizationRuleByNodeParametersCR params) override;
    ECPRESENTATION_EXPORT bvector<ExtendedDataRuleCP> _GetExtendedDataRules() override;
    ECPRESENTATION_EXPORT bvector<NodeArtifactsRuleCP> _GetNodeArtifactRules(CustomizationRuleByNodeParametersCR params) override;
    ECPRESENTATION_EXPORT bvector<NodeArtifactsRuleCP> _GetNodeArtifactRules(CustomizationRuleBySpecParametersCR params) override;
    ECPRESENTATION_EXPORT bvector<NodeArtifactsRuleCP> _GetNodeArtifactRules() override;
    ECPRESENTATION_EXPORT bvector<GroupingRuleCP> _GetGroupingRules(AggregateCustomizationRuleByNodeParametersCR params) override;
    ECPRESENTATION_EXPORT bvector<GroupingRuleCP> _GetGroupingRules() override;
    ECPRESENTATION_EXPORT bvector<SortingRuleCP> _GetSortingRules(AggregateCustomizationRuleByNodeParametersCR params) override;
    ECPRESENTATION_EXPORT bvector<SortingRuleCP> _GetSortingRules() override;

    // IRulesPreprocessor: Content rules
    ECPRESENTATION_EXPORT bvector<ContentRuleCP> _GetContentRules() override;
    ECPRESENTATION_EXPORT ContentRuleInputKeysContainer _GetContentSpecifications(ContentRuleParametersCR params) override;
    ECPRESENTATION_EXPORT bvector<ContentModifierCP> _GetContentModifiers() override;

public:
    RulesPreprocessor(IConnectionManagerCR connections, IConnectionCR connection, PresentationRuleSetCR ruleset,
        RulesetVariables const& rulesetVariables, IUsedRulesetVariablesListener* usedVariablesListener, ECExpressionsCache& ecexpressionsCache)
        : m_connections(connections), m_connection(connection), m_ruleset(ruleset), m_rulesetVariables(rulesetVariables),
        m_usedVariablesListener(usedVariablesListener), m_ecexpressionsCache(ecexpressionsCache)
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
/** @} */
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
