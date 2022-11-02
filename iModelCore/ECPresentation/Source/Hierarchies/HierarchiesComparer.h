/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentationManager.h>
#include "../RulesEngineTypes.h"
#include "DataSourceInfo.h"
#include "NavNodesCache.h"
#include "NavNodesHelper.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class HierarchyCompareStatus
    {
    Complete,
    Incomplete
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IHierarchyChangesReporter
{
protected:
    virtual void _OnBeforeCreateLhsProvider(NavNodesProviderContextR) {}
    virtual bool _OnAfterCreatedLhsProvider(NavNodesProviderCR, CombinedHierarchyLevelIdentifier const&) {return true;}
    virtual void _OnLhsProviderNotFound(CombinedHierarchyLevelIdentifier const&) {}
    virtual bool _OnStartCompare(NavNodesProviderCR, NavNodesProviderCR) {return true;}
    virtual void _OnEndCompare(NavNodesProviderCR, NavNodesProviderCP) {}
    virtual void _Added(CombinedHierarchyLevelIdentifier const&, NavNodeCR, NavNodeCPtr, size_t) {}
    virtual void _Removed(CombinedHierarchyLevelIdentifier const&, NavNodeCR, NavNodeCPtr, uint64_t) {}
    virtual void _Changed(CombinedHierarchyLevelIdentifier const&, NodeChanges const&) {}
    virtual bool _ShouldContinue() {return true;}
public:
    ~IHierarchyChangesReporter() {}
    void OnBeforeCreateLhsProvider(NavNodesProviderContextR context) {_OnBeforeCreateLhsProvider(context);}
    bool OnAfterCreatedLhsProvider(NavNodesProviderCR provider, CombinedHierarchyLevelIdentifier const& hlInfo) {return _OnAfterCreatedLhsProvider(provider, hlInfo);}
    void OnLhsProviderNotFound(CombinedHierarchyLevelIdentifier const& hlInfo) {_OnLhsProviderNotFound(hlInfo);}
    bool StartCompare(NavNodesProviderCR lhs, NavNodesProviderCR rhs) {return _OnStartCompare(lhs, rhs);}
    void EndCompare(NavNodesProviderCR lhs, NavNodesProviderCP rhs) {_OnEndCompare(lhs, rhs);}
    void Added(CombinedHierarchyLevelIdentifier const& hli, NavNodeCR node, NavNodeCPtr parentNode, size_t index) {_Added(hli, node, parentNode, index);}
    void Removed(CombinedHierarchyLevelIdentifier const& hli, NavNodeCR node, NavNodeCPtr parentNode, uint64_t position) {_Removed(hli, node, parentNode, position);}
    void Changed(CombinedHierarchyLevelIdentifier const& hli, NodeChanges const& changes) {_Changed(hli, changes);}
    bool ShouldContinue() {return _ShouldContinue();}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct HierarchiesComparer
{
    struct ComparerParams
    {
    private:
        IConnectionCacheCR m_connections;
        std::shared_ptr<INavNodesCache> m_nodesCache;
        INodesProviderContextFactoryCR m_contextFactory;
        INodesProviderFactoryCR m_providerFactory;
    public:
        ComparerParams(IConnectionCacheCR connections, std::shared_ptr<INavNodesCache> nodesCache, INodesProviderContextFactoryCR contextFactory, INodesProviderFactoryCR providerFactory)
            : m_connections(connections), m_nodesCache(nodesCache), m_contextFactory(contextFactory), m_providerFactory(providerFactory)
            {}
        IConnectionCacheCR GetConnections() const {return m_connections;}
        std::shared_ptr<INavNodesCache> GetNodesCache() const {return m_nodesCache;}
        INodesProviderContextFactoryCR GetProviderContextsFactory() const {return m_contextFactory;}
        INodesProviderFactoryCR GetProvidersFactory() const {return m_providerFactory;}
    };

    struct CompareParams
    {
    private:
        IHierarchyChangesReporter& m_reporter;
        CombinedHierarchyLevelIdentifier const& m_lhsHierarchy;
        CombinedHierarchyLevelIdentifier const& m_rhsHierarchy;
        RulesetVariables const& m_lhsVariables;
        RulesetVariables const& m_rhsVariables;
        bvector<NavNodeKeyCPtr> const& m_expandedNodeKeys;
        ICancelationTokenCP m_cancellationToken;
        HierarchyComparePositionPtr m_continuationToken;
        bool m_traverseRecursively;
        bool m_enableLoadingLhsNodes;
    public:
        CompareParams(IHierarchyChangesReporter& reporter, CombinedHierarchyLevelIdentifier const& lhsHierarchyIdentifier, RulesetVariables const& lhsVariables,
            CombinedHierarchyLevelIdentifier const& rhsHierarchyIdentifier, RulesetVariables const& rhsVariables, bvector<NavNodeKeyCPtr> const& expandedNodeKeys,
            HierarchyComparePositionPtr continuationToken, bool shouldTraverseRecursively, bool enableLoadingLhsNodes, ICancelationTokenCP cancellationToken)
            : m_reporter(reporter), m_lhsHierarchy(lhsHierarchyIdentifier), m_lhsVariables(lhsVariables), m_rhsHierarchy(rhsHierarchyIdentifier), m_rhsVariables(rhsVariables),
            m_expandedNodeKeys(expandedNodeKeys), m_continuationToken(continuationToken), m_traverseRecursively(shouldTraverseRecursively), m_enableLoadingLhsNodes(enableLoadingLhsNodes),
            m_cancellationToken(cancellationToken)
            {}
        IHierarchyChangesReporter& Reporter() const {return m_reporter;}
        CombinedHierarchyLevelIdentifier const& GetLhsHierarchyIdentifier() const {return m_lhsHierarchy;}
        CombinedHierarchyLevelIdentifier const& GetRhsHierarchyIdentifier() const {return m_rhsHierarchy;}
        RulesetVariables const& GetLhsVariables() const {return m_lhsVariables;}
        RulesetVariables const& GetRhsVariables() const {return m_rhsVariables;}
        bvector<NavNodeKeyCPtr> const& GetExpandedNodeKeys() const { return m_expandedNodeKeys; }
        ICancelationTokenCP GetCancellationToken() const {return m_cancellationToken;}
        HierarchyComparePositionPtr GetContinuationToken() const {return m_continuationToken;}
        bool ShouldTraverseRecursively() const {return m_traverseRecursively;}
        bool ShouldLoadLhsNodes() const {return m_enableLoadingLhsNodes;}
    };

    struct CompareWithConnectionParams : CompareParams
    {
    private:
        IConnectionCR m_connection;
    public:
        CompareWithConnectionParams(IHierarchyChangesReporter& reporter, IConnectionCR connection, CombinedHierarchyLevelIdentifier const& lhsHierarchyIdentifier,
            RulesetVariables const& lhsVariables, CombinedHierarchyLevelIdentifier const& rhsHierarchyIdentifier, RulesetVariables const& rhsVariables,
            bvector<NavNodeKeyCPtr> const& expandedNodeKeys, HierarchyComparePositionPtr continuationToken, bool shouldTraverseRecursively, bool enableLoadingLhsNodes, ICancelationTokenCP cancellationToken)
            : CompareParams(reporter, lhsHierarchyIdentifier, lhsVariables, rhsHierarchyIdentifier, rhsVariables, expandedNodeKeys, continuationToken, shouldTraverseRecursively, enableLoadingLhsNodes, cancellationToken),
            m_connection(connection)
            {}
        CompareWithConnectionParams(IConnectionCR connection, CompareParams const& baseParams)
            : CompareParams(baseParams), m_connection(connection)
            {}
        IConnectionCR GetConnection() const {return m_connection;}
    };

    struct CompareResult
    {
    private:
        HierarchyCompareStatus m_status;
        HierarchyComparePosition m_continuationToken;
    public:
        CompareResult() : m_status(HierarchyCompareStatus::Complete) {}
        CompareResult(Utf8StringCR nextLhsNode, Utf8StringCR nextRhsNode) : m_status(HierarchyCompareStatus::Incomplete), m_continuationToken(nextLhsNode, nextRhsNode) {}
        CompareResult(HierarchyComparePosition continuationToken) : m_status(HierarchyCompareStatus::Incomplete), m_continuationToken(continuationToken) {}
        HierarchyCompareStatus GetStatus() const {return m_status;}
        HierarchyComparePosition const& GetContinuationToken() const {return m_continuationToken;}
    };

    struct StartLookupContext
    {
    private:
        bvector<Utf8String> m_lhsHashPath;
        bvector<Utf8String> m_rhsHashPath;
        size_t m_depth;
        int m_currentDepth;
    public:
        StartLookupContext(HierarchyComparePosition const& hashPaths)
            : m_currentDepth(0), m_lhsHashPath(NavNodesHelper::NodeKeyHashPathFromString(hashPaths.first.c_str())), m_rhsHashPath(NavNodesHelper::NodeKeyHashPathFromString(hashPaths.second.c_str()))
            {
            m_depth = std::max(m_lhsHashPath.size(), m_rhsHashPath.size());
            }
        void IncrementDepth() {m_currentDepth++;}
        Utf8String GetLhsHash() const {return m_lhsHashPath.size() > m_currentDepth ? m_lhsHashPath.at(m_currentDepth) : "";}
        Utf8String GetRhsHash() const {return m_rhsHashPath.size() > m_currentDepth ? m_rhsHashPath.at(m_currentDepth) : "";}
        bool HasDeeperLevels() const {return m_depth > m_currentDepth + 1;}
    };

private:
    ComparerParams m_params;
    mutable std::unique_ptr<StartLookupContext> m_startLocationLookup;

private:
    NavNodesProviderPtr CreateProvider(NavNodesProviderContextR context) const;
    NavNodesProviderPtr CreateProvider(IConnectionCR, CombinedHierarchyLevelIdentifier const&, RulesetVariables const&) const;
    NavNodesProviderPtr GetCachedOrCreateProvider(IConnectionCR, CombinedHierarchyLevelIdentifier const&, RulesetVariables const&, bool, IHierarchyChangesReporter*) const;
    CompareResult CompareDataSources(CompareWithConnectionParams const&, NavNodesProviderCR oldProvider, NavNodesProviderR newProvider) const;
    CompareResult CompareNodes(CompareWithConnectionParams const&, NavNodesProviderCR lhsProvider, NavNodeCR oldNode, NavNodesProviderCR newProvider, NavNodeR newNode) const;
    CompareResult DoCompare(CompareWithConnectionParams const&) const;
    void CustomizeNode(NavNodeCP oldNode, NavNodeR newNode, NavNodesProviderCR newNodeProvider) const;

public:
    HierarchiesComparer(ComparerParams params) : m_params(params) {}
    CompareResult Compare(CompareParams const&) const;
    CompareResult Compare(CompareWithConnectionParams const&) const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
