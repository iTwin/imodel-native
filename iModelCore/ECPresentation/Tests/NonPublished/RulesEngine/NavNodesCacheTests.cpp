/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/NavNodesCacheTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
* @bsiclass                                     Grigas.Petraitis                04/2017
+===============+===============+===============+===============+===============+======*/
struct NodesCacheTests : ::testing::Test
    {
    static ECDbTestProject* s_project;

    JsonNavNodesFactory m_nodesFactory;
    TestNodesProviderContextFactory m_nodesProviderContextFactory;
    TestConnectionCache m_connectionCache;
    NodesCache* m_cache;

    void SetUp() override
        {
        if (!s_project->GetECDb().IsDbOpen())
            s_project->Open("NodesCacheTests");

        BeFileName temporaryDirectory;
        BeTest::GetHost().GetTempDir(temporaryDirectory);
        m_cache = new NodesCache(temporaryDirectory, m_nodesFactory, m_nodesProviderContextFactory, m_connectionCache);
        m_connectionCache.Cache(s_project->GetECDb());
        }

    void TearDown() override
        {
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
        TestNavNodePtr node = TestNavNode::Create(&GetDb());
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
            DataSourceInfo childInfo(info.GetConnectionId(), info.GetRulesetId(), &nodeId, &nodeId);
            m_cache->Cache(childInfo, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
            }
        }
    return nodes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsNullsWhenEmpty)
    {
    EXPECT_FALSE(m_cache->IsDataSourceCached(GetDb().GetDbGuid(), "test ruleset id"));
    EXPECT_FALSE(m_cache->IsDataSourceCached(999));
    EXPECT_FALSE(m_cache->IsNodeCached(999));

    EXPECT_TRUE(m_cache->GetDataSource(HierarchyLevelInfo(GetDb().GetDbGuid(), "test ruleset id", 999)).IsNull());
    EXPECT_TRUE(m_cache->GetDataSource(DataSourceInfo(GetDb().GetDbGuid(), "test ruleset id", nullptr, nullptr)).IsNull());
    EXPECT_TRUE(m_cache->GetDataSource(999).IsNull());
    EXPECT_TRUE(m_cache->GetNode(999).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedRootDataSource)
    {
    // cache root data source
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // verify the data source is cached
    EXPECT_TRUE(m_cache->IsDataSourceCached(GetDb().GetDbGuid(), info.GetRulesetId().c_str()));
    EXPECT_TRUE(m_cache->GetDataSource(HierarchyLevelInfo(GetDb().GetDbGuid(), info.GetRulesetId(), nullptr)).IsValid());
    EXPECT_TRUE(m_cache->GetDataSource(DataSourceInfo(GetDb().GetDbGuid(), info.GetRulesetId(), nullptr, nullptr)).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedRootNode)
    {
    // cache root data source
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // create the root node
    TestNavNodePtr node = TestNavNode::Create(&GetDb());
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(GetDb().GetDbGuid());
    extendedData.SetRulesetId(info.GetRulesetId().c_str());
    
    // the data source should be empty
    NavNodesProviderPtr cachedProvider = m_cache->GetDataSource(HierarchyLevelInfo(GetDb().GetDbGuid(), info.GetRulesetId(), nullptr));
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(0, cachedProvider->GetNodesCount());

    // cache the node
    m_cache->Cache(*node, false);

    // verify the node is cached
    cachedProvider = m_cache->GetDataSource(HierarchyLevelInfo(GetDb().GetDbGuid(), info.GetRulesetId(), nullptr));
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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // create the root node
    TestNavNodePtr node = TestNavNode::Create(&GetDb());
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(GetDb().GetDbGuid());
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
    DataSourceInfo rootInfo(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache root node
    TestNavNodePtr node = TestNavNode::Create(&GetDb());
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(GetDb().GetDbGuid());
    extendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    m_cache->Cache(*node, false);
    uint64_t nodeId = node->GetNodeId();

    // no child data source should exist
    EXPECT_TRUE(m_cache->GetDataSource(HierarchyLevelInfo(GetDb().GetDbGuid(), rootInfo.GetRulesetId(), nodeId)).IsNull());
    EXPECT_TRUE(m_cache->GetDataSource(DataSourceInfo(GetDb().GetDbGuid(), rootInfo.GetRulesetId(), &nodeId, &nodeId)).IsNull());

    // cache child data source
    DataSourceInfo childrenInfo(GetDb().GetDbGuid(), rootInfo.GetRulesetId(), &nodeId, &nodeId);
    m_cache->Cache(childrenInfo, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // verify the data source exists now
    EXPECT_TRUE(m_cache->IsDataSourceCached(nodeId));
    EXPECT_TRUE(m_cache->GetDataSource(HierarchyLevelInfo(GetDb().GetDbGuid(), rootInfo.GetRulesetId(), nodeId)).IsValid());
    EXPECT_TRUE(m_cache->GetDataSource(DataSourceInfo(GetDb().GetDbGuid(), rootInfo.GetRulesetId(), &nodeId, &nodeId)).IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsCachedChildNode)
    {
    // cache root data source
    DataSourceInfo rootInfo(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache root node
    TestNavNodePtr rootNode = TestNavNode::Create(&GetDb());
    NavNodeExtendedData rootExtendedData(*rootNode);
    rootExtendedData.SetConnectionId(GetDb().GetDbGuid());
    rootExtendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    m_cache->Cache(*rootNode, false);
    uint64_t nodeId = rootNode->GetNodeId();
    
    // cache child data source
    DataSourceInfo childrenInfo(GetDb().GetDbGuid(), rootInfo.GetRulesetId(), &nodeId, &nodeId);
    m_cache->Cache(childrenInfo, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // the data source should be empty
    NavNodesProviderPtr cachedProvider = m_cache->GetDataSource(HierarchyLevelInfo(GetDb().GetDbGuid(), rootInfo.GetRulesetId(), nodeId));
    ASSERT_TRUE(cachedProvider.IsValid());
    EXPECT_EQ(0, cachedProvider->GetNodesCount());

    // cache child node
    TestNavNodePtr childNode = TestNavNode::Create(&GetDb());
    NavNodeExtendedData childExtendedData(*childNode);
    childExtendedData.SetConnectionId(GetDb().GetDbGuid());
    childExtendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    childExtendedData.SetVirtualParentId(nodeId);
    m_cache->Cache(*childNode, false);    
    
    // verify the child node is cached
    cachedProvider = m_cache->GetDataSource(HierarchyLevelInfo(GetDb().GetDbGuid(), rootInfo.GetRulesetId(), nodeId));
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
    DataSourceInfo rootInfo(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache root node
    TestNavNodePtr rootNode = TestNavNode::Create(&GetDb());
    NavNodeExtendedData rootExtendedData(*rootNode);
    rootExtendedData.SetConnectionId(GetDb().GetDbGuid());
    rootExtendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    m_cache->Cache(*rootNode, false);
    uint64_t rootNodeId = rootNode->GetNodeId();
    
    // cache child data source
    DataSourceInfo childrenInfo(GetDb().GetDbGuid(), rootInfo.GetRulesetId(), &rootNodeId, &rootNodeId);
    m_cache->Cache(childrenInfo, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // create child node
    TestNavNodePtr childNode = TestNavNode::Create(&GetDb());
    NavNodeExtendedData childExtendedData(*childNode);
    childExtendedData.SetConnectionId(GetDb().GetDbGuid());
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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // create the node
    TestNavNodePtr node = TestNavNode::Create(&GetDb());
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(GetDb().GetDbGuid());
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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // create the node
    TestNavNodePtr node = TestNavNode::Create(&GetDb());
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(GetDb().GetDbGuid());
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
    DataSourceInfo info1(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info1, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    DataSourceInfo info2(info1.GetConnectionId(), info1.GetRulesetId(), info1.GetPhysicalParentNodeId(), info1.GetVirtualParentNodeId());
    m_cache->Cache(info2, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

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
    m_connectionCache.Cache(project2.GetECDb());
    
    // cache root data source for the first connection
    DataSourceInfo rootInfo1(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(rootInfo1, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
    
    // cache root data source for the second connection
    DataSourceInfo rootInfo2(project2.GetECDbCR().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(rootInfo2, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // verify correct sources are returned
    NavNodesProviderPtr cached1 = m_cache->GetDataSource(rootInfo1);
    ASSERT_TRUE(cached1.IsValid());
    EXPECT_EQ(&GetDb(), &cached1->GetContext().GetDb());

    NavNodesProviderPtr cached2 = m_cache->GetDataSource(rootInfo2);
    ASSERT_TRUE(cached2.IsValid());
    EXPECT_EQ(&project2.GetECDbCR(), &cached2->GetContext().GetDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ReturnsDataSourcesWithValidRulesetIds)
    {    
    // cache root data source for the first ruleset
    DataSourceInfo rootInfo1(GetDb().GetDbGuid(), "ruleset_id1", nullptr, nullptr);
    m_cache->Cache(rootInfo1, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
    
    // cache root data source for the second ruleset
    DataSourceInfo rootInfo2(GetDb().GetDbGuid(), "ruleset_id2", nullptr, nullptr);
    m_cache->Cache(rootInfo2, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // verify correct sources are returned
    NavNodesProviderPtr cached1 = m_cache->GetDataSource(rootInfo1);
    ASSERT_TRUE(cached1.IsValid());
    EXPECT_STREQ(rootInfo1.GetRulesetId().c_str(), cached1->GetContext().GetRuleset().GetRuleSetId().c_str());

    NavNodesProviderPtr cached2 = m_cache->GetDataSource(rootInfo2);
    ASSERT_TRUE(cached2.IsValid());
    EXPECT_STREQ(rootInfo2.GetRulesetId().c_str(), cached2->GetContext().GetRuleset().GetRuleSetId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Clear_Full)
    {
    // cache root data source
    DataSourceInfo rootInfo(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache root node
    TestNavNodePtr rootNode = TestNavNode::Create(&GetDb());
    NavNodeExtendedData rootExtendedData(*rootNode);
    rootExtendedData.SetConnectionId(GetDb().GetDbGuid());
    rootExtendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    m_cache->Cache(*rootNode, false);
    uint64_t rootNodeId = rootNode->GetNodeId();
    
    // cache child data source
    DataSourceInfo childrenInfo(GetDb().GetDbGuid(), rootInfo.GetRulesetId(), &rootNodeId, &rootNodeId);
    m_cache->Cache(childrenInfo, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache child node
    TestNavNodePtr childNode = TestNavNode::Create(&GetDb());
    NavNodeExtendedData childExtendedData(*childNode);
    childExtendedData.SetConnectionId(GetDb().GetDbGuid());
    childExtendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    childExtendedData.SetVirtualParentId(rootNodeId);
    m_cache->Cache(*childNode, false);
    uint64_t childNodeId = childNode->GetNodeId();
    
    // cache root data source for a different connection
    ECDbTestProject project2;
    project2.Create("test2");
    m_connectionCache.Cache(project2.GetECDb());
    DataSourceInfo rootInfo2(project2.GetECDbCR().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(rootInfo2, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
    
    // cache root data source for a different ruleset
    DataSourceInfo rootInfo3(GetDb().GetDbGuid(), "ruleset_id2", nullptr, nullptr);
    m_cache->Cache(rootInfo3, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // make sure everything's cached
    EXPECT_TRUE(m_cache->IsDataSourceCached(project2.GetECDbCR().GetDbGuid(), "ruleset_id"));
    EXPECT_TRUE(m_cache->IsDataSourceCached(GetDb().GetDbGuid(), "ruleset_id2"));
    EXPECT_TRUE(m_cache->IsDataSourceCached(GetDb().GetDbGuid(), "ruleset_id"));
    EXPECT_TRUE(m_cache->IsNodeCached(rootNodeId));
    EXPECT_TRUE(m_cache->IsDataSourceCached(rootNodeId));
    EXPECT_TRUE(m_cache->IsNodeCached(childNodeId));

    // fully clear the cache
    m_cache->Clear();

    // verify everything's gone
    EXPECT_FALSE(m_cache->IsDataSourceCached(project2.GetECDbCR().GetDbGuid(), "ruleset_id"));
    EXPECT_FALSE(m_cache->IsDataSourceCached(GetDb().GetDbGuid(), "ruleset_id2"));
    EXPECT_FALSE(m_cache->IsDataSourceCached(GetDb().GetDbGuid(), "ruleset_id"));
    EXPECT_FALSE(m_cache->IsNodeCached(rootNodeId));
    EXPECT_FALSE(m_cache->IsDataSourceCached(rootNodeId));
    EXPECT_FALSE(m_cache->IsNodeCached(childNodeId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Clear_ByConnection)
    {
    // cache root data source
    DataSourceInfo rootInfo(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache root node
    TestNavNodePtr rootNode = TestNavNode::Create(&GetDb());
    NavNodeExtendedData rootExtendedData(*rootNode);
    rootExtendedData.SetConnectionId(GetDb().GetDbGuid());
    rootExtendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    m_cache->Cache(*rootNode, false);
    uint64_t rootNodeId = rootNode->GetNodeId();
    
    // cache child data source
    DataSourceInfo childrenInfo(GetDb().GetDbGuid(), rootInfo.GetRulesetId(), &rootNodeId, &rootNodeId);
    m_cache->Cache(childrenInfo, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache child node
    TestNavNodePtr childNode = TestNavNode::Create(&GetDb());
    NavNodeExtendedData childExtendedData(*childNode);
    childExtendedData.SetConnectionId(GetDb().GetDbGuid());
    childExtendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    childExtendedData.SetVirtualParentId(rootNodeId);
    m_cache->Cache(*childNode, false);
    uint64_t childNodeId = childNode->GetNodeId();
    
    // cache root data source for a different connection
    ECDbTestProject project2;
    project2.Create("test2");
    m_connectionCache.Cache(project2.GetECDb());
    DataSourceInfo rootInfo2(project2.GetECDbCR().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(rootInfo2, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // make sure everything's cached
    EXPECT_TRUE(m_cache->IsDataSourceCached(project2.GetECDbCR().GetDbGuid(), "ruleset_id"));
    EXPECT_TRUE(m_cache->IsDataSourceCached(GetDb().GetDbGuid(), "ruleset_id"));
    EXPECT_TRUE(m_cache->IsNodeCached(rootNodeId));
    EXPECT_TRUE(m_cache->IsDataSourceCached(rootNodeId));
    EXPECT_TRUE(m_cache->IsNodeCached(childNodeId));

    // fully clear the cache
    m_cache->Clear(&GetDb());

    // verify everything related to primary connection is now gone
    EXPECT_TRUE(m_cache->IsDataSourceCached(project2.GetECDbCR().GetDbGuid(), "ruleset_id"));
    EXPECT_FALSE(m_cache->IsDataSourceCached(GetDb().GetDbGuid(), "ruleset_id"));
    EXPECT_FALSE(m_cache->IsNodeCached(rootNodeId));
    EXPECT_FALSE(m_cache->IsDataSourceCached(rootNodeId));
    EXPECT_FALSE(m_cache->IsNodeCached(childNodeId));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Clear_ByRulesetId)
    {
    // cache root data source
    DataSourceInfo rootInfo(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache root node
    TestNavNodePtr rootNode = TestNavNode::Create(&GetDb());
    NavNodeExtendedData rootExtendedData(*rootNode);
    rootExtendedData.SetConnectionId(GetDb().GetDbGuid());
    rootExtendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    m_cache->Cache(*rootNode, false);
    uint64_t rootNodeId = rootNode->GetNodeId();
    
    // cache child data source
    DataSourceInfo childrenInfo(GetDb().GetDbGuid(), rootInfo.GetRulesetId(), &rootNodeId, &rootNodeId);
    m_cache->Cache(childrenInfo, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache child node
    TestNavNodePtr childNode = TestNavNode::Create(&GetDb());
    NavNodeExtendedData childExtendedData(*childNode);
    childExtendedData.SetConnectionId(GetDb().GetDbGuid());
    childExtendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    childExtendedData.SetVirtualParentId(rootNodeId);
    m_cache->Cache(*childNode, false);
    uint64_t childNodeId = childNode->GetNodeId();
        
    // cache root data source for a different ruleset
    DataSourceInfo rootInfo3(GetDb().GetDbGuid(), "ruleset_id2", nullptr, nullptr);
    m_cache->Cache(rootInfo3, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // make sure everything's cached
    EXPECT_TRUE(m_cache->IsDataSourceCached(GetDb().GetDbGuid(), "ruleset_id2"));
    EXPECT_TRUE(m_cache->IsDataSourceCached(GetDb().GetDbGuid(), "ruleset_id"));
    EXPECT_TRUE(m_cache->IsNodeCached(rootNodeId));
    EXPECT_TRUE(m_cache->IsDataSourceCached(rootNodeId));
    EXPECT_TRUE(m_cache->IsNodeCached(childNodeId));

    // fully clear the cache
    m_cache->Clear(nullptr, "ruleset_id");

    // verify everything related to "ruleset_id" is now gone
    EXPECT_TRUE(m_cache->IsDataSourceCached(GetDb().GetDbGuid(), "ruleset_id2"));
    EXPECT_FALSE(m_cache->IsDataSourceCached(GetDb().GetDbGuid(), "ruleset_id"));
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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
    
    // verify it got cached
    EXPECT_TRUE(m_cache->IsDataSourceCached(GetDb().GetDbGuid(), info.GetRulesetId().c_str()));

    // remove
    BeGuid removalId = m_cache->CreateRemovalId(HierarchyLevelInfo(info.GetConnectionId(), info.GetRulesetId(), info.GetPhysicalParentNodeId()));
    EXPECT_TRUE(removalId.IsValid());
    m_cache->RemoveDataSource(removalId);

    // verify it got removed
    EXPECT_FALSE(m_cache->IsDataSourceCached(GetDb().GetDbGuid(), info.GetRulesetId().c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, RemovesChildDataSource)
    {
    // cache root data source
    DataSourceInfo rootInfo(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache root node
    TestNavNodePtr node = TestNavNode::Create(&GetDb());
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(GetDb().GetDbGuid());
    extendedData.SetRulesetId(rootInfo.GetRulesetId().c_str());
    m_cache->Cache(*node, false);
    uint64_t nodeId = node->GetNodeId();

    // cache child data source
    DataSourceInfo childrenInfo(GetDb().GetDbGuid(), rootInfo.GetRulesetId(), &nodeId, &nodeId);
    m_cache->Cache(childrenInfo, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // verify child data source is cached
    EXPECT_TRUE(m_cache->IsDataSourceCached(nodeId));

    // remove child data source
    BeGuid removalId = m_cache->CreateRemovalId(HierarchyLevelInfo(childrenInfo.GetConnectionId(), childrenInfo.GetRulesetId(), childrenInfo.GetPhysicalParentNodeId()));
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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
    
    // cache node
    TestNavNodePtr node = TestNavNode::Create(&GetDb());
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(GetDb().GetDbGuid());
    extendedData.SetRulesetId(info.GetRulesetId().c_str());
    m_cache->Cache(*node, false);

    // verify node is cached
    EXPECT_TRUE(m_cache->IsNodeCached(node->GetNodeId()));

    // remove
    BeGuid removalId = m_cache->CreateRemovalId(HierarchyLevelInfo(info.GetConnectionId(), info.GetRulesetId(), info.GetPhysicalParentNodeId()));
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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
    
    // cache node
    TestNavNodePtr node = TestNavNode::Create(&GetDb());
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(GetDb().GetDbGuid());
    extendedData.SetRulesetId(info.GetRulesetId().c_str());
    m_cache->Cache(*node, false);
    uint64_t parentNodeId = node->GetNodeId();
    
    // cache child data source
    DataSourceInfo childrenInfo(GetDb().GetDbGuid(), info.GetRulesetId(), &parentNodeId, &parentNodeId);
    m_cache->Cache(childrenInfo, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // verify child data source is cached
    EXPECT_TRUE(m_cache->IsDataSourceCached(parentNodeId));

    // remove parent data source
    BeGuid removalId = m_cache->CreateRemovalId(HierarchyLevelInfo(info.GetConnectionId(), info.GetRulesetId(), info.GetPhysicalParentNodeId()));
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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache root node
    TestNavNodePtr node = TestNavNode::Create(&GetDb());
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(info.GetConnectionId());
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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache root node
    TestNavNodePtr node = TestNavNode::Create(&GetDb());
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(info.GetConnectionId());
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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache root node
    TestNavNodePtr node = TestNavNode::Create(&GetDb());
    node->SetLabel("A");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(info.GetConnectionId());
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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), {ECClassId((uint64_t)1)}, bvector<Utf8String>());
        
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
    // unused - ECClassCP widgetHasGadgetRelationship = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    // cache the data source
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
        
    // verify we don't find related hierarchy level as the key is not related to the data source
    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.GetConnectionId(), keys);
    EXPECT_EQ(0, related.size());

    // update the datasource related class ids
    bvector<ECClassId> relatedClassIds = {ECClassId((uint64_t)1)};
    m_cache->Update(info, nullptr, &relatedClassIds, nullptr);
    
    // verify we do find related hierarchy level this time as the data source is now related to the lookup keys
    related = m_cache->GetRelatedHierarchyLevels(info.GetConnectionId(), keys);
    EXPECT_EQ(1, related.size());
    }

/*---------------------------------------------------------------------------------**//**
* note: the test uses GetRelatedHierarchyLevels function to test the Update
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, UpdateDataSource_UpdatesRelatedSettingIds)
    {
    // unused - ECClassCP widgetHasGadgetRelationship = GetDb().Schemas().GetClass("RulesEngineTest", "WidgetHasGadget");

    // cache the data source
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
        
    // verify we don't find related hierarchy level as data source is not related to any setting ids
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(info.GetRulesetId().c_str(), "setting_id");
    EXPECT_EQ(0, related.size());

    // update the datasource related setting ids
    bvector<Utf8String> relatedSettingIds = {"setting_id"};
    m_cache->Update(info, nullptr, nullptr, &relatedSettingIds);
    
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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache 2 root nodes
    bvector<JsonNavNodeCPtr> rootNodes = FillWithNodes(info, 2, false);

    // create a data source for root node 0
    uint64_t rootNodeId0 = rootNodes[0]->GetNodeId();
    DataSourceInfo childInfo(info.GetConnectionId(), info.GetRulesetId(), &rootNodeId0, &rootNodeId0);
    m_cache->Cache(childInfo, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // verify rootNodes[0] has a datasource and rootNodes[1] doesnt
    EXPECT_TRUE(m_cache->IsDataSourceCached(rootNodes[0]->GetNodeId()));
    EXPECT_FALSE(m_cache->IsDataSourceCached(rootNodes[1]->GetNodeId()));
    
    // remap
    m_cache->RemapNodeIds(rootNodes[0]->GetNodeId(), rootNodes[1]->GetNodeId());
    
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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache 2 root nodes
    bvector<JsonNavNodeCPtr> rootNodes = FillWithNodes(info, 2, false);
    m_cache->MakeVirtual(*rootNodes[0]);

    // create a data source for root node 0
    uint64_t rootNodeId0 = rootNodes[0]->GetNodeId();
    DataSourceInfo childInfo(info.GetConnectionId(), info.GetRulesetId(), nullptr, &rootNodeId0);
    m_cache->Cache(childInfo, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // verify rootNodes[0] has a datasource and rootNodes[1] doesnt
    EXPECT_TRUE(m_cache->IsDataSourceCached(rootNodes[0]->GetNodeId()));
    EXPECT_FALSE(m_cache->IsDataSourceCached(rootNodes[1]->GetNodeId()));
    
    // remap
    m_cache->RemapNodeIds(rootNodes[0]->GetNodeId(), rootNodes[1]->GetNodeId());
    
    // verify rootNodes[1] has a datasource and rootNodes[0] doesnt
    EXPECT_FALSE(m_cache->IsDataSourceCached(rootNodes[0]->GetNodeId()));
    EXPECT_TRUE(m_cache->IsDataSourceCached(rootNodes[1]->GetNodeId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, HasParentNode_ReturnsFalseForEmptySet)
    {
    // cache root data source
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache root nodes
    bvector<JsonNavNodeCPtr> rootNodes = FillWithNodes(info, 2, true);
    
    // cache child node for root node 0
    uint64_t rootNodeId0 = rootNodes[0]->GetNodeId();
    bvector<JsonNavNodeCPtr> childNodes = FillWithNodes(DataSourceInfo(info.GetConnectionId(), info.GetRulesetId(), &rootNodeId0, &rootNodeId0), 1);
    
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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache root nodes
    bvector<JsonNavNodeCPtr> rootNodes = FillWithNodes(info, 2, true);
    
    // cache child node for root node 0
    uint64_t rootNodeId0 = rootNodes[0]->GetNodeId();
    bvector<JsonNavNodeCPtr> childNodes = FillWithNodes(DataSourceInfo(info.GetConnectionId(), info.GetRulesetId(), &rootNodeId0, &rootNodeId0), 1);
    
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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache root nodes
    bvector<JsonNavNodeCPtr> rootNodes = FillWithNodes(info, 1, true);
    
    // cache child node for root node
    uint64_t rootNodeId = rootNodes[0]->GetNodeId();
    bvector<JsonNavNodeCPtr> childNodes = FillWithNodes(DataSourceInfo(info.GetConnectionId(), info.GetRulesetId(), &rootNodeId, &rootNodeId), 1, true);

    // cache grandchild node for child node
    uint64_t childNodeId = childNodes[0]->GetNodeId();
    bvector<JsonNavNodeCPtr> grandchildNodes = FillWithNodes(DataSourceInfo(info.GetConnectionId(), info.GetRulesetId(), &childNodeId, &childNodeId), 1);
    
    bset<uint64_t> parentNodeIds;
    parentNodeIds.insert(rootNodes[0]->GetNodeId());
    EXPECT_TRUE(m_cache->HasParentNode(grandchildNodes[0]->GetNodeId(), parentNodeIds));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECInstanceNode)
    {
    // create the root node
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    TestNavNodePtr node = TestNodesHelper::CreateInstanceNode(*widgetClass);
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(GetDb().GetDbGuid());
    extendedData.SetRulesetId("ruleset_id");
    
    // cache root data source
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache the node
    m_cache->Cache(*node, false);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*ECInstanceNodeKey::Create(*node->GetInstance()));
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(node->Equals(*locatedNode));

    // verify the node is not found when key is invalid
    ECInstanceKey invalidKey(
        ECClassId(node->GetKey().AsECInstanceNodeKey()->GetECClassId().GetValue() + 1),
        ECInstanceId(node->GetKey().AsECInstanceNodeKey()->GetInstanceId().GetValue() + 1));
    locatedNode = m_cache->LocateNode(*ECInstanceNodeKey::Create(invalidKey));
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECClassGroupingNode)
    {
    // create the root node
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    TestNavNodePtr node = TestNodesHelper::CreateClassGroupingNode(*widgetClass, "test label");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(GetDb().GetDbGuid());
    extendedData.SetRulesetId("ruleset_id");
    
    // cache root data source
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache the node
    m_cache->Cache(*node, false);
    
    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*ECClassGroupingNodeKey::Create(node->GetNodeId(), widgetClass->GetId()));
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(node->Equals(*locatedNode));

    // verify the node is not found when node id doesnt match
    locatedNode = m_cache->LocateNode(*ECClassGroupingNodeKey::Create(node->GetNodeId() + 1, widgetClass->GetId()));
    ASSERT_TRUE(locatedNode.IsNull());

    // verify the node is not found when class id doesnt match
    locatedNode = m_cache->LocateNode(*ECClassGroupingNodeKey::Create(node->GetNodeId(), ECClassId(widgetClass->GetId().GetValue() + 1)));
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECPropertyValueGroupingNode)
    {    
    // create the root node
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECPropertyCP groupingProperty = widgetClass->GetPropertyP("IntProperty");
    rapidjson::Document groupingValue;
    groupingValue.SetInt(9);
    TestNavNodePtr node = TestNodesHelper::CreatePropertyGroupingNode(*widgetClass, *groupingProperty, "test label", groupingValue, false);
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(GetDb().GetDbGuid());
    extendedData.SetRulesetId("ruleset_id");
    
    // cache root data source
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache the node
    m_cache->Cache(*node, false);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*ECPropertyGroupingNodeKey::Create(node->GetNodeId(), *widgetClass, *groupingProperty, -1, &groupingValue));
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(node->Equals(*locatedNode));

    // verify the node is not found when node id doesnt match
    locatedNode = m_cache->LocateNode(*ECPropertyGroupingNodeKey::Create(node->GetNodeId() + 1, *widgetClass, *groupingProperty, -1, &groupingValue));
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesECPropertyRangeGroupingNode)
    {    
    // create the root node
    ECClassCP widgetClass = GetDb().Schemas().GetClass("RulesEngineTest", "Widget");
    ECPropertyCP groupingProperty = widgetClass->GetPropertyP("IntProperty");
    rapidjson::Document groupingValue;
    groupingValue.SetInt(2);
    TestNavNodePtr node = TestNodesHelper::CreatePropertyGroupingNode(*widgetClass, *groupingProperty, "test label", groupingValue, true);
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(GetDb().GetDbGuid());
    extendedData.SetRulesetId("ruleset_id");
    
    // cache root data source
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache the node
    m_cache->Cache(*node, false);
    
    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*ECPropertyGroupingNodeKey::Create(node->GetNodeId(), *widgetClass, *groupingProperty, 2, nullptr));
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(node->Equals(*locatedNode));

    // verify the node is not found when node id doesnt match
    locatedNode = m_cache->LocateNode(*ECPropertyGroupingNodeKey::Create(node->GetNodeId() + 1, *widgetClass, *groupingProperty, 2, nullptr));
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesLabelGroupingNode)
    {
    // create the root node
    TestNavNodePtr node = TestNodesHelper::CreateLabelGroupingNode("test label");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(GetDb().GetDbGuid());
    extendedData.SetRulesetId("ruleset_id");
    
    // cache root data source
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache the node
    m_cache->Cache(*node, false);

    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*DisplayLabelGroupingNodeKey::Create(node->GetNodeId(), "test label"));
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(node->Equals(*locatedNode));

    // verify the node is not found when node id doesnt match
    locatedNode = m_cache->LocateNode(*DisplayLabelGroupingNodeKey::Create(node->GetNodeId() + 1, "test label"));
    ASSERT_TRUE(locatedNode.IsNull());

    // verify the node is not found when label doesnt match
    locatedNode = m_cache->LocateNode(*DisplayLabelGroupingNodeKey::Create(node->GetNodeId(), "different label"));
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, LocateNode_LocatesCustomNode)
    {
    // create the root node
    TestNavNodePtr node = TestNodesHelper::CreateCustomNode("test type", "test label", "test descr");
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(GetDb().GetDbGuid());
    extendedData.SetRulesetId("ruleset_id");
    
    // cache root data source
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache the node
    m_cache->Cache(*node, false);
    
    // verify the node is found successfully with valid key
    NavNodeCPtr locatedNode = m_cache->LocateNode(*DisplayLabelGroupingNodeKey::Create(node->GetNodeId(), "test label", "test type"));
    ASSERT_TRUE(locatedNode.IsValid());
    ASSERT_TRUE(node->Equals(*locatedNode));

    // verify the node is not found when node id doesnt match
    locatedNode = m_cache->LocateNode(*DisplayLabelGroupingNodeKey::Create(node->GetNodeId() + 1, "test label", "test type"));
    ASSERT_TRUE(locatedNode.IsNull());

    // verify the node is not found when label doesnt match
    locatedNode = m_cache->LocateNode(*DisplayLabelGroupingNodeKey::Create(node->GetNodeId(), "different label", "test type"));
    ASSERT_TRUE(locatedNode.IsNull());

    // verify the node is not found when type doesnt match
    locatedNode = m_cache->LocateNode(*DisplayLabelGroupingNodeKey::Create(node->GetNodeId(), "test label", "different type"));
    ASSERT_TRUE(locatedNode.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Quick_AddsToQuickCacheWhenDataSourceReachesRequiredSize)
    {
    // cache root data source
    DataSourceInfo rootInfo(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
    bvector<JsonNavNodeCPtr> rootNodes = FillWithNodes(rootInfo, 2);
    bvector<uint64_t> rootNodeIds;
    std::transform(rootNodes.begin(), rootNodes.end(), std::back_inserter(rootNodeIds), [](NavNodeCPtr node){return node->GetNodeId();});

    // cache child data source 1 (less than required size to add to quick cache)
    DataSourceInfo childrenInfo1(GetDb().GetDbGuid(), rootInfo.GetRulesetId(), &rootNodeIds[0], &rootNodeIds[0]);
    m_cache->Cache(childrenInfo1, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
    FillWithNodes(childrenInfo1, NODESCACHE_QUICK_Boundary);
    NavNodesProviderPtr datasource1 = m_cache->GetDataSource(HierarchyLevelInfo(childrenInfo1));
    EXPECT_EQ(NODESCACHE_QUICK_Boundary, datasource1->GetNodesCount());

    // cache child data source 2 (of required size to add to quick cache)
    DataSourceInfo childrenInfo2(GetDb().GetDbGuid(), rootInfo.GetRulesetId(), &rootNodeIds[1], &rootNodeIds[1]);
    m_cache->Cache(childrenInfo2, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
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
    DataSourceInfo rootInfo(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
    bvector<JsonNavNodeCPtr> rootNodes = FillWithNodes(rootInfo, 1);
    uint64_t rootNodeId = rootNodes[0]->GetNodeId();

    // cache child data source
    DataSourceInfo childrenInfo(GetDb().GetDbGuid(), rootInfo.GetRulesetId(), &rootNodeId, &rootNodeId);
    m_cache->Cache(childrenInfo, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
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
    DataSourceInfo rootInfo(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(rootInfo, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
    bvector<JsonNavNodeCPtr> rootNodes = FillWithNodes(rootInfo, NODESCACHE_QUICK_Size + 1);
    bvector<uint64_t> rootNodeIds;
    std::transform(rootNodes.begin(), rootNodes.end(), std::back_inserter(rootNodeIds), [](NavNodeCPtr node){return node->GetNodeId();});

    // cache NODESCACHE_QUICK_Size providers
    bvector<NavNodesProviderPtr> providers;
    for (size_t i = 0; i < NODESCACHE_QUICK_Size; ++i)
        {
        DataSourceInfo info(GetDb().GetDbGuid(), rootInfo.GetRulesetId(), &rootNodeIds[i], &rootNodeIds[i]);
        m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
        FillWithNodes(info, NODESCACHE_QUICK_Boundary + 1);
        NavNodesProviderPtr provider = m_cache->GetDataSource(HierarchyLevelInfo(info));
        m_cache->CacheHierarchyLevel(HierarchyLevelInfo(info), *provider);
        providers.push_back(provider);
        }

    // verify all providers are in the quick cache
    for (size_t i = 0; i < providers.size(); ++i)
        EXPECT_EQ(providers[i].get(), m_cache->GetDataSource(HierarchyLevelInfo(rootInfo.GetConnectionId(), rootInfo.GetRulesetId(), rootNodeIds[i])).get());

    // add a new provider
    DataSourceInfo info(GetDb().GetDbGuid(), rootInfo.GetRulesetId(), &rootNodeIds[NODESCACHE_QUICK_Size], &rootNodeIds[NODESCACHE_QUICK_Size]);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
    FillWithNodes(info, NODESCACHE_QUICK_Boundary + 1);
    NavNodesProviderPtr provider = m_cache->GetDataSource(HierarchyLevelInfo(info));
    m_cache->CacheHierarchyLevel(HierarchyLevelInfo(info), *provider);

    // verify the first cached provider is now removed from cache and the new one is inserted
    EXPECT_NE(providers[0].get(), m_cache->GetDataSource(HierarchyLevelInfo(rootInfo.GetConnectionId(), rootInfo.GetRulesetId(), rootNodeIds[0])).get());
    for (size_t i = 1; i < providers.size(); ++i)
        EXPECT_EQ(providers[i].get(), m_cache->GetDataSource(HierarchyLevelInfo(rootInfo.GetConnectionId(), rootInfo.GetRulesetId(), rootNodeIds[i])).get());
    EXPECT_EQ(provider.get(), m_cache->GetDataSource(HierarchyLevelInfo(rootInfo.GetConnectionId(), rootInfo.GetRulesetId(), rootNodeIds[NODESCACHE_QUICK_Size])).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Settings_ReturnsEmptyListWhenRulesetAndSettingIdsAreInvalid)
    {
    // cache the data source
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str()));

    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels("invalid", "any");
    EXPECT_TRUE(related.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Settings_ReturnsOnlyRelatedDataSources)
    {
    // cache root data source
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
    bvector<JsonNavNodeCPtr> rootNodes = FillWithNodes(info, 2);

    // cache unrelated datasource
    uint64_t rootNodeId0 = rootNodes[0]->GetNodeId();
    DataSourceInfo childInfo0(GetDb().GetDbGuid(), "ruleset_id", &rootNodeId0, &rootNodeId0);
    m_cache->Cache(childInfo0, DataSourceFilter(), bvector<ECClassId>(), {"setting_0"});
    
    // cache related datasource
    uint64_t rootNodeId1 = rootNodes[1]->GetNodeId();
    DataSourceInfo childInfo1(GetDb().GetDbGuid(), "ruleset_id", &rootNodeId1, &rootNodeId1);
    m_cache->Cache(childInfo1, DataSourceFilter(), bvector<ECClassId>(), {"setting_1"});

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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str()));

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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), {ECClassId((uint64_t)1)}, bvector<Utf8String>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str()));

    bset<ECInstanceKey> keys;
    keys.insert(ECInstanceKey(ECClassId((uint64_t)1), ECInstanceId((uint64_t)1)));
    bvector<HierarchyLevelInfo> related = m_cache->GetRelatedHierarchyLevels(BeGuid(true), keys);
    EXPECT_TRUE(related.empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetRelatedHierarchyLevels_Instances_ReturnsDataSourceWhenClassMatchesAndFilterIsNotSpecified)
    {
    // cache the data source
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), {ECClassId((uint64_t)1)}, bvector<Utf8String>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str()));

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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadget->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Backward, ECInstanceId((uint64_t)123)), nullptr);
    m_cache->Cache(info, filter, {ECClassId((uint64_t)1)}, bvector<Utf8String>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str()));

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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadgetRelationship->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Backward, ECInstanceId((uint64_t)123)), nullptr);
    m_cache->Cache(info, filter, {widgetClass->GetId(),gadgetClass->GetId()}, bvector<Utf8String>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str()));

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
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *widgetHasGadgetRelationship->GetRelationshipClassCP(), *widget, *gadget);

    ECInstanceId widgetId, gadgetId;
    ECInstanceId::FromString(widgetId, widget->GetInstanceId().c_str());
    ECInstanceId::FromString(gadgetId, gadget->GetInstanceId().c_str());

    // cache the data source
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadgetRelationship->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Forward, widgetId), nullptr);
    m_cache->Cache(info, filter, {widgetClass->GetId(),gadgetClass->GetId()}, bvector<Utf8String>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str()));

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
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *widgetHasGadgetRelationship->GetRelationshipClassCP(), *widget, *gadget);

    ECInstanceId widgetId, gadgetId;
    ECInstanceId::FromString(widgetId, widget->GetInstanceId().c_str());
    ECInstanceId::FromString(gadgetId, gadget->GetInstanceId().c_str());

    // cache the data source
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadgetRelationship->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Backward, gadgetId), nullptr);
    m_cache->Cache(info, filter, {widgetClass->GetId(),gadgetClass->GetId()}, bvector<Utf8String>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str()));

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
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *widgetHasGadgetRelationship->GetRelationshipClassCP(), *widget, *gadget);

    ECInstanceId widgetId, gadgetId;
    ECInstanceId::FromString(widgetId, widget->GetInstanceId().c_str());
    ECInstanceId::FromString(gadgetId, gadget->GetInstanceId().c_str());

    // cache the data source
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadgetRelationship->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Both, gadgetId), nullptr);
    m_cache->Cache(info, filter, {widgetClass->GetId(),gadgetClass->GetId()}, bvector<Utf8String>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str()));

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
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(*s_project, *widgetClass);
    IECInstancePtr gadget = RulesEngineTestHelpers::InsertInstance(*s_project, *gadgetClass);
    RulesEngineTestHelpers::InsertRelationship(*s_project, *widgetHasGadgetRelationship->GetRelationshipClassCP(), *widget, *gadget);

    ECInstanceId widgetId, gadgetId;
    ECInstanceId::FromString(widgetId, widget->GetInstanceId().c_str());
    ECInstanceId::FromString(gadgetId, gadget->GetInstanceId().c_str());

    // cache the data source
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadgetRelationship->GetId());
    relationshipIds.insert(widgetHasGadgetsRelationship->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Backward, gadgetId), nullptr);
    m_cache->Cache(info, filter, {widgetClass->GetId(),gadgetClass->GetId()}, bvector<Utf8String>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str()));

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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str()));
    
    // create a node
    TestNavNodePtr node = TestNavNode::Create(&GetDb());
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(GetDb().GetDbGuid());
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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str()));
    
    // create a node
    TestNavNodePtr node = TestNavNode::Create(&GetDb());
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(GetDb().GetDbGuid());
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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str()));
    
    // create a node
    TestNavNodePtr node = TestNavNode::Create(&GetDb());
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(GetDb().GetDbGuid());
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
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    bset<ECClassId> relationshipIds;
    relationshipIds.insert(widgetHasGadget->GetId());
    DataSourceFilter filter(DataSourceFilter::RelatedInstanceInfo(relationshipIds, RequiredRelationDirection_Backward, ECInstanceId((uint64_t)123)), nullptr);
    m_cache->Cache(info, filter, {ECClassId((uint64_t)1)}, bvector<Utf8String>());
    EXPECT_TRUE(m_cache->IsDataSourceCached(info.GetConnectionId(), info.GetRulesetId().c_str()));

    // create a node
    TestNavNodePtr node = TestNavNode::Create(&GetDb());
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(GetDb().GetDbGuid());
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
* @bsitest                                      Aidas.Vaiksnoras                05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, Updates_IsExpandedFlag)
    {
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache root node
    TestNavNodePtr node = TestNavNode::Create(&GetDb());
    NavNodeExtendedData extendedData(*node);
    extendedData.SetConnectionId(info.GetConnectionId());
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
* @bsitest                                      Aidas.Vaiksnoras                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, GetFilteredNodes)
    {
    // create the root node
    TestNavNodePtr node1 = TestNodesHelper::CreateCustomNode("test type", "test label", "test descr");
    NavNodeExtendedData extendedData1(*node1);
    extendedData1.SetConnectionId(GetDb().GetDbGuid());
    extendedData1.SetRulesetId("ruleset_id");

    TestNavNodePtr node2 = TestNodesHelper::CreateCustomNode("test type", "label", "test descr");
    NavNodeExtendedData extendedData2(*node2);
    extendedData2.SetConnectionId(GetDb().GetDbGuid());
    extendedData2.SetRulesetId("ruleset_id");

    // cache root data source
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache nodes
    m_cache->Cache(*node1, false);
    m_cache->Cache(*node2, false);
    
    bvector<NavNodeCPtr> filteredNodes = m_cache->GetFilteredNodes(GetDb(), info.GetRulesetId().c_str(), "test");
    // verify filtered node
    ASSERT_EQ(1, filteredNodes.size());
    EXPECT_TRUE(node1->Equals(*filteredNodes[0]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Aidas.Vaiksnoras                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesCacheTests, ResetIsExpandedFlag)
    {
    // create the root node
    TestNavNodePtr node1 = TestNodesHelper::CreateCustomNode("test type", "test label", "test descr");
    NavNodeExtendedData extendedData1(*node1);
    extendedData1.SetConnectionId(GetDb().GetDbGuid());
    extendedData1.SetRulesetId("ruleset_id");
    node1->SetIsExpanded(true);

    TestNavNodePtr node2 = TestNodesHelper::CreateCustomNode("test type", "label", "test descr");
    NavNodeExtendedData extendedData2(*node2);
    extendedData2.SetConnectionId(GetDb().GetDbGuid());
    extendedData2.SetRulesetId("ruleset_id");
    node2->SetIsExpanded(true);

    // cache root data source
    DataSourceInfo info(GetDb().GetDbGuid(), "ruleset_id", nullptr, nullptr);
    m_cache->Cache(info, DataSourceFilter(), bvector<ECClassId>(), bvector<Utf8String>());

    // cache nodes
    m_cache->Cache(*node1, false);
    m_cache->Cache(*node2, false);

    m_cache->ResetExpandedNodes(GetDb().GetDbGuid(), "ruleset_id");

    EXPECT_FALSE(m_cache->GetNode(node1->GetNodeId())->IsExpanded());
    EXPECT_FALSE(m_cache->GetNode(node2->GetNodeId())->IsExpanded());
    }
