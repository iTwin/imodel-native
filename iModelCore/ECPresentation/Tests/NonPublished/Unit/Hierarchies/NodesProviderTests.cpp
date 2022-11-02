/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "NodesProviderTests.h"

ECDbTestProject* NodesProviderTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesProviderTests::SetUpTestCase()
    {
    s_project = new ECDbTestProject();
    s_project->Create("NodesProviderTests", "RulesEngineTest.01.00.ecschema.xml");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesProviderTests::TearDownTestCase()
    {
    DELETE_AND_CLEAR(s_project);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesProviderTests::SetUp()
    {
    ECPresentationTest::SetUp();
    m_nodesCache = std::make_shared<TestNodesCache>();
    m_providerContextFactory.SetNodesCache(m_nodesCache);
    m_providerContextFactory.SetUsedClassesListener(&m_usedClassesListener);
    m_ruleset = PresentationRuleSet::CreateInstance("NodesProviderTests");
    m_providerContextFactory.SetRuleset(m_ruleset.get());
    m_connection = m_connections.NotifyConnectionOpened(s_project->GetECDb());
    m_context = m_providerContextFactory.Create(*m_connection, m_ruleset->GetRuleSetId().c_str(), nullptr, m_nodesCache);
    m_context->SetProvidersIndexAllocator(*new ProvidersIndexAllocator());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesProviderTests::TearDown()
    {
    s_project->GetECDb().AbandonChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesProviderTests::Cache(NavNodeR node) {RulesEngineTestHelpers::CacheNode(*m_nodesCache, node);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static NavNodesProviderPtr CreateProviderForCheckingNodesEvaluation(bool& didEvaluateNodes, NavNodesProviderContextR context)
    {
    auto provider = TestNodesProvider::Create(context);
    provider->SetHasNodesHandler([&](){didEvaluateNodes = true; return false;});
    return provider;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesProviderTests, HasNodes_ReturnsParentsChildrenFlagIfDetermined)
    {
    NavNodePtr parent = NavNode::Create();
    m_context->SetVirtualParentNode(parent.get());

    bool hasNodesEvaluationCalled = false;

    EXPECT_FALSE(CreateProviderForCheckingNodesEvaluation(hasNodesEvaluationCalled, *m_context)->HasNodes());
    EXPECT_TRUE(hasNodesEvaluationCalled);

    hasNodesEvaluationCalled = false;
    parent->SetHasChildren(true);
    EXPECT_TRUE(CreateProviderForCheckingNodesEvaluation(hasNodesEvaluationCalled, *m_context)->HasNodes());
    EXPECT_FALSE(hasNodesEvaluationCalled);

    hasNodesEvaluationCalled = false;
    parent->SetHasChildren(false);
    EXPECT_FALSE(CreateProviderForCheckingNodesEvaluation(hasNodesEvaluationCalled, *m_context)->HasNodes());
    EXPECT_FALSE(hasNodesEvaluationCalled);

    hasNodesEvaluationCalled = false;
    parent->ResetHasChildren();
    EXPECT_FALSE(CreateProviderForCheckingNodesEvaluation(hasNodesEvaluationCalled, *m_context)->HasNodes());
    EXPECT_TRUE(hasNodesEvaluationCalled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(NodesProviderTests, HasNodes_ReturnsBasedOnParentsChildrenHintIfSet)
    {
    NavNodePtr parent = NavNode::Create();
    NavNodeExtendedData ext(*parent);
    m_context->SetVirtualParentNode(parent.get());

    bool hasNodesEvaluationCalled = false;

    EXPECT_FALSE(CreateProviderForCheckingNodesEvaluation(hasNodesEvaluationCalled, *m_context)->HasNodes());
    EXPECT_TRUE(hasNodesEvaluationCalled);

    hasNodesEvaluationCalled = false;
    ext.SetChildrenHint(ChildrenHint::Always);
    EXPECT_TRUE(CreateProviderForCheckingNodesEvaluation(hasNodesEvaluationCalled, *m_context)->HasNodes());
    EXPECT_FALSE(hasNodesEvaluationCalled);

    hasNodesEvaluationCalled = false;
    ext.SetChildrenHint(ChildrenHint::Never);
    EXPECT_FALSE(CreateProviderForCheckingNodesEvaluation(hasNodesEvaluationCalled, *m_context)->HasNodes());
    EXPECT_FALSE(hasNodesEvaluationCalled);

    hasNodesEvaluationCalled = false;
    ext.SetChildrenHint(ChildrenHint::Unknown);
    EXPECT_FALSE(CreateProviderForCheckingNodesEvaluation(hasNodesEvaluationCalled, *m_context)->HasNodes());
    EXPECT_TRUE(hasNodesEvaluationCalled);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MultiNavNodesProviderTests : NodesProviderTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, GetTotalNodesCount_ReturnsCorrectIsAccurateFlag)
    {
    auto nestedAccurate = TestNodesProvider::Create(*m_context);
    auto nestedInaccurate = TestNodesProvider::Create(*m_context);
    nestedInaccurate->SetGetNodesCountHandler([](){return CountInfo(1, false);});

    auto multi1 = MultiNavNodesProvider::Create(*m_context);
    EXPECT_TRUE(multi1->GetTotalNodesCount().IsAccurate());

    auto multi2 = MultiNavNodesProvider::Create(*m_context);
    multi2->SetProviders({ nestedAccurate });
    EXPECT_TRUE(multi2->GetTotalNodesCount().IsAccurate());

    auto multi3 = MultiNavNodesProvider::Create(*m_context);
    multi3->SetProviders({ nestedInaccurate });
    EXPECT_FALSE(multi3->GetTotalNodesCount().IsAccurate());

    auto multi4 = MultiNavNodesProvider::Create(*m_context);
    multi4->SetProviders({ nestedAccurate, nestedInaccurate });
    EXPECT_FALSE(multi4->GetTotalNodesCount().IsAccurate());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, Iteration_SteppingThroughEmptyNestedProvidersInFront)
    {
    NavNodesProviderPtr provider1 = BVectorNodesProvider::Create(*m_context, bvector<NavNodePtr>());

    bvector<NavNodePtr> nodes2 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "1", "1", "1"),
        TestNodesHelper::CreateCustomNode(*m_connection, "2", "2", "2")
        };
    NavNodesProviderPtr provider2 = BVectorNodesProvider::Create(*m_context, nodes2);

    bvector<NavNodePtr> nodes3 = {
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, Iteration_SteppingThroughEmptyNestedProvidersInTheMiddle)
    {
    bvector<NavNodePtr> nodes1 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "1", "1", "1"),
        TestNodesHelper::CreateCustomNode(*m_connection, "2", "2", "2")
        };
    NavNodesProviderPtr provider1 = BVectorNodesProvider::Create(*m_context, nodes1);

    NavNodesProviderPtr provider2 = BVectorNodesProvider::Create(*m_context, bvector<NavNodePtr>());

    bvector<NavNodePtr> nodes3 = {
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, Iteration_SteppingThroughEmptyNestedProvidersInTheEnd)
    {
    bvector<NavNodePtr> nodes1 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "1", "1", "1"),
        TestNodesHelper::CreateCustomNode(*m_connection, "2", "2", "2")
        };
    NavNodesProviderPtr provider1 = BVectorNodesProvider::Create(*m_context, nodes1);

    bvector<NavNodePtr> nodes2 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "3", "3", "3")
        };
    NavNodesProviderPtr provider2 = BVectorNodesProvider::Create(*m_context, nodes2);

    NavNodesProviderPtr provider3 = BVectorNodesProvider::Create(*m_context, bvector<NavNodePtr>());

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, Iteration_SkippingZeroItems)
    {
    bvector<NavNodePtr> nodes = {
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, Iteration_SkippingItemsWithOneNestedProvider)
    {
    bvector<NavNodePtr> nodes = {
        TestNodesHelper::CreateCustomNode(*m_connection, "1", "1", "1"),
        TestNodesHelper::CreateCustomNode(*m_connection, "2", "2", "2"),
        TestNodesHelper::CreateCustomNode(*m_connection, "3", "3", "3"),
        TestNodesHelper::CreateCustomNode(*m_connection, "4", "4", "4"),
        TestNodesHelper::CreateCustomNode(*m_connection, "5", "5", "5")
        };
    NavNodesProviderPtr provider = BVectorNodesProvider::Create(*m_context, nodes);

    RefCountedPtr<MultiNavNodesProvider> multiProvider = MultiNavNodesProvider::Create(*m_context);
    multiProvider->SetProviders({provider});

    auto iter = multiProvider->begin();
    ASSERT_NE(iter, multiProvider->end());
    EXPECT_STREQ("1", (*iter)->GetType().c_str());

    iter += 2;
    ASSERT_NE(iter, multiProvider->end());
    EXPECT_STREQ("3", (*iter)->GetType().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, Iteration_SkippingItemsTillTheEndOfNestedProvider)
    {
    bvector<NavNodePtr> nodes1 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "1", "1", "1"),
        TestNodesHelper::CreateCustomNode(*m_connection, "2", "2", "2")
        };
    NavNodesProviderPtr provider1 = BVectorNodesProvider::Create(*m_context, nodes1);

    bvector<NavNodePtr> nodes2 = {
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, Iteration_SkippingItemsPastTheEndOfCurrentProvider)
    {
    bvector<NavNodePtr> nodes1 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "1", "1", "1"),
        TestNodesHelper::CreateCustomNode(*m_connection, "2", "2", "2")
        };
    NavNodesProviderPtr provider1 = BVectorNodesProvider::Create(*m_context, nodes1);

    bvector<NavNodePtr> nodes2 = {
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, Iteration_SkippingItemsTillTheEndOfTheLastProvider)
    {
    bvector<NavNodePtr> nodes1 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "1", "1", "1"),
        TestNodesHelper::CreateCustomNode(*m_connection, "2", "2", "2")
        };
    NavNodesProviderPtr provider1 = BVectorNodesProvider::Create(*m_context, nodes1);

    bvector<NavNodePtr> nodes2 = {
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, Iteration_SkippingItemsPastTheEndOfTheLastProvider)
    {
    bvector<NavNodePtr> nodes1 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "1", "1", "1"),
        TestNodesHelper::CreateCustomNode(*m_connection, "2", "2", "2")
        };
    NavNodesProviderPtr provider1 = BVectorNodesProvider::Create(*m_context, nodes1);

    bvector<NavNodePtr> nodes2 = {
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, RelatedRulesetVariables_CollectsRulesetVariablesFromNestedProviders)
    {
    NavNodesProviderContextPtr nestedContext1 = m_providerContextFactory.Create(*m_connection, m_ruleset->GetRuleSetId().c_str(), nullptr, m_nodesCache);
    NavNodesProviderPtr provider1 = BVectorNodesProvider::Create(*nestedContext1, {});
    nestedContext1->GetUsedVariablesListener().OnVariableUsed("nested_var_1");
    RulesetVariables relatedVariables = provider1->GetRelatedRulesetVariables();
    EXPECT_TRUE(relatedVariables.HasValue("nested_var_1"));

    NavNodesProviderContextPtr nestedContext2 = m_providerContextFactory.Create(*m_connection, m_ruleset->GetRuleSetId().c_str(), nullptr, m_nodesCache);
    NavNodesProviderPtr provider2 = BVectorNodesProvider::Create(*nestedContext2, {});
    nestedContext2->GetUsedVariablesListener().OnVariableUsed("nested_var_2");
    relatedVariables = provider2->GetRelatedRulesetVariables();
    EXPECT_TRUE(relatedVariables.HasValue("nested_var_2"));
    EXPECT_FALSE(relatedVariables.HasValue("nested_var_1"));

    RefCountedPtr<MultiNavNodesProvider> multiProvider = MultiNavNodesProvider::Create(*m_context);
    m_context->GetUsedVariablesListener().OnVariableUsed("root_var");
    relatedVariables = multiProvider->GetRelatedRulesetVariables();
    EXPECT_TRUE(relatedVariables.HasValue("root_var"));
    EXPECT_FALSE(relatedVariables.HasValue("nested_var_1"));
    EXPECT_FALSE(relatedVariables.HasValue("nested_var_2"));

    multiProvider->SetProviders({ provider1, provider2 });

    relatedVariables = multiProvider->GetRelatedRulesetVariables();
    EXPECT_TRUE(relatedVariables.HasValue("root_var"));
    EXPECT_TRUE(relatedVariables.HasValue("nested_var_1"));
    EXPECT_TRUE(relatedVariables.HasValue("nested_var_2"));
    }

