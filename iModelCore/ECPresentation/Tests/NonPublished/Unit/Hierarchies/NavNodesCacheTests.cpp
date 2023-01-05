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
        s_project->Open("NodesCacheTests", ECDb::OpenParams(Db::OpenMode::ReadWrite));

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
    s_project->Open("NodesCacheTests", ECDb::OpenParams(Db::OpenMode::ReadWrite));
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
    m_cache->OnRulesetUsed(*PresentationRuleSet::CreateInstance(TEST_RULESET_ID));
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
bpair<HierarchyLevelIdentifier, DataSourceIdentifier> NodesCacheTests::CacheDataSource(Utf8StringCR connectionId, Utf8StringCR rulesetId, BeGuidCR virtualParentId, BeGuidCR physicalParentId, bvector<uint64_t> const& index, bool finalize, RulesetVariables const& variables, InstanceFilterDefinitionCP instanceFilter)
    {
    return CacheDataSource(*m_cache, connectionId, rulesetId, virtualParentId, physicalParentId, index, finalize, variables, instanceFilter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
bpair<HierarchyLevelIdentifier, DataSourceIdentifier> NodesCacheTests::CacheDataSource(IHierarchyCache& cache, Utf8StringCR connectionId, Utf8StringCR rulesetId, BeGuidCR virtualParentId, BeGuidCR physicalParentId, bvector<uint64_t> const& index, bool finalize, RulesetVariables const& variables, InstanceFilterDefinitionCP instanceFilter)
    {
    HierarchyLevelIdentifier hlIdentifier(connectionId, rulesetId, physicalParentId, virtualParentId);
    hlIdentifier.SetId(cache.FindHierarchyLevelId(connectionId.c_str(), rulesetId.c_str(), virtualParentId, BeGuid()));
    if (!hlIdentifier.IsValid())
        hlIdentifier = CacheHierarchyLevel(cache, connectionId, rulesetId, virtualParentId, physicalParentId);

    DataSourceInfo dsInfo(DataSourceIdentifier(hlIdentifier.GetId(), index, instanceFilter ? std::make_unique<InstanceFilterDefinition>(*instanceFilter) : nullptr), variables, DataSourceFilter(), bmap<ECClassId, bool>(), "", "");
    dsInfo.SetIsInitialized(finalize);

    cache.Cache(dsInfo);
    EXPECT_TRUE(dsInfo.GetIdentifier().IsValid());

    return bpair<HierarchyLevelIdentifier, DataSourceIdentifier>(hlIdentifier, dsInfo.GetIdentifier());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
bpair<HierarchyLevelIdentifier, DataSourceIdentifier> NodesCacheTests::CacheDataSource(Utf8StringCR connectionId, Utf8StringCR rulesetId, BeGuidCR parentId, bvector<uint64_t> const& index, bool finalize, RulesetVariables const& variables, InstanceFilterDefinitionCP instanceFilter)
    {
    return CacheDataSource(*m_cache, connectionId, rulesetId, parentId, parentId, index, finalize, variables, instanceFilter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
bpair<HierarchyLevelIdentifier, DataSourceIdentifier> NodesCacheTests::CacheDataSource(IHierarchyCache& cache, Utf8StringCR connectionId, Utf8StringCR rulesetId, BeGuidCR parentId, bvector<uint64_t> const& index, bool finalize, RulesetVariables const& variables, InstanceFilterDefinitionCP instanceFilter)
    {
    return CacheDataSource(cache, connectionId, rulesetId, parentId, parentId, index, finalize, variables, instanceFilter);
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
        "JOIN [" NODESCACHE_TABLENAME_Rulesets "] r ON [r].[Id] = [hl].[RulesetId] "
        "WHERE [r].[Identifier] = ? AND [hl].[ParentNodeId] IS NULL";

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
        "WHERE [ParentNodeId] = ?";

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
    CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
    CacheDataSource(m_connection->GetId(), "ruleset_id2", BeGuid());
    CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());

    // make sure everything's cached
    EXPECT_TRUE(m_cache->FindHierarchyLevelId(m_connection->GetId().c_str(), TEST_RULESET_ID, BeGuid(), BeGuid()).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevelId(m_connection->GetId().c_str(), "ruleset_id2", BeGuid(), BeGuid()).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevelId(m_connection->GetId().c_str(), TEST_RULESET_ID, BeGuid(), BeGuid()).IsValid());

    // fully clear the cache
    m_cache->Clear();

    // verify everything's gone
    EXPECT_FALSE(m_cache->FindHierarchyLevelId(m_connection->GetId().c_str(), TEST_RULESET_ID, BeGuid(), BeGuid()).IsValid());
    EXPECT_FALSE(m_cache->FindHierarchyLevelId(m_connection->GetId().c_str(), "ruleset_id2", BeGuid(), BeGuid()).IsValid());
    EXPECT_FALSE(m_cache->FindHierarchyLevelId(m_connection->GetId().c_str(), TEST_RULESET_ID, BeGuid(), BeGuid()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Clear_ByRulesetId)
    {
    // create a different ruleset
    m_cache->OnRulesetUsed(*PresentationRuleSet::CreateInstance("ruleset_id2"));

    // cache
    CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
    CacheDataSource(m_connection->GetId(), "ruleset_id2", BeGuid());

    // make sure everything's cached
    EXPECT_TRUE(m_cache->FindHierarchyLevelId(m_connection->GetId().c_str(), TEST_RULESET_ID, BeGuid(), BeGuid()).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevelId(m_connection->GetId().c_str(), "ruleset_id2", BeGuid(), BeGuid()).IsValid());

    // clear TEST_RULESET_ID hierarchy from cache
    m_cache->Clear(TEST_RULESET_ID);

    // verify only TEST_RULESET_ID hierarchy was removed
    EXPECT_FALSE(m_cache->FindHierarchyLevelId(m_connection->GetId().c_str(), TEST_RULESET_ID, BeGuid(), BeGuid()).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevelId(m_connection->GetId().c_str(), "ruleset_id2", BeGuid(), BeGuid()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemoveHierarchyLevel_RemovesHierarchyLevel)
    {
    // cache data source
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);

    // verify it got cached
    EXPECT_TRUE(m_cache->FindHierarchyLevelId(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(), BeGuid(), BeGuid()).IsValid());

    // attempt to remove with invalid ruleset id and verify it didn't get removed
    BeGuid removalId(true);
    CombinedHierarchyLevelIdentifier identifierWithInvalidRulesetId(info.first.GetConnectionId(), "invalid", BeGuid());
    identifierWithInvalidRulesetId.SetRemovalId(removalId);
    m_cache->SetRemovalId(identifierWithInvalidRulesetId);
    m_cache->RemoveHierarchyLevels(removalId);
    EXPECT_TRUE(m_cache->FindHierarchyLevelId(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(), BeGuid(), BeGuid()).IsValid());

    // attempt to remove with invalid parent node and verify it didn't get removed
    CombinedHierarchyLevelIdentifier identifierWithInvalidParentNodeId(info.first.GetConnectionId(), info.first.GetRulesetId(), BeGuid(true));
    identifierWithInvalidParentNodeId.SetRemovalId(removalId);
    m_cache->SetRemovalId(identifierWithInvalidParentNodeId);
    m_cache->RemoveHierarchyLevels(removalId);
    EXPECT_TRUE(m_cache->FindHierarchyLevelId(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(), BeGuid(), BeGuid()).IsValid());

    // attempt to remove with valid parameters and verify it did get removed
    CombinedHierarchyLevelIdentifier validIdentifier(info.first.GetConnectionId(), info.first.GetRulesetId(), BeGuid());
    validIdentifier.SetRemovalId(removalId);
    m_cache->SetRemovalId(validIdentifier);
    m_cache->RemoveHierarchyLevels(removalId);
    EXPECT_FALSE(m_cache->FindHierarchyLevelId(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(), BeGuid(), BeGuid()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemoveHierarchyLevel_RemovesChildHierarchyLevel)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);
    auto nodes = FillWithNodes(rootInfo, 1);
    auto childInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, nodes[0]->GetNodeId());

    // verify child data source is cached
    EXPECT_TRUE(m_cache->GetHierarchyLevel(*CreateContext(TEST_RULESET_ID, nodes[0]->GetNodeId()), childInfo.first.GetId()).IsValid());

    // remove child data source
    BeGuid removalId(true);
    CombinedHierarchyLevelIdentifier identifier(childInfo.first.GetConnectionId(), childInfo.first.GetRulesetId(), nodes[0]->GetNodeId());
    identifier.SetRemovalId(removalId);
    m_cache->SetRemovalId(identifier);
    m_cache->RemoveHierarchyLevels(removalId);

    // verify child data source also got removed
    EXPECT_FALSE(m_cache->GetHierarchyLevel(*CreateContext(TEST_RULESET_ID, nodes[0]->GetNodeId()), childInfo.first.GetId()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemoveHierarchyLevel_RemovesChildNodesWhenParentHierarchyLevelIsRemoved)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);
    auto nodes = FillWithNodes(rootInfo, 1);

    // verify node is cached
    EXPECT_TRUE(m_cache->GetNode(nodes[0]->GetNodeId()).IsValid());

    // remove child data source
    BeGuid removalId(true);
    CombinedHierarchyLevelIdentifier identifier(rootInfo.first.GetConnectionId(), rootInfo.first.GetRulesetId(), BeGuid());
    identifier.SetRemovalId(removalId);
    m_cache->SetRemovalId(identifier);
    m_cache->RemoveHierarchyLevels(removalId);

    // verify child data source also got removed
    EXPECT_FALSE(m_cache->GetNode(nodes[0]->GetNodeId()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemovesChildDataSourcesWhenParentDataSourceIsRemoved)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);
    auto nodes = FillWithNodes(rootInfo, 1);
    auto childInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, nodes[0]->GetNodeId());

    // verify child data source is cached
    EXPECT_TRUE(m_cache->GetHierarchyLevel(*CreateContext(TEST_RULESET_ID, nodes[0]->GetNodeId()), childInfo.first.GetId()).IsValid());

    // remove root data source
    BeGuid removalId(true);
    CombinedHierarchyLevelIdentifier identifier(rootInfo.first.GetConnectionId(), rootInfo.first.GetRulesetId(), BeGuid());
    identifier.SetRemovalId(removalId);
    m_cache->SetRemovalId(identifier);
    m_cache->RemoveHierarchyLevels(removalId);

    // verify child data source also got removed
    EXPECT_FALSE(m_cache->GetHierarchyLevel(*CreateContext(TEST_RULESET_ID, nodes[0]->GetNodeId()), childInfo.first.GetId()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, MakeVirtual)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);
    auto nodes = FillWithNodes(rootInfo, 1, false, false);

    // cache some children
    auto virtualNodeChildrenInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, nodes[0]->GetNodeId(), nodes[0]->GetNodeId());
    auto childNodes = FillWithNodes(virtualNodeChildrenInfo, 1, false, false);

    // cache some grand-children
    auto virtualNodeGrandChildrenInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, childNodes[0]->GetNodeId(), nodes[0]->GetNodeId());
    auto grandChildNodes = FillWithNodes(virtualNodeGrandChildrenInfo, 1, false, false);

    // verify node is physical
    EXPECT_EQ(NodeVisibility::Visible, m_cache->GetNodeVisibility(nodes[0]->GetNodeId(), RulesetVariables(), nullptr));

    // make virtual
    m_cache->MakeVirtual(nodes[0]->GetNodeId(), RulesetVariables(), nullptr);

    // verify node is virtual
    EXPECT_EQ(NodeVisibility::Virtual, m_cache->GetNodeVisibility(nodes[0]->GetNodeId(), RulesetVariables(), nullptr));
    }

/*---------------------------------------------------------------------------------**//**
* note: the test uses GetRelatedHierarchyLevels function to test the Update
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, UpdateDataSource_UpdatesFilter)
    {
    ECClassCP widgetHasGadgetRelationship = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    // cache the data source
    HierarchyLevelIdentifier hlInfo(m_connection->GetId(), TEST_RULESET_ID, BeGuid(), BeGuid());
    m_cache->Cache(hlInfo);
    EXPECT_TRUE(hlInfo.IsValid());

    ECClassId usedClassId((uint64_t)1);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[usedClassId] = false;

    DataSourceInfo dsInfo(DataSourceIdentifier(hlInfo.GetId(), { 0 }, nullptr), RulesetVariables(), DataSourceFilter(), usedClassIds, "", "");
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
    HierarchyLevelIdentifier hlInfo(m_connection->GetId(), TEST_RULESET_ID, BeGuid(), BeGuid());
    m_cache->Cache(hlInfo);
    EXPECT_TRUE(hlInfo.IsValid());

    DataSourceInfo dsInfo(DataSourceIdentifier(hlInfo.GetId(), {0}, nullptr), RulesetVariables(), DataSourceFilter(), bmap<ECClassId, bool>(), "", "");
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

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, UpdateDataSource_UpdatesRelatedSettings)
    {
    // cache the data source
    HierarchyLevelIdentifier hlInfo(m_connection->GetId(), TEST_RULESET_ID, BeGuid(), BeGuid());
    m_cache->Cache(hlInfo);
    EXPECT_TRUE(hlInfo.IsValid());

    DataSourceIdentifier id(hlInfo.GetId(), { 0 }, nullptr);
    DataSourceInfo dsInfo(id, RulesetVariables({ RulesetVariableEntry("setting_id", "") }), DataSourceFilter(), bmap<ECClassId, bool>(), "", "");
    m_cache->Cache(dsInfo);
    EXPECT_TRUE(dsInfo.GetIdentifier().IsValid());

    // verify we don't find the data source as the variable value doesn't match
    EXPECT_FALSE(m_cache->FindDataSource(id, { RulesetVariableEntry("setting_id", "x") }).GetIdentifier().IsValid());

    // update the datasource related settings
    dsInfo.SetRelatedVariables({ RulesetVariableEntry("setting_id", "x") });
    m_cache->Update(dsInfo, DataSourceInfo::PART_Vars);

    // verify we do find the data source as it's is now related to the variable
    EXPECT_TRUE(m_cache->FindDataSource(id, { RulesetVariableEntry("setting_id", "x") }).GetIdentifier().IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetFilteredNodesProvider_FindsRootNode)
    {
    RulesetVariables variables({ RulesetVariableEntry("var_id", rapidjson::Value("value1")) });

    // create root data source
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid(), { 0 }, true, variables);

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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid(), { 0 }, true, variables);
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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid(), { 0 }, true, variables);
    auto nodes = FillWithNodes(info, 1);
    CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, nodes[0]->GetNodeId(), { 0 }, false, variables);

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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);
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
    EXPECT_FALSE(m_cache->FindHierarchyLevelId(m_connection->GetId().c_str(), "test ruleset id", BeGuid(), BeGuid()).IsValid());
    EXPECT_FALSE(m_cache->FindDataSource(DataSourceIdentifier(BeGuid(true), { 0 }, nullptr), RulesetVariables()).GetIdentifier().IsValid());

    BeGuid id(true);
    auto context = CreateContext("test ruleset id", id);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(), "test ruleset id", id)).IsNull());
    EXPECT_TRUE(m_cache->GetHierarchyLevel(*context, BeGuid(true)).IsNull());
    EXPECT_TRUE(m_cache->GetNode(id).IsNull());
    EXPECT_EQ(0, m_cache->GetCachedDirectNodesIterator(*context, DataSourceIdentifier(BeGuid(true), { 0 }, nullptr))->NodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedRootDataSource)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);
    auto context = CreateContext("test ruleset id", BeGuid());

    // verify the data source is cached
    EXPECT_TRUE(IsHierarchyLevelCached(*m_cache, info.first.GetRulesetId().c_str()));

    EXPECT_TRUE(m_cache->FindHierarchyLevelId(m_connection->GetId().c_str(), info.first.GetRulesetId().c_str(), BeGuid(), BeGuid()).IsValid());
    EXPECT_TRUE(m_cache->FindDataSource(DataSourceIdentifier(info.first.GetId(), { 0 }, nullptr), RulesetVariables()).GetIdentifier().IsValid());

    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(), info.first.GetRulesetId(), BeGuid())).IsValid());
    EXPECT_TRUE(m_cache->GetCachedDirectNodesIterator(*context, info.second) != nullptr);

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
    auto info1 = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid(), { 0 }, true, variables1);
    // cache second root data source with different variables
    auto info2 = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid(), { 0 }, true, variables2);

    // verify that data sources were cached for same hierarchy level
    EXPECT_TRUE(info1.first == info2.first);
    // verify that data sources are different
    EXPECT_FALSE(info1.second == info2.second);

    // verify the hierarchy level is cached
    EXPECT_TRUE(IsHierarchyLevelCached(*m_cache, info1.first.GetRulesetId().c_str()));
    EXPECT_TRUE(m_cache->FindHierarchyLevelId(m_connection->GetId().c_str(), info1.first.GetRulesetId().c_str(), BeGuid(), BeGuid()).IsValid());

    // verify that data sources are cached
    DataSourceInfo cachedSource1 = m_cache->FindDataSource(DataSourceIdentifier(info1.first.GetId(), { 0 }, nullptr), variables1);
    EXPECT_TRUE(cachedSource1.GetIdentifier().IsValid());
    DataSourceInfo cachedSource2 = m_cache->FindDataSource(DataSourceIdentifier(info1.first.GetId(), { 0 }, nullptr), variables2);
    EXPECT_TRUE(cachedSource2.GetIdentifier().IsValid());
    EXPECT_FALSE(cachedSource1.GetIdentifier() == cachedSource2.GetIdentifier());

    // verify hierarchy level is not found when looking with invalid variables
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info1.first.GetRulesetId(), BeGuid(), RulesetVariables()),
        CombinedHierarchyLevelIdentifier(m_connection->GetId(), info1.first.GetRulesetId(), BeGuid())).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsDifferentCachedRootDataSourceForDifferentInstanceFilters)
    {
    auto f1 = std::make_shared<InstanceFilterDefinition>("f1");
    auto f2 = std::make_shared<InstanceFilterDefinition>("f2");

    // cache root data source
    auto info1 = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid(), { 0 }, true, RulesetVariables(), f1.get());
    // cache second root data source with different variables
    auto info2 = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid(), { 0 }, true, RulesetVariables(), f2.get());

    // verify that data sources where cached for same hierarchy level
    EXPECT_TRUE(info1.first == info2.first);
    // verify that data sources are different
    EXPECT_FALSE(info1.second == info2.second);
    EXPECT_FALSE(info1.second.GetId() == info2.second.GetId());

    // verify the hierarchy level is cached
    EXPECT_TRUE(IsHierarchyLevelCached(*m_cache, info1.first.GetRulesetId().c_str()));
    EXPECT_TRUE(m_cache->FindHierarchyLevelId(m_connection->GetId().c_str(), info1.first.GetRulesetId().c_str(), BeGuid(), BeGuid()).IsValid());

    // verify that data sources are cached
    DataSourceInfo cachedSource1 = m_cache->FindDataSource(DataSourceIdentifier(info1.first.GetId(), { 0 }, f1), RulesetVariables());
    EXPECT_TRUE(cachedSource1.GetIdentifier().IsValid());
    DataSourceInfo cachedSource2 = m_cache->FindDataSource(DataSourceIdentifier(info1.first.GetId(), { 0 }, f2), RulesetVariables());
    EXPECT_TRUE(cachedSource2.GetIdentifier().IsValid());
    EXPECT_FALSE(cachedSource1.GetIdentifier() == cachedSource2.GetIdentifier());
    EXPECT_FALSE(cachedSource1.GetIdentifier().GetId() == cachedSource2.GetIdentifier().GetId());

    // verify hierarchy level is not found when looking with different instance filter
    auto context = CreateContext(info1.first.GetRulesetId(), BeGuid(), RulesetVariables());
    context->SetInstanceFilter(std::make_unique<InstanceFilterDefinition>("f3"));
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*context, info1.first.GetCombined()).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedRootNode)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);
    auto context = CreateContext(info.first.GetRulesetId(), BeGuid());

    // create a node
    auto node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    InitNode(*node, info.first);

    // the root data source should be empty
    NavNodesProviderPtr cachedProvider = m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(),
        info.first.GetRulesetId(), BeGuid()));
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(0, cachedProvider->GetNodesCount());

    // cache the node
    m_cache->Cache(*node, info.second, 0, NodeVisibility::Visible);

    // verify the node is cached
    cachedProvider = m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(),
        info.first.GetRulesetId(), BeGuid()));
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(1, cachedProvider->GetNodesCount());

    EXPECT_TRUE(IsNodeCached(*m_cache, node->GetNodeId()));
    EXPECT_TRUE(m_cache->GetNode(node->GetNodeId()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedChildDataSource)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);
    auto nodes = FillWithNodes(info, 1);
    BeGuid nodeId = nodes[0]->GetNodeId();
    auto context = CreateContext(info.first.GetRulesetId(), nodeId);

    // child data source should not exist
    EXPECT_FALSE(IsHierarchyLevelCached(*m_cache, nodeId));
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*context, CombinedHierarchyLevelIdentifier(m_connection->GetId(),
        info.first.GetRulesetId(), nodeId)).IsNull());

    // cache child data source
    auto childrenInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, nodeId);

    // verify the data source exists now
    EXPECT_TRUE(IsHierarchyLevelCached(*m_cache, nodeId));
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*context, childrenInfo.first.GetCombined()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsDifferentCachedChildDataSourceForDifferentRulesetVariables)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);
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

    auto childrenInfo1 = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, nodeId, { 0 }, true, variables1);
    auto childrenInfo2 = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, nodeId, { 0 }, true, variables2);
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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);
    auto nodes = FillWithNodes(info, 1);
    BeGuid nodeId = nodes[0]->GetNodeId();

    // cache child data source
    auto childrenInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, nodeId);

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
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetNodeVisibility_ReturnsPhysicalNodeVisibility)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);
    auto nodes = FillWithNodes(info, 1);

    // verify results
    EXPECT_EQ(NodeVisibility::Visible, m_cache->GetNodeVisibility(nodes[0]->GetNodeId(), RulesetVariables(), nullptr));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetNodeVisibility_ReturnsVirtualNodeVisibility)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);
    auto nodes = FillWithNodes(info, 1, false, true);

    // verify results
    EXPECT_EQ(NodeVisibility::Virtual, m_cache->GetNodeVisibility(nodes[0]->GetNodeId(), RulesetVariables(), nullptr));
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
    NavNodesProviderPtr cached1 = m_cache->GetHierarchyLevel(*CreateContext(info1.first.GetRulesetId(), BeGuid()), info1.first.GetId());
    ASSERT_TRUE(cached1.IsValid());
    EXPECT_STREQ(info1.first.GetRulesetId().c_str(), cached1->GetContext().GetRuleset().GetRuleSetId().c_str());

    NavNodesProviderPtr cached2 = m_cache->GetHierarchyLevel(*CreateContext(info2.first.GetRulesetId(), BeGuid()), info2.first.GetId());
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
    auto info1 = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid(), { 0 }, true, variables1);

    // cache root data source for the second variables
    auto info2 = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid(), { 0 }, true, variables2);

    // verify correct sources are returned
    NavNodesProviderPtr cached1 = m_cache->GetHierarchyLevel(*CreateContext(info1.first.GetRulesetId(), BeGuid(), variables1), info1.first.GetId());
    ASSERT_TRUE(cached1.IsValid());
    EXPECT_TRUE(variables1 == cached1->GetContext().GetRulesetVariables());

    NavNodesProviderPtr cached2 = m_cache->GetHierarchyLevel(*CreateContext(info2.first.GetRulesetId(), BeGuid(), variables2), info2.first.GetId());
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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
    bvector<NavNodePtr> nodes = {
        TestNodesHelper::CreateInstanceNode(*m_connection, *widgetClass)
        };
    FillWithNodes(info, nodes);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, TEST_RULESET_ID, *nodes[0]->GetKey());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when key is invalid
    ECClassInstanceKey invalidKey(
        nodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().front().GetClass(),
        ECInstanceId(nodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceKeys().front().GetId().GetValue() + 100));
    locatedNode = m_cache->LocateNode(*m_connection, TEST_RULESET_ID, *ECInstancesNodeKey::Create(invalidKey, "", { "wrong hash" }));
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
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id_1", *node1->GetKey());
    ASSERT_TRUE(locatedNode.IsValid());
    EXPECT_TRUE(node1->Equals(*locatedNode));

    locatedNode = m_cache->LocateNode(*m_connection, "ruleset_id_2", *node2->GetKey());
    ASSERT_TRUE(locatedNode.IsValid());
    EXPECT_TRUE(node2->Equals(*locatedNode));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECClassGroupingNode)
    {
    // create a node in an open connection
    ECClassCP widgetClass1 = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
    bvector<NavNodePtr> nodes = {
        TestNodesHelper::CreateClassGroupingNode(*m_connection, *widgetClass1, "test label")
        };
    FillWithNodes(info, nodes);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, TEST_RULESET_ID, *nodes[0]->GetKey());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when node hash doesnt match
    locatedNode = m_cache->LocateNode(*m_connection, TEST_RULESET_ID, *ECClassGroupingNodeKey::Create(*widgetClass1, false, "", {"different hash"}, 1));
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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
    bvector<NavNodePtr> nodes = {
        TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *widgetClass1, *groupingProperty1, "test label", {ECValue(9)}, false)
        };
    FillWithNodes(info, nodes);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, TEST_RULESET_ID, *nodes[0]->GetKey());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when node hash doesnt match
    locatedNode = m_cache->LocateNode(*m_connection, TEST_RULESET_ID, *ECPropertyGroupingNodeKey::Create(*widgetClass1, "IntProperty",
        nodes[0]->GetKey()->AsECPropertyGroupingNodeKey()->GetGroupingValuesArray(), "", {"different hash"}, 1));
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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
    bvector<NavNodePtr> nodes = {
        TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *widgetClass1, *groupingProperty1, "test label", {ECValue(2)}, true)
        };
    FillWithNodes(info, nodes);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, TEST_RULESET_ID, *nodes[0]->GetKey());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when node hash doesnt match
    locatedNode = m_cache->LocateNode(*m_connection, TEST_RULESET_ID, *ECPropertyGroupingNodeKey::Create(*widgetClass1, "IntProperty",
        nodes[0]->GetKey()->AsECPropertyGroupingNodeKey()->GetGroupingValuesArray(), "", {"different hash"}, 1));
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesLabelGroupingNode)
    {
    // create a node in an open connection
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
    bvector<NavNodePtr> nodes = {
        TestNodesHelper::CreateLabelGroupingNode(*m_connection, "test label")
        };
    FillWithNodes(info, nodes);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, TEST_RULESET_ID, *nodes[0]->GetKey());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when node hash doesnt match
    locatedNode = m_cache->LocateNode(*m_connection, TEST_RULESET_ID, *NavNodeKey::Create("type", "", {"wrong hash"}));
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesCustomNode)
    {
    // create a node in an open connection
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
    bvector<NavNodePtr> nodes = {
        TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test label", "test descr")
        };
    FillWithNodes(info, nodes);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, TEST_RULESET_ID, *nodes[0]->GetKey());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when node hash doesnt matchSetInstanceId
    locatedNode = m_cache->LocateNode(*m_connection, TEST_RULESET_ID, *NavNodeKey::Create("type", "", {"wrong hash"}));
    ASSERT_TRUE(locatedNode.IsNull());
    }

#ifdef wip_ruleset_variables
/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Settings_ReturnsEmptyListWhenRulesetAndSettingIdsAreInvalid)
    {
    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, 0);
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
    auto rootInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, 0);
    auto rootNodes = FillWithNodes(rootInfo, 2);

    // cache unrelated datasource
    auto childInfo0 = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, rootNodes[0]->GetNodeId());
    bvector<UserSettingEntry> settings0 = {UserSettingEntry("setting_0", Json::Value("value0"))};
    m_cache->Update(childInfo0.second, nullptr, nullptr, &settings0);

    // cache related datasource
    auto childInfo1 = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, rootNodes[1]->GetNodeId());
    bvector<UserSettingEntry> settings1 = {UserSettingEntry("setting_1", Json::Value("value1")) };
    m_cache->Update(childInfo1.second, nullptr, nullptr, &settings1);

    // verify the correct datasource is found
    bvector<HierarchyLevelIdentifier> related = m_cache->GetRelatedHierarchyLevels(TEST_RULESET_ID, "setting_1");
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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());

    // create a node
    ECClassCP nodeClass = GetDb().Schemas().GetClass("ECDbMeta", "ECClassDef");
    auto node = TestNodesHelper::CreateInstanceNode(*m_connection, *nodeClass, ECInstanceId((uint64_t)2));
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId(TEST_RULESET_ID);
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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());

    // create a node
    ECClassCP nodeClass = GetDb().Schemas().GetClass("ECDbMeta", "ECClassDef");
    auto node = TestNodesHelper::CreateInstanceNode(*m_connection, *nodeClass, ECInstanceId((uint64_t)2));
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId(TEST_RULESET_ID);
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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());

    // create a node
    ECClassCP nodeClass = GetDb().Schemas().GetClass("ECDbMeta", "ECClassDef");
    auto node = TestNodesHelper::CreateInstanceNode(*m_connection, *nodeClass, ECInstanceId((uint64_t)2));
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId(TEST_RULESET_ID);
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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
    bvector<ECClassId> relationshipIds = { nodeRelationship->GetId() };
    DataSourceFilter filter(std::make_unique<DataSourceFilter::RelatedInstanceInfo>(relationshipIds, RequiredRelationDirection_Backward, bvector<ECInstanceKey>{ECInstanceKey(nodeClass->GetId(), ECInstanceId((uint64_t)123))}), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[nodeClass->GetId()] = false;
    DataSourceInfo dsInfo(info.second, RulesetVariables(), filter, usedClassIds, "", "");
    m_cache->Update(dsInfo, DataSourceInfo::PART_RelatedClasses | DataSourceInfo::PART_Filter);

    // create a node
    auto node = TestNodesHelper::CreateInstanceNode(*m_connection, *nodeClass, ECInstanceId((uint64_t)2));
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId(TEST_RULESET_ID);
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
    RulesetVariables vars{ RulesetVariableEntry("variable_id", "value0") };
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid(), { 0 }, true, vars);

    // verify that valid data source is returned for matching ruleset variables
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined(), vars), info.first.GetCombined()).IsValid());

    // verify what valid datasource is not returned with different ruleset variables
    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined(), { RulesetVariableEntry("variable_id", "value") }), info.first.GetCombined()).IsValid());

    // verify what valid datasource is not returned for empty variables
    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined(), RulesetVariables()), info.first.GetCombined()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetCombinedHierarchyLevel_ReturnsInvalidChildDatasourceForDifferentRulesetVariables)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
    auto rootNodes = FillWithNodes(rootInfo, 1);

    // cache child data source
    RulesetVariables relatedVariables({ RulesetVariableEntry("variable_id", rapidjson::Value(1))});
    auto childInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, rootNodes[0]->GetNodeId());
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
    // cache data source without related variables
    CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid(), bvector<uint64_t>{ 0 });

    // cache data source with related variables for the same level
    RulesetVariables vars{ RulesetVariableEntry("variable_id", "1") };
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid(), { 1 }, true, vars);

    // verify that valid data source is returned for matching ruleset variables
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined(), vars), info.first.GetCombined()).IsValid());

    // verify that valid data source is not returned for empty variables
    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined(), RulesetVariables()), info.first.GetCombined()).IsValid());

    // verify what valid datasource is not returned with different ruleset variables
    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(*CreateContext(info.first.GetCombined(), { RulesetVariableEntry("variable_id", "2") }), info.first.GetCombined()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetCombinedHierarchyLevel_ReturnsInvalidChildProviderWhenMultipleDataSourcesCachedAndVariablesAreDifferent)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
    auto rootNodes = FillWithNodes(rootInfo, 1);

    // cache data source without related variables
    CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, rootNodes[0]->GetNodeId(), bvector<uint64_t>{ 0 });

    // cache data source with related variables for the same hierarchy level
    RulesetVariables vars{ RulesetVariableEntry("variable_id", "1") };
    auto childInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, rootNodes[0]->GetNodeId(), { 1 }, true, vars);

    // verify that valid data source is returned for matching ruleset variables
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(childInfo.first.GetCombined(), vars), childInfo.first.GetCombined()).IsValid());

    // verify that valid data source is not returned for empty ruleset variables
    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(*CreateContext(childInfo.first.GetCombined(), RulesetVariables()), childInfo.first.GetCombined()).IsValid());

    // verify what valid datasource is not returned with different ruleset variables
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(*CreateContext(childInfo.first.GetCombined(), { RulesetVariableEntry("variable_id", " 2") }), childInfo.first.GetCombined()).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetHierarchyLevel_ReturnsInvalidRootDatasourceForDifferentRulesetVariables)
    {
    RulesetVariables vars{ RulesetVariableEntry("variable_id", "1") };
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid(), { 0 }, true, vars);

    // verify that valid data source is returned for matching ruleset variables
    EXPECT_TRUE(m_cache->GetHierarchyLevel(*CreateContext(info.first.GetCombined(), vars), info.first.GetId()).IsValid());

    // verify that valid data source is not returned for empty ruleset variables
    EXPECT_FALSE(m_cache->GetHierarchyLevel(*CreateContext(info.first.GetCombined(), RulesetVariables()), info.first.GetId()).IsValid());

    // verify what valid datasource is not returned with different ruleset variables
    EXPECT_FALSE(m_cache->GetHierarchyLevel(*CreateContext(info.first.GetCombined(), { RulesetVariableEntry("variable_id", "2") }), info.first.GetId()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetHierarchyLevel_ReturnsInvalidChildDatasourceForDifferentRulesetVariables)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
    auto rootNodes = FillWithNodes(rootInfo, 1);

    // cache child data source
    RulesetVariables vars{ RulesetVariableEntry("variable_id", "1") };
    auto childInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, rootNodes[0]->GetNodeId(), { 0 }, true, vars);

    // verify that valid data source is returned for matching ruleset variables
    EXPECT_TRUE(m_cache->GetHierarchyLevel(*CreateContext(childInfo.first.GetCombined(), vars), childInfo.first.GetId()).IsValid());

    // verify that valid data source is not returned for empty ruleset variables
    EXPECT_FALSE(m_cache->GetHierarchyLevel(*CreateContext(childInfo.first.GetCombined(), RulesetVariables()), childInfo.first.GetId()).IsValid());

    // verify what valid datasource is not returned with different ruleset variables
    EXPECT_FALSE(m_cache->GetHierarchyLevel(*CreateContext(childInfo.first.GetCombined(), { RulesetVariableEntry("variable_id", "2")}), childInfo.first.GetId()).IsValid());
    }

#ifdef wip_ruleset_variables
/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetDataSource_ClearsRootDatasourceCacheIfRelatedSettingsValuesChanged)
    {
    m_userSettings.GetSettings(TEST_RULESET_ID).SetSettingValue("setting_id", "value0");
    bvector<UserSettingEntry> settingsValues = {UserSettingEntry("setting_id", Json::Value("value0"))};
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, 0);
    m_cache->Update(info.second, nullptr, nullptr, &settingsValues);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->GetDataSource(info.second).IsValid());

    // verify that valid datasource is not returned with different user settings
    m_userSettings.GetSettings(TEST_RULESET_ID).SetSettingValue("setting_id", "value1");

    EXPECT_FALSE(m_cache->GetDataSource(info.second).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetDataSource_DoesntClearRootDatasourceCacheIfRelatedSettingsValuesDidntChange)
    {
    m_userSettings.GetSettings(TEST_RULESET_ID).SetSettingIntValue("setting_id", 10);
    bvector<UserSettingEntry> settingsValues = {UserSettingEntry("setting_id", Json::Value(10))};
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, 0);
    m_cache->Update(info.second, nullptr, nullptr, &settingsValues);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->GetDataSource(info.second).IsValid());

    // verify what valid datasource is returned with same user settings
    m_userSettings.GetSettings(TEST_RULESET_ID).SetSettingIntValue("setting_id", 10);

    EXPECT_TRUE(m_cache->GetDataSource(info.second).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetDataSource_ClearsChildDatasourceCacheIfRelatedSettingsValuesChanged)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, 0);
    auto rootNodes = FillWithNodes(rootInfo, 1);

    // cache child data source
    m_userSettings.GetSettings(TEST_RULESET_ID).SetSettingIntValue("setting_id", 1);
    bvector<UserSettingEntry> settingsValues = {UserSettingEntry("setting_id", Json::Value(1))};
    auto childInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, rootNodes[0]->GetNodeId());
    m_cache->Update(childInfo.second, nullptr, nullptr, &settingsValues);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->GetDataSource(childInfo.second).IsValid());

    // verify what valid datasource is not returned with different user settings
    m_userSettings.GetSettings(TEST_RULESET_ID).SetSettingIntValue("setting_id", 10);

    EXPECT_TRUE(m_cache->GetDataSource(childInfo.second).IsNull());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Savepoint_DiscardsChangesWhenCanceled)
    {
    auto savepoint = m_cache->CreateSavepoint();

    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
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

    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
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
TEST_F(NodesCacheTests, FindPhysicalHierarchyLevelId_ReturnsPhysicalHierarchyLevelId)
    {
    CombinedHierarchyLevelIdentifier rootInfo(m_connection->GetId(), TEST_RULESET_ID, BeGuid());

    EXPECT_FALSE(m_cache->FindPhysicalHierarchyLevelId(rootInfo).IsValid());
    auto rootLevelInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);
    auto rootNode = FillWithNodes(rootLevelInfo, 1, false, false);
    EXPECT_TRUE(m_cache->FindPhysicalHierarchyLevelId(rootInfo).IsValid());

    CombinedHierarchyLevelIdentifier childInfo(m_connection->GetId(), TEST_RULESET_ID, rootNode[0]->GetNodeId());
    EXPECT_FALSE(m_cache->FindPhysicalHierarchyLevelId(childInfo).IsValid());
    auto childLevelInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, rootNode[0]->GetNodeId());
    EXPECT_TRUE(m_cache->FindPhysicalHierarchyLevelId(childInfo).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsPhysicalHierarchyLevelNodes)
    {
    auto cachedRootLevelInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);
    auto cachedRootNodes = FillWithNodes(cachedRootLevelInfo, 1, false, false);

    auto cachedChildLevelInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, cachedRootNodes[0]->GetNodeId());
    auto cachedChildNodes = FillWithNodes(cachedChildLevelInfo, 1, false, false);

    m_cache->MakeVirtual(cachedRootNodes[0]->GetNodeId(), RulesetVariables(), nullptr);

    CombinedHierarchyLevelIdentifier rootInfo(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
    EXPECT_TRUE(m_cache->FindPhysicalHierarchyLevelId(rootInfo).IsValid());
    auto rootNodes = m_cache->GetCombinedHierarchyLevel(
        *CreateContext(cachedRootLevelInfo.first.GetCombined(), RulesetVariables()),
        cachedRootLevelInfo.first.GetCombined());
    EXPECT_EQ(1, rootNodes->GetNodesCount());
    EXPECT_EQ(*cachedChildNodes[0]->GetKey(), *(*rootNodes->begin())->GetKey());
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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());

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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());
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
    auto info = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, BeGuid());

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
    auto rootLevelInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);
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
    auto rootLevelInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);
    auto rootNodes = FillWithNodes(rootLevelInfo, 1, false, false);

    auto childLevelInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, rootNodes[0]->GetNodeId(), rootNodes[0]->GetNodeId());

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
    auto rootLevelInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);
    auto rootNodes = FillWithNodes(rootLevelInfo, 1, false, false);

    auto childLevelInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, rootNodes[0]->GetNodeId(), rootNodes[0]->GetNodeId());

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
    auto rootLevelInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID);
    auto rootNodes = FillWithNodes(rootLevelInfo, 2, false, false);

    auto firstChildLevelInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, rootNodes[0]->GetNodeId(), rootNodes[0]->GetNodeId());
    auto secondChildLevelInfo = CacheDataSource(m_connection->GetId(), TEST_RULESET_ID, rootNodes[1]->GetNodeId(), rootNodes[1]->GetNodeId());

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
    CombinedHierarchyLevelIdentifier rootInfo(m_connection->GetId(), TEST_RULESET_ID, BeGuid());

    EXPECT_FALSE(m_cache->FindPhysicalHierarchyLevelId(rootInfo).IsValid());

    auto hierarchyLevelLocker = m_cache->CreateHierarchyLevelLocker(rootInfo);

    EXPECT_TRUE(m_cache->FindPhysicalHierarchyLevelId(rootInfo).IsValid());
    EXPECT_TRUE(m_cache->IsHierarchyLevelLocked(rootInfo));
    }
