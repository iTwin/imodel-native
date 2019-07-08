/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include "../../../Source/RulesDriven/RulesEngine/NavNodesCache.h"
#include "ECDbTestProject.h"
#include "TestNavNode.h"
#include "TestHelpers.h"
#include <UnitTests/BackDoor/ECPresentation/TestRuleSetLocater.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                10/2017
+===============+===============+===============+===============+===============+======*/
struct TestECSqlStatementsCacheProvider : IECSqlStatementCacheProvider
{
private:
    bmap<Utf8String, ECSqlStatementCache*> m_cache;
protected:
    ECSqlStatementCache& _GetECSqlStatementCache(IConnectionCR connection) override
        {
        auto iter = m_cache.find(connection.GetId());
        if (m_cache.end() == iter)
            iter = m_cache.Insert(connection.GetId(), new ECSqlStatementCache(10)).first;
        return *iter->second;
        }
public:
    TestECSqlStatementsCacheProvider() {}
    ~TestECSqlStatementsCacheProvider()
        {
        for (auto iter : m_cache)
            delete iter.second;
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2017
+===============+===============+===============+===============+===============+======*/
struct NodesCacheTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;
    
    TestConnectionManager m_connections;
    TestUserSettingsManager m_userSettings;
    JsonNavNodesFactory m_nodesFactory;
    TestNodesProviderContextFactory m_nodesProviderContextFactory;
    IConnectionPtr m_connection;
    TestECSqlStatementsCacheProvider m_ecsqlStatements;
    NodesCache* m_cache;

    NodesCacheTests() : m_nodesProviderContextFactory(m_connections) {}

    void SetUp() override
        {
        ECPresentationTest::SetUp();
        if (!s_project->GetECDb().IsDbOpen())
            s_project->Open("NodesCacheTests");

        BeFileName temporaryDirectory;
        BeTest::GetHost().GetTempDir(temporaryDirectory);
        m_cache = _CreateNodesCache(temporaryDirectory);
        m_connection = m_connections.NotifyConnectionOpened(s_project->GetECDb());
        m_cache->OnRulesetCreated(*PresentationRuleSet::CreateInstance("ruleset_id", 1, 0, false, "", "", "", false));
        m_nodesProviderContextFactory.SetNodesCache(m_cache);
        }

    virtual void TearDown() override
        {
        m_connections.ClearConnections();
        DELETE_AND_CLEAR(m_cache);
        s_project->GetECDb().AbandonChanges();
        }

    static ECDbR GetDb() {return s_project->GetECDb();}

    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("NodesCacheTests", "RulesEngineTest.01.00.ecschema.xml");
        }

    static void TearDownTestCase()
        {
        s_project->GetECDb().CloseDb();
        DELETE_AND_CLEAR(s_project);
        }

    void ReCreateProject()
        {
        if (nullptr != s_project)
            {
            m_connections.NotifyConnectionClosed(*m_connection);
            DELETE_AND_CLEAR(s_project);
            }

        s_project = new ECDbTestProject();
        s_project->Open("NodesCacheTests");
        m_connection = m_connections.NotifyConnectionOpened(s_project->GetECDb());
        }

    virtual NodesCache* _CreateNodesCache(BeFileName tempDir)
        {
        return new NodesCache(tempDir, m_nodesFactory, m_nodesProviderContextFactory, m_connections, m_userSettings, m_ecsqlStatements, NodesCacheType::Memory);
        }
    
    void InitNode(JsonNavNodeR, HierarchyLevelInfo const&);
    void FillWithNodes(bpair<HierarchyLevelInfo, DataSourceInfo> const&, bvector<JsonNavNodePtr> const&, bool createChildDataSources = false, bool areVirtual = false);
    bvector<JsonNavNodePtr> FillWithNodes(bpair<HierarchyLevelInfo, DataSourceInfo> const&, size_t count, bool createChildDataSources = false, bool areVirtual = false);
    bpair<HierarchyLevelInfo, DataSourceInfo> CacheDataSource(Utf8StringCR connectionId, Utf8StringCR rulesetId, Utf8StringCR locale, uint64_t parentId = 0, bool finalize = true);
    };
