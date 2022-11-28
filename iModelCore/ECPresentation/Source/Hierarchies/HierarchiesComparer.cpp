/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "HierarchiesComparer.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static IteratorWrapper<NavNodePtr> GetFrontIterator(NavNodesProviderCR dataSource, IteratorWrapper<NavNodePtr> const& end, Utf8StringCR hash, uint64_t& position)
    {
    if (hash.empty())
        return end;

    for (auto iter = dataSource.begin(); iter != end; ++iter)
        {
        if ((*iter)->GetKey()->GetHash() == hash)
            return iter;

        ++position;
        }
    return end;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr HierarchiesComparer::CreateProvider(NavNodesProviderContextR context) const
    {
    // create the provider
    NavNodesProviderPtr provider = m_params.GetProvidersFactory().Create(context);
    return provider->PostProcess(m_params.GetProvidersFactory().GetPostProcessors());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr HierarchiesComparer::CreateProvider(IConnectionCR connection, CombinedHierarchyLevelIdentifier const& id, RulesetVariables const& variables) const
    {
    // get parent node
    NavNodeCPtr physicalParent = id.GetPhysicalParentNodeId().IsValid() ? m_params.GetNodesCache()->GetNode(id.GetPhysicalParentNodeId()) : nullptr;

    // create the nodes provider context
    NavNodesProviderContextPtr context = m_params.GetProviderContextsFactory().Create(connection, id.GetRulesetId().c_str(),
        physicalParent.get(), m_params.GetNodesCache(), nullptr, variables);
    if (context.IsNull())
        return nullptr;

    // create the provider
    return CreateProvider(*context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderPtr HierarchiesComparer::GetCachedOrCreateProvider(IConnectionCR connection, CombinedHierarchyLevelIdentifier const& id, RulesetVariables const& variables, bool isNodesLoadingEnabled, IHierarchyChangesReporter* reporter) const
    {
    // get parent node
    NavNodeCPtr physicalParent = id.GetPhysicalParentNodeId().IsValid() ? m_params.GetNodesCache()->GetNode(id.GetPhysicalParentNodeId()) : nullptr;

    // create the nodes provider context
    NavNodesProviderContextPtr context = m_params.GetProviderContextsFactory().Create(connection, id.GetRulesetId().c_str(),
        physicalParent.get(), m_params.GetNodesCache(), nullptr, variables);
    if (context.IsNull())
        return nullptr;

    if (reporter)
        reporter->OnBeforeCreateLhsProvider(*context);

    // create the provider
    NavNodesProviderPtr provider = m_params.GetNodesCache()->GetCombinedHierarchyLevel(*context, context->GetHierarchyLevelIdentifier(), isNodesLoadingEnabled);
    if (provider.IsValid())
        return provider->PostProcess(m_params.GetProvidersFactory().GetPostProcessors());

    return isNodesLoadingEnabled ? CreateProvider(*context) : nullptr;
    }

#define CHECK_FOR_INTERRUPT(lhsHash, rhsHash) \
    { \
    ThrowIfCancelled(params.GetCancellationToken()); \
    if (!params.Reporter().ShouldContinue()) \
        return HierarchiesComparer::CompareResult(lhsHash, rhsHash); \
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
HierarchiesComparer::CompareResult HierarchiesComparer::CompareDataSources(CompareWithConnectionParams const& params, NavNodesProviderCR lhsProvider, NavNodesProviderR rhsProvider) const
    {
    auto scope = Diagnostics::Scope::Create("Compare data sources");
    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesUpdate, LOG_DEBUG, Utf8PrintfString("Sizes LHS: %" PRIu64 ", RHS: %" PRIu64, (uint64_t)lhsProvider.GetNodesCount(), (uint64_t)rhsProvider.GetNodesCount()));

    bool allowGettingPastLhsNodes = params.ShouldLoadLhsNodes()
        || m_params.GetNodesCache()->IsCombinedHierarchyLevelInitialized(lhsProvider.GetContext().GetHierarchyLevelIdentifier(), params.GetLhsVariables(), lhsProvider.GetContext().GetInstanceFilter());

    uint64_t lhsPositionIndex = 0;
    auto lhsEnd = lhsProvider.end();
    auto lhsIter = nullptr == m_startLocationLookup ? lhsProvider.begin() : GetFrontIterator(lhsProvider, lhsEnd, m_startLocationLookup->GetLhsHash(), lhsPositionIndex);

    uint64_t rhsPositionIndex = 0;
    auto rhsEnd = allowGettingPastLhsNodes ? rhsProvider.end() : rhsProvider.begin() + lhsProvider.GetNodesCount();
    auto rhsIter = nullptr == m_startLocationLookup ? rhsProvider.begin() : GetFrontIterator(rhsProvider, rhsEnd, m_startLocationLookup->GetRhsHash(), rhsPositionIndex);

    // start position is reached. Can continue comparison
    if (nullptr != m_startLocationLookup && !m_startLocationLookup->HasDeeperLevels())
        m_startLocationLookup = nullptr;

    while (lhsIter != lhsEnd && rhsIter != rhsEnd)
        {
        NavNodePtr lhsNode = *lhsIter;
        NavNodePtr rhsNode = *rhsIter;

        // lhs node and rhs node are similar compare them
        if (lhsNode->GetKey()->IsSimilar(*rhsNode->GetKey()))
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesUpdate, LOG_TRACE, Utf8PrintfString("Nodes at position %" PRIu64 " similar", rhsPositionIndex));
            HierarchiesComparer::CompareResult result = CompareNodes(params, lhsProvider, *lhsNode, rhsProvider, *rhsNode);
            // stop comparing this level if comparison was interrupted in deeper levels
            if (result.GetStatus() != HierarchyCompareStatus::Complete)
                {
                Utf8String lhsHashPath = !result.GetContinuationToken().first.empty() ? result.GetContinuationToken().first : NavNodesHelper::NodeKeyHashPathToString(*lhsNode->GetKey());
                Utf8String rhsHashPath = !result.GetContinuationToken().second.empty() ? result.GetContinuationToken().second : NavNodesHelper::NodeKeyHashPathToString(*rhsNode->GetKey());
                return HierarchiesComparer::CompareResult(lhsHashPath, rhsHashPath);
                }

            ++lhsIter;
            ++lhsPositionIndex;
            ++rhsIter;
            ++rhsPositionIndex;
            continue;
            }

        // lhs and rhs nodes are not similar. Look in rhs hierarchy for node matching lhs node
        auto rhsSimilarNodeIter = rhsIter;
        for (; rhsSimilarNodeIter != rhsEnd; ++rhsSimilarNodeIter)
            {
            if (lhsNode->GetKey()->IsSimilar(*(*rhsSimilarNodeIter)->GetKey()))
                break;
            }

        // matching node in rhs hierarchy was not found. Report that lhs node was removed
        if (rhsSimilarNodeIter == rhsEnd)
            {
            CHECK_FOR_INTERRUPT(NavNodesHelper::NodeKeyHashPathToString(*lhsNode->GetKey()), NavNodesHelper::NodeKeyHashPathToString(*rhsNode->GetKey()));
            NavNodeCPtr parent = lhsProvider.GetContext().GetNodesCache().GetPhysicalParentNode(
                lhsNode->GetNodeId(),
                lhsProvider.GetContext().GetRulesetVariables(),
                lhsProvider.GetContext().GetInstanceFilter());
            params.Reporter().Removed(lhsProvider.GetContext().GetHierarchyLevelIdentifier(), *lhsNode, parent, lhsPositionIndex);
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesUpdate, LOG_TRACE, Utf8PrintfString("Node '%s' removed at index %" PRIu64, lhsNode->GetLabelDefinition().GetDisplayValue().c_str(), rhsPositionIndex));
            ++lhsIter;
            ++lhsPositionIndex;
            continue;
            }

        // matching node was found in rhs hierarchy. Report that there are added nodes in rhs hierarchy
        for (; rhsIter != rhsSimilarNodeIter; ++rhsIter, ++rhsPositionIndex)
            {
            NavNodePtr addedNode = *rhsIter;
            CHECK_FOR_INTERRUPT(NavNodesHelper::NodeKeyHashPathToString(*lhsNode->GetKey()), NavNodesHelper::NodeKeyHashPathToString(*addedNode->GetKey()));
            CustomizeNode(nullptr, *addedNode, rhsProvider);
            params.Reporter().Added(rhsProvider.GetContext().GetHierarchyLevelIdentifier(), *addedNode, rhsProvider.GetContext().GetPhysicalParentNode(), rhsPositionIndex);
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesUpdate, LOG_TRACE, Utf8PrintfString("Node '%s' added at index %" PRIu64, addedNode->GetLabelDefinition().GetDisplayValue().c_str(), rhsPositionIndex));
            }
        }

    Utf8String rhsNextNodeHash = rhsIter != rhsEnd ? NavNodesHelper::NodeKeyHashPathToString(*(*rhsIter)->GetKey()) : "";
    // handle removed nodes
    for (; lhsIter != lhsEnd; ++lhsIter, ++lhsPositionIndex)
        {
        NavNodeCPtr node = *lhsIter;
        NavNodeCPtr parent = lhsProvider.GetContext().GetNodesCache().GetPhysicalParentNode(
            node->GetNodeId(),
            lhsProvider.GetContext().GetRulesetVariables(),
            lhsProvider.GetContext().GetInstanceFilter());
        CHECK_FOR_INTERRUPT(NavNodesHelper::NodeKeyHashPathToString(*node->GetKey()), rhsNextNodeHash);
        params.Reporter().Removed(lhsProvider.GetContext().GetHierarchyLevelIdentifier(), *node, parent, lhsPositionIndex);
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesUpdate, LOG_TRACE, Utf8PrintfString("Node '%s' removed at index %" PRIu64, node->GetLabelDefinition().GetDisplayValue().c_str(), rhsPositionIndex));
        }

    // insert added nodes
    for (; rhsIter != rhsEnd; ++rhsIter, ++rhsPositionIndex)
        {
        NavNodePtr node = *rhsIter;
        CHECK_FOR_INTERRUPT("", NavNodesHelper::NodeKeyHashPathToString(*node->GetKey()));
        CustomizeNode(nullptr, *node, rhsProvider);
        params.Reporter().Added(rhsProvider.GetContext().GetHierarchyLevelIdentifier(), *node, rhsProvider.GetContext().GetPhysicalParentNode(), rhsPositionIndex);
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesUpdate, LOG_TRACE, Utf8PrintfString("Node '%s' added at index %" PRIu64, node->GetLabelDefinition().GetDisplayValue().c_str(), rhsPositionIndex));
        }

    return HierarchiesComparer::CompareResult();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
