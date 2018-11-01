/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/NodesProviderTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NodesProviderTests::TearDown()
    {
    s_project->GetECDb().AbandonChanges();
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
        return true;
        });
    provider->FinalizeNodes();

    // verify finalization was aborted as soon as it was canceled (the third node wasn't even requested)
    EXPECT_EQ(2, nodesRequested);
    }