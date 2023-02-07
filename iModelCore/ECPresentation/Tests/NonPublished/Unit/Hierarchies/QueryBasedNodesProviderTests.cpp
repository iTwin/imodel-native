/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../../../../Source/Hierarchies/NavigationQueryContracts.h"
#include "NodesProviderTests.h"

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct QueryBasedNodesProviderTests : NodesProviderTests
    {
    ECEntityClassCP m_widgetClass;
    ECEntityClassCP m_gadgetClass;
    ECEntityClassCP m_sprocketClass;

    void SetUp() override;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBasedNodesProviderTests::SetUp()
    {
    m_widgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget")->GetEntityClassCP();
    m_gadgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Gadget")->GetEntityClassCP();
    m_sprocketClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Sprocket")->GetEntityClassCP();

    NodesProviderTests::SetUp();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedNodesProviderTests, AbortsInitializationWhenCanceled)
    {
    HierarchyLevelIdentifier cachedHierarchyLevel;
    DataSourceIdentifier cachedDataSource;
    int nodesCached = 0;
    m_nodesCache->SetCacheHierarchyLevelHandler([&](HierarchyLevelIdentifier& hl)
        {
        cachedHierarchyLevel = hl;
        });
    m_nodesCache->SetCacheDataSourceHandler([&](DataSourceInfo& ds)
        {
        cachedDataSource = ds.GetIdentifier();
        });
    m_nodesCache->SetCacheNodeHandler([&](NavNodeR, NodeVisibility)
        {
        nodesCached++;
        });

    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    SelectClass<ECClass> selectClass(*m_widgetClass, "this", false);
    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", m_widgetClass, CreateDisplayLabelField(selectClass));
    ComplexQueryBuilderPtr query = &ComplexQueryBuilder::Create()->SelectContract(*contract).From(selectClass);
    RefCountedPtr<QueryBasedNodesProvider> provider = QueryBasedNodesProvider::Create(*m_context, *query);

    ICancelationTokenPtr cancelationToken = new TestCancelationToken([&nodesCached]()
        {
        // cancel when at least one node is created
        return nodesCached > 0;
        });
    m_context->SetCancelationToken(cancelationToken.get());

    // verify the data source is still invalid
    ASSERT_TRUE(cachedHierarchyLevel.IsValid());
    ASSERT_TRUE(cachedDataSource.IsValid());
    EXPECT_FALSE(m_nodesCache->IsDataSourceInitialized(cachedDataSource.GetId()));
    EXPECT_EQ(2, provider->GetNodesCount());

    bool wasCancelled = false;
    try
        {
        // force initialization
        NavNodeCPtr node = *provider->begin();
        }
    catch (CancellationException const&)
        {
        wasCancelled = true;
        }

    // verify the initialization was aborted after creating the first node
    EXPECT_TRUE(wasCancelled);
    EXPECT_EQ(1, nodesCached);

    // verify the nodes cache is empty
    NavNodesProviderContextPtr context = m_providerContextFactory.Create(*m_connection, cachedHierarchyLevel.GetRulesetId().c_str(),
        nullptr, m_nodesCache, nullptr, RulesetVariables());
    EXPECT_TRUE(m_nodesCache->GetHierarchyLevel(*context, cachedHierarchyLevel.GetId()).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedNodesProviderTests, HasNodesDoesntQueryChildrenIfAlwaysReturnsChildrenFlagIsSet)
    {
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    SelectClass<ECClass> selectClass(*m_widgetClass, "this", false);
    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", m_widgetClass, CreateDisplayLabelField(selectClass));
    ComplexQueryBuilderPtr query = &ComplexQueryBuilder::Create()->SelectContract(*contract).From(selectClass);
    query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetChildrenHint(ChildrenHint::Always);

    RefCountedPtr<QueryBasedNodesProvider> provider = QueryBasedNodesProvider::Create(*m_context, *query);
    EXPECT_TRUE(provider->HasNodes());
    EXPECT_EQ(0, m_nodesCache->GetCachedChildrenCount(BeGuid()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedNodesProviderTests, DoesntFlagProviderAsInitializedInCacheIfProviderIsInitializedPartially)
    {
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    SelectClass<ECClass> selectClass(*m_widgetClass, "this", false);
    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create("", m_widgetClass, CreateDisplayLabelField(selectClass));
    ComplexQueryBuilderPtr query = &ComplexQueryBuilder::Create()->SelectContract(*contract).From(selectClass);
    query->GetNavigationResultParameters().GetNavNodeExtendedDataR().SetChildrenHint(ChildrenHint::Always);

    RefCountedPtr<QueryBasedNodesProvider> provider = QueryBasedNodesProvider::Create(*m_context, *query);
    provider->SetPageOptions(std::make_shared<NavNodesProviderContext::PageOptions>(0, 1));
    NavNodeCPtr node = *provider->begin();

    EXPECT_FALSE(m_nodesCache->IsDataSourceInitialized(provider->GetIdentifier().GetId()));
    }
