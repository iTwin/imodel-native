/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ContentQueryBuilder.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
ParsedSelectionInfo::ParsedSelectionInfo(NavNodeKeyListCR nodeKeys, INavNodeLocater const& nodesLocater, IConnectionCR connection, ECSchemaHelper const& helper)
    {
    Parse(nodeKeys, nodesLocater, connection, helper);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ParsedSelectionInfo::GetNodeClasses(NavNodeKeyCR nodeKey, INavNodeLocater const& nodesLocater, IConnectionCR connection, ECSchemaHelper const& helper)
    {
    if (nullptr != nodeKey.AsDisplayLabelGroupingNodeKey() && !nodeKey.GetType().Equals(NAVNODE_TYPE_DisplayLabelGroupingNode))
        {
        // Custom nodes
        // This type of nodes don't supply any content for class-based content specifications
        return;
        }

    if (nullptr != nodeKey.AsECInstanceNodeKey())
        {
        // ECInstance node
        ECInstanceNodeKey const& instanceNodeKey = *nodeKey.AsECInstanceNodeKey();
        ECClassCP ecClass = helper.GetECClass(instanceNodeKey.GetECClassId());
        if (nullptr == ecClass || !ecClass->IsEntityClass())
            {
            BeAssert(false);
            return;
            }
        if (m_classSelection.end() == m_classSelection.find(ecClass))
            m_orderedClasses.push_back(ecClass);
        if (instanceNodeKey.GetInstanceId().IsValid())
            m_classSelection[ecClass].push_back(instanceNodeKey.GetInstanceId());
        return;
        }
        
    // Some grouping node
    NavNodeCPtr node = nodesLocater.LocateNode(connection, nodeKey);
    if (node.IsNull())
        {
        BeAssert(false);
        return;
        }

    NavNodeExtendedData extendedData(*node);
    bvector<ECInstanceKey> groupedInstanceKeys = extendedData.GetGroupedInstanceKeys();
    for (ECInstanceKeyCR key : groupedInstanceKeys)
        GetNodeClasses(*ECInstanceNodeKey::Create(key), nodesLocater, connection, helper);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ParsedSelectionInfo::Parse(NavNodeKeyListCR keys, INavNodeLocater const& nodesLocater, IConnectionCR connection, ECSchemaHelper const& helper)
    {
    for (NavNodeKeyCPtr const& key : keys)
        GetNodeClasses(*key, nodesLocater, connection, helper);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECInstanceId> const& ParsedSelectionInfo::_GetInstanceIds(ECClassCR selectClass) const
    {
    auto iter = m_classSelection.find(&selectClass);
    if (m_classSelection.end() != iter)
        return iter->second;
        
    static bvector<ECInstanceId> s_empty;
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
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryPtr ContentQueryBuilder::CreateQuery(SelectedNodeInstancesSpecificationCR specification, ContentDescriptorCR descriptor, IParsedSelectionInfo const& selection)
    {
    ContentDescriptorBuilder::Context descriptorContext(m_params.GetSchemaHelper(), m_params.GetConnections(), m_params.GetConnection(), m_params.GetRuleset(), 
        descriptor.GetPreferredDisplayType().c_str(), m_params.GetCategorySupplier(), m_params.GetPropertyFormatter(), 
        m_params.GetLocalizationProvider());
    ContentDescriptorPtr specificationDescriptor = ContentDescriptorBuilder(descriptorContext).CreateDescriptor(specification, selection);
    if (specificationDescriptor.IsNull())
        return nullptr;
    
    ContentQueryPtr query;
    for (SelectClassInfo const& selectClassInfo : specificationDescriptor->GetSelectClasses())
        {
        ComplexContentQueryPtr classQuery = ComplexContentQuery::Create();
        ContentQueryContractPtr contract = ContentQueryContract::Create(++m_contractIdsCounter, descriptor, &selectClassInfo.GetSelectClass(), *classQuery);
        classQuery->SelectContract(*contract, "this");
        classQuery->From(selectClassInfo.GetSelectClass(), selectClassInfo.IsSelectPolymorphic(), "this");

        // handle related properties
        for (RelatedClassPath const& path : selectClassInfo.GetRelatedPropertyPaths())
            classQuery->Join(path, true);
        
        // handle filtering 
        InstanceFilteringParams filteringParams(m_params.GetConnection(), m_params.GetECExpressionsCache(), &selection, selectClassInfo, nullptr, nullptr);
        QueryBuilderHelpers::ApplyInstanceFilter(*classQuery, filteringParams);
        
        // handle selecting property for distinct values 
        if (descriptor.OnlyDistinctValues() && descriptor.GetVisibleFields().size() == 1 && nullptr != descriptor.GetVisibleFields()[0]->AsPropertiesField())
            classQuery->GroupByContract(*contract);

        QueryBuilderHelpers::SetOrUnion<ContentQuery>(query, *classQuery);
        }

    QueryBuilderHelpers::ApplyDescriptorOverrides(query, descriptor, m_params.GetECExpressionsCache());

    return query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryPtr ContentQueryBuilder::CreateQuery(ContentRelatedInstancesSpecificationCR specification, ContentDescriptorCR descriptor, IParsedSelectionInfo const& selection)
    {
    ContentDescriptorBuilder::Context descriptorContext(m_params.GetSchemaHelper(), m_params.GetConnections(), m_params.GetConnection(), m_params.GetRuleset(), 
        descriptor.GetPreferredDisplayType().c_str(), m_params.GetCategorySupplier(), m_params.GetPropertyFormatter(), 
        m_params.GetLocalizationProvider());
    ContentDescriptorPtr specificationDescriptor = ContentDescriptorBuilder(descriptorContext).CreateDescriptor(specification, selection);
    if (specificationDescriptor.IsNull())
        return nullptr;
    
    InstanceFilteringParams::RecursiveQueryInfo const* recursiveInfo = nullptr;
    if (specification.IsRecursive())
        recursiveInfo = new InstanceFilteringParams::RecursiveQueryInfo(GetPathsToPrimary(specificationDescriptor->GetSelectClasses()));

    ContentQueryPtr query;
    for (SelectClassInfo const& selectClassInfo : specificationDescriptor->GetSelectClasses())
        {
        ComplexContentQueryPtr classQuery = ComplexContentQuery::Create();
        ContentQueryContractPtr contract = ContentQueryContract::Create(++m_contractIdsCounter, descriptor, &selectClassInfo.GetSelectClass(), *classQuery);
        classQuery->SelectContract(*contract, "this");
        classQuery->From(selectClassInfo.GetSelectClass(), selectClassInfo.IsSelectPolymorphic(), "this");

        // handle related properties
        for (RelatedClassPath const& path : selectClassInfo.GetRelatedPropertyPaths())
            classQuery->Join(path, true);
        
        // handle filtering 
        InstanceFilteringParams filteringParams(m_params.GetConnection(), m_params.GetECExpressionsCache(), &selection, 
            selectClassInfo, recursiveInfo, specification.GetInstanceFilter().c_str());
        QueryBuilderHelpers::ApplyInstanceFilter(*classQuery, filteringParams);
            
        // handle selecting property for distinct values 
        if (descriptor.OnlyDistinctValues() && descriptor.GetVisibleFields().size() == 1 && nullptr != descriptor.GetVisibleFields()[0]->AsPropertiesField())
            classQuery->GroupByContract(*contract);

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
        descriptor.GetPreferredDisplayType().c_str(), m_params.GetCategorySupplier(), m_params.GetPropertyFormatter(), 
        m_params.GetLocalizationProvider());
    ContentDescriptorPtr specificationDescriptor = ContentDescriptorBuilder(descriptorContext).CreateDescriptor(specification);
    if (specificationDescriptor.IsNull())
        return nullptr;
        
    ContentQueryPtr query;
    for (SelectClassInfo const& selectClassInfo : specificationDescriptor->GetSelectClasses())
        {
        ComplexContentQueryPtr classQuery = ComplexContentQuery::Create();
        ContentQueryContractPtr contract = ContentQueryContract::Create(++m_contractIdsCounter, descriptor, &selectClassInfo.GetSelectClass(), *classQuery);
        classQuery->SelectContract(*contract, "this");
        classQuery->From(selectClassInfo.GetSelectClass(), selectClassInfo.IsSelectPolymorphic(), "this");

        // handle related properties
        for (RelatedClassPath const& path : selectClassInfo.GetRelatedPropertyPaths())
            classQuery->Join(path, true);
        
        // handle filtering 
        InstanceFilteringParams filteringParams(m_params.GetConnection(), m_params.GetECExpressionsCache(), nullptr, 
            selectClassInfo, nullptr, specification.GetInstanceFilter().c_str());
        QueryBuilderHelpers::ApplyInstanceFilter(*classQuery, filteringParams);

        // handle selecting property for distinct values 
        if (descriptor.OnlyDistinctValues() && descriptor.GetVisibleFields().size() == 1 && nullptr != descriptor.GetVisibleFields()[0]->AsPropertiesField())
            classQuery->GroupByContract(*contract);

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
        ContentDisplayType::Undefined, m_params.GetCategorySupplier(), m_params.GetPropertyFormatter(), 
        m_params.GetLocalizationProvider());
    ContentDescriptorPtr descriptor = ContentDescriptorBuilder(descriptorContext).CreateDescriptor(contentField);
    if (!descriptor.IsValid())
        return nullptr;
        
    ComplexContentQueryPtr query = ComplexContentQuery::Create();
    ContentQueryContractPtr contract = ContentQueryContract::Create(++m_contractIdsCounter, *descriptor, &contentField.GetContentClass(), *query, false);
    query->SelectContract(*contract, contentField.GetContentClassAlias().c_str());
    query->From(contentField.GetContentClass(), true, contentField.GetContentClassAlias().c_str());

    RelatedClassPath relationshipPath;
    for (auto iter = contentField.GetRelationshipPath().rbegin(); iter != contentField.GetRelationshipPath().rend(); ++iter)
        relationshipPath.push_back(*iter);
    query->Join(relationshipPath, false);
    
    return query;
    }
