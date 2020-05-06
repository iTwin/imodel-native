/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "HierarchiesComparer.h"
#include "CustomizationHelper.h"
#include "LoggingHelper.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr HierarchiesComparer::CreateProvider(IConnectionCR connection, HierarchyLevelInfo const& info) const
    {
    // create the nodes provider context
    NavNodesProviderContextPtr context = m_context.GetProviderContextsFactory().Create(connection, info.GetRulesetId().c_str(),
        info.GetLocale().c_str(), info.GetPhysicalParentNodeId(), nullptr, -1, m_context.GetVariables());
    if (context.IsNull())
        return nullptr;

    // create the provider
    JsonNavNodeCPtr parentNode = (nullptr != info.GetPhysicalParentNodeId()) ? m_context.GetNodesCache().GetNode(*info.GetPhysicalParentNodeId()) : nullptr;
    return m_context.GetProvidersFactory().Create(*context, parentNode.get(), ProviderCacheType::None);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void SynchronizeLists(NavNodesDataSource const& oldDs, size_t& oldIndex, NavNodesDataSource const& newDs, size_t& newIndex)
    {
    size_t newIndexReset = newIndex;
    while (oldIndex < oldDs.GetSize())
        {
        JsonNavNodeCPtr oldNode = oldDs.GetNode(oldIndex);
        newIndex = newIndexReset;
        bool found = false;
        while (newIndex < newDs.GetSize())
            {
            JsonNavNodePtr newNode = newDs.GetNode(newIndex);
            if (oldNode->GetKey()->IsSimilar(*newNode->GetKey()))
                {
                LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("[HierarchiesComparer::SynchronizeLists] Nodes similar. Indexes: %" PRIu64 " / %" PRIu64, (uint64_t)oldIndex, (uint64_t)newIndex).c_str(), NativeLogging::LOG_TRACE);
                found = true;
                break;
                }
            else
                {
                LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("[HierarchiesComparer::SynchronizeLists] Nodes NOT similar. Indexes: %" PRIu64 " / %" PRIu64, (uint64_t)oldIndex, (uint64_t)newIndex).c_str(), NativeLogging::LOG_TRACE);
                }
            ++newIndex;
            }
        if (found)
            break;
        ++oldIndex;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchiesComparer::CompareDataSources(NavNodesProviderCR lhsProvider, NavNodesProviderR rhsProvider) const
    {
    NavNodesDataSourcePtr oldDs = NavNodesDataSource::Create(lhsProvider);
    NavNodesDataSourcePtr newDs = NavNodesDataSource::Create(rhsProvider);

    LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("[HierarchiesComparer::CompareDataSources] Datasource sizes: %" PRIu64 ", %" PRIu64, (uint64_t)oldDs->GetSize(), (uint64_t)newDs->GetSize()).c_str(), NativeLogging::LOG_TRACE);

    size_t oldIndex = 0;
    size_t newIndex = 0;
    while (oldIndex < oldDs->GetSize() && newIndex < newDs->GetSize())
        {
        size_t oldIndexStart = oldIndex;
        size_t newIndexStart = newIndex;
        SynchronizeLists(*oldDs, oldIndex, *newDs, newIndex);

        // handle removed nodes
        for (size_t i = oldIndexStart; i < oldIndex; ++i)
            {
            JsonNavNodePtr node = oldDs->GetNode(i);
            m_context.Reporter().Removed(lhsProvider.GetContext().GetHierarchyLevelInfo(), *node);
            LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("[HierarchiesComparer::CompareDataSources] Removed: %" PRIu64 ", %s", (uint64_t)i, node->GetLabelDefinition().GetDisplayValue().c_str()).c_str(), NativeLogging::LOG_TRACE);
            }

        // insert added nodes
        for (size_t i = newIndexStart; i < newIndex; ++i)
            {
            JsonNavNodePtr node = newDs->GetNode(i);
            CustomizeNode(nullptr, *node, rhsProvider);
            m_context.Reporter().Added(rhsProvider.GetContext().GetHierarchyLevelInfo(), *node, i);
            LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("[HierarchiesComparer::CompareDataSources] Added: %" PRIu64 ", %s", (uint64_t)i, node->GetLabelDefinition().GetDisplayValue().c_str()).c_str(), NativeLogging::LOG_TRACE);
            }

        // now the lists are synchronized - iterate over both of them at the same time
        JsonNavNodePtr oldNode, newNode;
        while (oldIndex < oldDs->GetSize() && newIndex < newDs->GetSize()
            && (oldNode = oldDs->GetNode(oldIndex))->GetKey()->IsSimilar(*(newNode = newDs->GetNode(newIndex))->GetKey()))
            {
            LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("[HierarchiesComparer::CompareDataSources] Nodes similar. Indexes: %" PRIu64 " / %" PRIu64, (uint64_t)oldIndex, (uint64_t)newIndex).c_str(), NativeLogging::LOG_TRACE);
            CompareNodes(lhsProvider, *oldNode, rhsProvider, *newNode);
            ++oldIndex;
            ++newIndex;
            }
        }
    // handle removed nodes
    for (size_t i = oldIndex; i < oldDs->GetSize(); ++i)
        {
        JsonNavNodePtr node = oldDs->GetNode(i);
        m_context.Reporter().Removed(lhsProvider.GetContext().GetHierarchyLevelInfo(), *node);
        LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("[HierarchiesComparer::CompareDataSources] Removed: %" PRIu64 ", %s", (uint64_t)i, node->GetLabelDefinition().GetDisplayValue().c_str()).c_str(), NativeLogging::LOG_TRACE);
        }

    // insert added nodes
    for (size_t i = newIndex; i < newDs->GetSize(); ++i)
        {
        JsonNavNodePtr node = newDs->GetNode(i);
        CustomizeNode(nullptr, *node, rhsProvider);
        m_context.Reporter().Added(rhsProvider.GetContext().GetHierarchyLevelInfo(), *node, i);
        LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("[HierarchiesComparer::CompareDataSources] Added: %" PRIu64 ", %s", (uint64_t)i, node->GetLabelDefinition().GetDisplayValue().c_str()).c_str(), NativeLogging::LOG_TRACE);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchiesComparer::CompareNodes(NavNodesProviderCR lhsProvider, JsonNavNodeCR lhsNode, NavNodesProviderCR rhsProvider, JsonNavNodeR rhsNode) const
    {
    CustomizeNode(&lhsNode, rhsNode, rhsProvider);

    bvector<JsonChange> changes = NavNodesHelper::GetChanges(lhsNode, rhsNode);
    m_context.Reporter().Changed(rhsProvider.GetContext().GetHierarchyLevelInfo(), lhsNode, rhsNode, changes);
    LoggingHelper::LogMessage(Log::Navigation, Utf8PrintfString("[HierarchiesComparer::CompareNodes] Node '%s' changes: %" PRIu64, rhsNode.GetLabelDefinition().GetDisplayValue().c_str(), (uint64_t)changes.size()).c_str(), NativeLogging::LOG_TRACE);

    if (m_context.ShouldTraverseRecursively())
        {
        HierarchyLevelInfo lhsChildrenInfo(lhsProvider.GetContext().GetConnection().GetId(), lhsProvider.GetContext().GetRuleset().GetRuleSetId(),
            lhsProvider.GetContext().GetLocale(), lhsNode.GetNodeId(), 0);
        HierarchyLevelInfo rhsChildrenInfo(rhsProvider.GetContext().GetConnection().GetId(), rhsProvider.GetContext().GetRuleset().GetRuleSetId(),
            rhsProvider.GetContext().GetLocale(), rhsNode.GetNodeId(), 0);
        Compare(rhsProvider.GetContext().GetConnection(), lhsChildrenInfo, rhsChildrenInfo);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchiesComparer::CustomizeNode(JsonNavNodeCP lhsNode, JsonNavNodeR rhsNode, NavNodesProviderCR newNodeProvider) const
    {
    int updatedParts = 0;
    DataSourceRelatedVariablesUpdater updater(newNodeProvider.GetContext(), &rhsNode);

    // if the old node was customized, we have to customize the new one as well;
    // otherwise the comparison is incorrect
    bool shouldCustomize = (nullptr == lhsNode || NavNodeExtendedData(*lhsNode).IsCustomized());
    if (shouldCustomize && !NavNodeExtendedData(rhsNode).IsCustomized())
        {
        CustomizationHelper::Customize(newNodeProvider.GetContext(), rhsNode, NavNodesHelper::IsCustomNode(rhsNode));
        updatedParts |= IHierarchyCache::UPDATE_NodeItself | IHierarchyCache::UPDATE_NodeKey;
        }

    // the same with children
    bool shouldDetermineChildren = (nullptr == lhsNode || lhsNode->DeterminedChildren());
    if (shouldDetermineChildren && !rhsNode.DeterminedChildren())
        {
        newNodeProvider.DetermineChildren(rhsNode);
        updatedParts |= IHierarchyCache::UPDATE_NodeItself;
        }

    // if old node was expanded, we have to expand new node too
    bool isOldNodeExpanded = (nullptr != lhsNode && lhsNode->IsExpanded());
    if (isOldNodeExpanded && !rhsNode.IsExpanded())
        {
        rhsNode.SetIsExpanded(true);
        updatedParts |= IHierarchyCache::UPDATE_NodeItself;
        }

    // notify provider that we updated the node
    if (0 != updatedParts)
        newNodeProvider.NotifyNodeChanged(rhsNode, updatedParts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchiesComparer::Compare(IConnectionCacheCR connections, HierarchyLevelInfo const& lhs, HierarchyLevelInfo const& rhs) const
    {
    if (!lhs.GetConnectionId().Equals(rhs.GetConnectionId()))
        {
        BeAssert(false && "Can only compare hierarchies of the same connection");
        return;
        }

    IConnectionPtr connection = connections.GetConnection(lhs.GetConnectionId().c_str());
    if (connection.IsNull())
        return;
    Compare(*connection, lhs, rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchiesComparer::Compare(IConnectionCR connection, HierarchyLevelInfo const& lhs, HierarchyLevelInfo const& rhs) const
    {
    Savepoint txn(connection.GetDb(), "Compare");
    BeAssert(txn.IsActive());
    BeAssert(lhs.GetConnectionId().Equals(connection.GetId()));
    BeAssert(rhs.GetConnectionId().Equals(connection.GetId()));

    NavNodesProviderPtr lhsProvider = m_context.GetNodesCache().GetCombinedHierarchyLevel(lhs, m_context.GetVariables(), false);
    if (lhsProvider.IsNull())
        {
        BeAssert(false && "Could not find a data source to compare with");
        return;
        }
    m_context.Reporter().FoundLhsProvider(*lhsProvider);

    NavNodesProviderPtr rhsProvider = CreateProvider(connection, rhs);
    if (rhsProvider.IsValid() && m_context.Reporter().StartCompare(*lhsProvider, *rhsProvider))
        {
        DisabledFullNodesLoadContext doNotCustomizeLhs(*lhsProvider);
        DisabledFullNodesLoadContext doNotCustomizeRhs(*rhsProvider);
        CompareDataSources(*lhsProvider, *rhsProvider);
        }

    m_context.Reporter().EndCompare(*lhsProvider, rhsProvider.get());
    }
