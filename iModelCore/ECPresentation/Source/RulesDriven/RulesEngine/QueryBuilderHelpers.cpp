/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/QueryBuilderHelpers.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "QueryBuilder.h"
#include "RulesPreprocessor.h"
#include "ECExpressionContextsProvider.h"
#include "LocalizationHelper.h"
#include "NavNodeProviders.h"
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
            orderByClause.append(ovr.GetSortingField()->GetName());
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
        descriptor.AddContentFlag(ContentFlags::ShowLabels);
    
    if (DISPLAY_TYPES_EQUAL(ContentDisplayType::PropertyPane, displayType))
        descriptor.AddContentFlag(ContentFlags::MergeResults);
    
    if (DISPLAY_TYPES_EQUAL(ContentDisplayType::Graphics, displayType))
        descriptor.AddContentFlag(ContentFlags::KeysOnly);

    if (spec.GetShowImages())
        descriptor.AddContentFlag(ContentFlags::ShowImages);
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Tautvydas.Zinys                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderHelpers::AddCalculatedFields(ContentDescriptorR descriptor, CalculatedPropertiesSpecificationList const& calculatedProperties, ILocalizationProvider const* localizationProvider, PresentationRuleSetCR ruleSet, ECClassCP ecClass)
    {
    for (size_t i = 0; i < calculatedProperties.size(); i++)
        {
        Utf8String label = calculatedProperties[i]->GetLabel();

        if (localizationProvider != nullptr)
            LocalizationHelper(*localizationProvider, &ruleSet).LocalizeString(label);

        Utf8String propertyName = Utf8String("CalculatedProperty_").append(std::to_string(i).c_str());

        for (ContentDescriptor::Field const* field : descriptor.GetAllFields())
            {
            if (!field->IsCalculatedPropertyField())
                continue;

            ContentDescriptor::CalculatedPropertyField const* calculatedField = field->AsCalculatedPropertyField();
            if (calculatedField->GetName() == propertyName && nullptr != ecClass && ecClass->Is(calculatedField->GetClass()))
                return;
            }

        descriptor.GetAllFields().push_back(new ContentDescriptor::CalculatedPropertyField(label, propertyName,
            calculatedProperties[i]->GetValue(), ecClass, calculatedProperties[i]->GetPriority()));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String QueryBuilderHelpers::CreateFieldName(ContentDescriptor::ECPropertiesField const& field)
    {
    Utf8String name;
    bool isRelated = field.GetProperties().front().IsRelated();
    if (isRelated)
        name.append("rel_");

    bvector<Utf8CP> relatedClassNames;
    bvector<Utf8CP> propertyClassNames;

    bset<ECClassCP> usedRelatedClasses;
    bset<ECClassCP> usedPropertyClasses;

    for (ContentDescriptor::Property const& prop : field.GetProperties())
        {
        if (prop.IsRelated())
            {
            for (RelatedClass const& related : prop.GetRelatedClassPath())
                {
                if (usedRelatedClasses.end() != usedRelatedClasses.find(related.GetTargetClass()))
                    continue;

                relatedClassNames.push_back(related.GetTargetClass()->GetName().c_str());
                usedRelatedClasses.insert(related.GetTargetClass());
                }
            }

        if (usedPropertyClasses.end() == usedPropertyClasses.find(&prop.GetPropertyClass()))
            {
            propertyClassNames.push_back(prop.GetPropertyClass().GetName().c_str());
            usedPropertyClasses.insert(&prop.GetPropertyClass());
            }
        }

    if (!relatedClassNames.empty())
        name.append(BeStringUtilities::Join(relatedClassNames, "_")).append("_");

    if (!propertyClassNames.empty())
        name.append(BeStringUtilities::Join(propertyClassNames, "_")).append("_");

    name.append(field.GetProperties().front().GetProperty().GetName());
    return name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBuilderHelpers::Aggregate(ContentDescriptorPtr& aggregateDescriptor, ContentDescriptorR inputDescriptor)
    {
    if (aggregateDescriptor.IsNull())
        aggregateDescriptor = &inputDescriptor;
    else
        {
        aggregateDescriptor->MergeWith(inputDescriptor);
        for (ContentDescriptor::Field* field : aggregateDescriptor->GetAllFields())
            {
            if (field->IsPropertiesField())
                {
                ContentDescriptor::ECPropertiesField* propertiesField = field->AsPropertiesField();
                propertiesField->SetName(CreateFieldName(*propertiesField));
                }
            }
        }
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
IdSet<ECInstanceId> QueryBuilderHelpers::CreateIdSetFromJsonArray(RapidJsonValueCR json)
    {
    IdSet<ECInstanceId> ids;
    if (!json.IsArray())
        {
        BeAssert(false);
        return ids;
        }
    for (Json::ArrayIndex i = 0; i < json.Size(); i++)
        ids.insert(ECInstanceId(json[i].GetUint64()));
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
void QueryBuilderHelpers::Reverse(RelatedClassPath& path, Utf8CP firstTargetClassAlias, bool isFirstTargetPolymorphic)
    {
    // first pass: reverse the order in list
    for (size_t i = 0; i < path.size() / 2; ++i)
        {
        RelatedClass& lhs = path[i];
        RelatedClass& rhs = path[path.size() - i - 1];
        RelatedClass tmp = lhs;
        lhs = rhs;
        rhs = tmp;
        }

    // second pass: reverse each spec
    for (size_t i = 0; i < path.size(); ++i)
        {
        RelatedClass& spec = path[i];
        ECClassCP tmp = spec.GetSourceClass();
        spec.SetSourceClass(*spec.GetTargetClass());
        spec.SetTargetClass(*tmp);
        spec.SetTargetClassAlias((i < path.size() - 1) ? path[i + 1].GetTargetClassAlias() : firstTargetClassAlias);
        spec.SetIsPolymorphic((i < path.size() - 1) ? path[i + 1].IsPolymorphic() : isFirstTargetPolymorphic);
        if (nullptr == spec.GetTargetClassAlias() || 0 == *spec.GetTargetClassAlias())
            spec.SetTargetClassAlias(Utf8String(spec.GetTargetClass()->GetName()).ToLower());
        }
    }
