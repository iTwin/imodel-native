/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/ECPresentationManager.h>
#include <ECPresentation/Rules/SpecificationVisitor.h>
#include "../Shared/ECExpressions/ECExpressionContextsProvider.h"
#include "../Shared/Queries/CustomFunctions.h"
#include "ContentQueryBuilder.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ParsedInput::ParsedInput(bvector<ECInstanceKey> const& instanceKeys, INavNodeLocater const& nodesLocater, IConnectionCR connection, ECSchemaHelper const& helper)
    {
    Parse(instanceKeys, nodesLocater, connection, helper);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ParsedInput::GetNodeClasses(ECInstanceKeyCR instanceKey, INavNodeLocater const& nodesLocater, IConnectionCR connection, ECSchemaHelper const& helper)
    {
    ECClassCP ecClass = helper.GetECClass(instanceKey.GetClassId());
    if (nullptr == ecClass || !ecClass->IsEntityClass())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, Utf8PrintfString("Failed to find an ECClass for ID: %" PRIu64, (uint64_t)instanceKey.GetClassId().GetValue()));

    if (m_classInput.end() == m_classInput.find(ecClass))
        m_orderedClasses.push_back(ecClass);
    if (instanceKey.GetInstanceId().IsValid())
        m_classInput[ecClass].push_back(instanceKey.GetInstanceId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ParsedInput::Parse(bvector<ECInstanceKey> const& keys, INavNodeLocater const& nodesLocater, IConnectionCR connection, ECSchemaHelper const& helper)
    {
    for (ECInstanceKeyCR key : keys)
        GetNodeClasses(key, nodesLocater, connection, helper);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECInstanceId> const& ParsedInput::_GetInstanceIds(ECClassCR selectClass) const
    {
    auto iter = m_classInput.find(&selectClass);
    if (m_classInput.end() != iter)
        return iter->second;

    static const bvector<ECInstanceId> s_empty;
    return s_empty;
    }

#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ComplexQueryBuilderPtr WrapQueryIntoGroupingClause(ComplexQueryBuilderR query, ContentQueryContractCR queryContract)
    {
    auto grouped = ComplexQueryBuilder::Create();
    grouped->SelectAll();
    grouped->From(query);
    grouped->GroupByContract(queryContract);
    return grouped;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelatedClassesJoinContext
{
private:
    ContentQueryBuilderParameters const& m_params;
    bvector<ContentDescriptor::Field*> const& m_usedFields;
    std::unique_ptr<bset<Utf8String>> m_usedClassAliases;

private:
    static bool ShouldSkipXToManyField(ContentDescriptor::RelatedContentField const& field, bool skipXToManyRelatedContentFields)
        {
        if (!field.IsXToMany())
            return false;
        return skipXToManyRelatedContentFields || (field.GetPathFromSelectToContentClass().GetTargetsCount().IsValid() && field.GetPathFromSelectToContentClass().GetTargetsCount().Value() > 1);
        }

    static void FindUsedClassAliases(bset<Utf8String>& aliases, bvector<ContentDescriptor::Field*> const& fields, bool skipCompositeFields, bool skipXToManyRelatedContentFields)
        {
        for (ContentDescriptor::Field const* field : fields)
            {
            if (field->IsPropertiesField())
                {
                for (ContentDescriptor::Property const& prop : field->AsPropertiesField()->GetProperties())
                    aliases.insert(prop.GetPrefix());
                }
            if (field->IsNestedContentField())
                {
                bool recurse = false;
                if (field->AsNestedContentField()->AsRelatedContentField() && !ShouldSkipXToManyField(*field->AsNestedContentField()->AsRelatedContentField(), skipXToManyRelatedContentFields))
                    {
                    ContentDescriptor::RelatedContentField const* relatedContentField = field->AsNestedContentField()->AsRelatedContentField();
                    aliases.insert(relatedContentField->IsRelationshipField() ? relatedContentField->GetRelationshipClassAlias() : relatedContentField->GetContentClassAlias());
                    recurse = true;
                    }
                if (field->AsNestedContentField()->AsCompositeContentField() && !skipCompositeFields)
                    {
                    recurse = true;
                    }
                if (recurse)
                    FindUsedClassAliases(aliases, field->AsNestedContentField()->GetFields(), skipCompositeFields, skipXToManyRelatedContentFields);
                }
            }
        }

public:
    RelatedClassesJoinContext(ContentDescriptorCR descriptor, ContentQueryBuilderParameters const& params)
        : m_usedFields(descriptor.GetAllFields()), m_params(params)
        {}
    bset<Utf8String> const& GetUsedClassAliases()
        {
        if (!m_usedClassAliases)
            {
            m_usedClassAliases = std::make_unique<bset<Utf8String>>();
            FindUsedClassAliases(*m_usedClassAliases, m_usedFields, m_params.ShouldSkipCompositePropertyFields(), m_params.ShouldSkipXToManyRelatedContentFields());
            }
        return *m_usedClassAliases;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
enum class RelatedClassType
    {
    NavigationPropertyClass = 1 << 0,
    RelatedPropertyClass    = 1 << 1,
    RelatedInstanceClass    = 1 << 2,
    All = NavigationPropertyClass | RelatedPropertyClass | RelatedInstanceClass,
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void JoinRelatedClasses(ComplexQueryBuilderR query, RelatedClassesJoinContext& context, SelectClassInfo const& selectInfo, int joinedClassTypes)
    {
    if (0 != ((int)RelatedClassType::NavigationPropertyClass & joinedClassTypes))
        {
        // join navigation properties
        for (RelatedClass const& navPropertyClass : selectInfo.GetNavigationPropertyClasses())
            {
            if (context.GetUsedClassAliases().end() != context.GetUsedClassAliases().find(navPropertyClass.GetTargetClass().GetAlias()))
                query.Join(navPropertyClass);
            }
        }

    if (0 != ((int)RelatedClassType::RelatedPropertyClass & joinedClassTypes))
        {
        // join related properties
        for (RelatedClassPath const& path : selectInfo.GetRelatedPropertyPaths())
            {
            if (context.GetUsedClassAliases().end() != context.GetUsedClassAliases().find(path.back().GetTargetClass().GetAlias()))
                query.Join(path, true);
            else if (context.GetUsedClassAliases().end() != context.GetUsedClassAliases().find(path.back().GetRelationship().GetAlias()))
                query.Join(path, false);
            }
        }

    if (0 != ((int)RelatedClassType::RelatedInstanceClass & joinedClassTypes))
        {
        // join related instances
        for (RelatedClassPathCR path : selectInfo.GetRelatedInstancePaths())
            query.Join(path);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String CreateEnumOrderByClause(ContentDescriptor::Field const& field, ECEnumerationCR enumeration)
    {
    Utf8String clause = FUNCTION_NAME_GetECEnumerationValue;
    clause.append("('");
    clause.append(enumeration.GetSchema().GetName()).append("', '");
    clause.append(enumeration.GetName()).append("', ");
    clause.append(QueryHelpers::Wrap(field.GetUniqueName())).append(")");
    return clause;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ExpressionContextPtr CreateContentSpecificationInstanceFilterContext(ContentQueryBuilderParameters const& params, ContentDescriptorCR descriptor)
    {
    ECExpressionContextsProvider::ContentSpecificationInstanceFilterContextParameters contextParams(params.GetConnection(),
        descriptor.GetInputNodeKeys(), params.GetRulesetVariables(), params.GetUsedVariablesListener());
    return ECExpressionContextsProvider::GetContentSpecificationInstanceFilterContext(contextParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String CreateOrderByClause(bvector<SortingRuleCP> const& sortingRules, SelectClassWithExcludes<ECClass> selectClass, ECSchemaHelper const& helper)
    {
    bvector<ClassSortingRule> rules = QueryBuilderHelpers::GetClassSortingRules(sortingRules, selectClass, {}, helper);
    return QueryBuilderHelpers::CreatePropertySortingClause(rules, selectClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void ApplyDescriptorOverrides(RefCountedPtr<PresentationQueryBuilder>& query, ContentDescriptorCR ovr, ContentQueryBuilderParameters const& builderParams)
    {
    auto scope = Diagnostics::Scope::Create("Apply descriptor overrides");

    // ordering
    if (!ovr.MergeResults() && !ovr.HasContentFlag(ContentFlags::KeysOnly))
        {
        bvector<Utf8String> sortingFieldNames;
        Utf8String orderByClause;
        ContentDescriptor::Field const* sortingField = ovr.GetSortingField();
        if (nullptr != sortingField)
            {
            auto sortingScope = Diagnostics::Scope::Create("Set sorting field");
            switch (query->GetContract()->GetFieldType(sortingField->GetUniqueName()))
                {
                case PresentationQueryFieldType::Enum:
                    {
                    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Sorting by enum type field: `%s`", sortingField->GetUniqueName().c_str()));
                    if (!sortingField->IsPropertiesField() || !sortingField->AsPropertiesField()->GetProperties().front().GetProperty().GetIsPrimitive()
                        || nullptr == sortingField->AsPropertiesField()->GetProperties().front().GetProperty().GetAsPrimitiveProperty()->GetEnumeration())
                        {
                        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Content, Utf8PrintfString("Mismatch between property field type in contract and actual property type. "
                            "Field name : '%s'", sortingField->GetUniqueName().c_str()));
                        }
                    ECEnumerationCP enumeration = sortingField->AsPropertiesField()->GetProperties().front().GetProperty().GetAsPrimitiveProperty()->GetEnumeration();
                    orderByClause.append(CreateEnumOrderByClause(*sortingField, *enumeration));
                    break;
                    }
                case PresentationQueryFieldType::LabelDefinition:
                    {
                    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Sorting by label field");
                    orderByClause.append(FUNCTION_NAME_GetLabelDefinitionDisplayValue).append("(")
                        .append(QueryHelpers::Wrap(sortingField->GetUniqueName().c_str())).append(")");
                    break;
                    }
                case PresentationQueryFieldType::NavigationPropertyValue:
                    {
                    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Sorting by navigation type field: `%s`", sortingField->GetUniqueName().c_str()));
                    orderByClause.append(FUNCTION_NAME_GetLabelDefinitionDisplayValue).append("(json_extract(")
                        .append(QueryHelpers::Wrap(sortingField->GetUniqueName().c_str())).append(", '$.label'))");
                    break;
                    }
                default:
                    {
                    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Sorting by field: `%s`", sortingField->GetUniqueName().c_str()));
                    orderByClause.append(QueryHelpers::Wrap(sortingField->GetUniqueName().c_str()));
                    }
                }
            sortingFieldNames.push_back(sortingField->GetUniqueName());
            }
#ifdef WIP_SORTING_GRID_CONTENT
        else if (ovr.ShowLabels())
            {
            orderByClause = Utf8PrintfString(FUNCTION_NAME_GetSortingValue "(%s)", ContentQueryContract::DisplayLabelFieldName);
            sortingFieldNames.push_back(ContentQueryContract::DisplayLabelFieldName);
            }
#endif
        if (!orderByClause.empty() && SortDirection::Descending == ovr.GetSortDirection())
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Sorting in descending order");
            orderByClause.append(" DESC");
            }

#ifdef WIP_SORTING_GRID_CONTENT
        sortingFieldNames.push_back(ContentQueryContract::ECInstanceIdFieldName);
        if (!orderByClause.empty())
            orderByClause.append(", ");
        if (nullptr == query->AsComplexQuery() || NeedsNestingToUseAlias(*query, sortingFieldNames))
            orderByClause.append(ContentQueryContract::ECInstanceIdFieldName);
        else
            orderByClause.append(Utf8PrintfString("[this].[%s]", ContentQueryContract::ECInstanceIdFieldName));
#endif
        if (!orderByClause.empty())
            {
            query = QueryBuilderHelpers::CreateNestedQuery(*query);
            QueryBuilderHelpers::Order(*query, orderByClause.c_str());
            }
        }

    // filtering
    if (!ovr.GetFieldsFilterExpression().empty())
        {
        auto filteringScope = Diagnostics::Scope::Create("Set filtering");
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Applying descriptor's filter expression: `%s`", ovr.GetFieldsFilterExpression().c_str()));

        auto filteringExpressionContext = CreateContentSpecificationInstanceFilterContext(builderParams, ovr);
        auto whereClause = ECExpressionsHelper(builderParams.GetECExpressionsCache()).ConvertToECSql(ovr.GetFieldsFilterExpression(), query->GetContract(), filteringExpressionContext.get());
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Converted expression to ECSQL: `%s`", whereClause.GetClause().c_str()));

        if (!whereClause.GetClause().empty())
            {
            query = QueryBuilderHelpers::CreateNestedQuery(*query);
            QueryBuilderHelpers::Where(query, whereClause);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsSelectClassFilteredOut(SelectClassInfo const& selectClassInfo, InstanceFilterDefinition const* filter)
    {
    if (!filter || !filter->GetSelectClass() || selectClassInfo.GetSelectClass().GetClass().Is(filter->GetSelectClass()))
        return false;

    return !selectClassInfo.GetSelectClass().IsSelectPolymorphic() || !filter->GetSelectClass()->Is(&selectClassInfo.GetSelectClass().GetClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryContractPtr ContentQueryBuilder::CreateContract(ContentDescriptorCR descriptor, SelectClassInfo const& selectInfo, IQueryInfoProvider const& queryInfo)
    {
    PresentationQueryContractFieldPtr displayLabelField;
    if (descriptor.ShowLabels() && descriptor.GetDisplayLabelField())
        {
        auto labelOverrideValuesList = QueryBuilderHelpers::GetInstanceLabelOverrideSpecsForClass(descriptor.GetDisplayLabelField()->GetLabelOverrideSpecs(), selectInfo.GetSelectClass().GetClass());
        displayLabelField = QueryBuilderHelpers::CreateDisplayLabelField(descriptor.GetDisplayLabelField()->GetUniqueName().c_str(), m_params.GetSchemaHelper(), selectInfo.GetSelectClass(),
            nullptr, nullptr, selectInfo.GetRelatedInstancePaths(), labelOverrideValuesList);
        }

    return ContentQueryContract::Create(++m_contractIdsCounter, descriptor, &selectInfo.GetSelectClass().GetClass(),
        queryInfo, displayLabelField, selectInfo.GetRelatedInstancePaths(), m_params.ShouldSkipCompositePropertyFields(), m_params.ShouldSkipXToManyRelatedContentFields());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
QuerySet ContentQueryBuilder::CreateQuerySet(SelectedNodeInstancesSpecificationCR specification, ContentDescriptorCR descriptor, IParsedInput const& specificationInput)
    {
    RelatedClassesJoinContext relatedClassesJoinCtx(descriptor, m_params);
    QuerySet set;
    for (SelectClassInfo const* selectClassInfo : descriptor.GetSelectClasses(specification.GetHash()))
        {
        auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Create query for class `%s`", selectClassInfo->GetSelectClass().GetClass().GetFullName()));

        if (IsSelectClassFilteredOut(*selectClassInfo, descriptor.GetInstanceFilter().get()))
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Skipping select class: `%s` due to instance filter", selectClassInfo->GetSelectClass().GetClass().GetFullName()));
            continue;
            }

        ComplexQueryBuilderPtr classQuery = ComplexQueryBuilder::Create();
        classQuery->From(selectClassInfo->GetSelectClass());

        // join related instance classes
        JoinRelatedClasses(*classQuery, relatedClassesJoinCtx, *selectClassInfo, (int)RelatedClassType::All);

        // create params for input filtering
        auto inputFilter = QueryBuilderHelpers::CreateInputFilter(m_params.GetConnection(), *selectClassInfo, nullptr, specificationInput);
        if (nullptr == inputFilter)
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Results for class `%s` completely filtered-out by input keys. Skipping this query.", selectClassInfo->GetSelectClass().GetClass().GetFullName()));
            continue;
            }

        ContentQueryContractPtr contract = CreateContract(descriptor, *selectClassInfo, *classQuery);
        classQuery->SelectContract(*contract, selectClassInfo->GetSelectClass().GetAlias().c_str());

        // handle instances filtering
        auto filteringExpressionContext = CreateContentSpecificationInstanceFilterContext(m_params, descriptor);
        QueryBuilderHelpers::ApplyInstanceFilter(*classQuery, InstanceFilteringParams(m_params.GetECExpressionsCache(), filteringExpressionContext.get(),
            nullptr, inputFilter.get(), descriptor.GetInstanceFilter().get(), selectClassInfo->GetSelectClass()));

#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
        // handle selecting property for distinct values
        if (descriptor.GetDistinctField() != nullptr)
            classQuery = WrapQueryIntoGroupingClause(*classQuery, *contract);
#endif
        classQuery->OrderBy(CreateOrderByClause(m_params.GetRulesPreprocessor().GetSortingRules(), selectClassInfo->GetSelectClass(), m_params.GetSchemaHelper()).c_str());
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_INFO, Utf8PrintfString("Created query: `%s`", classQuery->GetQuery()->GetQueryString().c_str()));
        QueryBuilderHelpers::AddToUnionSet(set, *classQuery);
        }

    return set;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct HandledRecursiveClassesKey
    {
    ECClassCP m_inputClass;
    ECClassCP m_selectClass;
    bool m_isForward;
    HandledRecursiveClassesKey() : m_inputClass(nullptr), m_selectClass(nullptr), m_isForward(false) {}
    HandledRecursiveClassesKey(SelectClassInfo const& info)
        {
        m_inputClass = info.GetInputClass();
        m_selectClass = &info.GetSelectClass().GetClass();
        m_isForward = info.GetPathFromInputToSelectClass().back().IsForwardRelationship();
        }
    bool operator<(HandledRecursiveClassesKey const& other) const
        {
        return m_inputClass < other.m_inputClass
            || m_inputClass == other.m_inputClass && m_selectClass < other.m_selectClass
            || m_inputClass == other.m_inputClass && m_selectClass == other.m_selectClass && m_isForward < other.m_isForward;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
QuerySet ContentQueryBuilder::CreateQuerySet(ContentRelatedInstancesSpecificationCR specification, ContentDescriptorCR descriptor, IParsedInput const& specificationInput)
    {
    bvector<SelectClassInfo const*> selectClasses = descriptor.GetSelectClasses(specification.GetHash());

    std::unique_ptr<RecursiveQueryInfo const> recursiveInfo;
    if (specification.IsRecursive())
        {
        auto paths = ContainerHelpers::TransformContainer<bvector<RelatedClassPath>>(selectClasses, [](auto const& selectClass)
            {
            return selectClass->GetPathFromInputToSelectClass();
            });
        recursiveInfo = std::make_unique<RecursiveQueryInfo>(paths);
        }

    RelatedClassesJoinContext relatedClassesJoinCtx(descriptor, m_params);
    bset<HandledRecursiveClassesKey> recursiverlyHandledClasses;
    QuerySet set;
    for (SelectClassInfo const* selectClassInfo : selectClasses)
        {
        auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Create query for class `%s`", selectClassInfo->GetSelectClass().GetClass().GetFullName()));

        if (specification.IsRecursive())
            {
            HandledRecursiveClassesKey key(*selectClassInfo);
            if (recursiverlyHandledClasses.end() != recursiverlyHandledClasses.find(key))
                continue;
            recursiverlyHandledClasses.insert(key);
            }

        if (IsSelectClassFilteredOut(*selectClassInfo, descriptor.GetInstanceFilter().get()))
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Skipping select class: `%s` due to instance filter", selectClassInfo->GetSelectClass().GetClass().GetFullName()));
            continue;
            }

        Utf8CP selectClassAlias = selectClassInfo->GetSelectClass().GetAlias().c_str();

        ComplexQueryBuilderPtr classQuery = ComplexQueryBuilder::Create();
        classQuery->From(selectClassInfo->GetSelectClass());

        // join related instance classes
        JoinRelatedClasses(*classQuery, relatedClassesJoinCtx, *selectClassInfo, (int)RelatedClassType::All);

        // create params for input filtering
        auto inputFilter = QueryBuilderHelpers::CreateInputFilter(m_params.GetConnection(), *selectClassInfo, recursiveInfo.get(), specificationInput);
        if (nullptr == inputFilter && m_params.ShouldOmitFilteredOutQueries())
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Results for class `%s` completely filtered-out by input keys. Skipping this query.", selectClassInfo->GetSelectClass().GetClass().GetFullName()));
            continue;
            }

        ContentQueryContractPtr contract = CreateContract(descriptor, *selectClassInfo, *classQuery);
        if (inputFilter && inputFilter->GetPathToInputClass())
            contract->SetInputClassAlias(inputFilter->GetPathToInputClass()->back().GetTargetClass().GetAlias());
        if (inputFilter && inputFilter->GetInputKey().IsValid())
            contract->SetInputInstanceKey(inputFilter->GetInputKey());
        classQuery->SelectContract(*contract, selectClassAlias);

        // handle instance filtering
        auto filteringExpressionContext = CreateContentSpecificationInstanceFilterContext(m_params, descriptor);
        QueryBuilderHelpers::ApplyInstanceFilter(*classQuery, InstanceFilteringParams(m_params.GetECExpressionsCache(), filteringExpressionContext.get(),
            specification.GetInstanceFilter().c_str(), inputFilter.get(), descriptor.GetInstanceFilter().get(), selectClassInfo->GetSelectClass()));

#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
        // handle selecting property for distinct values
        if (descriptor.GetDistinctField() != nullptr)
            classQuery = WrapQueryIntoGroupingClause(*classQuery, *contract);
#endif
        classQuery->OrderBy(CreateOrderByClause(m_params.GetRulesPreprocessor().GetSortingRules(), selectClassInfo->GetSelectClass(), m_params.GetSchemaHelper()).c_str());
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_INFO, Utf8PrintfString("Created query: `%s`", classQuery->GetQuery()->GetQueryString().c_str()));
        QueryBuilderHelpers::AddToUnionSet(set, *classQuery);
        }

    return set;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
QuerySet ContentQueryBuilder::CreateQuerySet(ContentInstancesOfSpecificClassesSpecificationCR specification, ContentDescriptorCR descriptor)
    {
    RelatedClassesJoinContext relatedClassesJoinCtx(descriptor, m_params);
    QuerySet set;
    for (SelectClassInfo const* selectClassInfo : descriptor.GetSelectClasses(specification.GetHash()))
        {
        auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Create query for class `%s`", selectClassInfo->GetSelectClass().GetClass().GetFullName()));

        if (IsSelectClassFilteredOut(*selectClassInfo, descriptor.GetInstanceFilter().get()))
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Skipping select class: `%s` due to instance filter", selectClassInfo->GetSelectClass().GetClass().GetFullName()));
            continue;
            }

        ComplexQueryBuilderPtr classQuery = ComplexQueryBuilder::Create();
        ContentQueryContractPtr contract = CreateContract(descriptor, *selectClassInfo, *classQuery);
        classQuery->SelectContract(*contract, selectClassInfo->GetSelectClass().GetAlias().c_str());
        classQuery->From(selectClassInfo->GetSelectClass());

        // join related instance classes
        JoinRelatedClasses(*classQuery, relatedClassesJoinCtx, *selectClassInfo, (int)RelatedClassType::All);

        // handle instance filtering
        auto filteringExpressionContext = CreateContentSpecificationInstanceFilterContext(m_params, descriptor);
        QueryBuilderHelpers::ApplyInstanceFilter(*classQuery, InstanceFilteringParams(m_params.GetECExpressionsCache(), filteringExpressionContext.get(),
            specification.GetInstanceFilter().c_str(), nullptr, descriptor.GetInstanceFilter().get(), selectClassInfo->GetSelectClass()));

#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
        // handle selecting property for distinct values
        if (descriptor.GetDistinctField() != nullptr)
            classQuery = WrapQueryIntoGroupingClause(*classQuery, *contract);
#endif
        classQuery->OrderBy(CreateOrderByClause(m_params.GetRulesPreprocessor().GetSortingRules(), selectClassInfo->GetSelectClass(), m_params.GetSchemaHelper()).c_str());
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_INFO, Utf8PrintfString("Created query: `%s`", classQuery->GetQuery()->GetQueryString().c_str()));
        QueryBuilderHelpers::AddToUnionSet(set, *classQuery);
        }

    return set;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
QuerySet ContentQueryBuilder::CreateQuerySet(ContentDescriptor::NestedContentField const& contentField)
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Create queries for nested content field: `%s`", contentField.GetUniqueName().c_str()));

    ContentDescriptorBuilder::Context descriptorContext(m_params.GetSchemaHelper(), m_params.GetConnections(), m_params.GetConnection(), m_params.GetCancellationToken(), m_params.GetRulesPreprocessor(), m_params.GetRuleset(),
        ContentDisplayType::Undefined, m_params.GetRulesetVariables(), m_params.GetCategorySupplier(), m_params.GetPropertyFormatter(), ECPresentation::UnitSystem::Undefined,
        *NavNodeKeyListContainer::Create(), nullptr);
    ContentDescriptorPtr descriptor = ContentDescriptorBuilder(descriptorContext).CreateDescriptor(contentField);
    if (!descriptor.IsValid())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, "Got empty descriptor - returning empty query set");
        return QuerySet();
        }

    ComplexQueryBuilderPtr query = ComplexQueryBuilder::Create();
    SelectClassInfo selectClassInfo(contentField.GetContentClass(), contentField.GetContentClassAlias(), true);
    ContentQueryContractPtr contract = CreateContract(*descriptor, selectClassInfo, *query);
    if (contentField.AsRelatedContentField() && contentField.AsRelatedContentField()->IsRelationshipField())
        {
        contract->SetRelationshipClass(&contentField.AsRelatedContentField()->GetRelationshipClass());
        contract->SetRelationshipClassAlias(contentField.AsRelatedContentField()->GetRelationshipClassAlias());
        query->SelectContract(*contract, contentField.AsRelatedContentField()->GetRelationshipClassAlias().c_str());
        }
    else
        query->SelectContract(*contract, contentField.GetContentClassAlias().c_str());
    query->From(selectClassInfo.GetSelectClass());

    RelatedClassesJoinContext relatedClassesJoinCtx(*descriptor, m_params);
    JoinRelatedClasses(*query, relatedClassesJoinCtx, descriptor->GetSelectClasses().back(), (int)RelatedClassType::All);

    ContentDescriptor::RelatedContentField const* relatedContentField = contentField.AsRelatedContentField();
    if (nullptr != relatedContentField)
        {
        RelatedClassPath pathFromContentToSelectClass(relatedContentField->GetPathFromSelectToContentClass());
        pathFromContentToSelectClass.Reverse(relatedContentField->GetSelectClassAlias(), true);
        for (RelatedClass& rc : pathFromContentToSelectClass)
            rc.SetIsTargetOptional(false);
        query->Join(pathFromContentToSelectClass);

        RelatedClassCR lastStep = relatedContentField->GetPathFromSelectToContentClass().back();
        if (!lastStep.GetTargetInstanceFilter().empty())
            {
            Utf8String instanceFilter(lastStep.GetTargetInstanceFilter());
            instanceFilter.ReplaceAll(lastStep.GetTargetClass().GetAlias().c_str(), contentField.GetContentClassAlias().c_str());
            query->Where(instanceFilter.c_str(), BoundQueryValuesList());
            }
        }

    if (relatedContentField && relatedContentField->IsRelationshipField())
        {
        SelectClassWithExcludes<ECClass> relationshipClass(relatedContentField->GetRelationshipClass(), relatedContentField->GetRelationshipClassAlias(), true);
        query->OrderBy(CreateOrderByClause(m_params.GetRulesPreprocessor().GetSortingRules(), relationshipClass, m_params.GetSchemaHelper()).c_str());
        }
    else
        query->OrderBy(CreateOrderByClause(m_params.GetRulesPreprocessor().GetSortingRules(), selectClassInfo.GetSelectClass(), m_params.GetSchemaHelper()).c_str());
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_INFO, Utf8PrintfString("Created query: `%s`", query->GetQuery()->GetQueryString().c_str()));
    return QuerySet({ query });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultiContentQueryBuilder::Accept(SelectedNodeInstancesSpecificationCR specification, IParsedInput const& input)
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Create queries for %s", DiagnosticsHelpers::CreateRuleIdentifier(specification).c_str()));

    QuerySet querySet = m_builder->CreateQuerySet(specification, *m_descriptor, input);
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Created %" PRIu64 " queries.", (uint64_t)querySet.GetQueries().size()));

    if (querySet.GetQueries().empty())
        return false;

    for (auto const& query : querySet.GetQueries())
        QueryBuilderHelpers::AddToUnionSet(m_unions, *query);
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultiContentQueryBuilder::Accept(ContentRelatedInstancesSpecificationCR specification, IParsedInput const& input)
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Create queries for %s", DiagnosticsHelpers::CreateRuleIdentifier(specification).c_str()));

    QuerySet querySet = m_builder->CreateQuerySet(specification, *m_descriptor, input);
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Created %" PRIu64 " queries.", (uint64_t)querySet.GetQueries().size()));

    if (querySet.GetQueries().empty())
        return false;

    for (auto const& query : querySet.GetQueries())
        QueryBuilderHelpers::AddToUnionSet(m_unions, *query);
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MultiContentQueryBuilder::Accept(ContentInstancesOfSpecificClassesSpecificationCR specification)
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Create queries for %s", DiagnosticsHelpers::CreateRuleIdentifier(specification).c_str()));

    QuerySet querySet = m_builder->CreateQuerySet(specification, *m_descriptor);
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Content, LOG_TRACE, Utf8PrintfString("Created %" PRIu64 " queries.", (uint64_t)querySet.GetQueries().size()));

    if (querySet.GetQueries().empty())
        return false;

    for (auto const& query : querySet.GetQueries())
        QueryBuilderHelpers::AddToUnionSet(m_unions, *query);
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
QuerySet const& MultiContentQueryBuilder::GetQuerySet()
    {
    if (!m_adjustmentsApplied)
        {
        auto scope = Diagnostics::Scope::Create("Applying query adjustments: descriptor filtering, sorting, paging");
        for (auto& query : m_unions.GetQueries())
            {
            // handle descriptor-level filtering & sorting
            ApplyDescriptorOverrides(query, *m_descriptor, m_builder->GetParameters());

            // handle paging
            if (!m_pageOptions.Empty())
                QueryBuilderHelpers::Limit(*query, m_pageOptions.GetPageSize(), m_pageOptions.GetPageStart());
            }
        m_adjustmentsApplied = true;
        }
    return m_unions;
    }
