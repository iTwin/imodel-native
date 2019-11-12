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
static bvector<RelatedClassPath> GetPathsToPrimary(bvector<SelectClassInfo> const& selectInfos)
    {
    bvector<RelatedClassPath> paths;
    for (SelectClassInfo const& info : selectInfos)
        paths.push_back(info.GetPathToPrimaryClass());
    return paths;
    }

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
    for (RelatedClass const& relatedInstanceClass : selectInfo.GetRelatedInstanceClasses())
        query.Join(relatedInstanceClass, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryPtr ContentQueryBuilder::CreateQuery(SelectedNodeInstancesSpecificationCR specification, ContentDescriptorCR descriptor, IParsedInput const& input)
    {
    ContentDescriptorBuilder::Context descriptorContext(m_params.GetSchemaHelper(), m_params.GetConnections(), m_params.GetConnection(), m_params.GetRuleset(), 
        descriptor.GetPreferredDisplayType().c_str(), descriptor.GetContentFlags(), m_params.GetCategorySupplier(), m_params.GetPropertyFormatter(),
        m_params.GetLocalizationProvider(), m_params.GetLocale(), descriptor.GetInputNodeKeys(), descriptor.GetSelectionInfo());
    ContentDescriptorPtr specificationDescriptor = ContentDescriptorBuilder(descriptorContext).CreateDescriptor(specification, input);
    if (specificationDescriptor.IsNull())
        return nullptr;
    
    ContentQueryPtr query;
    for (SelectClassInfo const& selectClassInfo : specificationDescriptor->GetSelectClasses())
        {
        ComplexContentQueryPtr classQuery = ComplexContentQuery::Create();
        ContentQueryContractPtr contract = ContentQueryContract::Create(++m_contractIdsCounter, descriptor, &selectClassInfo.GetSelectClass(), *classQuery, selectClassInfo.GetRelatedInstanceClasses());
        classQuery->SelectContract(*contract, "this");
        classQuery->From(selectClassInfo.GetSelectClass(), selectClassInfo.IsSelectPolymorphic(), "this");

        // handle related classes
        JoinRelatedClasses(*classQuery, selectClassInfo);
        
        // handle filtering 
        InstanceFilteringParams filteringParams(m_params.GetConnection(), m_params.GetECExpressionsCache(), &input, selectClassInfo, nullptr, nullptr);
        QueryBuilderHelpers::ApplyInstanceFilter(*classQuery, filteringParams, RelatedClassPath());
        
        // handle selecting property for distinct values 
        if (descriptor.GetDistinctField() != nullptr)
            classQuery = WrapQueryIntoGroupingClause(*classQuery, *contract);

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
    ECClassCP m_source;
    ECClassCP m_target;
    bool m_isForward;
    HandledRecursiveClassesKey() : m_source(nullptr), m_target(nullptr), m_isForward(false) {}
    HandledRecursiveClassesKey(SelectClassInfo const& info)
        {
        m_source = &info.GetSelectClass();
        m_target = info.GetPathToPrimaryClass().back().GetTargetClass();
        m_isForward = info.GetPathToPrimaryClass().back().IsForwardRelationship();
        }
    bool operator<(HandledRecursiveClassesKey const& other) const
        {
        return m_source < other.m_source
            || m_source == other.m_source && m_target < other.m_target
            || m_source == other.m_source && m_target == other.m_target && m_isForward < other.m_isForward;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryPtr ContentQueryBuilder::CreateQuery(ContentRelatedInstancesSpecificationCR specification, ContentDescriptorCR descriptor, IParsedInput const& input)
    {
    ContentDescriptorBuilder::Context descriptorContext(m_params.GetSchemaHelper(), m_params.GetConnections(), m_params.GetConnection(), m_params.GetRuleset(), 
        descriptor.GetPreferredDisplayType().c_str(), descriptor.GetContentFlags(), m_params.GetCategorySupplier(), m_params.GetPropertyFormatter(), 
        m_params.GetLocalizationProvider(), m_params.GetLocale(), descriptor.GetInputNodeKeys(), descriptor.GetSelectionInfo());
    ContentDescriptorPtr specificationDescriptor = ContentDescriptorBuilder(descriptorContext).CreateDescriptor(specification, input);
    if (specificationDescriptor.IsNull())
        return nullptr;
    
    InstanceFilteringParams::RecursiveQueryInfo const* recursiveInfo = nullptr;
    if (specification.IsRecursive())
        recursiveInfo = new InstanceFilteringParams::RecursiveQueryInfo(GetPathsToPrimary(specificationDescriptor->GetSelectClasses()));

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
        ContentQueryContractPtr contract = ContentQueryContract::Create(++m_contractIdsCounter, descriptor, &selectClassInfo.GetSelectClass(), *classQuery, selectClassInfo.GetRelatedInstanceClasses());
        classQuery->SelectContract(*contract, "this");
        classQuery->From(selectClassInfo.GetSelectClass(), selectClassInfo.IsSelectPolymorphic(), "this");

        // handle related classes
        JoinRelatedClasses(*classQuery, selectClassInfo);
        
        // handle filtering 
        InstanceFilteringParams filteringParams(m_params.GetConnection(), m_params.GetECExpressionsCache(), &input, 
            selectClassInfo, recursiveInfo, specification.GetInstanceFilter().c_str());
        QueryBuilderHelpers::ApplyInstanceFilter(*classQuery, filteringParams, RelatedClassPath());
            
        // handle selecting property for distinct values 
        if (descriptor.GetDistinctField() != nullptr)
            classQuery = WrapQueryIntoGroupingClause(*classQuery, *contract);

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
        descriptor.GetPreferredDisplayType().c_str(), descriptor.GetContentFlags(), m_params.GetCategorySupplier(), m_params.GetPropertyFormatter(),
        m_params.GetLocalizationProvider(), m_params.GetLocale(), descriptor.GetInputNodeKeys(), descriptor.GetSelectionInfo());
    ContentDescriptorPtr specificationDescriptor = ContentDescriptorBuilder(descriptorContext).CreateDescriptor(specification);
    if (specificationDescriptor.IsNull())
        return nullptr;
        
    ContentQueryPtr query;
    for (SelectClassInfo const& selectClassInfo : specificationDescriptor->GetSelectClasses())
        {
        ComplexContentQueryPtr classQuery = ComplexContentQuery::Create();
        ContentQueryContractPtr contract = ContentQueryContract::Create(++m_contractIdsCounter, descriptor, &selectClassInfo.GetSelectClass(), *classQuery, selectClassInfo.GetRelatedInstanceClasses());
        classQuery->SelectContract(*contract, "this");
        classQuery->From(selectClassInfo.GetSelectClass(), selectClassInfo.IsSelectPolymorphic(), "this");

        // handle related classes
        JoinRelatedClasses(*classQuery, selectClassInfo);
        
        // handle filtering 
        InstanceFilteringParams filteringParams(m_params.GetConnection(), m_params.GetECExpressionsCache(), nullptr, 
            selectClassInfo, nullptr, specification.GetInstanceFilter().c_str());
        QueryBuilderHelpers::ApplyInstanceFilter(*classQuery, filteringParams, RelatedClassPath());

        // handle selecting property for distinct values 
        if (descriptor.GetDistinctField() != nullptr)
            classQuery = WrapQueryIntoGroupingClause(*classQuery, *contract);

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
        ContentDisplayType::Undefined, 0, m_params.GetCategorySupplier(), m_params.GetPropertyFormatter(), 
        m_params.GetLocalizationProvider(), m_params.GetLocale(), *NavNodeKeyListContainer::Create(), nullptr);
    ContentDescriptorPtr descriptor = ContentDescriptorBuilder(descriptorContext).CreateDescriptor(contentField);
    if (!descriptor.IsValid())
        return nullptr;
        
    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    ContentQueryContractPtr contract = ContentQueryContract::Create(++m_contractIdsCounter, *descriptor, &contentField.GetContentClass(), *query, bvector<RelatedClass>(), false);
    query->SelectContract(*contract, contentField.GetContentClassAlias().c_str());
    query->From(contentField.GetContentClass(), true, contentField.GetContentClassAlias().c_str());

    RelatedClassPath relationshipPath;
    for (auto iter = contentField.GetRelationshipPath().rbegin(); iter != contentField.GetRelationshipPath().rend(); ++iter)
        relationshipPath.push_back(*iter);
    query->Join(relationshipPath, false);
    
    return query;
    }
