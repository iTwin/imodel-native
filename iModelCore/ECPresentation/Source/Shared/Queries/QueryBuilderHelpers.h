/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include "PresentationQuery.h"
#include "../ECSchemaHelper.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define RULES_ENGINE_LOCAL_STATE_NAMESPACE                  "RulesEngine"
#define RULES_ENGINE_ACTIVE_GROUPS_LOCAL_STATE_NAMESPACE    RULES_ENGINE_LOCAL_STATE_NAMESPACE "_ActiveGroups"

#define RULES_ENGINE_RELATED_CLASS_ALIAS(cls, counter)      Utf8PrintfString("rel_%s_%s_%" PRIu64, (cls).GetSchema().GetAlias().c_str(), (cls).GetName().c_str(), (uint64_t)(counter))
#define RULES_ENGINE_NAV_CLASS_ALIAS(cls, counter)          Utf8PrintfString("nav_%s_%s_%" PRIu64, (cls).GetSchema().GetAlias().c_str(), (cls).GetName().c_str(), (uint64_t)(counter))

#define RELATED_CLASS_PATH_ALIASES_THRESHOLD 50

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct InputFilteringParams
{
private:
    ECInstanceKey m_inputKey;
    std::shared_ptr<RelatedClassPath const> m_pathToInputClass;
    bvector<ECInstanceId> m_selectIds;
public:
    InputFilteringParams(ECInstanceKey inputKey, std::shared_ptr<RelatedClassPath const> pathToInputClass, bvector<ECInstanceId> selectIds)
        : m_inputKey(std::move(inputKey)), m_pathToInputClass(std::move(pathToInputClass)), m_selectIds(std::move(selectIds))
        {}
    ECInstanceKeyCR GetInputKey() const { return m_inputKey; }
    RelatedClassPath const* GetPathToInputClass() const { return m_pathToInputClass.get(); }
    bvector<ECInstanceId> const& GetSelectIds() const { return m_selectIds; }
};

