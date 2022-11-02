/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationManager.h>
#include "../RulesEngineTypes.h"
#include "ECSchemaHelper.h"
#include "RulesPreprocessor.h"
#include "ExtendedData.h"

//===================================================================================
//! Customization rule with nesting depth.
// @bsiclass
//===================================================================================
template<typename RuleType>
struct CustomizationRuleOrder
{
private:
    RuleType const* m_rule;
    int m_depth;

public:
    CustomizationRuleOrder() : m_rule(nullptr), m_depth(0) {}
    CustomizationRuleOrder(RuleType const* rule, int depth) : m_rule(rule), m_depth(depth) {}
    RuleType const* GetRule() const { return m_rule; }
    int GetDepth() const { return m_depth; }
    bool operator<(CustomizationRuleOrder const& rhs) const
        {
        if (m_rule->GetPriority() == rhs.GetRule()->GetPriority())
            return m_depth >= rhs.GetDepth();
        return m_rule->GetPriority() > rhs.GetRule()->GetPriority();
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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

template <typename RuleType> static void MergeNodeRuleContents(RuleType& target, RuleType const& source);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void MergeNodeSpecifications(ChildNodeSpecificationR target, ChildNodeSpecificationCR source)
    {
    bvector<ChildNodeRuleP> const& targetNestedRules = target.GetNestedRules();
    for (ChildNodeRuleP rule : source.GetNestedRules())
        {
        auto duplicateRule = std::find_if(targetNestedRules.begin(), targetNestedRules.end(), [rule](ChildNodeRuleP targetRule) {return targetRule->ShallowEqual(*rule);});
        if (targetNestedRules.end() == duplicateRule)
            target.AddNestedRule(*new ChildNodeRule(*rule));
        else
            MergeNodeRuleContents(**duplicateRule, *rule);
        }

    bvector<RelatedInstanceSpecificationP> const& targetRelatedSpecs = target.GetRelatedInstances();
    for (RelatedInstanceSpecificationP spec : source.GetRelatedInstances())
        {
        auto duplicateSpec = std::find_if(targetRelatedSpecs.begin(), targetRelatedSpecs.end(), [spec](RelatedInstanceSpecificationP targetSpec) {return targetSpec->ShallowEqual(*spec); });
        if (targetRelatedSpecs.end() == duplicateSpec)
            target.AddRelatedInstance(*new RelatedInstanceSpecification(*spec));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename RuleType> static void MergeNodeRuleContents(RuleType& target, RuleType const& source)
    {
    bvector<ChildNodeSpecificationP> const& targetSpecs = target.GetSpecifications();
    for (ChildNodeSpecificationP spec : source.GetSpecifications())
        {
        auto duplicateSpec = std::find_if(targetSpecs.begin(), targetSpecs.end(), [spec](ChildNodeSpecificationP targetSpec) {return targetSpec->ShallowEqual(*spec);});
        if (targetSpecs.end() == duplicateSpec)
            target.AddSpecification(*spec->Clone());
        else
            MergeNodeSpecifications(**duplicateSpec, *spec);
        }

    bvector<SubConditionP> const& targetSubConditions = target.GetSubConditions();
    for (SubConditionP condition : source.GetSubConditions())
        {
        auto duplicateCondition = std::find_if(targetSubConditions.begin(), targetSubConditions.end(), [condition](SubConditionP targetCondition) {return targetCondition->ShallowEqual(*condition); });
        if (targetSubConditions.end() == duplicateCondition)
            target.AddSubCondition(*new SubCondition(*condition));
        else
            MergeNodeRuleContents(**duplicateCondition, *condition);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename RuleType> static bvector<RuleType*> MergeDuplicateNodeRules(bvector<RuleType*> const& rules)
    {
    bvector<RuleType*> uniqueRules;
    bvector<RuleType*> duplicateRules;
    for (RuleType* rule : rules)
        {
        auto duplicateRule = std::find_if(uniqueRules.begin(), uniqueRules.end(), [rule](RuleType* uniqueRule) {return uniqueRule->ShallowEqual(*rule);});
        if (uniqueRules.end() == duplicateRule)
            {
            uniqueRules.push_back(rule);
            }
        else
            {
            MergeNodeRuleContents(**duplicateRule, *rule);
            duplicateRules.push_back(rule);
            }
        }

    return duplicateRules;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void MergeDuplicateNodeRules(PresentationRuleSetR ruleSet)
    {
    bvector<RootNodeRuleP> duplicateRootRules = MergeDuplicateNodeRules(ruleSet.GetRootNodesRules());
    for (RootNodeRuleP rule : duplicateRootRules)
        {
        ruleSet.RemovePresentationRule(*rule);
        delete rule;
        }

    bvector<ChildNodeRuleP> duplicateChildNodeRules = MergeDuplicateNodeRules(ruleSet.GetChildNodesRules());
    for (ChildNodeRuleP rule : duplicateChildNodeRules)
        {
        ruleSet.RemovePresentationRule(*rule);
        delete rule;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static PresentationRuleSetPtr CreateSupplementedRuleSet(PresentationRuleSetCR primary, bvector<PresentationRuleSetPtr> const& supplemental)
    {
    PresentationRuleSetPtr ruleset = primary.Clone();
    for (PresentationRuleSetPtr const& supplementalSet : supplemental)
        ruleset->MergeWith(*supplementalSet);
    return ruleset;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool MeetsSchemaRequirements(ECDbCR ecdb, RequiredSchemaSpecificationsList const& requirements, Utf8StringCR containerIdentifier)
    {
    bool meetsRequirements = true;
    for (auto schemaRequirement : requirements)
        {
        auto schema = ecdb.Schemas().GetSchema(schemaRequirement->GetName(), false);
        if (!schema)
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_INFO, Utf8PrintfString("Omitting %s - schema requirement is not met. "
                "Rule requires `%s` schema which is not present in the dataset.", containerIdentifier.c_str(),
                schemaRequirement->GetName().c_str()));
            meetsRequirements = false;
            break;
            }
        if (schemaRequirement->GetMinVersion().IsValid())
            {
            SchemaKey minKey(schema->GetName().c_str(), schemaRequirement->GetMinVersion().Value().GetMajor(), schemaRequirement->GetMinVersion().Value().GetMinor(), schemaRequirement->GetMinVersion().Value().GetPatch());
            if (schema->GetSchemaKey().CompareByVersion(minKey) < 0)
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_INFO, Utf8PrintfString("Omitting %s - schema requirement is not met. "
                    "Rule requires `%s` schema version to be at least %s. Actual schema version is %s.", containerIdentifier.c_str(),
                    schema->GetName().c_str(), minKey.GetVersionString().c_str(), schema->GetSchemaKey().GetVersionString().c_str()));
                meetsRequirements = false;
                break;
                }
            }
        if (schemaRequirement->GetMaxVersion().IsValid())
            {
            SchemaKey maxKey(schema->GetName().c_str(), schemaRequirement->GetMaxVersion().Value().GetMajor(), schemaRequirement->GetMaxVersion().Value().GetMinor(), schemaRequirement->GetMaxVersion().Value().GetPatch());
            if (schema->GetSchemaKey().CompareByVersion(maxKey) >= 0)
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_INFO, Utf8PrintfString("Omitting %s - schema requirement is not met. "
                    "Rule requires `%s` schema version to be less than %s. Actual schema version is %s.", containerIdentifier.c_str(),
                    schema->GetName().c_str(), maxKey.GetVersionString().c_str(), schema->GetSchemaKey().GetVersionString().c_str()));
                meetsRequirements = false;
                break;
                }
            }
        }
    return meetsRequirements;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool MeetsSchemaRequirements(ECDbCR ecdb, RequiredSchemaSpecificationsList const& requirements, PresentationKeyCR validatedRule)
    {
    return MeetsSchemaRequirements(ecdb, requirements, DiagnosticsHelpers::CreateRuleIdentifier(validatedRule));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool MeetsSchemaRequirements(ECDbCR ecdb, RequiredSchemaSpecificationsList const& requirements, PresentationRuleSetCR ruleset)
    {
    return MeetsSchemaRequirements(ecdb, requirements, ruleset.GetFullRuleSetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Version const& GetRulesetVersion(PresentationRuleSetCR ruleset)
    {
    static Version s_default;
    return ruleset.GetRulesetVersion().IsValid() ? ruleset.GetRulesetVersion().Value() : s_default;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationRuleSetPtr RulesPreprocessor::GetPresentationRuleSet(IRulesetLocaterManager const& locaters, IConnectionCR connection, Utf8CP rulesetId)
    {
    ECSchemaHelper schemaHelper(connection, nullptr, nullptr);
    bvector<PresentationRuleSetPtr> rulesets = locaters.LocateRuleSets(connection, rulesetId);
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, LOG_DEBUG, Utf8PrintfString("Located %" PRIu64 " rulesets with id '%s'", (uint64_t)rulesets.size(), rulesetId));

    // find the primary ruleset
    PresentationRuleSetPtr primary;
    for (PresentationRuleSetPtr& ruleset : rulesets)
        {
        if (ruleset->GetIsSupplemental())
            continue;

        if (ruleset->GetRulesetSchemaVersion().GetMajor() > PresentationRuleSet::GetCurrentRulesetSchemaVersion().GetMajor())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_INFO, Utf8PrintfString("Skipping %s - ruleset schema major version (%s) "
                "is higher than latest supported major version of the library (%s).", ruleset->GetFullRuleSetId().c_str(),
                ruleset->GetRulesetSchemaVersion().ToString().c_str(), PresentationRuleSet::GetCurrentRulesetSchemaVersion().ToString().c_str()));
            continue;
            }

        if (!MeetsSchemaRequirements(connection.GetECDb(), ruleset->GetRequiredSchemaSpecifications(), *ruleset))
            continue;

        if (!schemaHelper.AreSchemasSupported(ruleset->GetSupportedSchemas()))
            continue;

        if (primary.IsValid() && GetRulesetVersion(*primary) >= GetRulesetVersion(*ruleset))
            continue;

        primary = ruleset;
        }

    if (primary.IsNull())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, LOG_INFO, "Primary ruleset not found");
        return nullptr;
        }

    // look for supplemental rule sets
    bmap<Utf8String, PresentationRuleSetPtr> supplementalRuleSets;
    for (PresentationRuleSetPtr& ruleset : rulesets)
        {
        if (!ruleset->GetIsSupplemental())
            continue;

        if (ruleset->GetRulesetSchemaVersion().GetMajor() > PresentationRuleSet::GetCurrentRulesetSchemaVersion().GetMajor())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Rules, LOG_DEBUG, LOG_INFO, Utf8PrintfString("Skipping %s - ruleset schema major version (%s) "
                "is higher than latest supported major version of the library (%s).", ruleset->GetFullRuleSetId().c_str(),
                ruleset->GetRulesetSchemaVersion().ToString().c_str(), PresentationRuleSet::GetCurrentRulesetSchemaVersion().ToString().c_str()));
            continue;
            }

        if (!MeetsSchemaRequirements(connection.GetECDb(), ruleset->GetRequiredSchemaSpecifications(), *ruleset))
            continue;

        if (!schemaHelper.AreSchemasSupported(ruleset->GetSupportedSchemas()))
            continue;

        auto iter = supplementalRuleSets.find(ruleset->GetSupplementationPurpose());
        if (supplementalRuleSets.end() != iter && GetRulesetVersion(*iter->second) >= GetRulesetVersion(*ruleset))
            continue;

        supplementalRuleSets[ruleset->GetSupplementationPurpose()] = ruleset;
        }
    auto supplementalRuleSetsList = ContainerHelpers::TransformContainer<bvector<PresentationRuleSetPtr>>(supplementalRuleSets, [](auto const& entry){return entry.second;});
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, LOG_DEBUG, Utf8PrintfString("Identified %" PRIu64 " supplemental rulesets.", (uint64_t)supplementalRuleSetsList.size()));

    PresentationRuleSetPtr supplementedRuleSet = supplementalRuleSets.empty() ? primary : CreateSupplementedRuleSet(*primary, supplementalRuleSetsList);
    MergeDuplicateNodeRules(*supplementedRuleSet);
    return supplementedRuleSet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
        DIAGNOSTICS_LOG(DiagnosticsCategory::ECExpressions, LOG_DEBUG, LOG_ERROR, Utf8PrintfString("Failed to evaluate ECExpression: %s", condition));
        return false;
        }

