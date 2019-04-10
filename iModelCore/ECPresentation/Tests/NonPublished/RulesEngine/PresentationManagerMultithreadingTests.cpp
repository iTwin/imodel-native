/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/PresentationManagerMultithreadingTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "TestHelpers.h"
#include "TestRulesDrivenPresentationManagerImpl.h"
#include <UnitTests/BackDoor/ECPresentation/TestRuleSetLocater.h>
#include <UnitTests/BackDoor/ECPresentation/TestSelectionProvider.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

#ifndef RULES_ENGINE_FORCE_SINGLE_THREAD

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerMultithreadingTestsBase : ECPresentationTest
    {
    static Utf8CP s_projectName;
    static ECDbTestProject* s_project;
    IConnectionManager* m_connections;
    IConnection* m_connection;
    RulesDrivenECPresentationManager* m_manager;

    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;
    virtual IConnectionManager* _CreateConnectionManager() const = 0;

    void Sync()
        {
        folly::via(&m_manager->GetExecutor(), []() {}).wait();
        }
    };
Utf8CP RulesDrivenECPresentationManagerMultithreadingTestsBase::s_projectName = "RulesDrivenECPresentationManagerMultithreadingTestsBase";
ECDbTestProject* RulesDrivenECPresentationManagerMultithreadingTestsBase::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerMultithreadingTestsBase::SetUpTestCase()
    {
    Localization::Init();
    s_project = new ECDbTestProject();
    s_project->Create(s_projectName, "RulesEngineTest.01.00.ecschema.xml");
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerMultithreadingTestsBase::TearDownTestCase()
    {
    DELETE_AND_CLEAR(s_project);
    Localization::Terminate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerMultithreadingTestsBase::SetUp()
    {
    ECPresentationTest::SetUp();
    m_connections = _CreateConnectionManager();
    m_manager = new RulesDrivenECPresentationManager(*m_connections, RulesEngineTestHelpers::GetPaths(BeTest::GetHost()), true);
    m_connection = m_connections->CreateConnection(s_project->GetECDb()).get();
    Sync();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerMultithreadingTestsBase::TearDown()
    {
    if (nullptr != m_manager)
        DELETE_AND_CLEAR(m_manager);
    DELETE_AND_CLEAR(m_connections);
    }

#ifdef RULES_ENGINE_FORCE_SINGLE_THREAD
    #define VERIFY_THREAD_NE(threadId)  (void)threadId;
#else
    #define VERIFY_THREAD_NE(threadId)  EXPECT_NE(threadId, BeThreadUtilities::GetCurrentThreadId())
#endif

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerMultithreadingTests : RulesDrivenECPresentationManagerMultithreadingTestsBase
    {
    IConnectionManager* _CreateConnectionManager() const override {return new TestConnectionManager();}
    };

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
    RuntimeJsonLocalState localState;
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
struct RulesDrivenECPresentationManagerMultithreadingRealConnectionTests : RulesDrivenECPresentationManagerMultithreadingTestsBase
    {
    ECDb m_db;
    IConnectionManager* _CreateConnectionManager() const override {return new ConnectionManager();}
    virtual void SetUp() override
        {
        RulesDrivenECPresentationManagerMultithreadingTestsBase::SetUp();

        BeFileName projectPath;
        BeTest::GetHost().GetTempDir(projectPath);
        projectPath.AppendToPath(L"RulesDrivenECPresentationManagerMultithreadingRealConnectionTests");
        BeFileName::CreateNewDirectory(projectPath);
        projectPath.AppendToPath(WString(BeTest::GetNameOfCurrentTest(), true).c_str());
        projectPath.BeDeleteFile();
        BeFileName::BeCopyFile(WString(s_project->GetECDbPath(), true).c_str(), projectPath, true);
        m_db.OpenBeSQLiteDb(projectPath, Db::OpenParams(Db::OpenMode::ReadWrite));

        m_connection = m_connections->CreateConnection(m_db).get();
        Sync();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerMultithreadingRealConnectionTests, HandlesBusyConnectionByWaiting)
    {
    // set up some ruleset so we do actually hit the db
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("ruleset_id", 1, 0, false, "", "", "", false);
    ruleset->AddPresentationRule(*new RootNodeRule());
    ruleset->GetRootNodesRules().back()->AddSpecification(*new AllInstanceNodesSpecification());
    RefCountedPtr<TestRuleSetLocater> locater = TestRuleSetLocater::Create();
    locater->AddRuleSet(*ruleset);
    m_manager->GetLocaters().RegisterLocater(*locater);

    // lock the db using the primary connection
    m_db.GetDefaultTransaction()->Cancel();
    Savepoint txn(m_db, "Lock primary", true, BeSQLiteTxnMode::Exclusive);
    ASSERT_TRUE(txn.IsActive());

    // attempt to get some data (don't expect to get any, but make sure request succeeds)
    NavNodeKeyCPtr key = LabelGroupingNodeKey::Create("label", {"a", "b", "c"}, 1);
    RulesDrivenECPresentationManager::NavigationOptions options(ruleset->GetRuleSetId().c_str(), RuleTargetTree::TargetTree_MainTree);
    folly::Future<size_t> count = m_manager->GetRootNodesCount(m_db, options.GetJson());

    // verify we don't get any result for 1 second
    count.wait(std::chrono::seconds(1));
    ASSERT_FALSE(count.isReady());

    // unlock the db
    txn.Cancel();
    m_db.GetDefaultTransaction()->Begin();

    // verify we do get the result after the lock is released
    size_t countResult = count.get();
    EXPECT_NE(0, countResult);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerMultithreadingRealConnectionTests, SetsUpCustomFunctionsInBothPrimaryAndProxyConnections)
    {
    // prepare the dataset
    ECClassCP ecClass = m_db.Schemas().GetClass("RulesEngineTest", "Widget");
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *ecClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    ContentInstancesOfSpecificClassesSpecificationP specification = new ContentInstancesOfSpecificClassesSpecification(1, 
        Utf8PrintfString("this.IsOfClass(\"%s\", \"%s\")", ecClass->GetName().c_str(), ecClass->GetSchema().GetName().c_str()), 
        ecClass->GetFullName(), true);
    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*specification);
    rules->AddPresentationRule(*rule);

    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    locater->AddRuleSet(*rules);
    m_manager->GetLocaters().RegisterLocater(*locater);

    // get content
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(m_db, nullptr, *KeySet::Create(), nullptr, options.GetJson()).get();
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();

    // assert
    ASSERT_TRUE(content.IsValid());
    DataContainer<ContentSetItemCPtr> contentSet = content->GetContentSet();
    EXPECT_EQ(1, contentSet.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas              02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerMultithreadingRealConnectionTests, UnregisterCustomFunctionsInBothPrimaryAndProxyConnections)
    {
    // prepare the dataset
    ECClassCP ecClass = m_db.Schemas().GetClass("RulesEngineTest", "Widget");
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *ecClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    ContentInstancesOfSpecificClassesSpecificationP specification = new ContentInstancesOfSpecificClassesSpecification(1,
        Utf8PrintfString("this.IsOfClass(\"%s\", \"%s\")", ecClass->GetName().c_str(), ecClass->GetSchema().GetName().c_str()),
        ecClass->GetFullName(), true);
    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*specification);
    rules->AddPresentationRule(*rule);

    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    locater->AddRuleSet(*rules);
    m_manager->GetLocaters().RegisterLocater(*locater);

    // get content
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(m_db, nullptr, *KeySet::Create(), nullptr, options.GetJson()).get();
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();

    // Assert that custom functions are registered
    ASSERT_TRUE(content.IsValid());
    DbFunction* function = nullptr;
    m_db.TryGetSqlFunction(function, FUNCTION_NAME_IsOfClass, 3);
    ASSERT_NE(nullptr, function);
    
    // Remove presentation manager
    DELETE_AND_CLEAR(m_manager);

    // Assert that custom functions are unregistered
    m_db.TryGetSqlFunction(function, FUNCTION_NAME_IsOfClass, 3);
    ASSERT_EQ(nullptr, function);
    }

/*---------------------------------------------------------------------------------**//**
* VSTS#73761
* @bsimethod                                    Haroldas.Vitunskas              02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerMultithreadingRealConnectionTests, HandlesProjectReloadCorrectly)
    {
    Utf8CP schemaXML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    "<ECSchema schemaName=\"TestSchema\" nameSpacePrefix=\"test\" version=\"01.01\" xmlns=\"http://www.bentley.com/schemas/Bentley.ECXML.2.0\">"
    "    <ECSchemaReference name =\"Bentley_Standard_CustomAttributes\" version =\"01.04\" prefix =\"bsca\" />"
    "    <ECClass typeName=\"ClassA\" isDomainClass=\"True\">"
    "        <ECProperty propertyName=\"n\" typeName=\"int\" />"
    "    </ECClass>"
    "    <ECClass typeName=\"ClassB\" >"
    "        <ECProperty propertyName=\"p\" typeName=\"int\" />"
    "    </ECClass>"
    "</ECSchema>";
    ECSchemaPtr schema = nullptr;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, schemaXML, *context);
    
    // prepare the dataset
    ECClassCP ecClass = m_db.Schemas().GetClass("RulesEngineTest", "Widget");
    IECInstancePtr widget = RulesEngineTestHelpers::InsertInstance(m_db, *ecClass, nullptr, true);

    // create the rule set
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest(), 1, 0, false, "", "", "", false);
    ContentInstancesOfSpecificClassesSpecificationP specification = new ContentInstancesOfSpecificClassesSpecification(1,
        Utf8PrintfString("this.IsOfClass(\"%s\", \"%s\")", ecClass->GetName().c_str(), ecClass->GetSchema().GetName().c_str()),
        ecClass->GetFullName(), true);
    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*specification);
    rules->AddPresentationRule(*rule);

    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    locater->AddRuleSet(*rules);
    m_manager->GetLocaters().RegisterLocater(*locater);

    // get content
    RulesDrivenECPresentationManager::ContentOptions options(rules->GetRuleSetId().c_str());
    ContentDescriptorCPtr descriptor = m_manager->GetContentDescriptor(m_db, nullptr, *KeySet::Create(), nullptr, options.GetJson()).get();
    ContentCPtr content = m_manager->GetContent(*descriptor, PageOptions()).get();

    // Attempt to trigger a deadlock by importing a schema and ask for content again
    bvector<ECSchemaCP> schemaList = bvector<ECSchemaCP>{ schema.get() };
    m_db.Schemas().ImportSchemas(schemaList);

    descriptor = m_manager->GetContentDescriptor(m_db, nullptr, *KeySet::Create(), nullptr, options.GetJson()).get();
    content = m_manager->GetContent(*descriptor, PageOptions()).get();
    ASSERT_TRUE(content.IsValid());
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
    RulesDrivenECPresentationManager::Params params(*m_connections, RulesEngineTestHelpers::GetPaths(BeTest::GetHost()));
    m_impl = new TestRulesDrivenECPresentationManagerImpl(*m_manager->CreateDependenciesFactory(), params);
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
    TestNavNodePtr parentNode = TestNavNode::Create(*m_connection);
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
    TestNavNodePtr parentNode = TestNavNode::Create(*m_connection);
    RulesDrivenECPresentationManager::NavigationOptions options("doesnt matter", TargetTree_Both);
    m_manager->GetChildren(s_project->GetECDb(), *parentNode, PageOptions(), options.GetJson()).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsNodeRequestOnECPresentationThread)
    {
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetGetNodeHandler([&](IConnectionCR, NavNodeKeyCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        return nullptr;
        });

    // request and verify
    RulesDrivenECPresentationManager::NavigationOptions options("doesnt matter", TargetTree_Both);
    m_manager->GetNode(s_project->GetECDb(), *NavNodeKey::Create("type", {"1"}), options.GetJson()).wait();
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
    TestNavNodePtr childNode = TestNavNode::Create(*m_connection);
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
    m_impl->SetContentDescriptorHandler([&](IConnectionCR, Utf8CP, KeySetCR, SelectionInfo const*, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        return nullptr;
        });

    // request and verify
    RulesDrivenECPresentationManager::ContentOptions options("doesnt matter");
    m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, *KeySet::Create(), nullptr, options.GetJson()).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsContentRequestOnECPresentationThread)
    {
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetContentHandler([&](ContentDescriptorCR, PageOptionsCR, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        return nullptr;
        });

    // request and verify
    RulesDrivenECPresentationManager::ContentOptions options("doesnt matter");
    ContentDescriptorCPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    m_manager->GetContent(*descriptor, PageOptions()).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsContentSetSizeRequestOnECPresentationThread)
    {        
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetContentSetSizeHandler([&](ContentDescriptorCR, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        return 0;
        });

    // request and verify
    RulesDrivenECPresentationManager::ContentOptions options("doesnt matter");
    ContentDescriptorCPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    m_manager->GetContentSetSize(*descriptor).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsNodeCheckedCallbackOnECPresentationThread)
    {    
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetNodeCheckedHandler([&](IConnectionCR, NavNodeKeyCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        });

    // request and verify
    RulesDrivenECPresentationManager::NavigationOptions options("doesnt matter", TargetTree_Both);
    m_manager->NotifyNodeChecked(s_project->GetECDb(), *NavNodeKey::Create("type", {"1"}), options.GetJson()).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsNodeUncheckedCallbackOnECPresentationThread)
    {    
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetNodeUncheckedHandler([&](IConnectionCR, NavNodeKeyCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        });

    // request and verify
    RulesDrivenECPresentationManager::NavigationOptions options("doesnt matter", TargetTree_Both);
    m_manager->NotifyNodeUnchecked(s_project->GetECDb(), *NavNodeKey::Create("type", {"1"}), options.GetJson()).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsNodeExpandedCallbackOnECPresentationThread)
    {    
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetNodeExpandedHandler([&](IConnectionCR, NavNodeKeyCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        });

    // request and verify
    RulesDrivenECPresentationManager::NavigationOptions options("doesnt matter", TargetTree_Both);
    m_manager->NotifyNodeExpanded(s_project->GetECDb(), *NavNodeKey::Create("type", {"1"}), options.GetJson()).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsNodeCollapsedCallbackOnECPresentationThread)
    {    
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetNodeCollapsedHandler([&](IConnectionCR, NavNodeKeyCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        });

    // request and verify
    RulesDrivenECPresentationManager::NavigationOptions options("doesnt matter", TargetTree_Both);
    m_manager->NotifyNodeCollapsed(s_project->GetECDb(), *NavNodeKey::Create("type", {"1"}), options.GetJson()).wait();
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

enum class BlockingState
    {
    Waiting,
    Blocking,
    Aborted,
    Finished
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerRequestCancelationTests : RulesDrivenECPresentationManagerCustomImplMultithreadingTests
    {
    static uint64_t s_blockingTaskTimeout;
    static Utf8CP s_rulesetId;
    TestRuleSetLocaterPtr m_locater;

    BeAtomic<BlockingState> m_blockingState;
    BeAtomic<int> m_hitCount;
    folly::Future<folly::Unit> m_result;
    bool m_connectionInterrupted;

    RulesDrivenECPresentationManagerRequestCancelationTests()
        : m_result(folly::makeFuture()), m_connectionInterrupted(false)
        {}

    virtual void SetUp() override
        {
        RulesDrivenECPresentationManagerCustomImplMultithreadingTests::SetUp();

        m_locater = TestRuleSetLocater::Create();
        m_manager->GetLocaters().RegisterLocater(*m_locater);
        m_locater->AddRuleSet(*PresentationRuleSet::CreateInstance(s_rulesetId, 1, 0, false, "", "", "", false));

        static_cast<TestConnection*>(m_connection)->SetInterruptHandler([&](){m_connectionInterrupted = true;});
        }

    void BlockECPresentationThread()
        {
        // add a blocking task to the ECPresentation thread
        uint64_t startTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        m_blockingState.store(BlockingState::Waiting);
        folly::via(&m_manager->GetExecutor(), [this, startTime]()
            {
            m_blockingState.store(BlockingState::Blocking);
            while (BlockingState::Blocking == m_blockingState.load() && (BeTimeUtilities::GetCurrentTimeAsUnixMillis() < (startTime + s_blockingTaskTimeout)))
                BeThreadUtilities::BeSleep(1);
            m_blockingState.store(BlockingState::Finished);
            });
        }

    void EnsureBlocked()
        {
        while (m_blockingState < BlockingState::Blocking)
            {
            // wait for the handler to block
            BeThreadUtilities::BeSleep(1);
            }
        }

    template<typename TCallback>
    void DoRequest(TCallback callback, bool expectCanceled = true)
        {
        m_hitCount.store(0);
        if (expectCanceled)
            {
            // unblock the ECPresentation thread when the request is canceled
            callback = callback.ensure([&]()
                {
                // this WILL be called before the blocking task completes if the cancelation worked
                EXPECT_LE(BlockingState::Blocking, m_blockingState.load());
                // set the blocking flag to false to unblock the ECPresentation thread
                m_blockingState.store(BlockingState::Aborted);
                });
            }
        m_result = callback.then([](){});
        }

    void VerifyCancelation(bool expectCanceled, bool expectConnectionInterrupted, int expectedHitCount)
        {
        if (expectCanceled)
            {
            // verify the request is finished with a cancelation exception
            EXPECT_TRUE(m_result.isReady());
            EXPECT_TRUE(m_result.hasException());
            EXPECT_TRUE(m_result.getTry().exception().is_compatible_with<folly::FutureCancellation>());
            
            }
        else
            {
            // wait for the result and verify it wasn't canceled
            m_blockingState.store(BlockingState::Aborted);
            m_result.wait();
            EXPECT_FALSE(m_result.hasException());
            EXPECT_TRUE(m_result.hasValue());
            }
        EXPECT_EQ(expectedHitCount, m_hitCount.load());
        EXPECT_EQ(expectConnectionInterrupted, m_connectionInterrupted);
        }

    void TerminateAndVerifyResult()
        {
        EnsureBlocked();
        DELETE_AND_CLEAR(m_manager);
        VerifyCancelation(true, false, 0);
        }

    void CloseConnectionAndVerifyResult()
        {
        EnsureBlocked();
        static_cast<TestConnectionManager*>(m_connections)->NotifyConnectionClosed(*m_connections->GetConnection(s_project->GetECDb()));
        VerifyCancelation(true, true, 0);
        }
    
    void DisposeRulesetAndVerifyResult()
        {
        EnsureBlocked();
        m_locater->Clear();
        VerifyCancelation(true, false, 0);
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
        m_hitCount.IncrementAtomicPre();
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
        m_hitCount.IncrementAtomicPre();
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
        m_hitCount.IncrementAtomicPre();
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
        m_hitCount.IncrementAtomicPre();
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
        m_hitCount.IncrementAtomicPre();
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
        m_hitCount.IncrementAtomicPre();
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
        m_hitCount.IncrementAtomicPre();
        return 0;
        });

    // request and verify
    TestNavNodePtr parentNode = TestNavNode::Create(*m_connection);
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
        m_hitCount.IncrementAtomicPre();
        return 0;
        });

    // request and verify
    TestNavNodePtr parentNode = TestNavNode::Create(*m_connection);
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
        m_hitCount.IncrementAtomicPre();
        return 0;
        });

    // request and verify
    TestNavNodePtr parentNode = TestNavNode::Create(*m_connection);
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
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // request and verify
    TestNavNodePtr parentNode = TestNavNode::Create(*m_connection);
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
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // request and verify
    TestNavNodePtr parentNode = TestNavNode::Create(*m_connection);
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
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // request and verify
    TestNavNodePtr parentNode = TestNavNode::Create(*m_connection);
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->GetChildren(s_project->GetECDb(), *parentNode, PageOptions(), options.GetJson()));
    DisposeRulesetAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsNodeRequestWhenManagerIsTerminated)
    {
    // set the request handler
    m_impl->SetGetNodeHandler([&](IConnectionCR, NavNodeKeyCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // request and verify
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->GetNode(s_project->GetECDb(), *NavNodeKey::Create("type", {"1"}), options.GetJson()));
    TerminateAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsNodeRequestWhenConnectionIsClosed)
    {
    // set the request handler
    m_impl->SetGetNodeHandler([&](IConnectionCR, NavNodeKeyCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // request and verify
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->GetNode(s_project->GetECDb(), *NavNodeKey::Create("type", {"1"}), options.GetJson()));
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
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // request and verify
    TestNavNodePtr childNode = TestNavNode::Create(*m_connection);
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
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // request and verify
    TestNavNodePtr childNode = TestNavNode::Create(*m_connection);
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
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // request and verify
    TestNavNodePtr childNode = TestNavNode::Create(*m_connection);
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
        m_hitCount.IncrementAtomicPre();
        return bvector<NavNodeCPtr>();
        });

    // request and verify
    TestNavNodePtr childNode = TestNavNode::Create(*m_connection);
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
        m_hitCount.IncrementAtomicPre();
        return bvector<NavNodeCPtr>();
        });

    // request and verify
    TestNavNodePtr childNode = TestNavNode::Create(*m_connection);
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
        m_hitCount.IncrementAtomicPre();
        return bvector<NavNodeCPtr>();
        });

    // request and verify
    TestNavNodePtr childNode = TestNavNode::Create(*m_connection);
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
        m_hitCount.IncrementAtomicPre();
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
        m_hitCount.IncrementAtomicPre();
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
        m_hitCount.IncrementAtomicPre();
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
    m_impl->SetContentDescriptorHandler([&](IConnectionCR, Utf8CP, KeySetCR, SelectionInfo const*, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // request and verify
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, *KeySet::Create(), nullptr, options.GetJson()));
    TerminateAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentDescriptorRequestWhenConnectionIsClosed)
    {
    // set the request handler
    m_impl->SetContentDescriptorHandler([&](IConnectionCR, Utf8CP, KeySetCR, SelectionInfo const*, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // request and verify
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, *KeySet::Create(), nullptr, options.GetJson()));
    CloseConnectionAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentDescriptorRequestWhenRulesetIsDisposed)
    {
    // set the request handler
    m_impl->SetContentDescriptorHandler([&](IConnectionCR, Utf8CP, KeySetCR, SelectionInfo const*, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // request and verify
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, *KeySet::Create(), nullptr, options.GetJson()));
    DisposeRulesetAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentDescriptorRequestCancelsOtherRequestsWithSameConnectionAndDisplayTypeAndDifferentSelectionTimestamps)
    {
    // set the request handler
    m_impl->SetContentDescriptorHandler([&](IConnectionCR, Utf8CP, KeySetCR, SelectionInfo const*, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // make first request
    SelectionInfoPtr selectionInfo1 = SelectionInfo::Create(BeTest::GetNameOfCurrentTest(), false, 1);
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentDescriptor(s_project->GetECDb(), "Test", *KeySet::Create(), selectionInfo1.get(), options.GetJson()));
    EnsureBlocked();

    // make second request
    SelectionInfoPtr selectionInfo2 = SelectionInfo::Create(BeTest::GetNameOfCurrentTest(), false, 2);
    m_manager->GetContentDescriptor(s_project->GetECDb(), "Test", *KeySet::Create(), selectionInfo2.get(), options.GetJson()).wait();

    // verify
    VerifyCancelation(true, true, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentDescriptorRequestDoesntCancelRequestsWithDifferentSelectionSources)
    {
    // set the request handler
    m_impl->SetContentDescriptorHandler([&](IConnectionCR, Utf8CP, KeySetCR, SelectionInfo const*, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // make first request
    SelectionInfoPtr selectionInfo1 = SelectionInfo::Create(Utf8PrintfString("%s:%d", BeTest::GetNameOfCurrentTest(), 1), false);
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, *KeySet::Create(), selectionInfo1.get(), options.GetJson()), false);

    // make second request
    SelectionInfoPtr selectionInfo2 = SelectionInfo::Create(Utf8PrintfString("%s:%d", BeTest::GetNameOfCurrentTest(), 2), false);
    auto req = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, *KeySet::Create(), selectionInfo2.get(), options.GetJson());

    // wait until first request gets blocked and abort blocking
    EnsureBlocked();
    m_blockingState.store(BlockingState::Aborted);

    // let the second request finish
    req.wait();

    // verify
    VerifyCancelation(false, false, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentDescriptorRequestDoesntCancelRequestsWithSameSelectionTimestamps)
    {
    // set the request handler
    m_impl->SetContentDescriptorHandler([&](IConnectionCR, Utf8CP, KeySetCR, SelectionInfo const*, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // make first request
    SelectionInfoPtr selectionInfo = SelectionInfo::Create(BeTest::GetNameOfCurrentTest(), false);
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, *KeySet::Create(), selectionInfo.get(), options.GetJson()), false);

    // make second request
    auto req = m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, *KeySet::Create(), selectionInfo.get(), options.GetJson());

    // wait until first request gets blocked and abort blocking
    EnsureBlocked();
    m_blockingState.store(BlockingState::Aborted);

    // let the second request finish
    req.wait();

    // verify
    VerifyCancelation(false, false, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentDescriptorRequestDoesntCancelRequestsWithDifferentConnections)
    {
    ECDbTestProject project2;
    project2.Create("DoesntCancelContentDescriptorRequestWhenSelectionOnDifferentConnectionChanges");
    RefCountedPtr<TestConnection> connection2 = static_cast<TestConnectionManager*>(m_connections)->NotifyConnectionOpened(project2.GetECDb());

    // set the request handler
    m_impl->SetContentDescriptorHandler([&](IConnectionCR, Utf8CP, KeySetCR, SelectionInfo const*, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // make the first request
    SelectionInfoPtr selectionInfo = SelectionInfo::Create(BeTest::GetNameOfCurrentTest(), false);
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentDescriptor(s_project->GetECDb(), nullptr, *KeySet::Create(), selectionInfo.get(), options.GetJson()), false);

    // make second request
    auto req = m_manager->GetContentDescriptor(connection2->GetECDb(), nullptr, *KeySet::Create(), selectionInfo.get(), options.GetJson());

    // wait until first request gets blocked and abort blocking
    EnsureBlocked();
    m_blockingState.store(BlockingState::Aborted);

    // let the second request finish
    req.wait();
    
    // verify
    VerifyCancelation(false, false, 2);

    static_cast<TestConnectionManager*>(m_connections)->NotifyConnectionClosed(*connection2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentDescriptorRequestDoesntCancelRequestsWithDifferentDisplayTypes)
    {
    // set the request handler
    m_impl->SetContentDescriptorHandler([&](IConnectionCR, Utf8CP, KeySetCR, SelectionInfo const*, RulesDrivenECPresentationManager::ContentOptions const&, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // make first request
    SelectionInfoPtr selectionInfo = SelectionInfo::Create(BeTest::GetNameOfCurrentTest(), false);
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentDescriptor(s_project->GetECDb(), "Test1", *KeySet::Create(), selectionInfo.get(), options.GetJson()), false);

    // make second request
    auto req = m_manager->GetContentDescriptor(s_project->GetECDb(), "Test2", *KeySet::Create(), selectionInfo.get(), options.GetJson());

    // wait until first request gets blocked and abort blocking
    EnsureBlocked();
    m_blockingState.store(BlockingState::Aborted);

    // let the second request finish
    req.wait();

    // verify
    VerifyCancelation(false, false, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentRequestWhenManagerIsTerminated)
    {
    // set the request handler
    m_impl->SetContentHandler([&](ContentDescriptorCR, PageOptionsCR, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // request and verify
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorCPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    BlockECPresentationThread();
    DoRequest(m_manager->GetContent(*descriptor, PageOptions()));
    TerminateAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentRequestWhenConnectionIsClosed)
    {
    // set the request handler
    m_impl->SetContentHandler([&](ContentDescriptorCR, PageOptionsCR, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // request and verify
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorCPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    BlockECPresentationThread();
    DoRequest(m_manager->GetContent(*descriptor, PageOptions()));
    CloseConnectionAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentRequestWhenRulesetIsDisposed)
    {
    // set the request handler
    m_impl->SetContentHandler([&](ContentDescriptorCR, PageOptionsCR, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // request and verify
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorCPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    BlockECPresentationThread();
    DoRequest(m_manager->GetContent(*descriptor, PageOptions()));
    DisposeRulesetAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentRequestCancelsOtherRequestsWithSameConnectionAndDisplayTypeAndDifferentSelectionTimestamps)
    {
    // set the request handler
    m_impl->SetContentHandler([&](ContentDescriptorCR, PageOptionsCR, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // make first request
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorPtr descriptor1 = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    descriptor1->SetSelectionInfo(*SelectionInfo::Create(BeTest::GetNameOfCurrentTest(), false, 1));
    BlockECPresentationThread();
    DoRequest(m_manager->GetContent(*descriptor1, PageOptions()));
    EnsureBlocked();
    
    // make second request
    ContentDescriptorPtr descriptor2 = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    descriptor2->SetSelectionInfo(*SelectionInfo::Create(BeTest::GetNameOfCurrentTest(), false, 2));
    m_manager->GetContent(*descriptor2, PageOptions()).wait();

    // verify
    VerifyCancelation(true, true, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentRequestDoesntCancelRequestsWithDifferentSelectionSources)
    {
    // set the request handler
    m_impl->SetContentHandler([&](ContentDescriptorCR, PageOptionsCR, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // make first request
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorPtr descriptor1 = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    descriptor1->SetSelectionInfo(*SelectionInfo::Create(Utf8PrintfString("%s:%d", BeTest::GetNameOfCurrentTest(), 1), false));
    BlockECPresentationThread();
    DoRequest(m_manager->GetContent(*descriptor1, PageOptions()), false);

    // make second request
    ContentDescriptorPtr descriptor2 = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    descriptor2->SetSelectionInfo(*SelectionInfo::Create(Utf8PrintfString("%s:%d", BeTest::GetNameOfCurrentTest(), 2), false));
    auto req = m_manager->GetContent(*descriptor2, PageOptions());

    // wait until first request gets blocked and abort blocking
    EnsureBlocked();
    m_blockingState.store(BlockingState::Aborted);

    // let the second request finish
    req.wait();

    // verify
    VerifyCancelation(false, false, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentRequestDoesntCancelRequestsWithSameSelectionTimestamps)
    {
    // set the request handler
    m_impl->SetContentHandler([&](ContentDescriptorCR, PageOptionsCR, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // make first request
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    descriptor->SetSelectionInfo(*SelectionInfo::Create(BeTest::GetNameOfCurrentTest(), false));
    BlockECPresentationThread();
    DoRequest(m_manager->GetContent(*descriptor, PageOptions()), false);

    // make second request
    auto req = m_manager->GetContent(*descriptor, PageOptions());

    // wait until first request gets blocked and abort blocking
    EnsureBlocked();
    m_blockingState.store(BlockingState::Aborted);

    // let the second request finish
    req.wait();

    // verify
    VerifyCancelation(false, false, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentRequestDoesntCancelRequestsWithDifferentConnections)
    {
    ECDbTestProject project2;
    project2.Create(BeTest::GetNameOfCurrentTest());
    IConnectionPtr connection2 = static_cast<TestConnectionManager*>(m_connections)->NotifyConnectionOpened(project2.GetECDb());

    // set the request handler
    m_impl->SetContentHandler([&](ContentDescriptorCR, PageOptionsCR, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // make the first request
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorCPtr descriptor1 = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    BlockECPresentationThread();
    DoRequest(m_manager->GetContent(*descriptor1, PageOptions()), false);

    // make second request
    ContentDescriptorCPtr descriptor2 = ContentDescriptor::Create(*connection2, options.GetJson(), *NavNodeKeyListContainer::Create());
    auto req = m_manager->GetContent(*descriptor2, PageOptions());

    // wait until first request gets blocked and abort blocking
    EnsureBlocked();
    m_blockingState.store(BlockingState::Aborted);

    // let the second request finish
    req.wait();
    
    // verify
    VerifyCancelation(false, false, 2);

    static_cast<TestConnectionManager*>(m_connections)->NotifyConnectionClosed(*connection2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentSetSizeRequestWhenManagerIsTerminated)
    {        
    // set the request handler
    m_impl->SetContentSetSizeHandler([&](ContentDescriptorCR, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return 0;
        });

    // request and verify
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorCPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentSetSize(*descriptor));
    TerminateAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentSetSizeRequestWhenConnectionIsClosed)
    {        
    // set the request handler
    m_impl->SetContentSetSizeHandler([&](ContentDescriptorCR, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return 0;
        });

    // request and verify
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorCPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentSetSize(*descriptor));
    CloseConnectionAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentSetSizeRequestWhenRulesetIsDisposed)
    {        
    // set the request handler
    m_impl->SetContentSetSizeHandler([&](ContentDescriptorCR, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return 0;
        });

    // request and verify
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorCPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentSetSize(*descriptor));
    DisposeRulesetAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentSetSizeRequestCancelsOtherRequestsWithSameConnectionAndDisplayTypeAndDifferentSelectionTimestamps)
    {
    // set the request handler
    m_impl->SetContentSetSizeHandler([&](ContentDescriptorCR, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return 0;
        });

    // make first request
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorPtr descriptor1 = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    descriptor1->SetSelectionInfo(*SelectionInfo::Create(BeTest::GetNameOfCurrentTest(), false, 1));
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentSetSize(*descriptor1));
    EnsureBlocked();

    // make second request
    ContentDescriptorPtr descriptor2 = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    descriptor2->SetSelectionInfo(*SelectionInfo::Create(BeTest::GetNameOfCurrentTest(), false, 2));
    m_manager->GetContentSetSize(*descriptor2).wait();

    // verify
    VerifyCancelation(true, true, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentSetSizeRequestDoesntCancelRequestsWithDifferentSelectionSources)
    {
    // set the request handler
    m_impl->SetContentSetSizeHandler([&](ContentDescriptorCR, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return 0;
        });

    // make first request
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorPtr descriptor1 = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    descriptor1->SetSelectionInfo(*SelectionInfo::Create(Utf8PrintfString("%s:%d", BeTest::GetNameOfCurrentTest(), 1), false));
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentSetSize(*descriptor1), false);

    // make second request
    ContentDescriptorPtr descriptor2 = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    descriptor2->SetSelectionInfo(*SelectionInfo::Create(Utf8PrintfString("%s:%d", BeTest::GetNameOfCurrentTest(), 2), false));
    auto req = m_manager->GetContentSetSize(*descriptor2);

    // wait until first request gets blocked and abort blocking
    EnsureBlocked();
    m_blockingState.store(BlockingState::Aborted);

    // let the second request finish
    req.wait();

    // verify
    VerifyCancelation(false, false, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentSetSizeRequestDoesntCancelRequestsWithSameSelectionTimestamps)
    {
    // set the request handler
    m_impl->SetContentSetSizeHandler([&](ContentDescriptorCR, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return 0;
        });

    // make first request
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    descriptor->SetSelectionInfo(*SelectionInfo::Create(BeTest::GetNameOfCurrentTest(), false));
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentSetSize(*descriptor), false);

    // make second request
    auto req = m_manager->GetContentSetSize(*descriptor);

    // wait until first request gets blocked and abort blocking
    EnsureBlocked();
    m_blockingState.store(BlockingState::Aborted);

    // let the second request finish
    req.wait();

    // verify
    VerifyCancelation(false, false, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentSetSizeRequestDoesntCancelRequestsWithDifferentConnections)
    {
    ECDbTestProject project2;
    project2.Create(BeTest::GetNameOfCurrentTest());
    RefCountedPtr<TestConnection> connection2 = static_cast<TestConnectionManager*>(m_connections)->NotifyConnectionOpened(project2.GetECDb());
    
    // set the request handler
    m_impl->SetContentSetSizeHandler([&](ContentDescriptorCR, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        return 0;
        });

    // make the first request
    RulesDrivenECPresentationManager::ContentOptions options(s_rulesetId);
    ContentDescriptorCPtr descriptor1 = ContentDescriptor::Create(*m_connection, options.GetJson(), *NavNodeKeyListContainer::Create());
    BlockECPresentationThread();
    DoRequest(m_manager->GetContentSetSize(*descriptor1), false);

    // make second request
    ContentDescriptorCPtr descriptor2 = ContentDescriptor::Create(*connection2, options.GetJson(), *NavNodeKeyListContainer::Create());
    auto req = m_manager->GetContentSetSize(*descriptor2);

    // wait until first request gets blocked and abort blocking
    EnsureBlocked();
    m_blockingState.store(BlockingState::Aborted);

    // let the second request finish
    req.wait();
    
    // verify
    VerifyCancelation(false, false, 2);

    static_cast<TestConnectionManager*>(m_connections)->NotifyConnectionClosed(*connection2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsNodeCheckedCallbackWhenManagerIsTerminated)
    {    
    // set the request handler
    m_impl->SetNodeCheckedHandler([&](IConnectionCR, NavNodeKeyCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        });

    // request and verify
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->NotifyNodeChecked(s_project->GetECDb(), *NavNodeKey::Create("type", {"1"}), options.GetJson()));
    TerminateAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsNodeUncheckedCallbackWhenManagerIsTerminated)
    {    
    // set the request handler
    m_impl->SetNodeUncheckedHandler([&](IConnectionCR, NavNodeKeyCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        });

    // request and verify
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->NotifyNodeUnchecked(s_project->GetECDb(), *NavNodeKey::Create("type", {"1"}), options.GetJson()));
    TerminateAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsNodeExpandedCallbackWhenManagerIsTerminated)
    {    
    // set the request handler
    m_impl->SetNodeExpandedHandler([&](IConnectionCR, NavNodeKeyCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        });

    // request and verify
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->NotifyNodeExpanded(s_project->GetECDb(), *NavNodeKey::Create("type", {"1"}), options.GetJson()));
    TerminateAndVerifyResult();
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsNodeCollapsedCallbackWhenManagerIsTerminated)
    {    
    // set the request handler
    m_impl->SetNodeCollapsedHandler([&](IConnectionCR, NavNodeKeyCR, RulesDrivenECPresentationManager::NavigationOptions const&, ICancelationTokenCR)
        {
        m_hitCount.IncrementAtomicPre();
        });

    // request and verify
    RulesDrivenECPresentationManager::NavigationOptions options(s_rulesetId, TargetTree_Both);
    BlockECPresentationThread();
    DoRequest(m_manager->NotifyNodeCollapsed(s_project->GetECDb(), *NavNodeKey::Create("type", {"1"}), options.GetJson()));
    TerminateAndVerifyResult();
    }

#endif // RULES_ENGINE_FORCE_SINGLE_THREAD
