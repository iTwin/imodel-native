/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "QueryBuilderHelpers.h"
#include "RulesPreprocessor.h"
#include "ECExpressionContextsProvider.h"
#include "LocalizationHelper.h"
#include "NavNodeProviders.h"
#include "LoggingHelper.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
void QueryBuilderHelpers::SetOrUnion(RefCountedPtr<T>& target, T& source)
    {
    if (target.IsNull())
        target = &source;
    else
        target = UnionPresentationQuery<T>::Create(*target, source);
    }
template void QueryBuilderHelpers::SetOrUnion<ContentQuery>(ContentQueryPtr&, ContentQuery&);
template void QueryBuilderHelpers::SetOrUnion<NavigationQuery>(NavigationQueryPtr&, NavigationQuery&);
template void QueryBuilderHelpers::SetOrUnion<GenericQuery>(GenericQueryPtr&, GenericQuery&);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
void QueryBuilderHelpers::Where(RefCountedPtr<T>& query, Utf8CP clause, BoundQueryValuesListCR bindings)
    {
    if (nullptr != query->AsComplexQuery())
        {
        query->AsComplexQuery()->Where(clause, bindings);
        return;
        }

    RefCountedPtr<ComplexPresentationQuery<T>> wrapper = ComplexPresentationQuery<T>::Create();
    wrapper->SelectAll();
    wrapper->From(*query);
    wrapper->Where(clause, bindings);
    query = wrapper;
    }
template void QueryBuilderHelpers::Where<ContentQuery>(ContentQueryPtr&, Utf8CP, BoundQueryValuesListCR);
template void QueryBuilderHelpers::Where<NavigationQuery>(NavigationQueryPtr&, Utf8CP, BoundQueryValuesListCR);
template void QueryBuilderHelpers::Where<GenericQuery>(GenericQueryPtr&, Utf8CP, BoundQueryValuesListCR);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
void QueryBuilderHelpers::Order(T& query, Utf8CP clause)
    {
    if (nullptr != query.AsComplexQuery())
        query.AsComplexQuery()->OrderBy(clause);
    else if (nullptr != query.AsUnionQuery())
        query.AsUnionQuery()->OrderBy(clause);
    else if (nullptr != query.AsExceptQuery())
        query.AsExceptQuery()->OrderBy(clause);
    else
        BeAssert(false);
    }
template void QueryBuilderHelpers::Order<ContentQuery>(ContentQuery&, Utf8CP);
template void QueryBuilderHelpers::Order<NavigationQuery>(NavigationQuery&, Utf8CP);
template void QueryBuilderHelpers::Order<GenericQuery>(GenericQuery&, Utf8CP);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
Utf8String QueryBuilderHelpers::GetOrderByClause(T const& query)
    {
    if (nullptr != query.AsComplexQuery())
        return query.AsComplexQuery()->GetClause(CLAUSE_OrderBy);
    if (nullptr != query.AsUnionQuery())
        return query.AsUnionQuery()->GetOrderByClause();
    if (nullptr != query.AsExceptQuery())
        return query.AsExceptQuery()->GetOrderByClause();
    return "";
    }
template Utf8String QueryBuilderHelpers::GetOrderByClause<ContentQuery>(ContentQuery const&);
template Utf8String QueryBuilderHelpers::GetOrderByClause<NavigationQuery>(NavigationQuery const&);
template Utf8String QueryBuilderHelpers::GetOrderByClause<GenericQuery>(GenericQuery const&);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
void QueryBuilderHelpers::Limit(T& query, uint64_t limit, uint64_t offset)
    {
    if (nullptr != query.AsComplexQuery())
        query.AsComplexQuery()->Limit(limit, offset);
    else if (nullptr != query.AsUnionQuery())
        query.AsUnionQuery()->Limit(limit, offset);
    else if (nullptr != query.AsExceptQuery())
        query.AsExceptQuery()->Limit(limit, offset);
    else
        BeAssert(false);
    }
