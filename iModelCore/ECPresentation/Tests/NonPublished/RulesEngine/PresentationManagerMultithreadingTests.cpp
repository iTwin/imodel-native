/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/PresentationManagerMultithreadingTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TestHelpers.h"
#include "TestRulesDrivenPresentationManagerImpl.h"
#include <UnitTests/BackDoor/ECPresentation/TestRulesetLocater.h>
#include <UnitTests/BackDoor/ECPresentation/TestSelectionProvider.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerMultithreadingTests : ::testing::Test
    {
    static Utf8CP s_projectName;
    static ECDbTestProject* s_project;
    TestConnectionManager m_connections;
    RulesDrivenECPresentationManager* m_manager;
    
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;

    void Sync()
        {
        folly::via(&m_manager->GetExecutor(), [](){}).wait();
        }
    };
Utf8CP RulesDrivenECPresentationManagerMultithreadingTests::s_projectName = "RulesDrivenECPresentationManagerMultithreadingTests";
ECDbTestProject* RulesDrivenECPresentationManagerMultithreadingTests::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerMultithreadingTests::SetUpTestCase()
    {
    s_project = new ECDbTestProject();
    s_project->Create(s_projectName);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerMultithreadingTests::TearDownTestCase()
    {
    DELETE_AND_CLEAR(s_project);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerMultithreadingTests::SetUp()
    {
    m_manager = new RulesDrivenECPresentationManager(m_connections, RulesEngineTestHelpers::GetPaths(BeTest::GetHost()), true);
    m_connections.NotifyConnectionOpened(s_project->GetECDb());
    Sync();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerMultithreadingTests::TearDown()
    {
    DELETE_AND_CLEAR(m_manager);
    }

#ifdef RULES_ENGINE_FORCE_SINGLE_THREAD
    #define VERIFY_THREAD_NE(threadId)  (void)threadId;
#else
    #define VERIFY_THREAD_NE(threadId)  EXPECT_NE(threadId, BeThreadUtilities::GetCurrentThreadId())
#endif

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerMultithreadingTests, RulesetLocaterCallsItsCallbacksOnECPresentationThread)
    {
    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    m_manager->GetLocaters().RegisterLocater(*locater);

    TestRulesetCallbacksHandler callbacksHandler;
    m_manager->GetLocaters().SetRulesetCallbacksHandler(&callbacksHandler);

    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();

    BeAtomic<bool> didGetCreatedCallback(false);
    callbacksHandler.SetCreatedHandler([&](PresentationRuleSetCR ruleset)
        {
        didGetCreatedCallback.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        });
    locater->AddRuleSet(*PresentationRuleSet::CreateInstance("test", 1, 0, false, "", "", "", false));
    Sync();
    EXPECT_TRUE(didGetCreatedCallback.load());

    BeAtomic<bool> didGetDisposedCallback(false);
    callbacksHandler.SetDisposedHandler([&](PresentationRuleSetCR ruleset)
        {
        didGetDisposedCallback.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        });   
    locater->Clear();
    Sync();
    EXPECT_TRUE(didGetDisposedCallback.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerMultithreadingTests, UserSettingsManagerCallsItsCallbacksOnECPresentationThread)
    {
    StubLocalState localState;
    m_manager->GetUserSettings().SetLocalState(&localState);
    TestUserSettingsChangeListener listener;
    m_manager->GetUserSettings().SetChangesListener(&listener);

    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();

    BeAtomic<bool> didGetCallback(false);
    listener.SetCallback([&](Utf8CP, Utf8CP)
        {
        didGetCallback.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        });
    m_manager->GetUserSettings("rulesetid").SetSettingIntValue("settingid", 111);
    Sync();
    EXPECT_TRUE(didGetCallback.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerMultithreadingTests, SelectionManagerCallbacksAreCalledOnECPresentationThread)
    {
    SelectionManager selectionManager(m_connections);
    m_manager->SetSelectionManager(&selectionManager);
    
    TestSelectionChangesListener listener;
    m_manager->GetSelectionManager()->AddListener(listener);

    BeAtomic<bool> didGetCallback(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    listener.SetCallback([&](SelectionChangedEventCR)
        {
        didGetCallback.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        });
    selectionManager.AddToSelection(s_project->GetECDb(), "", false, *NavNodeKeyListContainer::Create({DisplayLabelGroupingNodeKey::Create(1, "label")}));
    Sync();
    EXPECT_TRUE(didGetCallback.load());
    m_manager->SetSelectionManager(nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerMultithreadingTests, ECInstanceChangeEventSourceCallbacksAreCalledOnECPresentationThread)
    {
    RefCountedPtr<TestECInstanceChangeEventsSource> eventsSource = new TestECInstanceChangeEventsSource();
    m_manager->RegisterECInstanceChangeEventSource(*eventsSource);
    
    TestECInstanceChangeEventsHandler handler;
    m_manager->GetImpl().GetECInstanceChangeEventSources().back()->RegisterEventHandler(handler);

    BeAtomic<bool> didGetCallback(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    handler.SetCallback([&](ECDbCR, bvector<ECInstanceChangeEventSource::ChangedECInstance>)
        {
        didGetCallback.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        });
    IECInstancePtr anyInstance = s_project->GetECDb().Schemas().GetClass(ECClassId((uint64_t)1))->GetDefaultStandaloneEnabler()->CreateInstance();
    eventsSource->NotifyECInstanceDeleted(s_project->GetECDb(), *anyInstance);
    Sync();
    EXPECT_TRUE(didGetCallback.load());
    m_manager->UnregisterECInstanceChangeEventSource(*eventsSource);
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerCustomImplMultithreadingTests : RulesDrivenECPresentationManagerMultithreadingTests
    {
    TestRulesDrivenECPresentationManagerImpl* m_impl;
    virtual void SetUp() override;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerCustomImplMultithreadingTests::SetUp()
    {
    RulesDrivenECPresentationManagerMultithreadingTests::SetUp();
    m_impl = new TestRulesDrivenECPresentationManagerImpl(*m_manager->CreateDependenciesFactory(), m_connections, RulesEngineTestHelpers::GetPaths(BeTest::GetHost()));
    m_manager->SetImpl(*m_impl);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsRootNodesCountRequestOnECPresentationThread)
    {
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetRootNodesCountHandler([&](IConnectionCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        return 0;
        });

    RulesDrivenECPresentationManager::NavigationOptions options("doesnt matter", TargetTree_Both);
    m_manager->GetRootNodesCount(s_project->GetECDb(), options.GetJson()).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsRootNodesRequestOnECPresentationThread)
    {
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetRootNodesHandler([&](IConnectionCR, PageOptionsCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        return nullptr;
        });

    // request and verify
    RulesDrivenECPresentationManager::NavigationOptions options("doesnt matter", TargetTree_Both);
    m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsChildNodesCountRequestOnECPresentationThread)
    {
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetChildNodesCountHandler([&](IConnectionCR, NavNodeCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        return 0;
        });

    // request and verify
    TestNavNodePtr parentNode = TestNavNode::Create();
    RulesDrivenECPresentationManager::NavigationOptions options("doesnt matter", TargetTree_Both);
    m_manager->GetChildrenCount(s_project->GetECDb(), *parentNode, options.GetJson()).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsChildNodesRequestOnECPresentationThread)
    {
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetChildNodesHandler([&](IConnectionCR, NavNodeCR, PageOptionsCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        return nullptr;
        });

    // request and verify
    TestNavNodePtr parentNode = TestNavNode::Create();
    RulesDrivenECPresentationManager::NavigationOptions options("doesnt matter", TargetTree_Both);
    m_manager->GetChildren(s_project->GetECDb(), *parentNode, PageOptions(), options.GetJson()).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsHasChildRequestOnECPresentationThread)
    {
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetHasChildHandler([&](IConnectionCR, NavNodeCR, NavNodeKeyCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        return false;
        });

    // request and verify
    TestNavNodePtr parentNode = TestNavNode::Create();
    RulesDrivenECPresentationManager::NavigationOptions options("doesnt matter", TargetTree_Both);
    m_manager->HasChild(s_project->GetECDb(), *parentNode, *DisplayLabelGroupingNodeKey::Create(1, "test"), options.GetJson()).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsNodeRequestOnECPresentationThread)
    {
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetGetNodeHandler([&](IConnectionCR, uint64_t, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        return nullptr;
        });

    // request and verify
    m_manager->GetNode(s_project->GetECDb(), 123).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsParentNodeRequestOnECPresentationThread)
    {
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetGetParentHandler([&](IConnectionCR, NavNodeCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        return nullptr;
        });

    // request and verify
    TestNavNodePtr childNode = TestNavNode::Create();
    RulesDrivenECPresentationManager::NavigationOptions options("doesnt matter", TargetTree_Both);
    m_manager->GetParent(s_project->GetECDb(), *childNode, options.GetJson()).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsContentClassesRequestOnECPresentationThread)
    {
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetContentClassesHandler([&](IConnectionCR, Utf8CP, bvector<ECClassCP> const&, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        return bvector<SelectClassInfo>();
        });

    // request and verify
    RulesDrivenECPresentationManager::ContentOptions options("doesnt matter");
    m_manager->GetContentClasses(s_project->GetECDb(), nullptr, bvector<ECClassCP>(), options.GetJson()).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsContentDescriptorRequestOnECPresentationThread)
    {
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetContentDescriptorHandler([&](IConnectionCR, Utf8CP, SelectionInfo const&, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        return nullptr;
        });

    // request and verify
    SelectionInfo selection("selection provider", false, *NavNodeKeyListContainer::Create());
    RulesDrivenECPresentationManager::ContentOptions options("doesnt matter");
    m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsContentRequestOnECPresentationThread)
    {
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetContentHandler([&](IConnectionCR, ContentDescriptorCR, SelectionInfo const&, PageOptionsCR, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        return nullptr;
        });

    // request and verify
    SelectionInfo selection("selection provider", false, *NavNodeKeyListContainer::Create());
    RulesDrivenECPresentationManager::ContentOptions options("doesnt matter");
    ContentDescriptorCPtr descriptor = ContentDescriptor::Create();
    m_manager->GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsContentSetSizeRequestOnECPresentationThread)
    {        
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetContentSetSizeHandler([&](IConnectionCR, ContentDescriptorCR, SelectionInfo const&, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        return 0;
        });

    // request and verify
    SelectionInfo selection("selection provider", false, *NavNodeKeyListContainer::Create());
    RulesDrivenECPresentationManager::ContentOptions options("doesnt matter");
    ContentDescriptorCPtr descriptor = ContentDescriptor::Create();
    m_manager->GetContentSetSize(s_project->GetECDb(), *descriptor, selection, options.GetJson()).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsNodeCheckedCallbackOnECPresentationThread)
    {    
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetNodeCheckedHandler([&](IConnectionCR, uint64_t, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        });

    // request and verify
    m_manager->NotifyNodeChecked(s_project->GetECDb(), 1).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsNodeUncheckedCallbackOnECPresentationThread)
    {    
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetNodeUncheckedHandler([&](IConnectionCR, uint64_t, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        });

    // request and verify
    m_manager->NotifyNodeUnchecked(s_project->GetECDb(), 1).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsNodeExpandedCallbackOnECPresentationThread)
    {    
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetNodeExpandedHandler([&](IConnectionCR, uint64_t, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        });

    // request and verify
    m_manager->NotifyNodeExpanded(s_project->GetECDb(), 1).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsNodeCollapsedCallbackOnECPresentationThread)
    {    
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetNodeCollapsedHandler([&](IConnectionCR, uint64_t, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        });

    // request and verify
    m_manager->NotifyNodeCollapsed(s_project->GetECDb(), 1).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsAllNodesCollapsedCallbackOnECPresentationThread)
    {    
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetAllNodesCollapsedHandler([&](IConnectionCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        });

    // request and verify
    m_manager->NotifyAllNodesCollapsed(s_project->GetECDb(), 1).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsSaveValueChangeOnMainThread)
    {
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetSaveValueChangeHandler([&](IConnectionCR, bvector<ChangedECInstanceInfo> const&, Utf8CP, ECValueCR)
        {
        wasCalled.store(true);
        EXPECT_EQ(mainThreadId, BeThreadUtilities::GetCurrentThreadId());
        return bvector<ECInstanceChangeResult>();
        });
    m_manager->SaveValueChange(s_project->GetECDb(), bvector<ChangedECInstanceInfo>(), "", ECValue(), Json::Value()).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsCategoriesChangedCallbackOnECPresentationThread)
    {
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetCategoriesChangedHandler([&]()
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        });
    m_manager->NotifyCategoriesChanged();
    Sync();
    EXPECT_TRUE(wasCalled.load());
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerRequestCancelationTests : RulesDrivenECPresentationManagerCustomImplMultithreadingTests
    {
    static uint64_t s_blockingTaskTimeout;
    static Utf8CP s_rulesetId;
    TestRuleSetLocaterPtr m_locater;
    TestSelectionManager* m_selectionManager;
    BeAtomic<bool> m_block;
    BeAtomic<bool> m_didFinishBlocking;
    BeAtomic<bool> m_didGetHit;
    folly::Future<folly::Unit> m_result;

    RulesDrivenECPresentationManagerRequestCancelationTests()
        : m_result(folly::makeFuture())
        {}

    void BlockECPresentationThread()
        {
        // add a blocking task to the ECPresentation thread
        uint64_t startTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        m_block.store(true);
        m_didFinishBlocking.store(false);
        folly::via(&m_manager->GetExecutor(), [this, startTime]()
            {
            while (m_block && (BeTimeUtilities::GetCurrentTimeAsUnixMillis() < (startTime + s_blockingTaskTimeout)))
                BeThreadUtilities::BeSleep(1);
            m_didFinishBlocking.store(true);
            });
        }

    virtual void SetUp() override
        {
        RulesDrivenECPresentationManagerCustomImplMultithreadingTests::SetUp();

        m_selectionManager = new TestSelectionManager(m_connections);
        m_manager->SetSelectionManager(m_selectionManager);

        m_locater = TestRuleSetLocater::Create();
        m_manager->GetLocaters().RegisterLocater(*m_locater);
        m_locater->AddRuleSet(*PresentationRuleSet::CreateInstance(s_rulesetId, 1, 0, false, "", "", "", false));
        }

    virtual void TearDown() override
        {
        DELETE_AND_CLEAR(m_selectionManager);
        RulesDrivenECPresentationManagerCustomImplMultithreadingTests::TearDown();
        }

    template<typename TCallback>
    void DoRequest(TCallback callback, bool expectCanceled = true)
        {
        m_didGetHit.store(false);
        if (expectCanceled)
            {
            // unblock the ECPresentation thread when the request is canceled
            callback = callback.ensure([&]()
                {
                // this WILL be called before the blocking task completes if the cancelation worked
                EXPECT_FALSE(m_didFinishBlocking.load());
                // set the blocking flag to false to unblock the ECPresentation thread
                m_block.store(false);
                });
            }
        m_result = callback.then([](){});
        }

    void VerifyCancelation(bool expectCanceled = true)
        {
        if (expectCanceled)
            {
            // verify the request is finished with a cancelation exception
            EXPECT_TRUE(m_result.isReady());
            EXPECT_TRUE(m_result.hasException());
            EXPECT_TRUE(m_result.getTry().exception().is_compatible_with<folly::FutureCancellation>());
            EXPECT_FALSE(m_didGetHit.load());
            }
        else
            {
            // wait for the result and verify it wasn't canceled
            m_block.store(false);
            m_result.wait();
            EXPECT_FALSE(m_result.hasException());
            EXPECT_TRUE(m_result.hasValue());
            EXPECT_TRUE(m_didGetHit.load());
            }
        }

    void TerminateAndVerifyResult()
        {
        DELETE_AND_CLEAR(m_manager);
        VerifyCancelation();
        }

    void CloseConnectionAndVerifyResult()
        {
        m_connections.NotifyConnectionClosed(*m_connections.GetConnection(s_project->GetECDb()));
        VerifyCancelation();
        }
    
    void DisposeRulesetAndVerifyResult()
        {
        m_locater->Clear();
        VerifyCancelation();
        }
    
    void ChangeSelectionAndVerifyResult(bool expectCanceled = true)
        {
        m_selectionManager->SetSelection(s_project->GetECDb(), *NavNodeKeyListContainer::Create());
        VerifyCancelation(expectCanceled);
        }
    };
uint64_t RulesDrivenECPresentationManagerRequestCancelationTests::s_blockingTaskTimeout = 5 * 1000; // 5 seconds to avoid hanging tests on failure
Utf8CP RulesDrivenECPresentationManagerRequestCancelationTests::s_rulesetId = "some ruleset id";

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsRootNodesCountRequestWhenManagerIsTerminated)
    {
    // set the request handler
    m_impl->SetRootNodesCountHandler([&](IConnectionCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return 0;
        });

    // request and verify
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->GetRootNodesCount(s_project->GetECDb(), options.GetJson()));
    TerminateAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsRootNodesCountRequestWhenConnectionIsClosed)
    {
    // set the request handler
    m_impl->SetRootNodesCountHandler([&](IConnectionCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return 0;
        });

    // request and verify
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->GetRootNodesCount(s_project->GetECDb(), options.GetJson()));
    CloseConnectionAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsRootNodesCountRequestWhenRulesetIsDisposed)
    {
    // set the request handler
    m_impl->SetRootNodesCountHandler([&](IConnectionCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return 0;
        });

    // request and verify
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->GetRootNodesCount(s_project->GetECDb(), options.GetJson()));
    DisposeRulesetAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsRootNodesRequestWhenManagerIsTerminated)
    {
    // set the request handler
    m_impl->SetRootNodesHandler([&](IConnectionCR, PageOptionsCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return nullptr;
        });

    // request and verify
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()));
    TerminateAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsRootNodesRequestWhenConnectionIsClosed)
    {
    // set the request handler
    m_impl->SetRootNodesHandler([&](IConnectionCR, PageOptionsCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return nullptr;
        });

    // request and verify
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()));
    CloseConnectionAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsRootNodesRequestWhenRulesetIsDisposed)
    {
    // set the request handler
    m_impl->SetRootNodesHandler([&](IConnectionCR, PageOptionsCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return nullptr;
        });

    // request and verify
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->GetRootNodes(s_project->GetECDb(), PageOptions(), options.GetJson()));
    DisposeRulesetAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsChildNodesCountRequestWhenManagerIsTerminated)
    {
    // set the request handler
    m_impl->SetChildNodesCountHandler([&](IConnectionCR, NavNodeCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return 0;
        });

    // request and verify
    TestNavNodePtr parentNode = TestNavNode::Create();
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->GetChildrenCount(s_project->GetECDb(), *parentNode, options.GetJson()));
    TerminateAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsChildNodesCountRequestWhenConnectionIsClosed)
    {
    // set the request handler
    m_impl->SetChildNodesCountHandler([&](IConnectionCR, NavNodeCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return 0;
        });

    // request and verify
    TestNavNodePtr parentNode = TestNavNode::Create();
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->GetChildrenCount(s_project->GetECDb(), *parentNode, options.GetJson()));
    CloseConnectionAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsChildNodesCountRequestWhenRulesetIsDisposed)
    {
    // set the request handler
    m_impl->SetChildNodesCountHandler([&](IConnectionCR, NavNodeCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return 0;
        });

    // request and verify
    TestNavNodePtr parentNode = TestNavNode::Create();
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->GetChildrenCount(s_project->GetECDb(), *parentNode, options.GetJson()));
    DisposeRulesetAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsChildNodesRequestWhenManagerIsTerminated)
    {
    // set the request handler
    m_impl->SetChildNodesHandler([&](IConnectionCR, NavNodeCR, PageOptionsCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return nullptr;
        });

    // request and verify
    TestNavNodePtr parentNode = TestNavNode::Create();
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->GetChildren(s_project->GetECDb(), *parentNode, PageOptions(), options.GetJson()));
    TerminateAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsChildNodesRequestWhenConnectionIsClosed)
    {
    // set the request handler
    m_impl->SetChildNodesHandler([&](IConnectionCR, NavNodeCR, PageOptionsCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return nullptr;
        });

    // request and verify
    TestNavNodePtr parentNode = TestNavNode::Create();
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->GetChildren(s_project->GetECDb(), *parentNode, PageOptions(), options.GetJson()));
    CloseConnectionAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsChildNodesRequestWhenRulesetIsDisposed)
    {
    // set the request handler
    m_impl->SetChildNodesHandler([&](IConnectionCR, NavNodeCR, PageOptionsCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return nullptr;
        });

    // request and verify
    TestNavNodePtr parentNode = TestNavNode::Create();
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->GetChildren(s_project->GetECDb(), *parentNode, PageOptions(), options.GetJson()));
    DisposeRulesetAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsHasChildRequestWhenManagerIsTerminated)
    {
    // set the request handler
    m_impl->SetHasChildHandler([&](IConnectionCR, NavNodeCR, NavNodeKeyCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return false;
        });

    // request and verify
    TestNavNodePtr parentNode = TestNavNode::Create();
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->HasChild(s_project->GetECDb(), *parentNode, *DisplayLabelGroupingNodeKey::Create(1, "test"), options.GetJson()));
    TerminateAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsHasChildRequestWhenConnectionIsClosed)
    {
    // set the request handler
    m_impl->SetHasChildHandler([&](IConnectionCR, NavNodeCR, NavNodeKeyCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return false;
        });

    // request and verify
    TestNavNodePtr parentNode = TestNavNode::Create();
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->HasChild(s_project->GetECDb(), *parentNode, *DisplayLabelGroupingNodeKey::Create(1, "test"), options.GetJson()));
    CloseConnectionAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsHasChildRequestWhenRulesetIsDisposed)
    {
    // set the request handler
    m_impl->SetHasChildHandler([&](IConnectionCR, NavNodeCR, NavNodeKeyCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return false;
        });

    // request and verify
    TestNavNodePtr parentNode = TestNavNode::Create();
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->HasChild(s_project->GetECDb(), *parentNode, *DisplayLabelGroupingNodeKey::Create(1, "test"), options.GetJson()));
    DisposeRulesetAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsNodeRequestWhenManagerIsTerminated)
    {
    // set the request handler
    m_impl->SetGetNodeHandler([&](IConnectionCR, uint64_t, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return nullptr;
        });

    // request and verify
    BlockECPresentationThread();
    DoRequest(m_manager->GetNode(s_project->GetECDb(), 123));
    TerminateAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsNodeRequestWhenConnectionIsClosed)
    {
    // set the request handler
    m_impl->SetGetNodeHandler([&](IConnectionCR, uint64_t, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return nullptr;
        });

    // request and verify
    BlockECPresentationThread();
    DoRequest(m_manager->GetNode(s_project->GetECDb(), 123));
    CloseConnectionAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsParentNodeRequestWhenManagerIsTerminated)
    {
    // set the request handler
    m_impl->SetGetParentHandler([&](IConnectionCR, NavNodeCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return nullptr;
        });

    // request and verify
    TestNavNodePtr childNode = TestNavNode::Create();
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->GetParent(s_project->GetECDb(), *childNode, options.GetJson()));
    TerminateAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsParentNodeRequestWhenConnectionIsClosed)
    {
    // set the request handler
    m_impl->SetGetParentHandler([&](IConnectionCR, NavNodeCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return nullptr;
        });

    // request and verify
    TestNavNodePtr childNode = TestNavNode::Create();
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->GetParent(s_project->GetECDb(), *childNode, options.GetJson()));
    CloseConnectionAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsParentNodeRequestWhenRulesetIsDisposed)
    {
    // set the request handler
    m_impl->SetGetParentHandler([&](IConnectionCR, NavNodeCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return nullptr;
        });

    // request and verify
    TestNavNodePtr childNode = TestNavNode::Create();
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->GetParent(s_project->GetECDb(), *childNode, options.GetJson()));
    DisposeRulesetAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsFilteredNodesRequestWhenManagerIsTerminated)
    {
    // set the request handler
    m_impl->SetGetFilteredNodesHandler([&](IConnectionCR, Utf8CP, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return bvector<NavNodeCPtr>();
        });

    // request and verify
    TestNavNodePtr childNode = TestNavNode::Create();
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->GetFilteredNodesPaths(s_project->GetECDb(), "", options.GetJson()));
    TerminateAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsFilteredNodesRequestWhenConnectionIsClosed)
    {
    // set the request handler
    m_impl->SetGetFilteredNodesHandler([&](IConnectionCR, Utf8CP, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return bvector<NavNodeCPtr>();
        });

    // request and verify
    TestNavNodePtr childNode = TestNavNode::Create();
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->GetFilteredNodesPaths(s_project->GetECDb(), "", options.GetJson()));
    CloseConnectionAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsFilteredNodesRequestWhenRulesetIsDisposed)
    {
    // set the request handler
    m_impl->SetGetFilteredNodesHandler([&](IConnectionCR, Utf8CP, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return bvector<NavNodeCPtr>();
        });

    // request and verify
    TestNavNodePtr childNode = TestNavNode::Create();
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->GetFilteredNodesPaths(s_project->GetECDb(), "", options.GetJson()));
    DisposeRulesetAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentClassesRequestWhenManagerIsTerminated)
    {
    // set the request handler
    m_impl->SetContentClassesHandler([&](IConnectionCR, Utf8CP, bvector<ECClassCP> const&, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return bvector<SelectClassInfo>();
        });

    // request and verify
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentClasses(s_project->GetECDb(), nullptr, bvector<ECClassCP>(), options.GetJson()));
    TerminateAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentClassesRequestWhenConnectionIsClosed)
    {
    // set the request handler
    m_impl->SetContentClassesHandler([&](IConnectionCR, Utf8CP, bvector<ECClassCP> const&, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return bvector<SelectClassInfo>();
        });

    // request and verify
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentClasses(s_project->GetECDb(), nullptr, bvector<ECClassCP>(), options.GetJson()));
    CloseConnectionAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentClassesRequestWhenRulesetIsDisposed)
    {
    // set the request handler
    m_impl->SetContentClassesHandler([&](IConnectionCR, Utf8CP, bvector<ECClassCP> const&, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return bvector<SelectClassInfo>();
        });

    // request and verify
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentClasses(s_project->GetECDb(), nullptr, bvector<ECClassCP>(), options.GetJson()));
    DisposeRulesetAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentDescriptorRequestWhenManagerIsTerminated)
    {
    // set the request handler
    m_impl->SetContentDescriptorHandler([&](IConnectionCR, Utf8CP, SelectionInfo const&, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return nullptr;
        });

    // request and verify
    SelectionInfo selection("selection provider", false, *NavNodeKeyListContainer::Create());
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()));
    TerminateAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentDescriptorRequestWhenConnectionIsClosed)
    {
    // set the request handler
    m_impl->SetContentDescriptorHandler([&](IConnectionCR, Utf8CP, SelectionInfo const&, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return nullptr;
        });

    // request and verify
    SelectionInfo selection("selection provider", false, *NavNodeKeyListContainer::Create());
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()));
    CloseConnectionAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentDescriptorRequestWhenRulesetIsDisposed)
    {
    // set the request handler
    m_impl->SetContentDescriptorHandler([&](IConnectionCR, Utf8CP, SelectionInfo const&, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return nullptr;
        });

    // request and verify
    SelectionInfo selection("selection provider", false, *NavNodeKeyListContainer::Create());
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()));
    DisposeRulesetAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentDescriptorRequestWhenSelectionOnSameConnectionChanges)
    {
    // set the request handler
    m_impl->SetContentDescriptorHandler([&](IConnectionCR, Utf8CP, SelectionInfo const&, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return nullptr;
        });

    // request and verify
    SelectionInfo selection("selection provider", false, *NavNodeKeyListContainer::Create());
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, selection, options.GetJson()));
    ChangeSelectionAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, DoesntCancelContentDescriptorRequestWhenSelectionOnDifferentConnectionChanges)
    {
    ECDbTestProject project2;
    project2.Create("DoesntCancelContentDescriptorRequestWhenSelectionOnDifferentConnectionChanges");
    IConnectionPtr connection2 = m_connections.NotifyConnectionOpened(project2.GetECDb());

    // set the request handler
    m_impl->SetContentDescriptorHandler([&](IConnectionCR, Utf8CP, SelectionInfo const&, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return nullptr;
        });

    // request and verify
    SelectionInfo selection("selection provider", false, *NavNodeKeyListContainer::Create());
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentDescriptor(connection2->GetDb(), nullptr, selection, options.GetJson()), false);
    ChangeSelectionAndVerifyResult(false);

    m_connections.NotifyConnectionClosed(*connection2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentRequestWhenManagerIsTerminated)
    {
    // set the request handler
    m_impl->SetContentHandler([&](IConnectionCR, ContentDescriptorCR, SelectionInfo const&, PageOptionsCR, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return nullptr;
        });

    // request and verify
    SelectionInfo selection("selection provider", false, *NavNodeKeyListContainer::Create());
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorCPtr descriptor = ContentDescriptor::Create();
    BlockECPresentationThread();
    DoRequest(m_manager->GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()));
    TerminateAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentRequestWhenConnectionIsClosed)
    {
    // set the request handler
    m_impl->SetContentHandler([&](IConnectionCR, ContentDescriptorCR, SelectionInfo const&, PageOptionsCR, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return nullptr;
        });

    // request and verify
    SelectionInfo selection("selection provider", false, *NavNodeKeyListContainer::Create());
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorCPtr descriptor = ContentDescriptor::Create();
    BlockECPresentationThread();
    DoRequest(m_manager->GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()));
    CloseConnectionAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentRequestWhenRulesetIsDisposed)
    {
    // set the request handler
    m_impl->SetContentHandler([&](IConnectionCR, ContentDescriptorCR, SelectionInfo const&, PageOptionsCR, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return nullptr;
        });

    // request and verify
    SelectionInfo selection("selection provider", false, *NavNodeKeyListContainer::Create());
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorCPtr descriptor = ContentDescriptor::Create();
    BlockECPresentationThread();
    DoRequest(m_manager->GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()));
    DisposeRulesetAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentRequestWhenSelectionOnSameConnectionChanges)
    {
    // set the request handler
    m_impl->SetContentHandler([&](IConnectionCR, ContentDescriptorCR, SelectionInfo const&, PageOptionsCR, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return nullptr;
        });

    // request and verify
    SelectionInfo selection("selection provider", false, *NavNodeKeyListContainer::Create());
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorCPtr descriptor = ContentDescriptor::Create();
    BlockECPresentationThread();
    DoRequest(m_manager->GetContent(s_project->GetECDb(), *descriptor, selection, PageOptions(), options.GetJson()));
    ChangeSelectionAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, DoesntCancelContentRequestWhenSelectionOnDifferentConnectionChanges)
    {
    ECDbTestProject project2;
    project2.Create("DoesntCancelContentRequestWhenSelectionOnDifferentConnectionChanges");
    IConnectionPtr connection2 = m_connections.NotifyConnectionOpened(project2.GetECDb());

    // set the request handler
    m_impl->SetContentHandler([&](IConnectionCR, ContentDescriptorCR, SelectionInfo const&, PageOptionsCR, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return nullptr;
        });

    // request and verify
    SelectionInfo selection("selection provider", false, *NavNodeKeyListContainer::Create());
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorCPtr descriptor = ContentDescriptor::Create();
    BlockECPresentationThread();
    DoRequest(m_manager->GetContent(connection2->GetDb(), *descriptor, selection, PageOptions(), options.GetJson()), false);
    ChangeSelectionAndVerifyResult(false);

    m_connections.NotifyConnectionClosed(*connection2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentSetSizeRequestWhenManagerIsTerminated)
    {        
    // set the request handler
    m_impl->SetContentSetSizeHandler([&](IConnectionCR, ContentDescriptorCR, SelectionInfo const&, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return 0;
        });

    // request and verify
    SelectionInfo selection("selection provider", false, *NavNodeKeyListContainer::Create());
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorCPtr descriptor = ContentDescriptor::Create();
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentSetSize(s_project->GetECDb(), *descriptor, selection, options.GetJson()));
    TerminateAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentSetSizeRequestWhenConnectionIsClosed)
    {        
    // set the request handler
    m_impl->SetContentSetSizeHandler([&](IConnectionCR, ContentDescriptorCR, SelectionInfo const&, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return 0;
        });

    // request and verify
    SelectionInfo selection("selection provider", false, *NavNodeKeyListContainer::Create());
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorCPtr descriptor = ContentDescriptor::Create();
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentSetSize(s_project->GetECDb(), *descriptor, selection, options.GetJson()));
    CloseConnectionAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentSetSizeRequestWhenRulesetIsDisposed)
    {        
    // set the request handler
    m_impl->SetContentSetSizeHandler([&](IConnectionCR, ContentDescriptorCR, SelectionInfo const&, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return 0;
        });

    // request and verify
    SelectionInfo selection("selection provider", false, *NavNodeKeyListContainer::Create());
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorCPtr descriptor = ContentDescriptor::Create();
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentSetSize(s_project->GetECDb(), *descriptor, selection, options.GetJson()));
    DisposeRulesetAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentSetSizeRequestWhenSelectionOnSameConnectionChanges)
    {        
    // set the request handler
    m_impl->SetContentSetSizeHandler([&](IConnectionCR, ContentDescriptorCR, SelectionInfo const&, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return 0;
        });

    // request and verify
    SelectionInfo selection("selection provider", false, *NavNodeKeyListContainer::Create());
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorCPtr descriptor = ContentDescriptor::Create();
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentSetSize(s_project->GetECDb(), *descriptor, selection, options.GetJson()));
    ChangeSelectionAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, DoesntCancelContentSetSizeRequestWhenSelectionOnDifferentConnectionChanges)
    { 
    ECDbTestProject project2;
    project2.Create("DoesntCancelContentSetSizeRequestWhenSelectionOnDifferentConnectionChanges");
    IConnectionPtr connection2 = m_connections.NotifyConnectionOpened(project2.GetECDb());
       
    // set the request handler
    m_impl->SetContentSetSizeHandler([&](IConnectionCR, ContentDescriptorCR, SelectionInfo const&, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        return 0;
        });

    // request and verify
    SelectionInfo selection("selection provider", false, *NavNodeKeyListContainer::Create());
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorCPtr descriptor = ContentDescriptor::Create();
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentSetSize(connection2->GetDb(), *descriptor, selection, options.GetJson()), false);
    ChangeSelectionAndVerifyResult(false);

    m_connections.NotifyConnectionClosed(*connection2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsNodeCheckedCallbackWhenManagerIsTerminated)
    {    
    // set the request handler
    m_impl->SetNodeCheckedHandler([&](IConnectionCR, uint64_t, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        });

    // request and verify
    BlockECPresentationThread();
    DoRequest(m_manager->NotifyNodeChecked(s_project->GetECDb(), 1));
    TerminateAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsNodeUncheckedCallbackWhenManagerIsTerminated)
    {    
    // set the request handler
    m_impl->SetNodeUncheckedHandler([&](IConnectionCR, uint64_t, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        });

    // request and verify
    BlockECPresentationThread();
    DoRequest(m_manager->NotifyNodeUnchecked(s_project->GetECDb(), 1));
    TerminateAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsNodeExpandedCallbackWhenManagerIsTerminated)
    {    
    // set the request handler
    m_impl->SetNodeExpandedHandler([&](IConnectionCR, uint64_t, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        });

    // request and verify
    BlockECPresentationThread();
    DoRequest(m_manager->NotifyNodeExpanded(s_project->GetECDb(), 1));
    TerminateAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsNodeCollapsedCallbackWhenManagerIsTerminated)
    {    
    // set the request handler
    m_impl->SetNodeCollapsedHandler([&](IConnectionCR, uint64_t, ICancelationTokenCR)
        {
        m_didGetHit.store(true);
        });

    // request and verify
    BlockECPresentationThread();
    DoRequest(m_manager->NotifyNodeCollapsed(s_project->GetECDb(), 1));
    TerminateAndVerifyResult();
    }