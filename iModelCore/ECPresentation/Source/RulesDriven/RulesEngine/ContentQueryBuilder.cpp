/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>
#include "QueryBuilder.h"
#include "ECExpressionContextsProvider.h"
#include "LoggingHelper.h"
#include "NavNodeProviders.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ParsedInput::ParsedInput(bvector<ECInstanceKey> const& instanceKeys, INavNodeLocater const& nodesLocater, IConnectionCR connection, ECSchemaHelper const& helper)
    {
    Parse(instanceKeys, nodesLocater, connection, helper);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ParsedInput::GetNodeClasses(ECInstanceKeyCR instanceKey, INavNodeLocater const& nodesLocater, IConnectionCR connection, ECSchemaHelper const& helper)
    {
    ECClassCP ecClass = helper.GetECClass(instanceKey.GetClassId());
    if (nullptr == ecClass || !ecClass->IsEntityClass())
        {
        BeAssert(false);
        return;
        }
    if (m_classInput.end() == m_classInput.find(ecClass))
        m_orderedClasses.push_back(ecClass);
    if (instanceKey.GetInstanceId().IsValid())
        m_classInput[ecClass].push_back(instanceKey.GetInstanceId());
    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ParsedInput::Parse(bvector<ECInstanceKey> const& keys, INavNodeLocater const& nodesLocater, IConnectionCR connection, ECSchemaHelper const& helper)
    {
    for (ECInstanceKeyCR key : keys)
        GetNodeClasses(key, nodesLocater, connection, helper);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECInstanceId> const& ParsedInput::_GetInstanceIds(ECClassCR selectClass) const
    {
    auto iter = m_classInput.find(&selectClass);
    if (m_classInput.end() != iter)
        return iter->second;

    static const bvector<ECInstanceId> s_empty;
    return s_empty;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<RelatedClassPath> GetPathsToSelectClass(bvector<SelectClassInfo> const& selectInfos)
    {
    bvector<RelatedClassPath> paths;
    for (SelectClassInfo const& info : selectInfos)
        paths.push_back(info.GetPathFromInputToSelectClass());
    return paths;
    }

#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static ComplexContentQueryPtr WrapQueryIntoGroupingClause(ComplexContentQueryR query, ContentQueryContractCR queryContract)
    {
    ComplexContentQueryPtr grouped = ComplexContentQuery::Create();
    grouped->SelectAll();
    grouped->From(query);
    grouped->GroupByContract(queryContract);
    return grouped;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void JoinRelatedClasses(ComplexContentQueryR query, SelectClassInfo const& selectInfo)
    {
    // join navigation properties
    for (RelatedClass const& navPropertyClass : selectInfo.GetNavigationPropertyClasses())
        query.Join(navPropertyClass, true);

    // join related properties
    for (RelatedClassPath const& path : selectInfo.GetRelatedPropertyPaths())
        query.Join(path, true);

    // join related instances
    for (RelatedClassPathCR path : selectInfo.GetRelatedInstancePaths())
        query.Join(path, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryPtr ContentQueryBuilder::CreateQuery(SelectedNodeInstancesSpecificationCR specification, ContentDescriptorCR descriptor, IParsedInput const& input)
    {
    ContentDescriptorBuilder::Context descriptorContext(m_params.GetSchemaHelper(), m_params.GetConnections(), m_params.GetConnection(), m_params.GetRuleset(),
        descriptor.GetPreferredDisplayType().c_str(), m_params.GetCategorySupplier(), m_params.GetPropertyFormatter(), ECPresentation::UnitSystem::Undefined,
        m_params.GetLocalizationProvider(), m_params.GetLocale(), descriptor.GetInputNodeKeys(), descriptor.GetSelectionInfo());
    descriptorContext.SetContentFlagsCalculator([flags = descriptor.GetContentFlags()](int){return flags;});
    ContentDescriptorPtr specificationDescriptor = ContentDescriptorBuilder(descriptorContext).CreateDescriptor(specification, input);
    if (specificationDescriptor.IsNull())
        return nullptr;

    ContentQueryPtr query;
    for (SelectClassInfo const& selectClassInfo : specificationDescriptor->GetSelectClasses())
        {
        ComplexContentQueryPtr classQuery = ComplexContentQuery::Create();
        ContentQueryContractPtr contract = ContentQueryContract::Create(++m_contractIdsCounter, descriptor, &selectClassInfo.GetSelectClass().GetClass(),
            *classQuery, selectClassInfo.GetRelatedInstancePaths());
        classQuery->SelectContract(*contract, "this");
        classQuery->From(selectClassInfo.GetSelectClass(), "this");

        // handle related classes
        JoinRelatedClasses(*classQuery, selectClassInfo);

        // exclude derived classes if necessary
        QueryBuilderHelpers::FilterOutExcludes(*classQuery, "this", selectClassInfo.GetSelectClass().GetDerivedExcludedClasses(), m_params.GetConnection().GetECDb().Schemas());

        // handle filtering
        InstanceFilteringParams filteringParams(m_params.GetConnection(), m_params.GetECExpressionsCache(), &input, selectClassInfo, nullptr, nullptr);
        QueryBuilderHelpers::ApplyInstanceFilter(*classQuery, filteringParams, RelatedClassPath());

#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
        // handle selecting property for distinct values
        if (descriptor.GetDistinctField() != nullptr)
            classQuery = WrapQueryIntoGroupingClause(*classQuery, *contract);
#endif

        QueryBuilderHelpers::SetOrUnion<ContentQuery>(query, *classQuery);
        }

    QueryBuilderHelpers::ApplyDescriptorOverrides(query, descriptor, m_params.GetECExpressionsCache());

    return query;
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2018
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
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryPtr ContentQueryBuilder::CreateQuery(ContentRelatedInstancesSpecificationCR specification, ContentDescriptorCR descriptor, IParsedInput const& input)
    {
    ContentDescriptorBuilder::Context descriptorContext(m_params.GetSchemaHelper(), m_params.GetConnections(), m_params.GetConnection(), m_params.GetRuleset(),
        descriptor.GetPreferredDisplayType().c_str(), m_params.GetCategorySupplier(), m_params.GetPropertyFormatter(), ECPresentation::UnitSystem::Undefined,
        m_params.GetLocalizationProvider(), m_params.GetLocale(), descriptor.GetInputNodeKeys(), descriptor.GetSelectionInfo());
    descriptorContext.SetContentFlagsCalculator([flags = descriptor.GetContentFlags()](int){return flags;});
    ContentDescriptorPtr specificationDescriptor = ContentDescriptorBuilder(descriptorContext).CreateDescriptor(specification, input);
    if (specificationDescriptor.IsNull())
        return nullptr;

    InstanceFilteringParams::RecursiveQueryInfo const* recursiveInfo = nullptr;
    if (specification.IsRecursive())
        recursiveInfo = new InstanceFilteringParams::RecursiveQueryInfo(GetPathsToSelectClass(specificationDescriptor->GetSelectClasses()));

    bset<HandledRecursiveClassesKey> recursiverlyHandledClasses;
    ContentQueryPtr query;
    for (SelectClassInfo const& selectClassInfo : specificationDescriptor->GetSelectClasses())
        {
        if (specification.IsRecursive())
            {
            HandledRecursiveClassesKey key(selectClassInfo);
            if (recursiverlyHandledClasses.end() != recursiverlyHandledClasses.find(key))
                continue;
            recursiverlyHandledClasses.insert(key);
            }

        ComplexContentQueryPtr classQuery = ComplexContentQuery::Create();
        ContentQueryContractPtr contract = ContentQueryContract::Create(++m_contractIdsCounter, descriptor, &selectClassInfo.GetSelectClass().GetClass(),
            *classQuery, selectClassInfo.GetRelatedInstancePaths());
        classQuery->SelectContract(*contract, "this");
        classQuery->From(selectClassInfo.GetSelectClass(), "this");

        // handle related classes
        JoinRelatedClasses(*classQuery, selectClassInfo);

        // exclude derived classes if necessary
        QueryBuilderHelpers::FilterOutExcludes(*classQuery, "this", selectClassInfo.GetSelectClass().GetDerivedExcludedClasses(), m_params.GetConnection().GetECDb().Schemas());

        // handle filtering
        InstanceFilteringParams filteringParams(m_params.GetConnection(), m_params.GetECExpressionsCache(), &input,
            selectClassInfo, recursiveInfo, specification.GetInstanceFilter().c_str());
        QueryBuilderHelpers::ApplyInstanceFilter(*classQuery, filteringParams, RelatedClassPath());

#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
        // handle selecting property for distinct values
        if (descriptor.GetDistinctField() != nullptr)
            classQuery = WrapQueryIntoGroupingClause(*classQuery, *contract);
#endif

        QueryBuilderHelpers::SetOrUnion<ContentQuery>(query, *classQuery);
        }

    QueryBuilderHelpers::ApplyDescriptorOverrides(query, descriptor, m_params.GetECExpressionsCache());
    DELETE_AND_CLEAR(recursiveInfo);

    return query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryPtr ContentQueryBuilder::CreateQuery(ContentInstancesOfSpecificClassesSpecificationCR specification, ContentDescriptorCR descriptor)
    {
    ContentDescriptorBuilder::Context descriptorContext(m_params.GetSchemaHelper(), m_params.GetConnections(), m_params.GetConnection(), m_params.GetRuleset(),
        descriptor.GetPreferredDisplayType().c_str(), m_params.GetCategorySupplier(), m_params.GetPropertyFormatter(), ECPresentation::UnitSystem::Undefined,
        m_params.GetLocalizationProvider(), m_params.GetLocale(), descriptor.GetInputNodeKeys(), descriptor.GetSelectionInfo());
    descriptorContext.SetContentFlagsCalculator([flags = descriptor.GetContentFlags()](int){return flags;});
    ContentDescriptorPtr specificationDescriptor = ContentDescriptorBuilder(descriptorContext).CreateDescriptor(specification);
    if (specificationDescriptor.IsNull())
        return nullptr;

    ContentQueryPtr query;
    for (SelectClassInfo const& selectClassInfo : specificationDescriptor->GetSelectClasses())
        {
        ComplexContentQueryPtr classQuery = ComplexContentQuery::Create();
        ContentQueryContractPtr contract = ContentQueryContract::Create(++m_contractIdsCounter, descriptor, &selectClassInfo.GetSelectClass().GetClass(),
            *classQuery, selectClassInfo.GetRelatedInstancePaths());
        classQuery->SelectContract(*contract, "this");
        classQuery->From(selectClassInfo.GetSelectClass(), "this");

        // handle related classes
        JoinRelatedClasses(*classQuery, selectClassInfo);

        // exclude derived classes if necessary
        QueryBuilderHelpers::FilterOutExcludes(*classQuery, "this", selectClassInfo.GetSelectClass().GetDerivedExcludedClasses(), m_params.GetConnection().GetECDb().Schemas());

        // handle filtering
        InstanceFilteringParams filteringParams(m_params.GetConnection(), m_params.GetECExpressionsCache(), nullptr,
            selectClassInfo, nullptr, specification.GetInstanceFilter().c_str());
        QueryBuilderHelpers::ApplyInstanceFilter(*classQuery, filteringParams, RelatedClassPath());

#ifdef ENABLE_DEPRECATED_DISTINCT_VALUES_SUPPORT
        // handle selecting property for distinct values
        if (descriptor.GetDistinctField() != nullptr)
            classQuery = WrapQueryIntoGroupingClause(*classQuery, *contract);
#endif

        QueryBuilderHelpers::SetOrUnion<ContentQuery>(query, *classQuery);
        }

    QueryBuilderHelpers::ApplyDescriptorOverrides(query, descriptor, m_params.GetECExpressionsCache());

    return query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryPtr ContentQueryBuilder::CreateQuery(ContentDescriptor::NestedContentField const& contentField)
    {
    ContentDescriptorBuilder::Context descriptorContext(m_params.GetSchemaHelper(), m_params.GetConnections(), m_params.GetConnection(), m_params.GetRuleset(),
        ContentDisplayType::Undefined, m_params.GetCategorySupplier(), m_params.GetPropertyFormatter(), ECPresentation::UnitSystem::Undefined,
        m_params.GetLocalizationProvider(), m_params.GetLocale(), *NavNodeKeyListContainer::Create(), nullptr);
    ContentDescriptorPtr descriptor = ContentDescriptorBuilder(descriptorContext).CreateDescriptor(contentField);
    if (!descriptor.IsValid())
        return nullptr;

    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    ContentQueryContractPtr contract = ContentQueryContract::Create(++m_contractIdsCounter, *descriptor, &contentField.GetContentClass(), *query, bvector<RelatedClassPath>(), false);
    query->SelectContract(*contract, contentField.GetContentClassAlias());
    query->From(contentField.GetContentClass(), true, contentField.GetContentClassAlias());

    ContentDescriptor::RelatedContentField const* relatedContentField = contentField.AsRelatedContentField();
    if (nullptr != relatedContentField)
        {
        RelatedClassPath relationshipPath = RelatedClassPath(relatedContentField->GetPathFromSelectToContentClass()).Reverse(relatedContentField->GetSelectClassAlias(), false);
        for (RelatedClass& rc : relationshipPath)
            rc.SetIsTargetOptional(false);
        query->Join(relationshipPath);
        }

    return query;
    }
