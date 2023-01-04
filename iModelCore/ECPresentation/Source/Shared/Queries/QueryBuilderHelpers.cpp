/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationManager.h>
#include <ECPresentation/LabelDefinition.h>
#include "../ECExpressions/ECExpressionContextsProvider.h"
#include "../../Hierarchies/NavigationQuery.h"
#include "QueryBuilderHelpers.h"
#include "QueryExecutor.h"
#include "CustomFunctions.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderHelpers::AddToUnionSet(QuerySet& set, PresentationQueryBuilder& query)
    {
    for (auto queryIter = set.GetQueries().begin(); queryIter != set.GetQueries().end(); ++queryIter)
        {
        if (GetOrderByClause(**queryIter) != GetOrderByClause(query))
            continue;

        if ((*queryIter)->AsUnionQueryBuilder() && (*queryIter)->AsUnionQueryBuilder()->GetQueries().size() < MAX_COMPOUND_STATEMENTS_COUNT)
            {
            (*queryIter)->AsUnionQueryBuilder()->AddQuery(query);
            return;
            }
        if (!(*queryIter)->AsUnionQueryBuilder())
            {
            *queryIter = UnionQueryBuilder::Create({ *queryIter, &query });
            return;
            }
        }
    set.GetQueries().push_back(&query);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderHelpers::SetOrUnion(RefCountedPtr<PresentationQueryBuilder>& target, PresentationQueryBuilder& source)
    {
    if (target.IsNull())
        target = &source;
    else if (target->AsUnionQueryBuilder())
        target->AsUnionQueryBuilder()->AddQuery(source);
    else
        target = UnionQueryBuilder::Create({target, &source});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderHelpers::Where(RefCountedPtr<PresentationQueryBuilder>& query, QueryClauseAndBindings clause)
    {
    if (clause.GetClause().empty())
        return;

    if (nullptr != query->AsComplexQueryBuilder())
        {
        query->AsComplexQueryBuilder()->Where(clause);
        return;
        }

    RefCountedPtr<ComplexQueryBuilder> wrapper = ComplexQueryBuilder::Create();
    wrapper->SelectAll();
    wrapper->From(*query);
    wrapper->Where(clause);
    query = wrapper;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderHelpers::Where(RefCountedPtr<PresentationQueryBuilder>& query, Utf8CP clause, BoundQueryValuesListCR bindings)
    {
    Where(query, QueryClauseAndBindings(clause, bindings));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderHelpers::Order(PresentationQueryBuilder& query, Utf8CP clause)
    {
    if (nullptr != query.AsComplexQueryBuilder())
        query.AsComplexQueryBuilder()->OrderBy(clause);
    else if (nullptr != query.AsUnionQueryBuilder())
        query.AsUnionQueryBuilder()->OrderBy(clause);
    else if (nullptr != query.AsExceptQueryBuilder())
        query.AsExceptQueryBuilder()->OrderBy(clause);
    else
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, LOG_ERROR, "Unexpected query type");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderHelpers::RemoveOrdering(PresentationQueryBuilder& query)
    {
    if (query.AsComplexQueryBuilder() || query.AsUnionQueryBuilder() || query.AsExceptQueryBuilder())
        Order(query, "");
    if (query.AsComplexQueryBuilder() && query.AsComplexQueryBuilder()->GetNestedQuery())
        RemoveOrdering(*query.AsComplexQueryBuilder()->GetNestedQuery());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String QueryBuilderHelpers::GetOrderByClause(PresentationQueryBuilder const& query)
    {
    if (nullptr != query.AsComplexQueryBuilder())
        return query.AsComplexQueryBuilder()->GetClause(CLAUSE_OrderBy);
    if (nullptr != query.AsUnionQueryBuilder())
        return query.AsUnionQueryBuilder()->GetOrderByClause();
    if (nullptr != query.AsExceptQueryBuilder())
        return query.AsExceptQueryBuilder()->GetOrderByClause();
    return "";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderHelpers::Limit(PresentationQueryBuilder& query, uint64_t limit, uint64_t offset)
    {
    if (nullptr != query.AsComplexQueryBuilder())
        query.AsComplexQueryBuilder()->Limit(limit, offset);
    else if (nullptr != query.AsUnionQueryBuilder())
        query.AsUnionQueryBuilder()->Limit(limit, offset);
    else if (nullptr != query.AsExceptQueryBuilder())
        query.AsExceptQueryBuilder()->Limit(limit, offset);
    else
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, LOG_ERROR, "Unexpected query type");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ComplexQueryBuilder> QueryBuilderHelpers::CreateNestedQuery(PresentationQueryBuilder& innerQuery)
    {
    RefCountedPtr<ComplexQueryBuilder> query = ComplexQueryBuilder::Create();
    query->SelectAll();
    query->From(innerQuery);
    return query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool CheckQueryForAliases(Utf8CP queryString, bvector<Utf8String> const& aliases)
    {
    // Check for keyword 'AS'
    int afterAsIndex = -1;
    size_t querySize = std::strlen(queryString);
    for (size_t i = 0; i < querySize; ++i)
        {
        if (std::isspace(queryString[i]) &&
            ('A' == queryString[i + 1] || 'a' == queryString[i + 1]) &&
            ('S' == queryString[i + 2] || 's' == queryString[i + 2]) &&
            std::isspace(queryString[i + 3]))
            {
            afterAsIndex = i + 4;
            break;
            }
        }

    if (-1 == afterAsIndex || afterAsIndex >= querySize)
        return false; // Keyword 'AS' not found

    // Get alias string
    Utf8CP afterAs = queryString + afterAsIndex;
    Utf8String alias = afterAs;
    size_t aliasEnd = -1;
    size_t afterAsSize = std::strlen(afterAs);
    for (size_t i = 0; i < afterAsSize; ++i)
        {
        if ('[' == afterAs[i])
            {
            for (size_t j = i+1; j < afterAsSize; ++j)
                {
                if (']' == afterAs[j] && i + 1 < afterAsSize)
                    {
                    alias = Utf8String(afterAs, i + 1, j - (i + 1));
                    aliasEnd = j + 1;
                    break;
                    }
                }
            if (-1 != aliasEnd)
                break;
            }
        else if ('_' != afterAs[i] && !isalnum(afterAs[i]))
            {
            alias = Utf8String(afterAs, i);
            aliasEnd = i + 1;
            break;
            }
        }

    // Trim whitespaces
    alias = alias.Trim();
    if (0 == alias.size())
        return false;

    // In case aliases aren't specified, check if alias is alphanumeric or _
    if (aliases.empty())
        return true;

    // Else check if alias matches any given alias
    for (auto const& aliasCase : aliases)
        {
        if (aliasCase.Equals(alias))
            return true;
        }

    if (aliasEnd < std::strlen(afterAs))
        return CheckQueryForAliases(afterAs + aliasEnd, aliases);

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool QueryBuilderHelpers::NeedsNestingToUseAlias(PresentationQueryBuilder const& query, bvector<Utf8String> const& aliases)
    {
    if (aliases.empty())
        return false;

    if (nullptr != query.AsStringQueryBuilder())
        return true;

    if (nullptr != query.AsUnionQueryBuilder())
        {
        UnionQueryBuilder const& unionQuery = *query.AsUnionQueryBuilder();
        for (auto const& q : unionQuery.GetQueries())
            {
            if (NeedsNestingToUseAlias(*q, aliases))
                return true;
            }
        }

    if (nullptr != query.AsExceptQueryBuilder())
        {
        ExceptQueryBuilder const& exceptQuery = *query.AsExceptQueryBuilder();
        if (NeedsNestingToUseAlias(*exceptQuery.GetBase(), aliases) || NeedsNestingToUseAlias(*exceptQuery.GetExcept(), aliases))
            return true;
        }

    if (nullptr != query.AsComplexQueryBuilder() && CheckQueryForAliases(query.AsComplexQueryBuilder()->GetClause(CLAUSE_Select).c_str(), aliases))
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<PresentationQueryBuilder> QueryBuilderHelpers::CreateNestedQueryIfNecessary(PresentationQueryBuilder& query, bvector<Utf8String> const& aliases)
    {
    if (NeedsNestingToUseAlias(query, aliases) || nullptr != query.AsStringQueryBuilder())
        return CreateNestedQuery(query);
    return &query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<ComplexQueryBuilder> QueryBuilderHelpers::CreateComplexNestedQueryIfNecessary(PresentationQueryBuilder& query, bvector<Utf8String> const& aliases)
    {
    RefCountedPtr<PresentationQueryBuilder> q = CreateNestedQueryIfNecessary(query, aliases);
    if (nullptr == q->AsComplexQueryBuilder())
        return CreateNestedQuery(query);
    return q->AsComplexQueryBuilder();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool RecursiveQueriesHelper::IsRecursiveJoinForward(SelectClassInfo const& selectInfo)
    {
    if (selectInfo.GetPathFromInputToSelectClass().empty())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Expected path from input to select class to be not empty");

    return selectInfo.GetPathFromInputToSelectClass().back().IsForwardRelationship();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RecursiveQueriesHelper::ParseRecursiveRelationships(bset<ECRelationshipClassCP>& relationships, bool& isForward, RecursiveQueryInfo const& info)
    {
    if (info.GetPathsFromInputToSelectClass().empty())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Expected 1 path from input to select class, got 0");

    if (info.GetPathsFromInputToSelectClass().size() > 1)
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, LOG_ERROR, Utf8PrintfString("Expected 1 path from input to select class, got %" PRIu64, (uint64_t)info.GetPathsFromInputToSelectClass().size()));

    for (RelatedClassPath const& path : info.GetPathsFromInputToSelectClass())
        {
        for (RelatedClassCR rel : path)
            relationships.insert(&rel.GetRelationship().GetClass());
        }
    isForward = info.GetPathsFromInputToSelectClass().back().back().IsForwardRelationship();
    return SUCCESS;
    }

#define RECURSIVE_IDS_QUERY_FIELD_NAME_SourceId "SourceId"
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RecursiveQueriesHelper::RecursivelySelectRelatedKeys(std::unordered_map<ECInstanceId, std::unordered_set<ECInstanceId>>& result, IConnectionCR connection, Utf8StringCR partialQuery, bvector<ECInstanceId> const& sourceIds)
    {
    Utf8String query(partialQuery);
    ValuesFilteringHelper filteringHelper(sourceIds);
    query.append("WHERE ").append(filteringHelper.CreateWhereClause(RECURSIVE_IDS_QUERY_FIELD_NAME_SourceId));

    CachedECSqlStatementPtr stmt = connection.GetStatementCache().GetPreparedStatement(connection.GetECDb().Schemas(), connection.GetDb(), query.c_str());
    if (stmt.IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to prepare a statement: '%s'", query.c_str()));

    BoundQueryValuesList bindings = filteringHelper.CreateBoundValues();
    if (SUCCESS != bindings.Bind(*stmt))
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Failed to bind params for recursive related keys selection query");

    std::unordered_map<ECInstanceId, bvector<ECInstanceId>> newIds;
    while (BE_SQLITE_ROW == QueryExecutorHelper::Step(*stmt))
        {
        ECInstanceId targetId = stmt->GetValueId<ECInstanceId>(0);
        ECInstanceId sourceId = stmt->GetValueId<ECInstanceId>(1);

        auto iter = result.find(sourceId);
        if (result.end() == iter)
            iter = result.insert(std::make_pair(sourceId, std::unordered_set<ECInstanceId>())).first;

        bool didInsert = iter->second.insert(targetId).second;
        if (didInsert)
            {
            auto newIdsIter = newIds.find(sourceId);
            if (newIds.end() == newIdsIter)
                newIdsIter = newIds.insert(std::make_pair(sourceId, bvector<ECInstanceId>())).first;
            newIdsIter->second.push_back(targetId);
            }
        }

    if (!newIds.empty())
        RecursivelySelectRelatedKeys(result, connection, partialQuery, newIds);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RecursiveQueriesHelper::RecursivelySelectRelatedKeys(std::unordered_map<ECInstanceId, std::unordered_set<ECInstanceId>>& result, IConnectionCR connection, Utf8StringCR partialQuery, std::unordered_map<ECInstanceId, bvector<ECInstanceId>> const& sourceIds)
    {
    bool hasNewIds = false;
    std::unordered_map<ECInstanceId, bvector<ECInstanceId>> newIds;
    for (auto const& entry : sourceIds)
        {
        auto resultsIter = result.find(entry.first);
        if (result.end() == resultsIter)
            resultsIter = result.insert(std::make_pair(entry.first, std::unordered_set<ECInstanceId>())).first;
        auto& resultSet = resultsIter->second;

        auto newIdsIter = newIds.find(entry.first);
        if (newIds.end() == newIdsIter)
            newIdsIter = newIds.insert(std::make_pair(entry.first, bvector<ECInstanceId>())).first;
        auto& newIdsVector = newIdsIter->second;

        Utf8String query(partialQuery);
        ValuesFilteringHelper filteringHelper(entry.second);
        query.append("WHERE ").append(filteringHelper.CreateWhereClause(RECURSIVE_IDS_QUERY_FIELD_NAME_SourceId));

        CachedECSqlStatementPtr stmt = connection.GetStatementCache().GetPreparedStatement(connection.GetECDb().Schemas(), connection.GetDb(), query.c_str());
        if (stmt.IsNull())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Failed to prepare a statement: '%s'", query.c_str()));

        BoundQueryValuesList bindings = filteringHelper.CreateBoundValues();
        if (SUCCESS != bindings.Bind(*stmt))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Failed to bind params for recursive related keys selection query");

        while (BE_SQLITE_ROW == QueryExecutorHelper::Step(*stmt))
            {
            ECInstanceId targetId = stmt->GetValueId<ECInstanceId>(0);
            bool didInsert = resultSet.insert(targetId).second;
            if (didInsert)
                {
                newIdsVector.push_back(targetId);
                hasNewIds = true;
                }
            }
        }

    if (hasNewIds)
        RecursivelySelectRelatedKeys(result, connection, partialQuery, newIds);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String RecursiveQueriesHelper::CreatePartialRecursiveIdsQuery(bset<ECRelationshipClassCP> const& relationships, bool isForward)
    {
    static Utf8CP s_sourceECInstanceIdField = "SourceECInstanceId";
    static Utf8CP s_targetECInstanceIdField = "TargetECInstanceId";
    bool first = true;
    Utf8String query("SELECT * ");
    query.append("FROM (");
    for (ECRelationshipClassCP rel : relationships)
        {
        if (!first)
            query.append(" UNION ALL ");
        query.append("SELECT ");
        query.append(isForward ? s_targetECInstanceIdField : s_sourceECInstanceIdField).append(" AS TargetId, ");
        query.append(isForward ? s_sourceECInstanceIdField : s_targetECInstanceIdField).append(" AS " RECURSIVE_IDS_QUERY_FIELD_NAME_SourceId);
        query.append(" FROM ").append(rel->GetECSqlName());
        first = false;
        }
    query.append(") ");
    return query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECInstanceId> RecursiveQueriesHelper::GetRecursiveTargetIds(IConnectionCR connection, bset<ECRelationshipClassCP> const& relationships, bool isForward, bvector<ECInstanceId> const& sourceIds)
    {
    std::unordered_map<ECInstanceId, std::unordered_set<ECInstanceId>> setResult;
    RecursivelySelectRelatedKeys(setResult, connection, CreatePartialRecursiveIdsQuery(relationships, isForward), { std::make_pair(ECInstanceId(), sourceIds) });

    if (setResult.empty())
        return {};

    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, setResult.size() == 1, "Expected all target IDs to be grouped under one input ID");
    return ContainerHelpers::TransformContainer<bvector<ECInstanceId>>((*setResult.begin()).second);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unordered_map<ECInstanceId, bvector<ECInstanceId>> RecursiveQueriesHelper::GetRecursiveTargetIdsGroupedBySourceId(IConnectionCR connection, bset<ECRelationshipClassCP> const& relationships, bool isForward, bvector<ECInstanceId> const& sourceIds)
    {
    std::unordered_map<ECInstanceId, std::unordered_set<ECInstanceId>> setResult;
    RecursivelySelectRelatedKeys(setResult, connection, CreatePartialRecursiveIdsQuery(relationships, isForward), sourceIds);

    std::unordered_map<ECInstanceId, bvector<ECInstanceId>> vectorResult;
    for (auto const& entry : setResult)
        vectorResult.insert(std::make_pair(entry.first, ContainerHelpers::TransformContainer<bvector<ECInstanceId>>(entry.second)));
    return vectorResult;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECInstanceId> RecursiveQueriesHelper::DEPRECATED_GetRecursiveTargetIds(IParsedInput const& input, SelectClassInfo const& thisInfo) const
    {
    Savepoint txn(m_connection.GetDb(), "RecursiveQueriesHelper::DEPRECATED_GetRecursiveTargetIds");
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, txn.IsActive(), "Failed to start transaction");

    bset<ECRelationshipClassCP> relationships;
    for (RelatedClassPath const& path : m_recursiveQueryInfo.GetPathsFromInputToSelectClass())
        {
        for (RelatedClassCR rel : path)
            relationships.insert(&rel.GetRelationship().GetClass());
        }
    return GetRecursiveTargetIds(m_connection, relationships, IsRecursiveJoinForward(thisInfo), input.GetInstanceIds(*thisInfo.GetInputClass()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unordered_map<ECInstanceId, bvector<ECInstanceId>> RecursiveQueriesHelper::GetRecursiveTargetIdsGroupedBySourceId(bvector<ECInstanceId> const& sourceIds) const
    {
    std::unordered_map<ECInstanceId, bvector<ECInstanceId>> result;
    bset<ECRelationshipClassCP> relationships;
    bool handleRelationshipsInForwardDirection;
    if (SUCCESS != ParseRecursiveRelationships(relationships, handleRelationshipsInForwardDirection, m_recursiveQueryInfo))
        return result;

    Savepoint txn(m_connection.GetDb(), "RecursiveQueriesHelper::GetRecursiveTargetIdsGroupedBySourceId");
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, txn.IsActive(), "Failed to start transaction");

    return GetRecursiveTargetIdsGroupedBySourceId(m_connection, relationships, handleRelationshipsInForwardDirection, sourceIds);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECInstanceId> RecursiveQueriesHelper::GetRecursiveTargetIds(bvector<ECInstanceId> const& sourceIds) const
    {
    bvector<ECInstanceId> result;
    bset<ECRelationshipClassCP> relationships;
    bool handleRelationshipsInForwardDirection;
    if (SUCCESS != ParseRecursiveRelationships(relationships, handleRelationshipsInForwardDirection, m_recursiveQueryInfo))
        return result;

    Savepoint txn(m_connection.GetDb(), "RecursiveQueriesHelper::GetRecursiveTargetIds");
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, txn.IsActive(), "Failed to start transaction");

    return GetRecursiveTargetIds(m_connection, relationships, handleRelationshipsInForwardDirection, sourceIds);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<InputFilteringParams> QueryBuilderHelpers::CreateInputFilter(IConnectionCR connection, SelectClassInfo const& selectInfo, RecursiveQueryInfo const* recursiveQueryInfo, IParsedInput const& input)
    {
    if (selectInfo.GetPathFromInputToSelectClass().empty())
        {
        // this code path is generally used for SelectedNodeInstances specification - we just
        // need to filter by selected instance ids
        ECClassCR selectClass = selectInfo.GetSelectClass().GetClass();
        if (!ContainerHelpers::Contains(input.GetClasses(), [&selectClass](auto const& inputClass) {return selectClass.Is(inputClass); }))
            return nullptr;
        return std::make_unique<InputFilteringParams>(ECInstanceKey(), nullptr, input.GetInstanceIds(selectClass));
        }

    // this code path is generally used for ContentRelatedInstances specification - we need to filter either by
    // input ids or ids in the path-from-input-to-select-class
    if (nullptr != recursiveQueryInfo)
        {
        // deprecated path:
        // in case of recursive query just bind the children ids
        RecursiveQueriesHelper recursiveQueries(connection, *recursiveQueryInfo);
        bvector<ECInstanceId> ids = recursiveQueries.DEPRECATED_GetRecursiveTargetIds(input, selectInfo);
        if (ids.empty())
            return nullptr;
        return std::make_unique<InputFilteringParams>(ECInstanceKey(), nullptr, ids);
        }

    // non-deprecated path for ContentRelatedInstances:
    RelatedClassPath pathFromSelectToInputClass(selectInfo.GetPathFromInputToSelectClass());
    pathFromSelectToInputClass.Reverse("related", false);

    // the reversed path always becomes shorter if there are recursive relationships involved. if not, then
    // we just need to filter by the end of the join
    if (pathFromSelectToInputClass.size() == selectInfo.GetPathFromInputToSelectClass().size())
        {
        bvector<ECInstanceId> const& ids = input.GetInstanceIds(*selectInfo.GetInputClass());
        if (ids.empty())
            return nullptr;
        return std::make_unique<InputFilteringParams>(selectInfo.GetPathFromInputToSelectClass().GetInputKey(), std::make_unique<RelatedClassPath>(pathFromSelectToInputClass), ids);
        }

    // reversed path becomes empty only if the first step is recursive in which case it means we have
    // to filter 'this' by path-from-input-to-select-class target ids
    if (pathFromSelectToInputClass.empty())
        {
        if (selectInfo.GetPathFromInputToSelectClass().empty())
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Expected size of path from input to select class to be 1, got 0");
        if (selectInfo.GetPathFromInputToSelectClass().size() > 1)
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, LOG_ERROR, Utf8PrintfString("Expected size of path from input to select class to be 1, got: %" PRIu64, (uint64_t)selectInfo.GetPathFromInputToSelectClass().size()));
        auto const& ids = selectInfo.GetPathFromInputToSelectClass().back().GetTargetIds();
        if (ids.empty())
            return nullptr;
        return std::make_unique<InputFilteringParams>(selectInfo.GetPathFromInputToSelectClass().GetInputKey(), nullptr, ids);
        }

    // otherwise the appropriate filtering gets applied when the reversed path is joined to the select class, so
    // we just need to make sure it gets joined
    return std::make_unique<InputFilteringParams>(selectInfo.GetPathFromInputToSelectClass().GetInputKey(), std::make_unique<RelatedClassPath>(pathFromSelectToInputClass), bvector<ECInstanceId>());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AssignMissingAliases(bvector<RelatedClassPath>& relatedPaths)
    {
    ECClassUseCounter classUseCounter;
    for (RelatedClassPath& path : relatedPaths)
        {
        for (RelatedClass& step : path)
            {
            if (step.GetRelationship().GetAlias().empty())
                {
                ECClassCR relClass = step.GetRelationship().GetClass();
                step.GetRelationship().SetAlias(RULES_ENGINE_RELATED_CLASS_ALIAS(relClass, classUseCounter.Inc(&relClass)));
                }
            if (step.GetTargetClass().GetAlias().empty())
                {
                ECClassCR targetClass = step.GetTargetClass().GetClass();
                step.GetTargetClass().SetAlias(RULES_ENGINE_RELATED_CLASS_ALIAS(targetClass, classUseCounter.Inc(&targetClass)));
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void ApplyInstanceFilterDefinition(ComplexQueryBuilder& query, SelectClassWithExcludes<ECClass> const& selectClass, InstanceFilterDefinition const& filter,
    QueryClauseAndBindings const& expressionClause)
    {
    bool needToFilterClass = filter.GetSelectClass() ? !selectClass.GetClass().Is(filter.GetSelectClass()) : false;
    if (filter.GetRelatedInstances().empty() && !needToFilterClass)
        {
        query.Where(expressionClause);
        return;
        }

    SelectClassWithExcludes<ECClass> filterSelectClass = selectClass;
    filterSelectClass.SetAlias("this");
    if (needToFilterClass)
        filterSelectClass.SetClass(*filter.GetSelectClass());

    auto filterQuery = ComplexQueryBuilder::Create();
    filterQuery->SelectContract(*SimpleQueryContract::Create({ *PresentationQueryContractSimpleField::Create(nullptr, "[this].[ECInstanceId]") }));
    filterQuery->From(filterSelectClass);

    bvector<RelatedClassPath> relatedPaths = filter.GetRelatedInstances();
    AssignMissingAliases(relatedPaths);
    for (RelatedClassPathCR relatedPath : relatedPaths)
        filterQuery->Join(relatedPath);

    filterQuery->Where(expressionClause);

    Utf8String whereClause;
    if (needToFilterClass)
        whereClause.append("[").append(selectClass.GetAlias()).append("].[ECClassId] IS (").append(filter.GetSelectClass()->GetFullName()).append(") AND ");
    whereClause.append("[").append(selectClass.GetAlias()).append("].[ECInstanceId] IN (").append(filterQuery->GetQuery()->GetQueryString()).append(")");

    query.Where(QueryClauseAndBindings(whereClause, filterQuery->GetQuery()->GetBindings()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderHelpers::ApplyInstanceFilter(ComplexQueryBuilder& query, InstanceFilteringParams const& params)
    {
    if (params.GetInputFilter())
        {
        Utf8CP inputClassAlias = "this";
        if (params.GetInputFilter()->GetPathToInputClass())
            {
            query.Join(*params.GetInputFilter()->GetPathToInputClass());
            inputClassAlias = params.GetInputFilter()->GetPathToInputClass()->back().GetTargetClass().GetAlias().c_str();
            }

        if (!params.GetInputFilter()->GetSelectIds().empty())
            {
            ValuesFilteringHelper filteringHelper(params.GetInputFilter()->GetSelectIds());
            query.Where(filteringHelper.Create(Utf8PrintfString("[%s].[ECInstanceId]", inputClassAlias).c_str()));
            }
        }

    if (params.GetInstanceFilter() && 0 != *params.GetInstanceFilter())
        query.Where(ECExpressionsHelper(params.GetECExpressionsCache()).ConvertToECSql(params.GetInstanceFilter(), nullptr, params.GetECExpressionContext()));

    if (params.GetSelectClass().IsValid() && params.GetInstanceFilterDefinition())
        {
        QueryClauseAndBindings expressionClause = ECExpressionsHelper(params.GetECExpressionsCache()).ConvertToECSql(params.GetInstanceFilterDefinition()->GetExpression(), nullptr, params.GetECExpressionContext());
        ApplyInstanceFilterDefinition(query, params.GetSelectClass(), *params.GetInstanceFilterDefinition(), expressionClause);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderHelpers::ApplyPagingOptions(RefCountedPtr<PresentationQueryBuilder>& query, PageOptionsCR opts)
    {
    if (0 == opts.GetPageSize() && 0 == opts.GetPageStart())
        return;

    QueryBuilderHelpers::Limit(*query, opts.GetPageSize(), opts.GetPageStart());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderHelpers::Aggregate(ContentDescriptorPtr& aggregateDescriptor, ContentDescriptorR inputDescriptor)
    {
    if (aggregateDescriptor.IsNull())
        aggregateDescriptor = &inputDescriptor;
    else
        aggregateDescriptor->MergeWith(inputDescriptor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelOverrideSelectFieldsBuilder : InstanceLabelOverrideValueSpecificationVisitor
{
private:
    ECSchemaHelper const& m_schemaHelper;
    SelectClass<ECClass> const& m_selectClass;
    PresentationQueryContractFieldCPtr m_ecInstanceIdField;
    PresentationQueryContractFieldCPtr m_ecClassIdField;
    bvector<ECInstanceKey> const& m_labelRequestsStack;
    bvector<PresentationQueryContractFieldPtr> m_fields;
    bool m_definitelyHasValue;

private:
    PresentationQueryContractFieldPtr CreateRelatedInstanceValueQueryField(RelationshipPathSpecification const& pathToRelatedInstanceSpec,
        std::function<PresentationQueryContractFieldPtr(PresentationQueryContractFieldCR, PresentationQueryContractFieldCR, ECClassCR, Utf8StringCR)> const& valueSelectFieldFactory) const
        {
        ECClassUseCounter counter;
        bvector<ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification> pathSpecs = { ECSchemaHelper::RelationshipPathsRequestParams::PathSpecification(0, pathToRelatedInstanceSpec, false) };
        bvector<RelatedClassPath> noRelatedInstances;
        ECSchemaHelper::RelationshipPathsRequestParams params(m_selectClass, pathSpecs, nullptr, noRelatedInstances, counter, false);
        auto result = m_schemaHelper.GetRelationshipPaths(params);
        RefCountedPtr<PresentationQueryBuilder> query;
        for (auto& path : result.GetPaths(0))
            {
            if (path.m_path.empty())
                DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Got an empty path from select to property class when constructing instance label override query");

            for (auto& step : path.m_path)
                step.SetIsTargetOptional(false);

            SelectClass<ECClass> selectClass(m_selectClass);
            selectClass.SetAlias(Utf8PrintfString("%s_%" PRIu64, m_selectClass.GetAlias().c_str(), counter.Inc(&m_selectClass.GetClass())));

            SelectClassWithExcludes<ECClass> const& targetClass = path.m_path.back().GetTargetClass();
            Utf8StringCR targetClassAlias = targetClass.GetAlias();

            auto targetECClassIdField = PresentationQueryContractSimpleField::Create("/TargetECClassId/", "ECClassId");
            targetECClassIdField->SetPrefixOverride(targetClassAlias);

            auto targetECInstanceIdField = PresentationQueryContractSimpleField::Create("/TargetECInstanceId/", "ECInstanceId");
            targetECInstanceIdField->SetPrefixOverride(targetClassAlias);

            auto valueSelectField = valueSelectFieldFactory(*targetECClassIdField, *targetECInstanceIdField, targetClass.GetClass(), targetClassAlias);
            auto contract = SimpleQueryContract::Create({ valueSelectField });
            auto pathQuery = ComplexQueryBuilder::Create();
            pathQuery->SelectContract(*contract, targetClassAlias.c_str())
                .From(selectClass)
                .Join(path.m_path)
                .Where(Utf8PrintfString("[%s].[ECInstanceId] = %s", selectClass.GetAlias().c_str(), m_ecInstanceIdField->GetSelectClause(m_selectClass.GetAlias().c_str()).GetClause().c_str()).c_str(), {});
            QueryBuilderHelpers::SetOrUnion(query, *pathQuery);
            }
        if (query.IsValid())
            {
            // ECSQL parser fails if we use binding here (Affan notified)
            // QueryBuilderHelpers::Limit<GenericQuery>(*query, 1);
            Utf8String queryStr = Utf8String(query->GetQuery()->GetQueryString()).append(" LIMIT 1");
            return PresentationQueryContractSimpleField::Create("/RelatedPropertyValue/", QueryClauseAndBindings(queryStr, query->GetQuery()->GetBindings()), false);
            }
        return nullptr;
        }

    static PresentationQueryContractFieldPtr CreatePropertyValueField(PresentationQueryContractFieldCR ecClassIdField, Utf8StringCR propertyName, ECClassCR ecClass, Utf8StringCR prefix)
        {
        PrimitiveType const& propertyType = ecClass.GetPropertyP(propertyName)->GetAsPrimitiveProperty()->GetType();
        PresentationQueryContractFieldPtr propertyValueField;
        if (propertyType == PRIMITIVETYPE_Point3d || propertyType == PRIMITIVETYPE_Point2d)
            propertyValueField = QueryContractHelpers::CreatePointAsJsonStringSelectField(propertyName, prefix, (propertyType == PRIMITIVETYPE_Point2d) ? 2 : 3);
        else
            propertyValueField = PresentationQueryContractSimpleField::Create("/PropertyValue/", propertyName.c_str()).get();

        return PresentationQueryContractFunctionField::Create("/PropertyValue/", FUNCTION_NAME_GetECPropertyValueLabel,
            {
            &ecClassIdField,
            PresentationQueryContractSimpleField::Create("/PropertyName/", Utf8PrintfString("'%s'", propertyName.c_str()).c_str()),
            propertyValueField
            });
        }

    PresentationQueryContractFieldPtr CreateDisplayLabelField(PresentationQueryContractFieldCR ecClassIdField, PresentationQueryContractFieldCR ecInstanceIdField) const
        {
        auto labelRequestsStackStr = Utf8String("'").append(ValueHelpers::GetECInstanceKeysAsJsonString(m_labelRequestsStack)).append("'");
        return PresentationQueryContractFunctionField::Create("/RelatedInstanceLabel/", FUNCTION_NAME_GetRelatedDisplayLabel,
            {
            &ecClassIdField,
            &ecInstanceIdField,
            PresentationQueryContractSimpleField::Create("/LabelRequestsStack/", QueryClauseAndBindings(labelRequestsStackStr), false),
            });
        }

protected:
    void _Visit(InstanceLabelOverrideCompositeValueSpecification const& spec) override
        {
        auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Handle %s", DiagnosticsHelpers::CreateRuleIdentifier(spec).c_str()));

        bvector<PresentationQueryContractFieldCPtr> functionParameters;
        functionParameters.push_back(PresentationQueryContractSimpleField::Create("", Utf8PrintfString("'%s'", spec.GetSeparator().c_str()).c_str()));
        bool definitelyHasAllRequiredParts = true;
        bool hasValue = false;
        for (auto valuePart : spec.GetValueParts())
            {
            InstanceLabelOverrideSelectFieldsBuilder builder(m_schemaHelper, m_selectClass, *m_ecInstanceIdField, *m_ecClassIdField, m_labelRequestsStack);
            valuePart->GetSpecification()->Accept(builder);
            if (builder.GetSelectFields().empty())
                {
                DIAGNOSTICS_EDITOR_LOG(DiagnosticsCategory::Default, LOG_ERROR, "Value part did not result in any select fields");
                continue;
                }

            hasValue = true;
            functionParameters.push_back(builder.GetSelectFields().front());
            functionParameters.push_back(PresentationQueryContractSimpleField::Create("", valuePart->IsRequired() ? "TRUE" : "FALSE"));

            if (valuePart->IsRequired() && !builder.DefinitelyHasValue())
                definitelyHasAllRequiredParts = false;
            }

        if (hasValue)
            m_fields.push_back(PresentationQueryContractFunctionField::Create("/CombinedValue/", FUNCTION_NAME_JoinOptionallyRequired, functionParameters));

        m_definitelyHasValue |= definitelyHasAllRequiredParts;
        }

    void _Visit(InstanceLabelOverridePropertyValueSpecification const& spec) override
        {
        auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Handle %s", DiagnosticsHelpers::CreateRuleIdentifier(spec).c_str()));
        if (!spec.GetPathToRelatedInstanceSpecification().GetSteps().empty())
            {
            auto relatedInstancePropertyValueField = CreateRelatedInstanceValueQueryField(spec.GetPathToRelatedInstanceSpecification(), [&spec](auto const& ecClassIdField, auto const&, ECClassCR targetClass, Utf8String prefix)
                {
                return CreatePropertyValueField(ecClassIdField, spec.GetPropertyName(), targetClass, prefix);
                });
            if (relatedInstancePropertyValueField.IsNull())
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_INFO, LOG_ERROR, Utf8PrintfString("Failed to create %s instance label override - is the specified relationship path valid?", spec.GetJsonElementType()));
                }
            else
                {
                m_fields.push_back(relatedInstancePropertyValueField);
                return;
                }
            }
        m_fields.push_back(CreatePropertyValueField(*m_ecClassIdField, spec.GetPropertyName(), m_selectClass.GetClass(), m_selectClass.GetAlias()));
        }

    void _Visit(InstanceLabelOverrideClassNameValueSpecification const& spec) override
        {
        auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Handle %s", DiagnosticsHelpers::CreateRuleIdentifier(spec).c_str()));
        m_fields.push_back(PresentationQueryContractFunctionField::Create("/ClassName/", FUNCTION_NAME_GetECClassName,
            {
            m_ecClassIdField,
            PresentationQueryContractSimpleField::Create("/Full/", spec.ShouldUseFullName() ? "TRUE" : "FALSE")
            }));
        m_definitelyHasValue = true;
        }

    void _Visit(InstanceLabelOverrideClassLabelValueSpecification const& spec) override
        {
        auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Handle %s", DiagnosticsHelpers::CreateRuleIdentifier(spec).c_str()));
        m_fields.push_back(PresentationQueryContractFunctionField::Create("/ClassLabel/", FUNCTION_NAME_GetECClassLabel,
            {
            m_ecClassIdField,
            }));
        m_definitelyHasValue = true;
        }

    void _Visit(InstanceLabelOverrideBriefcaseIdValueSpecification const& spec) override
        {
        auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Handle %s", DiagnosticsHelpers::CreateRuleIdentifier(spec).c_str()));
        // WIP: the clause can be replaced with the following when ECSQL start supporting bitwise operators
        // FUNCTION_NAME_ToBase36 "(ECInstanceId >> 40)"
        m_fields.push_back(PresentationQueryContractFunctionField::Create("/Base36BriefcaseId/", FUNCTION_NAME_ToBase36,
            {
            PresentationQueryContractFunctionField::Create("/BriefcaseId/", FUNCTION_NAME_ParseBriefcaseId,
                {
                m_ecInstanceIdField,
                })
            }));
        m_definitelyHasValue = true;
        }

    void _Visit(InstanceLabelOverrideLocalIdValueSpecification const& spec) override
        {
        auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Handle %s", DiagnosticsHelpers::CreateRuleIdentifier(spec).c_str()));
        // WIP: the clause can be replaced with the following when ECSQL start supporting bitwise operators
        // FUNCTION_NAME_ToBase36 "(ECInstanceId & ((1 << 40) - 1))"
        m_fields.push_back(PresentationQueryContractFunctionField::Create("/Base36LocalId/", FUNCTION_NAME_ToBase36,
            {
            PresentationQueryContractFunctionField::Create("/LocalId/", FUNCTION_NAME_ParseLocalId,
                {
                m_ecInstanceIdField,
                })
            }));
        m_definitelyHasValue = true;
        }

    void _Visit(InstanceLabelOverrideStringValueSpecification const& spec) override
        {
        auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Handle %s", DiagnosticsHelpers::CreateRuleIdentifier(spec).c_str()));
        if (spec.GetValue().empty())
            return;

        Utf8String value = Utf8String("'").append(LabelDefinition::Create(spec.GetValue().c_str())->ToJsonString()).append("'");
        m_fields.push_back(PresentationQueryContractSimpleField::Create("/StringValue/", value.c_str()));
        m_definitelyHasValue |= !spec.GetValue().empty();
        }

    void _Visit(InstanceLabelOverrideRelatedInstanceLabelSpecification const& spec) override
        {
        auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Handle %s", DiagnosticsHelpers::CreateRuleIdentifier(spec).c_str()));
        auto relatedInstanceDisplayLabelField = CreateRelatedInstanceValueQueryField(spec.GetPathToRelatedInstanceSpecification(),
            std::bind(&InstanceLabelOverrideSelectFieldsBuilder::CreateDisplayLabelField, this, std::placeholders::_1, std::placeholders::_2));
        if (relatedInstanceDisplayLabelField.IsNull())
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_INFO, LOG_ERROR, Utf8PrintfString("Failed to create %s instance label override - is the specified relationship path valid?", spec.GetJsonElementType()));
            return;
            }
        m_fields.push_back(relatedInstanceDisplayLabelField);
        }

public:
    InstanceLabelOverrideSelectFieldsBuilder(ECSchemaHelper const& schemaHelper, SelectClass<ECClass> const& selectClass, PresentationQueryContractFieldCR instanceIdField,
        PresentationQueryContractFieldCR classIdField, bvector<ECInstanceKey> const& labelRequestsStack)
        : m_schemaHelper(schemaHelper), m_selectClass(selectClass), m_ecInstanceIdField(&instanceIdField), m_ecClassIdField(&classIdField), m_labelRequestsStack(labelRequestsStack), m_definitelyHasValue(false)
        {}
    bvector<PresentationQueryContractFieldPtr> const& GetSelectFields() const {return m_fields;}
    bool DefinitelyHasValue() const {return m_definitelyHasValue;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContractFieldPtr QueryBuilderHelpers::CreateDisplayLabelField(Utf8CP name, ECSchemaHelper const& schemaHelper, SelectClass<ECClass> const& selectClass,
    PresentationQueryContractFieldCPtr classIdField, PresentationQueryContractFieldCPtr instanceIdField, bvector<RelatedClassPath> const& relatedInstancePaths,
    bvector<InstanceLabelOverrideValueSpecification const*> const& labelOverrideValueSpecs, bvector<ECInstanceKey> const& labelRequestsStack)
    {
    if (classIdField.IsNull())
        classIdField = PresentationQueryContractSimpleField::Create("/ECClassId/", "ECClassId");
    if (instanceIdField.IsNull())
        instanceIdField = PresentationQueryContractSimpleField::Create("/ECInstanceId/", "ECInstanceId");

    ECPropertyCP labelProperty = selectClass.GetClass().GetInstanceLabelProperty();
    Utf8CP labelClause = nullptr != labelProperty ? labelProperty->GetName().c_str() : "''";
    RefCountedPtr<PresentationQueryContractSimpleField> defaultPropertyValueField = PresentationQueryContractSimpleField::Create(nullptr, labelClause);

    PresentationQueryContractFieldPtr labelField = PresentationQueryContractFunctionField::Create(name, FUNCTION_NAME_GetECInstanceDisplayLabel,
        {
        classIdField,
        instanceIdField,
        defaultPropertyValueField,
        PresentationQueryContract::CreateRelatedInstanceInfoField(relatedInstancePaths),
        });

    if (!labelOverrideValueSpecs.empty())
        {
        auto fallback = labelField;

        InstanceLabelOverrideSelectFieldsBuilder builder(schemaHelper, selectClass, *instanceIdField, *classIdField, labelRequestsStack);
        for (InstanceLabelOverrideValueSpecification const* spec : labelOverrideValueSpecs)
            {
            spec->Accept(builder);
            if (builder.DefinitelyHasValue())
                break;
            }

        if (builder.GetSelectFields().empty())
            return labelField;

        if (builder.GetSelectFields().size() == 1 && builder.DefinitelyHasValue())
            {
            labelField = builder.GetSelectFields().front();
            labelField->SetName(name);
            return labelField;
            }

        RefCountedPtr<PresentationQueryContractFunctionField> coalesceField = PresentationQueryContractFunctionField::Create(name,
            "COALESCE", ContainerHelpers::TransformContainer<bvector<PresentationQueryContractFieldCPtr>>(builder.GetSelectFields()));

        if (!builder.DefinitelyHasValue())
            coalesceField->GetFunctionParameters().push_back(labelField);

        labelField = coalesceField;
        }

    labelField->SetGroupingClause(CreateDisplayLabelValueClause(labelField->GetName()));
    labelField->SetResultType(PresentationQueryFieldType::LabelDefinition);
    return labelField;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IdSet<BeInt64Id> QueryBuilderHelpers::CreateIdSetFromJsonArray(RapidJsonValueCR json)
    {
    IdSet<BeInt64Id> ids;
    if (!json.IsArray())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Expected a JSON array");
    for (Json::ArrayIndex i = 0; i < json.Size(); i++)
        ids.insert(BeInt64Id(json[i].GetUint64()));
    return ids;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECValue QueryBuilderHelpers::CreateECValueFromJson(RapidJsonValueCR json)
    {
    ECValue v;
    if (json.IsString())
        v.SetUtf8CP(json.GetString());
    else if (json.IsBool())
        v.SetBoolean(json.GetBool());
    else if (json.IsInt())
        v.SetInteger(json.GetInt());
    else if (json.IsInt64())
        v.SetLong(json.GetInt64());
    else if (json.IsDouble())
        v.SetDouble(json.GetDouble());
    else
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Default, LOG_ERROR, "Unhandled ECValue type");
    return v;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bmap<ECClassCP, bvector<InstanceLabelOverride const*>> QueryBuilderHelpers::GetLabelOverrideValuesMap(ECSchemaHelper const& helper, bvector<InstanceLabelOverrideCP> labelOverrides)
    {
    std::sort(labelOverrides.begin(), labelOverrides.end(), [](InstanceLabelOverrideCP a, InstanceLabelOverrideCP b) {return a->GetPriority() > b->GetPriority();});
    bmap<ECClassCP, bvector<InstanceLabelOverride const*>> mappedFields;
    for (auto labelOverride : labelOverrides)
        {
        ECClassCP ecClass = helper.GetECClass(labelOverride->GetClassName().c_str());
        if (nullptr == ecClass)
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_INFO, LOG_ERROR, Utf8PrintfString("LabelOverride class not found: '%s'", labelOverride->GetClassName().c_str()));
            continue;
            }

        auto iter = mappedFields.find(ecClass);
        if (iter == mappedFields.end())
            iter = mappedFields.Insert(ecClass, bvector<InstanceLabelOverride const*>()).first;

        iter->second.push_back(labelOverride);
        }
    return mappedFields;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
static bvector<T> SerializeECClassMapPolymorphically(bmap<ECClassCP, bvector<T>> const& map, ECClassCR ecClass, std::function<bool(T const&, ECClassCR)> const& pred)
    {
    bvector<T> list;
    auto iter = map.find(&ecClass);
    if (iter != map.end())
        {
        for (auto const& item : iter->second)
            {
            if (pred(item, ecClass))
                list.push_back(item);
            }
        }
    for (ECClassCP baseClass : ecClass.GetBaseClasses())
        {
        bvector<T> baseList = SerializeECClassMapPolymorphically(map, *baseClass, pred);
        std::move(baseList.begin(), baseList.end(), std::back_inserter(list));
        }
    return list;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<InstanceLabelOverrideValueSpecification const*> QueryBuilderHelpers::GetInstanceLabelOverrideSpecsForClass(bmap<ECClassCP, bvector<InstanceLabelOverride const*>> const& instanceLabelOverrides, ECClassCR ecClass)
    {
    bool handled = false;
    bvector<InstanceLabelOverride const*> usedOverrides = SerializeECClassMapPolymorphically<InstanceLabelOverride const*>(instanceLabelOverrides, ecClass, [&handled](InstanceLabelOverride const* ovr, ECClassCR)
        {
        if (ovr->GetOnlyIfNotHandled() && handled)
            return false;
        handled = true;
        return true;
        });
    bvector<InstanceLabelOverrideValueSpecification const*> specs;
    for (auto const& ovr : usedOverrides)
        {
        DiagnosticsHelpers::ReportRule(*ovr);
        ContainerHelpers::Push(specs, ovr->GetValueSpecifications());
        }
    return specs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GroupSpecificationCP QueryBuilderHelpers::GetActiveGroupingSpecification(GroupingRuleCR rule, IJsonLocalState const* localState)
    {
    if (rule.GetGroups().empty())
        return nullptr;

    if (rule.GetSettingsId().empty() || nullptr == localState)
        return rule.GetGroups()[0];

    Json::Value settingValue = localState->GetJsonValue(RULES_ENGINE_ACTIVE_GROUPS_LOCAL_STATE_NAMESPACE, Utf8String(rule.GetSettingsId().c_str()).c_str());
    if (!settingValue.isInt())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Expected active group index to be an integer, actual type: %d", (int)settingValue.type()));

    int activeGroupIndex = settingValue.asInt();
    if (activeGroupIndex < 0 || activeGroupIndex >= (int)rule.GetGroups().size())
        {
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Invalid active group index: %" PRIu64 ". Groups count: %" PRIu64,
            (uint64_t)activeGroupIndex, (uint64_t)rule.GetGroups().size()));
        }

    return rule.GetGroups()[activeGroupIndex];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECValue GetPropertyGroupRangeValue(ECPropertyCR prop, Utf8StringCR rangeValueStr)
    {
    if (!prop.GetIsPrimitive())
        {
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("Can only get range values for primitive properties. "
            "Property: '%s.%s'", prop.GetClass().GetFullName(), prop.GetName().c_str()));
        }
    return ValueHelpers::GetECValueFromString(prop.GetAsPrimitiveProperty()->GetType(), rangeValueStr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
QueryClauseAndBindings QueryBuilderHelpers::CreatePropertyGroupFilteringClause(ECPropertyCR ecProperty, Utf8StringCR propertyValueSelector, PropertyGroupCR groupingSpecification, RapidJsonValueCR groupingValues)
    {
    if (groupingSpecification.GetRanges().empty())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Filtering by property values");
        PrimitiveType groupedValuesType = ecProperty.GetIsPrimitive() ? ecProperty.GetAsPrimitiveProperty()->GetType() : ecProperty.GetIsNavigation() ? PRIMITIVETYPE_Long : PRIMITIVETYPE_String;
        return QueryClauseAndBindings(Utf8String("InVirtualSet(?, ").append(propertyValueSelector).append(")"),
            { std::make_shared<BoundRapidJsonValueSet>(groupingValues, groupedValuesType) });
        }

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Hierarchies, LOG_TRACE, "Filtering by property value ranges");
    PropertyRangeGroupList const& groupingRanges = groupingSpecification.GetRanges();
    Utf8String rangesClause;
    BoundQueryValuesList rangesBindings;
    for (rapidjson::SizeType i = 0; i < groupingValues.Size(); ++i)
        {
        int rangeIndex = groupingValues[i].GetInt();
        Utf8String rangeClause;
        BoundQueryValuesList rangeBindings;
        if (rangeIndex < 0)
            {
            for (PropertyRangeGroupSpecificationCP range : groupingRanges)
                {
                if (!rangeClause.empty())
                    rangeClause.append(" AND ");
                rangeClause.append(propertyValueSelector).append(" NOT BETWEEN ? AND ?");
                rangeBindings.push_back(std::make_unique<BoundQueryECValue>(GetPropertyGroupRangeValue(ecProperty, range->GetFromValue())));
                rangeBindings.push_back(std::make_unique<BoundQueryECValue>(GetPropertyGroupRangeValue(ecProperty, range->GetToValue())));
                }
            }
        else if (rangeIndex < (int)groupingRanges.size())
            {
            PropertyRangeGroupSpecificationCP range = groupingRanges[rangeIndex];
            rangeClause.append(propertyValueSelector).append(" BETWEEN ? AND ?");
            rangeBindings = {
                std::make_shared<BoundQueryECValue>(GetPropertyGroupRangeValue(ecProperty, range->GetFromValue())),
                std::make_shared<BoundQueryECValue>(GetPropertyGroupRangeValue(ecProperty, range->GetToValue()))
                };
            }
        else
            {
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Hierarchies, Utf8PrintfString("Range index larger than ranges array. Index: %d, size of array: %" PRIu64, rangeIndex, (uint64_t)groupingRanges.size()));
            }
        if (!rangesClause.empty())
            rangesClause.append(" OR ");
        rangesClause.append("(").append(rangeClause).append(")");
        ContainerHelpers::Push(rangesBindings, rangeBindings);
        }
    return QueryClauseAndBindings(rangesClause, rangesBindings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ShouldSplitIntoDerivedClasses(RuleApplicationInfo const& ruleInfo, ECClassCR candidate)
    {
    // expand ECClass into its subclasses in 2 cases:
    // - rule applies to one of the child classes
    // - rule applies to this class, but is not polymorphic (applies to this class, but not its children)
    return ruleInfo.GetRuleClass()->Is(&candidate) && ruleInfo.GetRuleClass() != &candidate
        || ruleInfo.GetRuleClass() == &candidate && !ruleInfo.IsRulePolymorphic() && !candidate.GetDerivedClasses().empty();
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SelectClassSplitResultComparer
    {
    bool operator()(SelectClassSplitResult const& lhs, SelectClassSplitResult const& rhs) const
        {
        int selectClassNameCmp = strcmp(lhs.GetSelectClass().GetClass().GetFullName(), rhs.GetSelectClass().GetClass().GetFullName());
        if (selectClassNameCmp < 0)
            return true;
        if (selectClassNameCmp > 0)
            return false;
        if (!lhs.GetSelectClass().IsSelectPolymorphic() && rhs.GetSelectClass().IsSelectPolymorphic())
            return true;
        return false;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static SelectClass<ECClass>* FindExcludedRuleClass(RuleApplicationInfo const& rule, bvector<SelectClass<ECClass>>& excludedClasses)
    {
    for (SelectClass<ECClass>& excludedClass : excludedClasses)
        {
        // Is rule affecting a class that is excluded?
        if (&excludedClass.GetClass() == rule.GetRuleClass() || excludedClass.IsSelectPolymorphic() && rule.GetRuleClass()->Is(&excludedClass.GetClass()))
            return &excludedClass;
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessSelectClassesBasedOnCustomizationRules(bvector<SelectClassSplitResult>& splitInfos, bvector<RuleApplicationInfo> const& customizationRuleInfos, SchemaManagerCR schemas)
    {
    bset<SelectClassSplitResult, SelectClassSplitResultComparer> selectsToAppend;
    for (SelectClassSplitResult& splitInfo : splitInfos)
        {
        if (!splitInfo.GetSelectClass().IsSelectPolymorphic())
            continue;

        for (RuleApplicationInfo const& ruleInfo : customizationRuleInfos)
            {
            // check the primary select class
            if (ShouldSplitIntoDerivedClasses(ruleInfo, splitInfo.GetSelectClass().GetClass()))
                {
                // the rule wants to customize a subclass of the class and leave other subclasses as is -
                // this means we have to expand the ecClass into its subclasses
                SelectClassSplitResult customizationClassSplitInfo(SelectClassWithExcludes<ECClass>(*ruleInfo.GetRuleClass(), splitInfo.GetSelectClass().GetAlias(), ruleInfo.IsRulePolymorphic()));
                std::copy(splitInfo.GetSplitPath().begin(), splitInfo.GetSplitPath().end(), std::back_inserter(customizationClassSplitInfo.GetSplitPath()));
                customizationClassSplitInfo.GetSplitPath().push_back(splitInfo.GetSelectClass());

                SelectClass<ECClass>* result = FindExcludedRuleClass(ruleInfo, splitInfo.GetSelectClass().GetDerivedExcludedClasses());
                if (result != nullptr)
                    {
                    // If the exclusion is polymorphic, we can skip the rule since all of the affected classes are excluded.
                    if (result->IsSelectPolymorphic())
                        continue;

                    // If the rule applies only to a single excluded class, we can also skip it since the only affected class is excluded.
                    if (!ruleInfo.IsRulePolymorphic())
                        continue;

                    // The rule is polymorphic, but the exclusion is non polymorphic
                    // Pass the related excludes to the split result
                    auto const& directChildren = schemas.GetDerivedClasses(*ruleInfo.GetRuleClass());
                    for (auto const& excludedClass : splitInfo.GetSelectClass().GetDerivedExcludedClasses())
                        {
                        for (auto const& childClass : directChildren)
                            {
                            if (childClass->Is(&excludedClass.GetClass()))
                                customizationClassSplitInfo.GetSelectClass().GetDerivedExcludedClasses().push_back(excludedClass);
                            }
                        }

                    // Make the exclude polymorphic because we are splitting the select class into derived class selects.
                    result->SetIsSelectPolymorphic(true);
                    }
                else
                    {
                    // Exclude rule class polymorphically because we are splitting the select class into derived class selects.
                    splitInfo.GetSelectClass().GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*ruleInfo.GetRuleClass(), "", ruleInfo.IsRulePolymorphic()));
                    }

                bvector<SelectClassSplitResult> customizationClassSplitInfos = { customizationClassSplitInfo };
                ProcessSelectClassesBasedOnCustomizationRules(customizationClassSplitInfos, customizationRuleInfos, schemas);
                std::move(customizationClassSplitInfos.begin(), customizationClassSplitInfos.end(), std::inserter(selectsToAppend, selectsToAppend.end()));
                }
            }
        }

    // don't want to append selects that are already in the input vector
    for (SelectClassSplitResult const& splitInfo : splitInfos)
        {
        auto iter = selectsToAppend.find(splitInfo);
        if (selectsToAppend.end() != iter)
            selectsToAppend.erase(iter);
        }

    // append the additional selects
    std::move(selectsToAppend.begin(), selectsToAppend.end(), std::back_inserter(splitInfos));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<SelectClassSplitResult> QueryBuilderHelpers::ProcessSelectClassesBasedOnCustomizationRules(bvector<SelectClassWithExcludes<ECClass>> const& selectClasses,
    bvector<RuleApplicationInfo> const& customizationRuleInfos, SchemaManagerCR schemas)
    {
    bvector<SelectClassSplitResult> result = ContainerHelpers::TransformContainer<bvector<SelectClassSplitResult>>(selectClasses, [](auto const& sc){return SelectClassSplitResult(sc);});
    ::ProcessSelectClassesBasedOnCustomizationRules(result, customizationRuleInfos, schemas);
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool RelationshipPathsEqualOrDifferOnlyByTarget(RelatedClassPathCR lhs, RelatedClassPathCR rhs)
    {
    if (lhs.size() != rhs.size())
        return false;
    if (lhs.size() == 0)
        return true;

    for (size_t i = 0; i < lhs.size() - 1; ++i)
        {
        if (!lhs[i].ClassesEqual(rhs[i]))
            return false;
        }

    RelatedClassCR lhsLast = lhs.back();
    RelatedClassCR rhsLast = rhs.back();
    return lhsLast.GetSourceClass() == rhsLast.GetSourceClass()
        && &lhsLast.GetRelationship().GetClass() == &rhsLast.GetRelationship().GetClass();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessRelationshipPathsBasedOnCustomizationRules(bvector<RelatedClassPath>& relationshipPaths, bvector<RuleApplicationInfo> const& customizationRuleInfos, SchemaManagerCR schemas)
    {
    bvector<RelatedClassPath> childPaths;
    for (RuleApplicationInfo const& ruleInfo : customizationRuleInfos)
        {
        for (RelatedClassPath& path : relationshipPaths)
            {
            if (!path.back().GetTargetClass().IsSelectPolymorphic())
                continue;

            if (ShouldSplitIntoDerivedClasses(ruleInfo, path.back().GetTargetClass().GetClass()))
                {
                path.back().GetTargetClass().GetDerivedExcludedClasses().push_back(SelectClass<ECClass>(*ruleInfo.GetRuleClass(), "", ruleInfo.IsRulePolymorphic()));

                RelatedClassPath copy(path);
                copy.back().SetTargetClass(SelectClassWithExcludes<ECClass>(*ruleInfo.GetRuleClass(), path.back().GetTargetClass().GetAlias(), ruleInfo.IsRulePolymorphic()));
                bvector<RelatedClassPath> derivedPaths({copy});
                ProcessRelationshipPathsBasedOnCustomizationRules(derivedPaths, customizationRuleInfos, schemas);
                std::move(derivedPaths.begin(), derivedPaths.end(), std::back_inserter(childPaths));
                }
            }
        }
    for (RelatedClassPathCR relatedPath : childPaths)
        {
        auto pred = [&relatedPath](RelatedClassPathCR path)
            {
            return RelationshipPathsEqualOrDifferOnlyByTarget(relatedPath, path)
                && relatedPath.back().GetTargetClass() == path.back().GetTargetClass();
            };
        if (relationshipPaths.end() == std::find_if(relationshipPaths.begin(), relationshipPaths.end(), pred))
            relationshipPaths.push_back(relatedPath);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RelatedClassPath> QueryBuilderHelpers::ProcessRelationshipPathsBasedOnCustomizationRules(bvector<RelatedClassPath> const& relationshipPaths,
    bvector<RuleApplicationInfo> const& customizationRuleInfos, SchemaManagerCR schemas)
    {
    if (customizationRuleInfos.empty())
        return relationshipPaths;

    bvector<RelatedClassPath> result(relationshipPaths);
    ::ProcessRelationshipPathsBasedOnCustomizationRules(result, customizationRuleInfos, schemas);
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String QueryBuilderHelpers::CreateDisplayLabelValueClause(Utf8StringCR fieldName)
    {
    return Utf8String(FUNCTION_NAME_GetLabelDefinitionDisplayValue).append("(").append(QueryHelpers::Wrap(fieldName)).append(")");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderHelpers::CollectRelatedClassPathAliases(bset<Utf8String>& aliases, bvector<RelatedClass> const& classes)
    {
    for (RelatedClassCR relatedClass : classes)
        {
        aliases.insert(relatedClass.GetRelationship().GetAlias());
        aliases.insert(relatedClass.GetTargetClass().GetAlias());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderHelpers::CollectRelatedClassPathAliases(bset<Utf8String>& aliases, bvector<RelatedClassPath> const& relatedPaths)
    {
    for (RelatedClassPathCR path : relatedPaths)
        CollectRelatedClassPathAliases(aliases, path);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AppendRule(bvector<ClassSortingRule>& rules, SortingRuleCR rule, SelectClassWithExcludes<ECClass> const& selectClass, bvector<RelatedClassPath> const& relatedPaths, std::function<bool(ECClassCR)> const& cond)
    {
    if (cond(selectClass.GetClass()))
        rules.push_back(ClassSortingRule(selectClass, rule));
    for (RelatedClassPathCR relatedInstancePath : relatedPaths)
        {
        if (!relatedInstancePath.empty() && cond(relatedInstancePath.back().GetTargetClass().GetClass()))
            rules.push_back(ClassSortingRule(relatedInstancePath.back().GetTargetClass(), rule));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ClassSortingRule> QueryBuilderHelpers::GetClassSortingRules(bvector<SortingRuleCP> const& allRules, SelectClassWithExcludes<ECClass> const& selectClass, bvector<RelatedClassPath> const& relatedPaths, ECSchemaHelper const& helper)
    {
    bvector<ClassSortingRule> rules;
    for (SortingRuleCP rule : allRules)
        {
        if (rule->GetSchemaName().empty())
            {
            rules.push_back(ClassSortingRule(selectClass, *rule));
            continue;
            }

        if (rule->GetClassName().empty())
            {
            ECSchemaCP ruleSchema = helper.GetSchema(rule->GetSchemaName().c_str(), false);
            if (nullptr == ruleSchema)
                {
                DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_INFO, LOG_ERROR, Utf8PrintfString("Requested sorting rule schema not found: '%s'", rule->GetSchemaName().c_str()));
                continue;
                }
            AppendRule(rules, *rule, selectClass, relatedPaths, [&ruleSchema](ECClassCR selectClass) {return ruleSchema == &selectClass.GetSchema(); });
            continue;
            }

        ECClassCP ruleClass = helper.GetECClass(rule->GetSchemaName().c_str(), rule->GetClassName().c_str());
        if (nullptr == ruleClass)
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Hierarchies, LOG_INFO, LOG_ERROR, Utf8PrintfString("Requested sorting rule class not found: `%s.%s`",
                rule->GetSchemaName().c_str(), rule->GetClassName().c_str()));
            continue;
            }
        AppendRule(rules, *rule, selectClass, relatedPaths, [&ruleClass, &rule](ECClassCR selectedClass)
            {
            return ruleClass == &selectedClass
                || rule->GetIsPolymorphic() && selectedClass.Is(ruleClass);
            });
        }
    return rules;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String QueryBuilderHelpers::CreatePropertySortingClause(SortingRuleCR rule, SelectClass<ECClass> selectClass, ECPropertyCR ecProperty)
    {
    Utf8String orderByClause;
    bool useSortingValueFunction = false;
    ECEnumerationCP enumeration = ecProperty.GetIsPrimitive() ? ecProperty.GetAsPrimitiveProperty()->GetEnumeration() : nullptr;
    PrimitiveECPropertyCP primitiveGroupingProperty = ecProperty.GetAsPrimitiveProperty();
    if (ecProperty.GetIsPrimitive() && (PRIMITIVETYPE_String == primitiveGroupingProperty->GetType() || nullptr != enumeration))
        {
        useSortingValueFunction = true;
        orderByClause.append(FUNCTION_NAME_GetSortingValue).append("(");
        if (nullptr != enumeration)
            {
            orderByClause.append(FUNCTION_NAME_GetECEnumerationValue).append("('").append(enumeration->GetSchema().GetName()).append("',");
            orderByClause.append("'").append(enumeration->GetName()).append("',");
            }
        }

    orderByClause.append("[").append(selectClass.GetAlias()).append("].")
        .append("[").append(rule.GetPropertyName()).append("]");

    if (useSortingValueFunction)
        {
        if (nullptr != enumeration)
            orderByClause.append(")");
        orderByClause.append(")");
        }

    if (!rule.GetSortAscending())
        orderByClause.append(" DESC");
    return orderByClause;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String QueryBuilderHelpers::CreatePropertySortingClause(bvector<ClassSortingRule> const& rules, SelectClassWithExcludes<ECClass> selectClass)
    {
    Utf8String orderByClause;
    for (ClassSortingRule const& rule : rules)
        {
        if (!rule.GetRule().GetClassName().empty() && !rule.GetRule().GetIsPolymorphic()
            && selectClass.IsSelectPolymorphic() && !selectClass.GetClass().GetDerivedClasses().empty())
            {
            // don't apply the rule if it's not polymorphic and we're selecting polymorphically
            continue;
            }
        DiagnosticsHelpers::ReportRule(rule.GetRule());
        if (rule.GetRule().GetDoNotSort())
            break;
        ECPropertyCP ecProperty = rule.GetSelectClass().GetClass().GetPropertyP(rule.GetRule().GetPropertyName().c_str());
        if (nullptr == ecProperty)
            {
            DIAGNOSTICS_LOG(DiagnosticsCategory::Default, LOG_INFO, LOG_ERROR, Utf8PrintfString("Requested sorting rule property not found: '%s.%s'",
                rule.GetSelectClass().GetClass().GetFullName(), rule.GetRule().GetPropertyName().c_str()));
            continue;
            }
        if (!orderByClause.empty())
            orderByClause.append(", ");
        orderByClause.append(CreatePropertySortingClause(rule.GetRule(), rule.GetSelectClass(), *ecProperty));
        }
    return orderByClause;
    }