ECDbTestProject* NodesCacheTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheTests::InitNode(JsonNavNodeR node, HierarchyLevelInfo const& info)
    {
    if (nullptr != info.GetPhysicalParentNodeId())
        node.SetParentNodeId(*info.GetPhysicalParentNodeId());

    NavNodeExtendedData extendedData(node);
    extendedData.SetConnectionId(info.GetConnectionId());
    extendedData.SetRulesetId(info.GetRulesetId().c_str());
    if (nullptr != info.GetVirtualParentNodeId())
        extendedData.SetVirtualParentId(*info.GetVirtualParentNodeId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesCacheTests::FillWithNodes(bpair<HierarchyLevelInfo, DataSourceInfo> const& info, bvector<JsonNavNodePtr> const& nodes, bool createChildDataSources, bool areVirtual)
    {
    for (size_t i = 0; i < nodes.size(); ++i)
        {
        InitNode(*nodes[i], info.first);
        m_cache->Cache(*nodes[i], info.second, (uint64_t)i, areVirtual);

        if (createChildDataSources)
            CacheDataSource(info.first.GetConnectionId(), info.first.GetRulesetId(), info.first.GetLocale(), nodes[i]->GetNodeId());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<JsonNavNodePtr> NodesCacheTests::FillWithNodes(bpair<HierarchyLevelInfo, DataSourceInfo> const& info, size_t count, bool createChildDataSources, bool areVirtual)
    {
    bvector<JsonNavNodePtr> nodes;
    for (size_t i = 0; i < count; ++i)
        {
        TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", std::to_string(i).c_str(), "descr");
        nodes.push_back(node);
        }
    FillWithNodes(info, nodes, createChildDataSources, areVirtual);
    return nodes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bpair<HierarchyLevelInfo, DataSourceInfo> NodesCacheTests::CacheDataSource(Utf8StringCR connectionId, Utf8StringCR rulesetId, Utf8StringCR locale, uint64_t parentId, bool finalize)
    {
    HierarchyLevelInfo hlInfo(connectionId, rulesetId, locale, parentId, parentId);
    m_cache->Cache(hlInfo);
    EXPECT_TRUE(hlInfo.IsValid());

    DataSourceInfo dsInfo(hlInfo.GetId(), { 0 });
    m_cache->Cache(dsInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    EXPECT_TRUE(dsInfo.IsValid());

    if (finalize)
        m_cache->FinalizeInitialization(dsInfo);

    return bpair<HierarchyLevelInfo, DataSourceInfo>(hlInfo, dsInfo);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Clear_Full)
    {
    // create a different connection
    ECDbTestProject project2;
    project2.Create("test2");
    IConnectionPtr connection2 = m_connections.NotifyConnectionOpened(project2.GetECDb());

    // create a different ruleset
    m_cache->OnRulesetCreated(*PresentationRuleSet::CreateInstance("ruleset_id2", 1, 0, false, "", "", "", false));

    // cache
    CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    CacheDataSource(connection2->GetId(), "ruleset_id", "locale", 0);
    CacheDataSource(m_connection->GetId(), "ruleset_id2", "locale", 0);
    CacheDataSource(m_connection->GetId(), "ruleset_id", "locale2", 0);

    // make sure everything's cached
    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id", "locale", nullptr).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevel(connection2->GetId().c_str(), "ruleset_id", "locale", nullptr).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id2", "locale", nullptr).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id", "locale2", nullptr).IsValid());

    // fully clear the cache
    m_cache->Clear();

    // verify everything's gone
    EXPECT_FALSE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id", "locale", nullptr).IsValid());
    EXPECT_FALSE(m_cache->FindHierarchyLevel(connection2->GetId().c_str(), "ruleset_id", "locale", nullptr).IsValid());
    EXPECT_FALSE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id2", "locale", nullptr).IsValid());
    EXPECT_FALSE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id", "locale2", nullptr).IsValid());

    m_connections.NotifyConnectionClosed(*connection2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Clear_ByConnection)
    {
    // create a different connection
    ECDbTestProject project2;
    project2.Create("test2");
    IConnectionPtr connection2 = m_connections.NotifyConnectionOpened(project2.GetECDb());
    
    // cache
    CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    CacheDataSource(connection2->GetId(), "ruleset_id", "locale", 0);

    // make sure everything's cached
    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id", "locale", nullptr).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevel(connection2->GetId().c_str(), "ruleset_id", "locale", nullptr).IsValid());

    // clear m_connection hierarchy from cache
    m_cache->Clear(m_connection.get());

    // verify only m_connection hierarchy is removed
    EXPECT_FALSE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id", "locale", nullptr).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevel(connection2->GetId().c_str(), "ruleset_id", "locale", nullptr).IsValid());

    m_connections.NotifyConnectionClosed(*connection2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Clear_ByRulesetId)
    {
    // create a different ruleset
    m_cache->OnRulesetCreated(*PresentationRuleSet::CreateInstance("ruleset_id2", 1, 0, false, "", "", "", false));

    // cache
    CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    CacheDataSource(m_connection->GetId(), "ruleset_id2", "locale", 0);

    // make sure everything's cached
    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id", "locale", nullptr).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id2", "locale", nullptr).IsValid());

    // clear "ruleset_id" hierarchy from cache
    m_cache->Clear(nullptr, "ruleset_id");

    // verify only "ruleset_id" hierarchy was removed
    EXPECT_FALSE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id", "locale", nullptr).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "ruleset_id2", "locale", nullptr).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemoveHierarchyLevel_RemovesHierarchyLevel)
    {
    // cache data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    
    // verify it got cached
    EXPECT_TRUE(m_cache->FindHierarchyLevel(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), nullptr).IsValid());

    // attempt to remove with invalid connection id and verify it didn't get removed
    BeGuid removalId = m_cache->CreateRemovalId(CombinedHierarchyLevelInfo("invalid", 
        info.first.GetRulesetId(), info.first.GetLocale(), 0));
    m_cache->RemoveHierarchyLevel(removalId);
    EXPECT_TRUE(m_cache->FindHierarchyLevel(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), nullptr).IsValid());
    
    // attempt to remove with invalid ruleset id and verify it didn't get removed
    removalId = m_cache->CreateRemovalId(CombinedHierarchyLevelInfo(info.first.GetConnectionId(), 
        "invalid", info.first.GetLocale(), 0));
    m_cache->RemoveHierarchyLevel(removalId);
    EXPECT_TRUE(m_cache->FindHierarchyLevel(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), nullptr).IsValid());
    
    // attempt to remove with invalid locale and verify it didn't get removed
    removalId = m_cache->CreateRemovalId(CombinedHierarchyLevelInfo(info.first.GetConnectionId(), 
        info.first.GetRulesetId(), "invalid", 0));
    m_cache->RemoveHierarchyLevel(removalId);
    EXPECT_TRUE(m_cache->FindHierarchyLevel(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), nullptr).IsValid());
    
    // attempt to remove with invalid parent node and verify it didn't get removed
    removalId = m_cache->CreateRemovalId(CombinedHierarchyLevelInfo(info.first.GetConnectionId(), 
        info.first.GetRulesetId(), info.first.GetLocale(), 1));
    m_cache->RemoveHierarchyLevel(removalId);
    EXPECT_TRUE(m_cache->FindHierarchyLevel(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), nullptr).IsValid());

    // attempt to remove with valid parameters and verify it did get removed
    removalId = m_cache->CreateRemovalId(CombinedHierarchyLevelInfo(info.first.GetConnectionId(), 
        info.first.GetRulesetId(), info.first.GetLocale(), 0));
    m_cache->RemoveHierarchyLevel(removalId);
    EXPECT_FALSE(m_cache->FindHierarchyLevel(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), nullptr).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemoveHierarchyLevel_RemovesChildHierarchyLevel)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(rootInfo, 1);
    auto childInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", nodes[0]->GetNodeId());

    // verify child data source is cached
    EXPECT_TRUE(m_cache->GetHierarchyLevel(childInfo.first).IsValid());

    // remove child data source
    BeGuid removalId = m_cache->CreateRemovalId(CombinedHierarchyLevelInfo(childInfo.first.GetConnectionId(), 
        childInfo.first.GetRulesetId(), childInfo.first.GetLocale(), nodes[0]->GetNodeId()));
    m_cache->RemoveHierarchyLevel(removalId);

    // verify child data source also got removed
    EXPECT_FALSE(m_cache->GetHierarchyLevel(childInfo.first).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemoveHierarchyLevel_RemovesChildNodesWhenParentHierarchyLevelIsRemoved)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(rootInfo, 1);

    // verify node is cached
    EXPECT_TRUE(m_cache->GetNode(nodes[0]->GetNodeId()).IsValid());

    // remove child data source
    BeGuid removalId = m_cache->CreateRemovalId(CombinedHierarchyLevelInfo(rootInfo.first.GetConnectionId(), 
        rootInfo.first.GetRulesetId(), rootInfo.first.GetLocale(), 0));
    m_cache->RemoveHierarchyLevel(removalId);

    // verify child data source also got removed
    EXPECT_FALSE(m_cache->GetNode(nodes[0]->GetNodeId()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemovesChildDataSourcesWhenParentDataSourceIsRemoved)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(rootInfo, 1);
    auto childInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", nodes[0]->GetNodeId());

    // verify child data source is cached
    EXPECT_TRUE(m_cache->GetHierarchyLevel(childInfo.first).IsValid());

    // remove root data source
    BeGuid removalId = m_cache->CreateRemovalId(CombinedHierarchyLevelInfo(rootInfo.first.GetConnectionId(), 
        rootInfo.first.GetRulesetId(), rootInfo.first.GetLocale(), 0));
    m_cache->RemoveHierarchyLevel(removalId);

    // verify child data source also got removed
    EXPECT_FALSE(m_cache->GetHierarchyLevel(childInfo.first).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, MakePhysical)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(rootInfo, 1, false, true);

    // verify node is virtual
    EXPECT_EQ(NodeVisibility::Virtual, m_cache->GetNodeVisibility(nodes[0]->GetNodeId()));

    // make physical
    m_cache->MakePhysical(*nodes[0]);
    
    // verify node is physical
    EXPECT_EQ(NodeVisibility::Physical, m_cache->GetNodeVisibility(nodes[0]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, MakeVirtual)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(rootInfo, 1, false, false);

    // verify node is virtual
    EXPECT_EQ(NodeVisibility::Physical, m_cache->GetNodeVisibility(nodes[0]->GetNodeId()));

    // make physical
    m_cache->MakeVirtual(*nodes[0]);
    
    // verify node is physical
    EXPECT_EQ(NodeVisibility::Virtual, m_cache->GetNodeVisibility(nodes[0]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, UpdateNode)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(rootInfo, 1);
    
    // verify node's label is valid
    EXPECT_STREQ("0", m_cache->GetNode(nodes[0]->GetNodeId())->GetLabel().c_str());

    // update
    nodes[0]->SetLabel("B");
    m_cache->Update(nodes[0]->GetNodeId(), *nodes[0]);
    
    // verify node's label has changed
    EXPECT_STREQ("B", m_cache->GetNode(nodes[0]->GetNodeId())->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, UpdateNode_IsExpandedFlag)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(rootInfo, 1);
    uint64_t nodeId = nodes[0]->GetNodeId();
    ASSERT_TRUE(m_cache->IsNodeCached(nodeId));

    NavNodeCPtr cachedNode = m_cache->GetNode(nodeId);
    ASSERT_TRUE(cachedNode.IsValid());
    EXPECT_FALSE(cachedNode->IsExpanded());

    nodes[0]->SetIsExpanded(true);
    m_cache->Update(nodeId, *nodes[0]);
    cachedNode = m_cache->GetNode(nodeId);
    ASSERT_TRUE(cachedNode.IsValid());
    EXPECT_TRUE(cachedNode->IsExpanded());
    
    nodes[0]->SetIsExpanded(false);
    m_cache->Update(nodes[0]->GetNodeId(), *nodes[0]);
    cachedNode = m_cache->GetNode(nodeId);
    ASSERT_TRUE(cachedNode.IsValid());
    EXPECT_FALSE(cachedNode->IsExpanded());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ResetExpandedNodes)
    {
    // create root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);

    // create the root nodes
    bvector<JsonNavNodePtr> nodes = {
        TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test_A_test", "test descr"),
        TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test_B_test", "test descr")
        };
    for (JsonNavNodePtr node : nodes)
        node->SetIsExpanded(true);
    FillWithNodes(info, nodes);

    // expect all nodes to be expanded
    EXPECT_TRUE(m_cache->GetNode(nodes[0]->GetNodeId())->IsExpanded());
    EXPECT_TRUE(m_cache->GetNode(nodes[1]->GetNodeId())->IsExpanded());

    // reset
    m_cache->ResetExpandedNodes(m_connection->GetId().c_str(), info.first.GetRulesetId().c_str());
    
    // verify
    EXPECT_FALSE(m_cache->GetNode(nodes[0]->GetNodeId())->IsExpanded());
    EXPECT_FALSE(m_cache->GetNode(nodes[1]->GetNodeId())->IsExpanded());
    }

/*---------------------------------------------------------------------------------**//**
* note: the test uses GetRelatedHierarchyLevels function to test the Update
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, UpdateDataSource_UpdatesFilter)
    {
    ECClassCP widgetHasGadgetRelationship = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    // cache the data source
    HierarchyLevelInfo hlInfo(m_connection->GetId(), "ruleset_id", "locale", 0, 0);
    m_cache->Cache(hlInfo);
    EXPECT_TRUE(hlInfo.IsValid());

    DataSourceInfo dsInfo(hlInfo.GetId(), { 0 });
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[ECClassId((uint64_t)1)] = false;
    m_cache->Cache(dsInfo, DataSourceFilter(), usedClassIds, bvector<UserSettingEntry>());
    EXPECT_TRUE(dsInfo.IsValid());

    // verify the filter is not applied and we find related hierarchy level
    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(hlInfo.GetConnectionId(), keys);
    EXPECT_EQ(1, related.size());

    // update the datasource with new filter
    bset<ECClassId> relationshipClassIds;
    relationshipClassIds.insert(widgetHasGadgetRelationship->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipClassIds, RequiredRelationDirection_Both, ECInstanceId((uint64_t)123)), nullptr);
    m_cache->Update(dsInfo, &filter, nullptr, nullptr);
    
    // verify the filter is applied and we dont find related hierarchy level anymore
    related = m_cache->GetRelatedHierarchyLevels(hlInfo.GetConnectionId(), keys);
    EXPECT_EQ(0, related.size());
    }

/*---------------------------------------------------------------------------------**//**
* note: the test uses GetRelatedHierarchyLevels function to test the Update
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, UpdateDataSource_UpdatesRelatedClassIds)
    {
    // cache the data source
    HierarchyLevelInfo hlInfo(m_connection->GetId(), "ruleset_id", "locale", 0, 0);
    m_cache->Cache(hlInfo);
    EXPECT_TRUE(hlInfo.IsValid());

    DataSourceInfo dsInfo(hlInfo.GetId(), { 0 });
    m_cache->Cache(dsInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    EXPECT_TRUE(dsInfo.IsValid());
    
    // verify we don't find related hierarchy level as the key is not related to the data source
    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(hlInfo.GetConnectionId(), keys);
    EXPECT_EQ(0, related.size());

    // update the datasource related class ids
    bmap<ECN::ECClassId, bool> relatedClassIds;
    relatedClassIds[ECN::ECClassId((uint64_t)1)] = false;
    m_cache->Update(dsInfo, nullptr, &relatedClassIds, nullptr);
    
    // verify we do find related hierarchy level this time as the data source is now related to the lookup keys
    related = m_cache->GetRelatedHierarchyLevels(hlInfo.GetConnectionId(), keys);
    EXPECT_EQ(1, related.size());
    }

/*---------------------------------------------------------------------------------**//**
* note: the test uses GetRelatedHierarchyLevels function to test the Update
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, UpdateDataSource_UpdatesRelatedSettings)
    {
    // cache the data source
    HierarchyLevelInfo hlInfo(m_connection->GetId(), "ruleset_id", "locale", 0, 0);
    m_cache->Cache(hlInfo);
    EXPECT_TRUE(hlInfo.IsValid());

    DataSourceInfo dsInfo(hlInfo.GetId(), { 0 });
    m_cache->Cache(dsInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    EXPECT_TRUE(dsInfo.IsValid());
        
    // verify we don't find related hierarchy level as data source is not related to any setting ids
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(hlInfo.GetRulesetId().c_str(), "setting_id");
    EXPECT_EQ(0, related.size());

    // update the datasource related settings
    bvector<UserSettingEntry> settings = {UserSettingEntry("setting_id", Json::Value("value0"))};
    m_cache->Update(dsInfo, nullptr, nullptr, &settings);
    
    // verify we do find related hierarchy level this time as the data source is now related to the setting id
    related = m_cache->GetRelatedHierarchyLevels(hlInfo.GetRulesetId().c_str(), "setting_id");
    EXPECT_EQ(1, related.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemapNodeIds_RemapsDataSourcesWhenParentIsPhysical)
    {
    // create 2 root nodes
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto rootNodes = FillWithNodes(rootInfo, 2, false);

    // create a data source for root node 0
    auto childInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodes[0]->GetNodeId());
    
    // verify rootNodes[0] has a datasource and rootNodes[1] doesnt
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(rootNodes[0]->GetNodeId()));
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(rootNodes[1]->GetNodeId()));
    
    // remap
    bmap<uint64_t, uint64_t> remapInfo;
    remapInfo.Insert(rootNodes[0]->GetNodeId(), rootNodes[1]->GetNodeId());
    m_cache->RemapNodeIds(remapInfo);
    
    // verify rootNodes[1] has a datasource and rootNodes[0] doesnt
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(rootNodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(rootNodes[1]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemapNodeIds_RemapsDataSourcesWhenParentIsVirtual)
    {
    // create 2 root nodes
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto rootNodes = FillWithNodes(rootInfo, 2, false);
    m_cache->MakeVirtual(*rootNodes[0]);

    // create a data source for root node 0
    auto childInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodes[0]->GetNodeId());

    // verify rootNodes[0] has a datasource and rootNodes[1] doesnt
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(rootNodes[0]->GetNodeId()));
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(rootNodes[1]->GetNodeId()));
    
    // remap
    bmap<uint64_t, uint64_t> remapInfo;
    remapInfo.Insert(rootNodes[0]->GetNodeId(), rootNodes[1]->GetNodeId());
    m_cache->RemapNodeIds(remapInfo);
    
    // verify rootNodes[1] has a datasource and rootNodes[0] doesnt
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(rootNodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(rootNodes[1]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetFilteredNodes_FindsRootNode)
    {
    // create root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);

    // create the root nodes
    bvector<JsonNavNodePtr> nodes = {
        TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test_A_test", "test descr"),
        TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test_B_test", "test descr")
        };
    FillWithNodes(info, nodes);

    // filter
    bvector<NavNodeCPtr> filteredNodes = m_cache->GetFilteredNodes(*m_connection, info.first.GetRulesetId().c_str(), "locale", "A");

    // verify filtered node
    ASSERT_EQ(1, filteredNodes.size());
    EXPECT_TRUE(nodes[0]->Equals(*filteredNodes[0]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetFilteredNodes_DoesntFindNodesFromDifferentLocales)
    {
    // create root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale1", 0);

    // create the root nodes
    bvector<JsonNavNodePtr> nodes = {
        TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test_A_test", "test descr")
        };
    FillWithNodes(info, nodes);

    // filter
    bvector<NavNodeCPtr> filteredNodes = m_cache->GetFilteredNodes(*m_connection, info.first.GetRulesetId().c_str(), "locale2", "A");
    ASSERT_EQ(0, filteredNodes.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetUndeterminedNodesProvider_ReturnsNodeWhichDoesntHaveChildHierarchyLevel)
    {
    // create root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(info, 1);

    // expect the node to have undetermined children
    EXPECT_FALSE(m_cache->GetNode(nodes[0]->GetNodeId())->DeterminedChildren());

    // expect provider to contain the node
    NavNodesProviderPtr provider = m_cache->GetUndeterminedNodesProvider(*m_connection, 
        info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), false);
    ASSERT_TRUE(provider.IsValid());
    EXPECT_EQ(1, provider->GetNodesCount());

    JsonNavNodePtr expectedNode;
    provider->GetNode(expectedNode, 0);
    EXPECT_EQ(expectedNode->GetNodeId(), nodes[0]->GetNodeId());
    EXPECT_TRUE(expectedNode->DeterminedChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetUndeterminedNodesProvider_ReturnsNodeWhichHasUnitializedChildHierarchyLevel)
    {
    // create root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(info, 1);
    CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", nodes[0]->GetNodeId(), false);
        
    // expect the node to have undetermined children
    EXPECT_FALSE(m_cache->GetNode(nodes[0]->GetNodeId())->DeterminedChildren());

    // expect provider to contain the node
    NavNodesProviderPtr provider = m_cache->GetUndeterminedNodesProvider(*m_connection, info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), false);
    ASSERT_TRUE(provider.IsValid());
    ASSERT_EQ(1, provider->GetNodesCount());

    JsonNavNodePtr expectedNode;
    provider->GetNode(expectedNode, 0);
    EXPECT_EQ(expectedNode->GetNodeId(), nodes[0]->GetNodeId());
    EXPECT_TRUE(expectedNode->DeterminedChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetUndeterminedNodesProvider_DoesNotReturnNodeThatHasInitializedHierarchyLevel)
    {
    // create root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(info, 1, true);

    // expect provider to be empty
    NavNodesProviderPtr provider = m_cache->GetUndeterminedNodesProvider(*m_connection, 
        info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), false);
    ASSERT_TRUE(provider.IsValid());
    EXPECT_EQ(0, provider->GetNodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsNullsWhenEmpty)
    {
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(m_connection->GetId(), "test ruleset id", "test locale"));
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(999));
    EXPECT_FALSE(m_cache->IsNodeCached(999));
    
    EXPECT_FALSE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), "test ruleset id", "test locale", nullptr).IsValid());
    EXPECT_FALSE(m_cache->FindHierarchyLevel(1).IsValid());
    EXPECT_FALSE(m_cache->FindDataSource(1, { 0 }).IsValid());
    EXPECT_FALSE(m_cache->FindDataSource(999).IsValid());

    IGNORE_BE_ASSERT();

    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(CombinedHierarchyLevelInfo(m_connection->GetId(), "test ruleset id", "test locale", 999)).IsNull());
    EXPECT_TRUE(m_cache->GetHierarchyLevel(HierarchyLevelInfo(1, m_connection->GetId(), "test ruleset id", "test locale", 0, 0)).IsNull());
    EXPECT_TRUE(m_cache->GetDataSource(DataSourceInfo(1, { 0 })).IsNull());
    EXPECT_TRUE(m_cache->GetDataSource(999).IsNull());
    EXPECT_TRUE(m_cache->GetNode(999).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedRootDataSource)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");

    // verify the data source is cached
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(m_connection->GetId(), info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str()));
    
    EXPECT_TRUE(m_cache->FindHierarchyLevel(m_connection->GetId().c_str(), info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), nullptr).IsValid());
    EXPECT_TRUE(m_cache->FindHierarchyLevel(info.first.GetId()).IsValid());
    EXPECT_TRUE(m_cache->FindDataSource(info.first.GetId(), { 0 }).IsValid());

    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(CombinedHierarchyLevelInfo(m_connection->GetId(), info.first.GetRulesetId(), info.first.GetLocale(), 0)).IsValid());
    EXPECT_TRUE(m_cache->GetHierarchyLevel(info.first).IsValid());
    EXPECT_TRUE(m_cache->GetDataSource(info.second).IsValid());
    
    // verify it's not found when looking with invalid parameters
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached("invalid", info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str()));
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(CombinedHierarchyLevelInfo("invalid", info.first.GetRulesetId(), info.first.GetLocale(), 0)).IsNull());
    
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(m_connection->GetId(), "invalid", info.first.GetLocale().c_str()));
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(CombinedHierarchyLevelInfo(m_connection->GetId(), "invalid", info.first.GetLocale(), 0)).IsNull());
    
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(m_connection->GetId(), info.first.GetRulesetId().c_str(), "invalid"));
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(CombinedHierarchyLevelInfo(m_connection->GetId(), info.first.GetRulesetId(), "invalid", 0)).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedRootNode)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");

    // create a node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    InitNode(*node, info.first);

    // the root data source should be empty
    NavNodesProviderPtr cachedProvider = m_cache->GetCombinedHierarchyLevel(CombinedHierarchyLevelInfo(m_connection->GetId(), 
        info.first.GetRulesetId(), info.first.GetLocale(), 0));
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(0, cachedProvider->GetNodesCount());
    
    // the node's data source should also be empty
    cachedProvider = m_cache->GetDataSource(node->GetNodeId());
    ASSERT_FALSE(cachedProvider.IsValid());

    // cache the node
    m_cache->Cache(*node, info.second, 0, false);

    // verify the node is cached
    cachedProvider = m_cache->GetCombinedHierarchyLevel(CombinedHierarchyLevelInfo(m_connection->GetId(), 
        info.first.GetRulesetId(), info.first.GetLocale(), 0));
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(1, cachedProvider->GetNodesCount());

    EXPECT_TRUE(m_cache->IsNodeCached(node->GetNodeId()));
    EXPECT_TRUE(m_cache->GetNode(node->GetNodeId()).IsValid());

    // verify node's datasource is not empty
    cachedProvider = m_cache->GetDataSource(node->GetNodeId());
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(1, cachedProvider->GetNodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedChildDataSource)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(info, 1);
    uint64_t nodeId = nodes[0]->GetNodeId();

    // child data source should not exist
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(nodeId));
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(CombinedHierarchyLevelInfo(m_connection->GetId(), 
        info.first.GetRulesetId(), info.first.GetLocale(), nodeId)).IsNull());

    // cache child data source
    auto childrenInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", nodeId);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodeId));
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(childrenInfo.first).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedChildNode)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(info, 1);
    uint64_t nodeId = nodes[0]->GetNodeId();
    
    // cache child data source
    auto childrenInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", nodeId);

    // the data source should be empty
    NavNodesProviderPtr cachedProvider = m_cache->GetCombinedHierarchyLevel(CombinedHierarchyLevelInfo(m_connection->GetId(), 
        info.first.GetRulesetId(), info.first.GetLocale(), nodeId));
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(0, cachedProvider->GetNodesCount());

    // cache child node
    auto childNodes = FillWithNodes(childrenInfo, 1);
    
    // verify the child node is cached
    cachedProvider = m_cache->GetCombinedHierarchyLevel(CombinedHierarchyLevelInfo(m_connection->GetId(), 
        info.first.GetRulesetId(), info.first.GetLocale(), nodeId));
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(1, cachedProvider->GetNodesCount());

    EXPECT_TRUE(m_cache->IsNodeCached(childNodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->GetNode(childNodes[0]->GetNodeId()).IsValid());

    // verify we can get its parent data source
    cachedProvider = m_cache->GetDataSource(childNodes[0]->GetNodeId());
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(1, cachedProvider->GetNodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetNodeVisibility_ReturnsPhysicalNodeVisibility)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(info, 1);
    
    // verify results
    EXPECT_EQ(NodeVisibility::Physical, m_cache->GetNodeVisibility(nodes[0]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetNodeVisibility_ReturnsVirtualNodeVisibility)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale");
    auto nodes = FillWithNodes(info, 1, false, true);
    
    // verify results
    EXPECT_EQ(NodeVisibility::Virtual, m_cache->GetNodeVisibility(nodes[0]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetHierarchyLevel_ReturnsDataSourcesFromValidConnections)
    {
    // create a new connection
    ECDbTestProject project2;
    project2.Create("test2");
    IConnectionPtr connection2 = m_connections.NotifyConnectionOpened(project2.GetECDb());
    
    // cache root data source for the first connection
    auto info1 = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    
    // cache root data source for the second connection
    auto info2 = CacheDataSource(connection2->GetId(), "ruleset_id", "locale", 0);

    // verify correct sources are returned
    NavNodesProviderPtr cached1 = m_cache->GetHierarchyLevel(info1.first);
    ASSERT_TRUE(cached1.IsValid());
    EXPECT_EQ(m_connection.get(), &cached1->GetContext().GetConnection());

    NavNodesProviderPtr cached2 = m_cache->GetHierarchyLevel(info2.first);
    ASSERT_TRUE(cached2.IsValid());
    EXPECT_EQ(connection2.get(), &cached2->GetContext().GetConnection());

    m_connections.NotifyConnectionClosed(*connection2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetHierarchyLevel_ReturnsDataSourcesWithValidRulesetIds)
    {    
    // cache root data source for the first ruleset
    m_cache->OnRulesetCreated(*PresentationRuleSet::CreateInstance("ruleset_id1", 1, 0, false, "", "", "", false));
    auto info1 = CacheDataSource(m_connection->GetId(), "ruleset_id1", "locale", 0);
    
    // cache root data source for the second ruleset
    m_cache->OnRulesetCreated(*PresentationRuleSet::CreateInstance("ruleset_id2", 1, 0, false, "", "", "", false));
    auto info2 = CacheDataSource(m_connection->GetId(), "ruleset_id2", "locale", 0);

    // verify correct sources are returned
    NavNodesProviderPtr cached1 = m_cache->GetHierarchyLevel(info1.first);
    ASSERT_TRUE(cached1.IsValid());
    EXPECT_STREQ(info1.first.GetRulesetId().c_str(), cached1->GetContext().GetRuleset().GetRuleSetId().c_str());

    NavNodesProviderPtr cached2 = m_cache->GetHierarchyLevel(info2.first);
    ASSERT_TRUE(cached2.IsValid());
    EXPECT_STREQ(info2.first.GetRulesetId().c_str(), cached2->GetContext().GetRuleset().GetRuleSetId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetHierarchyLevel_ReturnsDataSourcesWithValidLocales)
    {    
    // cache root data source for the first locale
    auto info1 = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale1", 0);
    
    // cache root data source for the second locale
    auto info2 = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale2", 0);

    // verify correct sources are returned
    NavNodesProviderPtr cached1 = m_cache->GetHierarchyLevel(info1.first);
    ASSERT_TRUE(cached1.IsValid());
    EXPECT_STREQ(info1.first.GetLocale().c_str(), cached1->GetContext().GetLocale().c_str());

    NavNodesProviderPtr cached2 = m_cache->GetHierarchyLevel(info2.first);
    ASSERT_TRUE(cached2.IsValid());
    EXPECT_STREQ(info2.first.GetLocale().c_str(), cached2->GetContext().GetLocale().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, HasParentNode_ReturnsFalseForEmptySet)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);

    // cache root node
    auto nodes = FillWithNodes(info, 1);

    bset<uint64_t> parentNodeIds;
    EXPECT_FALSE(m_cache->HasParentNode(nodes[0]->GetNodeId(), parentNodeIds));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, HasParentNode_ReturnsFalseForRootNode)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);

    // cache root node
    auto nodes = FillWithNodes(info, 1);
    
    bset<uint64_t> parentNodeIds;
    parentNodeIds.insert(nodes[0]->GetNodeId() + 1);
    EXPECT_FALSE(m_cache->HasParentNode(nodes[0]->GetNodeId(), parentNodeIds));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, HasParentNode_ReturnsFalseIfListDoesntContainParentNodeId)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);

    // cache root node
    auto rootNodes = FillWithNodes(info, 2);
    
    // cache child node for root node 0
    auto childNodesInfo = CacheDataSource(info.first.GetConnectionId().c_str(),
        info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), rootNodes[0]->GetNodeId());
    auto childNodes = FillWithNodes(childNodesInfo, 1);

    // cache child data source for root node 1
    CacheDataSource(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(),
        info.first.GetLocale().c_str(), rootNodes[1]->GetNodeId());
    
    bset<uint64_t> parentNodeIds;
    parentNodeIds.insert(rootNodes[1]->GetNodeId());
    EXPECT_FALSE(m_cache->HasParentNode(childNodes[0]->GetNodeId(), parentNodeIds));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, HasParentNode_ReturnsTrueIfListContainsImmediateParentNodeId)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);

    // cache root node
    auto rootNodes = FillWithNodes(info, 2);
    
    // cache child node for root node 0
    auto childNodesInfo = CacheDataSource(info.first.GetConnectionId().c_str(),
        info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), rootNodes[0]->GetNodeId());
    auto childNodes = FillWithNodes(childNodesInfo, 1);

    // cache child data source for root node 1
    CacheDataSource(info.first.GetConnectionId().c_str(), info.first.GetRulesetId().c_str(),
        info.first.GetLocale().c_str(), rootNodes[1]->GetNodeId());
    
    bset<uint64_t> parentNodeIds;
    parentNodeIds.insert(rootNodes[0]->GetNodeId());
    EXPECT_TRUE(m_cache->HasParentNode(childNodes[0]->GetNodeId(), parentNodeIds));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, HasParentNode_ReturnsTrueIfListContainsGrandParentNodeId)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);

    // cache root node
    auto rootNodes = FillWithNodes(info, 1);
    
    // cache child node for the root node
    auto childNodesInfo = CacheDataSource(info.first.GetConnectionId().c_str(),
        info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), rootNodes[0]->GetNodeId());
    auto childNodes = FillWithNodes(childNodesInfo, 1);

    // cache grandchild node for child node
    auto grandchildNodesInfo = CacheDataSource(info.first.GetConnectionId().c_str(),
        info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str(), childNodes[0]->GetNodeId());
    auto grandchildNodes = FillWithNodes(grandchildNodesInfo, 1);
    
    bset<uint64_t> parentNodeIds;
    parentNodeIds.insert(rootNodes[0]->GetNodeId());
    EXPECT_TRUE(m_cache->HasParentNode(grandchildNodes[0]->GetNodeId(), parentNodeIds));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECInstanceNode)
    {
    // create a similar node in a closed connection
    ECDbTestProject project;
    project.Create(BeTest::GetNameOfCurrentTest(), "RulesEngineTest.01.00.ecschema.xml");
    IConnectionPtr connection2 = m_connections.NotifyConnectionOpened(project.GetECDb());
    ECClassCP widgetClass2 = project.GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
    auto diffConnectionInfo = CacheDataSource(connection2->GetId(), "ruleset_id", "locale", 0);
    bvector<JsonNavNodePtr> diffConnectionNodes = {
        TestNodesHelper::CreateInstanceNode(*connection2, *widgetClass2)
        };
    FillWithNodes(diffConnectionInfo, diffConnectionNodes);
    m_connections.NotifyConnectionClosed(*connection2);
    project.GetECDb().CloseDb();

    // create a node in an open connection
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bvector<JsonNavNodePtr> nodes = {
        TestNodesHelper::CreateInstanceNode(*m_connection, *widgetClass)
        };
    FillWithNodes(info, nodes);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "locale", *nodes[0]->GetKey());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when key is invalid
    ECClassInstanceKey invalidKey(
        nodes[0]->GetKey()->AsECInstanceNodeKey()->GetECClass(),
        ECInstanceId(nodes[0]->GetKey()->AsECInstanceNodeKey()->GetInstanceId().GetValue() + 100));
    locatedNode = m_cache->LocateNode(*m_connection, "locale", *ECInstanceNodeKey::Create(invalidKey, { "wrong hash" }));
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECInstanceNode_FromCorrectRuleset)
    {
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");

    // create a node with ruleset 1
    m_cache->OnRulesetCreated(*PresentationRuleSet::CreateInstance("ruleset_id_1", 1, 0, false, "", "", "", false));
    auto info1 = CacheDataSource(m_connection->GetId(), "ruleset_id_1", "locale", 0);
    JsonNavNodePtr node1 = TestNodesHelper::CreateInstanceNode(*m_connection, *widgetClass);
    NavNodeExtendedData(*node1).SetRulesetId(info1.first.GetRulesetId().c_str());
    FillWithNodes(info1, { node1 });

    // create a node with ruleset 2
    m_cache->OnRulesetCreated(*PresentationRuleSet::CreateInstance("ruleset_id_2", 1, 0, false, "", "", "", false));
    auto info2 = CacheDataSource(m_connection->GetId(), "ruleset_id_2", "locale", 0);
    JsonNavNodePtr node2 = TestNodesHelper::CreateInstanceNode(*m_connection, *widgetClass);
    NavNodeExtendedData(*node2).SetRulesetId(info2.first.GetRulesetId().c_str());
    FillWithNodes(info2, { node2 });

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "locale", *node1->GetKey());
    ASSERT_TRUE(locatedNode.IsValid());
    EXPECT_TRUE(node1->Equals(*locatedNode));

    locatedNode = m_cache->LocateNode(*m_connection, "locale", *node2->GetKey());
    ASSERT_TRUE(locatedNode.IsValid());
    EXPECT_TRUE(node2->Equals(*locatedNode));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECClassGroupingNode)
    {
    // create a similar node in a closed connection
    ECDbTestProject project;
    project.Create("LocateNode_LocatesECClassGroupingNode", "RulesEngineTest.01.00.ecschema.xml");
    IConnectionPtr connection2 = m_connections.NotifyConnectionOpened(project.GetECDb());
    ECClassCP widgetClass2 = project.GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
    auto diffConnectionInfo = CacheDataSource(connection2->GetId(), "ruleset_id", "locale", 0);
    bvector<JsonNavNodePtr> diffConnectionNodes = {
        TestNodesHelper::CreateClassGroupingNode(*m_connection, *widgetClass2, "test label")
        };
    FillWithNodes(diffConnectionInfo, diffConnectionNodes);
    m_connections.NotifyConnectionClosed(*connection2);
    project.GetECDb().CloseDb();

    // create a node in an open connection
    ECClassCP widgetClass1 = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bvector<JsonNavNodePtr> nodes = {
        TestNodesHelper::CreateClassGroupingNode(*m_connection, *widgetClass1, "test label")
        };
    FillWithNodes(info, nodes);
    
    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "locale", *nodes[0]->GetKey());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when node hash doesnt match
    locatedNode = m_cache->LocateNode(*m_connection, "locale", *ECClassGroupingNodeKey::Create(*widgetClass1, {"different hash"}, 1));
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECPropertyValueGroupingNode)
    {
    rapidjson::Document groupingValue;
    groupingValue.SetInt(9);

    // create a similar node in a closed connection
    ECDbTestProject project;
    project.Create("LocateNode_LocatesECPropertyValueGroupingNode", "RulesEngineTest.01.00.ecschema.xml");
    IConnectionPtr connection2 = m_connections.NotifyConnectionOpened(project.GetECDb());
    ECClassCP widgetClass2 = project.GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECPropertyCP groupingProperty2 = widgetClass2->GetPropertyP("IntProperty");
    auto diffConnectionInfo = CacheDataSource(connection2->GetId(), "ruleset_id", "locale", 0);
    bvector<JsonNavNodePtr> diffConnectionNodes = {
        TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *widgetClass2, *groupingProperty2, "test label", groupingValue, false)
        };
    FillWithNodes(diffConnectionInfo, diffConnectionNodes);
    m_connections.NotifyConnectionClosed(*connection2);
    project.GetECDb().CloseDb();

    // create a node in an open connection
    ECClassCP widgetClass1 = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECPropertyCP groupingProperty1 = widgetClass1->GetPropertyP("IntProperty");
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bvector<JsonNavNodePtr> nodes = {
        TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *widgetClass1, *groupingProperty1, "test label", groupingValue, false)
        };
    FillWithNodes(info, nodes);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "locale", *nodes[0]->GetKey());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when node hash doesnt match
    locatedNode = m_cache->LocateNode(*m_connection, "locale", *ECPropertyGroupingNodeKey::Create(*widgetClass1, "IntProperty", &groupingValue, {"different hash"}, 1));
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECPropertyRangeGroupingNode)
    {
    rapidjson::Document groupingValue;
    groupingValue.SetInt(2);

    // create a similar node in a closed connection
    ECDbTestProject project;
    project.Create("LocateNode_LocatesECPropertyRangeGroupingNode", "RulesEngineTest.01.00.ecschema.xml");
    IConnectionPtr connection2 = m_connections.NotifyConnectionOpened(project.GetECDb());
    ECClassCP widgetClass2 = project.GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECPropertyCP groupingProperty2 = widgetClass2->GetPropertyP("IntProperty");
    auto diffConnectionInfo = CacheDataSource(connection2->GetId(), "ruleset_id", "locale", 0);
    bvector<JsonNavNodePtr> diffConnectionNodes = {
        TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *widgetClass2, *groupingProperty2, "test label", groupingValue, true)
        };
    FillWithNodes(diffConnectionInfo, diffConnectionNodes);
    m_connections.NotifyConnectionClosed(*connection2);
    project.GetECDb().CloseDb();

    // create a node in an open connection
    ECClassCP widgetClass1 = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECPropertyCP groupingProperty1 = widgetClass1->GetPropertyP("IntProperty");
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bvector<JsonNavNodePtr> nodes = {
        TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *widgetClass1, *groupingProperty1, "test label", groupingValue, true)
        };
    FillWithNodes(info, nodes);
    
    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "locale", *nodes[0]->GetKey());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when node hash doesnt match
    locatedNode = m_cache->LocateNode(*m_connection, "locale", *ECPropertyGroupingNodeKey::Create(*widgetClass1, "IntProperty", &groupingValue, {"different hash"}, 1));
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesLabelGroupingNode)
    {
    // create a similar node in a closed connection
    ECDbTestProject project;
    project.Create("LocateNode_LocatesLabelGroupingNode");
    IConnectionPtr connection2 = m_connections.NotifyConnectionOpened(project.GetECDb());
    auto diffConnectionInfo = CacheDataSource(connection2->GetId(), "ruleset_id", "locale", 0);
    bvector<JsonNavNodePtr> diffConnectionNodes = {
        TestNodesHelper::CreateLabelGroupingNode(*m_connection, "test label")
        };
    FillWithNodes(diffConnectionInfo, diffConnectionNodes);
    m_connections.NotifyConnectionClosed(*connection2);
    project.GetECDb().CloseDb();

    // create a node in an open connection
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bvector<JsonNavNodePtr> nodes = {
        TestNodesHelper::CreateLabelGroupingNode(*m_connection, "test label")
        };
    FillWithNodes(info, nodes);
    
    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "locale", *nodes[0]->GetKey());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when node hash doesnt match
    locatedNode = m_cache->LocateNode(*m_connection, "locale", *NavNodeKey::Create("type", {"wrong hash"}));
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesCustomNode)
    {
    // create a similar node in a closed connection
    ECDbTestProject project;
    project.Create("LocateNode_LocatesCustomNode");
    IConnectionPtr connection2 = m_connections.NotifyConnectionOpened(project.GetECDb());
    auto diffConnectionInfo = CacheDataSource(connection2->GetId(), "ruleset_id", "locale", 0);
    bvector<JsonNavNodePtr> diffConnectionNodes = {
        TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test label", "test descr")
        };
    FillWithNodes(diffConnectionInfo, diffConnectionNodes);
    m_connections.NotifyConnectionClosed(*connection2);
    project.GetECDb().CloseDb();

    // create a node in an open connection
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bvector<JsonNavNodePtr> nodes = {
        TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test label", "test descr")
        };
    FillWithNodes(info, nodes);
    
    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "locale", *nodes[0]->GetKey());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(nodes[0]->Equals(*locatedNode));

    // verify the node is not found when node hash doesnt matchSetInstanceId
    locatedNode = m_cache->LocateNode(*m_connection, "locale", *NavNodeKey::Create("type", {"wrong hash"}));
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Quick_AddsToQuickCacheWhenDataSourceReachesRequiredSize)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    auto rootNodes = FillWithNodes(info, 2);
    bvector<uint64_t> rootNodeIds;
    std::transform(rootNodes.begin(), rootNodes.end(), std::back_inserter(rootNodeIds), [](NavNodeCPtr node){return node->GetNodeId();});

    // cache child data source 1 (less than required size to add to quick cache)
    auto childrenInfo1 = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodeIds[0]);
    FillWithNodes(childrenInfo1, NODESCACHE_QUICK_Boundary);
    NavNodesProviderPtr datasource1 = m_cache->GetCombinedHierarchyLevel(CombinedHierarchyLevelInfo(childrenInfo1.first));
    EXPECT_EQ(NODESCACHE_QUICK_Boundary, datasource1->GetNodesCount());

    // cache child data source 2 (of required size to add to quick cache)
    auto childrenInfo2 = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodeIds[1]);
    FillWithNodes(childrenInfo2, NODESCACHE_QUICK_Boundary + 1);
    NavNodesProviderPtr datasource2 = m_cache->GetCombinedHierarchyLevel(CombinedHierarchyLevelInfo(childrenInfo2.first));
    EXPECT_EQ(NODESCACHE_QUICK_Boundary + 1, datasource2->GetNodesCount());

    // attempt to add both providers to quick cache
    m_cache->CacheHierarchyLevel(childrenInfo1.first, *datasource1);
    m_cache->CacheHierarchyLevel(childrenInfo2.first, *datasource2);

    // verify only the second one got cached
    EXPECT_NE(datasource1.get(), m_cache->GetCombinedHierarchyLevel(childrenInfo1.first).get());
    EXPECT_EQ(datasource2.get(), m_cache->GetCombinedHierarchyLevel(childrenInfo2.first).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Quick_RemovesPreviousDataSourceIfNewOneIsAddedForTheSameHierarchyLevel)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    auto rootNodes = FillWithNodes(info, 2);
    uint64_t rootNodeId = rootNodes[0]->GetNodeId();

    // cache child data source
    auto childrenInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodeId);
    FillWithNodes(childrenInfo, NODESCACHE_QUICK_Boundary + 1);

    // both datasources point to the same data, but they're different instances
    NavNodesProviderPtr datasource1 = m_cache->GetCombinedHierarchyLevel(childrenInfo.first);
    NavNodesProviderPtr datasource2 = m_cache->GetCombinedHierarchyLevel(childrenInfo.first);

    // add the 1st provider to quick cache
    m_cache->CacheHierarchyLevel(childrenInfo.first, *datasource1);

    // verify 1st provider is in the quick cache, but not the 2nd one
    EXPECT_EQ(datasource1.get(), m_cache->GetCombinedHierarchyLevel(childrenInfo.first).get());
    EXPECT_NE(datasource2.get(), m_cache->GetCombinedHierarchyLevel(childrenInfo.first).get());
    
    // add the 2nd provider to quick cache
    m_cache->CacheHierarchyLevel(childrenInfo.first, *datasource2);
    
    // verify 2nd provider is in the quick cache, and the 1st one is removed
    EXPECT_NE(datasource1.get(), m_cache->GetCombinedHierarchyLevel(childrenInfo.first).get());
    EXPECT_EQ(datasource2.get(), m_cache->GetCombinedHierarchyLevel(childrenInfo.first).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Quick_RemovesLastUsedProvidersWhenMaxSizeIsReached)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    auto rootNodes = FillWithNodes(info, NODESCACHE_QUICK_Size + 1);
    bvector<uint64_t> rootNodeIds;
    std::transform(rootNodes.begin(), rootNodes.end(), std::back_inserter(rootNodeIds), [](NavNodeCPtr node){return node->GetNodeId();});

    // cache NODESCACHE_QUICK_Size providers
    bvector<NavNodesProviderPtr> providers;
    for (size_t i = 0; i < NODESCACHE_QUICK_Size; ++i)
        {
        auto childrenInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodeIds[i]);
        FillWithNodes(childrenInfo, NODESCACHE_QUICK_Boundary + 1);
        
        NavNodesProviderPtr provider = m_cache->GetCombinedHierarchyLevel(childrenInfo.first);
        m_cache->CacheHierarchyLevel(childrenInfo.first, *provider);
        providers.push_back(provider);
        }

    // verify all providers are in the quick cache
    for (size_t i = 0; i < providers.size(); ++i)
        {
        CombinedHierarchyLevelInfo providerInfo(info.first.GetConnectionId(), info.first.GetRulesetId(), info.first.GetLocale(), rootNodeIds[i]);
        EXPECT_EQ(providers[i].get(), m_cache->GetCombinedHierarchyLevel(providerInfo).get());
        }

    // add a new provider
    auto newInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodeIds[NODESCACHE_QUICK_Size]);
    FillWithNodes(newInfo, NODESCACHE_QUICK_Boundary + 1);
    NavNodesProviderPtr provider = m_cache->GetCombinedHierarchyLevel(newInfo.first);
    m_cache->CacheHierarchyLevel(newInfo.first, *provider);

    // verify the first cached provider is now removed from cache and the new one is inserted
    EXPECT_NE(providers[0].get(), m_cache->GetCombinedHierarchyLevel(CombinedHierarchyLevelInfo(info.first.GetConnectionId(), info.first.GetRulesetId(), info.first.GetLocale(), rootNodeIds[0])).get());
    for (size_t i = 1; i < providers.size(); ++i)
        EXPECT_EQ(providers[i].get(), m_cache->GetCombinedHierarchyLevel(CombinedHierarchyLevelInfo(info.first.GetConnectionId(), info.first.GetRulesetId(), info.first.GetLocale(), rootNodeIds[i])).get());
    EXPECT_EQ(provider.get(), m_cache->GetCombinedHierarchyLevel(CombinedHierarchyLevelInfo(info.first.GetConnectionId(), info.first.GetRulesetId(), info.first.GetLocale(), rootNodeIds[NODESCACHE_QUICK_Size])).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Settings_ReturnsEmptyListWhenRulesetAndSettingIdsAreInvalid)
    {
    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(info.first.GetConnectionId().c_str(),
        info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str()));

    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels("invalid", "any");
    EXPECT_TRUE(related.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Settings_ReturnsOnlyRelatedDataSources)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    auto rootNodes = FillWithNodes(rootInfo, 2);

    // cache unrelated datasource
    auto childInfo0 = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodes[0]->GetNodeId());
    bvector<UserSettingEntry> settings0 = {UserSettingEntry("setting_0", Json::Value("value0"))};
    m_cache->Update(childInfo0.second, nullptr, nullptr, &settings0);
    
    // cache related datasource
    auto childInfo1 = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodes[1]->GetNodeId());
    bvector<UserSettingEntry> settings1 = {UserSettingEntry("setting_1", Json::Value("value1")) };
    m_cache->Update(childInfo1.second, nullptr, nullptr, &settings1);

    // verify the correct datasource is found
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels("ruleset_id", "setting_1");
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(childInfo1.first, related[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsEmptyListWhenInstancesArentRelated)
    {
    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(info.first.GetConnectionId(), 
        info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str()));

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.first.GetConnectionId(), keys);
    EXPECT_TRUE(related.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsEmptyListWhenConnectionsAreDifferent)
    {
    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[ECClassId((uint64_t)1)] = false;
    m_cache->Update(info.second, nullptr, &usedClassIds, nullptr);
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(info.first.GetConnectionId(), 
        info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str()));

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels("does not exist", keys);
    EXPECT_TRUE(related.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenClassMatchesAndFilterIsNotSpecified)
    {
    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[ECClassId((uint64_t)1)] = false;
    m_cache->Update(info.second, nullptr, &usedClassIds, nullptr);
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(info.first.GetConnectionId(), 
        info.first.GetRulesetId().c_str(), info.first.GetLocale().c_str()));

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.first.GetConnectionId(), keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info.first);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsEmptyListWhenClassMatchesButFilterDoesnt)
    {
    ECClassCP widgetHasGadget = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadget->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Backward, ECInstanceId((uint64_t)123)), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[ECClassId((uint64_t)1)] = false;
    m_cache->Update(info.second, &filter, &usedClassIds, nullptr);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.first.GetConnectionId(), keys);
    EXPECT_TRUE(related.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenClassAndFilterMatchesByInstanceId)
    {
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECClassCP gadgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Gadget");
    ECClassCP widgetHasGadgetRelationship = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadgetRelationship->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Backward, ECInstanceId((uint64_t)123)), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[widgetClass->GetId()] = false;
    usedClassIds[gadgetClass->GetId()] = false;
    m_cache->Update(info.second, &filter, &usedClassIds, nullptr);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(gadgetClass->GetId(), filter.GetRelatedInstanceInfo()->m_instanceId));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.first.GetConnectionId(), keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info.first);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
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
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadgetRelationship->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Forward, widgetId), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[widgetClass->GetId()] = false;
    usedClassIds[gadgetClass->GetId()] = false;
    m_cache->Update(info.second, &filter, &usedClassIds, nullptr);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(gadgetClass->GetId(), gadgetId));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.first.GetConnectionId(), keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info.first);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
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
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadgetRelationship->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Backward, gadgetId), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[widgetClass->GetId()] = false;
    usedClassIds[gadgetClass->GetId()] = false;
    m_cache->Update(info.second, &filter, &usedClassIds, nullptr);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(widgetClass->GetId(), widgetId));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.first.GetConnectionId(), keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info.first);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
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
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadgetRelationship->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Both, gadgetId), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[widgetClass->GetId()] = false;
    usedClassIds[gadgetClass->GetId()] = false;
    m_cache->Update(info.second, &filter, &usedClassIds, nullptr);
    
    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(widgetClass->GetId(), widgetId));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.first.GetConnectionId(), keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info.first);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
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
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadgetRelationship->GetId());
    relationshipIds.insert(widgetHasGadgetsRelationship->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Backward, gadgetId), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[widgetClass->GetId()] = false;
    usedClassIds[gadgetClass->GetId()] = false;
    m_cache->Update(info.second, &filter, &usedClassIds, nullptr);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(widgetClass->GetId(), widgetId));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.first.GetConnectionId(), keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info.first);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsEmptyListWhenChildNodeGroupedInstanceKeysMatchByClassId)
    {
    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    
    // create a node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId("ruleset_id");
    extendedData.SetGroupedInstanceKey(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)2)));
    m_cache->Cache(*node, info.second, 0, false);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)2), ECInstanceId((uint64_t)2)));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.first.GetConnectionId(), keys);
    EXPECT_TRUE(related.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsEmptyListWhenChildNodeGroupedInstanceKeysMatchByInstanceId)
    {
    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    
    // create a node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId("ruleset_id");
    extendedData.SetGroupedInstanceKey(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)2)));
    m_cache->Cache(*node, info.second, 0, false);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.first.GetConnectionId(), keys);
    EXPECT_TRUE(related.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenChildNodeGroupedInstanceKeysMatch)
    {
    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    
    // create a node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId("ruleset_id");
    extendedData.SetGroupedInstanceKey(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)2)));
    m_cache->Cache(*node, info.second, 0, false);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)2)));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.first.GetConnectionId(), keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info.first);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenChildNodeGroupedInstanceKeysMatchButFilterDoesnt)
    {
    ECClassCP widgetHasGadget = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    // cache the data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadget->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Backward, ECInstanceId((uint64_t)123)), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[ECClassId((uint64_t)1)] = false;
    m_cache->Update(info.second, &filter, &usedClassIds, nullptr);

    // create a node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId("ruleset_id");
    extendedData.SetGroupedInstanceKey(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)2)));
    m_cache->Cache(*node, info.second, 0, false);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)2)));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.first.GetConnectionId(), keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info.first);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, AllowsUsingMultipleConnectionsToTheSameDbAtDifferentPaths)
    {
    // copy the dataset
    BeFileName name1(s_project->GetECDb().GetDbFileName(), true);
    BeFileName name2 = BeFileName(name1).AppendExtension(L"copy");
    name2.BeDeleteFile();
    BeFileName::BeCopyFile(name1.c_str(), name2.c_str());

    // make sure both datasets have the same mod time
    time_t fileModTime;
    name1.GetFileTime(nullptr, nullptr, &fileModTime);
    name2.SetFileTime(nullptr, &fileModTime);

    // open the second project
    ECDbTestProject project2;
    ASSERT_EQ(BE_SQLITE_OK, project2.Open(name2.GetNameUtf8().c_str()));
    IConnectionPtr connection2 = m_connections.NotifyConnectionOpened(project2.GetECDb());

    // cache root data source for the first connection
    auto info1 = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    
    // cache root data source for the second connection
    auto info2 = CacheDataSource(connection2->GetId(), "ruleset_id", "locale", 0);

    // verify both data sources got cached
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(m_connection->GetId(), "ruleset_id", "locale"));
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(connection2->GetId(), "ruleset_id", "locale"));

    m_connections.NotifyConnectionClosed(*connection2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetCombinedHierarchyLevel_ClearsRootDatasourceCacheIfRelatedSettingsValuesChanged)
    {
    m_userSettings.GetSettings("ruleset_id").SetSettingValue("setting_id", "value0");
    bvector<UserSettingEntry> settingsValues = {UserSettingEntry("setting_id", Json::Value("value0"))};
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    m_cache->Update(info.second, nullptr, nullptr, &settingsValues);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(info.first).IsValid());
    
    // verify that valid datasource is not returned with different user settings
    m_userSettings.GetSettings("ruleset_id").SetSettingValue("setting_id", "value1");

    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(info.first).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetCombinedHierarchyLevel_DoesntClearRootDatasourceCacheIfRelatedSettingsValuesDidntChange)
    {
    m_userSettings.GetSettings("ruleset_id").SetSettingIntValue("setting_id", 10);
    bvector<UserSettingEntry> settingsValues = {UserSettingEntry("setting_id", Json::Value(10))};
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    m_cache->Update(info.second, nullptr, nullptr, &settingsValues);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(info.first).IsValid());
    
    // verify what valid datasource is returned with same user settings
    m_userSettings.GetSettings("ruleset_id").SetSettingIntValue("setting_id", 10);

    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(info.first).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetCombinedHierarchyLevel_ClearsChildDatasourceCacheIfRelatedSettingsValuesChanged)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    auto rootNodes = FillWithNodes(rootInfo, 1);

    // cache child data source
    m_userSettings.GetSettings("ruleset_id").SetSettingIntValue("setting_id", 1);
    bvector<UserSettingEntry> settingsValues = {UserSettingEntry("setting_id", Json::Value(1))};
    auto childInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodes[0]->GetNodeId());
    m_cache->Update(childInfo.second, nullptr, nullptr, &settingsValues);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(childInfo.first).IsValid());

    // verify what valid datasource is not returned with different user settings
    m_userSettings.GetSettings("ruleset_id").SetSettingIntValue("setting_id", 10);
    
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(childInfo.first).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetHierarchyLevel_ClearsRootDatasourceCacheIfRelatedSettingsValuesChanged)
    {
    m_userSettings.GetSettings("ruleset_id").SetSettingValue("setting_id", "value0");
    bvector<UserSettingEntry> settingsValues = {UserSettingEntry("setting_id", Json::Value("value0"))};
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    m_cache->Update(info.second, nullptr, nullptr, &settingsValues);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->GetHierarchyLevel(info.first).IsValid());
    
    // verify that valid datasource is not returned with different user settings
    m_userSettings.GetSettings("ruleset_id").SetSettingValue("setting_id", "value1");

    EXPECT_FALSE(m_cache->GetHierarchyLevel(info.first).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetHierarchyLevel_DoesntClearRootDatasourceCacheIfRelatedSettingsValuesDidntChange)
    {
    m_userSettings.GetSettings("ruleset_id").SetSettingIntValue("setting_id", 10);
    bvector<UserSettingEntry> settingsValues = {UserSettingEntry("setting_id", Json::Value(10))};
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    m_cache->Update(info.second, nullptr, nullptr, &settingsValues);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->GetHierarchyLevel(info.first).IsValid());
    
    // verify what valid datasource is returned with same user settings
    m_userSettings.GetSettings("ruleset_id").SetSettingIntValue("setting_id", 10);

    EXPECT_TRUE(m_cache->GetHierarchyLevel(info.first).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetHierarchyLevel_ClearsChildDatasourceCacheIfRelatedSettingsValuesChanged)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    auto rootNodes = FillWithNodes(rootInfo, 1);

    // cache child data source
    m_userSettings.GetSettings("ruleset_id").SetSettingIntValue("setting_id", 1);
    bvector<UserSettingEntry> settingsValues = {UserSettingEntry("setting_id", Json::Value(1))};
    auto childInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodes[0]->GetNodeId());
    m_cache->Update(childInfo.second, nullptr, nullptr, &settingsValues);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->GetHierarchyLevel(childInfo.first).IsValid());

    // verify what valid datasource is not returned with different user settings
    m_userSettings.GetSettings("ruleset_id").SetSettingIntValue("setting_id", 10);
    
    EXPECT_TRUE(m_cache->GetHierarchyLevel(childInfo.first).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetDataSource_ClearsRootDatasourceCacheIfRelatedSettingsValuesChanged)
    {
    m_userSettings.GetSettings("ruleset_id").SetSettingValue("setting_id", "value0");
    bvector<UserSettingEntry> settingsValues = {UserSettingEntry("setting_id", Json::Value("value0"))};
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    m_cache->Update(info.second, nullptr, nullptr, &settingsValues);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->GetDataSource(info.second).IsValid());
    
    // verify that valid datasource is not returned with different user settings
    m_userSettings.GetSettings("ruleset_id").SetSettingValue("setting_id", "value1");

    EXPECT_FALSE(m_cache->GetDataSource(info.second).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetDataSource_DoesntClearRootDatasourceCacheIfRelatedSettingsValuesDidntChange)
    {
    m_userSettings.GetSettings("ruleset_id").SetSettingIntValue("setting_id", 10);
    bvector<UserSettingEntry> settingsValues = {UserSettingEntry("setting_id", Json::Value(10))};
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    m_cache->Update(info.second, nullptr, nullptr, &settingsValues);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->GetDataSource(info.second).IsValid());
    
    // verify what valid datasource is returned with same user settings
    m_userSettings.GetSettings("ruleset_id").SetSettingIntValue("setting_id", 10);

    EXPECT_TRUE(m_cache->GetDataSource(info.second).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetDataSource_ClearsChildDatasourceCacheIfRelatedSettingsValuesChanged)
    {
    // cache root data source
    auto rootInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    auto rootNodes = FillWithNodes(rootInfo, 1);

    // cache child data source
    m_userSettings.GetSettings("ruleset_id").SetSettingIntValue("setting_id", 1);
    bvector<UserSettingEntry> settingsValues = {UserSettingEntry("setting_id", Json::Value(1))};
    auto childInfo = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", rootNodes[0]->GetNodeId());
    m_cache->Update(childInfo.second, nullptr, nullptr, &settingsValues);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->GetDataSource(childInfo.second).IsValid());

    // verify what valid datasource is not returned with different user settings
    m_userSettings.GetSettings("ruleset_id").SetSettingIntValue("setting_id", 10);
    
    EXPECT_TRUE(m_cache->GetDataSource(childInfo.second).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Savepoint_DiscardsChangesWhenCanceled)
    {
    auto savepoint = m_cache->CreateSavepoint();
    
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    auto nodes = FillWithNodes(info, 1);
    
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(info.first).IsValid());
    EXPECT_TRUE(m_cache->GetNode(nodes[0]->GetNodeId()).IsValid());

    savepoint->Cancel();

    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(info.first).IsValid());
    EXPECT_FALSE(m_cache->GetNode(nodes[0]->GetNodeId()).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Savepoint_DoesntDiscardChangesWhenNotCanceled)
    {
    auto savepoint = m_cache->CreateSavepoint();
    
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    auto nodes = FillWithNodes(info, 1);
    
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(info.first).IsValid());
    EXPECT_TRUE(m_cache->GetNode(nodes[0]->GetNodeId()).IsValid());

    savepoint = nullptr;
    
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(info.first).IsValid());
    EXPECT_TRUE(m_cache->GetNode(nodes[0]->GetNodeId()).IsValid());
    }
    
/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                09/2017
+===============+===============+===============+===============+===============+======*/
struct DiskNodesCacheTests : NodesCacheTests
    {
    void TearDown() override
        {
        BeFileName cacheDb(m_cache->GetDb().GetDbFileName());
        NodesCacheTests::TearDown();

        cacheDb.BeDeleteFile();
        }

    virtual NodesCache* _CreateNodesCache(BeFileName tempDir) override
        {
        return new NodesCache(tempDir, m_nodesFactory, m_nodesProviderContextFactory, m_connections, m_userSettings, m_ecsqlStatements, NodesCacheType::Disk);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, CreatesNewDbFileIfCacheIsAlreadyInUse)
    {
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    NodesCache secondCache(tempDir, m_nodesFactory, m_nodesProviderContextFactory, m_connections, m_userSettings, m_ecsqlStatements, NodesCacheType::Disk);

    Utf8CP firstCacheName = m_cache->GetDb().GetDbFileName();
    Utf8CP secondCacheName = secondCache.GetDb().GetDbFileName();

    EXPECT_STRNE(firstCacheName, secondCacheName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
#if !defined (BENTLEYCONFIG_OS_LINUX) // informed Kyle/Caleb/Gintaras 9/28/18
TEST_F(DiskNodesCacheTests, ShareCachedHierarchiesBetweenSessions)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    auto nodes = FillWithNodes(info, 2, true);

    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(info.first).IsValid());
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodes[1]->GetNodeId()));

    // close cache
    m_connections.NotifyConnectionClosed(*m_connection);
    DELETE_AND_CLEAR(s_project);
    DELETE_AND_CLEAR(m_cache);

    // open cache
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    m_cache = new NodesCache(tempDir, m_nodesFactory, m_nodesProviderContextFactory, m_connections, m_userSettings, m_ecsqlStatements, NodesCacheType::Disk);
    
    // mock connection re-opening
    ReCreateProject();

    // cache should not be cleaned
    CombinedHierarchyLevelInfo updatedInfo(m_connection->GetId(), info.first.GetRulesetId(),
        info.first.GetLocale(), 0);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(updatedInfo).IsValid());
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodes[1]->GetNodeId()));
    }
