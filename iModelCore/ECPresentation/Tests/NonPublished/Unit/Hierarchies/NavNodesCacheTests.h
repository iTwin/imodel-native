/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentationManager.h>
#include <UnitTests/ECPresentation/TestRuleSetLocater.h>
#include "../../../../Source/Hierarchies/NavNodesCache.h"
#include "../../Helpers/ECDbTestProject.h"
#include "../../Helpers/TestNavNode.h"
#include "../../Helpers/TestHelpers.h"
#include "../../Helpers/TestNodesProvider.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodesCacheTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;

    TestConnectionManager m_connections;
    NavNodesFactory m_nodesFactory;
    TestNodesProviderContextFactory m_nodesProviderContextFactory;
    TestNodesProviderFactory m_providersFactory;
    IConnectionPtr m_connection;
    std::shared_ptr<NodesCache> m_cache;

    NodesCacheTests()
        : m_nodesProviderContextFactory(m_connections)
        {
        m_providersFactory.SetPostProcessors({ std::make_unique<NodesFinalizingPostProcessor>() });
        }

    void SetUp() override;
    virtual void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();
    static ECDbR GetDb() { return s_project->GetECDb(); }

    virtual std::shared_ptr<NodesCache> _CreateNodesCache(IConnectionR connection, BeFileNameCR tempDirectory);

    void ReCreateProject();
    void ReCreateNodesCache();

    NavNodesProviderContextPtr CreateContext(std::shared_ptr<INavNodesCache> cache, CombinedHierarchyLevelIdentifier const& id, RulesetVariables vars = RulesetVariables()) const;
    NavNodesProviderContextPtr CreateContext(Utf8StringCR rulesetId, BeGuidCR parentId, RulesetVariables vars = RulesetVariables()) const;
    NavNodesProviderContextPtr CreateContext(CombinedHierarchyLevelIdentifier const& id, RulesetVariables vars = RulesetVariables()) const;

    void InitNode(NavNodeR, HierarchyLevelIdentifier const&);
    void FillWithNodes(bpair<HierarchyLevelIdentifier, DataSourceIdentifier> const&, bvector<NavNodePtr> const&, bool createChildDataSources = false, bool areVirtual = false);
    bvector<NavNodePtr> FillWithNodes(bpair<HierarchyLevelIdentifier, DataSourceIdentifier> const&, size_t count, bool createChildDataSources = false, bool areVirtual = false);
    HierarchyLevelIdentifier CacheHierarchyLevel(IHierarchyCache& cache, Utf8StringCR connectionId, Utf8StringCR rulesetId, BeGuidCR virtualParentId = BeGuid(), BeGuidCR physicalParentId = BeGuid());
    HierarchyLevelIdentifier CacheHierarchyLevel(Utf8StringCR connectionId, Utf8StringCR rulesetId, BeGuidCR virtualParentId = BeGuid(), BeGuidCR physicalParentId = BeGuid());
    bpair<HierarchyLevelIdentifier, DataSourceIdentifier> CacheDataSource(IHierarchyCache& cache, Utf8StringCR connectionId, Utf8StringCR rulesetId, BeGuidCR virtualParentId = BeGuid(), bool finalize = true, RulesetVariables const& variables = RulesetVariables());
    bpair<HierarchyLevelIdentifier, DataSourceIdentifier> CacheDataSource(Utf8StringCR connectionId, Utf8StringCR rulesetId, BeGuidCR virtualParentId = BeGuid(), bool finalize = true, RulesetVariables const& variables = RulesetVariables());
    bpair<HierarchyLevelIdentifier, DataSourceIdentifier> CacheDataSource(IHierarchyCache& cache, Utf8StringCR connectionId, Utf8StringCR rulesetId, BeGuidCR virtualParentId, BeGuidCR physicalParentId, bool finalize = true, RulesetVariables const& variables = RulesetVariables());
    bpair<HierarchyLevelIdentifier, DataSourceIdentifier> CacheDataSource(Utf8StringCR connectionId, Utf8StringCR rulesetId, BeGuidCR virtualParentId, BeGuidCR physicalParentId, bool finalize = true, RulesetVariables const& variables = RulesetVariables());
    };
