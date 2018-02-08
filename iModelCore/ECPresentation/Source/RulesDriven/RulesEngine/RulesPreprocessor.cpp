/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/RulesPreprocessor.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "RulesPreprocessor.h"
#include "ExtendedData.h"
#include "LoggingHelper.h"

//This constant is used to identify what latest major version is supported by this particular
//version of RuleDrivenPresentationECPlugin. Major version change in RuleSet means breaking
//changes that are not supported by the older plugin version. So we should accept only certain
// PresentationRuleSet versions. If iModel needs to show content in old and new version, application
//can simply put two versions of the PresentationRuleSet. The plugin will figure it out which
//version to use.
#define LATEST_MAJOR_VERSION_SUPPORTED  1

//===================================================================================
//! Customization rule with nesting depth.
// @bsiclass                                    Aidas.Vaiksnoras            04/2017
//===================================================================================
template<typename RuleType>
struct CustomizationRuleOrder
{
private:
    RuleType* m_rule;
    int m_depth;

public:
    CustomizationRuleOrder() : m_rule(nullptr), m_depth(0) {}
    CustomizationRuleOrder(RuleType* rule, int depth) : m_rule(rule), m_depth(depth) {}
    RuleType* GetRule() const { return m_rule; }
    int GetDepth() const { return m_depth; }
    bool operator<(CustomizationRuleOrder const& rhs) const 
        {
        if (m_rule->GetPriority() == rhs.GetRule()->GetPriority())
            return m_depth >= rhs.GetDepth();
        return m_rule->GetPriority() > rhs.GetRule()->GetPriority();
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static void PrioritySortedAdd(bvector<T>& list, T const& element)
    {
    auto iter = list.rbegin();
    for (; iter != list.rend(); iter++)
        {
        T const& curr = *iter;
        if (curr.GetPriority() >= element.GetPriority())
            break;
        }
    list.insert(iter.base(), element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static void CopyRules(PresentationRuleSetR target, bvector<T*> const& source)
    {
    for (T* rule : source)
        {
        T* copy = new T(*rule);
        target.AddPresentationRule(*copy);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void AppendRules(PresentationRuleSetR target, PresentationRuleSetCR source)
    {
    CopyRules(target, source.GetRootNodesRules());
    CopyRules(target, source.GetChildNodesRules());
    CopyRules(target, source.GetContentRules());
    CopyRules(target, source.GetImageIdOverrides());
    CopyRules(target, source.GetLabelOverrides());
    CopyRules(target, source.GetInstanceLabelOverrides());
    CopyRules(target, source.GetStyleOverrides());
    CopyRules(target, source.GetGroupingRules());
    CopyRules(target, source.GetLocalizationResourceKeyDefinitions());
    CopyRules(target, source.GetUserSettings());
    CopyRules(target, source.GetCheckBoxRules());
    CopyRules(target, source.GetRenameNodeRules());
    CopyRules(target, source.GetSortingRules());
    CopyRules(target, source.GetContentModifierRules());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static PresentationRuleSetPtr CreateSupplementedRuleSet(PresentationRuleSetCR primary, bvector<PresentationRuleSetPtr> supplemental)
    {
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance(primary.GetRuleSetId(), primary.GetVersionMajor(), primary.GetVersionMinor(), 
        false, "", primary.GetSupportedSchemas(), primary.GetPreferredImage(), primary.GetIsSearchEnabled());

    ruleset->SetSearchClasses(primary.GetSearchClasses());
    ruleset->SetExtendedData(primary.GetExtendedData());
    AppendRules(*ruleset, primary);

    for (PresentationRuleSetPtr supplementalSet : supplemental)
        AppendRules(*ruleset, *supplementalSet);

    return ruleset;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr RulesPreprocessor::GetPresentationRuleSet(IRulesetLocaterManager const& locaters, IConnectionCR connection, Utf8CP rulesetId)
    {
    bvector<PresentationRuleSetPtr> rulesets = locaters.LocateRuleSets(connection, rulesetId);

    // find the primary ruleset
    PresentationRuleSetPtr primary;
    for (PresentationRuleSetPtr& ruleset : rulesets)
        {
        if (ruleset->GetIsSupplemental() || ruleset->GetVersionMajor() > LATEST_MAJOR_VERSION_SUPPORTED)
            continue;

        if (primary.IsNull() || primary->GetVersionMajor() < ruleset->GetVersionMajor() 
            || (primary->GetVersionMajor() == ruleset->GetVersionMajor() && primary->GetVersionMinor() < ruleset->GetVersionMinor()))
            {
            primary = ruleset;
            }
        }

    if (primary.IsNull())
        return nullptr;

    // look for supplemental rule sets
    bmap<Utf8String, PresentationRuleSetPtr> supplementalRuleSets;
    for (PresentationRuleSetPtr& ruleset : rulesets)
        {
        if (ruleset->GetIsSupplemental() && ruleset->GetVersionMajor() == primary->GetVersionMajor())
            {
            auto iter = supplementalRuleSets.find(ruleset->GetSupplementationPurpose());
            if (supplementalRuleSets.end() == iter || iter->second->GetVersionMinor() < ruleset->GetVersionMinor())
                supplementalRuleSets[ruleset->GetSupplementationPurpose()] = ruleset;
            }
        }

    bvector<PresentationRuleSetPtr> supplementalRuleSetsList;
    for (auto pair : supplementalRuleSets)
        supplementalRuleSetsList.push_back(pair.second);

    return supplementalRuleSets.empty() ? primary : CreateSupplementedRuleSet(*primary, supplementalRuleSetsList);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool RulesPreprocessor::VerifyCondition(Utf8CP condition, ECExpressionsCache& expressionsCache, OptimizedExpressionsParameters const* optimizedParams, std::function<ExpressionContextPtr()> contextPreparer)
    {
    if (nullptr == condition || 0 == *condition)
        return true;

    if (nullptr != optimizedParams)
        {
        OptimizedExpressionPtr optimizedExp = ECExpressionOptimizer(expressionsCache).GetOptimizedExpression(condition);
        if (optimizedExp.IsValid())
            return optimizedExp->Value(*optimizedParams);
        }

    NodePtr node = ECExpressionsHelper(expressionsCache).GetNodeFromExpression(condition);

    ValueResultPtr valueResult;
    ExpressionContextPtr context = contextPreparer();
    if (ExpressionStatus::Success != node->GetValue(valueResult, *context))
        {
        LoggingHelper::LogMessage(*NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE_ECPRESENTATION_RULESENGINE),
            Utf8PrintfString("Failed to evaluate ECExpression: %s", condition).c_str(), NativeLogging::LOG_ERROR);
        return false;
        }

    ECValue value;
    if (ExpressionStatus::Success != valueResult->GetECValue(value))
        return false;

    return value.IsBoolean() && value.GetBoolean();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename RuleType>
bool RulesPreprocessor::AddMatchingSpecifications(bvector<RuleType*> const& rules, RuleTargetTree tree, ECExpressionsCache& ecexpressionsCache, 
    bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled, OptimizedExpressionsParameters const* optParams, std::function<ExpressionContextPtr()> contextPreparer)
    {
    for (RuleType* rule : rules)
        {
        if (rule->GetTargetTree() != tree && rule->GetTargetTree() != TargetTree_Both)
            continue;

        if (rule->GetOnlyIfNotHandled() && (handled || specs.size() > 0))
            continue;

        if (!rule->GetCondition().empty() && !VerifyCondition(rule->GetCondition().c_str(), ecexpressionsCache, optParams, contextPreparer))
            continue;

        if (rule->GetStopFurtherProcessing())
            return true;

        for (ChildNodeSpecificationP spec : rule->GetSpecifications())
            PrioritySortedAdd(specs, NavigationRuleSpecification<RuleType>(*spec, *rule));

        handled = true;

        ProcessSubConditions(*rule, rule->GetSubConditions(), ecexpressionsCache, specs, optParams, contextPreparer);
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename RuleType>
void RulesPreprocessor::ProcessSubConditions(RuleType const& rule, SubConditionList const& subConditions, ECExpressionsCache& ecexpressionsCache,
    bvector<NavigationRuleSpecification<RuleType>>& specs, OptimizedExpressionsParameters const* optParams, std::function<ExpressionContextPtr()> contextPreparer)
    {
    for (SubConditionP subCondition : subConditions)
        {
        if (!subCondition->GetCondition().empty() && !VerifyCondition(subCondition->GetCondition().c_str(), ecexpressionsCache, optParams, contextPreparer))
            continue;

        for (ChildNodeSpecificationP spec : subCondition->GetSpecifications())
            PrioritySortedAdd(specs, NavigationRuleSpecification<RuleType>(*spec, rule));

        ProcessSubConditions(rule, subCondition->GetSubConditions(), ecexpressionsCache, specs, optParams, contextPreparer);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesPreprocessor::AddSpecificationsByHierarchy(PresentationRuleSetCR ruleset, Utf8CP specificationHash, bool requested, RuleTargetTree tree, ECExpressionsCache& ecexpressionsCache,
    ChildNodeRuleSpecificationsList& specs, bool& handled, bool& stopProcessing, OptimizedExpressionsParameters const* optParams, std::function<ExpressionContextPtr()> contextPreparer)
    {
    ChildNodeRuleList childNodeRules;
    for (RootNodeRule* rule : ruleset.GetRootNodesRules())
        childNodeRules.push_back(rule);
    for (ChildNodeRule* rule : ruleset.GetChildNodesRules())
        childNodeRules.push_back(rule);

    AddSpecificationsByHierarchy(childNodeRules, specificationHash, requested, tree, ecexpressionsCache, 0, specs, handled, stopProcessing, optParams, contextPreparer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<ChildNodeRuleP> GetChildNodeRules(PresentationRuleSetCR ruleset)
    {
    bvector<ChildNodeRuleP> childNodeRules;
    for (RootNodeRule* rule : ruleset.GetRootNodesRules())
        childNodeRules.push_back(rule);
    for (ChildNodeRule* rule : ruleset.GetChildNodesRules())
        childNodeRules.push_back(rule);
    return childNodeRules;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static bool FindCustomizationRules(bset<CustomizationRuleOrder<CustomizationRule>>& customizationRules, 
    bvector<ChildNodeRuleP> const& childNodeRules, Utf8CP specificationHash, int depth)
    {
    for (ChildNodeRule* rule : childNodeRules)
        {
        for (ChildNodeSpecificationP spec : rule->GetSpecifications())
            {
            if (0 == strcmp(specificationHash, spec->GetHash().c_str()) || FindCustomizationRules(customizationRules, spec->GetNestedRules(), specificationHash,  depth+1))
                {
                for (CustomizationRuleP customization : rule->GetCustomizationRules())
                    customizationRules.insert(CustomizationRuleOrder<CustomizationRule>(customization, depth));
                return true;
                }
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename RuleType>
bool RulesPreprocessor::AddSpecificationsByHierarchy(bvector<RuleType*> const& rules, Utf8CP specificationHash, bool requested, RuleTargetTree tree, ECExpressionsCache& ecexpressionsCache,
    unsigned depth, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled, bool& stopProcessing, OptimizedExpressionsParameters const* optParams, std::function<ExpressionContextPtr()> contextPreparer)
    {
    if (stopProcessing)
        return false;

    bool specificationFound = false;
    for (RuleType* rule : rules)
        {
        bool specificationFound = ProcessSpecificationsByHash(*rule, rule->GetSpecifications(), specificationHash, requested, tree, 
            ecexpressionsCache, depth, specs, handled, stopProcessing, optParams, contextPreparer);
        if (stopProcessing)
            return false;

        if (specificationFound)
            break;

        specificationFound = ProcessSpecificationsByHash(*rule, rule->GetSubConditions(), specificationHash, requested, tree, ecexpressionsCache, depth, specs, handled,
            stopProcessing, optParams, contextPreparer);
        if (stopProcessing)
            return false;

        if (specificationFound)
            break;
        }

    if (specificationFound && !requested && depth > 0)
        stopProcessing = AddMatchingSpecifications(rules, tree, ecexpressionsCache, specs, handled, optParams, contextPreparer);

    return specificationFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename RuleType>
bool RulesPreprocessor::ProcessSpecificationsByHash(RuleType const& rule, ChildNodeSpecificationList const& searchIn, Utf8CP specificationHash, bool requested, RuleTargetTree tree,
    ECExpressionsCache& ecexpressionsCache, unsigned depth, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled, bool& stopProcessing, OptimizedExpressionsParameters const* optParams, std::function<ExpressionContextPtr()> contextPreparer)
    {
    for (ChildNodeSpecificationP specification : searchIn)
        {
        if (0 == strcmp(specification->GetHash().c_str(), specificationHash))
            {
            if (requested)
                PrioritySortedAdd(specs, ChildNodeRuleSpecification(*specification, rule));

            stopProcessing = AddMatchingSpecifications(specification->GetNestedRules(), tree, ecexpressionsCache, specs, handled, optParams, contextPreparer);
            stopProcessing |= requested;
            return true;
            }
        else if (AddSpecificationsByHierarchy(specification->GetNestedRules(), specificationHash, requested, tree,
            ecexpressionsCache, depth + 1, specs, handled, stopProcessing, optParams, contextPreparer))
            {
            return true;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename RuleType>
bool RulesPreprocessor::ProcessSpecificationsByHash(RuleType const& rule, SubConditionList const& subConditions, Utf8CP specificationHash, bool requested, RuleTargetTree tree,
    ECExpressionsCache& ecexpressionsCache, unsigned depth, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled, bool& stopProcessing, OptimizedExpressionsParameters const* optParams, std::function<ExpressionContextPtr()> contextPreparer)
    {
    for (SubConditionP subCondition : subConditions)
        {
        if (ProcessSpecificationsByHash(rule, subCondition->GetSpecifications(), specificationHash, requested, tree, ecexpressionsCache, depth, specs, handled, stopProcessing, optParams, contextPreparer))
            return true;

        if (stopProcessing)
            return false;

        if (ProcessSpecificationsByHash(rule, subCondition->GetSubConditions(), specificationHash, requested, tree, ecexpressionsCache, depth, specs, handled, stopProcessing, optParams, contextPreparer))
            return true;

        if (stopProcessing)
            return false;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
RootNodeRuleSpecificationsList RulesPreprocessor::GetRootNodeSpecifications(RootNodeRuleParametersCR params)
    {
    bool handled = false;
    RootNodeRuleSpecificationsList specs;
    ECDbExpressionSymbolContext ecdbExpressionContext(params.GetConnection().GetECDb());
    std::function<ExpressionContextPtr()> contextPreparer = [&]()
        {
        ECExpressionContextsProvider::NodeRulesContextParameters contextParams(nullptr, params.GetConnection(), params.GetUserSettings(), params.GetUsedUserSettingsListener());
        return ECExpressionContextsProvider::GetNodeRulesContext(contextParams);
        };
    AddMatchingSpecifications(params.GetRuleset().GetRootNodesRules(), params.GetTargetTree(), params.GetECExpressionsCache(), specs, handled, nullptr, contextPreparer);
    return specs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRuleSpecificationsList RulesPreprocessor::GetChildNodeSpecifications(ChildNodeRuleParametersCR params)
    {
    bool handled = false;
    bool stopProcessing = false;
    ChildNodeRuleSpecificationsList specs;
    ECDbExpressionSymbolContext ecdbExpressionContext(params.GetConnection().GetECDb());
    std::function<ExpressionContextPtr()> contextPreparer = [&]()
        {
        ECExpressionContextsProvider::NodeRulesContextParameters contextParams(&params.GetParentNode(), params.GetConnection(), params.GetUserSettings(), params.GetUsedUserSettingsListener());
        return ECExpressionContextsProvider::GetNodeRulesContext(contextParams);
        };
    OptimizedExpressionsParameters optParams(params.GetConnections(), params.GetConnection(), &params.GetParentNode().GetKey(), "");

    NavNodeExtendedData parentNodeExtendedData(params.GetParentNode());
    if (parentNodeExtendedData.HasSpecificationHash())
        {
        AddSpecificationsByHierarchy(params.GetRuleset(), parentNodeExtendedData.GetSpecificationHash(), parentNodeExtendedData.GetRequestedSpecification(), 
            params.GetTargetTree(), params.GetECExpressionsCache(), specs, handled, stopProcessing, &optParams, contextPreparer);
        }

    if (!stopProcessing)
        AddMatchingSpecifications(params.GetRuleset().GetChildNodesRules(), params.GetTargetTree(), params.GetECExpressionsCache(), specs, handled, &optParams, contextPreparer);

    return specs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static bset<CustomizationRuleOrder<CustomizationRule>> GetNestedCustomizationRules(PresentationRuleSetCR ruleset, Utf8CP specificationHash)
    {
    ChildNodeRuleList childNodeRules = GetChildNodeRules(ruleset);
    bset<CustomizationRuleOrder<CustomizationRule>> customizationRules;
    FindCustomizationRules(customizationRules, childNodeRules, specificationHash,  0);
    return customizationRules;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename Rule>
struct Visitor : CustomizationRuleVisitor
{
private:
    bvector<Rule const*>& m_list;
protected:
    void _Visit (Rule const& rule) override {m_list.push_back(&rule);}
public:
    Visitor(bvector<Rule const*>& list) : m_list(list){}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
LabelOverrideCP RulesPreprocessor::GetLabelOverride(CustomizationRuleParametersCR params)
    {
    ECDbExpressionSymbolContext ecdbExpressionContext(params.GetConnection().GetECDb());
    std::function<ExpressionContextPtr()> contextPreparer = [&]()
        {
        ECExpressionContextsProvider::CustomizationRulesContextParameters contextParams(params.GetNode(), params.GetParentNode(), params.GetConnection(), params.GetUserSettings(), params.GetUsedUserSettingsListener());
        return ECExpressionContextsProvider::GetCustomizationRulesContext(contextParams);
        };
    bset<CustomizationRuleOrder<CustomizationRule>> customizationRules;
    OptimizedExpressionsParameters optParams(params.GetConnections(), params.GetConnection(), &params.GetNode().GetKey(), "");

    //Finds nested customization rules
    NavNodeExtendedData nodeExtendedData(params.GetNode());
    if (nodeExtendedData.HasSpecificationHash())
        customizationRules = GetNestedCustomizationRules(params.GetRuleset(), nodeExtendedData.GetSpecificationHash());

    //Adds root level customization rules
    for (LabelOverrideP rule : params.GetRuleset().GetLabelOverrides())
        customizationRules.insert(CustomizationRuleOrder<CustomizationRule>(rule, 0));

    //Filters LabelOverideRules
    bvector<LabelOverrideCP> labelOverrides;
    Visitor<LabelOverride> visitor(labelOverrides);
    for (CustomizationRuleOrder<CustomizationRule> const& rule : customizationRules)
        rule.GetRule()->Accept(visitor);

    for (LabelOverrideCP override : labelOverrides)
        {
        if (override->GetLabel().empty() && override->GetDescription().empty())
            continue; // invalid if neither label nor description are set

        if (override->GetCondition().empty() || VerifyCondition(override->GetCondition().c_str(), params.GetECExpressionsCache(), &optParams, contextPreparer))
            return override;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StyleOverrideCP RulesPreprocessor::GetStyleOverride(CustomizationRuleParametersCR params)
    {
    ECDbExpressionSymbolContext ecdbExpressionContext(params.GetConnection().GetECDb());
    std::function<ExpressionContextPtr()> contextPreparer = [&]() 
        {
        ECExpressionContextsProvider::CustomizationRulesContextParameters contextParams(params.GetNode(), params.GetParentNode(), params.GetConnection(), params.GetUserSettings(), params.GetUsedUserSettingsListener());
        return ECExpressionContextsProvider::GetCustomizationRulesContext(contextParams);
        };
    bset<CustomizationRuleOrder<CustomizationRule>> customizationRules;
    OptimizedExpressionsParameters optParams(params.GetConnections(), params.GetConnection(), &params.GetNode().GetKey(), "");

    //Finds nested customization rules
    NavNodeExtendedData nodeExtendedData(params.GetNode());
    if (nodeExtendedData.HasSpecificationHash())
        customizationRules = GetNestedCustomizationRules(params.GetRuleset(), nodeExtendedData.GetSpecificationHash());

    //Adds root level customization rules
    for (StyleOverrideP rule : params.GetRuleset().GetStyleOverrides())
        customizationRules.insert(CustomizationRuleOrder<CustomizationRule>(rule,0));

    //Filters StyleOverrideRules
    bvector<StyleOverrideCP> styleOverrides;
    Visitor<StyleOverride> visitor(styleOverrides);
    for (CustomizationRuleOrder<CustomizationRule> const& rule : customizationRules)
        rule.GetRule()->Accept(visitor);

    for (StyleOverrideCP override : styleOverrides)
        {
        if (override->GetCondition().empty() || VerifyCondition(override->GetCondition().c_str(), params.GetECExpressionsCache(), &optParams, contextPreparer))
            return override;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<GroupingRuleCP> RulesPreprocessor::GetGroupingRules(AggregateCustomizationRuleParametersCR params)
    {
    std::function<ExpressionContextPtr()> contextPreparer = [&]()
        {
        ECExpressionContextsProvider::NodeRulesContextParameters contextParams(params.GetParentNode(), params.GetConnection(), params.GetUserSettings(), params.GetUsedUserSettingsListener());
        return ECExpressionContextsProvider::GetNodeRulesContext(contextParams);
        };
    OptimizedExpressionsParameters optParams(params.GetConnections(), params.GetConnection(), nullptr == params.GetParentNode() ? nullptr : &params.GetParentNode()->GetKey(), "");

    //Finds nested customization rules
    bset<CustomizationRuleOrder<CustomizationRule>> customizationRules = GetNestedCustomizationRules(params.GetRuleset(), params.GetSpecificationHash().c_str());

    //Adds root level customization rules
    for (GroupingRuleP rule : params.GetRuleset().GetGroupingRules())
        customizationRules.insert(CustomizationRuleOrder<CustomizationRule>(rule, 0));

    //Filters GroupingRules
    bvector<GroupingRuleCP> groupingRules;
    Visitor<GroupingRule> visitor(groupingRules);
    for (CustomizationRuleOrder<CustomizationRule> const& rule : customizationRules)
        rule.GetRule()->Accept(visitor);

    bvector<GroupingRuleCP> matchingGroupingRules;
    for (GroupingRuleCP  rule : groupingRules)
        {
        if (rule->GetOnlyIfNotHandled() && !matchingGroupingRules.empty())
            continue;

        if (rule->GetCondition().empty() || VerifyCondition(rule->GetCondition().c_str(), params.GetECExpressionsCache(), &optParams, contextPreparer))
            matchingGroupingRules.push_back(rule);
        }
    return matchingGroupingRules;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<SortingRuleCP> RulesPreprocessor::GetSortingRules(AggregateCustomizationRuleParametersCR params)
    {
    std::function<ExpressionContextPtr()> contextPreparer = [&]()
        {
        ECExpressionContextsProvider::NodeRulesContextParameters contextParams(params.GetParentNode(), params.GetConnection(), params.GetUserSettings(), params.GetUsedUserSettingsListener());
        return ECExpressionContextsProvider::GetNodeRulesContext(contextParams);
        };
    OptimizedExpressionsParameters optParams(params.GetConnections(), params.GetConnection(), nullptr == params.GetParentNode() ? nullptr : &params.GetParentNode()->GetKey(), "");

    //Finds nested customization rules
    bset<CustomizationRuleOrder<CustomizationRule>> customizationRules = GetNestedCustomizationRules(params.GetRuleset(), params.GetSpecificationHash().c_str());

    //Adds root level customization rules    
    for (SortingRuleP rule : params.GetRuleset().GetSortingRules())
        customizationRules.insert(CustomizationRuleOrder<CustomizationRule>(rule, 0));

    //Filters SortingRules
    bvector<SortingRuleCP> sortingRules;
    Visitor<SortingRule> visitor(sortingRules);
    for (CustomizationRuleOrder<CustomizationRule> const& rule : customizationRules)
        rule.GetRule()->Accept(visitor);   

    bvector<SortingRuleCP> matchingSortingRules;
    for (SortingRuleCP  rule : sortingRules)
        {
        if (rule->GetCondition().empty() || VerifyCondition(rule->GetCondition().c_str(), params.GetECExpressionsCache(), &optParams, contextPreparer))
            matchingSortingRules.push_back(rule);
        }
    return matchingSortingRules;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
LocalizationResourceKeyDefinitionCP RulesPreprocessor::GetLocalizationResourceKeyDefinition(Utf8StringCR id, PresentationRuleSetCR ruleset)
    {
    LocalizationResourceKeyDefinitionList const& definitions = ruleset.GetLocalizationResourceKeyDefinitions();
    for (LocalizationResourceKeyDefinitionCP definition : definitions)
        {
        if (definition->GetId().Equals(id))
            return definition;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ImageIdOverrideCP RulesPreprocessor::GetImageIdOverride(CustomizationRuleParametersCR params)
    {
    ECDbExpressionSymbolContext ecdbExpressionContext(params.GetConnection().GetECDb());
    std::function<ExpressionContextPtr()> contextPreparer = [&]() 
        {
        ECExpressionContextsProvider::CustomizationRulesContextParameters contextParams(params.GetNode(), params.GetParentNode(), params.GetConnection(), params.GetUserSettings(), params.GetUsedUserSettingsListener());
        return ECExpressionContextsProvider::GetCustomizationRulesContext(contextParams);
        };
    bset<CustomizationRuleOrder<CustomizationRule>> customizationRules;  
    NavNodeExtendedData nodeExtendedData(params.GetNode());
    OptimizedExpressionsParameters optParams(params.GetConnections(), params.GetConnection(), &params.GetNode().GetKey(), "");

    //Finds nested customization rules
    if (nodeExtendedData.HasSpecificationHash())
        customizationRules = GetNestedCustomizationRules(params.GetRuleset(), nodeExtendedData.GetSpecificationHash());

    //Adds root level customization rules
    for (ImageIdOverrideP tempImageIdOverride : params.GetRuleset().GetImageIdOverrides())
        customizationRules.insert(CustomizationRuleOrder<CustomizationRule>(tempImageIdOverride, 0));

    //Filters ImageIdOverride Rules
    bvector<ImageIdOverrideCP> imageIdOverrides;
    Visitor<ImageIdOverride> visitor(imageIdOverrides);
    for (CustomizationRuleOrder<CustomizationRule> const& rule : customizationRules)
        rule.GetRule()->Accept(visitor);

    for (ImageIdOverrideCP override : imageIdOverrides)
        {
        if (override->GetCondition().empty() || VerifyCondition(override->GetCondition().c_str(), params.GetECExpressionsCache(), &optParams, contextPreparer))
            return override;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CheckBoxRuleCP RulesPreprocessor::GetCheckboxRule(CustomizationRuleParametersCR params)
    {
    ECDbExpressionSymbolContext ecdbExpressionContext(params.GetConnection().GetECDb());
    std::function<ExpressionContextPtr()> contextPreparer = [&]()
        {
        ECExpressionContextsProvider::CustomizationRulesContextParameters contextParams(params.GetNode(), params.GetParentNode(), params.GetConnection(), params.GetUserSettings(), params.GetUsedUserSettingsListener());
        return ECExpressionContextsProvider::GetCustomizationRulesContext(contextParams);
        };
    bset<CustomizationRuleOrder<CustomizationRule>> customizationRules;
    NavNodeExtendedData nodeExtendedData(params.GetNode());
    OptimizedExpressionsParameters optParams(params.GetConnections(), params.GetConnection(), &params.GetNode().GetKey(), "");

    //Adds nested customization rules
    if (nodeExtendedData.HasSpecificationHash())
        customizationRules = GetNestedCustomizationRules(params.GetRuleset(), nodeExtendedData.GetSpecificationHash());

    //Adds root level customization rules
    for (CheckBoxRuleP checkbox : params.GetRuleset().GetCheckBoxRules())
        customizationRules.insert(CustomizationRuleOrder<CustomizationRule>(checkbox, 0));

    //Filters CheckBoxRules
    bvector<CheckBoxRuleCP> checkboxRules;
    Visitor<CheckBoxRule> visitor(checkboxRules);
    for (CustomizationRuleOrder<CustomizationRule> const& rule : customizationRules)
        rule.GetRule()->Accept(visitor);

    for (CheckBoxRuleCP rule : checkboxRules)
        {
        if (rule->GetCondition().empty() || VerifyCondition(rule->GetCondition().c_str(), params.GetECExpressionsCache(), &optParams, contextPreparer))
            return rule;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRuleSpecificationsList RulesPreprocessor::GetContentSpecifications(ContentRuleParametersCR params)
    {
    ECDbExpressionSymbolContext ecdbExpressionContext(params.GetConnection().GetECDb());

    ContentRuleSpecificationsList specs;
    bset<NavNodeKeyCP> handledNodes;

    if (params.HasSelectionInfo())
        {
        for (NavNodeKeyCPtr const& selectedNodeKey : params.GetSelectedNodeKeys())
            {
            std::function<ExpressionContextPtr()> contextPreparer = [&]()
                {
                ECExpressionContextsProvider::ContentRulesContextParameters contextParams(params.GetPreferredDisplayType().c_str(), params.GetSelectionProviderName().c_str(),
                    params.IsSubSelection(), params.GetConnection(), params.GetNodeLocater(), selectedNodeKey.get(), params.GetUserSettings(), params.GetUsedUserSettingsListener());
                return ECExpressionContextsProvider::GetContentRulesContext(contextParams);
                };
            OptimizedExpressionsParameters optimizedParams(params.GetConnections(), params.GetConnection(), selectedNodeKey.get(), params.GetPreferredDisplayType().c_str());

            for (ContentRuleCP rule : params.GetRuleset().GetContentRules())
                {
                if (rule->GetOnlyIfNotHandled() && handledNodes.end() != handledNodes.find(selectedNodeKey.get()))
                    continue;

                if (rule->GetCondition().empty() || VerifyCondition(rule->GetCondition().c_str(), params.GetECExpressionsCache(), &optimizedParams, contextPreparer))
                    {
                    auto iter = specs.find(*rule);
                    if (specs.end() == iter)
                        iter = specs.insert(*rule).first;

                    iter->GetMatchingSelectedNodeKeys().push_back(selectedNodeKey);
                    handledNodes.insert(selectedNodeKey.get());
                    }
                }
            }
        }
    std::function<ExpressionContextPtr()> contextPreparer = [&]() 
        {
        ECExpressionContextsProvider::ContentRulesContextParameters contextParams(params.GetPreferredDisplayType().c_str(), "", false,
            params.GetConnection(), params.GetNodeLocater(), nullptr, params.GetUserSettings(), params.GetUsedUserSettingsListener());
        return ECExpressionContextsProvider::GetContentRulesContext(contextParams);
        };

    for (ContentRuleCP rule : params.GetRuleset().GetContentRules())
        {
        if (rule->GetOnlyIfNotHandled() && !specs.empty())
            continue;

        if (rule->GetCondition().empty() || VerifyCondition(rule->GetCondition().c_str(), params.GetECExpressionsCache(), nullptr, contextPreparer))
            specs.insert(*rule);
        }

    return specs;
    }