    ECValue value;
    if (ExpressionStatus::Success != valueResult->GetECValue(value))
        return false;

    return value.IsBoolean() && value.GetBoolean();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename RuleType>
bool RulesPreprocessor::AddMatchingSpecifications(SpecificationsLookupContext const& context, bvector<RuleType*> const& rules, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled)
    {
    for (RuleType const* rule : rules)
        {
        if (!MeetsSchemaRequirements(context.GetECDb(), rule->GetRequiredSchemaSpecifications(), *rule))
            continue;

        if (rule->GetTargetTree() != context.GetRuleTargetTree() && rule->GetTargetTree() != TargetTree_Both)
            continue;

        if (rule->GetOnlyIfNotHandled() && (handled || specs.size() > 0))
            continue;

        if (!rule->GetCondition().empty() && !VerifyCondition(rule->GetCondition().c_str(), context.GetECExpressionsCahce(), context.GetOptimizedExpressionsParams(), context.GetExpressionsContextPreparer()))
            continue;

        if (rule->GetStopFurtherProcessing())
            return true;

        DiagnosticsHelpers::ReportRule(*rule);
        for (ChildNodeSpecificationP spec : rule->GetSpecifications())
            PrioritySortedAdd(specs, NavigationRuleSpecification<RuleType>(*spec, *rule));

        handled = true;

        ProcessSubConditions(context, *rule, rule->GetSubConditions(), specs);
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename RuleType>
bool RulesPreprocessor::AddSpecificationsByHierarchy(SpecificationsLookupContext const& context, bvector<RuleType*> const& rules, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled, bool& stopProcessing, unsigned depth)
    {
    if (stopProcessing)
        return false;

    bool specificationFound = false;
    for (RuleType* rule : rules)
        {
        if (!MeetsSchemaRequirements(context.GetECDb(), rule->GetRequiredSchemaSpecifications(), *rule))
            continue;

        specificationFound = ProcessSpecificationsByHash(context, *rule, rule->GetSpecifications(), specs, handled, stopProcessing, depth);
        if (stopProcessing)
            return false;

        if (specificationFound)
            break;

        specificationFound = ProcessSpecificationsByHash(context, *rule, rule->GetSubConditions(), specs, handled, stopProcessing, depth);
        if (stopProcessing)
            return false;

        if (specificationFound)
            {
            DiagnosticsHelpers::ReportRule(*rule);
            break;
            }
        }

    return specificationFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename RuleType>
void RulesPreprocessor::ProcessSubConditions(SpecificationsLookupContext const& context, RuleType const& rule, SubConditionList const& subConditions, bvector<NavigationRuleSpecification<RuleType>>& specs)
    {
    for (SubConditionP subCondition : subConditions)
        {
        if (!MeetsSchemaRequirements(context.GetECDb(), subCondition->GetRequiredSchemaSpecifications(), *subCondition))
            continue;

        if (!subCondition->GetCondition().empty() && !VerifyCondition(subCondition->GetCondition().c_str(), context.GetECExpressionsCahce(), context.GetOptimizedExpressionsParams(), context.GetExpressionsContextPreparer()))
            continue;

        for (ChildNodeSpecificationP spec : subCondition->GetSpecifications())
            PrioritySortedAdd(specs, NavigationRuleSpecification<RuleType>(*spec, rule));

        ProcessSubConditions(context, rule, subCondition->GetSubConditions(), specs);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename RuleType>
bool RulesPreprocessor::ProcessSpecificationsByHash(SpecificationsLookupContext const& context, RuleType const& rule, ChildNodeSpecificationList const& searchIn, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled, bool& stopProcessing, unsigned depth)
    {
    for (ChildNodeSpecificationP specification : searchIn)
        {
        if (specification->GetHash().Equals(context.GetSpecificationHash()))
            {
            if (context.IsRequestedSpecification())
                PrioritySortedAdd(specs, ChildNodeRuleSpecification(*specification, rule));
            else
                stopProcessing = AddMatchingSpecifications(context, specification->GetNestedRules(), specs, handled);
            stopProcessing |= context.IsRequestedSpecification();
            return true;
            }
        if (AddSpecificationsByHierarchy(context, specification->GetNestedRules(), specs, handled, stopProcessing, depth + 1))
            {
            return true;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename RuleType>
bool RulesPreprocessor::ProcessSpecificationsByHash(SpecificationsLookupContext const& context, RuleType const& rule, SubConditionList const& subConditions, bvector<NavigationRuleSpecification<RuleType>>& specs, bool& handled, bool& stopProcessing, unsigned depth)
    {
    for (SubConditionP subCondition : subConditions)
        {
        if (!MeetsSchemaRequirements(context.GetECDb(), subCondition->GetRequiredSchemaSpecifications(), *subCondition))
            continue;

        if (ProcessSpecificationsByHash(context, rule, subCondition->GetSpecifications(), specs, handled, stopProcessing, depth))
            return true;

        if (stopProcessing)
            return false;

        if (ProcessSpecificationsByHash(context, rule, subCondition->GetSubConditions(), specs, handled, stopProcessing, depth))
            return true;

        if (stopProcessing)
            return false;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<ChildNodeRuleP> GetAllHierarchyRulesAtRootLevel(PresentationRuleSetCR ruleset)
    {
    bvector<ChildNodeRuleP> childNodeRules;
    for (RootNodeRule* rule : ruleset.GetRootNodesRules())
        childNodeRules.push_back(rule);
    for (ChildNodeRule* rule : ruleset.GetChildNodesRules())
        childNodeRules.push_back(rule);
    return childNodeRules;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool FindCustomizationRules(ECDbCR ecdb, bset<CustomizationRuleOrder<CustomizationRule>>& customizationRules,
    bvector<ChildNodeRuleP> const& childNodeRules, bvector<Utf8String> const& specificationHashes, int depth)
    {
    for (ChildNodeRule* rule : childNodeRules)
        {
        for (ChildNodeSpecificationP spec : rule->GetSpecifications())
            {
            if (ContainerHelpers::Contains(specificationHashes, spec->GetHash()) || FindCustomizationRules(ecdb, customizationRules, spec->GetNestedRules(), specificationHashes, depth + 1))
                {
                for (CustomizationRuleP customization : rule->GetCustomizationRules())
                    {
                    if (MeetsSchemaRequirements(ecdb, customization->GetRequiredSchemaSpecifications(), *customization))
                        customizationRules.insert(CustomizationRuleOrder<CustomizationRule>(customization, depth));
                    }
                return true;
                }
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RootNodeRuleSpecificationsList RulesPreprocessor::_GetRootNodeSpecifications(RootNodeRuleParametersCR params)
    {
    bool handled = false;
    RootNodeRuleSpecificationsList specs;
    std::function<ExpressionContextPtr()> contextPreparer = [&]()
        {
        ECExpressionContextsProvider::NodeRulesContextParameters contextParams(nullptr, m_connection,
            m_rulesetVariables, m_usedVariablesListener);
        return ECExpressionContextsProvider::GetNodeRulesContext(contextParams);
        };
    SpecificationsLookupContext context(m_connection.GetECDb(), params.GetTargetTree(), m_ecexpressionsCache, nullptr, false, nullptr, contextPreparer);
    AddMatchingSpecifications(context, m_ruleset.GetRootNodesRules(), specs, handled);
    return specs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeRuleSpecificationsList RulesPreprocessor::_GetChildNodeSpecifications(ChildNodeRuleParametersCR params)
    {
    bool handled = false;
    bool stopProcessing = false;
    ChildNodeRuleSpecificationsList specs;
    std::function<ExpressionContextPtr()> contextPreparer = [&]()
        {
        ECExpressionContextsProvider::NodeRulesContextParameters contextParams(static_cast<NavNodeCP>(&params.GetParentNode()), m_connection,
            m_rulesetVariables, m_usedVariablesListener);
        return ECExpressionContextsProvider::GetNodeRulesContext(contextParams);
        };
    OptimizedExpressionsParameters optParams(m_connections, m_connection, params.GetParentNode().GetKey(), "");

    NavNodeExtendedData parentNodeExtendedData(params.GetParentNode());
    SpecificationsLookupContext specificationContext(m_connection.GetECDb(), params.GetTargetTree(), m_ecexpressionsCache, params.GetParentNode().GetKey()->GetSpecificationIdentifier().c_str(),
        parentNodeExtendedData.GetRequestedSpecification(), &optParams, contextPreparer);
    AddSpecificationsByHierarchy(specificationContext, GetAllHierarchyRulesAtRootLevel(m_ruleset), specs, handled, stopProcessing, 0);

    if (!stopProcessing)
        {
        SpecificationsLookupContext context(m_connection.GetECDb(), params.GetTargetTree(), m_ecexpressionsCache, nullptr, false, &optParams, contextPreparer);
        AddMatchingSpecifications(context, m_ruleset.GetChildNodesRules(), specs, handled);
        }

    return specs;
    }

#define RETURN_IF_NOT_NULL(expr) \
    { \
    auto result = expr; \
    if (result != nullptr) \
        return result; \
    }

template<typename TRule> static ChildNodeSpecificationCP FindSpecificationByHash(bvector<TRule> const& rules, Utf8StringCR hash);
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ChildNodeSpecificationCP FindSpecificationByHash(ChildNodeSpecificationList const& specs, Utf8StringCR hash)
    {
    for (auto const& spec : specs)
        {
        if (spec->GetHash().Equals(hash))
            return spec;

        RETURN_IF_NOT_NULL(FindSpecificationByHash(spec->GetNestedRules(), hash));
        }
    return nullptr;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TRule> static ChildNodeSpecificationCP FindSpecificationByHash(bvector<TRule> const& rules, Utf8StringCR hash)
    {
    for (auto const& rule : rules)
        {
        RETURN_IF_NOT_NULL(FindSpecificationByHash(rule->GetSpecifications(), hash));
        for (auto const& subCond : rule->GetSubConditions())
            RETURN_IF_NOT_NULL(FindSpecificationByHash(subCond->GetSpecifications(), hash));
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ChildNodeSpecificationCP RulesPreprocessor::_FindChildNodeSpecification(Utf8StringCR specificationHash)
    {
    RETURN_IF_NOT_NULL(FindSpecificationByHash(m_ruleset.GetRootNodesRules(), specificationHash));
    RETURN_IF_NOT_NULL(FindSpecificationByHash(m_ruleset.GetChildNodesRules(), specificationHash));
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bset<CustomizationRuleOrder<CustomizationRule>> GetNestedCustomizationRules(ECDbCR ecdb, PresentationRuleSetCR ruleset, bvector<Utf8String> const& specificationHashes)
    {
    ChildNodeRuleList childNodeRules = GetAllHierarchyRulesAtRootLevel(ruleset);
    bset<CustomizationRuleOrder<CustomizationRule>> customizationRules;
    FindCustomizationRules(ecdb, customizationRules, childNodeRules, specificationHashes, 0);
    return customizationRules;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename Rule>
struct ConcreteCustomizationRuleFilter : CustomizationRuleVisitor
{
private:
    bvector<Rule const*>& m_list;
protected:
    void _Visit(Rule const& rule) override {m_list.push_back(&rule);}
public:
    ConcreteCustomizationRuleFilter(bvector<Rule const*>& list) : m_list(list){}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TRule>
static bvector<TRule const*> GetRootRules(ECDbCR ecdb, PresentationRuleSetCR ruleset, bvector<TRule*> const& (PresentationRuleSet::*rootRulesGetter)() const)
    {
    bvector<TRule const*> rules;
    ConcreteCustomizationRuleFilter<TRule> filter(rules);
    for (TRule const* rule : (ruleset.*rootRulesGetter)())
        {
        if (MeetsSchemaRequirements(ecdb, rule->GetRequiredSchemaSpecifications(), *rule))
            rule->Accept(filter);
        }
    return rules;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TRule>
static bvector<TRule const*> GetCustomizationRules(ECDbCR ecdb, bvector<Utf8String> const& specificationHashes, PresentationRuleSetCR ruleset, bvector<TRule*> const& (PresentationRuleSet::*rootRulesGetter)() const)
    {
    // adds customization rules specified under specific specs
    bset<CustomizationRuleOrder<CustomizationRule>> customizationRules = GetNestedCustomizationRules(ecdb, ruleset, specificationHashes);

    // adds root level customization rules
    for (TRule* rule : (ruleset.*rootRulesGetter)())
        {
        if (MeetsSchemaRequirements(ecdb, rule->GetRequiredSchemaSpecifications(), *rule))
            customizationRules.insert(CustomizationRuleOrder<CustomizationRule>(rule, 0));
        }

    // filters specific type of rules
    bvector<TRule const*> concreteRules;
    ConcreteCustomizationRuleFilter<TRule> filter(concreteRules);
    for (CustomizationRuleOrder<CustomizationRule> const& rule : customizationRules)
        rule.GetRule()->Accept(filter);
    return concreteRules;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TRule>
static bvector<TRule const*> GetCustomizationRules(ECDbCR ecdb, NavNodeCR node, PresentationRuleSetCR ruleset, bvector<TRule*> const& (PresentationRuleSet::*rootRulesGetter)() const)
    {
    return GetCustomizationRules(ecdb, { node.GetKey()->GetSpecificationIdentifier() }, ruleset, rootRulesGetter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LabelOverrideCP RulesPreprocessor::_GetLabelOverride(CustomizationRuleByNodeParametersCR params)
    {
    std::function<ExpressionContextPtr()> contextPreparer = [&]()
        {
        ECExpressionContextsProvider::CustomizationRulesContextParameters contextParams(static_cast<NavNodeCR>(params.GetNode()), static_cast<NavNodeCP>(params.GetParentNode()),
            m_connection, m_rulesetVariables, m_usedVariablesListener);
        return ECExpressionContextsProvider::GetCustomizationRulesContext(contextParams);
        };
    OptimizedExpressionsParameters optParams(m_connections, m_connection, params.GetNode().GetKey(), "");
    bvector<LabelOverrideCP> labelOverrides = GetCustomizationRules(m_connection.GetECDb(), params.GetNode(), m_ruleset, &PresentationRuleSet::GetLabelOverrides);
    for (LabelOverrideCP override : labelOverrides)
        {
        if (override->GetLabel().empty() && override->GetDescription().empty())
            continue; // invalid if neither label nor description are set

        if (override->GetCondition().empty() || VerifyCondition(override->GetCondition().c_str(), m_ecexpressionsCache, &optParams, contextPreparer))
            return override;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<LabelOverrideCP> RulesPreprocessor::_GetLabelOverrides()
    {
    return GetRootRules<LabelOverride>(m_connection.GetECDb(), m_ruleset, &PresentationRuleSet::GetLabelOverrides);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StyleOverrideCP RulesPreprocessor::_GetStyleOverride(CustomizationRuleByNodeParametersCR params)
    {
    std::function<ExpressionContextPtr()> contextPreparer = [&]()
        {
        ECExpressionContextsProvider::CustomizationRulesContextParameters contextParams(static_cast<NavNodeCR>(params.GetNode()), static_cast<NavNodeCP>(params.GetParentNode()),
            m_connection, m_rulesetVariables, m_usedVariablesListener);
        return ECExpressionContextsProvider::GetCustomizationRulesContext(contextParams);
        };
    OptimizedExpressionsParameters optParams(m_connections, m_connection, params.GetNode().GetKey(), "");
    bvector<StyleOverrideCP> styleOverrides = GetCustomizationRules(m_connection.GetECDb(), params.GetNode(), m_ruleset, &PresentationRuleSet::GetStyleOverrides);
    for (StyleOverrideCP override : styleOverrides)
        {
        if (override->GetCondition().empty() || VerifyCondition(override->GetCondition().c_str(), m_ecexpressionsCache, &optParams, contextPreparer))
            return override;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<StyleOverrideCP> RulesPreprocessor::_GetStyleOverrides()
    {
    return GetRootRules<StyleOverride>(m_connection.GetECDb(), m_ruleset, &PresentationRuleSet::GetStyleOverrides);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<GroupingRuleCP> RulesPreprocessor::_GetGroupingRules(AggregateCustomizationRuleByNodeParametersCR params)
    {
    std::function<ExpressionContextPtr()> contextPreparer = [&]()
        {
        ECExpressionContextsProvider::NodeRulesContextParameters contextParams(static_cast<NavNodeCP>(params.GetParentNode()), m_connection,
            m_rulesetVariables, m_usedVariablesListener);
        return ECExpressionContextsProvider::GetNodeRulesContext(contextParams);
        };
    OptimizedExpressionsParameters optParams(m_connections, m_connection, nullptr == params.GetParentNode() ? nullptr : params.GetParentNode()->GetKey(), "");
    bvector<GroupingRuleCP> groupingRules = GetCustomizationRules(m_connection.GetECDb(), params.GetSpecificationHashes(), m_ruleset, &PresentationRuleSet::GetGroupingRules);
    bvector<GroupingRuleCP> matchingGroupingRules;
    for (GroupingRuleCP  rule : groupingRules)
        {
        if (rule->GetOnlyIfNotHandled() && !matchingGroupingRules.empty())
            continue;

        if (rule->GetCondition().empty() || VerifyCondition(rule->GetCondition().c_str(), m_ecexpressionsCache, &optParams, contextPreparer))
            matchingGroupingRules.push_back(rule);
        }
    return matchingGroupingRules;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<GroupingRuleCP> RulesPreprocessor::_GetGroupingRules()
    {
    return GetRootRules<GroupingRule>(m_connection.GetECDb(), m_ruleset, &PresentationRuleSet::GetGroupingRules);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<SortingRuleCP> RulesPreprocessor::_GetSortingRules(AggregateCustomizationRuleByNodeParametersCR params)
    {
    std::function<ExpressionContextPtr()> contextPreparer = [&]()
        {
        ECExpressionContextsProvider::NodeRulesContextParameters contextParams(static_cast<NavNodeCP>(params.GetParentNode()), m_connection,
            m_rulesetVariables, m_usedVariablesListener);
        return ECExpressionContextsProvider::GetNodeRulesContext(contextParams);
        };
    OptimizedExpressionsParameters optParams(m_connections, m_connection, nullptr == params.GetParentNode() ? nullptr : params.GetParentNode()->GetKey(), "");
    bvector<SortingRuleCP> sortingRules = GetCustomizationRules(m_connection.GetECDb(), params.GetSpecificationHashes(), m_ruleset, &PresentationRuleSet::GetSortingRules);
    bvector<SortingRuleCP> matchingSortingRules;
    for (SortingRuleCP  rule : sortingRules)
        {
        if (rule->GetCondition().empty() || VerifyCondition(rule->GetCondition().c_str(), m_ecexpressionsCache, &optParams, contextPreparer))
            matchingSortingRules.push_back(rule);
        }
    return matchingSortingRules;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<SortingRuleCP> RulesPreprocessor::_GetSortingRules()
    {
    return GetRootRules<SortingRule>(m_connection.GetECDb(), m_ruleset, &PresentationRuleSet::GetSortingRules);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ImageIdOverrideCP RulesPreprocessor::_GetImageIdOverride(CustomizationRuleByNodeParametersCR params)
    {
    std::function<ExpressionContextPtr()> contextPreparer = [&]()
        {
        ECExpressionContextsProvider::CustomizationRulesContextParameters contextParams(static_cast<NavNodeCR>(params.GetNode()), static_cast<NavNodeCP>(params.GetParentNode()),
            m_connection, m_rulesetVariables, m_usedVariablesListener);
        return ECExpressionContextsProvider::GetCustomizationRulesContext(contextParams);
        };
    OptimizedExpressionsParameters optParams(m_connections, m_connection, params.GetNode().GetKey(), "");
    bvector<ImageIdOverrideCP> imageIdOverrides = GetCustomizationRules(m_connection.GetECDb(), params.GetNode(), m_ruleset, &PresentationRuleSet::GetImageIdOverrides);
    for (ImageIdOverrideCP override : imageIdOverrides)
        {
        if (override->GetCondition().empty() || VerifyCondition(override->GetCondition().c_str(), m_ecexpressionsCache, &optParams, contextPreparer))
            return override;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ImageIdOverrideCP> RulesPreprocessor::_GetImageIdOverrides()
    {
    return GetRootRules<ImageIdOverride>(m_connection.GetECDb(), m_ruleset, &PresentationRuleSet::GetImageIdOverrides);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CheckBoxRuleCP RulesPreprocessor::_GetCheckboxRule(CustomizationRuleByNodeParametersCR params)
    {
    std::function<ExpressionContextPtr()> contextPreparer = [&]()
        {
        ECExpressionContextsProvider::CustomizationRulesContextParameters contextParams(static_cast<NavNodeCR>(params.GetNode()), static_cast<NavNodeCP>(params.GetParentNode()),
            m_connection, m_rulesetVariables, m_usedVariablesListener);
        return ECExpressionContextsProvider::GetCustomizationRulesContext(contextParams);
        };
    OptimizedExpressionsParameters optParams(m_connections, m_connection, params.GetNode().GetKey(), "");
    bvector<CheckBoxRuleCP> checkboxRules = GetCustomizationRules(m_connection.GetECDb(), params.GetNode(), m_ruleset, &PresentationRuleSet::GetCheckBoxRules);
    for (CheckBoxRuleCP rule : checkboxRules)
        {
        if (rule->GetCondition().empty() || VerifyCondition(rule->GetCondition().c_str(), m_ecexpressionsCache, &optParams, contextPreparer))
            return rule;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<CheckBoxRuleCP> RulesPreprocessor::_GetCheckboxRules()
    {
    return GetRootRules<CheckBoxRule>(m_connection.GetECDb(), m_ruleset, &PresentationRuleSet::GetCheckBoxRules);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DefaultPropertyCategoryOverrideCP RulesPreprocessor::_GetDefaultPropertyCategoryOverride()
    {
    bset<CustomizationRuleOrder<DefaultPropertyCategoryOverride>> rules;
    for (auto rule : m_ruleset.GetDefaultPropertyCategoryOverrides())
        {
        if (MeetsSchemaRequirements(m_connection.GetECDb(), rule->GetRequiredSchemaSpecifications(), *rule))
            rules.insert(CustomizationRuleOrder<DefaultPropertyCategoryOverride>(rule, 0));
        }
    if (!rules.empty())
        return (*rules.begin()).GetRule();
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ExtendedDataRuleCP> RulesPreprocessor::_GetExtendedDataRules(CustomizationRuleByNodeParametersCR params)
    {
    std::function<ExpressionContextPtr()> contextPreparer = [&]()
        {
        ECExpressionContextsProvider::CustomizationRulesContextParameters contextParams(static_cast<NavNodeCR>(params.GetNode()), static_cast<NavNodeCP>(params.GetParentNode()),
            m_connection, m_rulesetVariables, m_usedVariablesListener);
        return ECExpressionContextsProvider::GetCustomizationRulesContext(contextParams);
        };
    OptimizedExpressionsParameters optParams(m_connections, m_connection, params.GetNode().GetKey(), "");
    bvector<ExtendedDataRuleCP> rules = GetCustomizationRules(m_connection.GetECDb(), params.GetNode(), m_ruleset, &PresentationRuleSet::GetExtendedDataRules);
    bvector<ExtendedDataRuleCP> matchingRules;
    for (ExtendedDataRuleCP rule : rules)
        {
        if (rule->GetOnlyIfNotHandled() && !matchingRules.empty())
            continue;

        if (rule->GetCondition().empty() || VerifyCondition(rule->GetCondition().c_str(), m_ecexpressionsCache, &optParams, contextPreparer))
            matchingRules.push_back(rule);
        }
    return matchingRules;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ExtendedDataRuleCP> RulesPreprocessor::_GetExtendedDataRules()
    {
    return GetRootRules<ExtendedDataRule>(m_connection.GetECDb(), m_ruleset, &PresentationRuleSet::GetExtendedDataRules);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NodeArtifactsRuleCP> RulesPreprocessor::_GetNodeArtifactRules(CustomizationRuleByNodeParametersCR params)
    {
    std::function<ExpressionContextPtr()> contextPreparer = [&]()
        {
        ECExpressionContextsProvider::CustomizationRulesContextParameters contextParams(static_cast<NavNodeCR>(params.GetNode()), static_cast<NavNodeCP>(params.GetParentNode()),
            m_connection, m_rulesetVariables, m_usedVariablesListener);
        return ECExpressionContextsProvider::GetCustomizationRulesContext(contextParams);
        };
    OptimizedExpressionsParameters optParams(m_connections, m_connection, params.GetNode().GetKey(), "");
    bvector<NodeArtifactsRuleCP> rules = GetCustomizationRules(m_connection.GetECDb(), params.GetNode(), m_ruleset, &PresentationRuleSet::GetNodeArtifactRules);
    bvector<NodeArtifactsRuleCP> matchingRules;
    for (NodeArtifactsRuleCP rule : rules)
        {
        if (rule->GetCondition().empty() || VerifyCondition(rule->GetCondition().c_str(), m_ecexpressionsCache, &optParams, contextPreparer))
            matchingRules.push_back(rule);
        }
    return matchingRules;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NodeArtifactsRuleCP> RulesPreprocessor::_GetNodeArtifactRules(CustomizationRuleBySpecParametersCR params)
    {
    return GetCustomizationRules(m_connection.GetECDb(), { params.GetSpecificationHash() }, m_ruleset, &PresentationRuleSet::GetNodeArtifactRules);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NodeArtifactsRuleCP> RulesPreprocessor::_GetNodeArtifactRules()
    {
    return GetRootRules<NodeArtifactsRule>(m_connection.GetECDb(), m_ruleset, &PresentationRuleSet::GetNodeArtifactRules);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ContentRuleCP> RulesPreprocessor::_GetContentRules()
    {
    bvector<ContentRuleCP> result;
    for (auto rule : m_ruleset.GetContentRules())
        {
        if (MeetsSchemaRequirements(m_connection.GetECDb(), rule->GetRequiredSchemaSpecifications(), *rule))
            result.push_back(rule);
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentRuleInputKeysContainer RulesPreprocessor::_GetContentSpecifications(ContentRuleParametersCR params)
    {
    ContentRuleInputKeysContainer specs;
    bset<NavNodeKeyCP> handledNodes;
    for (NavNodeKeyCPtr const& inputNodeKey : params.GetInputNodeKeys())
        {
        std::function<ExpressionContextPtr()> contextPreparer = [&]()
            {
            ECExpressionContextsProvider::ContentRulesContextParameters contextParams(params.GetPreferredDisplayType().c_str(), params.GetSelectionProviderName(), params.IsSubSelection(), m_connection, m_ruleset.GetRuleSetId(),
             params.GetNodeLocater(), inputNodeKey.get(), m_rulesetVariables, m_usedVariablesListener, params.GetNodeLabelCalculator());
            return ECExpressionContextsProvider::GetContentRulesContext(contextParams);
            };
        OptimizedExpressionsParameters optimizedParams(m_connections, m_connection, inputNodeKey.get(), params.GetPreferredDisplayType().c_str());
        for (ContentRuleCP rule : m_ruleset.GetContentRules())
            {
            if (!MeetsSchemaRequirements(m_connection.GetECDb(), rule->GetRequiredSchemaSpecifications(), *rule))
                continue;

            if (rule->GetOnlyIfNotHandled() && handledNodes.end() != handledNodes.find(inputNodeKey.get()))
                continue;

            if (rule->GetCondition().empty() || VerifyCondition(rule->GetCondition().c_str(), m_ecexpressionsCache, &optimizedParams, contextPreparer))
                {
                auto iter = specs.find(*rule);
                if (specs.end() == iter)
                    {
                    specs.push_back(*rule);
                    iter = std::prev(specs.end());
                    }

                iter->GetMatchingNodeKeys().push_back(inputNodeKey);
                handledNodes.insert(inputNodeKey.get());
                }
            }
        }

    std::function<ExpressionContextPtr()> contextPreparer = [&]()
        {
        ECExpressionContextsProvider::ContentRulesContextParameters contextParams(params.GetPreferredDisplayType().c_str(), "", false,
            m_connection, m_ruleset.GetRuleSetId(), params.GetNodeLocater(), nullptr, m_rulesetVariables, m_usedVariablesListener, params.GetNodeLabelCalculator());
        return ECExpressionContextsProvider::GetContentRulesContext(contextParams);
        };

    for (ContentRuleCP rule : m_ruleset.GetContentRules())
        {
        if (!MeetsSchemaRequirements(m_connection.GetECDb(), rule->GetRequiredSchemaSpecifications(), *rule))
            continue;

        if (rule->GetOnlyIfNotHandled() && !specs.empty())
            continue;

        if (rule->GetCondition().empty() || VerifyCondition(rule->GetCondition().c_str(), m_ecexpressionsCache, nullptr, contextPreparer))
            specs.push_back(*rule);
        }

    return specs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<InstanceLabelOverrideCP> RulesPreprocessor::_GetInstanceLabelOverrides()
    {
    bvector<InstanceLabelOverrideCP> result;
    for (auto modifier : m_ruleset.GetInstanceLabelOverrides())
        {
        if (MeetsSchemaRequirements(m_connection.GetECDb(), modifier->GetRequiredSchemaSpecifications(), *modifier))
            result.push_back(modifier);
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<InstanceLabelOverrideCP> RulesPreprocessor::_GetInstanceLabelOverrides(CustomizationRuleBySpecParametersCR params)
    {
    return GetCustomizationRules(m_connection.GetECDb(), { params.GetSpecificationHash() }, m_ruleset, &PresentationRuleSet::GetInstanceLabelOverrides);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ContentModifierCP> RulesPreprocessor::_GetContentModifiers()
    {
    bvector<ContentModifierCP> result;
    for (auto modifier : m_ruleset.GetContentModifierRules())
        {
        if (MeetsSchemaRequirements(m_connection.GetECDb(), modifier->GetRequiredSchemaSpecifications(), *modifier))
            result.push_back(modifier);
        }
    return result;
    }