HierarchiesComparer::CompareResult HierarchiesComparer::CompareNodes(CompareWithConnectionParams const& params, NavNodesProviderCR lhsProvider, NavNodeCR lhsNode, NavNodesProviderCR rhsProvider,NavNodeR rhsNode) const
    {
    auto scope = Diagnostics::Scope::Create("Compare nodes");

    if (nullptr == m_startLocationLookup)
        {
        CHECK_FOR_INTERRUPT(NavNodesHelper::NodeKeyHashPathToString(*lhsNode.GetKey()), NavNodesHelper::NodeKeyHashPathToString(*rhsNode.GetKey()));
        CustomizeNode(&lhsNode, rhsNode, rhsProvider);
        NodeChanges nodeChanges(lhsNode, rhsNode);
        params.Reporter().Changed(rhsProvider.GetContext().GetHierarchyLevelIdentifier(), nodeChanges);
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::HierarchiesUpdate, LOG_TRACE, Utf8PrintfString("Node '%s' changes count: %" PRIu64, rhsNode.GetLabelDefinition().GetDisplayValue().c_str(), (uint64_t)nodeChanges.GetNumChangedFields()));
        }

    if (params.ShouldTraverseRecursively() && ContainerHelpers::Contains(params.GetExpandedNodeKeys(), [&lhsNode](NavNodeKeyCPtr const& key){return *key == *lhsNode.GetKey();}))
        {
        CombinedHierarchyLevelIdentifier lhsChildrenInfo(lhsProvider.GetContext().GetConnection().GetId(), lhsProvider.GetContext().GetRuleset().GetRuleSetId(), lhsNode.GetNodeId());
        CombinedHierarchyLevelIdentifier rhsChildrenInfo(rhsProvider.GetContext().GetConnection().GetId(), rhsProvider.GetContext().GetRuleset().GetRuleSetId(), rhsNode.GetNodeId());

        if (nullptr != m_startLocationLookup)
            m_startLocationLookup->IncrementDepth();

        return DoCompare(CompareWithConnectionParams(params.Reporter(), params.GetConnection(), lhsChildrenInfo, params.GetLhsVariables(),
            rhsChildrenInfo, params.GetRhsVariables(), params.GetExpandedNodeKeys(), params.GetContinuationToken(), params.ShouldTraverseRecursively(),
            params.ShouldLoadLhsNodes(), params.GetCancellationToken()));
        }

    return HierarchiesComparer::CompareResult();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void HierarchiesComparer::CustomizeNode(NavNodeCP lhsNode, NavNodeR rhsNode, NavNodesProviderCR rhsNodeProvider) const
    {
    NodesFinalizer rhsFinalizer(rhsNodeProvider.GetContextR());

    // if the old node was customized, we have to customize the new one as well;
    // otherwise the comparison is incorrect
    if ((nullptr == lhsNode || NavNodeExtendedData(*lhsNode).IsCustomized()) && !NavNodeExtendedData(rhsNode).IsCustomized())
        rhsFinalizer.Customize(rhsNode);

    // the same with children
    if ((nullptr == lhsNode || lhsNode->DeterminedChildren()) && !rhsNode.DeterminedChildren())
        rhsFinalizer.DetermineChildren(rhsNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
HierarchiesComparer::CompareResult HierarchiesComparer::DoCompare(HierarchiesComparer::CompareWithConnectionParams const& params) const
    {
    Savepoint txn(params.GetConnection().GetDb(), "Compare");
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, txn.IsActive(), "Failed to start a transaction");

    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, params.GetLhsHierarchyIdentifier().GetConnectionId().Equals(params.GetConnection().GetId()),
        Utf8PrintfString("Given connection doesn't match LHS hierarchy: '%s' vs '%s'", params.GetConnection().GetId().c_str(), params.GetLhsHierarchyIdentifier().GetConnectionId().c_str()));
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, params.GetRhsHierarchyIdentifier().GetConnectionId().Equals(params.GetConnection().GetId()),
        Utf8PrintfString("Given connection doesn't match RHS hierarchy: '%s' vs '%s'", params.GetConnection().GetId().c_str(), params.GetRhsHierarchyIdentifier().GetConnectionId().c_str()));

    NavNodesProviderPtr lhsProvider = GetCachedOrCreateProvider(params.GetConnection(), params.GetLhsHierarchyIdentifier(), params.GetLhsVariables(), params.ShouldLoadLhsNodes(), &params.Reporter());
    if (lhsProvider.IsNull())
        {
        DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Default, !params.ShouldLoadLhsNodes(), "Should always get a LHS provider if loading nodes is allowed");
        params.Reporter().OnLhsProviderNotFound(params.GetLhsHierarchyIdentifier());
        return HierarchiesComparer::CompareResult(); // nothing to compare with
        }
    if (!params.Reporter().OnAfterCreatedLhsProvider(*lhsProvider, params.GetLhsHierarchyIdentifier()))
        {
        params.Reporter().EndCompare(*lhsProvider, nullptr);
        return HierarchiesComparer::CompareResult();
        }

    if (params.ShouldLoadLhsNodes())
        {
        // iterate over all nodes in lhs provider to ensure it's initialized before
        // we create the rhs provider
        for (NavNodePtr node : *lhsProvider)
            ;
        }

    NavNodesProviderPtr rhsProvider = CreateProvider(params.GetConnection(), params.GetRhsHierarchyIdentifier(), params.GetRhsVariables());
    if (rhsProvider.IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "Failed to create RHS provider")

    HierarchiesComparer::CompareResult result;
    if (params.Reporter().StartCompare(*lhsProvider, *rhsProvider))
        {
        DisabledFullNodesLoadContext doNotCustomizeRhs(*rhsProvider);
        result = CompareDataSources(params, *lhsProvider, *rhsProvider);
        }

    params.Reporter().EndCompare(*lhsProvider, rhsProvider.get());
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
HierarchiesComparer::CompareResult HierarchiesComparer::Compare(HierarchiesComparer::CompareParams const& params) const
    {
    if (!params.GetLhsHierarchyIdentifier().GetConnectionId().Equals(params.GetRhsHierarchyIdentifier().GetConnectionId()))
        {
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Connection ID of LHS and RHS hierarchies doesn't match: "
            "'%s' vs '%s'", params.GetLhsHierarchyIdentifier().GetConnectionId().c_str(), params.GetRhsHierarchyIdentifier().GetConnectionId().c_str()));
        }

    IConnectionCPtr connection = m_params.GetConnections().GetConnection(params.GetLhsHierarchyIdentifier().GetConnectionId().c_str());
    if (connection.IsNull())
        {
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Did not find an active connection with ID: '%s'",
            params.GetLhsHierarchyIdentifier().GetConnectionId().c_str()));
        }

    return Compare(CompareWithConnectionParams(*connection, params));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
HierarchiesComparer::CompareResult HierarchiesComparer::Compare(HierarchiesComparer::CompareWithConnectionParams const& params) const
    {
    if (nullptr != params.GetContinuationToken())
        m_startLocationLookup = std::make_unique<StartLookupContext>(*params.GetContinuationToken());

    return DoCompare(params);
    }
