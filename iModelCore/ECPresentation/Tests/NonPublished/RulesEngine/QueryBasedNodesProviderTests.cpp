/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../../../Source/RulesDriven/RulesEngine/JsonNavNode.h"
#include "../../../Source/RulesDriven/RulesEngine/QueryContracts.h"
#include "NodesProviderTests.h"

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct QueryBasedNodesProviderTests : NodesProviderTests
    {
    ECEntityClassCP m_widgetClass;
    ECEntityClassCP m_gadgetClass;
    ECEntityClassCP m_sprocketClass;

    void SetUp() override;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBasedNodesProviderTests::SetUp()
    {
    m_widgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget")->GetEntityClassCP();
    m_gadgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Gadget")->GetEntityClassCP();
    m_sprocketClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Sprocket")->GetEntityClassCP();

    NodesProviderTests::SetUp();
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedNodesProviderTests, AbortsInitializationWhenCanceled)
    {
    HierarchyLevelIdentifier cachedHierarchyLevel;
    DataSourceIdentifier cachedDataSource;
    int nodesCached = 0;
    m_nodesCache.SetCacheHierarchyLevelHandler([&](HierarchyLevelIdentifier& hl)
        {
        cachedHierarchyLevel = hl;
        });
    m_nodesCache.SetCacheDataSourceHandler([&](DataSourceInfo& ds)
        {
        cachedDataSource = ds.GetIdentifier();
        });
    m_nodesCache.SetCacheNodeHandler([&](JsonNavNodeR, NodeVisibility)
        {
        nodesCached++;
        });

    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(m_widgetClass);
    ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()->SelectContract(*contract).From(*m_widgetClass, false);
    query->GetResultParametersR().GetNavNodeExtendedDataR().SetSpecificationHash("");
    RefCountedPtr<QueryBasedNodesProvider> provider = QueryBasedNodesProvider::Create(*m_context, *query);

    ICancelationTokenPtr cancelationToken = new TestCancelationToken([&nodesCached]()
        {
        // cancel when at least one node is created
        return nodesCached > 0;
        });
    m_context->SetCancelationToken(cancelationToken.get());
    RulesetVariables variables;

    // verify the data source is still invalid
    ASSERT_TRUE(cachedHierarchyLevel.IsValid());
    ASSERT_TRUE(cachedDataSource.IsValid());
    EXPECT_FALSE(m_nodesCache.IsInitialized(cachedDataSource, variables));
    EXPECT_EQ(2, provider->GetNodesCount());

    // force initialization
    JsonNavNodePtr node;
    provider->GetNode(node, 0);

    // verify the initialization was aborted after creating the first node
    EXPECT_EQ(1, nodesCached);

    // verify the nodes cache is empty
    EXPECT_TRUE(m_nodesCache.GetHierarchyLevel(*m_providerContextFactory.Create(*m_connection, cachedHierarchyLevel.GetRulesetId().c_str(), cachedHierarchyLevel.GetLocale().c_str(),
        cachedHierarchyLevel.GetPhysicalParentNodeId(), nullptr, -1, variables), cachedHierarchyLevel).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedNodesProviderTests, HasNodesDoesntQueryChildrenIfAlwaysReturnsChildrenFlagIsSet)
    {
    RulesEngineTestHelpers::InsertInstance(s_project->GetECDb(), *m_widgetClass);

    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(m_widgetClass);
    ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()->SelectContract(*contract).From(*m_widgetClass, false);
    query->GetResultParametersR().GetNavNodeExtendedDataR().SetSpecificationHash("");
    query->GetResultParametersR().GetNavNodeExtendedDataR().SetChildrenHint(ChildrenHint::Always);

    RefCountedPtr<QueryBasedNodesProvider> provider = QueryBasedNodesProvider::Create(*m_context, *query);
    EXPECT_TRUE(provider->HasNodes());
    EXPECT_EQ(0, m_nodesCache.GetCachedChildrenCount(0));
    }