#define EXPECT_OPTIMIZATION_FLAGS_ENABLED() \
    EXPECT_TRUE(GetContext().GetOptimizationFlags().IsFullNodesLoadDisabled()); \
    EXPECT_TRUE(GetContext().GetOptimizationFlags().IsPostProcessingDisabled());

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct OptimizationFlagsTestingProvider : NavNodesProvider
{
private:
    bvector<NavNodePtr> m_nodes;
private:
    OptimizationFlagsTestingProvider(NavNodesProviderContextR context, NavNodeP node)
        : NavNodesProvider(context)
        {
        if (node)
            m_nodes.push_back(node);
        }
protected:
    Utf8CP _GetName() const override {return "Optimization flags testing provider";}
    bool _HasNodes() const override
        {
        EXPECT_OPTIMIZATION_FLAGS_ENABLED();
        return !m_nodes.empty();
        }
    CountInfo _GetTotalNodesCount() const override
        {
        EXPECT_OPTIMIZATION_FLAGS_ENABLED();
        return CountInfo(m_nodes.size(), true);
        }
    Iterator _CreateFrontIterator() const override
        {
        EXPECT_OPTIMIZATION_FLAGS_ENABLED();
        return Iterator(std::make_unique<IterableIteratorImpl<bvector<NavNodePtr>::const_iterator, NavNodePtr>>(m_nodes.begin()));
        }
    Iterator _CreateBackIterator() const override
        {
        EXPECT_OPTIMIZATION_FLAGS_ENABLED();
        return Iterator(std::make_unique<IterableIteratorImpl<bvector<NavNodePtr>::const_iterator, NavNodePtr>>(m_nodes.end()));
        }
public:
    static RefCountedPtr<OptimizationFlagsTestingProvider> Create(NavNodesProviderContextR context, NavNodeP node)
        {
        auto myContext = NavNodesProviderContext::Create(context);
        myContext->GetOptimizationFlags().SetParentContainer(&context.GetOptimizationFlags());
        return new OptimizationFlagsTestingProvider(*myContext, node);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, Iteration_PassesDownOptimizationFlags)
    {
    RefCountedPtr<MultiNavNodesProvider> multiProvider = MultiNavNodesProvider::Create(*m_context);
    multiProvider->SetProviders(
        {
        OptimizationFlagsTestingProvider::Create(*m_context, nullptr),
        OptimizationFlagsTestingProvider::Create(*m_context, TestNodesHelper::CreateCustomNode(*m_connection, "2", "2", "2").get()),
        OptimizationFlagsTestingProvider::Create(*m_context, nullptr),
        OptimizationFlagsTestingProvider::Create(*m_context, TestNodesHelper::CreateCustomNode(*m_connection, "4", "4", "4").get()),
        OptimizationFlagsTestingProvider::Create(*m_context, TestNodesHelper::CreateCustomNode(*m_connection, "5", "5", "5").get()),
        });

    DisabledFullNodesLoadContext disableFullLoad(*multiProvider);
    DisabledPostProcessingContext disablePostProcessing(*multiProvider);
    //NodesChildrenCheckContext checkingChildren(*multiProvider);

    int nodesCount = 0;
    for (auto node : *multiProvider)
        {
        ++nodesCount;
        // expectations are verified in nested providers
        }
    EXPECT_EQ(3, nodesCount);
    EXPECT_EQ(multiProvider->end(), multiProvider->begin() + 3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, Paging_TakesNodesOnlyFromFirstProvider)
    {
    bvector<NavNodePtr> nodes1 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "1", "1", "1"),
        TestNodesHelper::CreateCustomNode(*m_connection, "2", "2", "2")
        };
    NavNodesProviderPtr provider1 = BVectorNodesProvider::Create(*NavNodesProviderContext::Create(*m_context), nodes1);

    bvector<NavNodePtr> nodes2 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "3", "3", "3"),
        TestNodesHelper::CreateCustomNode(*m_connection, "4", "4", "4")
        };
    NavNodesProviderPtr provider2 = BVectorNodesProvider::Create(*NavNodesProviderContext::Create(*m_context), nodes2);

    RefCountedPtr<MultiNavNodesProvider> multiProvider = MultiNavNodesProvider::Create(*NavNodesProviderContext::Create(*m_context));
    multiProvider->SetProviders({provider1, provider2});

    multiProvider->SetPageOptions(std::make_shared<NavNodesProviderContext::PageOptions>(0, 2));
    EXPECT_EQ(2, multiProvider->GetNodesCount());

    auto iter = multiProvider->begin();
    EXPECT_TRUE((*iter)->Equals(*nodes1[0]));

    ++iter;
    EXPECT_TRUE((*iter)->Equals(*nodes1[1]));

    ++iter;
    EXPECT_EQ(multiProvider->end(), iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, Paging_TakesNodesFromMultipleProviders)
    {
    bvector<NavNodePtr> nodes1 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "1", "1", "1"),
        TestNodesHelper::CreateCustomNode(*m_connection, "2", "2", "2")
        };
    NavNodesProviderPtr provider1 = BVectorNodesProvider::Create(*NavNodesProviderContext::Create(*m_context), nodes1);

    bvector<NavNodePtr> nodes2 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "3", "3", "3"),
        TestNodesHelper::CreateCustomNode(*m_connection, "4", "4", "4")
        };
    NavNodesProviderPtr provider2 = BVectorNodesProvider::Create(*NavNodesProviderContext::Create(*m_context), nodes2);

    RefCountedPtr<MultiNavNodesProvider> multiProvider = MultiNavNodesProvider::Create(*NavNodesProviderContext::Create(*m_context));
    multiProvider->SetProviders({provider1, provider2});

    multiProvider->SetPageOptions(std::make_shared<NavNodesProviderContext::PageOptions>(1, 2));
    EXPECT_EQ(2, multiProvider->GetNodesCount());

    auto iter = multiProvider->begin();
    EXPECT_TRUE((*iter)->Equals(*nodes1[1]));

    ++iter;
    EXPECT_TRUE((*iter)->Equals(*nodes2[0]));

    ++iter;
    EXPECT_EQ(multiProvider->end(), iter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(MultiNavNodesProviderTests, Paging_SkipsFirstProvider)
    {
    bvector<NavNodePtr> nodes1 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "1", "1", "1"),
        TestNodesHelper::CreateCustomNode(*m_connection, "2", "2", "2")
        };
    NavNodesProviderPtr provider1 = BVectorNodesProvider::Create(*NavNodesProviderContext::Create(*m_context), nodes1);

    bvector<NavNodePtr> nodes2 = {
        TestNodesHelper::CreateCustomNode(*m_connection, "3", "3", "3"),
        TestNodesHelper::CreateCustomNode(*m_connection, "4", "4", "4")
        };
    NavNodesProviderPtr provider2 = BVectorNodesProvider::Create(*NavNodesProviderContext::Create(*m_context), nodes2);

    RefCountedPtr<MultiNavNodesProvider> multiProvider = MultiNavNodesProvider::Create(*NavNodesProviderContext::Create(*m_context));
    multiProvider->SetProviders({ provider1, provider2 });

    multiProvider->SetPageOptions(std::make_shared<NavNodesProviderContext::PageOptions>(2, 2));
    EXPECT_EQ(2, multiProvider->GetNodesCount());

    auto iter = multiProvider->begin();
    EXPECT_TRUE((*iter)->Equals(*nodes2[0]));

    ++iter;
    EXPECT_TRUE((*iter)->Equals(*nodes2[1]));

    ++iter;
    EXPECT_EQ(multiProvider->end(), iter);
    }
