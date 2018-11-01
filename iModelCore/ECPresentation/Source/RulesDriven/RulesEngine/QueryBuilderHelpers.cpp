/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/QueryBuilderHelpers.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "QueryBuilderHelpers.h"
#include "RulesPreprocessor.h"
#include "ECExpressionContextsProvider.h"
#include "LocalizationHelper.h"
#include "NavNodeProviders.h"
#include "LoggingHelper.h"
#include <regex>

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
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
bool QueryBuilderHelpers::NeedsNestingToUseAlias(T const& query, bvector<Utf8CP> const& aliases)
    {
    if (aliases.empty())
        return false;

    Utf8String regexStr = "\\s+AS\\s+\\[?(";
    regexStr.append(aliases.empty() ? "\\w+" : BeStringUtilities::Join(aliases, "|").c_str()).append(")\\]?");
    std::regex regex(regexStr.c_str(), std::regex_constants::icase);

    if (nullptr != query.AsStringQuery())
        return true;

    if (nullptr != query.AsComplexQuery() && std::regex_search(query.AsComplexQuery()->GetClause(CLAUSE_Select).c_str(), regex))
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
        if (selectInfo.GetPathToPrimaryClass().empty())
            {
            BeAssert(false);
            return true;
            }
    
        return !selectInfo.GetPathToPrimaryClass().back().IsForwardRelationship(); // invert direction
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                04/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static bool IsPathValidForRecursiveSelect(Utf8StringR errorMessage, RelatedClassPath const& path, ECClassCR selectClass)
        {
        RelatedClass const& relatedClassDef = path.front();
        if (!selectClass.Is(relatedClassDef.GetSourceClass()))
            {
            errorMessage = "Using IsRecursive requires recursive relationship";
            return false;
            }
        return true;
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                03/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void RecursivelySelectRelatedKeys(bset<ECInstanceId>& result, Utf8StringCR baseQuery, Utf8CP idSelector, bvector<ECInstanceId> const& sourceIds)
        {
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
        for (RelatedClassPath const& path : m_recursiveQueryInfo.GetPathsToPrimary())
            {
            Utf8String validationErrorMessage;
            if (!IsPathValidForRecursiveSelect(validationErrorMessage, path, *path.front().GetSourceClass()))
                {
                BeAssert(false);
                LoggingHelper::LogMessage(Log::Content, validationErrorMessage.c_str(), NativeLogging::LOG_ERROR);
                continue;
                }
            for (RelatedClassCR rel : path)
                relationships.insert(rel.GetRelationship());
            }
        return GetRecursiveChildrenIds(relationships, IsRecursiveJoinForward(thisInfo), input.GetInstanceIds(*thisInfo.GetPrimaryClass()));
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
void QueryBuilderHelpers::ApplyInstanceFilter(ComplexPresentationQuery<T>& query, InstanceFilteringParams const& params, RelatedClassPath relatedClassPath)
    {
    if (!relatedClassPath.empty())
        relatedClassPath.front().SetTargetClassAlias("this"); // Changed in order instance filtering to work using this keyword

    if (nullptr != params.GetInput())
        {
        if (params.GetSelectInfo().GetPathToPrimaryClass().empty())
            {
            bvector<ECInstanceId> const& selectedInstanceIds = params.GetInput()->GetInstanceIds(params.GetSelectInfo().GetSelectClass());
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
                }
            else
                {
                BeAssert(!params.GetSelectInfo().GetPathToPrimaryClass().empty());
                relatedClassPath.insert(relatedClassPath.end(), params.GetSelectInfo().GetPathToPrimaryClass().begin(), params.GetSelectInfo().GetPathToPrimaryClass().end());
                bvector<ECInstanceId> const& ids = params.GetInput()->GetInstanceIds(*params.GetSelectInfo().GetPrimaryClass());
                IdsFilteringHelper<bvector<ECInstanceId>> filteringHelper(ids);
                query.Where(filteringHelper.CreateWhereClause("[related].[ECInstanceId]").c_str(), filteringHelper.CreateBoundValues());
                }
            }
        }
    
    if (!relatedClassPath.empty())
        query.Join(relatedClassPath, false);

    if (params.GetInstanceFilter() && 0 != *params.GetInstanceFilter())
        query.Where(ECExpressionsHelper(params.GetECExpressionsCache()).ConvertToECSql(params.GetInstanceFilter()).c_str(), BoundQueryValuesList());
    }
template void QueryBuilderHelpers::ApplyInstanceFilter<ContentQuery>(ComplexContentQuery&, InstanceFilteringParams const&, RelatedClassPath);
template void QueryBuilderHelpers::ApplyInstanceFilter<GenericQuery>(ComplexGenericQuery&, InstanceFilteringParams const&, RelatedClassPath);

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
    BeAssert(0 == descriptor.GetContentFlags());

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
* @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RelatedClassPath::Reverse(Utf8CP firstTargetClassAlias, bool isFirstTargetPolymorphic)
    {
    // first pass: reverse the order in list
    for (size_t i = 0; i < size() / 2; ++i)
        {
        RelatedClass& lhs = at(i);
        RelatedClass& rhs = at(size() - i - 1);
        RelatedClass tmp = lhs;
        lhs = rhs;
        rhs = tmp;
        }

    // second pass: reverse each spec
    for (size_t i = 0; i < size(); ++i)
        {
        RelatedClass& spec = at(i);
        ECClassCP tmp = spec.GetSourceClass();
        spec.SetSourceClass(*spec.GetTargetClass());
        spec.SetTargetClass(*tmp);
        spec.SetTargetClassAlias((i < size() - 1) ? at(i + 1).GetTargetClassAlias() : firstTargetClassAlias);
        spec.SetIsPolymorphic((i < size() - 1) ? at(i + 1).IsPolymorphic() : isFirstTargetPolymorphic);
        if (nullptr == spec.GetTargetClassAlias() || 0 == *spec.GetTargetClassAlias())
            spec.SetTargetClassAlias(Utf8String(spec.GetTargetClass()->GetName()).ToLower());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bmap<ECClassCP, bvector<ECPropertyCP>> QueryBuilderHelpers::GetMappedLabelOverridingProperties(ECSchemaHelper const& helper, InstanceLabelOverrideList labelOverrides)
    {
    std::sort(labelOverrides.begin(),labelOverrides.end(), [](InstanceLabelOverrideCP a, InstanceLabelOverrideCP b) {return a->GetPriority() > b->GetPriority();});
    bmap<ECClassCP, bvector<ECPropertyCP>> mappedProperties;
    for (InstanceLabelOverrideP labelOverride : labelOverrides)
        {
        ECClassCP ecClass = helper.GetECClass(labelOverride->GetClassName().c_str());
        if (nullptr == ecClass)
            {
            BeAssert(false && "Invalid class name");
            continue;
            }
        auto iter = mappedProperties.find(ecClass);
        if (iter == mappedProperties.end())
            iter = mappedProperties.Insert(ecClass, bvector<ECPropertyCP>()).first;

        bvector<ECPropertyCP> properties;
        for (Utf8StringCR propertyName : labelOverride->GetPropertyNames())
            {
            ECPropertyCP ecProperty = ecClass->GetPropertyP(propertyName);
            if (nullptr == ecProperty)
                {
                BeAssert(false && "Invalid property name");
                continue;
                }
            iter->second.push_back(ecProperty);
            }
        }
    return mappedProperties;
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
        relatedClassInfo.SetIsOuterJoin(!spec->IsRequired());
        relatedClasses.push_back(relatedClassInfo);
        aliases.insert(spec->GetAlias());
        }
    return relatedClasses;
    }
