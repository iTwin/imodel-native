/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "NavNodesCacheTests.h"

ECDbTestProject* NodesCacheTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheTests::SetUp()
    {
    ECPresentationTest::SetUp();
    if (!s_project->GetECDb().IsDbOpen())
        s_project->Open("NodesCacheTests", Db::OpenParams(Db::OpenMode::ReadWrite));

    ReCreateNodesCache();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheTests::TearDown()
    {
    BeFileName cacheFile(m_cache->GetDb().GetDbFileName());
    m_nodesProviderContextFactory.SetNodesCache(nullptr);
    m_cache = nullptr;
    m_connections.CloseConnections();
    if (cacheFile.DoesPathExist())
        cacheFile.BeDeleteFile();
    s_project->GetECDb().AbandonChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheTests::SetUpTestCase()
    {
    s_project = new ECDbTestProject();
    s_project->Create("NodesCacheTests", "RulesEngineTest.01.00.ecschema.xml");
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheTests::TearDownTestCase()
    {
    s_project->GetECDb().CloseDb();
    DELETE_AND_CLEAR(s_project);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheTests::ReCreateProject()
    {
    if (nullptr != s_project)
        {
        m_connections.NotifyConnectionClosed(*m_connection);
        DELETE_AND_CLEAR(s_project);
        }

    s_project = new ECDbTestProject();
    s_project->Open("NodesCacheTests", Db::OpenParams(Db::OpenMode::ReadWrite));
    m_connection = m_connections.NotifyConnectionOpened(s_project->GetECDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<NodesCache> NodesCacheTests::_CreateNodesCache(IConnectionR connection, BeFileNameCR tempDirectory)
    {
    return NodesCache::Create(connection, tempDirectory, m_nodesFactory, m_nodesProviderContextFactory, m_providersFactory, NodesCacheType::Memory, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheTests::ReCreateNodesCache()
    {
    m_nodesProviderContextFactory.SetNodesCache(nullptr);
    m_cache = nullptr;
    BeFileName temporaryDirectory;
    BeTest::GetHost().GetTempDir(temporaryDirectory);
    m_connection = m_connections.NotifyConnectionOpened(s_project->GetECDb());
    m_cache = _CreateNodesCache(*m_connection, temporaryDirectory);
    m_cache->OnRulesetUsed(*PresentationRuleSet::CreateInstance("ruleset_id"));
    m_nodesProviderContextFactory.SetNodesCache(m_cache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderContextPtr NodesCacheTests::CreateContext(std::shared_ptr<INavNodesCache> cache, CombinedHierarchyLevelIdentifier const& id, RulesetVariables vars) const
    {
    NavNodeCPtr parent = cache->GetNode(id.GetPhysicalParentNodeId());
    return m_nodesProviderContextFactory.Create(*m_connection, id.GetRulesetId().c_str(), parent.get(), cache, nullptr, vars);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderContextPtr NodesCacheTests::CreateContext(Utf8StringCR rulesetId, BeGuidCR parentId, RulesetVariables vars) const
    {
    return CreateContext(m_cache, CombinedHierarchyLevelIdentifier(m_connection->GetId(), rulesetId, parentId), vars);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
NavNodesProviderContextPtr NodesCacheTests::CreateContext(CombinedHierarchyLevelIdentifier const& id, RulesetVariables vars) const
    {
    return CreateContext(m_cache, id, vars);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheTests::InitNode(NavNodeR node, HierarchyLevelIdentifier const& info)
    {
    if (info.GetPhysicalParentNodeId().IsValid())
        node.SetParentNodeId(info.GetPhysicalParentNodeId());

    NavNodeExtendedData extendedData(node);
    extendedData.SetConnectionId(info.GetConnectionId());
    extendedData.SetRulesetId(info.GetRulesetId().c_str());
    if (info.GetVirtualParentNodeId().IsValid())
        extendedData.SetVirtualParentIds({info.GetVirtualParentNodeId()});
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheTests::FillWithNodes(bpair<HierarchyLevelIdentifier, DataSourceIdentifier> const& info, bvector<NavNodePtr> const& nodes, bool createChildDataSources, bool areVirtual)
    {
    for (size_t i = 0; i < nodes.size(); ++i)
        {
        InitNode(*nodes[i], info.first);
        m_cache->Cache(*nodes[i], info.second, (uint64_t)i, areVirtual ? NodeVisibility::Virtual : NodeVisibility::Visible);

        if (createChildDataSources)
            CacheDataSource(info.first.GetConnectionId(), info.first.GetRulesetId(), nodes[i]->GetNodeId());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<NavNodePtr> NodesCacheTests::FillWithNodes(bpair<HierarchyLevelIdentifier, DataSourceIdentifier> const& info, size_t count, bool createChildDataSources, bool areVirtual)
    {
    bvector<NavNodePtr> nodes;
    for (size_t i = 0; i < count; ++i)
        {
        auto node = TestNodesHelper::CreateCustomNode(*m_connection, "type", std::to_string(i).c_str(), "descr");
        nodes.push_back(node);
        }
    FillWithNodes(info, nodes, createChildDataSources, areVirtual);
    return nodes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
HierarchyLevelIdentifier NodesCacheTests::CacheHierarchyLevel(Utf8StringCR connectionId, Utf8StringCR rulesetId, BeGuidCR virtualParentId, BeGuidCR physicalParentId)
    {
    return CacheHierarchyLevel(*m_cache, connectionId, rulesetId, virtualParentId, physicalParentId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
HierarchyLevelIdentifier NodesCacheTests::CacheHierarchyLevel(IHierarchyCache& cache, Utf8StringCR connectionId, Utf8StringCR rulesetId, BeGuidCR virtualParentId, BeGuidCR physicalParentId)
    {
    HierarchyLevelIdentifier hlInfo = HierarchyLevelIdentifier(connectionId, rulesetId, physicalParentId, virtualParentId);
    cache.Cache(hlInfo);
    EXPECT_TRUE(hlInfo.IsValid());
    return hlInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
bpair<HierarchyLevelIdentifier, DataSourceIdentifier> NodesCacheTests::CacheDataSource(Utf8StringCR connectionId, Utf8StringCR rulesetId, BeGuidCR virtualParentId, BeGuidCR physicalParentId, bool finalize, RulesetVariables const& variables)
    {
    return CacheDataSource(*m_cache, connectionId, rulesetId, virtualParentId, physicalParentId, finalize, variables);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
bpair<HierarchyLevelIdentifier, DataSourceIdentifier> NodesCacheTests::CacheDataSource(IHierarchyCache& cache, Utf8StringCR connectionId, Utf8StringCR rulesetId, BeGuidCR virtualParentId, BeGuidCR physicalParentId, bool finalize, RulesetVariables const& variables)
    {
    HierarchyLevelIdentifier hlInfo = cache.FindHierarchyLevel(connectionId.c_str(), rulesetId.c_str(), virtualParentId, BeGuid());
    if (!hlInfo.IsValid())
        hlInfo = CacheHierarchyLevel(cache, connectionId, rulesetId, virtualParentId, physicalParentId);

    DataSourceInfo dsInfo(DataSourceIdentifier(hlInfo.GetId(), { 0 }), variables, DataSourceFilter(), bmap<ECClassId, bool>(), "", "");
    dsInfo.SetIsInitialized(finalize);

    cache.Cache(dsInfo);
    EXPECT_TRUE(dsInfo.GetIdentifier().IsValid());

    return bpair<HierarchyLevelIdentifier, DataSourceIdentifier>(hlInfo, dsInfo.GetIdentifier());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
bpair<HierarchyLevelIdentifier, DataSourceIdentifier> NodesCacheTests::CacheDataSource(Utf8StringCR connectionId, Utf8StringCR rulesetId, BeGuidCR parentId, bool finalize, RulesetVariables const& variables)
    {
    return CacheDataSource(*m_cache, connectionId, rulesetId, parentId, parentId, finalize, variables);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
bpair<HierarchyLevelIdentifier, DataSourceIdentifier> NodesCacheTests::CacheDataSource(IHierarchyCache& cache, Utf8StringCR connectionId, Utf8StringCR rulesetId, BeGuidCR parentId, bool finalize, RulesetVariables const& variables)
    {
    return CacheDataSource(cache, connectionId, rulesetId, parentId, parentId, finalize, variables);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsNodeCached(NodesCache& cache, BeGuidCR nodeId)
    {
    static Utf8CP query =
        "SELECT 1 FROM [" NODESCACHE_TABLENAME_Nodes "] "
        "WHERE [Id] = ?";

    Statement stmt(cache.GetDb(), query);
    NodesCacheHelpers::BindGuid(stmt, 1, nodeId);
    return (BE_SQLITE_ROW == stmt.Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsHierarchyLevelCached(NodesCache& cache, Utf8CP rulesetId)
    {
    static Utf8CP query =
        "SELECT 1 "
        "FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] hl "
        "JOIN [" NODESCACHE_TABLENAME_PhysicalHierarchyLevels "] phl ON [phl].[Id] = [hl].[PhysicalHierarchyLevelId] "
        "JOIN [" NODESCACHE_TABLENAME_Rulesets "] r ON [r].[Id] = [phl].[RulesetId] "
        "WHERE [r].[Identifier] = ? AND [hl].[VirtualParentNodeId] IS NULL";

    Statement stmt(cache.GetDb(), query);
    stmt.BindText(1, rulesetId, Statement::MakeCopy::No);
    return (BE_SQLITE_ROW == stmt.Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsHierarchyLevelCached(NodesCache& cache, BeGuidCR virtualParentNodeId)
    {
    static Utf8CP query =
        "SELECT 1 FROM [" NODESCACHE_TABLENAME_HierarchyLevels "] "
        "WHERE [VirtualParentNodeId] = ?";

    Statement stmt(cache.GetDb(), query);
    NodesCacheHelpers::BindGuid(stmt, 1, virtualParentNodeId);
    return (BE_SQLITE_ROW == stmt.Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Clear_Full)
    {
    // create a different ruleset
    m_cache->OnRulesetUsed(*PresentationRuleSet::CreateInstance("ruleset_id2"));

    // cache
    CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    CacheDataSource(m_connection->GetId(), "ruleset_id2", BeGuid());
    CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());

    // make sure everything's cached
    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id", BeGuid(), BeGuid()).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id2", BeGuid(), BeGuid()).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id", BeGuid(), BeGuid()).IsValid());

    // fully clear the cache
    m_cache->Clear();

    // verify everything's gone
    EXPECT_FALSE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id", BeGuid(), BeGuid()).IsValid());
    EXPECT_FALSE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id2", BeGuid(), BeGuid()).IsValid());
    EXPECT_FALSE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id", BeGuid(), BeGuid()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Clear_ByRulesetId)
    {
    // create a different ruleset
    m_cache->OnRulesetUsed(*PresentationRuleSet::CreateInstance("ruleset_id2"));

    // cache
    CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    CacheDataSource(m_connection->GetId(), "ruleset_id2", BeGuid());

    // make sure everything's cached
    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id", BeGuid(), BeGuid()).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id2", BeGuid(), BeGuid()).IsValid());

    // clear "ruleset_id" hierarchy from cache
    m_cache->Clear("ruleset_id");

    // verify only "ruleset_id" hierarchy was removed
    EXPECT_FALSE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id", BeGuid(), BeGuid()).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id2", BeGuid(), BeGuid()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemoveHierarchyLevel_RemovesHierarchyLevel)
    {
    // cache data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id");

    // verify it got cached
    EXPECT_TRUE(m_cache->FindHierarchyLevel(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(), BeGuid(), BeGuid()).IsValid());

    // attempt to remove with invalid ruleset id and verify it didn't get removed
    BeGuid removalId = m_cache->CreateRemovalId(CombinedHierarchyLevelIdentifier(info.first.GetConnectionId(),
        "invalid", BeGuid()));
    m_cache->RemoveHierarchyLevel(removalId);
    EXPECT_TRUE(m_cache->FindHierarchyLevel(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(), BeGuid(), BeGuid()).IsValid());

    // attempt to remove with invalid parent node and verify it didn't get removed
    removalId = m_cache->CreateRemovalId(CombinedHierarchyLevelIdentifier(info.first.GetConnectionId(),
        info.first.GetRulesetId(), BeGuid(true)));
    m_cache->RemoveHierarchyLevel(removalId);
    EXPECT_TRUE(m_cache->FindHierarchyLevel(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(), BeGuid(), BeGuid()).IsValid());

    // attempt to remove with valid parameters and verify it did get removed
    removalId = m_cache->CreateRemovalId(CombinedHierarchyLevelIdentifier(info.first.GetConnectionId(),
        info.first.GetRulesetId(), BeGuid()));
    m_cache->RemoveHierarchyLevel(removalId);
    EXPECT_FALSE(m_cache->FindHierarchyLevel(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(), BeGuid(), BeGuid()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemoveHierarchyLevel_RemovesChildHierarchyLevel)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id");
    auto nodes = FillWithNodes(rootInfo, 1);
    auto childInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", nodes[0]->GetNodeId());

    // verify child data source is cached
    EXPECT_TRUE(m_cache->GetHierarchyLevel(*CreateContext("ruleset_id", nodes[0]->GetNodeId()), childInfo.first).IsValid());

    // remove child data source
    BeGuid removalId = m_cache->CreateRemovalId(CombinedHierarchyLevelIdentifier(childInfo.first.GetConnectionId(),
        childInfo.first.GetRulesetId(), nodes[0]->GetNodeId()));
    m_cache->RemoveHierarchyLevel(removalId);

    // verify child data source also got removed
    EXPECT_FALSE(m_cache->GetHierarchyLevel(*CreateContext("ruleset_id", nodes[0]->GetNodeId()), childInfo.first).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemoveHierarchyLevel_RemovesChildNodesWhenParentHierarchyLevelIsRemoved)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id");
    auto nodes = FillWithNodes(rootInfo, 1);

    // verify node is cached
    EXPECT_TRUE(m_cache->GetNode(nodes[0]->GetNodeId()).IsValid());

    // remove child data source
    BeGuid removalId = m_cache->CreateRemovalId(CombinedHierarchyLevelIdentifier(rootInfo.first.GetConnectionId(),
        rootInfo.first.GetRulesetId(), BeGuid()));
    m_cache->RemoveHierarchyLevel(removalId);

    // verify child data source also got removed
    EXPECT_FALSE(m_cache->GetNode(nodes[0]->GetNodeId()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemovesChildDataSourcesWhenParentDataSourceIsRemoved)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id");
    auto nodes = FillWithNodes(rootInfo, 1);
    auto childInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", nodes[0]->GetNodeId());

    // verify child data source is cached
    EXPECT_TRUE(m_cache->GetHierarchyLevel(*CreateContext("ruleset_id", nodes[0]->GetNodeId()), childInfo.first).IsValid());

    // remove root data source
    BeGuid removalId = m_cache->CreateRemovalId(CombinedHierarchyLevelIdentifier(rootInfo.first.GetConnectionId(),
        rootInfo.first.GetRulesetId(), BeGuid()));
    m_cache->RemoveHierarchyLevel(removalId);

    // verify child data source also got removed
    EXPECT_FALSE(m_cache->GetHierarchyLevel(*CreateContext("ruleset_id", nodes[0]->GetNodeId()), childInfo.first).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, MakePhysical)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id");
    auto nodes = FillWithNodes(rootInfo, 1, false, true);

    // cache some children
    auto virtualNodeChildrenInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", nodes[0]->GetNodeId(), BeGuid());
    auto childNodes = FillWithNodes(virtualNodeChildrenInfo, 1, false, true);

    // cache some grand-children
    auto virtualNodeGrandChildrenInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", childNodes[0]->GetNodeId(), BeGuid());
    auto grandChildNodes = FillWithNodes(virtualNodeGrandChildrenInfo, 1, false, true);

    // verify node is virtual
    EXPECT_EQ(NodeVisibility::Virtual, m_cache->GetNodeVisibility(nodes[0]->GetNodeId()));

    // verify child and grandchild nodes' physical parent is not set (root)
    EXPECT_EQ(BeGuid(), m_cache->GetNode(childNodes[0]->GetNodeId())->GetParentNodeId());
    EXPECT_EQ(BeGuid(), m_cache->GetNode(grandChildNodes[0]->GetNodeId())->GetParentNodeId());

    // make physical
    m_cache->MakePhysical(*nodes[0]);

    // verify node is physical
    EXPECT_EQ(NodeVisibility::Visible, m_cache->GetNodeVisibility(nodes[0]->GetNodeId()));

    // verify child and grandchild nodes' physical parent is now set
    EXPECT_EQ(nodes[0]->GetNodeId(), m_cache->GetNode(childNodes[0]->GetNodeId())->GetParentNodeId());
    EXPECT_EQ(nodes[0]->GetNodeId(), m_cache->GetNode(grandChildNodes[0]->GetNodeId())->GetParentNodeId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, MakeVirtual)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id");
    auto nodes = FillWithNodes(rootInfo, 1, false, false);

    // cache some children
    auto virtualNodeChildrenInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", nodes[0]->GetNodeId(), nodes[0]->GetNodeId());
    auto childNodes = FillWithNodes(virtualNodeChildrenInfo, 1, false, false);

    // cache some grand-children
    auto virtualNodeGrandChildrenInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", childNodes[0]->GetNodeId(), nodes[0]->GetNodeId());
    auto grandChildNodes = FillWithNodes(virtualNodeGrandChildrenInfo, 1, false, false);

    // verify node is physical
    EXPECT_EQ(NodeVisibility::Visible, m_cache->GetNodeVisibility(nodes[0]->GetNodeId()));

    // verify child and grandchild nodes' physical parent is set to their closest physical parent node
    EXPECT_EQ(nodes[0]->GetNodeId(), m_cache->GetNode(childNodes[0]->GetNodeId())->GetParentNodeId());
    EXPECT_EQ(nodes[0]->GetNodeId(), m_cache->GetNode(grandChildNodes[0]->GetNodeId())->GetParentNodeId());

    // make virtual
    m_cache->MakeVirtual(*nodes[0]);

    // verify node is virtual
    EXPECT_EQ(NodeVisibility::Virtual, m_cache->GetNodeVisibility(nodes[0]->GetNodeId()));

    // verify child and grandchild nodes' physical parent is updated
    EXPECT_EQ(nodes[0]->GetParentNodeId(), m_cache->GetNode(childNodes[0]->GetNodeId())->GetParentNodeId());
    EXPECT_EQ(nodes[0]->GetParentNodeId(), m_cache->GetNode(grandChildNodes[0]->GetNodeId())->GetParentNodeId());
    }

/*---------------------------------------------------------------------------------**//**
* note: the test uses GetRelatedHierarchyLevels function to test the Update
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, UpdateDataSource_UpdatesFilter)
    {
    ECClassCP widgetHasGadgetRelationship = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    // cache the data source
    HierarchyLevelIdentifier hlInfo(m_connection->GetId(), "ruleset_id", BeGuid(), BeGuid());
    m_cache->Cache(hlInfo);
    EXPECT_TRUE(hlInfo.IsValid());

    ECClassId usedClassId((uint64_t)1);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[usedClassId] = false;

    DataSourceInfo dsInfo(DataSourceIdentifier(hlInfo.GetId(), { 0 }), RulesetVariables(), DataSourceFilter(), usedClassIds, "", "");
    m_cache->Cache(dsInfo);
    EXPECT_TRUE(dsInfo.GetIdentifier().IsValid());

    // verify the filter is not applied and we find related hierarchy level
    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(usedClassId, ECInstanceId((uint64_t)1)));
    bset<CombinedHierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    EXPECT_EQ(1, related.size());

    // update the datasource with new filter
    bvector<ECClassId> relationshipClassIds = {widgetHasGadgetRelationship->GetId()};
    dsInfo.SetFilter(DataSourceFilter(std::make_unique<DataSourceFilter::RelatedInstanceInfo>(relationshipClassIds, RequiredRelationDirection_Both, bvector<ECInstanceKey>{ECInstanceKey(usedClassId, ECInstanceId((uint64_t)2))}), nullptr));
    m_cache->Update(dsInfo, DataSourceInfo::PART_Filter);

    // verify the filter is applied and we dont find related hierarchy level anymore
    related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    EXPECT_EQ(0, related.size());
    }

/*---------------------------------------------------------------------------------**//**
* note: the test uses GetRelatedHierarchyLevels function to test the Update
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, UpdateDataSource_UpdatesRelatedClassIds)
    {
    // cache the data source
    HierarchyLevelIdentifier hlInfo(m_connection->GetId(), "ruleset_id", BeGuid(), BeGuid());
    m_cache->Cache(hlInfo);
    EXPECT_TRUE(hlInfo.IsValid());

    DataSourceInfo dsInfo(DataSourceIdentifier(hlInfo.GetId(), {0}), RulesetVariables(), DataSourceFilter(), bmap<ECClassId, bool>(), "", "");
    m_cache->Cache(dsInfo);
    EXPECT_TRUE(dsInfo.GetIdentifier().IsValid());

    // verify we don't find related hierarchy level as the key is not related to the data source
    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
    bset<CombinedHierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    EXPECT_EQ(0, related.size());

    // update the datasource related class ids
    bmap<ECN::ECClassId, bool> relatedClassIds;
    relatedClassIds[ECN::ECClassId((uint64_t)1)] = false;
    dsInfo.SetRelatedClasses(relatedClassIds);
    m_cache->Update(dsInfo, DataSourceInfo::PART_RelatedClasses);

    // verify we do find related hierarchy level this time as the data source is now related to the lookup keys
    related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    EXPECT_EQ(1, related.size());
    }

#ifdef wip_ruleset_variables
/*---------------------------------------------------------------------------------**//**
* note: the test uses GetRelatedHierarchyLevels function to test the Update
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, UpdateDataSource_UpdatesRelatedSettings)
    {
    // cache the data source
    HierarchyLevelIdentifier hlInfo(m_connection->GetId(), "ruleset_id", 0, 0);
    m_cache->Cache(hlInfo);
    EXPECT_TRUE(hlInfo.IsValid());

    DataSourceIdentifier dsInfo(hlInfo.GetId(), { 0 });
    m_cache->Cache(dsInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    EXPECT_TRUE(dsInfo.IsValid());

    // verify we don't find related hierarchy level as data source is not related to any setting ids
    bvector<HierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(hlInfo.GetRulesetId().c_str(), "setting_id");
    EXPECT_EQ(0, related.size());

    // update the datasource related settings
    bvector<UserSettingEntry> settings = {UserSettingEntry("setting_id", Json::Value("value0"))};
    m_cache->Update(dsInfo, nullptr, nullptr, &settings);

    // verify we do find related hierarchy level this time as the data source is now related to the setting id
    related = m_cache->GetRelatedHierarchyLevels(hlInfo.GetRulesetId().c_str(), "setting_id");
    EXPECT_EQ(1, related.size());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetFilteredNodesProvider_FindsRootNode)
    {
    RulesetVariables variables({ RulesetVariableEntry("var_id", rapidjson::Value("value1")) });

    // create root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid(), true, variables);

    // create the root nodes
    bvector<NavNodePtr> nodes = {
        TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test_A_test", "test descr"),
        TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test_B_test", "test descr")
        };
    FillWithNodes(info, nodes);

    // filter
    auto context = CreateContext(info.first.GetRulesetId(), BeGuid(), variables);
    NavNodesProviderPtr provider = m_cache->GetFilteredNodesProvider(*context, "A");

    // verify filtered node
    ASSERT_EQ(1, provider->GetNodesCount());
    EXPECT_TRUE((*provider->begin())->Equals(*nodes[0]));

    // try to filter nodes with different ruleset variables
    auto newContext = CreateContext(info.first.GetRulesetId(), BeGuid());
    NavNodesProviderPtr emptyProvider = m_cache->GetFilteredNodesProvider(*newContext, "A");
    // verify that no nodes were found
    EXPECT_EQ(0, emptyProvider->GetNodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetUndeterminedNodesProvider_ReturnsNodeWhichDoesntHaveChildHierarchyLevel)
    {
    RulesetVariables variables({ RulesetVariableEntry("var_id", rapidjson::Value("value1")) });

    // create root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid(), true, variables);
    auto nodes = FillWithNodes(info, 1);

    // expect the node to have undetermined children
    EXPECT_FALSE(m_cache->GetNode(nodes[0]->GetNodeId())->DeterminedChildren());

    // expect provider to contain the node
    auto context = CreateContext(info.first.GetRulesetId(), BeGuid(), variables);
    NavNodesProviderPtr provider = m_cache->GetUndeterminedNodesProvider(*context);
    ASSERT_TRUE(provider.IsValid());
    ASSERT_EQ(1, provider->GetNodesCount());

    NavNodePtr expectedNode = *provider->begin();
    EXPECT_EQ(expectedNode->GetNodeId(), nodes[0]->GetNodeId());
    EXPECT_TRUE(expectedNode->DeterminedChildren());

    // expect empty provider if different variables are used
    auto newContext = CreateContext(info.first.GetRulesetId(), BeGuid());
    NavNodesProviderPtr emptyProvider = m_cache->GetUndeterminedNodesProvider(*newContext);
    ASSERT_TRUE(emptyProvider.IsValid());
    EXPECT_EQ(0, emptyProvider->GetNodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetUndeterminedNodesProvider_ReturnsNodeWhichHasUnitializedChildHierarchyLevel)
    {
    RulesetVariables variables({ RulesetVariableEntry("var_id", rapidjson::Value("value1")) });

    // create root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid(), true, variables);
    auto nodes = FillWithNodes(info, 1);
    CacheDataSource(m_connection->GetId(), "ruleset_id", nodes[0]->GetNodeId(), false, variables);

    // expect the node to have undetermined children
    EXPECT_FALSE(m_cache->GetNode(nodes[0]->GetNodeId())->DeterminedChildren());

    // expect provider to contain the node
    auto context = CreateContext(info.first.GetRulesetId(), BeGuid(), variables);
    NavNodesProviderPtr provider = m_cache->GetUndeterminedNodesProvider(*context);
    ASSERT_TRUE(provider.IsValid());
    ASSERT_EQ(1, provider->GetNodesCount());

    NavNodePtr expectedNode = *provider->begin();
    EXPECT_EQ(expectedNode->GetNodeId(), nodes[0]->GetNodeId());
    EXPECT_TRUE(expectedNode->DeterminedChildren());

    // expect empty provider if different variables are used
    auto newContext = CreateContext(info.first.GetRulesetId(), BeGuid());
    NavNodesProviderPtr emptyProvider = m_cache->GetUndeterminedNodesProvider(*newContext);
    ASSERT_TRUE(emptyProvider.IsValid());
    EXPECT_EQ(0, emptyProvider->GetNodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetUndeterminedNodesProvider_DoesNotReturnNodeThatHasInitializedHierarchyLevel)
    {
    // create root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id");
    auto nodes = FillWithNodes(info, 1, true);

    // expect provider to be empty
    auto context = CreateContext(info.first.GetRulesetId(), BeGuid());
    NavNodesProviderPtr provider = m_cache->GetUndeterminedNodesProvider(*context);
    ASSERT_TRUE(provider.IsValid());
    EXPECT_EQ(0, provider->GetNodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsNullsWhenEmpty)
    {
    EXPECT_FALSE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "test ruleset id", BeGuid(), BeGuid()).IsValid());
    EXPECT_FALSE(m_cache->FindHierarchyLevel(BeGuid(true)).IsValid());
    EXPECT_FALSE(m_cache->FindDataSource(DataSourceIdentifier(BeGuid(true), { 0 }), RulesetVariables()).GetIdentifier().IsValid());
    EXPECT_FALSE(m_cache->FindDataSource(BeGuid(true)).GetIdentifier().IsValid());

    BeGuid id(true);
    auto context = CreateContext("test ruleset id", id);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(), "test ruleset id", id)).IsNull());
    EXPECT_TRUE(m_cache->GetHierarchyLevel(*context, HierarchyLevelIdentifier(BeGuid(true), m_connection->GetId(), "test ruleset id", BeGuid(), BeGuid())).IsNull());
    EXPECT_TRUE(m_cache->GetDataSource(*context, DataSourceIdentifier(BeGuid(true), { 0 }), false).IsNull());
    EXPECT_TRUE(m_cache->GetDataSource(*context, id).IsNull());
    EXPECT_TRUE(m_cache->GetNode(id).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedRootDataSource)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id");
    auto context = CreateContext("test ruleset id", BeGuid());

    // verify the data source is cached
    EXPECT_TRUE(IsHierarchyLevelCached(*m_cache, info.first.GetRulesetId().c_str()));

    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), info.first.GetRulesetId().c_str(), BeGuid(), BeGuid()).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevel(info.first.GetId()).IsValid());
    EXPECT_TRUE(m_cache->FindDataSource(DataSourceIdentifier(info.first.GetId(), { 0 }), RulesetVariables()).GetIdentifier().IsValid());

    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(), info.first.GetRulesetId(), BeGuid())).IsValid());
    EXPECT_TRUE(m_cache->GetHierarchyLevel(*context, info.first).IsValid());
    EXPECT_TRUE(m_cache->GetDataSource(*context, info.second).IsValid());

    // verify it's not found when looking with invalid parameters
    EXPECT_FALSE(IsHierarchyLevelCached(*m_cache, "invalid"));
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(), "invalid", BeGuid())).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsDifferentCachedRootDataSourceForDifferentVariables)
    {
    RulesetVariables variables1({ RulesetVariableEntry("var_id", rapidjson::Value("value1")) });
    RulesetVariables variables2({ RulesetVariableEntry("var_id", rapidjson::Value("value2")) });

    // cache root data source
    auto info1 = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid(), true, variables1);
    // cache second root data source with different variables
    auto info2 = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid(), true, variables2);

    // verify that data sources where cached for same hierarchy level
    EXPECT_TRUE(info1.first == info2.first);
    // verify that data sources are different
    EXPECT_FALSE(info1.second == info2.second);

    // verify the hierarchy level is cached
    EXPECT_TRUE(IsHierarchyLevelCached(*m_cache, info1.first.GetRulesetId().c_str()));
    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), info1.first.GetRulesetId().c_str(), BeGuid(), BeGuid()).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevel(info1.first.GetId()).IsValid());

    // verify that data sources are cached
    DataSourceInfo cachedSource1 = m_cache->FindDataSource(DataSourceIdentifier(info1.first.GetId(), { 0 }), variables1);
    EXPECT_TRUE(cachedSource1.GetIdentifier().IsValid());
    DataSourceInfo cachedSource2 = m_cache->FindDataSource(DataSourceIdentifier(info1.first.GetId(), { 0 }), variables2);
    EXPECT_TRUE(cachedSource2.GetIdentifier().IsValid());
    EXPECT_FALSE(cachedSource1.GetIdentifier() == cachedSource2.GetIdentifier());

    // verify hierarchy level is not found when looking with invalid variables
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info1.first.GetRulesetId(), BeGuid(), RulesetVariables()),
        CombinedHierarchyLevelIdentifier(m_connection->GetId(), info1.first.GetRulesetId(), BeGuid())).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedRootNode)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id");
    auto context = CreateContext(info.first.GetRulesetId(), BeGuid());

    // create a node
    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    InitNode(*node, info.first);

    // the root data source should be empty
    NavNodesProviderPtr cachedProvider = m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(),
        info.first.GetRulesetId(), BeGuid()));
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(0, cachedProvider->GetNodesCount());

    // the node's data source should also be empty
    cachedProvider = m_cache->GetDataSource(*context, node->GetNodeId());
    ASSERT_FALSE(cachedProvider.IsValid());

    // cache the node
    m_cache->Cache(*node, info.second, 0, NodeVisibility::Visible);

    // verify the node is cached
    cachedProvider = m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(),
        info.first.GetRulesetId(), BeGuid()));
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(1, cachedProvider->GetNodesCount());

    EXPECT_TRUE(IsNodeCached(*m_cache, node->GetNodeId()));
    EXPECT_TRUE(m_cache->GetNode(node->GetNodeId()).IsValid());

    // verify node's datasource is not empty
    cachedProvider = m_cache->GetDataSource(*context, node->GetNodeId());
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(1, cachedProvider->GetNodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsDifferentCachedChildDataSourceForDifferentRulesetVariables)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id");
    auto nodes = FillWithNodes(info, 1);
    BeGuid nodeId = nodes[0]->GetNodeId();
    auto context = CreateContext(info.first.GetRulesetId(), nodeId);

    // child data source should not exist
    EXPECT_FALSE(IsHierarchyLevelCached(*m_cache, nodeId));
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(),
        info.first.GetRulesetId(), nodeId)).IsNull());

    // cache child data source
    auto childrenInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", nodeId);

    // verify the data source exists now
    EXPECT_TRUE(IsHierarchyLevelCached(*m_cache, nodeId));
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*context, childrenInfo.first.GetCombined()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedChildDataSource)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id");
    auto nodes = FillWithNodes(info, 1);
    BeGuid nodeId = nodes[0]->GetNodeId();
    auto context = CreateContext(info.first.GetRulesetId(), nodeId);

    // child data source should not exist
    EXPECT_FALSE(IsHierarchyLevelCached(*m_cache, nodeId));
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(),
        info.first.GetRulesetId(), nodeId)).IsNull());

    // two child data sources with different variables
    RulesetVariables variables1({ RulesetVariableEntry("var_id", rapidjson::Value("value1")) });
    RulesetVariables variables2({ RulesetVariableEntry("var_id", rapidjson::Value("value2")) });

    auto childrenInfo1 = CacheDataSource(m_connection->GetId(), "ruleset_id", nodeId, true, variables1);
    auto childrenInfo2 = CacheDataSource(m_connection->GetId(), "ruleset_id", nodeId, true, variables2);
    EXPECT_TRUE(childrenInfo1.first == childrenInfo2.first);
    EXPECT_FALSE(childrenInfo1.second == childrenInfo2.second);

    // verify the data source exists now
    EXPECT_TRUE(IsHierarchyLevelCached(*m_cache, nodeId));

    NavNodesProviderPtr cachedSource1 = m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetRulesetId(), nodeId, variables1), childrenInfo1.first.GetCombined());
    EXPECT_TRUE(cachedSource1.IsValid());

    NavNodesProviderPtr cachedSource2 = m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetRulesetId(), nodeId, variables2), childrenInfo2.first.GetCombined());
    EXPECT_TRUE(cachedSource2.IsValid());
    EXPECT_TRUE(cachedSource1 != cachedSource2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedChildNode)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id");
    auto nodes = FillWithNodes(info, 1);
    BeGuid nodeId = nodes[0]->GetNodeId();

    // cache child data source
    auto childrenInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", nodeId);

    auto context = CreateContext(info.first.GetRulesetId(), nodeId);

    // the data source should be empty
    NavNodesProviderPtr cachedProvider = m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(),
        info.first.GetRulesetId(), nodeId));
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(0, cachedProvider->GetNodesCount());

    // cache child node
    auto childNodes = FillWithNodes(childrenInfo, 1);

    // verify the child node is cached
    cachedProvider = m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(),
        info.first.GetRulesetId(), nodeId));
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(1, cachedProvider->GetNodesCount());

    EXPECT_TRUE(IsNodeCached(*m_cache, childNodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->GetNode(childNodes[0]->GetNodeId()).IsValid());

    // verify we can get its parent data source
    cachedProvider = m_cache->GetDataSource(*context, childNodes[0]->GetNodeId());
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(1, cachedProvider->GetNodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetNodeVisibility_ReturnsPhysicalNodeVisibility)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id");
    auto nodes = FillWithNodes(info, 1);

    // verify results
    EXPECT_EQ(NodeVisibility::Visible, m_cache->GetNodeVisibility(nodes[0]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetNodeVisibility_ReturnsVirtualNodeVisibility)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id");
    auto nodes = FillWithNodes(info, 1, false, true);

    // verify results
    EXPECT_EQ(NodeVisibility::Virtual, m_cache->GetNodeVisibility(nodes[0]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetHierarchyLevel_ReturnsDataSourcesWithValidRulesetIds)
    {
    // cache root data source for the first ruleset
    m_cache->OnRulesetUsed(*PresentationRuleSet::CreateInstance("ruleset_id1"));
    auto info1 = CacheDataSource(m_connection->GetId(), "ruleset_id1", BeGuid());

    // cache root data source for the second ruleset
    m_cache->OnRulesetUsed(*PresentationRuleSet::CreateInstance("ruleset_id2"));
    auto info2 = CacheDataSource(m_connection->GetId(), "ruleset_id2", BeGuid());

    // verify correct sources are returned
    NavNodesProviderPtr cached1 = m_cache->GetHierarchyLevel(*CreateContext(info1.first.GetRulesetId(), BeGuid()), info1.first);
    ASSERT_TRUE(cached1.IsValid());
    EXPECT_STREQ(info1.first.GetRulesetId().c_str(), cached1->GetContext().GetRuleset().GetRuleSetId().c_str());

    NavNodesProviderPtr cached2 = m_cache->GetHierarchyLevel(*CreateContext(info2.first.GetRulesetId(), BeGuid()), info2.first);
    ASSERT_TRUE(cached2.IsValid());
    EXPECT_STREQ(info2.first.GetRulesetId().c_str(), cached2->GetContext().GetRuleset().GetRuleSetId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetHierarchyLevel_ReturnsDataSourcesWithValidVariables)
    {
    RulesetVariables variables1({ RulesetVariableEntry("var_id", rapidjson::Value("value1")) });
    RulesetVariables variables2({ RulesetVariableEntry("var_id", rapidjson::Value("value2")) });

    // cache root data source for the first variables
    auto info1 = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid(), true, variables1);

    // cache root data source for the second variables
    auto info2 = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid(), true, variables2);

    // verify correct sources are returned
    NavNodesProviderPtr cached1 = m_cache->GetHierarchyLevel(*CreateContext(info1.first.GetRulesetId(), BeGuid(), variables1), info1.first);
    ASSERT_TRUE(cached1.IsValid());
    EXPECT_TRUE(variables1 == cached1->GetContext().GetRulesetVariables());

    NavNodesProviderPtr cached2 = m_cache->GetHierarchyLevel(*CreateContext(info2.first.GetRulesetId(), BeGuid(), variables2), info2.first);
    ASSERT_TRUE(cached2.IsValid());
    EXPECT_TRUE(variables2 == cached2->GetContext().GetRulesetVariables());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECInstanceNode)
    {
    // create a node in an open connection
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    bvector<NavNodePtr> nodes = {
        TestNodesHelper::CreateInstanceNode(*m_connection, *widgetClass)
        };
    FillWithNodes(info, nodes);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", *nodes[0]->GetKey(), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when key is invalid
    ECClassInstanceKey invalidKey(
        nodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().front().GetClass(),
        ECInstanceId(nodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().front().GetId().GetValue() + 100));
    locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", *ECInstancesNodeKey::Create(invalidKey, "", { "wrong hash" }), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECInstanceNode_FromCorrectRuleset)
    {
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");

    // create a node with ruleset 1
    m_cache->OnRulesetUsed(*PresentationRuleSet::CreateInstance("ruleset_id_1"));
    auto info1 = CacheDataSource(m_connection->GetId(), "ruleset_id_1", BeGuid());
    NavNodePtr node1 = TestNodesHelper::CreateInstanceNode(*m_connection, *widgetClass);
    NavNodeExtendedData(*node1).SetRulesetId(info1.first.GetRulesetId().c_str());
    FillWithNodes(info1, { node1 });

    // create a node with ruleset 2
    m_cache->OnRulesetUsed(*PresentationRuleSet::CreateInstance("ruleset_id_2"));
    auto info2 = CacheDataSource(m_connection->GetId(), "ruleset_id_2", BeGuid());
    NavNodePtr node2 = TestNodesHelper::CreateInstanceNode(*m_connection, *widgetClass);
    NavNodeExtendedData(*node2).SetRulesetId(info2.first.GetRulesetId().c_str());
    FillWithNodes(info2, { node2 });

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id_1", *node1->GetKey(), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsValid());
    EXPECT_TRUE(node1->Equals(*locatedNode));

    locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id_2", *node2->GetKey(), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsValid());
    EXPECT_TRUE(node2->Equals(*locatedNode));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECInstanceNode_ForCorrectRulesetVariables)
    {
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");

    // create a node with ruleset variables 1
    RulesetVariables variables1({ RulesetVariableEntry("var_id", rapidjson::Value("value1")) });
    auto info1 = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid(), true, variables1);
    NavNodePtr node1 = TestNodesHelper::CreateInstanceNode(*m_connection, *widgetClass);
    FillWithNodes(info1, { node1 });

    // create a node with ruleset 2
    RulesetVariables variables2({ RulesetVariableEntry("var_id", rapidjson::Value("value2")) });
    auto info2 = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid(), true, variables2);
    NavNodePtr node2 = TestNodesHelper::CreateInstanceNode(*m_connection, *widgetClass);
    FillWithNodes(info2, { node2 });

    // verify the node is found successfully with valid key and variables
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", *node1->GetKey(), variables1);
    ASSERT_TRUE(locatedNode.IsValid());
    EXPECT_TRUE(node1->Equals(*locatedNode));

    // verify the node is not found with invalid variables
    locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", *node1->GetKey(), RulesetVariables());
    EXPECT_FALSE(locatedNode.IsValid());

    // verify the second node is found successfully with valid key and variables
    locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", *node2->GetKey(), variables2);
    ASSERT_TRUE(locatedNode.IsValid());
    EXPECT_TRUE(node2->Equals(*locatedNode));

    // verify the second node is not found with invalid variables
    locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", *node2->GetKey(), RulesetVariables());
    EXPECT_FALSE(locatedNode.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECClassGroupingNode)
    {
    // create a node in an open connection
    ECClassCP widgetClass1 = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    bvector<NavNodePtr> nodes = {
        TestNodesHelper::CreateClassGroupingNode(*m_connection, *widgetClass1, "test label")
        };
    FillWithNodes(info, nodes);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", *nodes[0]->GetKey(), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when node hash doesnt match
    locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", *ECClassGroupingNodeKey::Create(*widgetClass1, false, "", {"different hash"}, 1), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECPropertyValueGroupingNode)
    {
    // create a node in an open connection
    ECClassCP widgetClass1 = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECPropertyCP groupingProperty1 = widgetClass1->GetPropertyP("IntProperty");
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    bvector<NavNodePtr> nodes = {
        TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *widgetClass1, *groupingProperty1, "test label", {ECValue(9)}, false)
        };
    FillWithNodes(info, nodes);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", *nodes[0]->GetKey(), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when node hash doesnt match
    locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", *ECPropertyGroupingNodeKey::Create(*widgetClass1, "IntProperty",
        nodes[0]->GetKey()->AsECPropertyGroupingNodeKey()->GetGroupingValuesArray(), "", {"different hash"}, 1), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECPropertyRangeGroupingNode)
    {
    // create a node in an open connection
    ECClassCP widgetClass1 = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECPropertyCP groupingProperty1 = widgetClass1->GetPropertyP("IntProperty");
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    bvector<NavNodePtr> nodes = {
        TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *widgetClass1, *groupingProperty1, "test label", {ECValue(2)}, true)
        };
    FillWithNodes(info, nodes);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", *nodes[0]->GetKey(), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when node hash doesnt match
    locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", *ECPropertyGroupingNodeKey::Create(*widgetClass1, "IntProperty",
        nodes[0]->GetKey()->AsECPropertyGroupingNodeKey()->GetGroupingValuesArray(), "", {"different hash"}, 1), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesLabelGroupingNode)
    {
    // create a node in an open connection
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    bvector<NavNodePtr> nodes = {
        TestNodesHelper::CreateLabelGroupingNode(*m_connection, "test label")
        };
    FillWithNodes(info, nodes);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", *nodes[0]->GetKey(), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when node hash doesnt match
    locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", *NavNodeKey::Create("type", "", {"wrong hash"}), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesCustomNode)
    {
    // create a node in an open connection
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    bvector<NavNodePtr> nodes = {
        TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test label", "test descr")
        };
    FillWithNodes(info, nodes);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", *nodes[0]->GetKey(), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when node hash doesnt matchSetInstanceId
    locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id", *NavNodeKey::Create("type", "", {"wrong hash"}), RulesetVariables());
    ASSERT_TRUE(locatedNode.IsNull());
    }

#ifdef wip_ruleset_variables
/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Settings_ReturnsEmptyListWhenRulesetAndSettingIdsAreInvalid)
    {
    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", 0);
    EXPECT_TRUE(IsHierarchyLevelCached(*m_cache, info.first.GetRulesetId().c_str()));

    bvector<HierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels("invalid", "any");
    EXPECT_TRUE(related.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Settings_ReturnsOnlyRelatedDataSources)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", 0);
    auto rootNodes = FillWithNodes(rootInfo, 2);

    // cache unrelated datasource
    auto childInfo0 = CacheDataSource(m_connection->GetId(), "ruleset_id", rootNodes[0]->GetNodeId());
    bvector<UserSettingEntry> settings0 = {UserSettingEntry("setting_0", Json::Value("value0"))};
    m_cache->Update(childInfo0.second, nullptr, nullptr, &settings0);

    // cache related datasource
    auto childInfo1 = CacheDataSource(m_connection->GetId(), "ruleset_id", rootNodes[1]->GetNodeId());
    bvector<UserSettingEntry> settings1 = {UserSettingEntry("setting_1", Json::Value("value1")) };
    m_cache->Update(childInfo1.second, nullptr, nullptr, &settings1);

    // verify the correct datasource is found
    bvector<HierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels("ruleset_id", "setting_1");
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(childInfo1.first, related[0]);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsEmptyListWhenInstancesArentRelated)
    {
    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    EXPECT_TRUE(IsHierarchyLevelCached(*m_cache, info.first.GetRulesetId().c_str()));

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
    bset<CombinedHierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    EXPECT_TRUE(related.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenClassMatchesAndFilterIsNotSpecified)
    {
    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[ECClassId((uint64_t)1)] = false;
    DataSourceInfo dsInfo(info.second, RulesetVariables(), DataSourceFilter(), usedClassIds, "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_RelatedClasses);
    EXPECT_TRUE(IsHierarchyLevelCached(*m_cache, info.first.GetRulesetId().c_str()));

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
    bset<CombinedHierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    ASSERT_EQ(1, related.size());
    EXPECT_TRUE(related.end() != related.find(info.first.GetCombined()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsEmptyListWhenClassMatchesButFilterDoesnt)
    {
    ECClassCP widgetHasGadget = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    ECClassId usedClassId((uint64_t)1);
    ECInstanceId usedInstanceId((uint64_t)1);

    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    bvector<ECClassId> relationshipIds = {widgetHasGadget->GetId()};
    DataSourceFilter filter(std::make_unique<DataSourceFilter::RelatedInstanceInfo>(relationshipIds, RequiredRelationDirection_Backward, bvector<ECInstanceKey>{ECInstanceKey(usedClassId, usedInstanceId)}), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[usedClassId] = false;
    DataSourceInfo dsInfo(info.second, RulesetVariables(), filter, usedClassIds, "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_RelatedClasses | DataSourceInfo::PART_Filter);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(usedClassId, ECInstanceId((uint64_t)456)));
    bset<CombinedHierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    EXPECT_TRUE(related.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenClassAndFilterMatchesByInstanceId)
    {
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECClassCP gadgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Gadget");
    ECClassCP widgetHasGadgetRelationship = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    ECClassId usedClassId(gadgetClass->GetId());
    ECInstanceId usedInstanceId((uint64_t)123);

    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    bvector<ECClassId> relationshipIds = {widgetHasGadgetRelationship->GetId()};
    DataSourceFilter filter(std::make_unique<DataSourceFilter::RelatedInstanceInfo>(relationshipIds, RequiredRelationDirection_Backward, bvector<ECInstanceKey>{ECInstanceKey(usedClassId, usedInstanceId)}), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[widgetClass->GetId()] = false;
    usedClassIds[gadgetClass->GetId()] = false;
    DataSourceInfo dsInfo(info.second, RulesetVariables(), filter, usedClassIds, "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_RelatedClasses | DataSourceInfo::PART_Filter);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(usedClassId, usedInstanceId));
    bset<CombinedHierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    ASSERT_EQ(1, related.size());
    EXPECT_TRUE(related.end() != related.find(info.first.GetCombined()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenClassAndFilterMatchesByForwardRelatedInstances)
    {
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECClassCP gadgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Gadget");
    ECClassCP widgetHasGadgetRelationship = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    // set up
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetRelationship->GetRelationshipClassCP(), *widget, *gadget);

    ECInstanceId widgetId, gadgetId;
    ECInstanceId::FromString(widgetId, widget->GetInstanceId().c_str());
    ECInstanceId::FromString(gadgetId, gadget->GetInstanceId().c_str());

    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    bvector<ECClassId> relationshipIds = {widgetHasGadgetRelationship->GetId()};
    DataSourceFilter filter(std::make_unique<DataSourceFilter::RelatedInstanceInfo>(relationshipIds, RequiredRelationDirection_Forward, bvector<ECInstanceKey>{ECInstanceKey(widgetClass->GetId(), widgetId)}), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[widgetClass->GetId()] = false;
    usedClassIds[gadgetClass->GetId()] = false;
    DataSourceInfo dsInfo(info.second, RulesetVariables(), filter, usedClassIds, "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_RelatedClasses | DataSourceInfo::PART_Filter);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(gadgetClass->GetId(), gadgetId));
    bset<CombinedHierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    ASSERT_EQ(1, related.size());
    EXPECT_TRUE(related.end() != related.find(info.first.GetCombined()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenClassAndFilterMatchesByBackwardRelatedInstances)
    {
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECClassCP gadgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Gadget");
    ECClassCP widgetHasGadgetRelationship = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    // set up
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetRelationship->GetRelationshipClassCP(), *widget, *gadget);

    ECInstanceId widgetId, gadgetId;
    ECInstanceId::FromString(widgetId, widget->GetInstanceId().c_str());
    ECInstanceId::FromString(gadgetId, gadget->GetInstanceId().c_str());

    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    bvector<ECClassId> relationshipIds = {widgetHasGadgetRelationship->GetId()};
    DataSourceFilter filter(std::make_unique<DataSourceFilter::RelatedInstanceInfo>(relationshipIds, RequiredRelationDirection_Backward, bvector<ECInstanceKey>{ECInstanceKey(gadgetClass->GetId(), gadgetId)}), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[widgetClass->GetId()] = false;
    usedClassIds[gadgetClass->GetId()] = false;
    DataSourceInfo dsInfo(info.second, RulesetVariables(), filter, usedClassIds, "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_RelatedClasses | DataSourceInfo::PART_Filter);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(widgetClass->GetId(), widgetId));
    bset<CombinedHierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    ASSERT_EQ(1, related.size());
    EXPECT_TRUE(related.end() != related.find(info.first.GetCombined()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenClassAndFilterMatchesByAnyDirectionRelatedInstances)
    {
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECClassCP gadgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Gadget");
    ECClassCP widgetHasGadgetRelationship = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    // set up
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetRelationship->GetRelationshipClassCP(), *widget, *gadget);

    ECInstanceId widgetId, gadgetId;
    ECInstanceId::FromString(widgetId, widget->GetInstanceId().c_str());
    ECInstanceId::FromString(gadgetId, gadget->GetInstanceId().c_str());

    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    bvector<ECClassId> relationshipIds = {widgetHasGadgetRelationship->GetId()};
    DataSourceFilter filter(std::make_unique<DataSourceFilter::RelatedInstanceInfo>(relationshipIds, RequiredRelationDirection_Both, bvector<ECInstanceKey>{ECInstanceKey(gadgetClass->GetId(), gadgetId)}), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[widgetClass->GetId()] = false;
    usedClassIds[gadgetClass->GetId()] = false;
    DataSourceInfo dsInfo(info.second, RulesetVariables(), filter, usedClassIds, "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_RelatedClasses | DataSourceInfo::PART_Filter);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(widgetClass->GetId(), widgetId));
    bset<CombinedHierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    ASSERT_EQ(1, related.size());
    EXPECT_TRUE(related.end() != related.find(info.first.GetCombined()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenClassAndFilterMatchesByRelatedInstancesFollowingMultipleRelationships)
    {
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECClassCP gadgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Gadget");
    ECClassCP widgetHasGadgetRelationship = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");
    ECClassCP widgetHasGadgetsRelationship = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadgets");

    // set up
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(s_project->GetECDb(), *widgetHasGadgetRelationship->GetRelationshipClassCP(), *widget, *gadget);

    ECInstanceId widgetId, gadgetId;
    ECInstanceId::FromString(widgetId, widget->GetInstanceId().c_str());
    ECInstanceId::FromString(gadgetId, gadget->GetInstanceId().c_str());

    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    bvector<ECClassId> relationshipIds = {widgetHasGadgetRelationship->GetId(), widgetHasGadgetsRelationship->GetId()};
    DataSourceFilter filter(std::make_unique<DataSourceFilter::RelatedInstanceInfo>(relationshipIds, RequiredRelationDirection_Backward, bvector<ECInstanceKey>{ECInstanceKey(gadgetClass->GetId(), gadgetId)}), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[widgetClass->GetId()] = false;
    usedClassIds[gadgetClass->GetId()] = false;
    DataSourceInfo dsInfo(info.second, RulesetVariables(), filter, usedClassIds, "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_RelatedClasses | DataSourceInfo::PART_Filter);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(widgetClass->GetId(), widgetId));
    bset<CombinedHierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    ASSERT_EQ(1, related.size());
    EXPECT_TRUE(related.end() != related.find(info.first.GetCombined()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsEmptyListWhenChildNodeGroupedInstanceKeysDontMatchByClassId)
    {
    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());

    // create a node
    ECClassCP nodeClass = GetDb().Schemas().GetClass("ECDbMeta", "ECClassDef");
    auto node = TestNodesHelper::CreateInstanceNode(*m_connection, *nodeClass, ECInstanceId((uint64_t)2));
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId("ruleset_id");
    m_cache->Cache(*node, info.second, 0, NodeVisibility::Visible);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId(nodeClass->GetId().GetValue() + 1), ECInstanceId((uint64_t)2)));
    bset<CombinedHierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    EXPECT_TRUE(related.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsEmptyListWhenChildNodeInstanceKeysDontMatchByInstanceId)
    {
    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());

    // create a node
    ECClassCP nodeClass = GetDb().Schemas().GetClass("ECDbMeta", "ECClassDef");
    auto node = TestNodesHelper::CreateInstanceNode(*m_connection, *nodeClass, ECInstanceId((uint64_t)2));
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId("ruleset_id");
    m_cache->Cache(*node, info.second, 0, NodeVisibility::Visible);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(nodeClass->GetId(), ECInstanceId((uint64_t)1)));
    bset<CombinedHierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    EXPECT_TRUE(related.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenChildNodeGroupedInstanceKeysMatch)
    {
    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());

    // create a node
    ECClassCP nodeClass = GetDb().Schemas().GetClass("ECDbMeta", "ECClassDef");
    auto node = TestNodesHelper::CreateInstanceNode(*m_connection, *nodeClass, ECInstanceId((uint64_t)2));
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId("ruleset_id");
    m_cache->Cache(*node, info.second, 0, NodeVisibility::Visible);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(nodeClass->GetId(), ECInstanceId((uint64_t)2)));
    bset<CombinedHierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    ASSERT_EQ(1, related.size());
    EXPECT_TRUE(related.end() != related.find(info.first.GetCombined()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenChildNodeGroupedInstanceKeysMatchButFilterDoesnt)
    {
    ECClassCP nodeClass = GetDb().Schemas().GetClass("ECDbMeta", "ECClassDef");
    ECClassCP nodeRelationship = GetDb().Schemas().GetClass("ECDbMeta", "SchemaOwnsClasses");

    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    bvector<ECClassId> relationshipIds = { nodeRelationship->GetId() };
    DataSourceFilter filter(std::make_unique<DataSourceFilter::RelatedInstanceInfo>(relationshipIds, RequiredRelationDirection_Backward, bvector<ECInstanceKey>{ECInstanceKey(nodeClass->GetId(), ECInstanceId((uint64_t)123))}), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[nodeClass->GetId()] = false;
    DataSourceInfo dsInfo(info.second, RulesetVariables(), filter, usedClassIds, "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_RelatedClasses | DataSourceInfo::PART_Filter);

    // create a node
    auto node = TestNodesHelper::CreateInstanceNode(*m_connection, *nodeClass, ECInstanceId((uint64_t)2));
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId("ruleset_id");
    m_cache->Cache(*node, info.second, 0, NodeVisibility::Visible);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(nodeClass->GetId(), ECInstanceId((uint64_t)2)));
    bset<CombinedHierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(*m_connection, keys);
    ASSERT_EQ(1, related.size());
    EXPECT_TRUE(related.end() != related.find(info.first.GetCombined()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetCombinedHierarchyLevel_ReturnsInvalidRootDatasourceForDifferentRulesetVariables)
    {
    RulesetVariables relatedVariables({ RulesetVariableEntry("variable_id", rapidjson::Value("value0"))});
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    DataSourceInfo dsInfo(info.second, relatedVariables, DataSourceFilter(), bmap<ECClassId, bool>(), "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_Vars);

    // verify that valid data source is returned for matching ruleset variables
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined(), relatedVariables), info.first.GetCombined()).IsValid());

    // verify what valid datasource is not returned with different ruleset variables
    RulesetVariables differentVariables({ RulesetVariableEntry("variable_id", rapidjson::Value("value1")) });
    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined(), differentVariables), info.first.GetCombined()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetCombinedHierarchyLevel_ReturnsInvalidChildDatasourceCacheForDifferentRulesetVariables)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    auto rootNodes = FillWithNodes(rootInfo, 1);

    // cache child data source
    RulesetVariables relatedVariables({ RulesetVariableEntry("variable_id", rapidjson::Value(1))});
    auto childInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", rootNodes[0]->GetNodeId());
    DataSourceInfo dsInfo(childInfo.second, relatedVariables, DataSourceFilter(), bmap<ECClassId, bool>(), "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_Vars);

    // verify that valid data source is returned for matching ruleset variables
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(childInfo.first.GetCombined(), relatedVariables), childInfo.first.GetCombined()).IsValid());

    // verify what valid datasource is not returned with different ruleset variables
    RulesetVariables differentVariables({ RulesetVariableEntry("variable_id", rapidjson::Value(10)) });
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(childInfo.first.GetCombined(), differentVariables), childInfo.first.GetCombined()).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetCombinedHierarchyLevel_ReturnsInvalidRootProviderWhenMultipleDataSourcesCachedAndVariablesAreDifferent)
    {
    RulesetVariables relatedVariables({ RulesetVariableEntry("variable_id", rapidjson::Value("value0")) });
    // cache data source without related variables
    CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());

    // cache data source with related variables for the same level
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid(), true, relatedVariables);

    // verify that valid data source is returned for matching ruleset variables
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined(), relatedVariables), info.first.GetCombined()).IsValid());

    // verify that valid data source is returned for empty variables
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined()), info.first.GetCombined()).IsValid());

    // verify what valid datasource is not returned with different ruleset variables
    RulesetVariables differentVariables({ RulesetVariableEntry("variable_id", rapidjson::Value("value1")) });
    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined(), differentVariables), info.first.GetCombined()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetCombinedHierarchyLevel_ReturnsInvalidChildProviderWhenMultipleDataSourcesCachedAndVariablesAreDifferent)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    auto rootNodes = FillWithNodes(rootInfo, 1);

    // cache child data source
    RulesetVariables relatedVariables({ RulesetVariableEntry("variable_id", rapidjson::Value(1)) });

    // cache data source without related variables
    CacheDataSource(m_connection->GetId(), "ruleset_id", rootNodes[0]->GetNodeId());

    // cache data source with related variables for the same hierarchy level
    auto childInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", rootNodes[0]->GetNodeId(), true, relatedVariables);

    // verify that valid data source is returned for matching ruleset variables
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(childInfo.first.GetCombined(), relatedVariables), childInfo.first.GetCombined()).IsValid());

    // verify that valid data source is returned for empty ruleset variables
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(childInfo.first.GetCombined()), childInfo.first.GetCombined()).IsValid());

    // verify what valid datasource is not returned with different ruleset variables
    RulesetVariables differentVariables({ RulesetVariableEntry("variable_id", rapidjson::Value(10)) });
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(childInfo.first.GetCombined(), differentVariables), childInfo.first.GetCombined()).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetHierarchyLevel_ReturnsInvalidRootDatasourceForDIfferentRulesetVariables)
    {
    RulesetVariables relatedVariables({ RulesetVariableEntry("variable_id", rapidjson::Value("value0"))});
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    DataSourceInfo dsInfo(info.second, relatedVariables, DataSourceFilter(), bmap<ECClassId, bool>(), "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_Vars);

    // verify that valid data source is returned for matching ruleset variables
    EXPECT_TRUE(m_cache->GetHierarchyLevel(*CreateContext(info.first.GetCombined(), relatedVariables), info.first).IsValid());

    // verify what valid datasource is not returned with different ruleset variables
    RulesetVariables differentVariables({ RulesetVariableEntry("variable_id", rapidjson::Value("value1")) });
    EXPECT_FALSE(m_cache->GetHierarchyLevel(*CreateContext(info.first.GetCombined(), differentVariables), info.first).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetHierarchyLevel_ReturnsInvalidChildDatasourceForDifferentRulesetVariables)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    auto rootNodes = FillWithNodes(rootInfo, 1);

    // cache child data source
    RulesetVariables relatedVariables({ RulesetVariableEntry("variable_id", rapidjson::Value(1))});
    auto childInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", rootNodes[0]->GetNodeId());
    DataSourceInfo dsInfo(childInfo.second, relatedVariables, DataSourceFilter(), bmap<ECClassId, bool>(), "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_Vars);

    // verify that valid data source is returned for matching ruleset variables
    EXPECT_TRUE(m_cache->GetHierarchyLevel(*CreateContext(childInfo.first.GetCombined(), relatedVariables), childInfo.first).IsValid());

    // verify what valid datasource is not returned with different ruleset variables
    RulesetVariables differentVariables({ RulesetVariableEntry("variable_id", rapidjson::Value(10)) });
    EXPECT_TRUE(m_cache->GetHierarchyLevel(*CreateContext(childInfo.first.GetCombined(), differentVariables), childInfo.first).IsNull());
    }

#ifdef wip_ruleset_variables
/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetDataSource_ClearsRootDatasourceCacheIfRelatedSettingsValuesChanged)
    {
    m_userSettings.GetSettings("ruleset_id").SetSettingValue("setting_id", "value0");
    bvector<UserSettingEntry> settingsValues = {UserSettingEntry("setting_id", Json::Value("value0"))};
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", 0);
    m_cache->Update(info.second, nullptr, nullptr, &settingsValues);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->GetDataSource(info.second).IsValid());

    // verify that valid datasource is not returned with different user settings
    m_userSettings.GetSettings("ruleset_id").SetSettingValue("setting_id", "value1");

    EXPECT_FALSE(m_cache->GetDataSource(info.second).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetDataSource_DoesntClearRootDatasourceCacheIfRelatedSettingsValuesDidntChange)
    {
    m_userSettings.GetSettings("ruleset_id").SetSettingIntValue("setting_id", 10);
    bvector<UserSettingEntry> settingsValues = {UserSettingEntry("setting_id", Json::Value(10))};
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", 0);
    m_cache->Update(info.second, nullptr, nullptr, &settingsValues);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->GetDataSource(info.second).IsValid());

    // verify what valid datasource is returned with same user settings
    m_userSettings.GetSettings("ruleset_id").SetSettingIntValue("setting_id", 10);

    EXPECT_TRUE(m_cache->GetDataSource(info.second).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetDataSource_ClearsChildDatasourceCacheIfRelatedSettingsValuesChanged)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", 0);
    auto rootNodes = FillWithNodes(rootInfo, 1);

    // cache child data source
    m_userSettings.GetSettings("ruleset_id").SetSettingIntValue("setting_id", 1);
    bvector<UserSettingEntry> settingsValues = {UserSettingEntry("setting_id", Json::Value(1))};
    auto childInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", rootNodes[0]->GetNodeId());
    m_cache->Update(childInfo.second, nullptr, nullptr, &settingsValues);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->GetDataSource(childInfo.second).IsValid());

    // verify what valid datasource is not returned with different user settings
    m_userSettings.GetSettings("ruleset_id").SetSettingIntValue("setting_id", 10);

    EXPECT_TRUE(m_cache->GetDataSource(childInfo.second).IsNull());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Savepoint_DiscardsChangesWhenCanceled)
    {
    auto savepoint = m_cache->CreateSavepoint();

    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    auto nodes = FillWithNodes(info, 1);

    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined()), info.first.GetCombined()).IsValid());
    EXPECT_TRUE(m_cache->GetNode(nodes[0]->GetNodeId()).IsValid());

    savepoint->Cancel();

    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined()), info.first.GetCombined()).IsValid());
    EXPECT_FALSE(m_cache->GetNode(nodes[0]->GetNodeId()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Savepoint_DoesntDiscardChangesWhenNotCanceled)
    {
    auto savepoint = m_cache->CreateSavepoint();

    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    auto nodes = FillWithNodes(info, 1);

    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined()), info.first.GetCombined()).IsValid());
    EXPECT_TRUE(m_cache->GetNode(nodes[0]->GetNodeId()).IsValid());

    savepoint = nullptr;

    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined()), info.first.GetCombined()).IsValid());
    EXPECT_TRUE(m_cache->GetNode(nodes[0]->GetNodeId()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, HierarchyVariationsLimit_DoesNotClearVariationsWhenThresholdIsNotExceeded)
    {
    // cache root hierarchy level
    HierarchyLevelIdentifier info(m_connection->GetId(), "ruleset_id", BeGuid(), BeGuid());
    m_cache->Cache(info);
    ASSERT_TRUE(info.IsValid());

    bvector<DataSourceIdentifier> dataSourceIdentifiers;
    // cache some data sources with different variables not exceeding threshold
    for (uint64_t i = 0; i < NODESCACHE_VARIATIONS_Threshold; ++i)
        {
        RulesetVariables variables({ RulesetVariableEntry("var_id", (int64_t)i) });
        auto cachedDs = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid(), true, variables);
        // check that data source exists
        // GetDataSource does not use RulesetVariables when looking for data source. Using default value to simplify test.
        EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(info.GetCombined()), cachedDs.second).IsValid());
        dataSourceIdentifiers.push_back(cachedDs.second);
        BeThreadUtilities::BeSleep(5); // sleep for 5 milliseconds to make sure the "last used" time differs
        }

    // check that all data sources still exists
    for (uint64_t i = 0; i < dataSourceIdentifiers.size(); ++i)
        EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(info.GetCombined()), dataSourceIdentifiers[i]).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, HierarchyVariationsLimit_ClearVariationsWhenThresholdIsExceeded)
    {
    // cache root hierarchy level
    HierarchyLevelIdentifier info(m_connection->GetId(), "ruleset_id", BeGuid(), BeGuid());
    m_cache->Cache(info);
    ASSERT_TRUE(info.IsValid());

    bvector<DataSourceIdentifier> dataSourceIdentifiers;
    // cache some data sources with different ruleset variables
    for (uint64_t i = 0; i < NODESCACHE_VARIATIONS_Threshold; ++i)
        {
        RulesetVariables variables({ RulesetVariableEntry("var_id", (int64_t)i) });
        auto cachedDs = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid(), true, variables);
        // check that data source exists
        // GetDataSource does not use RulesetVariables when looking for data source. Using default value to simplify test.
        EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(info.GetCombined()), cachedDs.second).IsValid());
        dataSourceIdentifiers.push_back(cachedDs.second);
        BeThreadUtilities::BeSleep(5); // sleep for 5 milliseconds to make sure the "last used" time differs
        }

    // check that data sources with oldest variables still exist
    EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(info.GetCombined()), dataSourceIdentifiers[0]).IsValid());

    // cache one more data source
    RulesetVariables variables({ RulesetVariableEntry("var_id", (int64_t)NODESCACHE_VARIATIONS_Threshold) });
    auto newDs = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid(), true, variables);
    EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(info.GetCombined()), newDs.second).IsValid());

    // check that data source with oldest variables is removed
    EXPECT_FALSE(m_cache->GetDataSource(*CreateContext(info.GetCombined()), dataSourceIdentifiers[0]).IsValid());

    // check that all other data sources exist
    for (uint64_t i = 1; i < dataSourceIdentifiers.size(); ++i)
        EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(info.GetCombined()), dataSourceIdentifiers[i]).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, HierarchyVariationsLimit_ClearsVariationWhenThresholdIsExceededAndVariationHaveMultipleDataSources)
    {
    // cache root hierarchy level
    HierarchyLevelIdentifier info(m_connection->GetId(), "ruleset_id", BeGuid(), BeGuid());
    m_cache->Cache(info);
    ASSERT_TRUE(info.IsValid());

    bvector<DataSourceIdentifier> dataSourceIdentifiers;
    // cache some data sources with different ruleset variables
    for (int64_t i = 0; i < NODESCACHE_VARIATIONS_Threshold; ++i)
        {
        RulesetVariables variables({ RulesetVariableEntry("var_id", (int64_t)i) });
        auto cachedDs1 = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid(), true, variables);
        auto cachedDs2 = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid(), true, variables);
        // check that data source exists
        // GetDataSource does not use RulesetVariables when looking for data source. Using default value to simplify test.
        EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(info.GetCombined()), cachedDs1.second).IsValid());
        EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(info.GetCombined()), cachedDs2.second).IsValid());
        dataSourceIdentifiers.push_back(cachedDs1.second);
        dataSourceIdentifiers.push_back(cachedDs2.second);
        BeThreadUtilities::BeSleep(5); // sleep for 5 milliseconds to make sure the "last used" time differs
        }

    // check that data sources with oldest variables still exist
    EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(info.GetCombined()), dataSourceIdentifiers[0]).IsValid());
    EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(info.GetCombined()), dataSourceIdentifiers[1]).IsValid());

    // cache one more data source
    RulesetVariables variables({ RulesetVariableEntry("var_id", (int64_t)NODESCACHE_VARIATIONS_Threshold) });
    auto newDs = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid(), true, variables);
    EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(info.GetCombined()), newDs.second).IsValid());

    // check that data sources with oldest variables are removed
    EXPECT_FALSE(m_cache->GetDataSource(*CreateContext(info.GetCombined()), dataSourceIdentifiers[0]).IsValid());
    EXPECT_FALSE(m_cache->GetDataSource(*CreateContext(info.GetCombined()), dataSourceIdentifiers[1]).IsValid());

    // check that all other data sources exist
    for (uint64_t i = 2; i < dataSourceIdentifiers.size(); ++i)
        {
        EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(info.GetCombined()), dataSourceIdentifiers[i]).IsValid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, HierarchyVariationsLimit_ClearsVariationDatasourceAndChildrenDataSourcesWhenThresholdIsExceeded)
    {
    // cache root hierarchy level
    HierarchyLevelIdentifier info(m_connection->GetId(), "ruleset_id", BeGuid(), BeGuid());
    m_cache->Cache(info);
    ASSERT_TRUE(info.IsValid());

    RulesetVariables dsWithChildrenVariables;
    dsWithChildrenVariables.SetIntValue("var_id", NODESCACHE_VARIATIONS_Threshold + 1);

    auto dsWithChildren = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid(), true, dsWithChildrenVariables);
    auto nodes = FillWithNodes(dsWithChildren, 1, false, false);
    RulesetVariables childrenVariables(dsWithChildrenVariables);
    childrenVariables.SetStringValue("child_var_id", "value");
    auto childrenDs = CacheDataSource(m_connection->GetId(), "ruleset_id", nodes[0]->GetNodeId(), true, childrenVariables);

    // check that data sources exist
    // GetDataSource does not use RulesetVariables when looking for data source. Using default value to simplify test.
    EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(info.GetCombined()), dsWithChildren.second).IsValid());
    EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(info.GetCombined()), childrenDs.second).IsValid());

    // cache some more data sources with different ruleset variables to exceed variations threshold
    for (uint64_t i = 0; i < NODESCACHE_VARIATIONS_Threshold; ++i)
        {
        BeThreadUtilities::BeSleep(5); // sleep for 5 milliseconds to make sure the "last used" time differs
        RulesetVariables variables({ RulesetVariableEntry("var_id", (int64_t)i) });
        auto cachedDs = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid(), true, variables);
        // check that data sources exists
        EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(info.GetCombined()), cachedDs.second).IsValid());
        }

    // check that data sources with children and children data sources are removed
    EXPECT_FALSE(m_cache->GetDataSource(*CreateContext(info.GetCombined()), dsWithChildren.second).IsValid());
    EXPECT_FALSE(m_cache->GetDataSource(*CreateContext(info.GetCombined()), childrenDs.second).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, HierarchyVariationsLimit_DoesNotClearDataSourceFromDifferentHierarchyLevel)
    {
    RulesetVariables variables({ RulesetVariableEntry("var_id", (int64_t)0) });
    // create second ruleset
    m_cache->OnRulesetUsed(*PresentationRuleSet::CreateInstance("ruleset_id2"));

    // cache data source
    auto firstHierarchyLevel = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid(), true, variables);
    // GetDataSource does not use RulesetVariables when looking for data source. Using default value to simplify test.
    EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(firstHierarchyLevel.first.GetCombined()), firstHierarchyLevel.second).IsValid());

    // cache data source in differnt hierarchy level with same variables
    auto secondHierarchyLevel = CacheDataSource(m_connection->GetId(), "ruleset_id2", BeGuid(), true, variables);
    EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(secondHierarchyLevel.first.GetCombined()), secondHierarchyLevel.second).IsValid());

    BeThreadUtilities::BeSleep(5); // sleep for 5 milliseconds before caching more data sources to make sure the "last used" time differs
    // cache some more data source with different variables to reach threshold in first hierarchy level
    for (uint64_t i = 1; i < NODESCACHE_VARIATIONS_Threshold; ++i)
        {
        variables.SetIntValue("var_id", i);
        auto cachedDs = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid(), true, variables);
        // check that data source exists
        EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(cachedDs.first.GetCombined()), cachedDs.second).IsValid());
        BeThreadUtilities::BeSleep(5); // sleep for 5 milliseconds to make sure the "last used" time differs
        }

    // check that data source with oldest variables still exist
    EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(firstHierarchyLevel.first.GetCombined()), firstHierarchyLevel.second).IsValid());
    // check that data source with same variables in different hierarchy level still exists
    EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(secondHierarchyLevel.first.GetCombined()), secondHierarchyLevel.second).IsValid());

    // cache one more data source
    variables.SetIntValue("var_id", NODESCACHE_VARIATIONS_Threshold);
    auto newDs = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid(), true, variables);
    EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(newDs.first.GetCombined()), newDs.second).IsValid());

    // check that data source with oldest variables from first hierarchy level is removed
    EXPECT_FALSE(m_cache->GetDataSource(*CreateContext(firstHierarchyLevel.first.GetCombined()), firstHierarchyLevel.second).IsValid());
    // check that data source with same variables in different hierarchy level still exists
    EXPECT_TRUE(m_cache->GetDataSource(*CreateContext(secondHierarchyLevel.first.GetCombined()), secondHierarchyLevel.second).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, FindPhysicalHierarchyLevelId_ReturnsPhysicalHierarchyLevelId)
    {
    CombinedHierarchyLevelIdentifier rootInfo(m_connection->GetId(), "ruleset_id", BeGuid());

    EXPECT_FALSE(m_cache->FindPhysicalHierarchyLevelId(rootInfo).IsValid());
    auto rootLevelInfo = CacheDataSource(m_connection->GetId(), "ruleset_id");
    auto rootNode = FillWithNodes(rootLevelInfo, 1, false, false);
    EXPECT_TRUE(m_cache->FindPhysicalHierarchyLevelId(rootInfo).IsValid());

    CombinedHierarchyLevelIdentifier childInfo(m_connection->GetId(), "ruleset_id", rootNode[0]->GetNodeId());
    EXPECT_FALSE(m_cache->FindPhysicalHierarchyLevelId(childInfo).IsValid());
    auto childLevelInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", rootNode[0]->GetNodeId());
    EXPECT_TRUE(m_cache->FindPhysicalHierarchyLevelId(childInfo).IsValid());
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DiskNodesCacheTests : NodesCacheTests
    {
    virtual std::shared_ptr<NodesCache> _CreateNodesCache(IConnectionR connection, BeFileNameCR tempDir) override
        {
        return NodesCache::Create(connection, tempDir, m_nodesFactory, m_nodesProviderContextFactory, m_providersFactory, NodesCacheType::Disk);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, CreatesTempCacheWhenTryingToOpenInvalidLockedCacheDb)
    {
    BeFileName cachePath(m_connection->GetDb().GetDbFileName());
    cachePath.append(L"-hierarchies");
    cachePath.BeDeleteFile();

    BeFile fileLock;
    ASSERT_EQ(BeFileStatus::Success, fileLock.Create(cachePath.c_str()));

    auto cache = _CreateNodesCache(*m_connection, BeFileName());
    ASSERT_TRUE(cache != nullptr);
    EXPECT_STRNE(cachePath.GetNameUtf8().c_str(), cache->GetDb().GetDbFileName());
    cache = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
#if !defined (BENTLEYCONFIG_OS_LINUX) // informed Kyle/Caleb/Gintaras 9/28/18
TEST_F(DiskNodesCacheTests, ShareCachedHierarchiesBetweenSessions)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    auto nodes = FillWithNodes(info, 2, true);

    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined()), info.first.GetCombined()).IsValid());
    EXPECT_TRUE(IsHierarchyLevelCached(*m_cache, nodes[0]->GetNodeId()));
    EXPECT_TRUE(IsHierarchyLevelCached(*m_cache, nodes[1]->GetNodeId()));

    // close cache
    m_cache = nullptr;

    // open cache
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    m_cache = NodesCache::Create(*m_connection, tempDir, m_nodesFactory, m_nodesProviderContextFactory, m_providersFactory, NodesCacheType::Disk, true);
    m_nodesProviderContextFactory.SetNodesCache(m_cache);

    // cache should not be cleaned
    CombinedHierarchyLevelIdentifier updatedInfo(m_connection->GetId(), info.first.GetRulesetId(), BeGuid());
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(updatedInfo), updatedInfo).IsValid());
    EXPECT_TRUE(IsHierarchyLevelCached(*m_cache, nodes[0]->GetNodeId()));
    EXPECT_TRUE(IsHierarchyLevelCached(*m_cache, nodes[1]->GetNodeId()));
    }
#endif

/*---------------------------------------------------------------------------------**//**
* Test situation when app is closed, ruleset is modified and app is turned on.
* In this case OnRulesetDisposed isn't called and only OnRulesetCreated is called.
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, ClearSharedCacheIfRulesetWasModified)
    {
    // create ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("TestRuleset");
    ruleset->AddPresentationRule(*new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false));
    m_cache->OnRulesetUsed(*ruleset);

    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), ruleset->GetRuleSetId(), BeGuid());

    // cache some nodes
    auto nodes = FillWithNodes(info, 2, true);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined()), info.first.GetCombined()).IsValid());
    EXPECT_TRUE(IsHierarchyLevelCached(*m_cache, nodes[0]->GetNodeId()));
    EXPECT_TRUE(IsHierarchyLevelCached(*m_cache, nodes[1]->GetNodeId()));

    // mock new session: app is turned on with modified ruleset and OnRulesetCreated is called
    ruleset->AddPresentationRule(*new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree));
    m_cache->OnRulesetUsed(*ruleset);

    // verify cache is cleared
    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined()), info.first.GetCombined()).IsValid());
    EXPECT_FALSE(IsHierarchyLevelCached(*m_cache, nodes[0]->GetNodeId()));
    EXPECT_FALSE(IsHierarchyLevelCached(*m_cache, nodes[1]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, ClearsCacheIfCacheFileSizeExceedsLimit)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());

    // cache some nodes
    auto nodes = FillWithNodes(info, 2, true);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined()), info.first.GetCombined()).IsValid());
    EXPECT_TRUE(IsHierarchyLevelCached(*m_cache, nodes[0]->GetNodeId()));
    EXPECT_TRUE(IsHierarchyLevelCached(*m_cache, nodes[1]->GetNodeId()));

    // set cache limit
    m_cache->SetCacheFileSizeLimit(1);

    // close cache
    m_nodesProviderContextFactory.SetNodesCache(nullptr);
    m_cache = nullptr;

    // open cache
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    m_cache = NodesCache::Create(*m_connection, tempDir, m_nodesFactory, m_nodesProviderContextFactory, m_providersFactory, NodesCacheType::Disk, true);
    m_nodesProviderContextFactory.SetNodesCache(m_cache);

    // verify cache is cleared
    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined()), info.first.GetCombined()).IsValid());
    EXPECT_FALSE(IsHierarchyLevelCached(*m_cache, nodes[0]->GetNodeId()));
    EXPECT_FALSE(IsHierarchyLevelCached(*m_cache, nodes[1]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, ClearsOldestRulesetCacheWhenCacheFileSizeExceedsLimit)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    FillWithNodes(info, 30, true);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined()), info.first.GetCombined()).IsValid());

    // create second ruleset and cache root data source
    m_cache->OnRulesetUsed(*PresentationRuleSet::CreateInstance("ruleset_id2"));

    auto info2 = CacheDataSource(m_connection->GetId(), "ruleset_id2", BeGuid());
    FillWithNodes(info2, 40, true);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info2.first.GetCombined()), info2.first.GetCombined()).IsValid());

    // do wal checkpoint to get correct cache file size
    const_cast<Db&>(m_cache->GetDb()).TryExecuteSql("PRAGMA wal_checkpoint(TRUNCATE)");

    // set cache file size limit
    BeFileName cacheFile(m_cache->GetDb().GetDbFileName());
    uint64_t fileSize = 0;
    cacheFile.GetFileSize(fileSize);
    m_cache->SetCacheFileSizeLimit(fileSize - 1);

    // close cache
    m_nodesProviderContextFactory.SetNodesCache(nullptr);
    m_cache = nullptr;

    // open cache
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    m_cache = NodesCache::Create(*m_connection, tempDir, m_nodesFactory, m_nodesProviderContextFactory, m_providersFactory, NodesCacheType::Disk, true);
    m_nodesProviderContextFactory.SetNodesCache(m_cache);

    // verify first ruleset cache was deleted
    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined()), info.first.GetCombined()).IsValid());

    // verify second ruleset cache wasn't deleted
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info2.first.GetCombined()), info2.first.GetCombined()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, DoesntClearCacheWhenCacheFileSizeAndLimitAreEqual)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());
    FillWithNodes(info, 30, true);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined()), info.first.GetCombined()).IsValid());

    // do wal checkpoint to get correct cache file size
    const_cast<Db&>(m_cache->GetDb()).TryExecuteSql("PRAGMA wal_checkpoint(TRUNCATE)");

    // set cache file size limit
    BeFileName cacheFile(m_cache->GetDb().GetDbFileName());
    uint64_t fileSize = 0;
    cacheFile.GetFileSize(fileSize);
    m_cache->SetCacheFileSizeLimit(fileSize);

    // close cache
    m_nodesProviderContextFactory.SetNodesCache(nullptr);
    m_cache = nullptr;

    // open cache
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    m_cache = NodesCache::Create(*m_connection, tempDir, m_nodesFactory, m_nodesProviderContextFactory, m_providersFactory, NodesCacheType::Disk, true);
    m_nodesProviderContextFactory.SetNodesCache(m_cache);

    // verify the cache is still valid
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined()), info.first.GetCombined()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, ClearSharedCacheIfHierarchyWasModified)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", BeGuid());

    // cache some nodes
    auto nodes = FillWithNodes(info, 2, true);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined()), info.first.GetCombined()).IsValid());
    EXPECT_TRUE(IsHierarchyLevelCached(*m_cache, nodes[0]->GetNodeId()));
    EXPECT_TRUE(IsHierarchyLevelCached(*m_cache, nodes[1]->GetNodeId()));

    // close cache
    m_nodesProviderContextFactory.SetNodesCache(nullptr);
    m_cache = nullptr;

    // change connection file last modification time
    BeFileName projectPath(s_project->GetECDbPath());
    time_t modTime;
    projectPath.GetFileTime(nullptr, nullptr, &modTime);
    modTime -= 100;
    projectPath.SetFileTime(nullptr, &modTime);

    // open cache
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    m_cache = NodesCache::Create(*m_connection, tempDir, m_nodesFactory, m_nodesProviderContextFactory, m_providersFactory, NodesCacheType::Disk, true);
    m_nodesProviderContextFactory.SetNodesCache(m_cache);

    // cache should be empty for this connection
    CombinedHierarchyLevelIdentifier updatedInfo(m_connection->GetId(), info.first.GetRulesetId(), BeGuid());
    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(*CreateContext(updatedInfo), updatedInfo).IsValid());
    EXPECT_FALSE(IsHierarchyLevelCached(*m_cache, nodes[0]->GetNodeId()));
    EXPECT_FALSE(IsHierarchyLevelCached(*m_cache, nodes[1]->GetNodeId()));
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DiskNodesCacheLocationTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;

    TestConnectionManager m_connections;
    IConnectionPtr m_connection;
    TestUserSettingsManager m_userSettings;
    NavNodesFactory m_nodesFactory;
    TestNodesProviderContextFactory m_nodesProviderContextFactory;
    TestNodesProviderFactory m_providersFactory;
    BeFileName m_directory;
    Utf8String m_ecdbFileName;
    DiskNodesCacheLocationTests() : m_nodesProviderContextFactory(m_connections) {}
    void SetUp() override
        {
        BeTest::GetHost().GetOutputRoot(m_directory);
        m_directory.AppendToPath(L"DiskNodesCacheLocationTests");
        if (!m_directory.DoesPathExist())
            BeFileName::CreateNewDirectory(m_directory.c_str());
        m_ecdbFileName = Utf8String(BeFileName(s_project->GetECDbCR().GetDbFileName()).GetFileNameAndExtension().c_str());
        m_connection = m_connections.NotifyConnectionOpened(s_project->GetECDb());
        }

    virtual void TearDown() override
        {
        m_connections.CloseConnections();
        s_project->GetECDb().AbandonChanges();
        }

    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("DiskNodesCacheLocationTestsProject");
        }

    static void TearDownTestCase()
        {
        s_project->GetECDb().CloseDb();
        DELETE_AND_CLEAR(s_project);
        }
    };
ECDbTestProject* DiskNodesCacheLocationTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheLocationTests, CreatesCacheInSpecifiedDirectory)
    {
    BeFileName expectedPath = m_directory;
    Utf8PrintfString cacheName("%s-hierarchies", m_ecdbFileName.c_str());
    expectedPath.AppendSeparator().AppendUtf8(cacheName.c_str());

    auto cache = NodesCache::Create(*m_connection, m_directory, m_nodesFactory, m_nodesProviderContextFactory, m_providersFactory, NodesCacheType::Disk, true);
    EXPECT_STREQ(expectedPath.GetNameUtf8().c_str(), cache->GetDb().GetDbFileName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheLocationTests, ReusesExistingCache)
    {
    BeFileName expectedPath = m_directory;
    Utf8PrintfString cacheName("%s-hierarchies", m_ecdbFileName.c_str());
    expectedPath.AppendSeparator().AppendUtf8(cacheName.c_str());

    {
    NodesCache::Create(*m_connection, m_directory, m_nodesFactory, m_nodesProviderContextFactory, m_providersFactory, NodesCacheType::Disk, true);
    }
    EXPECT_TRUE(expectedPath.DoesPathExist());

    auto cache = NodesCache::Create(*m_connection, m_directory, m_nodesFactory, m_nodesProviderContextFactory, m_providersFactory,  NodesCacheType::Disk, true);
    EXPECT_STREQ(expectedPath.GetNameUtf8().c_str(), cache->GetDb().GetDbFileName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheLocationTests, CreatesCacheInTheSameDirectoryAsIModelIfDirectoryNotSpecified)
    {
    BeFileName expectedPath = BeFileName(s_project->GetECDbPath()).GetDirectoryName();
    Utf8PrintfString cacheName("%s-hierarchies", m_ecdbFileName.c_str());
    expectedPath.AppendUtf8(cacheName.c_str());

    auto cache = NodesCache::Create(*m_connection, BeFileName(), m_nodesFactory, m_nodesProviderContextFactory, m_providersFactory, NodesCacheType::Disk, true);
    EXPECT_STREQ(expectedPath.GetNameUtf8().c_str(), cache->GetDb().GetDbFileName());
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NodesCacheHierarchyLevelLockerTests : NodesCacheTests
{
};

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheHierarchyLevelLockerTests, LocksRootHierarchyLevel)
    {
    auto rootLevelInfo = CacheDataSource(m_connection->GetId(), "ruleset_id");
    auto rootLevelLocker = m_cache->CreateHierarchyLevelLocker(rootLevelInfo.first.GetCombined());

    // expect root level to be unlocked
    EXPECT_FALSE(m_cache->IsHierarchyLevelLocked(rootLevelInfo.first.GetCombined()));

    // lock root level
    EXPECT_TRUE(rootLevelLocker->Lock());

    // expect root to be locked now
    EXPECT_TRUE(m_cache->IsHierarchyLevelLocked(rootLevelInfo.first.GetCombined()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheHierarchyLevelLockerTests, LocksChildHierarchyLevel)
    {
    auto rootLevelInfo = CacheDataSource(m_connection->GetId(), "ruleset_id");
    auto rootNodes = FillWithNodes(rootLevelInfo, 1, false, false);

    auto childLevelInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", rootNodes[0]->GetNodeId(), rootNodes[0]->GetNodeId());

    auto childLevelLocker = m_cache->CreateHierarchyLevelLocker(childLevelInfo.first.GetCombined());

    // expect root and child levels to be unlocked
    EXPECT_FALSE(m_cache->IsHierarchyLevelLocked(rootLevelInfo.first.GetCombined()));
    EXPECT_FALSE(m_cache->IsHierarchyLevelLocked(childLevelInfo.first.GetCombined()));

    // lock child level
    EXPECT_TRUE(childLevelLocker->Lock());

    // expect only child level to be locked
    EXPECT_FALSE(m_cache->IsHierarchyLevelLocked(rootLevelInfo.first.GetCombined()));
    EXPECT_TRUE(m_cache->IsHierarchyLevelLocked(childLevelInfo.first.GetCombined()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheHierarchyLevelLockerTests, ChildHierarchyLevelShouldBeConsideredLockedWhenAncestorIsLocked)
    {
    auto rootLevelInfo = CacheDataSource(m_connection->GetId(), "ruleset_id");
    auto rootNodes = FillWithNodes(rootLevelInfo, 1, false, false);

    auto childLevelInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", rootNodes[0]->GetNodeId(), rootNodes[0]->GetNodeId());

    auto rootLevelLocker = m_cache->CreateHierarchyLevelLocker(rootLevelInfo.first.GetCombined());

    // expect root and child levels to be unlocked
    EXPECT_FALSE(m_cache->IsHierarchyLevelLocked(rootLevelInfo.first.GetCombined()));
    EXPECT_FALSE(m_cache->IsHierarchyLevelLocked(childLevelInfo.first.GetCombined()));

    // lock root level
    EXPECT_TRUE(rootLevelLocker->Lock());

    // expect both levels to be locked
    EXPECT_TRUE(m_cache->IsHierarchyLevelLocked(rootLevelInfo.first.GetCombined()));
    EXPECT_TRUE(m_cache->IsHierarchyLevelLocked(childLevelInfo.first.GetCombined()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheHierarchyLevelLockerTests, LocksSiblingHierarchyLevel)
    {
    auto rootLevelInfo = CacheDataSource(m_connection->GetId(), "ruleset_id");
    auto rootNodes = FillWithNodes(rootLevelInfo, 2, false, false);

    auto firstChildLevelInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", rootNodes[0]->GetNodeId(), rootNodes[0]->GetNodeId());
    auto secondChildLevelInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", rootNodes[1]->GetNodeId(), rootNodes[1]->GetNodeId());

    auto secondChildLevelLocker = m_cache->CreateHierarchyLevelLocker(secondChildLevelInfo.first.GetCombined());

    // expect root and child levels to be unlocked
    EXPECT_FALSE(m_cache->IsHierarchyLevelLocked(rootLevelInfo.first.GetCombined()));
    EXPECT_FALSE(m_cache->IsHierarchyLevelLocked(firstChildLevelInfo.first.GetCombined()));
    EXPECT_FALSE(m_cache->IsHierarchyLevelLocked(secondChildLevelInfo.first.GetCombined()));

    // lock root level
    EXPECT_TRUE(secondChildLevelLocker->Lock());

    // expect only second child level to be locked
    EXPECT_FALSE(m_cache->IsHierarchyLevelLocked(rootLevelInfo.first.GetCombined()));
    EXPECT_FALSE(m_cache->IsHierarchyLevelLocked(firstChildLevelInfo.first.GetCombined()));
    EXPECT_TRUE(m_cache->IsHierarchyLevelLocked(secondChildLevelInfo.first.GetCombined()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheHierarchyLevelLockerTests, CreatesAndLocksHierarchyLevelIfItDoesNotExistsWhenLockerIsCreated)
    {
    CombinedHierarchyLevelIdentifier rootInfo(m_connection->GetId(), "ruleset_id", BeGuid());

    EXPECT_FALSE(m_cache->FindPhysicalHierarchyLevelId(rootInfo).IsValid());

    auto hierarchyLevelLocker = m_cache->CreateHierarchyLevelLocker(rootInfo);

    EXPECT_TRUE(m_cache->FindPhysicalHierarchyLevelId(rootInfo).IsValid());
    EXPECT_TRUE(m_cache->IsHierarchyLevelLocked(rootInfo));
    }
