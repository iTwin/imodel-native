/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/QueryBasedNodesProviderTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../../../Source/RulesDriven/RulesEngine/JsonNavNode.h"
#include "../../../Source/RulesDriven/RulesEngine/QueryContracts.h"
#include "QueryBasedNodesProviderTests.h"
#include "TestHelpers.h"

ECDbTestProject* QueryBasedNodesProviderTests::s_project = nullptr;
CustomFunctionsInjector* QueryBasedNodesProviderTests::s_customFunctions = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBasedNodesProviderTests::SetUpTestCase()
    {
    QueryBasedNodesProviderTests::s_project = new ECDbTestProject();
    QueryBasedNodesProviderTests::s_project->Create("QueryBasedNodesProviderTests", "RulesEngineTest.01.00.ecschema.xml");
    QueryBasedNodesProviderTests::s_customFunctions = new CustomFunctionsInjector(QueryBasedNodesProviderTests::s_project->GetECDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBasedNodesProviderTests::TearDownTestCase()
    {
    DELETE_AND_CLEAR(QueryBasedNodesProviderTests::s_project);
    DELETE_AND_CLEAR(QueryBasedNodesProviderTests::s_customFunctions);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryBasedNodesProviderTests::SetUp()
    {
    m_widgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Widget")->GetEntityClassCP();
    m_gadgetClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Gadget")->GetEntityClassCP();
    m_sprocketClass = s_project->GetECDb().Schemas().GetClass("RulesEngineTest", "Sprocket")->GetEntityClassCP();

    m_ruleset = PresentationRuleSet::CreateInstance("QueryBasedNodesProviderTests", 1, 0, false, "", "", "", false);
    RootNodeRuleP rule = new RootNodeRule("", 1, false, RuleTargetTree::TargetTree_MainTree, false);
    m_ruleset->AddPresentationRule(*rule);

    m_context = NavNodesProviderContext::Create(*m_ruleset, true, TargetTree_Both, 0, 
        m_settings, m_expressionsCache, m_relatedPathsCache, m_nodesFactory, m_nodesCache, m_providerFactory, nullptr);
    m_context->SetQueryContext(s_project->GetECDb(), m_statementCache, *s_customFunctions, nullptr);
    m_context->SetRootNodeContext(*rule);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (QueryBasedNodesProviderTests, DoesntCustomizeNodesIfNotNecessary)
    {
    RulesEngineTestHelpers::DeleteInstances(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(m_widgetClass);
    ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()->SelectContract(*contract).From(*m_widgetClass, false);

    RefCountedPtr<QueryBasedNodesProvider> provider = QueryBasedNodesProvider::Create(*m_context, *query);
    
    // checking for nodes doesnt customize nodes
    EXPECT_TRUE(provider->HasNodes());
    for (size_t i = 0; i < 3; i++)
        {
        JsonNavNodePtr node = provider->GetExecutor().GetNode(i);
        EXPECT_FALSE(NavNodeExtendedData(*node).IsCustomized());
        }

    // requesting nodes count doesnt customize nodes
    EXPECT_EQ(3, provider->GetNodesCount());
    for (size_t i = 0; i < 3; i++)
        {
        JsonNavNodePtr node = provider->GetExecutor().GetNode(i);
        EXPECT_FALSE(NavNodeExtendedData(*node).IsCustomized());
        }

    // requesting a node should customize it
    JsonNavNodePtr node;
    provider->GetNode(node, 1);
    EXPECT_TRUE(NavNodeExtendedData(*node).IsCustomized());

    // other nodes should still be not customized
    node = provider->GetExecutor().GetNode(0);
    EXPECT_FALSE(NavNodeExtendedData(*node).IsCustomized());
    node = provider->GetExecutor().GetNode(2);
    EXPECT_FALSE(NavNodeExtendedData(*node).IsCustomized());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(QueryBasedNodesProviderTests, HasNodesDoesntQueryChildrenIfAlwaysReturnsChildrenFlagIsSet)
    {
    RulesEngineTestHelpers::InsertInstance(*s_project, *m_widgetClass);

    NavigationQueryContractPtr contract = ECInstanceNodesQueryContract::Create(m_widgetClass);
    ComplexNavigationQueryPtr query = &ComplexNavigationQuery::Create()->SelectContract(*contract).From(*m_widgetClass, false);
    query->GetResultParametersR().GetNavNodeExtendedDataR().SetAlwaysReturnsChildren(true);

    RefCountedPtr<QueryBasedNodesProvider> provider = QueryBasedNodesProvider::Create(*m_context, *query);
    EXPECT_TRUE(provider->HasNodes());
    EXPECT_FALSE(provider->GetExecutor().IsReadStarted());
    }
