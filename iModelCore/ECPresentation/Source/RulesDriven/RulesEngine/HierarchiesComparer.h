/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "RulesEngineTypes.h"
#include "DataSourceInfo.h"
#include "NavNodesCache.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2020
+===============+===============+===============+===============+===============+======*/
struct IHierarchyChangesReporter
{
protected:
    virtual void _OnFoundLhsProvider(NavNodesProviderCR, CombinedHierarchyLevelIdentifier const&) {}
    virtual bool _OnStartCompare(NavNodesProviderCR, NavNodesProviderCR) {return true;}
    virtual void _OnEndCompare(NavNodesProviderCR, NavNodesProviderCP) {}
    virtual void _Added(HierarchyLevelIdentifier const&, JsonNavNodeCR, size_t) {}
    virtual void _Removed(HierarchyLevelIdentifier const&, JsonNavNodeCR) {}
    virtual void _Changed(HierarchyLevelIdentifier const&, JsonNavNodeCR, JsonNavNodeCR, bvector<JsonChange> const&) {}
public:
    ~IHierarchyChangesReporter() {}
    void FoundLhsProvider(NavNodesProviderCR provider, CombinedHierarchyLevelIdentifier const& hlInfo) {_OnFoundLhsProvider(provider, hlInfo);}
    bool StartCompare(NavNodesProviderCR lhs, NavNodesProviderCR rhs) {return _OnStartCompare(lhs, rhs);}
    void EndCompare(NavNodesProviderCR lhs, NavNodesProviderCP rhs) {_OnEndCompare(lhs, rhs);}
    void Added(HierarchyLevelIdentifier const& hli, JsonNavNodeCR node, size_t index) {_Added(hli, node, index);}
    void Removed(HierarchyLevelIdentifier const& hli, JsonNavNodeCR node) {_Removed(hli, node);}
    void Changed(HierarchyLevelIdentifier const& hli, JsonNavNodeCR lhsNode, JsonNavNodeCR rhsNode, bvector<JsonChange> const& changes) {_Changed(hli, lhsNode, rhsNode, changes);}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2020
+===============+===============+===============+===============+===============+======*/
struct HierarchiesComparer
{
    struct Params
    {
    private:
        NodesCache& m_nodesCache;
        INodesProviderContextFactoryCR m_contextFactory;
        INodesProviderFactoryCR m_providerFactory;
        bool m_traverseRecursively;
        IHierarchyChangesReporter& m_changesReporter;
        std::unique_ptr<RulesetVariables> m_variables;
    public:
        Params(NodesCache& nodesCache, INodesProviderContextFactoryCR contextFactory, INodesProviderFactoryCR providerFactory,
            IHierarchyChangesReporter& changesReporter, bool traverseRecursively, std::unique_ptr<RulesetVariables> variables)
            : m_nodesCache(nodesCache), m_contextFactory(contextFactory), m_providerFactory(providerFactory),
            m_traverseRecursively(traverseRecursively), m_changesReporter(changesReporter), m_variables(std::move(variables))
            {}
        Params(Params const& other)
            : m_nodesCache(other.m_nodesCache), m_contextFactory(other.m_contextFactory), m_providerFactory(other.m_providerFactory),
            m_traverseRecursively(other.m_traverseRecursively), m_changesReporter(other.m_changesReporter), m_variables(std::make_unique<RulesetVariables>(*other.m_variables))
            {}
        NodesCache& GetNodesCache() const {return m_nodesCache;}
        INodesProviderContextFactoryCR GetProviderContextsFactory() const {return m_contextFactory;}
        INodesProviderFactoryCR GetProvidersFactory() const {return m_providerFactory;}
        bool ShouldTraverseRecursively() const {return m_traverseRecursively;}
        IHierarchyChangesReporter& Reporter() const {return m_changesReporter;}
        RulesetVariables const& GetVariables() const { return *m_variables; }
    };

    struct Context : Params
    {
    friend struct HierarchiesComparer;
    private:
        Context(Params const& params) : Params(params) {}
    };

private:
    Context m_context;

private:
    NavNodesProviderPtr CreateProvider(IConnectionCR, HierarchyLevelIdentifier const&) const;
    void Compare(IConnectionCR, HierarchyLevelIdentifier const&, HierarchyLevelIdentifier const&) const;
    void CompareDataSources(NavNodesProviderCR oldProvider, NavNodesProviderR newProvider) const;
    void CompareNodes(NavNodesProviderCR lhsProvider, JsonNavNodeCR oldNode, NavNodesProviderCR newProvider, JsonNavNodeR newNode) const;
    void CustomizeNode(JsonNavNodeCP oldNode, JsonNavNodeR newNode, NavNodesProviderCR newNodeProvider) const;

public:
    HierarchiesComparer(Params const& params) : m_context(params) {}
    void Compare(IConnectionCacheCR connections, HierarchyLevelIdentifier const&, HierarchyLevelIdentifier const&) const;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
