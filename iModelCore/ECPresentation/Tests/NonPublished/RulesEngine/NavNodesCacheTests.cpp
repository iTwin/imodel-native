/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/NavNodesCacheTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

    bvector<JsonNavNodeCPtr> FillWithNodes(DataSourceInfo const&, size_t count, bool createChildDataSources = false);
    };
ECDbTestProject* NodesCacheTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<JsonNavNodeCPtr> NodesCacheTests::FillWithNodes(DataSourceInfo const& info, size_t count, bool createChildDataSources)
    {
    bvector<JsonNavNodeCPtr> nodes;
    for (size_t i = 0; i < count; ++i)
        {
        TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
        if (nullptr != info.GetPhysicalParentNodeId())
            node->SetParentNodeId(*info.GetPhysicalParentNodeId());
        NavNodeExtendedData extendedData(*node);
        extendedData.SetConnectionId(info.GetConnectionId());
        extendedData.SetRulesetId(info.GetRulesetId().c_str());
        if (nullptr != info.GetVirtualParentNodeId())
            extendedData.SetVirtualParentId(*info.GetVirtualParentNodeId());
        m_cache->Cache(*node, false);
        nodes.push_back(node);

        if (createChildDataSources)
            {
            uint64_t nodeId = node->GetNodeId();
            DataSourceInfo childInfo(info.GetConnectionId(), info.GetRulesetId(), info.GetLocale(), &nodeId, &nodeId);
            m_cache->Cache(childInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
            }
        }
    return nodes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Clear_Full)
    {
    // cache root data source
    DataSourceInfo rootInfo(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache root node
    TestNavNodePtr rootNode = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData rootExtendedData(*rootNode);
    rootExtendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    m_cache->Cache(*rootNode, false);
    uint64_t rootNodeId = rootNode->GetNodeId();
    
    // cache child data source
    DataSourceInfo childrenInfo(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), &rootNodeId, &rootNodeId);
    m_cache->Cache(childrenInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache child node
    TestNavNodePtr childNode = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData childExtendedData(*childNode);
    childExtendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    childExtendedData.SetVirtualParentId(rootNodeId);
    m_cache->Cache(*childNode, false);
    uint64_t childNodeId = childNode->GetNodeId();
    
    // cache root data source for a different connection
    ECDbTestProject project2;
    project2.Create("test2");
    IConnectionPtr connection2 = m_connections.NotifyConnectionOpened(project2.GetECDb());
    DataSourceInfo rootInfo2(connection2->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(rootInfo2, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    
    // cache root data source for a different ruleset
    m_cache->OnRulesetCreated(*PresentationRuleSet::CreateInstance("ruleset_id2", 1, 0, false, "", "", "", false));
    DataSourceInfo rootInfo3(m_connection->GetId(), "ruleset_id2", "locale", nullptr, nullptr);
    m_cache->Cache(rootInfo3, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // make sure everything's cached
    EXPECT_TRUE(m_cache->IsDataSourceCached(connection2->GetId(), "ruleset_id", "locale"));
    EXPECT_TRUE(m_cache->IsDataSourceCached(m_connection->GetId(), "ruleset_id2", "locale"));
    EXPECT_TRUE(m_cache->IsDataSourceCached(m_connection->GetId(), "ruleset_id", "locale"));
    EXPECT_TRUE(m_cache->IsNodeCached(rootNodeId));
    EXPECT_TRUE(m_cache->IsDataSourceCached(rootNodeId));
    EXPECT_TRUE(m_cache->IsNodeCached(childNodeId));

    // fully clear the cache
    m_cache->Clear();

    // verify everything's gone
    EXPECT_FALSE(m_cache->IsDataSourceCached(connection2->GetId(), "ruleset_id", "locale"));
    EXPECT_FALSE(m_cache->IsDataSourceCached(m_connection->GetId(), "ruleset_id2", "locale"));
    EXPECT_FALSE(m_cache->IsDataSourceCached(m_connection->GetId(), "ruleset_id", "locale"));
    EXPECT_FALSE(m_cache->IsNodeCached(rootNodeId));
    EXPECT_FALSE(m_cache->IsDataSourceCached(rootNodeId));
    EXPECT_FALSE(m_cache->IsNodeCached(childNodeId));

    m_connections.NotifyConnectionClosed(*connection2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Clear_ByConnection)
    {
    // cache root data source
    DataSourceInfo rootInfo(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache root node
    TestNavNodePtr rootNode = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData rootExtendedData(*rootNode);
    rootExtendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    m_cache->Cache(*rootNode, false);
    uint64_t rootNodeId = rootNode->GetNodeId();
    
    // cache child data source
    DataSourceInfo childrenInfo(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), &rootNodeId, &rootNodeId);
    m_cache->Cache(childrenInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache child node
    TestNavNodePtr childNode = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData childExtendedData(*childNode);
    childExtendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    childExtendedData.SetVirtualParentId(rootNodeId);
    m_cache->Cache(*childNode, false);
    uint64_t childNodeId = childNode->GetNodeId();
    
    // cache root data source for a different connection
    ECDbTestProject project2;
    project2.Create("test2");
    IConnectionPtr connection2 = m_connections.NotifyConnectionOpened(project2.GetECDb());
    DataSourceInfo rootInfo2(connection2->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(rootInfo2, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // make sure everything's cached
    EXPECT_TRUE(m_cache->IsDataSourceCached(connection2->GetId(), "ruleset_id", "locale"));
    EXPECT_TRUE(m_cache->IsDataSourceCached(m_connection->GetId(), "ruleset_id", "locale"));
    EXPECT_TRUE(m_cache->IsNodeCached(rootNodeId));
    EXPECT_TRUE(m_cache->IsDataSourceCached(rootNodeId));
    EXPECT_TRUE(m_cache->IsNodeCached(childNodeId));

    // fully clear the cache
    m_cache->Clear(m_connection.get());

    // verify everything related to primary connection is now gone
    EXPECT_TRUE(m_cache->IsDataSourceCached(connection2->GetId(), "ruleset_id", "locale"));
    EXPECT_FALSE(m_cache->IsDataSourceCached(m_connection->GetId(), "ruleset_id", "locale"));
    EXPECT_FALSE(m_cache->IsNodeCached(rootNodeId));
    EXPECT_FALSE(m_cache->IsDataSourceCached(rootNodeId));
    EXPECT_FALSE(m_cache->IsNodeCached(childNodeId));

    m_connections.NotifyConnectionClosed(*connection2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Clear_ByRulesetId)
    {
    // cache root data source
    DataSourceInfo rootInfo(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache root node
    TestNavNodePtr rootNode = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData rootExtendedData(*rootNode);
    rootExtendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    m_cache->Cache(*rootNode, false);
    uint64_t rootNodeId = rootNode->GetNodeId();
    
    // cache child data source
    DataSourceInfo childrenInfo(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), &rootNodeId, &rootNodeId);
    m_cache->Cache(childrenInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache child node
    TestNavNodePtr childNode = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData childExtendedData(*childNode);
    childExtendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    childExtendedData.SetVirtualParentId(rootNodeId);
    m_cache->Cache(*childNode, false);
    uint64_t childNodeId = childNode->GetNodeId();
        
    // cache root data source for a different ruleset
    m_cache->OnRulesetCreated(*PresentationRuleSet::CreateInstance("ruleset_id2", 1, 0, false, "", "", "", false));
    DataSourceInfo rootInfo3(m_connection->GetId(), "ruleset_id2", "locale", nullptr, nullptr);
    m_cache->Cache(rootInfo3, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // make sure everything's cached
    EXPECT_TRUE(m_cache->IsDataSourceCached(m_connection->GetId(), "ruleset_id2", "locale"));
    EXPECT_TRUE(m_cache->IsDataSourceCached(m_connection->GetId(), "ruleset_id", "locale"));
    EXPECT_TRUE(m_cache->IsNodeCached(rootNodeId));
    EXPECT_TRUE(m_cache->IsDataSourceCached(rootNodeId));
    EXPECT_TRUE(m_cache->IsNodeCached(childNodeId));

    // fully clear the cache
    m_cache->Clear(nullptr, "ruleset_id");

    // verify everything related to "ruleset_id" is now gone
    EXPECT_TRUE(m_cache->IsDataSourceCached(m_connection->GetId(), "ruleset_id2", "locale"));
    EXPECT_FALSE(m_cache->IsDataSourceCached(m_connection->GetId(), "ruleset_id", "locale"));
    EXPECT_FALSE(m_cache->IsNodeCached(rootNodeId));
    EXPECT_FALSE(m_cache->IsDataSourceCached(rootNodeId));
    EXPECT_FALSE(m_cache->IsNodeCached(childNodeId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemovesRootDataSource)
    {
    // cache data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    
    // verify it got cached
    EXPECT_TRUE(m_cache->IsDataSourceCached(m_connection->GetId(), info.GetRulesetId().c_str(), info.GetLocale().c_str()));

    // attempt to remove with invalid connection id and verify it didn't get removed
    BeGuid removalId = m_cache->CreateRemovalId(HierarchyLevelInfo("invalid", info.GetRulesetId(), info.GetLocale(), info.GetPhysicalParentNodeId()));
    m_cache->RemoveDataSource(removalId);
    EXPECT_TRUE(m_cache->IsDataSourceCached(m_connection->GetId(), info.GetRulesetId().c_str(), info.GetLocale().c_str()));
    
    // attempt to remove with invalid ruleset id and verify it didn't get removed
    removalId = m_cache->CreateRemovalId(HierarchyLevelInfo(m_connection->GetId(), "invalid", info.GetLocale(), info.GetPhysicalParentNodeId()));
    m_cache->RemoveDataSource(removalId);
    EXPECT_TRUE(m_cache->IsDataSourceCached(m_connection->GetId(), info.GetRulesetId().c_str(), info.GetLocale().c_str()));
    
    // attempt to remove with invalid locale and verify it didn't get removed
    removalId = m_cache->CreateRemovalId(HierarchyLevelInfo(m_connection->GetId(), info.GetRulesetId(), "invalid", info.GetPhysicalParentNodeId()));
    m_cache->RemoveDataSource(removalId);
    EXPECT_TRUE(m_cache->IsDataSourceCached(m_connection->GetId(), info.GetRulesetId().c_str(), info.GetLocale().c_str()));

    // attempt to remove with valid parameters and verify it did get removed
    removalId = m_cache->CreateRemovalId(HierarchyLevelInfo(info.GetConnectionId(), info.GetRulesetId(), info.GetLocale(), info.GetPhysicalParentNodeId()));
    m_cache->RemoveDataSource(removalId);
    EXPECT_FALSE(m_cache->IsDataSourceCached(m_connection->GetId(), info.GetRulesetId().c_str(), info.GetLocale().c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemovesChildDataSource)
    {
    // cache root data source
    DataSourceInfo rootInfo(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache root node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    m_cache->Cache(*node, false);
    uint64_t nodeId = node->GetNodeId();

    // cache child data source
    DataSourceInfo childrenInfo(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), &nodeId, &nodeId);
    m_cache->Cache(childrenInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // verify child data source is cached
    EXPECT_TRUE(m_cache->IsDataSourceCached(nodeId));

    // remove child data source
    BeGuid removalId = m_cache->CreateRemovalId(HierarchyLevelInfo(childrenInfo.GetConnectionId(), childrenInfo.GetRulesetId(), childrenInfo.GetLocale(), childrenInfo.GetPhysicalParentNodeId()));
    m_cache->RemoveDataSource(removalId);

    // verify child data source also got removed
    EXPECT_FALSE(m_cache->IsDataSourceCached(nodeId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemovesChildNodesWhenParentDataSourceIsRemoved)
    {
    // cache data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    
    // cache node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId(info.GetRulesetId().c_str());
    m_cache->Cache(*node, false);

    // verify node is cached
    EXPECT_TRUE(m_cache->IsNodeCached(node->GetNodeId()));

    // remove
    BeGuid removalId = m_cache->CreateRemovalId(HierarchyLevelInfo(info.GetConnectionId(), info.GetRulesetId(), info.GetLocale(), info.GetPhysicalParentNodeId()));
    m_cache->RemoveDataSource(removalId);

    // verify it got removed
    EXPECT_FALSE(m_cache->IsNodeCached(node->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemovesChildDataSourcesWhenParentDataSourceIsRemoved)
    {
    // cache data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    
    // cache node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId(info.GetRulesetId().c_str());
    m_cache->Cache(*node, false);
    uint64_t parentNodeId = node->GetNodeId();
    
    // cache child data source
    DataSourceInfo childrenInfo(m_connection->GetId(), info.GetRulesetId(), info.GetLocale(), &parentNodeId, &parentNodeId);
    m_cache->Cache(childrenInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // verify child data source is cached
    EXPECT_TRUE(m_cache->IsDataSourceCached(parentNodeId));

    // remove parent data source
    BeGuid removalId = m_cache->CreateRemovalId(HierarchyLevelInfo(info.GetConnectionId(), info.GetRulesetId(), info.GetLocale(), info.GetPhysicalParentNodeId()));
    m_cache->RemoveDataSource(removalId);

    // verify child data source also got removed
    EXPECT_FALSE(m_cache->IsDataSourceCached(parentNodeId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, MakePhysical)
    {
    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache root node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId(info.GetRulesetId().c_str());
    m_cache->Cache(*node, true);

    // verify node is virtual
    EXPECT_FALSE(m_cache->GetNode(node->GetNodeId(), NodeVisibility::Physical).IsValid());
    EXPECT_TRUE(m_cache->GetNode(node->GetNodeId(), NodeVisibility::Virtual).IsValid());

    // make physical
    m_cache->MakePhysical(*node);
    
    // verify node is physical
    EXPECT_TRUE(m_cache->GetNode(node->GetNodeId(), NodeVisibility::Physical).IsValid());
    EXPECT_FALSE(m_cache->GetNode(node->GetNodeId(), NodeVisibility::Virtual).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, MakeVirtual)
    {
    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache root node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId(info.GetRulesetId().c_str());
    m_cache->Cache(*node, false);
    
    // verify node is physical
    EXPECT_TRUE(m_cache->GetNode(node->GetNodeId(), NodeVisibility::Physical).IsValid());
    EXPECT_FALSE(m_cache->GetNode(node->GetNodeId(), NodeVisibility::Virtual).IsValid());

    // make physical
    m_cache->MakeVirtual(*node);
    
   // verify node is virtual
    EXPECT_FALSE(m_cache->GetNode(node->GetNodeId(), NodeVisibility::Physical).IsValid());
    EXPECT_TRUE(m_cache->GetNode(node->GetNodeId(), NodeVisibility::Virtual).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, UpdateNode)
    {
    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache root node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "A", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId(info.GetRulesetId().c_str());
    m_cache->Cache(*node, false);
    
    // verify node's label is valid
    EXPECT_STREQ("A", m_cache->GetNode(node->GetNodeId())->GetLabel().c_str());

    // update
    node->SetLabel("B");
    m_cache->Update(node->GetNodeId(), *node);
    
    // verify node's label has changed
    EXPECT_STREQ("B", m_cache->GetNode(node->GetNodeId())->GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* note: the test uses GetRelatedHierarchyLevels function to test the Update
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, UpdateDataSource_UpdatesFilter)
    {
    ECClassCP widgetHasGadgetRelationship = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    // cache the data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[ECClassId((uint64_t)1)] = false;
    m_cache->Cache(info, DataSourceFilter(), usedClassIds, bvector<UserSettingEntry>());

    // verify the filter is not applied and we find related hierarchy level
    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.GetConnectionId(), keys);
    EXPECT_EQ(1, related.size());

    // update the datasource with new filter
    bset<ECClassId> relationshipClassIds;
    relationshipClassIds.insert(widgetHasGadgetRelationship->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipClassIds, RequiredRelationDirection_Both, ECInstanceId((uint64_t)123)), nullptr);
    m_cache->Update(info, &filter, nullptr, nullptr);
    
    // verify the filter is applied and we dont find related hierarchy level anymore
    related = m_cache->GetRelatedHierarchyLevels(info.GetConnectionId(), keys);
    EXPECT_EQ(0, related.size());
    }

/*---------------------------------------------------------------------------------**//**
* note: the test uses GetRelatedHierarchyLevels function to test the Update
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, UpdateDataSource_UpdatesRelatedClassIds)
    {
    // cache the data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
        
    // verify we don't find related hierarchy level as the key is not related to the data source
    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.GetConnectionId(), keys);
    EXPECT_EQ(0, related.size());

    // update the datasource related class ids
    bmap<ECN::ECClassId, bool> relatedClassIds;
    relatedClassIds[ECN::ECClassId((uint64_t)1)] = false;
    m_cache->Update(info, nullptr, &relatedClassIds, nullptr);
    
    // verify we do find related hierarchy level this time as the data source is now related to the lookup keys
    related = m_cache->GetRelatedHierarchyLevels(info.GetConnectionId(), keys);
    EXPECT_EQ(1, related.size());
    }

/*---------------------------------------------------------------------------------**//**
* note: the test uses GetRelatedHierarchyLevels function to test the Update
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, UpdateDataSource_UpdatesRelatedSettings)
    {
    // unused - ECClassCP widgetHasGadgetRelationship = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    // cache the data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
        
    // verify we don't find related hierarchy level as data source is not related to any setting ids
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.GetRulesetId().c_str(), "setting_id");
    EXPECT_EQ(0, related.size());

    // update the datasource related settings
    bvector<UserSettingEntry> settings = {UserSettingEntry("setting_id", Json::Value("value0"))};
    m_cache->Update(info, nullptr, nullptr, &settings);
    
    // verify we do find related hierarchy level this time as the data source is now related to the setting id
    related = m_cache->GetRelatedHierarchyLevels(info.GetRulesetId().c_str(), "setting_id");
    EXPECT_EQ(1, related.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemapNodeIds_RemapsDataSourcesWhenParentIsPhysical)
    {
    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache 2 root nodes
    bvector<JsonNavNodeCPtr> rootNodes = FillWithNodes(info, 2, false);

    // create a data source for root node 0
    uint64_t rootNodeId0 = rootNodes[0]->GetNodeId();
    DataSourceInfo childInfo(info.GetConnectionId(), info.GetRulesetId(), info.GetLocale(), &rootNodeId0, &rootNodeId0);
    m_cache->Cache(childInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // verify rootNodes[0] has a datasource and rootNodes[1] doesnt
    EXPECT_TRUE(m_cache->IsDataSourceCached(rootNodes[0]->GetNodeId()));
    EXPECT_FALSE(m_cache->IsDataSourceCached(rootNodes[1]->GetNodeId()));
    
    // remap
    bmap<uint64_t, uint64_t> remapInfo;
    remapInfo.Insert(rootNodes[0]->GetNodeId(), rootNodes[1]->GetNodeId());
    m_cache->RemapNodeIds(remapInfo);
    
    // verify rootNodes[1] has a datasource and rootNodes[0] doesnt
    EXPECT_FALSE(m_cache->IsDataSourceCached(rootNodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->IsDataSourceCached(rootNodes[1]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemapNodeIds_RemapsDataSourcesWhenParentIsVirtual)
    {
    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache 2 root nodes
    bvector<JsonNavNodeCPtr> rootNodes = FillWithNodes(info, 2, false);
    m_cache->MakeVirtual(*rootNodes[0]);

    // create a data source for root node 0
    uint64_t rootNodeId0 = rootNodes[0]->GetNodeId();
    DataSourceInfo childInfo(info.GetConnectionId(), info.GetRulesetId(), info.GetLocale(), nullptr, &rootNodeId0);
    m_cache->Cache(childInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // verify rootNodes[0] has a datasource and rootNodes[1] doesnt
    EXPECT_TRUE(m_cache->IsDataSourceCached(rootNodes[0]->GetNodeId()));
    EXPECT_FALSE(m_cache->IsDataSourceCached(rootNodes[1]->GetNodeId()));
    
    // remap
    bmap<uint64_t, uint64_t> remapInfo;
    remapInfo.Insert(rootNodes[0]->GetNodeId(), rootNodes[1]->GetNodeId());
    m_cache->RemapNodeIds(remapInfo);
    
    // verify rootNodes[1] has a datasource and rootNodes[0] doesnt
    EXPECT_FALSE(m_cache->IsDataSourceCached(rootNodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->IsDataSourceCached(rootNodes[1]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetFilteredNodes)
    {
    // create the root node
    TestNavNodePtr node1 = TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test_A_test", "test descr");
    NavNodeExtendedData(*node1).SetRulesetId("ruleset_id");

    TestNavNodePtr node2 = TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test_B_test", "test descr");
    NavNodeExtendedData(*node2).SetRulesetId("ruleset_id");

    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache nodes
    m_cache->Cache(*node1, false);
    m_cache->Cache(*node2, false);
    
    bvector<NavNodeCPtr> filteredNodes = m_cache->GetFilteredNodes(*m_connection, info.GetRulesetId().c_str(), "locale", "A");
    // verify filtered node
    ASSERT_EQ(1, filteredNodes.size());
    EXPECT_TRUE(node1->Equals(*filteredNodes[0]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetFilteredNodes_DoesntFindNodesFromDifferentLocales)
    {
    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache root node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test label", "test descr");
    NavNodeExtendedData(*node).SetRulesetId(info.GetRulesetId().c_str());
    m_cache->Cache(*node, false);
    
    // verify the node is not found
    bvector<NavNodeCPtr> filteredNodes = m_cache->GetFilteredNodes(*m_connection, info.GetRulesetId().c_str(), "invalid", "test");
    ASSERT_EQ(0, filteredNodes.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Updates_IsExpandedFlag)
    {
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache root node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId(info.GetRulesetId().c_str());
    node->SetIsExpanded(true);
    m_cache->Cache(*node, false);

    ASSERT_TRUE(m_cache->IsNodeCached(node->GetNodeId()));
    NavNodeCPtr cachedNode = m_cache->GetNode(node->GetNodeId());
    ASSERT_TRUE(cachedNode.IsValid());
    EXPECT_TRUE(cachedNode->IsExpanded());

    node->SetIsExpanded(false);
    m_cache->Update(node->GetNodeId(), *node);
    ASSERT_TRUE(m_cache->IsNodeCached(node->GetNodeId()));
    cachedNode = m_cache->GetNode(node->GetNodeId());
    ASSERT_TRUE(cachedNode.IsValid());
    EXPECT_FALSE(cachedNode->IsExpanded());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ResetIsExpandedFlag)
    {
    // create the root node
    TestNavNodePtr node1 = TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test label", "test descr");
    NavNodeExtendedData extendedData1(*node1);
    extendedData1.SetConnectionId(m_connection->GetId());
    extendedData1.SetRulesetId("ruleset_id");
    node1->SetIsExpanded(true);

    TestNavNodePtr node2 = TestNodesHelper::CreateCustomNode(*m_connection, "test type", "label", "test descr");
    NavNodeExtendedData extendedData2(*node2);
    extendedData2.SetConnectionId(m_connection->GetId());
    extendedData2.SetRulesetId("ruleset_id");
    node2->SetIsExpanded(true);

    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache nodes
    m_cache->Cache(*node1, false);
    m_cache->Cache(*node2, false);

    m_cache->ResetExpandedNodes(m_connection->GetId().c_str(), "ruleset_id");

    EXPECT_FALSE(m_cache->GetNode(node1->GetNodeId())->IsExpanded());
    EXPECT_FALSE(m_cache->GetNode(node2->GetNodeId())->IsExpanded());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetUndeterminedNodesProvider_ReturnsNodeThatIsNotYetKnownToHaveChildren)
    {
    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // create node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test label", "test descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(m_connection->GetId());
    extendedData.SetRulesetId("ruleset_id");
    EXPECT_FALSE(node->DeterminedChildren());

    // cache node
    m_cache->Cache(*node, true);

    // expect provider with nodes that are not parents for other nodes (yet)
    NavNodesProviderPtr provider = m_cache->GetUndeterminedNodesProvider(*m_connection, info.GetRulesetId().c_str(), info.GetLocale().c_str(), false);
    ASSERT_TRUE(provider.IsValid());
    EXPECT_EQ(1, provider->GetNodesCount());

    // expect node that is not parent for other nodes
    JsonNavNodePtr expectedNode;
    provider->GetNode(expectedNode, 0);
    EXPECT_EQ(expectedNode->GetNodeId(), node->GetNodeId());
    EXPECT_TRUE(expectedNode->DeterminedChildren());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetUndeterminedNodesProvider_DoesNotReturnNodeThatHasChildren)
    {
    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // create node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test label", "test descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(m_connection->GetId());
    extendedData.SetRulesetId("ruleset_id");
    EXPECT_FALSE(node->DeterminedChildren());

    // cache node
    m_cache->Cache(*node, true);

    // cache child data source
    uint64_t nodeId = node->GetNodeId();
    DataSourceInfo childrenInfo(m_connection->GetId(), info.GetRulesetId(), info.GetLocale(), &nodeId, &nodeId);
    m_cache->Cache(childrenInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // expect provider to be empty, because all nodes are data sources
    NavNodesProviderPtr provider = m_cache->GetUndeterminedNodesProvider(*m_connection, info.GetRulesetId().c_str(), info.GetLocale().c_str(), false);
    ASSERT_TRUE(provider.IsValid());
    EXPECT_EQ(0, provider->GetNodesCount());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsNullsWhenEmpty)
    {
    EXPECT_FALSE(m_cache->IsDataSourceCached(m_connection->GetId(), "test ruleset id", "test locale"));
    EXPECT_FALSE(m_cache->IsDataSourceCached(999));
    EXPECT_FALSE(m_cache->IsNodeCached(999));

    EXPECT_TRUE(m_cache->GetDataSource(HierarchyLevelInfo(m_connection->GetId(), "test ruleset id", "test locale", 999)).IsNull());
    EXPECT_TRUE(m_cache->GetDataSource(DataSourceInfo(m_connection->GetId(), "test ruleset id", "test locale", nullptr, nullptr)).IsNull());
    EXPECT_TRUE(m_cache->GetDataSource(999).IsNull());
    EXPECT_TRUE(m_cache->GetNode(999).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedRootDataSource)
    {
    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // verify the data source is cached
    EXPECT_TRUE(m_cache->IsDataSourceCached(m_connection->GetId(), info.GetRulesetId().c_str(), info.GetLocale().c_str()));
    EXPECT_TRUE(m_cache->GetDataSource(HierarchyLevelInfo(m_connection->GetId(), info.GetRulesetId(), info.GetLocale(), nullptr)).IsValid());
    EXPECT_TRUE(m_cache->GetDataSource(DataSourceInfo(m_connection->GetId(), info.GetRulesetId(), info.GetLocale(), nullptr, nullptr)).IsValid());
    
    // verify it's not found when looking with invalid parameters
    EXPECT_FALSE(m_cache->IsDataSourceCached("invalid", info.GetRulesetId().c_str(), info.GetLocale().c_str()));
    EXPECT_TRUE(m_cache->GetDataSource(HierarchyLevelInfo("invalid", info.GetRulesetId(), info.GetLocale(), nullptr)).IsNull());
    EXPECT_TRUE(m_cache->GetDataSource(DataSourceInfo("invalid", info.GetRulesetId(), info.GetLocale(), nullptr, nullptr)).IsNull());
    
    EXPECT_FALSE(m_cache->IsDataSourceCached(m_connection->GetId(), "invalid", info.GetLocale().c_str()));
    EXPECT_TRUE(m_cache->GetDataSource(HierarchyLevelInfo(m_connection->GetId(), "invalid", info.GetLocale(), nullptr)).IsNull());
    EXPECT_TRUE(m_cache->GetDataSource(DataSourceInfo(m_connection->GetId(), "invalid", info.GetLocale(), nullptr, nullptr)).IsNull());
    
    EXPECT_FALSE(m_cache->IsDataSourceCached(m_connection->GetId(), info.GetRulesetId().c_str(), "invalid"));
    EXPECT_TRUE(m_cache->GetDataSource(HierarchyLevelInfo(m_connection->GetId(), info.GetRulesetId(), "invalid", nullptr)).IsNull());
    EXPECT_TRUE(m_cache->GetDataSource(DataSourceInfo(m_connection->GetId(), info.GetRulesetId(), "invalid", nullptr, nullptr)).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedRootNode)
    {
    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // create the root node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId(info.GetRulesetId().c_str());
    
    // the data source should be empty
    NavNodesProviderPtr cachedProvider = m_cache->GetDataSource(HierarchyLevelInfo(m_connection->GetId(), info.GetRulesetId(), info.GetLocale(), nullptr));
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(0, cachedProvider->GetNodesCount());

    // cache the node
    m_cache->Cache(*node, false);

    // verify the node is cached
    cachedProvider = m_cache->GetDataSource(HierarchyLevelInfo(m_connection->GetId(), info.GetRulesetId(), info.GetLocale(), nullptr));
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(1, cachedProvider->GetNodesCount());

    EXPECT_TRUE(m_cache->IsNodeCached(node->GetNodeId()));
    NavNodeCPtr cachedNode = m_cache->GetNode(node->GetNodeId());
    EXPECT_TRUE(cachedNode.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedRootNodeDataSource)
    {
    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // create the root node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId(info.GetRulesetId().c_str());
    
    // the data source should not be found until its node is cached
    NavNodesProviderPtr cachedProvider = m_cache->GetDataSource(node->GetNodeId());
    EXPECT_FALSE(cachedProvider.IsValid());

    // cache the node
    m_cache->Cache(*node, false);

    // verify the provider is returned now
    cachedProvider = m_cache->GetDataSource(node->GetNodeId());
    EXPECT_TRUE(cachedProvider.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedChildDataSource)
    {
    // cache root data source
    DataSourceInfo rootInfo(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache root node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    m_cache->Cache(*node, false);
    uint64_t nodeId = node->GetNodeId();

    // no child data source should exist
    EXPECT_TRUE(m_cache->GetDataSource(HierarchyLevelInfo(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), nodeId)).IsNull());
    EXPECT_TRUE(m_cache->GetDataSource(DataSourceInfo(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), &nodeId, &nodeId)).IsNull());

    // cache child data source
    DataSourceInfo childrenInfo(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), &nodeId, &nodeId);
    m_cache->Cache(childrenInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // verify the data source exists now
    EXPECT_TRUE(m_cache->IsDataSourceCached(nodeId));
    EXPECT_TRUE(m_cache->GetDataSource(HierarchyLevelInfo(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), nodeId)).IsValid());
    EXPECT_TRUE(m_cache->GetDataSource(DataSourceInfo(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), &nodeId, &nodeId)).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedChildNode)
    {
    // cache root data source
    DataSourceInfo rootInfo(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache root node
    TestNavNodePtr rootNode = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData rootExtendedData(*rootNode);
    rootExtendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    m_cache->Cache(*rootNode, false);
    uint64_t nodeId = rootNode->GetNodeId();
    
    // cache child data source
    DataSourceInfo childrenInfo(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), &nodeId, &nodeId);
    m_cache->Cache(childrenInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // the data source should be empty
    NavNodesProviderPtr cachedProvider = m_cache->GetDataSource(HierarchyLevelInfo(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), nodeId));
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(0, cachedProvider->GetNodesCount());

    // cache child node
    TestNavNodePtr childNode = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData childExtendedData(*childNode);
    childExtendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    childExtendedData.SetVirtualParentId(nodeId);
    m_cache->Cache(*childNode, false);    
    
    // verify the child node is cached
    cachedProvider = m_cache->GetDataSource(HierarchyLevelInfo(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), nodeId));
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(1, cachedProvider->GetNodesCount());

    EXPECT_TRUE(m_cache->IsNodeCached(childNode->GetNodeId()));
    NavNodeCPtr cachedNode = m_cache->GetNode(childNode->GetNodeId());
    EXPECT_TRUE(cachedNode.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedChildNodeDataSource)
    {
    // cache root data source
    DataSourceInfo rootInfo(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache root node
    TestNavNodePtr rootNode = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData rootExtendedData(*rootNode);
    rootExtendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    m_cache->Cache(*rootNode, false);
    uint64_t rootNodeId = rootNode->GetNodeId();
    
    // cache child data source
    DataSourceInfo childrenInfo(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), &rootNodeId, &rootNodeId);
    m_cache->Cache(childrenInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // create child node
    TestNavNodePtr childNode = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData childExtendedData(*childNode);
    childExtendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    childExtendedData.SetVirtualParentId(rootNodeId);

    // the data source should not be found
    NavNodesProviderPtr cachedProvider = m_cache->GetDataSource(childNode->GetNodeId());
    EXPECT_FALSE(cachedProvider.IsValid());
    
    // cache the node
    m_cache->Cache(*childNode, false);    
    
    // verify the data source is now found
    cachedProvider = m_cache->GetDataSource(childNode->GetNodeId());
    EXPECT_TRUE(cachedProvider.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsRequestedNodeTypeWhenNodeIsPhysical)
    {
    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // create the node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId(info.GetRulesetId().c_str());
    
    // cache the node
    m_cache->Cache(*node, false);

    // verify results
    EXPECT_TRUE(m_cache->GetNode(node->GetNodeId(), NodeVisibility::Any).IsValid());
    EXPECT_TRUE(m_cache->GetNode(node->GetNodeId(), NodeVisibility::Physical).IsValid());
    EXPECT_FALSE(m_cache->GetNode(node->GetNodeId(), NodeVisibility::Virtual).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsRequestedNodeTypeWhenNodeIsVirtual)
    {
    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // create the node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId(info.GetRulesetId().c_str());
    
    // cache the node
    m_cache->Cache(*node, true);

    // verify results
    EXPECT_TRUE(m_cache->GetNode(node->GetNodeId(), NodeVisibility::Any).IsValid());
    EXPECT_FALSE(m_cache->GetNode(node->GetNodeId(), NodeVisibility::Physical).IsValid());
    EXPECT_TRUE(m_cache->GetNode(node->GetNodeId(), NodeVisibility::Virtual).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsPreviouslyCachedDataSourceWhenCachingForTheSameParentTwice)
    {
    // cache the data source
    DataSourceInfo info1(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info1, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    DataSourceInfo info2(info1.GetConnectionId(), info1.GetRulesetId(), info1.GetLocale(), info1.GetPhysicalParentNodeId(), info1.GetVirtualParentNodeId());
    m_cache->Cache(info2, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // verify the data source ids are valid and match
    EXPECT_NE(0, info1.GetDataSourceId());
    EXPECT_EQ(info1.GetDataSourceId(), info2.GetDataSourceId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsDataSourcesFromValidConnections)
    {
    // create a new connection
    ECDbTestProject project2;
    project2.Create("test2");
    IConnectionPtr connection2 = m_connections.NotifyConnectionOpened(project2.GetECDb());
    
    // cache root data source for the first connection
    DataSourceInfo rootInfo1(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(rootInfo1, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    
    // cache root data source for the second connection
    DataSourceInfo rootInfo2(connection2->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(rootInfo2, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // verify correct sources are returned
    NavNodesProviderPtr cached1 = m_cache->GetDataSource(rootInfo1);
    ASSERT_TRUE(cached1.IsValid());
    EXPECT_EQ(m_connection.get(), &cached1->GetContext().GetConnection());

    NavNodesProviderPtr cached2 = m_cache->GetDataSource(rootInfo2);
    ASSERT_TRUE(cached2.IsValid());
    EXPECT_EQ(connection2.get(), &cached2->GetContext().GetConnection());

    m_connections.NotifyConnectionClosed(*connection2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsDataSourcesWithValidRulesetIds)
    {    
    // cache root data source for the first ruleset
    m_cache->OnRulesetCreated(*PresentationRuleSet::CreateInstance("ruleset_id1", 1, 0, false, "", "", "", false));
    DataSourceInfo rootInfo1(m_connection->GetId(), "ruleset_id1", "locale", nullptr, nullptr);
    m_cache->Cache(rootInfo1, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    
    // cache root data source for the second ruleset
    m_cache->OnRulesetCreated(*PresentationRuleSet::CreateInstance("ruleset_id2", 1, 0, false, "", "", "", false));
    DataSourceInfo rootInfo2(m_connection->GetId(), "ruleset_id2", "locale", nullptr, nullptr);
    m_cache->Cache(rootInfo2, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // verify correct sources are returned
    NavNodesProviderPtr cached1 = m_cache->GetDataSource(rootInfo1);
    ASSERT_TRUE(cached1.IsValid());
    EXPECT_STREQ(rootInfo1.GetRulesetId().c_str(), cached1->GetContext().GetRuleset().GetRuleSetId().c_str());

    NavNodesProviderPtr cached2 = m_cache->GetDataSource(rootInfo2);
    ASSERT_TRUE(cached2.IsValid());
    EXPECT_STREQ(rootInfo2.GetRulesetId().c_str(), cached2->GetContext().GetRuleset().GetRuleSetId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsDataSourcesWithValidLocales)
    {    
    // cache root data source for the first locale
    DataSourceInfo rootInfo1(m_connection->GetId(), "ruleset_id", "locale1", nullptr, nullptr);
    m_cache->Cache(rootInfo1, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    
    // cache root data source for the second locale
    DataSourceInfo rootInfo2(m_connection->GetId(), "ruleset_id", "locale2", nullptr, nullptr);
    m_cache->Cache(rootInfo2, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // verify correct sources are returned
    NavNodesProviderPtr cached1 = m_cache->GetDataSource(rootInfo1);
    ASSERT_TRUE(cached1.IsValid());
    EXPECT_STREQ(rootInfo1.GetLocale().c_str(), cached1->GetContext().GetLocale().c_str());

    NavNodesProviderPtr cached2 = m_cache->GetDataSource(rootInfo2);
    ASSERT_TRUE(cached2.IsValid());
    EXPECT_STREQ(rootInfo2.GetLocale().c_str(), cached2->GetContext().GetLocale().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, HasParentNode_ReturnsFalseForEmptySet)
    {
    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache root node
    bvector<JsonNavNodeCPtr> nodes = FillWithNodes(info, 1);
    JsonNavNodeCPtr node = nodes[0];

    bset<uint64_t> parentNodeIds;
    EXPECT_FALSE(m_cache->HasParentNode(node->GetNodeId(), parentNodeIds));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, HasParentNode_ReturnsFalseForRootNode)
    {
    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache root node
    bvector<JsonNavNodeCPtr> nodes = FillWithNodes(info, 1);
    JsonNavNodeCPtr node = nodes[0];
    
    bset<uint64_t> parentNodeIds;
    parentNodeIds.insert(node->GetNodeId() + 1);
    EXPECT_FALSE(m_cache->HasParentNode(node->GetNodeId(), parentNodeIds));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, HasParentNode_ReturnsFalseIfListDoesntContainParentNodeId)
    {
    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache root nodes
    bvector<JsonNavNodeCPtr> rootNodes = FillWithNodes(info, 2, true);
    
    // cache child node for root node 0
    uint64_t rootNodeId0 = rootNodes[0]->GetNodeId();
    bvector<JsonNavNodeCPtr> childNodes = FillWithNodes(DataSourceInfo(info.GetConnectionId(), info.GetRulesetId(), info.GetLocale(), &rootNodeId0, &rootNodeId0), 1);
    
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
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache root nodes
    bvector<JsonNavNodeCPtr> rootNodes = FillWithNodes(info, 2, true);
    
    // cache child node for root node 0
    uint64_t rootNodeId0 = rootNodes[0]->GetNodeId();
    bvector<JsonNavNodeCPtr> childNodes = FillWithNodes(DataSourceInfo(info.GetConnectionId(), info.GetRulesetId(), info.GetLocale(), &rootNodeId0, &rootNodeId0), 1);
    
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
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache root nodes
    bvector<JsonNavNodeCPtr> rootNodes = FillWithNodes(info, 1, true);
    
    // cache child node for root node
    uint64_t rootNodeId = rootNodes[0]->GetNodeId();
    bvector<JsonNavNodeCPtr> childNodes = FillWithNodes(DataSourceInfo(info.GetConnectionId(), info.GetRulesetId(), info.GetLocale(), &rootNodeId, &rootNodeId), 1, true);

    // cache grandchild node for child node
    uint64_t childNodeId = childNodes[0]->GetNodeId();
    bvector<JsonNavNodeCPtr> grandchildNodes = FillWithNodes(DataSourceInfo(info.GetConnectionId(), info.GetRulesetId(), info.GetLocale(), &childNodeId, &childNodeId), 1);
    
    bset<uint64_t> parentNodeIds;
    parentNodeIds.insert(rootNodes[0]->GetNodeId());
    EXPECT_TRUE(m_cache->HasParentNode(grandchildNodes[0]->GetNodeId(), parentNodeIds));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static void CacheNode(NodesCache& cache, IConnectionCR connection, JsonNavNodeR node)
    {
    NavNodeExtendedData extendedData(node);
    extendedData.SetConnectionId(connection.GetId());
    extendedData.SetRulesetId("ruleset_id");
    
    // cache root data source
    DataSourceInfo info(connection.GetId(), "ruleset_id", "locale", nullptr, nullptr);
    cache.Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache the node
    cache.Cache(node, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECInstanceNode)
    {
    // create a similar node in a closed connection
    ECDbTestProject project;
    project.Create("LocateNode_LocatesECInstanceNode", "RulesEngineTest.01.00.ecschema.xml");
    IConnectionPtr connection2 = m_connections.NotifyConnectionOpened(project.GetECDb());
    ECClassCP widgetClass2 = project.GetECDb().Schemas().GetClass("RulesEngineTest", "Widget");
    TestNavNodePtr node2 = TestNodesHelper::CreateInstanceNode(*m_connection, *widgetClass2);
    CacheNode(*m_cache, *connection2, *node2);
    m_connections.NotifyConnectionClosed(*connection2);
    project.GetECDb().CloseDb();

    // create a node in an open connection
    ECClassCP widgetClass1 = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    TestNavNodePtr node1 = TestNodesHelper::CreateInstanceNode(*m_connection, *widgetClass1);
    CacheNode(*m_cache, *m_connection, *node1);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "locale", *node1->GetKey());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(node1->Equals(*locatedNode));

    // verify the node is not found when key is invalid
    ECClassInstanceKey invalidKey(
        node1->GetKey()->AsECInstanceNodeKey()->GetECClass(),
        ECInstanceId(node1->GetKey()->AsECInstanceNodeKey()->GetInstanceId().GetValue() + 100));
    locatedNode = m_cache->LocateNode(*m_connection, "locale", *ECInstanceNodeKey::Create(invalidKey, { "wrong hash" }));
    ASSERT_TRUE(locatedNode.IsNull());
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
    TestNavNodePtr node2 = TestNodesHelper::CreateClassGroupingNode(*m_connection, *widgetClass2, "test label");
    CacheNode(*m_cache, *connection2, *node2);
    m_connections.NotifyConnectionClosed(*connection2);
    project.GetECDb().CloseDb();

    // create a node in an open connection
    ECClassCP widgetClass1 = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    TestNavNodePtr node1 = TestNodesHelper::CreateClassGroupingNode(*m_connection, *widgetClass1, "test label");
    CacheNode(*m_cache, *m_connection, *node1);
    
    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "locale", *node1->GetKey());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(node1->Equals(*locatedNode));

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
    TestNavNodePtr node2 = TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *widgetClass2, *groupingProperty2, "test label", groupingValue, false);
    CacheNode(*m_cache, *connection2, *node2);
    m_connections.NotifyConnectionClosed(*connection2);
    project.GetECDb().CloseDb();

    // create a node in an open connection
    ECClassCP widgetClass1 = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECPropertyCP groupingProperty1 = widgetClass1->GetPropertyP("IntProperty");
    TestNavNodePtr node1 = TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *widgetClass1, *groupingProperty1, "test label", groupingValue, false);
    CacheNode(*m_cache, *m_connection, *node1);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "locale", *node1->GetKey());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(node1->Equals(*locatedNode));

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
    TestNavNodePtr node2 = TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *widgetClass2, *groupingProperty2, "test label", groupingValue, true);
    CacheNode(*m_cache, *connection2, *node2);
    m_connections.NotifyConnectionClosed(*connection2);
    project.GetECDb().CloseDb();

    // create a node in an open connection
    ECClassCP widgetClass1 = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECPropertyCP groupingProperty1 = widgetClass1->GetPropertyP("IntProperty");
    TestNavNodePtr node1 = TestNodesHelper::CreatePropertyGroupingNode(*m_connection, *widgetClass1, *groupingProperty1, "test label", groupingValue, true);
    CacheNode(*m_cache, *m_connection, *node1);
    
    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "locale", *node1->GetKey());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(node1->Equals(*locatedNode));

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
    TestNavNodePtr node2 = TestNodesHelper::CreateLabelGroupingNode(*m_connection, "test label");
    CacheNode(*m_cache, *connection2, *node2);
    m_connections.NotifyConnectionClosed(*connection2);
    project.GetECDb().CloseDb();

    // create a node in an open connection
    TestNavNodePtr node1 = TestNodesHelper::CreateLabelGroupingNode(*m_connection, "test label");
    CacheNode(*m_cache, *m_connection, *node1);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "locale", *node1->GetKey());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(node1->Equals(*locatedNode));

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
    TestNavNodePtr node2 = TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test label", "test descr");
    CacheNode(*m_cache, *connection2, *node2);
    m_connections.NotifyConnectionClosed(*connection2);
    project.GetECDb().CloseDb();

    // create a node in an open connection
    TestNavNodePtr node1 = TestNodesHelper::CreateCustomNode(*m_connection, "test type", "test label", "test descr");
    CacheNode(*m_cache, *m_connection, *node1);
    
    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*m_connection, "locale", *node1->GetKey());
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(node1->Equals(*locatedNode));

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
    DataSourceInfo rootInfo(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    bvector<JsonNavNodeCPtr> rootNodes = FillWithNodes(rootInfo, 2);
    bvector<uint64_t> rootNodeIds;
    std::transform(rootNodes.begin(), rootNodes.end(), std::back_inserter(rootNodeIds), [](NavNodeCPtr node){return node->GetNodeId();});

    // cache child data source 1 (less than required size to add to quick cache)
    DataSourceInfo childrenInfo1(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), &rootNodeIds[0], &rootNodeIds[0]);
    m_cache->Cache(childrenInfo1, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    FillWithNodes(childrenInfo1, NODESCACHE_QUICK_Boundary);
    NavNodesProviderPtr datasource1 = m_cache->GetDataSource(HierarchyLevelInfo(childrenInfo1));
    EXPECT_EQ(NODESCACHE_QUICK_Boundary, datasource1->GetNodesCount());

    // cache child data source 2 (of required size to add to quick cache)
    DataSourceInfo childrenInfo2(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), &rootNodeIds[1], &rootNodeIds[1]);
    m_cache->Cache(childrenInfo2, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    FillWithNodes(childrenInfo2, NODESCACHE_QUICK_Boundary + 1);
    NavNodesProviderPtr datasource2 = m_cache->GetDataSource(HierarchyLevelInfo(childrenInfo2));
    EXPECT_EQ(NODESCACHE_QUICK_Boundary + 1, datasource2->GetNodesCount());

    // attempt to add both providers to quick cache
    m_cache->CacheHierarchyLevel(HierarchyLevelInfo(childrenInfo1), *datasource1);
    m_cache->CacheHierarchyLevel(HierarchyLevelInfo(childrenInfo2), *datasource2);

    // verify only the second one got cached
    EXPECT_NE(datasource1.get(), m_cache->GetDataSource(HierarchyLevelInfo(childrenInfo1)).get());
    EXPECT_EQ(datasource2.get(), m_cache->GetDataSource(HierarchyLevelInfo(childrenInfo2)).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Quick_RemovesPreviousDataSourceIfNewOneIsAddedForTheSameHierarchyLevel)
    {
    // cache root data source
    DataSourceInfo rootInfo(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    bvector<JsonNavNodeCPtr> rootNodes = FillWithNodes(rootInfo, 1);
    uint64_t rootNodeId = rootNodes[0]->GetNodeId();

    // cache child data source
    DataSourceInfo childrenInfo(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), &rootNodeId, &rootNodeId);
    m_cache->Cache(childrenInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    FillWithNodes(childrenInfo, NODESCACHE_QUICK_Boundary + 1);

    // both datasources point to the same data, but they're different instances
    NavNodesProviderPtr datasource1 = m_cache->GetDataSource(HierarchyLevelInfo(childrenInfo));
    NavNodesProviderPtr datasource2 = m_cache->GetDataSource(HierarchyLevelInfo(childrenInfo));

    // add the 1st provider to quick cache
    m_cache->CacheHierarchyLevel(HierarchyLevelInfo(childrenInfo), *datasource1);

    // verify 1st provider is in the quick cache, but not the 2nd one
    EXPECT_EQ(datasource1.get(), m_cache->GetDataSource(HierarchyLevelInfo(childrenInfo)).get());
    EXPECT_NE(datasource2.get(), m_cache->GetDataSource(HierarchyLevelInfo(childrenInfo)).get());
    
    // add the 2nd provider to quick cache
    m_cache->CacheHierarchyLevel(HierarchyLevelInfo(childrenInfo), *datasource2);
    
    // verify 2nd provider is in the quick cache, and the 1st one is removed
    EXPECT_NE(datasource1.get(), m_cache->GetDataSource(HierarchyLevelInfo(childrenInfo)).get());
    EXPECT_EQ(datasource2.get(), m_cache->GetDataSource(HierarchyLevelInfo(childrenInfo)).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Quick_RemovesLastUsedProvidersWhenMaxSizeIsReached)
    {
    // cache root data source
    DataSourceInfo rootInfo(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    bvector<JsonNavNodeCPtr> rootNodes = FillWithNodes(rootInfo, NODESCACHE_QUICK_Size + 1);
    bvector<uint64_t> rootNodeIds;
    std::transform(rootNodes.begin(), rootNodes.end(), std::back_inserter(rootNodeIds), [](NavNodeCPtr node){return node->GetNodeId();});

    // cache NODESCACHE_QUICK_Size providers
    bvector<NavNodesProviderPtr> providers;
    for (size_t i = 0; i < NODESCACHE_QUICK_Size; ++i)
        {
        DataSourceInfo info(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), &rootNodeIds[i], &rootNodeIds[i]);
        m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
        FillWithNodes(info, NODESCACHE_QUICK_Boundary + 1);
        NavNodesProviderPtr provider = m_cache->GetDataSource(HierarchyLevelInfo(info));
        m_cache->CacheHierarchyLevel(HierarchyLevelInfo(info), *provider);
        providers.push_back(provider);
        }

    // verify all providers are in the quick cache
    for (size_t i = 0; i < providers.size(); ++i)
        EXPECT_EQ(providers[i].get(), m_cache->GetDataSource(HierarchyLevelInfo(rootInfo.GetConnectionId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), rootNodeIds[i])).get());

    // add a new provider
    DataSourceInfo info(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), &rootNodeIds[NODESCACHE_QUICK_Size], &rootNodeIds[NODESCACHE_QUICK_Size]);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    FillWithNodes(info, NODESCACHE_QUICK_Boundary + 1);
    NavNodesProviderPtr provider = m_cache->GetDataSource(HierarchyLevelInfo(info));
    m_cache->CacheHierarchyLevel(HierarchyLevelInfo(info), *provider);

    // verify the first cached provider is now removed from cache and the new one is inserted
    EXPECT_NE(providers[0].get(), m_cache->GetDataSource(HierarchyLevelInfo(rootInfo.GetConnectionId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), rootNodeIds[0])).get());
    for (size_t i = 1; i < providers.size(); ++i)
        EXPECT_EQ(providers[i].get(), m_cache->GetDataSource(HierarchyLevelInfo(rootInfo.GetConnectionId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), rootNodeIds[i])).get());
    EXPECT_EQ(provider.get(), m_cache->GetDataSource(HierarchyLevelInfo(rootInfo.GetConnectionId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), rootNodeIds[NODESCACHE_QUICK_Size])).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Settings_ReturnsEmptyListWhenRulesetAndSettingIdsAreInvalid)
    {
    // cache the data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str(), info.GetLocale().c_str()));

    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels("invalid", "any");
    EXPECT_TRUE(related.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Settings_ReturnsOnlyRelatedDataSources)
    {
    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    bvector<JsonNavNodeCPtr> rootNodes = FillWithNodes(info, 2);

    // cache unrelated datasource
    uint64_t rootNodeId0 = rootNodes[0]->GetNodeId();
    DataSourceInfo childInfo0(m_connection->GetId(), "ruleset_id", "locale", &rootNodeId0, &rootNodeId0);
    bvector<UserSettingEntry> settings0 = {UserSettingEntry("setting_0", Json::Value("value0"))};
    m_cache->Cache(childInfo0, DataSourceFilter(), bmap<ECClassId, bool>(), settings0);
    
    // cache related datasource
    uint64_t rootNodeId1 = rootNodes[1]->GetNodeId();
    DataSourceInfo childInfo1(m_connection->GetId(), "ruleset_id", "locale", &rootNodeId1, &rootNodeId1);
    bvector<UserSettingEntry> settings1 = {UserSettingEntry("setting_1", Json::Value("value1")) };
    m_cache->Cache(childInfo1, DataSourceFilter(), bmap<ECClassId, bool>(), settings1);

    // verify the correct datasource is found
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.GetRulesetId().c_str(), "setting_1");
    ASSERT_EQ(1, related.size());
    ASSERT_TRUE(nullptr != related[0].GetPhysicalParentNodeId());
    EXPECT_EQ(rootNodeId1, *related[0].GetPhysicalParentNodeId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsEmptyListWhenInstancesArentRelated)
    {
    // cache the data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str(), info.GetLocale().c_str()));

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.GetConnectionId(), keys);
    EXPECT_TRUE(related.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsEmptyListWhenConnectionsAreDifferent)
    {
    // cache the data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[ECClassId((uint64_t)1)] = false;
    m_cache->Cache(info, DataSourceFilter(), usedClassIds, bvector<UserSettingEntry>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str(), info.GetLocale().c_str()));

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
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[ECClassId((uint64_t)1)] = false;
    m_cache->Cache(info, DataSourceFilter(), usedClassIds, bvector<UserSettingEntry>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str(), info.GetLocale().c_str()));

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.GetConnectionId(), keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsEmptyListWhenClassMatchesButFilterDoesnt)
    {
    ECClassCP widgetHasGadget = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    // cache the data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadget->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Backward, ECInstanceId((uint64_t)123)), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[ECClassId((uint64_t)1)] = false;
    m_cache->Cache(info, filter, usedClassIds, bvector<UserSettingEntry>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str(), info.GetLocale().c_str()));

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.GetConnectionId(), keys);
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
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadgetRelationship->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Backward, ECInstanceId((uint64_t)123)), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[widgetClass->GetId()] = false;
    usedClassIds[gadgetClass->GetId()] = false;
    m_cache->Cache(info, filter, usedClassIds, bvector<UserSettingEntry>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str(), info.GetLocale().c_str()));

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(gadgetClass->GetId(), filter.GetRelatedInstanceInfo()->m_instanceId));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.GetConnectionId(), keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info);
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
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadgetRelationship->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Forward, widgetId), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[widgetClass->GetId()] = false;
    usedClassIds[gadgetClass->GetId()] = false;
    m_cache->Cache(info, filter, usedClassIds, bvector<UserSettingEntry>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str(), info.GetLocale().c_str()));

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(gadgetClass->GetId(), gadgetId));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.GetConnectionId(), keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info);
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
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadgetRelationship->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Backward, gadgetId), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[widgetClass->GetId()] = false;
    usedClassIds[gadgetClass->GetId()] = false;
    m_cache->Cache(info, filter, usedClassIds, bvector<UserSettingEntry>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str(), info.GetLocale().c_str()));

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(widgetClass->GetId(), widgetId));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.GetConnectionId(), keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info);
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
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadgetRelationship->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Both, gadgetId), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[widgetClass->GetId()] = false;
    usedClassIds[gadgetClass->GetId()] = false;
    m_cache->Cache(info, filter, usedClassIds, bvector<UserSettingEntry>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str(), info.GetLocale().c_str()));

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(widgetClass->GetId(), widgetId));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.GetConnectionId(), keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info);
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
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadgetRelationship->GetId());
    relationshipIds.insert(widgetHasGadgetsRelationship->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Backward, gadgetId), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[widgetClass->GetId()] = false;
    usedClassIds[gadgetClass->GetId()] = false;
    m_cache->Cache(info, filter, usedClassIds, bvector<UserSettingEntry>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str(), info.GetLocale().c_str()));

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(widgetClass->GetId(), widgetId));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.GetConnectionId(), keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsEmptyListWhenChildNodeGroupedInstanceKeysMatchByClassId)
    {
    // cache the data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str(), info.GetLocale().c_str()));
    
    // create a node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId("ruleset_id");
    extendedData.SetGroupedInstanceKey(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)2)));

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)2), ECInstanceId((uint64_t)2)));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.GetConnectionId(), keys);
    EXPECT_TRUE(related.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsEmptyListWhenChildNodeGroupedInstanceKeysMatchByInstanceId)
    {
    // cache the data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str(), info.GetLocale().c_str()));
    
    // create a node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId("ruleset_id");
    extendedData.SetGroupedInstanceKey(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)2)));

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.GetConnectionId(), keys);
    EXPECT_TRUE(related.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenChildNodeGroupedInstanceKeysMatch)
    {
    // cache the data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str(), info.GetLocale().c_str()));
    
    // create a node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId("ruleset_id");
    extendedData.SetGroupedInstanceKey(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)2)));
    m_cache->Cache(*node, false);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)2)));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.GetConnectionId(), keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenChildNodeGroupedInstanceKeysMatchButFilterDoesnt)
    {
    ECClassCP widgetHasGadget = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    // cache the data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadget->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Backward, ECInstanceId((uint64_t)123)), nullptr);
    bmap<ECClassId, bool> usedClassIds;
    usedClassIds[ECClassId((uint64_t)1)] = false;
    m_cache->Cache(info, filter, usedClassIds, bvector<UserSettingEntry>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str(), info.GetLocale().c_str()));

    // create a node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId("ruleset_id");
    extendedData.SetGroupedInstanceKey(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)2)));
    m_cache->Cache(*node, false);

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)2)));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.GetConnectionId(), keys);
    ASSERT_EQ(1, related.size());
    EXPECT_EQ(related[0], info);
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
    DataSourceInfo info1(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info1, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache root data source for the second connection
    DataSourceInfo info2(connection2->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info2, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // verify both data sources got cached
    EXPECT_TRUE(m_cache->IsDataSourceCached(m_connection->GetId(), "ruleset_id", "locale"));
    EXPECT_TRUE(m_cache->IsDataSourceCached(connection2->GetId(), "ruleset_id", "locale"));

    m_connections.NotifyConnectionClosed(*connection2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ClearRootDatasourceCacheIfRelatedSettingsValuesHasChanged)
    {
    bvector<UserSettingEntry> settingsValues = {UserSettingEntry("setting_id", Json::Value("value0"))};
    DataSourceInfo rootInfo(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bmap<ECClassId, bool>(), settingsValues);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->GetDataSource(HierarchyLevelInfo(rootInfo.GetConnectionId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), nullptr), false).IsValid());
    
    // verify what valid datasource is not returned with different user settings
    m_userSettings.GetSettings("ruleset_id").SetSettingValue("setting_id", "value1");
    NavNodesProviderPtr cachedValidDatasource = m_cache->GetDataSource(HierarchyLevelInfo(rootInfo.GetConnectionId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), nullptr));
    EXPECT_TRUE(cachedValidDatasource.IsNull());
    EXPECT_TRUE(m_cache->GetDataSource(HierarchyLevelInfo(rootInfo.GetConnectionId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), nullptr), false).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, DoesntClearRootDatasourceCacheIfRelatedSettingsValuesHasNotChanged)
    {
    bvector<UserSettingEntry> settingsValues = {UserSettingEntry("setting_id", Json::Value(10))};
    DataSourceInfo rootInfo(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bmap<ECClassId, bool>(), settingsValues);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->GetDataSource(HierarchyLevelInfo(rootInfo.GetConnectionId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), nullptr), false).IsValid());
    
    // verify what valid datasource is returned with same user settings
    m_userSettings.GetSettings("ruleset_id").SetSettingIntValue("setting_id", 10);
    NavNodesProviderPtr cachedValidDatasource = m_cache->GetDataSource(HierarchyLevelInfo(rootInfo.GetConnectionId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), nullptr));
    EXPECT_TRUE(cachedValidDatasource.IsValid());
    EXPECT_TRUE(m_cache->GetDataSource(HierarchyLevelInfo(rootInfo.GetConnectionId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), nullptr), false).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Mantas.Kontrimas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ClearChildDatasourceCacheIfRelatedSettingsValuesHasChanged)
    {
    // cache root data source
    DataSourceInfo rootInfo(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache root node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode(*m_connection, "type", "label", "descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    m_cache->Cache(*node, false);
    uint64_t nodeId = node->GetNodeId();

    // cache child data source
    bvector<UserSettingEntry> settingsValues = {UserSettingEntry("setting_id", Json::Value(1))};
    DataSourceInfo childrenInfo(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), &nodeId, &nodeId);
    m_cache->Cache(childrenInfo, DataSourceFilter(), bmap<ECClassId, bool>(), settingsValues);

    // verify the data source exists now
    EXPECT_TRUE(m_cache->GetDataSource(HierarchyLevelInfo(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), nullptr), false).IsValid());
    EXPECT_TRUE(m_cache->GetDataSource(DataSourceInfo(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), &nodeId, &nodeId), false).IsValid());

    // verify what valid datasource is not returned with different user settings
    m_userSettings.GetSettings("ruleset_id").SetSettingIntValue("setting_id", 10);
    NavNodesProviderPtr cachedValidDatasource = m_cache->GetDataSource(HierarchyLevelInfo(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), nullptr));
    NavNodesProviderPtr cachedValidChildDatasource = m_cache->GetDataSource(DataSourceInfo(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), &nodeId, &nodeId));
    EXPECT_TRUE(cachedValidDatasource.IsValid());
    EXPECT_TRUE(cachedValidChildDatasource.IsNull());
    EXPECT_TRUE(m_cache->GetDataSource(HierarchyLevelInfo(rootInfo.GetConnectionId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), nullptr), false).IsValid());
    EXPECT_TRUE(m_cache->GetDataSource(DataSourceInfo(m_connection->GetId(), rootInfo.GetRulesetId(), rootInfo.GetLocale(), &nodeId, &nodeId), false).IsNull());
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

    NodesCache* _CreateNodesCache(BeFileName tempDir) override
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
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    bvector<JsonNavNodeCPtr> nodes = FillWithNodes(info, 2, true);
    EXPECT_FALSE(m_cache->GetDataSource(info).IsNull());
    EXPECT_TRUE(m_cache->IsDataSourceCached(nodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->IsDataSourceCached(nodes[1]->GetNodeId()));

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
    DataSourceInfo info2(m_connection->GetId(), info.GetRulesetId(), info.GetLocale(), info.GetPhysicalParentNodeId(), info.GetVirtualParentNodeId());

    // cache should not be cleaned
    EXPECT_FALSE(m_cache->GetDataSource(info2).IsNull());
    EXPECT_TRUE(m_cache->IsDataSourceCached(nodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->IsDataSourceCached(nodes[1]->GetNodeId()));
    }
#endif
    
/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, ClearSharedCacheIfHierarchyWasModified)
    {
    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache some nodes
    bvector<JsonNavNodeCPtr> nodes = FillWithNodes(info, 2, true);
    EXPECT_FALSE(m_cache->GetDataSource(info).IsNull());
    EXPECT_TRUE(m_cache->IsDataSourceCached(nodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->IsDataSourceCached(nodes[1]->GetNodeId()));

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
    DataSourceInfo info2(m_connection->GetId(), info.GetRulesetId(), info.GetLocale(), info.GetPhysicalParentNodeId(), info.GetVirtualParentNodeId());

    // cache should be empty for this connection
    EXPECT_TRUE(m_cache->GetDataSource(info2).IsNull());
    EXPECT_FALSE(m_cache->IsDataSourceCached(nodes[0]->GetNodeId()));
    EXPECT_FALSE(m_cache->IsDataSourceCached(nodes[1]->GetNodeId()));
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
    DataSourceInfo info(m_connection->GetId(), ruleset->GetRuleSetId(), "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache some nodes
    bvector<JsonNavNodeCPtr> nodes = FillWithNodes(info, 2, true);
    EXPECT_FALSE(m_cache->GetDataSource(info).IsNull());
    EXPECT_TRUE(m_cache->IsDataSourceCached(nodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->IsDataSourceCached(nodes[1]->GetNodeId()));

    // mock new session: app is turned on with modified ruleset and OnRulesetCreated is called
    ruleset->AddPresentationRule(*new ChildNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree));
    m_cache->OnRulesetCreated(*ruleset);

    // verify cache is cleared
    EXPECT_TRUE(m_cache->GetDataSource(info).IsNull());
    EXPECT_FALSE(m_cache->IsDataSourceCached(nodes[0]->GetNodeId()));
    EXPECT_FALSE(m_cache->IsDataSourceCached(nodes[1]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, ClearsCacheIfCacheFileSizeExceedsLimit)
    {
    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache some nodes
    bvector<JsonNavNodeCPtr> nodes = FillWithNodes(info, 2, true);
    EXPECT_FALSE(m_cache->GetDataSource(info).IsNull());
    EXPECT_TRUE(m_cache->IsDataSourceCached(nodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->IsDataSourceCached(nodes[1]->GetNodeId()));

    // set cache limit
    m_cache->SetCacheFileSizeLimit(1);

    // close cache
    DELETE_AND_CLEAR(m_cache);

    // open cache
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    m_cache = new NodesCache(tempDir, m_nodesFactory, m_nodesProviderContextFactory, m_connections, m_userSettings, m_ecsqlStatements, NodesCacheType::Disk);

    // verify cache is cleared
    EXPECT_TRUE(m_cache->GetDataSource(info).IsNull());
    EXPECT_FALSE(m_cache->IsDataSourceCached(nodes[0]->GetNodeId()));
    EXPECT_FALSE(m_cache->IsDataSourceCached(nodes[1]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, ClearsOldestConnectionCacheWhenCacheFileSizeExceedsLimit)
    {
    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache some nodes
    FillWithNodes(info, 30, true);
    EXPECT_FALSE(m_cache->GetDataSource(info).IsNull());
    m_connections.NotifyConnectionClosed(*m_connection);

    // open second connection and cache root data source
    ECDbTestProject project;
    project.Create("ClearsOldestConnectionCacheWhenCacheFileSizeExceedsLimit");
    IConnectionPtr connection2 = m_connections.NotifyConnectionOpened(project.GetECDb());

    DataSourceInfo info2(connection2->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info2, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache some nodes
    FillWithNodes(info2, 40, true);
    EXPECT_FALSE(m_cache->GetDataSource(info2).IsNull());
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
    DataSourceInfo newInfo(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    EXPECT_TRUE(m_cache->GetDataSource(newInfo).IsNull());

    // verify second connection cache wasn't deleted
    connection2 = m_connections.NotifyConnectionOpened(project.GetECDb());
    DataSourceInfo newInfo2(connection2->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    EXPECT_FALSE(m_cache->GetDataSource(newInfo2).IsNull());
    m_connections.NotifyConnectionClosed(*connection2);
    project.GetECDb().CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Saulius.Skliutas                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DiskNodesCacheTests, DoesntClearCacheWhenCacheFileSizeAndLimitAreEqual)
    {
    // cache root data source
    DataSourceInfo info(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache some nodes
    FillWithNodes(info, 30, true);
    EXPECT_FALSE(m_cache->GetDataSource(info).IsNull());
    m_connections.NotifyConnectionClosed(*m_connection);

    // open second connection and cache root data source
    ECDbTestProject project;
    project.Create("DoesntClearCacheWhenCacheFileSizeAndLimitAreEqual");
    IConnectionPtr connection2 = m_connections.NotifyConnectionOpened(project.GetECDb());

    DataSourceInfo info2(connection2->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    m_cache->Cache(info2, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());

    // cache some nodes
    FillWithNodes(info2, 40, true);
    EXPECT_FALSE(m_cache->GetDataSource(info2).IsNull());
    m_connections.NotifyConnectionClosed(*connection2);

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

    // verify first connection cache was deleted
    m_connection = m_connections.NotifyConnectionOpened(GetDb());
    DataSourceInfo newInfo(m_connection->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    EXPECT_FALSE(m_cache->GetDataSource(newInfo).IsNull());

    // verify second connection cache wasn't deleted
    connection2 = m_connections.NotifyConnectionOpened(project.GetECDb());
    DataSourceInfo newInfo2(connection2->GetId(), "ruleset_id", "locale", nullptr, nullptr);
    EXPECT_FALSE(m_cache->GetDataSource(newInfo2).IsNull());
    m_connections.NotifyConnectionClosed(*connection2);
    project.GetECDb().CloseDb();
    }