template void QueryBuilderHelpers::Limit<ContentQuery>(ContentQuery&, uint64_t, uint64_t);
template void QueryBuilderHelpers::Limit<NavigationQuery>(NavigationQuery&, uint64_t, uint64_t);
template void QueryBuilderHelpers::Limit<GenericQuery>(GenericQuery&, uint64_t, uint64_t);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String QueryBuilderHelpers::Escape(Utf8String str)
    {
    str.ReplaceAll("'", "''");
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
RefCountedPtr<ComplexPresentationQuery<T>> QueryBuilderHelpers::CreateNestedQuery(T& innerQuery)
    {
    RefCountedPtr<ComplexPresentationQuery<T>> query = ComplexPresentationQuery<T>::Create();
    query->SelectAll();
    query->From(innerQuery);
    return query;
    }
template ComplexContentQueryPtr QueryBuilderHelpers::CreateNestedQuery<ContentQuery>(ContentQuery&);
template ComplexNavigationQueryPtr QueryBuilderHelpers::CreateNestedQuery<NavigationQuery>(NavigationQuery&);
template ComplexGenericQueryPtr QueryBuilderHelpers::CreateNestedQuery<GenericQuery>(GenericQuery&);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool CheckQueryForAliases(Utf8CP queryString, bvector<Utf8CP> const& aliases)
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
    for (Utf8CP aliasCase : aliases)
        {
        if (0 == std::strcmp(aliasCase, alias.c_str()))
            return true;
        }

    if (aliasEnd < std::strlen(afterAs))
        return CheckQueryForAliases(afterAs + aliasEnd, aliases);

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
bool QueryBuilderHelpers::NeedsNestingToUseAlias(T const& query, bvector<Utf8CP> const& aliases)
    {
    if (aliases.empty())
        return false;

    if (nullptr != query.AsStringQuery())
        return true;

    if (nullptr != query.AsUnionQuery())
        {
        UnionPresentationQuery<T> const& unionQuery = *query.AsUnionQuery();
        if (NeedsNestingToUseAlias(*unionQuery.GetFirst(), aliases) || NeedsNestingToUseAlias(*unionQuery.GetSecond(), aliases))
            return true;
        }

    if (nullptr != query.AsExceptQuery())
        {
        ExceptPresentationQuery<T> const& exceptQuery = *query.AsExceptQuery();
        if (NeedsNestingToUseAlias(*exceptQuery.GetBase(), aliases) || NeedsNestingToUseAlias(*exceptQuery.GetExcept(), aliases))
            return true;
        }

    if (nullptr != query.AsComplexQuery() && CheckQueryForAliases(query.AsComplexQuery()->GetClause(CLAUSE_Select).c_str(), aliases))
        return true;

    return false;
    }
template bool QueryBuilderHelpers::NeedsNestingToUseAlias<ContentQuery>(ContentQuery const&, bvector<Utf8CP> const&);
template bool QueryBuilderHelpers::NeedsNestingToUseAlias<NavigationQuery>(NavigationQuery const&, bvector<Utf8CP> const&);
template bool QueryBuilderHelpers::NeedsNestingToUseAlias<GenericQuery>(GenericQuery const&, bvector<Utf8CP> const&);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
RefCountedPtr<T> QueryBuilderHelpers::CreateNestedQueryIfNecessary(T& query, bvector<Utf8CP> const& aliases)
    {
    if (NeedsNestingToUseAlias(query, aliases) || nullptr != query.AsStringQuery())
        return CreateNestedQuery(query);
    return &query;
    }
template ContentQueryPtr QueryBuilderHelpers::CreateNestedQueryIfNecessary<ContentQuery>(ContentQuery&, bvector<Utf8CP> const&);
template NavigationQueryPtr QueryBuilderHelpers::CreateNestedQueryIfNecessary<NavigationQuery>(NavigationQuery&, bvector<Utf8CP> const&);
template GenericQueryPtr QueryBuilderHelpers::CreateNestedQueryIfNecessary<GenericQuery>(GenericQuery&, bvector<Utf8CP> const&);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
RefCountedPtr<ComplexPresentationQuery<T>> QueryBuilderHelpers::CreateComplexNestedQueryIfNecessary(T& query, bvector<Utf8CP> const& aliases)
    {
    RefCountedPtr<T> q = CreateNestedQueryIfNecessary(query, aliases);
    if (nullptr == q->AsComplexQuery())
        return CreateNestedQuery(query);
    return q->AsComplexQuery();
    }
template ComplexContentQueryPtr QueryBuilderHelpers::CreateComplexNestedQueryIfNecessary<ContentQuery>(ContentQuery&, bvector<Utf8CP> const&);
template ComplexNavigationQueryPtr QueryBuilderHelpers::CreateComplexNestedQueryIfNecessary<NavigationQuery>(NavigationQuery&, bvector<Utf8CP> const&);
template ComplexGenericQueryPtr QueryBuilderHelpers::CreateComplexNestedQueryIfNecessary<GenericQuery>(GenericQuery&, bvector<Utf8CP> const&);

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2017
+===============+===============+===============+===============+===============+======*/
struct RecursiveQueriesHelper
{
private:
    IConnectionCR m_connection;
    InstanceFilteringParams::RecursiveQueryInfo const& m_recursiveQueryInfo;

private:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool IsRecursiveJoinForward(SelectClassInfo const& selectInfo)
        {
        if (selectInfo.GetPathToInputClass().empty())
            {
            BeAssert(false);
            return true;
            }

        return !selectInfo.GetPathToInputClass().back().IsForwardRelationship(); // invert direction
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                03/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void RecursivelySelectRelatedKeys(bset<ECInstanceId>& result, Utf8StringCR baseQuery, Utf8CP idSelector, bvector<ECInstanceId> const& sourceIds)
        {
        Savepoint txn(m_connection.GetDb(), "RecursiveQueriesHelper::RecursivelySelectRelatedKeys");
        BeAssert(txn.IsActive());

        IdsFilteringHelper<bvector<ECInstanceId>> filteringHelper(sourceIds);
        Utf8String query(baseQuery);
        query.append("WHERE ").append(filteringHelper.CreateWhereClause(idSelector));

        ECSqlStatement stmt;
        if (!stmt.Prepare(m_connection.GetECDb().Schemas(), m_connection.GetDb(), query.c_str()).IsSuccess())
            {
            BeAssert(false);
            return;
            }

        BoundQueryValuesList bindings = filteringHelper.CreateBoundValues();
        for (size_t i = 0; i < bindings.size(); ++i)
            {
            BoundQueryValue const* binding = bindings[i];
            binding->Bind(stmt, (uint32_t)(i + 1));
            }

        bvector<ECInstanceId> tempIds;
        while (BE_SQLITE_ROW == stmt.Step())
            {
            ECInstanceId id = stmt.GetValueId<ECInstanceId>(0);
            result.insert(id);
            tempIds.push_back(id);
            }

        txn.Cancel();

        for (BoundQueryValue const* binding : bindings)
            delete binding;

        if (!tempIds.empty())
            RecursivelySelectRelatedKeys(result, baseQuery, idSelector, tempIds);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                02/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    bset<ECInstanceId> GetRecursiveChildrenIds(bset<ECRelationshipClassCP> const& relationships, bool isForward, bvector<ECInstanceId> const& parentIds)
        {
        static Utf8CP s_sourceECInstanceIdField = "SourceECInstanceId";
        static Utf8CP s_targetECInstanceIdField = "TargetECInstanceId";
        Utf8CP idSelector = isForward ? s_sourceECInstanceIdField : s_targetECInstanceIdField;

        bool first = true;
        Utf8String query("SELECT ");
        query.append(isForward ? s_targetECInstanceIdField : s_sourceECInstanceIdField);
        query.append(" FROM (");
        for (ECRelationshipClassCP rel : relationships)
            {
            if (!first)
                query.append(" UNION ALL ");
            query.append("SELECT ").append(s_sourceECInstanceIdField).append(",").append(s_targetECInstanceIdField).append(" ");
            query.append("FROM ").append(rel->GetECSqlName());
            first = false;
            }
        query.append(") ");

        bset<ECInstanceId> result;
        RecursivelySelectRelatedKeys(result, query, idSelector, parentIds);
        return result;
        }

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    RecursiveQueriesHelper(IConnectionCR connection, InstanceFilteringParams::RecursiveQueryInfo const& recursiveQueryInfo)
        : m_connection(connection), m_recursiveQueryInfo(recursiveQueryInfo)
        {}
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    bset<ECInstanceId> GetRecursiveChildrenIds(IParsedInput const& input, SelectClassInfo const& thisInfo)
        {
        bset<ECRelationshipClassCP> relationships;
        for (RelatedClassPath const& path : m_recursiveQueryInfo.GetPathsFromInputToSelectClass())
            {
            for (RelatedClassCR rel : path)
                relationships.insert(rel.GetRelationship());
            }
        return GetRecursiveChildrenIds(relationships, IsRecursiveJoinForward(thisInfo), input.GetInstanceIds(*thisInfo.GetInputClass()));
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
InstanceFilteringResult QueryBuilderHelpers::ApplyInstanceFilter(ComplexPresentationQuery<T>& query, InstanceFilteringParams const& params, RelatedClassPath pathFromSourceClassToSelectClass)
    {
    InstanceFilteringResult result = InstanceFilteringResult::Success;
    if (!pathFromSourceClassToSelectClass.empty())
        pathFromSourceClassToSelectClass.front().SetTargetClassAlias("this"); // Changed in order instance filtering to work using this keyword

    if (nullptr != params.GetInput())
        {
        if (params.GetSelectInfo().GetPathToInputClass().empty())
            {
            bvector<ECInstanceId> const& selectedInstanceIds = params.GetInput()->GetInstanceIds(params.GetSelectInfo().GetSelectClass().GetClass());
            if (!selectedInstanceIds.empty())
                {
                IdsFilteringHelper<bvector<ECInstanceId>> filteringHelper(selectedInstanceIds);
                query.Where(filteringHelper.CreateWhereClause("[this].[ECInstanceId]").c_str(), filteringHelper.CreateBoundValues());
                }
            }
        else
            {
            if (nullptr != params.GetRecursiveQueryInfo())
                {
                // in case of recursive query just bind the children ids
                RecursiveQueriesHelper recursiveQueries(params.GetConnection(), *params.GetRecursiveQueryInfo());
                bset<ECInstanceId> ids = recursiveQueries.GetRecursiveChildrenIds(*params.GetInput(), params.GetSelectInfo());
                IdsFilteringHelper<bset<ECInstanceId>> filteringHelper(ids);
                query.Where(filteringHelper.CreateWhereClause("[this].[ECInstanceId]").c_str(), filteringHelper.CreateBoundValues());
                if (ids.empty())
                    result = InstanceFilteringResult::NoResults;
                }
            else
                {
                BeAssert(!params.GetSelectInfo().GetPathToInputClass().empty());
                pathFromSourceClassToSelectClass.insert(pathFromSourceClassToSelectClass.end(), params.GetSelectInfo().GetPathToInputClass().begin(), params.GetSelectInfo().GetPathToInputClass().end());
                bvector<ECInstanceId> const& ids = params.GetInput()->GetInstanceIds(*params.GetSelectInfo().GetInputClass());
                IdsFilteringHelper<bvector<ECInstanceId>> filteringHelper(ids);
                query.Where(filteringHelper.CreateWhereClause("[related].[ECInstanceId]").c_str(), filteringHelper.CreateBoundValues());
                if (ids.empty())
                    result = InstanceFilteringResult::NoResults;
                }
            }
        }

    if (!pathFromSourceClassToSelectClass.empty())
        query.Join(pathFromSourceClassToSelectClass, true);

    if (params.GetInstanceFilter() && 0 != *params.GetInstanceFilter())
        query.Where(ECExpressionsHelper(params.GetECExpressionsCache()).ConvertToECSql(params.GetInstanceFilter()).c_str(), BoundQueryValuesList());

    return result;
    }
template InstanceFilteringResult QueryBuilderHelpers::ApplyInstanceFilter<ContentQuery>(ComplexContentQuery&, InstanceFilteringParams const&, RelatedClassPath);
template InstanceFilteringResult QueryBuilderHelpers::ApplyInstanceFilter<GenericQuery>(ComplexGenericQuery&, InstanceFilteringParams const&, RelatedClassPath);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static void AppendDerivedClassIds(bset<ECClassId>& classIds, SchemaManagerCR schemas)
    {
    bset<ECClassId> derivedClassIds;
    for (ECClassId classId : classIds)
        {
        ECClassCP baseClass = schemas.GetClass(classId);
        ECDerivedClassesList const& derivedClasses = schemas.GetDerivedClasses(*baseClass);
        for (ECClassCP derivedClass : derivedClasses)
            derivedClassIds.insert(derivedClass->GetId());
        }
    if (!derivedClassIds.empty())
        AppendDerivedClassIds(derivedClassIds, schemas);
    std::move(derivedClassIds.begin(), derivedClassIds.end(), std::inserter(classIds, classIds.end()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
void QueryBuilderHelpers::FilterOutExcludes(ComplexPresentationQuery<T>& query, Utf8CP alias, bvector<SelectClass> const& excludes, SchemaManagerCR schemas)
    {
    if (excludes.empty())
        return;

    bset<ECClassId> excludedClassIds;

    // first insert ids of classes we want to exclude polymorphically
    for (SelectClass const& sc : excludes)
        {
        if (sc.IsSelectPolymorphic())
            excludedClassIds.insert(sc.GetClass().GetId());
        }
    // now insert all their derived class ids
    AppendDerivedClassIds(excludedClassIds, schemas);
    // and finally insert ids of classes we want to exclude non-polymorphically
    for (SelectClass const& sc : excludes)
        {
        if (!sc.IsSelectPolymorphic())
            excludedClassIds.insert(sc.GetClass().GetId());
        }

    IdsFilteringHelper<bset<ECClassId>> helper(excludedClassIds);
    query.Where(helper.CreateWhereClause(Utf8PrintfString("[%s].[ECClassId]", alias).c_str(), true).c_str(), helper.CreateBoundValues());
    }
template void QueryBuilderHelpers::FilterOutExcludes<ContentQuery>(ComplexContentQuery&, Utf8CP, bvector<SelectClass> const&, SchemaManagerCR);
template void QueryBuilderHelpers::FilterOutExcludes<NavigationQuery>(ComplexNavigationQuery&, Utf8CP, bvector<SelectClass> const&, SchemaManagerCR);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderHelpers::ApplyDescriptorOverrides(RefCountedPtr<ContentQuery>& query, ContentDescriptorCR ovr, ECExpressionsCache& ecexpressionsCache)
    {
    // ordering
    if (!ovr.MergeResults() && !ovr.HasContentFlag(ContentFlags::KeysOnly))
        {
        bvector<Utf8CP> sortingFieldNames;
        Utf8String orderByClause;
        ContentDescriptor::Field const* sortingField = ovr.GetSortingField();
        if (nullptr != sortingField)
            {
            ECEnumerationCP enumeration = nullptr;
            if (sortingField->IsPropertiesField() && sortingField->AsPropertiesField()->GetProperties().front().GetProperty().GetIsPrimitive())
                enumeration = sortingField->AsPropertiesField()->GetProperties().front().GetProperty().GetAsPrimitiveProperty()->GetEnumeration();
            if (nullptr != enumeration)
                {
                orderByClause.append(FUNCTION_NAME_GetECEnumerationValue).append("('");
                orderByClause.append(enumeration->GetSchema().GetName()).append("', '");
                orderByClause.append(enumeration->GetName()).append("', ");
                }
            orderByClause.append(QueryHelpers::Wrap(ovr.GetSortingField()->GetName()));
            if (nullptr != enumeration)
                orderByClause.append(")");
            sortingFieldNames.push_back(ovr.GetSortingField()->GetName().c_str());
            }
#ifdef WIP_SORTING_GRID_CONTENT
        else if (ovr.ShowLabels())
            {
            orderByClause = Utf8PrintfString(FUNCTION_NAME_GetSortingValue "(%s)", ContentQueryContract::DisplayLabelFieldName);
            sortingFieldNames.push_back(ContentQueryContract::DisplayLabelFieldName);
            }
#endif
        if (!orderByClause.empty() && SortDirection::Descending == ovr.GetSortDirection())
            orderByClause.append(" DESC");

#ifdef WIP_SORTING_GRID_CONTENT
        sortingFieldNames.push_back(ContentQueryContract::ECInstanceIdFieldName);
        if (!orderByClause.empty())
            orderByClause.append(", ");
        if (nullptr == query->AsComplexQuery() || NeedsNestingToUseAlias(*query, sortingFieldNames))
            orderByClause.append(ContentQueryContract::ECInstanceIdFieldName);
        else
            orderByClause.append(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName));
#endif

        query = CreateNestedQueryIfNecessary(*query, sortingFieldNames);
        Order(*query, orderByClause.c_str());
        }

    // filtering
    if (!ovr.GetFilterExpression().empty())
        {
        query = CreateNestedQuery(*query);
        Utf8String ecsqlExpression = "(";
        ecsqlExpression.append(ECExpressionsHelper(ecexpressionsCache).ConvertToECSql(ovr.GetFilterExpression()));
        ecsqlExpression.append(")");
        if (ecsqlExpression.length() > 2)
            Where(query, ecsqlExpression.c_str(), BoundQueryValuesList());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderHelpers::ApplyPagingOptions(RefCountedPtr<ContentQuery>& query, PageOptionsCR opts)
    {
    if (0 == opts.GetPageSize() && 0 == opts.GetPageStart())
        return;

    QueryBuilderHelpers::Limit<ContentQuery>(*query, opts.GetPageSize(), opts.GetPageStart());
    }

#define DISPLAY_TYPES_EQUAL(lhs, rhs)   lhs == rhs || 0 == strcmp(lhs, rhs)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderHelpers::ApplyDefaultContentFlags(ContentDescriptorR descriptor, Utf8CP displayType, ContentSpecificationCR spec)
    {
    if (0 != descriptor.GetContentFlags())
        {
        // do not apply default flags is there's already something set - we don't
        // want to override whatever is set by the requestor
        return;
        }

    if (DISPLAY_TYPES_EQUAL(ContentDisplayType::Grid, displayType))
        {
        descriptor.AddContentFlag(ContentFlags::ShowLabels);
        }
    else if (DISPLAY_TYPES_EQUAL(ContentDisplayType::PropertyPane, displayType))
        {
        descriptor.AddContentFlag(ContentFlags::MergeResults);
        }
    else if (DISPLAY_TYPES_EQUAL(ContentDisplayType::Graphics, displayType))
        {
        descriptor.AddContentFlag(ContentFlags::NoFields);
        descriptor.AddContentFlag(ContentFlags::KeysOnly);
        }
    else if (DISPLAY_TYPES_EQUAL(ContentDisplayType::List, displayType))
        {
        descriptor.AddContentFlag(ContentFlags::NoFields);
        descriptor.AddContentFlag(ContentFlags::ShowLabels);
        }

    if (spec.GetShowImages())
        descriptor.AddContentFlag(ContentFlags::ShowImages);
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Tautvydas.Zinys                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderHelpers::AddCalculatedFields(ContentDescriptorR descriptor, CalculatedPropertiesSpecificationList const& calculatedProperties,
    ILocalizationProvider const* localizationProvider, Utf8StringCR locale, PresentationRuleSetCR ruleSet, ECClassCP ecClass)
    {
    for (size_t i = 0; i < calculatedProperties.size(); i++)
        {
        Utf8String label = calculatedProperties[i]->GetLabel();

        if (localizationProvider != nullptr)
            LocalizationHelper(*localizationProvider, locale, &ruleSet).LocalizeString(label);

        Utf8String propertyName = Utf8String("CalculatedProperty_").append(std::to_string(i).c_str());

        for (ContentDescriptor::Field const* field : descriptor.GetAllFields())
            {
            if (!field->IsCalculatedPropertyField())
                continue;

            ContentDescriptor::CalculatedPropertyField const* calculatedField = field->AsCalculatedPropertyField();
            if (calculatedField->GetName() == propertyName && nullptr != ecClass && ecClass->Is(calculatedField->GetClass()))
                return;
            }

        descriptor.AddField(new ContentDescriptor::CalculatedPropertyField(label, propertyName,
            calculatedProperties[i]->GetValue(), ecClass, calculatedProperties[i]->GetPriority()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderHelpers::Aggregate(ContentDescriptorPtr& aggregateDescriptor, ContentDescriptorR inputDescriptor)
    {
    if (aggregateDescriptor.IsNull())
        aggregateDescriptor = &inputDescriptor;
    else
        aggregateDescriptor->MergeWith(inputDescriptor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryPtr QueryBuilderHelpers::CreateMergedResultsQuery(ContentQueryR query, ContentDescriptorR descriptor)
    {
    if (nullptr != query.AsComplexQuery() || nullptr != query.AsStringQuery())
        return &query;

    if (0 == ((int)ContentFlags::MergeResults & descriptor.GetContentFlags()))
        return &query;

    // note: the descriptor there is the aggregate descriptor that all unioned queries use;
    // the merging query must clone it and make some modifications
    ContentDescriptorPtr outerDescriptor = ContentDescriptor::Create(descriptor);
    ComplexContentQueryPtr outerQuery = ComplexContentQuery::Create();
    outerQuery->SelectContract(*ContentQueryContract::Create(0, *outerDescriptor, nullptr, *outerQuery));
    outerQuery->From(query);

    QueryBuilderHelpers::Order(query, "");
    descriptor.RemoveContentFlag(ContentFlags::MergeResults);

    return outerQuery;
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelOverrideSelectFieldsBuilder : InstanceLabelOverrideValueSpecificationVisitor
    {
    private:
        bvector<PresentationQueryContractFieldCPtr> m_fields;
    protected:
        void _Visit(InstanceLabelOverrideCompositeValueSpecification const& spec) override
            {
            bvector<PresentationQueryContractFieldCPtr> functionParameters;
            functionParameters.push_back(PresentationQueryContractSimpleField::Create("", Utf8PrintfString("'%s'", spec.GetSeparator().c_str()).c_str()));
            for (auto valuePart : spec.GetValueParts())
                {
                InstanceLabelOverrideSelectFieldsBuilder builder;
                valuePart->GetSpecification()->Accept(builder);
                if (builder.GetSelectFields().empty())
                    {
                    BeAssert(false);
                    continue;
                    }
                functionParameters.push_back(builder.GetSelectFields().front());
                functionParameters.push_back(PresentationQueryContractSimpleField::Create("", valuePart->IsRequired() ? "TRUE" : "FALSE"));
                }
            m_fields.push_back(PresentationQueryContractFunctionField::Create("/CombinedValue/", FUNCTION_NAME_JoinOptionallyRequired, functionParameters));
            }
        void _Visit(InstanceLabelOverridePropertyValueSpecification const& spec) override
            {
            m_fields.push_back(PresentationQueryContractSimpleField::Create("/PropertyValue/", spec.GetPropertyName().c_str()));
            }
        void _Visit(InstanceLabelOverrideClassNameValueSpecification const& spec) override
            {
            m_fields.push_back(PresentationQueryContractFunctionField::Create("/ClassName/", FUNCTION_NAME_GetECClassName,
                {
                PresentationQueryContractSimpleField::Create("/ClassId/", "ECClassId"),
                PresentationQueryContractSimpleField::Create("/Full/", spec.ShouldUseFullName() ? "TRUE" : "FALSE")
                }));
            }
        void _Visit(InstanceLabelOverrideClassLabelValueSpecification const&) override
            {
            m_fields.push_back(PresentationQueryContractFunctionField::Create("/ClassLabel/", FUNCTION_NAME_GetECClassLabel,
                {
                PresentationQueryContractSimpleField::Create("/ClassId/", "ECClassId"),
                }));
            }
        void _Visit(InstanceLabelOverrideBriefcaseIdValueSpecification const&) override
            {
            // WIP: the clause can be replaced with the following when ECSQL start supporting bitwise operators
            // FUNCTION_NAME_ToBase36 "(ECInstanceId >> 40)"
            m_fields.push_back(PresentationQueryContractFunctionField::Create("/Base36BriefcaseId/", FUNCTION_NAME_ToBase36,
                {
                PresentationQueryContractFunctionField::Create("/BriefcaseId/", FUNCTION_NAME_ParseBriefcaseId,
                    {
                    PresentationQueryContractSimpleField::Create("/InstanceId/", "ECInstanceId"),
                    })
                }));
            }
        void _Visit(InstanceLabelOverrideLocalIdValueSpecification const&) override
            {
            // WIP: the clause can be replaced with the following when ECSQL start supporting bitwise operators
            // FUNCTION_NAME_ToBase36 "(ECInstanceId & ((1 << 40) - 1))"
            m_fields.push_back(PresentationQueryContractFunctionField::Create("/Base36LocalId/", FUNCTION_NAME_ToBase36,
                {
                PresentationQueryContractFunctionField::Create("/LocalId/", FUNCTION_NAME_ParseLocalId,
                    {
                    PresentationQueryContractSimpleField::Create("/InstanceId/", "ECInstanceId"),
                    })
                }));
            }
        void _Visit(InstanceLabelOverrideStringValueSpecification const& spec) override
            {
            m_fields.push_back(PresentationQueryContractSimpleField::Create("/StringValue/", Utf8PrintfString("'%s'", spec.GetValue().c_str()).c_str()));
            }
    public:
        bvector<PresentationQueryContractFieldCPtr> const& GetSelectFields() const { return m_fields; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryContractFieldPtr QueryBuilderHelpers::CreateInstanceLabelField(Utf8CP name, bvector<InstanceLabelOverrideValueSpecification const*> const& labelOverrideValueSpecs,
    PresentationQueryContractField const* fallback)
    {
    InstanceLabelOverrideSelectFieldsBuilder builder;
    for (InstanceLabelOverrideValueSpecification const* spec : labelOverrideValueSpecs)
        spec->Accept(builder);

    RefCountedPtr<PresentationQueryContractFunctionField> labelField = PresentationQueryContractFunctionField::Create(name,
        "COALESCE", builder.GetSelectFields());
    if (nullptr != fallback)
        labelField->GetFunctionParameters().push_back(fallback);
    return labelField;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
GenericQueryPtr QueryBuilderHelpers::CreateInstanceLabelQuery(ECClassInstanceKeyCR key, bvector<InstanceLabelOverrideValueSpecification const*> const& labelOverrideValueSpecs)
    {
    ECPropertyCP defaultLabelProperty = (nullptr != key.GetClass()) ? key.GetClass()->GetInstanceLabelProperty() : nullptr;
    Utf8CP defaultLabelClause = (nullptr != defaultLabelProperty) ? defaultLabelProperty->GetName().c_str() : "''";
    RefCountedPtr<PresentationQueryContractSimpleField> defaultLabelField = PresentationQueryContractSimpleField::Create(nullptr, defaultLabelClause);
    PresentationQueryContractFieldPtr fallbackLabelField = PresentationQueryContractFunctionField::Create("/FallbackDisplayLabel/", FUNCTION_NAME_GetECInstanceDisplayLabel,
        {
        PresentationQueryContractSimpleField::Create("/ECClassId/", "ECClassId"),
        PresentationQueryContractSimpleField::Create("/ECInstanceId/", "ECInstanceId"),
        defaultLabelField,
        PresentationQueryContractSimpleField::Create("/RelatedInstanceInfo/", "NULL"),
        });
    PresentationQueryContractFieldPtr labelField = (labelOverrideValueSpecs.size() > 0) ? CreateInstanceLabelField("/DisplayLabel/", labelOverrideValueSpecs, fallbackLabelField.get()) : fallbackLabelField;

    RefCountedPtr<SimpleQueryContract> contract = SimpleQueryContract::Create();
    contract->AddField(*labelField);

    ComplexGenericQueryPtr query = ComplexGenericQuery::Create();
    query->SelectContract(*contract);
    query->From(*key.GetClass(), false);
    query->Where("ECInstanceId = ?", { new BoundQueryId(key.GetId()) });

    return query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IdSet<BeInt64Id> QueryBuilderHelpers::CreateIdSetFromJsonArray(RapidJsonValueCR json)
    {
    IdSet<BeInt64Id> ids;
    if (!json.IsArray())
        {
        BeAssert(false);
        return ids;
        }
    for (Json::ArrayIndex i = 0; i < json.Size(); i++)
        ids.insert(BeInt64Id(json[i].GetUint64()));
    return ids;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2017
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
        BeAssert(false);
    return v;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> QueryBuilderHelpers::GetLabelOverrideValuesMap(ECSchemaHelper const& helper, InstanceLabelOverrideList labelOverrides)
    {
    std::sort(labelOverrides.begin(), labelOverrides.end(), [](InstanceLabelOverrideCP a, InstanceLabelOverrideCP b) {return a->GetPriority() > b->GetPriority();});
    bmap<ECClassCP, bvector<InstanceLabelOverrideValueSpecification const*>> mappedFields;
    for (InstanceLabelOverrideP labelOverride : labelOverrides)
        {
        ECClassCP ecClass = helper.GetECClass(labelOverride->GetClassName().c_str());
        if (nullptr == ecClass)
            {
            BeAssert(false && "Invalid class name");
            continue;
            }
        auto iter = mappedFields.find(ecClass);
        if (iter == mappedFields.end())
            iter = mappedFields.Insert(ecClass, bvector<InstanceLabelOverrideValueSpecification const*>()).first;

        iter->second.insert(iter->second.end(), labelOverride->GetValueSpeficications().begin(), labelOverride->GetValueSpeficications().end());
        }
    return mappedFields;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RelatedClass> QueryBuilderHelpers::GetRelatedInstanceClasses(ECSchemaHelper const& schemaHelper, ECClassCR selectClass,
    RelatedInstanceSpecificationList const& relatedInstanceSpecs, bmap<ECRelationshipClassCP, int>& relationshipUsedCount)
    {
    bvector<RelatedClass> relatedClasses;
    bset<Utf8String> aliases;
    for (RelatedInstanceSpecificationCP spec : relatedInstanceSpecs)
        {
        if (aliases.end() != aliases.find(spec->GetAlias()))
            {
            BeAssert(false && "related instance alias must be unique per parent specification");
            continue;
            }

        ECClassCP relatedClass = schemaHelper.GetECClass(spec->GetClassName().c_str());
        if (nullptr == relatedClass || !relatedClass->IsEntityClass())
            {
            BeAssert(false && "related class not found");
            continue;
            }

        ECClassCP relationshipClass = schemaHelper.GetECClass(spec->GetRelationshipName().c_str());
        if (nullptr == relationshipClass || !relationshipClass->IsRelationshipClass())
            {
            BeAssert(false && "relationship class not found");
            continue;
            }

        bool isForward;
        switch (spec->GetRelationshipDirection())
            {
            case RequiredRelationDirection_Forward:
                {
                if (!relationshipClass->GetRelationshipClassCP()->GetSource().SupportsClass(selectClass))
                    continue;
                isForward = true;
                BeAssert(relationshipClass->GetRelationshipClassCP()->GetTarget().SupportsClass(*relatedClass));
                break;
                }
            case RequiredRelationDirection_Backward:
                {
                if (!relationshipClass->GetRelationshipClassCP()->GetTarget().SupportsClass(selectClass))
                    continue;
                isForward = false;
                BeAssert(relationshipClass->GetRelationshipClassCP()->GetSource().SupportsClass(*relatedClass));
                break;
                }
            default:
                {
                if (relationshipClass->GetRelationshipClassCP()->GetSource().SupportsClass(selectClass))
                    {
                    isForward = true;
                    BeAssert(relationshipClass->GetRelationshipClassCP()->GetTarget().SupportsClass(*relatedClass));
                    }
                else if (relationshipClass->GetRelationshipClassCP()->GetTarget().SupportsClass(selectClass))
                    {
                    isForward = false;
                    BeAssert(relationshipClass->GetRelationshipClassCP()->GetSource().SupportsClass(*relatedClass));
                    }
                else
                    continue;
                }
            }

        RelatedClass relatedClassInfo(selectClass, *relatedClass->GetEntityClassCP(), *relationshipClass->GetRelationshipClassCP(), isForward);
        relatedClassInfo.SetTargetClassAlias(spec->GetAlias().c_str());
        relatedClassInfo.SetRelationshipAlias(Utf8PrintfString("rel_%s_%s_%d", relationshipClass->GetSchema().GetAlias().c_str(), relationshipClass->GetName().c_str(), relationshipUsedCount[relationshipClass->GetRelationshipClassCP()]++).c_str());
        relatedClassInfo.SetIsTargetOptional(!spec->IsRequired());
        relatedClasses.push_back(relatedClassInfo);
        aliases.insert(spec->GetAlias());
        }
    return relatedClasses;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GroupSpecificationCP QueryBuilderHelpers::GetActiveGroupingSpecification(GroupingRuleCR rule, IJsonLocalState const* localState)
    {
    if (rule.GetGroups().empty())
        return nullptr;

    if (rule.GetSettingsId().empty() || nullptr == localState)
        return rule.GetGroups()[0];

    Json::Value settingValue = localState->GetJsonValue(RULES_ENGINE_ACTIVE_GROUPS_LOCAL_STATE_NAMESPACE, Utf8String(rule.GetSettingsId().c_str()).c_str());
    if (!settingValue.isInt())
        {
        BeAssert(false);
        return rule.GetGroups()[0];
        }

    int activeGroupIndex = settingValue.asInt();
    if (activeGroupIndex < 0 || activeGroupIndex >= (int)rule.GetGroups().size())
        {
        BeAssert(false);
        return rule.GetGroups()[0];
        }

    return rule.GetGroups()[activeGroupIndex];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
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
* @bsiclass                                     Grigas.Petraitis                12/2019
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
* @bsimethod                                    Grigas.Petraitis                12/2019
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
                splitInfo.GetSelectClass().GetDerivedExcludedClasses().push_back(SelectClass(*ruleInfo.GetRuleClass(), ruleInfo.IsRulePolymorphic()));

                SelectClassSplitResult customizationClassSplitInfo(SelectClassWithExcludes(*ruleInfo.GetRuleClass(), ruleInfo.IsRulePolymorphic()));
                std::copy(splitInfo.GetSplitPath().begin(), splitInfo.GetSplitPath().end(), std::back_inserter(customizationClassSplitInfo.GetSplitPath()));
                customizationClassSplitInfo.GetSplitPath().push_back(splitInfo.GetSelectClass());

                bvector<SelectClassSplitResult> customizationClassSplitInfos = {customizationClassSplitInfo};
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
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<SelectClassSplitResult> QueryBuilderHelpers::ProcessSelectClassesBasedOnCustomizationRules(bvector<SelectClass> const& selectClasses,
    bvector<RuleApplicationInfo> const& customizationRuleInfos, SchemaManagerCR schemas)
    {
    bvector<SelectClassSplitResult> result = ContainerHelpers::TransformContainer<bvector<SelectClassSplitResult>>(selectClasses, [](SelectClass const& sc){return SelectClassSplitResult(sc);});
    ::ProcessSelectClassesBasedOnCustomizationRules(result, customizationRuleInfos, schemas);
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
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
                path.back().GetTargetClass().GetDerivedExcludedClasses().push_back(SelectClass(*ruleInfo.GetRuleClass(), ruleInfo.IsRulePolymorphic()));

                RelatedClassPath copy(path);
                copy.back().SetTargetClass(SelectClassWithExcludes(*ruleInfo.GetRuleClass(), ruleInfo.IsRulePolymorphic()));
                bvector<RelatedClassPath> derivedPaths({copy});
                ProcessRelationshipPathsBasedOnCustomizationRules(derivedPaths, customizationRuleInfos, schemas);
                std::move(derivedPaths.begin(), derivedPaths.end(), std::back_inserter(childPaths));
                }
            }
        }
    for (RelatedClassPath const& relatedPath : childPaths)
        {
        // TODO: would be nice to filter-out duplicate paths...
        relationshipPaths.push_back(relatedPath);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<RelatedClassPath> QueryBuilderHelpers::ProcessRelationshipPathsBasedOnCustomizationRules(bvector<RelatedClassPath> const& relationshipPaths,
    bvector<RuleApplicationInfo> const& customizationRuleInfos, SchemaManagerCR schemas)
    {
    bvector<RelatedClassPath> result(relationshipPaths);
    ::ProcessRelationshipPathsBasedOnCustomizationRules(result, customizationRuleInfos, schemas);
    return result;
    }