/*=================================================================================**//**
* deprecated
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RecursiveQueryInfo
{
private:
    bvector<RelatedClassPath> m_pathsFromInputToSelectClass;
public:
    RecursiveQueryInfo(bvector<RelatedClassPath> pathsFromInputToSelectClass) : m_pathsFromInputToSelectClass(pathsFromInputToSelectClass) {}
    bvector<RelatedClassPath> const& GetPathsFromInputToSelectClass() const { return m_pathsFromInputToSelectClass; }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ClassSortingRule
{
private:
    SelectClass<ECClass> const* m_class;
    SortingRuleCP m_rule;
public:
    ClassSortingRule() : m_class(nullptr), m_rule(nullptr) {}
    ClassSortingRule(SelectClass<ECClass> const& selectClass, SortingRuleCR rule) : m_class(&selectClass), m_rule(&rule) {}
    SelectClass<ECClass> const& GetSelectClass() const { return *m_class; }
    SortingRuleCR GetRule() const { return *m_rule; }
 };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RecursiveQueriesHelper
{
private:
    IConnectionCR m_connection;
    RecursiveQueryInfo const& m_recursiveQueryInfo;

private:
    static bool IsRecursiveJoinForward(SelectClassInfo const& selectInfo);
    static BentleyStatus ParseRecursiveRelationships(bset<ECRelationshipClassCP>& relationships, bool& isForward, RecursiveQueryInfo const& info);
    static void RecursivelySelectRelatedKeys(std::unordered_map<ECInstanceId, std::unordered_set<ECInstanceId>>& result, IConnectionCR connection, Utf8StringCR partialQuery, bvector<ECInstanceId> const& sourceIds);
    static void RecursivelySelectRelatedKeys(std::unordered_map<ECInstanceId, std::unordered_set<ECInstanceId>>& result, IConnectionCR connection, Utf8StringCR partialQuery, std::unordered_map<ECInstanceId, bvector<ECInstanceId>> const& sourceIds);
    static Utf8String CreatePartialRecursiveIdsQuery(bset<ECRelationshipClassCP> const& relationships, bool isForward);
    static bvector<ECInstanceId> GetRecursiveTargetIds(IConnectionCR connection, bset<ECRelationshipClassCP> const& relationships, bool isForward, bvector<ECInstanceId> const& sourceIds);
    static std::unordered_map<ECInstanceId, bvector<ECInstanceId>> GetRecursiveTargetIdsGroupedBySourceId(IConnectionCR connection, bset<ECRelationshipClassCP> const& relationships, bool isForward, bvector<ECInstanceId> const& sourceIds);

public:
    RecursiveQueriesHelper(IConnectionCR connection, RecursiveQueryInfo const& recursiveQueryInfo)
        : m_connection(connection), m_recursiveQueryInfo(recursiveQueryInfo)
        {}
    bvector<ECInstanceId> DEPRECATED_GetRecursiveTargetIds(IParsedInput const& input, SelectClassInfo const& thisInfo) const;
    std::unordered_map<ECInstanceId, bvector<ECInstanceId>> GetRecursiveTargetIdsGroupedBySourceId(bvector<ECInstanceId> const& sourceIds) const;
    bvector<ECInstanceId> GetRecursiveTargetIds(bvector<ECInstanceId> const& sourceIds) const;
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct InstanceFilteringParams
{
private:
    ECExpressionsCache& m_ecexpressionsCache;
    Utf8CP m_instanceFilter;
    ExpressionContext* m_expressionContext;
    InputFilteringParams const* m_inputFilter;
    InstanceFilterDefinition const* m_instanceFilterDefinition;
    SelectClassWithExcludes<ECClass> m_selectClass;

public:
    InstanceFilteringParams(ECExpressionsCache& ecexpressionsCache, ExpressionContext* expressionContext, Utf8CP instanceFilter, InputFilteringParams const* inputFilter)
        : m_ecexpressionsCache(ecexpressionsCache), m_instanceFilter(instanceFilter), m_expressionContext(expressionContext), m_inputFilter(inputFilter),
        m_instanceFilterDefinition(nullptr)
        {}
    InstanceFilteringParams(ECExpressionsCache& ecexpressionsCache, ExpressionContext* expressionContext, Utf8CP instanceFilter, InputFilteringParams const* inputFilter,
        InstanceFilterDefinition const* instanceFilterDefinition, SelectClassWithExcludes<ECClass> const& selectClass)
        : m_ecexpressionsCache(ecexpressionsCache), m_instanceFilter(instanceFilter), m_expressionContext(expressionContext), m_inputFilter(inputFilter),
        m_instanceFilterDefinition(instanceFilterDefinition), m_selectClass(selectClass)
        {}

    ECExpressionsCache& GetECExpressionsCache() const {return m_ecexpressionsCache;}
    ExpressionContext* GetECExpressionContext() const {return m_expressionContext;}
    Utf8CP GetInstanceFilter() const {return m_instanceFilter;}
    InputFilteringParams const* GetInputFilter() const {return m_inputFilter;}
    InstanceFilterDefinition const* GetInstanceFilterDefinition() const {return m_instanceFilterDefinition;}
    SelectClassWithExcludes<ECClass> const& GetSelectClass() const {return m_selectClass;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SelectClassSplitResult
{
private:
    bvector<SelectClassWithExcludes<ECClass>> m_splitPath;
    SelectClassWithExcludes<ECClass> m_selectClass;
public:
    SelectClassSplitResult() {}
    SelectClassSplitResult(SelectClassWithExcludes<ECClass> selectClass) : m_selectClass(selectClass) {}
    SelectClassWithExcludes<ECClass> const& GetSelectClass() const {return m_selectClass;}
    SelectClassWithExcludes<ECClass>& GetSelectClass() {return m_selectClass;}
    bvector<SelectClassWithExcludes<ECClass>> const& GetSplitPath() const {return m_splitPath;}
    bvector<SelectClassWithExcludes<ECClass>>& GetSplitPath() {return m_splitPath;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct QueryBuilderHelpers
{
private:
    QueryBuilderHelpers() {}

public:
    template<typename T> static Utf8String GetOrderByClause(T const& query);
    template<typename T> static void AddToUnionSet(QuerySet<T>& set, T& query);
    template<typename T> static void SetOrUnion(RefCountedPtr<T>& target, T& source);
    template<typename T> static void Where(RefCountedPtr<T>& query, Utf8CP clause, BoundQueryValuesListCR bindings);
    template<typename T> static void Where(RefCountedPtr<T>& query, QueryClauseAndBindings);
    template<typename T> static void Order(T& query, Utf8CP clause);
    template<typename T> static void RemoveOrdering(T& query);
    template<typename T> static void Limit(T& query, uint64_t limit, uint64_t offset = 0);

    template<typename T> static RefCountedPtr<ComplexPresentationQuery<T>> CreateNestedQuery(T& innerQuery);
    template<typename T> static bool NeedsNestingToUseAlias(T const& query, bvector<Utf8String> const& aliases);
    template<typename T> static RefCountedPtr<T> CreateNestedQueryIfNecessary(T& query, bvector<Utf8String> const& aliases);
    template<typename T> static RefCountedPtr<ComplexPresentationQuery<T>> CreateComplexNestedQueryIfNecessary(T& query, bvector<Utf8String> const& aliases);

    static std::unique_ptr<InputFilteringParams> CreateInputFilter(IConnectionCR, SelectClassInfo const&, RecursiveQueryInfo const*, IParsedInput const&);
    template<typename T> static void ApplyInstanceFilter(ComplexPresentationQuery<T>&, InstanceFilteringParams const&);

    static bvector<SelectClassSplitResult> ProcessSelectClassesBasedOnCustomizationRules(bvector<SelectClassWithExcludes<ECClass>> const& selectClasses,
        bvector<RuleApplicationInfo> const& customizationRuleInfos, SchemaManagerCR schemas);
    static bvector<RelatedClassPath> ProcessRelationshipPathsBasedOnCustomizationRules(bvector<RelatedClassPath> const& relationshipPaths,
        bvector<RuleApplicationInfo> const& customizationRuleInfos, SchemaManagerCR);

    static void ApplyPagingOptions(RefCountedPtr<ContentQuery>& query, PageOptionsCR opts);
    static void Aggregate(ContentDescriptorPtr& aggregateDescriptor, ContentDescriptorR inputDescriptor);

    static bmap<ECClassCP, bvector<InstanceLabelOverride const*>> GetLabelOverrideValuesMap(ECSchemaHelper const& helper, bvector<InstanceLabelOverrideCP>);
    static bvector<InstanceLabelOverrideValueSpecification const*> GetInstanceLabelOverrideSpecsForClass(bmap<ECClassCP, bvector<InstanceLabelOverride const*>> const& instanceLabelOverrides, ECClassCR ecClass);
    ECPRESENTATION_EXPORT static PresentationQueryContractFieldPtr CreateDisplayLabelField(Utf8CP name, ECSchemaHelper const&, SelectClass<ECClass> const&,
        PresentationQueryContractFieldCPtr classIdField, PresentationQueryContractFieldCPtr instanceIdField, bvector<RelatedClassPath> const& relatedInstancePaths,
        bvector<InstanceLabelOverrideValueSpecification const*> const& labelOverrideValueSpecs, bvector<ECInstanceKey> const& labelRequestsStack = {});
    ECPRESENTATION_EXPORT static Utf8String CreateDisplayLabelValueClause(Utf8StringCR fieldName);

    static IdSet<BeInt64Id> CreateIdSetFromJsonArray(RapidJsonValueCR);
    static ECValue CreateECValueFromJson(RapidJsonValueCR);

    static GroupSpecificationCP GetActiveGroupingSpecification(GroupingRuleCR, IJsonLocalState const*);

    static QueryClauseAndBindings CreatePropertyGroupFilteringClause(ECPropertyCR ecProperty, Utf8StringCR propertyValueSelector, PropertyGroupCR groupingSpecification, RapidJsonValueCR groupingValues);

    static void CollectRelatedClassPathAliases(bset<Utf8String>& aliases, bvector<RelatedClass> const& classes);
    static void CollectRelatedClassPathAliases(bset<Utf8String>& aliases, bvector<RelatedClassPath> const& relatedPaths);
    static Utf8String CreatePropertySortingClause(SortingRuleCR rule, SelectClass<ECClass> selectClass, ECPropertyCR ecProperty);
    static Utf8String CreatePropertySortingClause(bvector<ClassSortingRule> const& rules, SelectClassWithExcludes<ECClass> selectClass);
    static bvector<ClassSortingRule> GetClassSortingRules(bvector<SortingRuleCP> const& rules, SelectClassWithExcludes<ECClass> const& selectClass, bvector<RelatedClassPath> const& relatedPaths, ECSchemaHelper const& helper);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