#endif
    
/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, ClearSharedCacheIfHierarchyWasModified)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);

    // cache some nodes
    auto nodes = FillWithNodes(info, 2, true);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(info.first).IsValid());
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodes[1]->GetNodeId()));

    // close cache
    BeFileName projectPath(s_project->GetECDbPath());
    m_connections.NotifyConnectionClosed(*m_connection);
    DELETE_AND_CLEAR(s_project);
    DELETE_AND_CLEAR(m_cache);

    // change connection file last modification time
    time_t modTime;
    projectPath.GetFileTime(nullptr, nullptr, &modTime);
    modTime -= 100;
    projectPath.SetFileTime(nullptr, &modTime);

    // open cache
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    m_cache = new NodesCache(tempDir, m_nodesFactory, m_nodesProviderContextFactory, m_connections, m_userSettings, m_ecsqlStatements, NodesCacheType::Disk);

    // mock connection re-opening
    ReCreateProject();

    // cache should be empty for this connection
    CombinedHierarchyLevelInfo updatedInfo(m_connection->GetId(), info.first.GetRulesetId(),
        info.first.GetLocale(), 0);
    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(updatedInfo).IsValid());
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(nodes[0]->GetNodeId()));
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(nodes[1]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* Test situation when app is closed, ruleset is modified and app is turned on.
* In this case OnRulesetDisposed isn't called and only OnRulesetCreated is called.
* @bsitest                                      Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, ClearSharedCacheIfRulesetWasModified)
    {
    // create ruleset
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("TestRuleset", 1, 0, false, "", "", "", false);
    ruleset->AddPresentationRule(*new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false));
    m_cache->OnRulesetCreated(*ruleset);

    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), ruleset->GetRuleSetId(), "locale", 0);

    // cache some nodes
    auto nodes = FillWithNodes(info, 2, true);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(info.first).IsValid());
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodes[1]->GetNodeId()));

    // mock new session: app is turned on with modified ruleset and OnRulesetCreated is called
    ruleset->AddPresentationRule(*new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree));
    m_cache->OnRulesetCreated(*ruleset);

    // verify cache is cleared
    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(info.first).IsValid());
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(nodes[0]->GetNodeId()));
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(nodes[1]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, ClearsCacheIfCacheFileSizeExceedsLimit)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);

    // cache some nodes
    auto nodes = FillWithNodes(info, 2, true);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(info.first).IsValid());
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->IsHierarchyLevelCached(nodes[1]->GetNodeId()));

    // set cache limit
    m_cache->SetCacheFileSizeLimit(1);

    // close cache
    DELETE_AND_CLEAR(m_cache);

    // open cache
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    m_cache = new NodesCache(tempDir, m_nodesFactory, m_nodesProviderContextFactory, m_connections, m_userSettings, m_ecsqlStatements, NodesCacheType::Disk);

    // verify cache is cleared
    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(info.first).IsValid());
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(nodes[0]->GetNodeId()));
    EXPECT_FALSE(m_cache->IsHierarchyLevelCached(nodes[1]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, ClearsOldestConnectionCacheWhenCacheFileSizeExceedsLimit)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    FillWithNodes(info, 30, true);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(info.first).IsValid());
    m_connections.NotifyConnectionClosed(*m_connection);

    // open second connection and cache root data source
    ECDbTestProject project;
    project.Create("ClearsOldestConnectionCacheWhenCacheFileSizeExceedsLimit");
    IConnectionPtr connection2 = m_connections.NotifyConnectionOpened(project.GetECDb());

    auto info2 = CacheDataSource(connection2->GetId(), "ruleset_id", "locale", 0);
    FillWithNodes(info2, 40, true);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(info2.first).IsValid());
    m_connections.NotifyConnectionClosed(*connection2);

    // set cache file size limit
    const_cast<Db&>(m_cache->GetDb()).SaveChanges();
    BeFileName cacheFile(m_cache->GetDb().GetDbFileName());
    uint64_t fileSize = 0;
    cacheFile.GetFileSize(fileSize);
    m_cache->SetCacheFileSizeLimit(fileSize - 1);

    // close cache
    DELETE_AND_CLEAR(m_cache);

    // open cache
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    m_cache = new NodesCache(tempDir, m_nodesFactory, m_nodesProviderContextFactory, m_connections, m_userSettings, m_ecsqlStatements, NodesCacheType::Disk);

    // verify first connection cache was deleted
    m_connection = m_connections.NotifyConnectionOpened(GetDb());
    EXPECT_FALSE(m_cache->GetCombinedHierarchyLevel(info.first).IsValid());

    // verify second connection cache wasn't deleted
    connection2 = m_connections.NotifyConnectionOpened(project.GetECDb());
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(info2.first).IsValid());

    m_connections.NotifyConnectionClosed(*connection2);
    project.GetECDb().CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, DoesntClearCacheWhenCacheFileSizeAndLimitAreEqual)
    {
    // cache root data source
    auto info = CacheDataSource(m_connection->GetId(), "ruleset_id", "locale", 0);
    FillWithNodes(info, 30, true);
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(info.first).IsValid());
    m_connections.NotifyConnectionClosed(*m_connection);

    // set cache file size limit
    const_cast<Db&>(m_cache->GetDb()).SaveChanges();
    BeFileName cacheFile(m_cache->GetDb().GetDbFileName());
    uint64_t fileSize = 0;
    cacheFile.GetFileSize(fileSize);
    m_cache->SetCacheFileSizeLimit(fileSize);

    // close cache
    DELETE_AND_CLEAR(m_cache);

    // open cache
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    m_cache = new NodesCache(tempDir, m_nodesFactory, m_nodesProviderContextFactory, m_connections, m_userSettings, m_ecsqlStatements, NodesCacheType::Disk);

    // verify the cache is still valid
    m_connection = m_connections.NotifyConnectionOpened(GetDb());
    EXPECT_TRUE(m_cache->GetCombinedHierarchyLevel(info.first).IsValid());
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                12/2018
+===============+===============+===============+===============+===============+======*/
struct DiskNodesCacheLocationTests : ECPresentationTest
    {    
    TestConnectionManager m_connections;
    TestUserSettingsManager m_userSettings;
    JsonNavNodesFactory m_nodesFactory;
    TestNodesProviderContextFactory m_nodesProviderContextFactory;
    TestECSqlStatementsCacheProvider m_ecsqlStatements;
    BeFileName m_directory;
    DiskNodesCacheLocationTests() : m_nodesProviderContextFactory(m_connections) {}
    void SetUp() override
        {
        BeTest::GetHost().GetOutputRoot(m_directory);
        m_directory.AppendToPath(L"DiskNodesCacheLocationTests");
        if (!m_directory.DoesPathExist())
            BeFileName::CreateNewDirectory(m_directory.c_str());
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheLocationTests, CreatesCacheInSpecifiedDirectory)
    {
    BeFileName expectedPath = m_directory;
    expectedPath.AppendToPath(L"HierarchyCache.db");

    NodesCache cache(m_directory, m_nodesFactory, m_nodesProviderContextFactory, m_connections, m_userSettings, m_ecsqlStatements, NodesCacheType::Disk);
    EXPECT_STREQ(expectedPath.GetNameUtf8().c_str(), cache.GetDb().GetDbFileName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheLocationTests, ReusesExistingCache)
    {
    BeFileName expectedPath = m_directory;
    expectedPath.AppendToPath(L"HierarchyCache.db");

    {
    NodesCache cache(m_directory, m_nodesFactory, m_nodesProviderContextFactory, m_connections, m_userSettings, m_ecsqlStatements, NodesCacheType::Disk);
    }
    EXPECT_TRUE(expectedPath.DoesPathExist());

    NodesCache cache(m_directory, m_nodesFactory, m_nodesProviderContextFactory, m_connections, m_userSettings, m_ecsqlStatements, NodesCacheType::Disk);
    EXPECT_STREQ(expectedPath.GetNameUtf8().c_str(), cache.GetDb().GetDbFileName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheLocationTests, CreatesSeparateCacheWhenExistingIsLocked)
    {
    BeFileName expectedPath1 = m_directory;
    expectedPath1.AppendToPath(L"HierarchyCache.db");
    NodesCache cache1(m_directory, m_nodesFactory, m_nodesProviderContextFactory, m_connections, m_userSettings, m_ecsqlStatements, NodesCacheType::Disk);
    EXPECT_STREQ(expectedPath1.GetNameUtf8().c_str(), cache1.GetDb().GetDbFileName());

    BeFileName cache2Path;
    {
    NodesCache cache2(m_directory, m_nodesFactory, m_nodesProviderContextFactory, m_connections, m_userSettings, m_ecsqlStatements, NodesCacheType::Disk);
    cache2Path = BeFileName(cache2.GetDb().GetDbFileName());
    BeFileName expectedDirectory2 = BeFileName(m_directory).AppendSeparator();
    EXPECT_STREQ(expectedDirectory2.c_str(), cache2Path.GetDirectoryName().c_str());

    bvector<WString> nameParts;
    BeStringUtilities::Split(cache2Path.GetFileNameAndExtension().c_str(), L".", nullptr, nameParts);
    ASSERT_EQ(3, nameParts.size());
    EXPECT_STREQ(L"HierarchyCache", nameParts[0].c_str());
    EXPECT_EQ(SUCCESS, BeGuid().FromString(Utf8String(nameParts[1].c_str()).c_str()));
    EXPECT_STREQ(L"db", nameParts[2].c_str());
    }
    EXPECT_FALSE(cache2Path.DoesPathExist());
    }