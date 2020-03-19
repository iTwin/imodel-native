/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "NodesProviderTests.h"

static JsonNavNodesFactory s_nodesFactory;
ECDbTestProject* NodesProviderTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesProviderTests::SetUpTestCase()
    {
    Localization::Init();
    s_project = new ECDbTestProject();
    s_project->Create("NodesProviderTests", "RulesEngineTest.01.00.ecschema.xml");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesProviderTests::TearDownTestCase()
    {
    DELETE_AND_CLEAR(s_project);
    Localization::Terminate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesProviderTests::SetUp()
    {
    ECPresentationTest::SetUp();
    m_providerContextFactory.SetNodesCache(&m_nodesCache);
    m_providerContextFactory.SetUsedClassesListener(&m_usedClassesListener);
    m_ruleset = PresentationRuleSet::CreateInstance("QueryBasedSpecificationNodesProviderTests", 1, 0, false, "", "", "", false);
    m_providerContextFactory.SetRuleset(m_ruleset.get());
    m_connection = m_connections.NotifyConnectionOpened(s_project->GetECDb());
    m_context = m_providerContextFactory.Create(*m_connection, m_ruleset->GetRuleSetId().c_str(), "", nullptr);
    m_context->SetProvidersIndexAllocator(*new ProvidersIndexAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesProviderTests::TearDown()
    {
    s_project->GetECDb().AbandonChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesProviderTests::Cache(JsonNavNodeR node)
    {
    NavNodeExtendedData extendedData(node);
    uint64_t virtualParentId = extendedData.HasVirtualParentId() ? extendedData.GetVirtualParentId() : 0;
    HierarchyLevelInfo hlInfo = m_nodesCache.FindHierarchyLevel(extendedData.GetConnectionId(), 
        extendedData.GetRulesetId(), extendedData.GetLocale(), extendedData.HasVirtualParentId() ? &virtualParentId : nullptr);
    if (!hlInfo.IsValid())
        {
        hlInfo = HierarchyLevelInfo(extendedData.GetConnectionId(), extendedData.GetRulesetId(), 
            extendedData.GetLocale(), node.GetParentNodeId(), virtualParentId);
        m_nodesCache.Cache(hlInfo);
        }
    DataSourceInfo dsInfo = m_nodesCache.FindDataSource(hlInfo.GetId(), { 0 });
    if (!dsInfo.IsValid())
        {
        dsInfo = DataSourceInfo(hlInfo.GetId(), { 0 });
        m_nodesCache.Cache(dsInfo, DataSourceFilter(), bmap<ECClassId, bool>(), bvector<UserSettingEntry>());
        }
    m_nodesCache.Cache(node, dsInfo, 0, NodeVisibility::Visible);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesProviderTests, AbortsFinalizingNodesWhenCanceled)
    {
    bool isCanceled = false;
    ICancelationTokenPtr cancelationToken = new TestCancelationToken([&](){return isCanceled;});
    m_context->SetCancelationToken(cancelationToken.get());

    int nodesRequested = 0;
    RefCountedPtr<TestNodesProvider> provider = TestNodesProvider::Create(*m_context);
    provider->SetHasNodesHandler([](){return true;});
    provider->SetGetNodesCountHandler([](){return 3;});
    provider->SetGetNodeHandler([&](JsonNavNodePtr& node, size_t index)
        {
        nodesRequested++;

        // cancel as soon as the first node is retrieved
        if (index > 0)
            isCanceled = true;

        node = TestNodesHelper::CreateCustomNode(*m_connection, "T", "L", "D");
        RulesEngineTestHelpers::CacheNode(m_nodesCache, *node);
        return true;
        });
    provider->FinalizeNodes();

    // verify finalization was aborted as soon as it was canceled (the third node wasn't even requested)
    EXPECT_EQ(2, nodesRequested);
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2020
+===============+===============+===============+===============+===============+======*/
struct MultiNavNodesProviderTests : NodesProviderTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, Iteration_SteppingThroughEmptyNestedProvidersInFront)
    {
    NavNodesProviderPtr provider1 = BVectorNodesProvider::Create(*m_context, bvector<JsonNavNodePtr>());

    bvector<JsonNavNodePtr> nodes2 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "1", "1", "1"),
        TestNodesHelper::CreateCustomNode(*m_connection, "2", "2", "2")
        };
    NavNodesProviderPtr provider2 = BVectorNodesProvider::Create(*m_context, nodes2);

    bvector<JsonNavNodePtr> nodes3 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "3", "3", "3")
        };
    NavNodesProviderPtr provider3 = BVectorNodesProvider::Create(*m_context, nodes3);

    RefCountedPtr<MultiNavNodesProvider> multiProvider = MultiNavNodesProvider::Create(*m_context);
    multiProvider->SetProviders({provider1, provider2, provider3});

    auto iter = multiProvider->begin();
    ASSERT_NE(iter, multiProvider->end());
    EXPECT_STREQ("1", (*iter)->GetType().c_str());

    ++iter;
    ASSERT_NE(iter, multiProvider->end());
    EXPECT_STREQ("2", (*iter)->GetType().c_str());

    ++iter;
    ASSERT_NE(iter, multiProvider->end());
    EXPECT_STREQ("3", (*iter)->GetType().c_str());

    ++iter;
    ASSERT_EQ(iter, multiProvider->end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, Iteration_SteppingThroughEmptyNestedProvidersInTheMiddle)
    {
    bvector<JsonNavNodePtr> nodes1 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "1", "1", "1"),
        TestNodesHelper::CreateCustomNode(*m_connection, "2", "2", "2")
        };
    NavNodesProviderPtr provider1 = BVectorNodesProvider::Create(*m_context, nodes1);

    NavNodesProviderPtr provider2 = BVectorNodesProvider::Create(*m_context, bvector<JsonNavNodePtr>());

    bvector<JsonNavNodePtr> nodes3 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "3", "3", "3")
        };
    NavNodesProviderPtr provider3 = BVectorNodesProvider::Create(*m_context, nodes3);

    RefCountedPtr<MultiNavNodesProvider> multiProvider = MultiNavNodesProvider::Create(*m_context);
    multiProvider->SetProviders({ provider1, provider2, provider3 });

    auto iter = multiProvider->begin();
    ASSERT_NE(iter, multiProvider->end());
    EXPECT_STREQ("1", (*iter)->GetType().c_str());

    ++iter;
    ASSERT_NE(iter, multiProvider->end());
    EXPECT_STREQ("2", (*iter)->GetType().c_str());

    ++iter;
    ASSERT_NE(iter, multiProvider->end());
    EXPECT_STREQ("3", (*iter)->GetType().c_str());

    ++iter;
    ASSERT_EQ(iter, multiProvider->end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, Iteration_SteppingThroughEmptyNestedProvidersInTheEnd)
    {
    bvector<JsonNavNodePtr> nodes1 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "1", "1", "1"),
        TestNodesHelper::CreateCustomNode(*m_connection, "2", "2", "2")
        };
    NavNodesProviderPtr provider1 = BVectorNodesProvider::Create(*m_context, nodes1);
    
    bvector<JsonNavNodePtr> nodes2 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "3", "3", "3")
        };
    NavNodesProviderPtr provider2 = BVectorNodesProvider::Create(*m_context, nodes2);

    NavNodesProviderPtr provider3 = BVectorNodesProvider::Create(*m_context, bvector<JsonNavNodePtr>());

    RefCountedPtr<MultiNavNodesProvider> multiProvider = MultiNavNodesProvider::Create(*m_context);
    multiProvider->SetProviders({provider1, provider2, provider3});

    auto iter = multiProvider->begin();
    ASSERT_NE(iter, multiProvider->end());
    EXPECT_STREQ("1", (*iter)->GetType().c_str());

    ++iter;
    ASSERT_NE(iter, multiProvider->end());
    EXPECT_STREQ("2", (*iter)->GetType().c_str());

    ++iter;
    ASSERT_NE(iter, multiProvider->end());
    EXPECT_STREQ("3", (*iter)->GetType().c_str());

    ++iter;
    ASSERT_EQ(iter, multiProvider->end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, Iteration_SkippingZeroItems)
    {
    bvector<JsonNavNodePtr> nodes = {
        TestNodesHelper::CreateCustomNode(*m_connection, "1", "1", "1"),
        TestNodesHelper::CreateCustomNode(*m_connection, "2", "2", "2")
        };
    NavNodesProviderPtr provider = BVectorNodesProvider::Create(*m_context, nodes);

    RefCountedPtr<MultiNavNodesProvider> multiProvider = MultiNavNodesProvider::Create(*m_context);
    multiProvider->SetProviders({ provider });

    auto iter = multiProvider->begin();
    ASSERT_NE(iter, multiProvider->end());
    EXPECT_STREQ("1", (*iter)->GetType().c_str());

    iter += 0;
    ASSERT_NE(iter, multiProvider->end());
    EXPECT_STREQ("1", (*iter)->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, Iteration_SkippingItemsWithOneNestedProvider)
    {
    bvector<JsonNavNodePtr> nodes = {
        TestNodesHelper::CreateCustomNode(*m_connection, "1", "1", "1"),
        TestNodesHelper::CreateCustomNode(*m_connection, "2", "2", "2"),
        TestNodesHelper::CreateCustomNode(*m_connection, "3", "3", "3"),
        TestNodesHelper::CreateCustomNode(*m_connection, "4", "4", "4"),
        TestNodesHelper::CreateCustomNode(*m_connection, "5", "5", "5")
        };
    NavNodesProviderPtr provider = BVectorNodesProvider::Create(*m_context, nodes);

    RefCountedPtr<MultiNavNodesProvider> multiProvider = MultiNavNodesProvider::Create(*m_context);
    multiProvider->SetProviders({ provider });

    auto iter = multiProvider->begin();
    ASSERT_NE(iter, multiProvider->end());
    EXPECT_STREQ("1", (*iter)->GetType().c_str());

    iter += 2;
    ASSERT_NE(iter, multiProvider->end());
    EXPECT_STREQ("3", (*iter)->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, Iteration_SkippingItemsTillTheEndOfNestedProvider)
    {
    bvector<JsonNavNodePtr> nodes1 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "1", "1", "1"),
        TestNodesHelper::CreateCustomNode(*m_connection, "2", "2", "2")
        };
    NavNodesProviderPtr provider1 = BVectorNodesProvider::Create(*m_context, nodes1);

    bvector<JsonNavNodePtr> nodes2 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "3", "3", "3"),
        TestNodesHelper::CreateCustomNode(*m_connection, "4", "4", "4")
        };
    NavNodesProviderPtr provider2 = BVectorNodesProvider::Create(*m_context, nodes2);

    RefCountedPtr<MultiNavNodesProvider> multiProvider = MultiNavNodesProvider::Create(*m_context);
    multiProvider->SetProviders({ provider1, provider2 });

    auto iter = multiProvider->begin();
    ASSERT_NE(iter, multiProvider->end());
    EXPECT_STREQ("1", (*iter)->GetType().c_str());

    iter += 1;
    ASSERT_NE(iter, multiProvider->end());
    EXPECT_STREQ("2", (*iter)->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, Iteration_SkippingItemsPastTheEndOfCurrentProvider)
    {
    bvector<JsonNavNodePtr> nodes1 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "1", "1", "1"),
        TestNodesHelper::CreateCustomNode(*m_connection, "2", "2", "2")
        };
    NavNodesProviderPtr provider1 = BVectorNodesProvider::Create(*m_context, nodes1);

    bvector<JsonNavNodePtr> nodes2 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "3", "3", "3"),
        TestNodesHelper::CreateCustomNode(*m_connection, "4", "4", "4")
        };
    NavNodesProviderPtr provider2 = BVectorNodesProvider::Create(*m_context, nodes2);

    RefCountedPtr<MultiNavNodesProvider> multiProvider = MultiNavNodesProvider::Create(*m_context);
    multiProvider->SetProviders({ provider1, provider2 });

    auto iter = multiProvider->begin();
    ASSERT_NE(iter, multiProvider->end());
    EXPECT_STREQ("1", (*iter)->GetType().c_str());

    iter += 2;
    ASSERT_NE(iter, multiProvider->end());
    EXPECT_STREQ("3", (*iter)->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, Iteration_SkippingItemsTillTheEndOfTheLastProvider)
    {
    bvector<JsonNavNodePtr> nodes1 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "1", "1", "1"),
        TestNodesHelper::CreateCustomNode(*m_connection, "2", "2", "2")
        };
    NavNodesProviderPtr provider1 = BVectorNodesProvider::Create(*m_context, nodes1);

    bvector<JsonNavNodePtr> nodes2 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "3", "3", "3"),
        TestNodesHelper::CreateCustomNode(*m_connection, "4", "4", "4")
        };
    NavNodesProviderPtr provider2 = BVectorNodesProvider::Create(*m_context, nodes2);

    RefCountedPtr<MultiNavNodesProvider> multiProvider = MultiNavNodesProvider::Create(*m_context);
    multiProvider->SetProviders({ provider1, provider2 });

    auto iter = multiProvider->begin();
    ASSERT_NE(iter, multiProvider->end());
    EXPECT_STREQ("1", (*iter)->GetType().c_str());

    iter += 3;
    ASSERT_NE(iter, multiProvider->end());
    EXPECT_STREQ("4", (*iter)->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, Iteration_SkippingItemsPastTheEndOfTheLastProvider)
    {
    bvector<JsonNavNodePtr> nodes1 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "1", "1", "1"),
        TestNodesHelper::CreateCustomNode(*m_connection, "2", "2", "2")
        };
    NavNodesProviderPtr provider1 = BVectorNodesProvider::Create(*m_context, nodes1);

    bvector<JsonNavNodePtr> nodes2 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "3", "3", "3"),
        TestNodesHelper::CreateCustomNode(*m_connection, "4", "4", "4")
        };
    NavNodesProviderPtr provider2 = BVectorNodesProvider::Create(*m_context, nodes2);

    RefCountedPtr<MultiNavNodesProvider> multiProvider = MultiNavNodesProvider::Create(*m_context);
    multiProvider->SetProviders({ provider1, provider2 });

    auto iter = multiProvider->begin();
    ASSERT_NE(iter, multiProvider->end());
    EXPECT_STREQ("1", (*iter)->GetType().c_str());

    iter += 4;
    ASSERT_EQ(iter, multiProvider->end());
    }