/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <UnitTests/ECPresentation/TestRuleSetLocater.h>
#include <UnitTests/ECPresentation/TestECInstanceChangeEventsSource.h>
#include <folly/BeFolly.h>
#include "../../../Source/TaskScheduler.h"
#include "../Helpers/TestHelpers.h"
#include "../Helpers/TestRulesDrivenECPresentationManagerImpl.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerMultithreadingTestsBase : ECPresentationTest
    {
    static Utf8CP s_projectName;
    static ECDbTestProject* s_project;
    std::shared_ptr<IConnectionManager> m_connections;
    IConnection* m_connection;
    std::shared_ptr<TestECInstanceChangeEventsSource> m_eventsSource;
    ECPresentationManager* m_manager;

    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp() override;
    virtual void TearDown() override;

    virtual std::unique_ptr<IConnectionManager> _CreateConnectionManager() const = 0;
    virtual ECPresentationManager::Params _CreateManagerParams() const
        {
        ECPresentationManager::Params::CachingParams cachingParams;
#ifdef USE_HYBRID_CACHE
        cachingParams.SetCacheMode(ECPresentationManager::Params::CachingParams::Mode::Hybrid);
#else
        cachingParams.SetCacheMode(ECPresentationManager::Params::CachingParams::Mode::Memory);
#endif
        ECPresentationManager::Params params(RulesEngineTestHelpers::GetPaths(BeTest::GetHost()));
        params.SetConnections(m_connections);
        params.SetCachingParams(cachingParams);
        params.SetECInstanceChangeEventSources({ m_eventsSource });
        return params;
        }
    virtual ECPresentationManager::Impl* _CreateCustomImpl(ECPresentationManager::Impl::Params const&) {return nullptr;}

    void Sync() { m_manager->GetTasksCompletion().wait(); }
    };
Utf8CP RulesDrivenECPresentationManagerMultithreadingTestsBase::s_projectName = "RulesDrivenECPresentationManagerMultithreadingTestsBase";
ECDbTestProject* RulesDrivenECPresentationManagerMultithreadingTestsBase::s_project = nullptr;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerMultithreadingTestsBase::SetUpTestCase()
    {
    s_project = new ECDbTestProject();
    s_project->Create(s_projectName, "RulesEngineTest.01.00.ecschema.xml");
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerMultithreadingTestsBase::TearDownTestCase()
    {
    DELETE_AND_CLEAR(s_project);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerMultithreadingTestsBase::SetUp()
    {
    ECPresentationTest::SetUp();
    m_connections = _CreateConnectionManager();
    m_connection = m_connections->CreateConnection(s_project->GetECDb()).get();
    m_eventsSource = std::make_shared<TestECInstanceChangeEventsSource>();
    ECPresentationManager::Params params(_CreateManagerParams());
    m_manager = new ECPresentationManager(params);

    std::shared_ptr<IConnectionManager> connections = _CreateConnectionManager();
    params.SetConnections(connections);
    ECPresentationManager::Impl* customImpl = _CreateCustomImpl(*m_manager->CreateImplParams(params));
    if (nullptr != customImpl)
        {
        m_connections = connections;
        m_connection = m_connections->CreateConnection(s_project->GetECDb()).get();
        m_manager->SetImpl(*customImpl);
        }

    Sync();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void RulesDrivenECPresentationManagerMultithreadingTestsBase::TearDown()
    {
    DELETE_AND_CLEAR(m_manager);
    }

#define VERIFY_THREAD_NE(threadId)  EXPECT_NE(threadId, BeThreadUtilities::GetCurrentThreadId())

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerMultithreadingTests : RulesDrivenECPresentationManagerMultithreadingTestsBase
    {
    std::unique_ptr<IConnectionManager> _CreateConnectionManager() const override {return std::make_unique<TestConnectionManager>();}
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerMultithreadingTests, AllowsLocatingRulesetsFromAnyThread)
    {
    DelayLoadingRuleSetLocaterPtr locater = DelayLoadingRuleSetLocater::Create(*PresentationRuleSet::CreateInstance("test"));
    m_manager->GetLocaters().RegisterLocater(*locater);

    // verify we can locate rulesets on the ECPresentation worker thread
    m_manager->GetTasksManager().CreateAndExecute([&](IECPresentationTaskCR)
        {
        EXPECT_EQ(1, m_manager->GetLocaters().LocateRuleSets(*m_connection, "test").size());
        }).wait();

    locater->Reset();
    Sync();

    // verify we can locate rulesets on the main thread
    EXPECT_EQ(1, m_manager->GetLocaters().LocateRuleSets(*m_connection, "test").size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerMultithreadingTests, ECInstanceChangeEventSourceCallbacksAreCalledOnECPresentationThread)
    {
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
    m_eventsSource->NotifyECInstanceDeleted(s_project->GetECDb(), *anyInstance);
    Sync();
    EXPECT_TRUE(didGetCallback.load());
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerMultithreadingRealConnectionTests : RulesDrivenECPresentationManagerMultithreadingTestsBase
    {
    ECDb m_db;
    std::unique_ptr<IConnectionManager> _CreateConnectionManager() const override {return std::make_unique<ConnectionManager>(_CreateConnectionManagerProps());}
    virtual ConnectionManager::Props _CreateConnectionManagerProps() const {return ConnectionManager::Props();}
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
    virtual void TearDown() override
        {
        m_db.AbandonChanges();
        RulesDrivenECPresentationManagerMultithreadingTestsBase::TearDown();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerMultithreadingRealConnectionTests, HandlesBusyConnectionByWaiting)
    {
    // set up some ruleset so we do actually hit the db
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("ruleset_id");
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
    folly::Future<NodesCountResponse> count = m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(m_db, ruleset->GetRuleSetId(), RulesetVariables()));

    // verify we don't get any result for 1 second
    count.wait(std::chrono::seconds(1));
    ASSERT_FALSE(count.isReady());

    // unlock the db
    txn.Cancel();
    m_db.GetDefaultTransaction()->Begin();

    // verify we do get the result after the lock is released
    NodesCountResponse countResult = count.get();
    EXPECT_NE(0, *countResult);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus ImportSchema(ECDbR db, Utf8CP schemaXml)
    {
    ECN::ECSchemaReadContextPtr context = ECN::ECSchemaReadContext::CreateContext();

    // not all ECDb schemas are included in the ECDb file by default. So add the path to the ECDb XML files to the search paths
    BeFileName ecdbSchemaSearchPath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(ecdbSchemaSearchPath);
    ecdbSchemaSearchPath.AppendToPath(L"ECSchemas").AppendToPath(L"ECDb");
    context->AddSchemaPath(ecdbSchemaSearchPath);

    context->AddSchemaLocater(db.GetSchemaLocater());

    ECSchemaPtr schema = nullptr;
    if (SchemaReadStatus::Success != ECSchema::ReadFromXmlString(schema, schemaXml, *context))
        return ERROR;

    Savepoint sp(db, "ECSchema Import");
    if (SUCCESS == db.Schemas().ImportSchemas(context->GetCache().GetSchemas()))
        {
        sp.Commit();
        return SUCCESS;
        }

    sp.Cancel();
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerMultithreadingRealConnectionTests, HandlesSchemaImport)
    {
    ECClassCP widgetClass = m_db.Schemas().GetClass("RulesEngineTest", "Widget");
    RulesEngineTestHelpers::InsertInstance(m_db, *widgetClass);

    // set up some ruleset so we do actually hit the db
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("ruleset_id");
    ruleset->AddPresentationRule(*new RootNodeRule());
    ruleset->GetRootNodesRules().back()->AddSpecification(*new AllInstanceNodesSpecification());
    RefCountedPtr<TestRuleSetLocater> locater = TestRuleSetLocater::Create();
    locater->AddRuleSet(*ruleset);
    m_manager->GetLocaters().RegisterLocater(*locater);

    // attempt to get nodes count
    folly::Future<NodesCountResponse> count = m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(m_db, ruleset->GetRuleSetId(), RulesetVariables()));

    // import a schema
    Utf8CP schemaXml = ""
        "<?xml version='1.0' encoding='utf-8'?>"
        "<ECSchema schemaName='TestSchema1' nameSpacePrefix='ts1' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
        "</ECSchema>";
    EXPECT_EQ(SUCCESS, ImportSchema(m_db, schemaXml));

    // verify we do get the result after the import
    NodesCountResponse countResult = count.get();
    EXPECT_NE(1, *countResult);
    }

/*---------------------------------------------------------------------------------**//**
* VSTS#73761
* @bsimethod
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
    PresentationRuleSetPtr rules = PresentationRuleSet::CreateInstance(BeTest::GetNameOfCurrentTest());
    ContentInstancesOfSpecificClassesSpecificationP specification = new ContentInstancesOfSpecificClassesSpecification(1,
        Utf8PrintfString("this.IsOfClass(\"%s\", \"%s\")", ecClass->GetName().c_str(), ecClass->GetSchema().GetName().c_str()),
        ecClass->GetFullName(), true, false);
    ContentRuleP rule = new ContentRule("", 1, false);
    rule->AddSpecification(*specification);
    rules->AddPresentationRule(*rule);

    TestRuleSetLocaterPtr locater = TestRuleSetLocater::Create();
    locater->AddRuleSet(*rules);
    m_manager->GetLocaters().RegisterLocater(*locater);

    // get content
    ContentDescriptorCPtr descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    ContentCPtr content = GetValidatedResponse(m_manager->GetContent(AsyncContentRequestParams::Create(m_db, *descriptor)));

    // Attempt to trigger a deadlock by importing a schema and ask for content again
    bvector<ECSchemaCP> schemaList = bvector<ECSchemaCP>{ schema.get() };
    m_db.Schemas().ImportSchemas(schemaList);

    descriptor = GetValidatedResponse(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(m_db, rules->GetRuleSetId(), RulesetVariables(), "", 0, *KeySet::Create())));
    content = GetValidatedResponse(m_manager->GetContent(AsyncContentRequestParams::Create(m_db, *descriptor)));
    ASSERT_TRUE(content.IsValid());
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerMultithreadingRealConnectionWithNoBusyTimeoutTests : RulesDrivenECPresentationManagerMultithreadingRealConnectionTests
    {
    ConnectionManager::Props _CreateConnectionManagerProps() const override
        {
        ConnectionManager::Props props;
        props.SetBusyTimeout((uint64_t)0);
        return props;
        }

    virtual void TearDown() override
        {
        BeTest::SetFailOnAssert(true);
        RulesDrivenECPresentationManagerMultithreadingRealConnectionTests::TearDown();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerMultithreadingRealConnectionWithNoBusyTimeoutTests, ThrowsDbConnectionBusyExceptionWhenReadingLockedDb)
    {
    // set up some ruleset so we do actually hit the db
    PresentationRuleSetPtr ruleset = PresentationRuleSet::CreateInstance("ruleset_id");
    ruleset->AddPresentationRule(*new RootNodeRule());
    ruleset->GetRootNodesRules().back()->AddSpecification(*new AllInstanceNodesSpecification());
    RefCountedPtr<TestRuleSetLocater> locater = TestRuleSetLocater::Create();
    locater->AddRuleSet(*ruleset);
    m_manager->GetLocaters().RegisterLocater(*locater);

    // make a request to create a valid proxy connection
    auto nodesCount = m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(m_db, ruleset->GetRuleSetId(), RulesetVariables())).get();
    EXPECT_NE(0, *nodesCount);

    // lock the db using the primary connection
    m_db.GetDefaultTransaction()->Cancel();
    Savepoint txn(m_db, "Lock primary", true, BeSQLiteTxnMode::Exclusive);
    ASSERT_TRUE(txn.IsActive());

    // Note: This is necessary, because the process of creating nodes starts a read transaction (which succeeds), but then
    // executes a `PRAGMA data_version`, which fails with SQLITE_BUSY and causes an assertion failure. We have no control over
    // that assertion.
    BeTest::SetFailOnAssert(false);

    // initiate a request that executes a query
    folly::Future<NodesResponse> nodesResponse = m_manager->GetNodes(AsyncHierarchyRequestParams::Create(m_db, ruleset->GetRuleSetId(), RulesetVariables()));

    // wait for 1 minute at most, but we expect the future to resolve much quicker - as soon as we hit the BE_SQLITE_BUSY status
    nodesResponse.wait(std::chrono::seconds(60));
    ASSERT_TRUE(nodesResponse.isReady());
    ASSERT_TRUE(nodesResponse.hasException());
    ASSERT_TRUE(nodesResponse.getTry().exception().is_compatible_with<DbConnectionBusyException>());
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesDrivenECPresentationManagerCustomImplMultithreadingTests : RulesDrivenECPresentationManagerMultithreadingTests
    {
    TestRulesDrivenECPresentationManagerImpl* m_impl;
    virtual ECPresentationManager::Impl* _CreateCustomImpl(ECPresentationManager::Impl::Params const& params) override
        {
        m_impl = new TestRulesDrivenECPresentationManagerImpl(params);
        return m_impl;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsNodesCountRequestOnECPresentationThread)
    {
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetNodesCountHandler([&](auto const& params) -> size_t
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        return 0;
        });

    m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), "", RulesetVariables())).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsNodesRequestOnECPresentationThread)
    {
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetNodesHandler([&](auto const& params) -> INavNodesDataSourcePtr
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        return nullptr;
        });

    // request and verify
    m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), "", RulesetVariables())).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsContentClassesRequestOnECPresentationThread)
    {
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetContentClassesHandler([&](auto const& params) -> bvector<SelectClassInfo>
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        return bvector<SelectClassInfo>();
        });

    // request and verify
    m_manager->GetContentClasses(AsyncContentClassesRequestParams::Create(s_project->GetECDb(), "", RulesetVariables(), "", 0, bvector<ECClassCP>())).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsContentDescriptorRequestOnECPresentationThread)
    {
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetContentDescriptorHandler([&](auto const& params) -> ContentDescriptorCPtr
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        return nullptr;
        });

    // request and verify
    m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), "", RulesetVariables(), "", 0, *KeySet::Create())).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsContentRequestOnECPresentationThread)
    {
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetContentHandler([&](auto const& params) -> ContentCPtr
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        return nullptr;
        });

    // request and verify
    ContentDescriptorCPtr descriptor = ContentDescriptor::Create(*m_connection, *PresentationRuleSet::CreateInstance(""), RulesetVariables(), *NavNodeKeyListContainer::Create());
    m_manager->GetContent(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor)).wait();
    EXPECT_TRUE(wasCalled.load());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerCustomImplMultithreadingTests, CallsContentSetSizeRequestOnECPresentationThread)
    {
    BeAtomic<bool> wasCalled(false);
    uintptr_t mainThreadId = BeThreadUtilities::GetCurrentThreadId();
    m_impl->SetContentSetSizeHandler([&](auto const& params) -> size_t
        {
        wasCalled.store(true);
        VERIFY_THREAD_NE(mainThreadId);
        return 0;
        });

    // request and verify
    ContentDescriptorCPtr descriptor = ContentDescriptor::Create(*m_connection, *PresentationRuleSet::CreateInstance(""), RulesetVariables(), *NavNodeKeyListContainer::Create());
    m_manager->GetContentSetSize(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor)).wait();
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
* @bsiclass
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
        : m_result(folly::makeFuture())
        {}

    virtual void SetUp() override
        {
        RulesDrivenECPresentationManagerCustomImplMultithreadingTests::SetUp();

        m_blockingState.store(BlockingState::Waiting);
        m_hitCount.store(0);
        m_result = folly::makeFuture();
        m_connectionInterrupted = false;

        m_locater = TestRuleSetLocater::Create();
        m_manager->GetLocaters().RegisterLocater(*m_locater);
        m_locater->AddRuleSet(*PresentationRuleSet::CreateInstance(s_rulesetId));

        static_cast<TestConnection*>(m_connection)->SetInterruptHandler([&](){m_connectionInterrupted = true;});
        }

    virtual void TearDown() override
        {
        m_locater->Clear();
        RulesDrivenECPresentationManagerCustomImplMultithreadingTests::TearDown();
        }

    void RunTestWithParams(std::function<void(bool)> test)
        {
        test(false);
        TearDown();
        SetUp();
        test(true);
        }

    void BlockThisThread(uint64_t startTime, ICancelationTokenCP cancellationToken)
        {
        m_blockingState.store(BlockingState::Blocking);
        auto shouldBlock = [this, startTime, cancellationToken]()
            {
            return BlockingState::Blocking == m_blockingState.load()
                && (!cancellationToken || !cancellationToken->IsCanceled())
                && (BeTimeUtilities::GetCurrentTimeAsUnixMillis() < (startTime + s_blockingTaskTimeout));
            };
        while (shouldBlock())
            BeThreadUtilities::BeSleep(1);
        m_blockingState.store(BlockingState::Finished);
        }

    void BlockECPresentationThreads()
        {
        // add a blocking task to the ECPresentation thread
        uint64_t startTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        m_blockingState.store(BlockingState::Waiting);
        for (unsigned i = 0; i < m_manager->GetBackgroundThreadsCount(); ++i)
            {
            m_manager->GetTasksManager().CreateAndExecute([this, startTime](IECPresentationTaskCR task)
                {
                BlockThisThread(startTime, task.GetCancelationToken());
                });
            }
        }

    void EnsureBlocked()
        {
        while (m_blockingState < BlockingState::Blocking)
            {
            // wait for the handler to block
            BeThreadUtilities::BeSleep(1);
            }
        }

    void WaitTillStateGetsChanged(ICancelationTokenCP cancellationToken)
        {
        BlockThisThread(BeTimeUtilities::GetCurrentTimeAsUnixMillis(), cancellationToken);
        }

    void Unblock() { m_blockingState.store(BlockingState::Aborted); }

    template<typename TCallback>
    void DoRequest(TCallback callback, bool expectCanceled = true)
        {
        m_hitCount.store(0);
        m_result = callback.then([](){});
        }

    void VerifyCancelation(bool expectCanceled, bool expectConnectionInterrupted, int expectedHitCount)
        {
        // wait for the result and verify the request is finished with expected result
        Unblock();
        m_result.wait();
        if (expectCanceled)
            {
            EXPECT_TRUE(m_result.hasException());
            EXPECT_TRUE(m_result.getTry().exception().is_compatible_with<CancellationException>());
            }
        else
            {
            EXPECT_FALSE(m_result.hasException());
            EXPECT_TRUE(m_result.hasValue());
            }
        EXPECT_EQ(expectedHitCount, m_hitCount.load());
        EXPECT_EQ(expectConnectionInterrupted, m_connectionInterrupted);
        }

    void TerminateAndVerifyResult(bool expectConnectionInterrupted)
        {
        EnsureBlocked();
        DELETE_AND_CLEAR(m_manager);
        VerifyCancelation(true, expectConnectionInterrupted, 0);
        }

    void CloseConnectionAndVerifyResult(bool expectConnectionInterrupted)
        {
        EnsureBlocked();
        static_cast<TestConnectionManager*>(m_connections.get())->NotifyConnectionClosed(*m_connections->GetConnection(s_project->GetECDb()));
        VerifyCancelation(true, expectConnectionInterrupted, 0);
        }

    void DisposeRulesetAndVerifyResult(bool expectConnectionInterrupted)
        {
        EnsureBlocked();
        m_locater->Clear();
        VerifyCancelation(true, expectConnectionInterrupted, 0);
        }
    };
uint64_t RulesDrivenECPresentationManagerRequestCancelationTests::s_blockingTaskTimeout = 5 * 1000; // 5 seconds to avoid hanging tests on failure
Utf8CP RulesDrivenECPresentationManagerRequestCancelationTests::s_rulesetId = "some ruleset id";

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsNodesCountRequestWhenManagerIsTerminated)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetNodesCountHandler([&](auto const& params) -> size_t
            {
            if (started)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return 0;
                }
            m_hitCount.IncrementAtomicPre();
            return 0;
            });

        // request and verify
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables())));
        TerminateAndVerifyResult(started);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsNodesCountRequestWhenConnectionIsClosed)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetNodesCountHandler([&](auto const& params) -> size_t
            {
            if (started)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return 0;
                }
            m_hitCount.IncrementAtomicPre();
            return 0;
            });

        // request and verify
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables())));
        CloseConnectionAndVerifyResult(started);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsNodesCountRequestWhenRulesetIsDisposed)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetNodesCountHandler([&](auto const& params) -> size_t
            {
            if (started)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return 0;
                }
            m_hitCount.IncrementAtomicPre();
            return 0;
            });

        // request and verify
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetNodesCount(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables())));
        DisposeRulesetAndVerifyResult(started);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsNodesRequestWhenManagerIsTerminated)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetNodesHandler([&](auto const& params) -> INavNodesDataSourcePtr
            {
            if (started)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return nullptr;
                }
            m_hitCount.IncrementAtomicPre();
            return nullptr;
            });

        // request and verify
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables())));
        TerminateAndVerifyResult(started);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsNodesRequestWhenConnectionIsClosed)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetNodesHandler([&](auto const& params) -> INavNodesDataSourcePtr
            {
            if (started)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return nullptr;
                }
            m_hitCount.IncrementAtomicPre();
            return nullptr;
            });

        // request and verify
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables())));
        CloseConnectionAndVerifyResult(started);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsNodesRequestWhenRulesetIsDisposed)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetNodesHandler([&](auto const& params) -> INavNodesDataSourcePtr
            {
            if (started)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return nullptr;
                }
            m_hitCount.IncrementAtomicPre();
            return nullptr;
            });

        // request and verify
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetNodes(AsyncHierarchyRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables())));
        DisposeRulesetAndVerifyResult(started);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsFilteredNodesRequestWhenManagerIsTerminated)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetGetFilteredNodesHandler([&](auto const& params) -> bvector<NavNodeCPtr>
            {
            if (started)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return bvector<NavNodeCPtr>();
                }
            m_hitCount.IncrementAtomicPre();
            return bvector<NavNodeCPtr>();
            });

        // request and verify
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetNodePaths(AsyncNodePathsFromFilterTextRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables(), "")));
        TerminateAndVerifyResult(started);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsFilteredNodesRequestWhenConnectionIsClosed)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetGetFilteredNodesHandler([&](auto const& params) -> bvector<NavNodeCPtr>
            {
            if (started)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return bvector<NavNodeCPtr>();
                }
            m_hitCount.IncrementAtomicPre();
            return bvector<NavNodeCPtr>();
            });

        // request and verify
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetNodePaths(AsyncNodePathsFromFilterTextRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables(), "")));
        CloseConnectionAndVerifyResult(started);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsFilteredNodesRequestWhenRulesetIsDisposed)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetGetFilteredNodesHandler([&](auto const& params) -> bvector<NavNodeCPtr>
            {
            if (started)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return bvector<NavNodeCPtr>();
                }
            m_hitCount.IncrementAtomicPre();
            return bvector<NavNodeCPtr>();
            });

        // request and verify
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetNodePaths(AsyncNodePathsFromFilterTextRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables(), "")));
        DisposeRulesetAndVerifyResult(started);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentClassesRequestWhenManagerIsTerminated)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetContentClassesHandler([&](auto const& params) -> bvector<SelectClassInfo>
            {
            if (started)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return bvector<SelectClassInfo>();
                }
            m_hitCount.IncrementAtomicPre();
            return bvector<SelectClassInfo>();
            });

        // request and verify
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetContentClasses(AsyncContentClassesRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables(), "", 0, bvector<ECClassCP>())));
        TerminateAndVerifyResult(started);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentClassesRequestWhenConnectionIsClosed)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetContentClassesHandler([&](auto const& params) -> bvector<SelectClassInfo>
            {
            if (started)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return bvector<SelectClassInfo>();
                }
            m_hitCount.IncrementAtomicPre();
            return bvector<SelectClassInfo>();
            });

        // request and verify
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetContentClasses(AsyncContentClassesRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables(), "", 0, bvector<ECClassCP>())));
        CloseConnectionAndVerifyResult(started);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentClassesRequestWhenRulesetIsDisposed)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetContentClassesHandler([&](auto const& params) -> bvector<SelectClassInfo>
            {
            if (started)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return bvector<SelectClassInfo>();
                }
            m_hitCount.IncrementAtomicPre();
            return bvector<SelectClassInfo>();
            });

        // request and verify
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetContentClasses(AsyncContentClassesRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables(), "", 0, bvector<ECClassCP>())));
        DisposeRulesetAndVerifyResult(started);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentDescriptorRequestWhenManagerIsTerminated)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetContentDescriptorHandler([&](auto const& params) -> ContentDescriptorCPtr
            {
            if (started)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return nullptr;
                }
            m_hitCount.IncrementAtomicPre();
            return nullptr;
            });

        // request and verify
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables(), "", 0, *KeySet::Create())));
        TerminateAndVerifyResult(started);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentDescriptorRequestWhenConnectionIsClosed)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetContentDescriptorHandler([&](auto const& params) -> ContentDescriptorCPtr
            {
            if (started)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return nullptr;
                }
            m_hitCount.IncrementAtomicPre();
            return nullptr;
            });

        // request and verify
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables(), "", 0, *KeySet::Create())));
        CloseConnectionAndVerifyResult(started);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentDescriptorRequestWhenRulesetIsDisposed)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetContentDescriptorHandler([&](auto const& params) -> ContentDescriptorCPtr
            {
            if (started)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return nullptr;
                }
            m_hitCount.IncrementAtomicPre();
            return nullptr;
            });

        // request and verify
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables(), "", 0, *KeySet::Create())));
        DisposeRulesetAndVerifyResult(started);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentDescriptorRequestCancelsOtherRequestsWithSameConnectionAndDisplayTypeAndDifferentSelectionTimestamps)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetContentDescriptorHandler([&](auto const& params) -> ContentDescriptorCPtr
            {
            if (started && m_blockingState < BlockingState::Blocking)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return nullptr;
                }
            m_hitCount.IncrementAtomicPre();
            return nullptr;
            });

        // make first request
        SelectionInfoPtr selectionInfo1 = SelectionInfo::Create(BeTest::GetNameOfCurrentTest(), false, 1);
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables(), "Test", 0, *KeySet::Create(), selectionInfo1.get())));
        EnsureBlocked();

        // make second request
        SelectionInfoPtr selectionInfo2 = SelectionInfo::Create(BeTest::GetNameOfCurrentTest(), false, 2);
        auto req = m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables(), "Test", 0, *KeySet::Create(), selectionInfo2.get()));

        Unblock();
        req.wait();

        // verify
        VerifyCancelation(true, started, 1);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentDescriptorRequestDoesntCancelRequestsWithDifferentSelectionSources)
    {
    // set the request handler
    m_impl->SetContentDescriptorHandler([&](auto const& params) -> ContentDescriptorCPtr
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // make first request
    SelectionInfoPtr selectionInfo1 = SelectionInfo::Create(Utf8PrintfString("%s:%d", BeTest::GetNameOfCurrentTest(), 1), false);
    BlockECPresentationThreads();
    DoRequest(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables(), "", 0, *KeySet::Create(), selectionInfo1.get())), false);

    // make second request
    SelectionInfoPtr selectionInfo2 = SelectionInfo::Create(Utf8PrintfString("%s:%d", BeTest::GetNameOfCurrentTest(), 2), false);
    auto req = m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables(), "", 0, *KeySet::Create(), selectionInfo2.get()));

    // wait until first request gets blocked and abort blocking
    EnsureBlocked();
    Unblock();

    // let the second request finish
    req.wait();

    // verify
    VerifyCancelation(false, false, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentDescriptorRequestDoesntCancelRequestsWithSameSelectionTimestamps)
    {
    // set the request handler
    m_impl->SetContentDescriptorHandler([&](auto const& params) -> ContentDescriptorCPtr
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // make first request
    SelectionInfoPtr selectionInfo = SelectionInfo::Create(BeTest::GetNameOfCurrentTest(), false);
    BlockECPresentationThreads();
    DoRequest(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables(), "", 0, *KeySet::Create(), selectionInfo.get())), false);

    // make second request
    auto req = m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables(), "", 0, *KeySet::Create(), selectionInfo.get()));

    // wait until first request gets blocked and abort blocking
    EnsureBlocked();
    Unblock();

    // let the second request finish
    req.wait();

    // verify
    VerifyCancelation(false, false, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentDescriptorRequestDoesntCancelRequestsWithDifferentConnections)
    {
    ECDbTestProject project2;
    project2.Create("DoesntCancelContentDescriptorRequestWhenSelectionOnDifferentConnectionChanges");
    RefCountedPtr<TestConnection> connection2 = static_cast<TestConnectionManager*>(m_connections.get())->NotifyConnectionOpened(project2.GetECDb());

    // set the request handler
    m_impl->SetContentDescriptorHandler([&](auto const& params) -> ContentDescriptorCPtr
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // make the first request
    SelectionInfoPtr selectionInfo = SelectionInfo::Create(BeTest::GetNameOfCurrentTest(), false);
    BlockECPresentationThreads();
    DoRequest(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables(), "", 0, *KeySet::Create(), selectionInfo.get())), false);

    // make second request
    auto req = m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(connection2->GetECDb(), s_rulesetId, RulesetVariables(), "", 0, *KeySet::Create(), selectionInfo.get()));

    // wait until first request gets blocked and abort blocking
    EnsureBlocked();
    Unblock();

    // let the second request finish
    req.wait();

    // verify
    VerifyCancelation(false, false, 2);

    static_cast<TestConnectionManager*>(m_connections.get())->NotifyConnectionClosed(*connection2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentDescriptorRequestDoesntCancelRequestsWithDifferentDisplayTypes)
    {
    // set the request handler
    m_impl->SetContentDescriptorHandler([&](auto const& params) -> ContentDescriptorCPtr
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // make first request
    SelectionInfoPtr selectionInfo = SelectionInfo::Create(BeTest::GetNameOfCurrentTest(), false);
    BlockECPresentationThreads();
    DoRequest(m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables(), "Test1", 0, *KeySet::Create(), selectionInfo.get())), false);

    // make second request
    auto req = m_manager->GetContentDescriptor(AsyncContentDescriptorRequestParams::Create(s_project->GetECDb(), s_rulesetId, RulesetVariables(), "Test2", 0, *KeySet::Create(), selectionInfo.get()));

    // wait until first request gets blocked and abort blocking
    EnsureBlocked();
    Unblock();

    // let the second request finish
    req.wait();

    // verify
    VerifyCancelation(false, false, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentRequestWhenManagerIsTerminated)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetContentHandler([&](auto const& params) -> ContentCPtr
            {
            if (started)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return nullptr;
                }
            m_hitCount.IncrementAtomicPre();
            return nullptr;
            });

        // request and verify
        ContentDescriptorCPtr descriptor = ContentDescriptor::Create(*m_connection, *PresentationRuleSet::CreateInstance(s_rulesetId), RulesetVariables(), *NavNodeKeyListContainer::Create());
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetContent(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor)));
        TerminateAndVerifyResult(started);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentRequestWhenConnectionIsClosed)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetContentHandler([&](auto const& params) -> ContentCPtr
            {
            if (started)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return nullptr;
                }
            m_hitCount.IncrementAtomicPre();
            return nullptr;
            });

        // request and verify
        ContentDescriptorCPtr descriptor = ContentDescriptor::Create(*m_connection, *PresentationRuleSet::CreateInstance(s_rulesetId), RulesetVariables(), *NavNodeKeyListContainer::Create());
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetContent(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor)));
        CloseConnectionAndVerifyResult(started);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentRequestWhenRulesetIsDisposed)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetContentHandler([&](auto const& params) -> ContentCPtr
            {
            if (started)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return nullptr;
                }
            m_hitCount.IncrementAtomicPre();
            return nullptr;
            });

        // request and verify
        ContentDescriptorCPtr descriptor = ContentDescriptor::Create(*m_connection, *PresentationRuleSet::CreateInstance(s_rulesetId), RulesetVariables(), *NavNodeKeyListContainer::Create());
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetContent(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor)));
        DisposeRulesetAndVerifyResult(started);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentRequestCancelsOtherRequestsWithSameConnectionAndDisplayTypeAndDifferentSelectionTimestamps)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetContentHandler([&](auto const& params) -> ContentCPtr
            {
            if (started && m_blockingState < BlockingState::Blocking)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return nullptr;
                }
            m_hitCount.IncrementAtomicPre();
            return nullptr;
            });

        // make first request
        ContentDescriptorPtr descriptor1 = ContentDescriptor::Create(*m_connection, *PresentationRuleSet::CreateInstance(s_rulesetId), RulesetVariables(), *NavNodeKeyListContainer::Create());
        descriptor1->SetSelectionInfo(*SelectionInfo::Create(BeTest::GetNameOfCurrentTest(), false, 1));
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetContent(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor1)));
        EnsureBlocked();

        // make second request
        ContentDescriptorPtr descriptor2 = ContentDescriptor::Create(*m_connection, *PresentationRuleSet::CreateInstance(s_rulesetId), RulesetVariables(), *NavNodeKeyListContainer::Create());
        descriptor2->SetSelectionInfo(*SelectionInfo::Create(BeTest::GetNameOfCurrentTest(), false, 2));
        auto req = m_manager->GetContent(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor2));

        Unblock();
        req.wait();

        // verify
        VerifyCancelation(true, started, 1);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentRequestDoesntCancelRequestsWithDifferentSelectionSources)
    {
    // set the request handler
    m_impl->SetContentHandler([&](auto const& params) -> ContentCPtr
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // make first request
    ContentDescriptorPtr descriptor1 = ContentDescriptor::Create(*m_connection, *PresentationRuleSet::CreateInstance(s_rulesetId), RulesetVariables(), *NavNodeKeyListContainer::Create());
    descriptor1->SetSelectionInfo(*SelectionInfo::Create(Utf8PrintfString("%s:%d", BeTest::GetNameOfCurrentTest(), 1), false));
    BlockECPresentationThreads();
    DoRequest(m_manager->GetContent(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor1)), false);

    // make second request
    ContentDescriptorPtr descriptor2 = ContentDescriptor::Create(*m_connection, *PresentationRuleSet::CreateInstance(s_rulesetId), RulesetVariables(), *NavNodeKeyListContainer::Create());
    descriptor2->SetSelectionInfo(*SelectionInfo::Create(Utf8PrintfString("%s:%d", BeTest::GetNameOfCurrentTest(), 2), false));
    auto req = m_manager->GetContent(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor2));

    // wait until first request gets blocked and abort blocking
    EnsureBlocked();
    Unblock();

    // let the second request finish
    req.wait();

    // verify
    VerifyCancelation(false, false, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentRequestDoesntCancelRequestsWithSameSelectionTimestamps)
    {
    // set the request handler
    m_impl->SetContentHandler([&](auto const& params) -> ContentCPtr
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // make first request
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, *PresentationRuleSet::CreateInstance(s_rulesetId), RulesetVariables(), *NavNodeKeyListContainer::Create());
    descriptor->SetSelectionInfo(*SelectionInfo::Create(BeTest::GetNameOfCurrentTest(), false));
    BlockECPresentationThreads();
    DoRequest(m_manager->GetContent(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor)), false);

    // make second request
    auto req = m_manager->GetContent(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor));

    // wait until first request gets blocked and abort blocking
    EnsureBlocked();
    Unblock();

    // let the second request finish
    req.wait();

    // verify
    VerifyCancelation(false, false, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentRequestDoesntCancelRequestsWithDifferentConnections)
    {
    ECDbTestProject project2;
    project2.Create(BeTest::GetNameOfCurrentTest());
    IConnectionPtr connection2 = static_cast<TestConnectionManager*>(m_connections.get())->NotifyConnectionOpened(project2.GetECDb());

    // set the request handler
    m_impl->SetContentHandler([&](auto const& params) -> ContentCPtr
        {
        m_hitCount.IncrementAtomicPre();
        return nullptr;
        });

    // make the first request
    ContentDescriptorCPtr descriptor1 = ContentDescriptor::Create(*m_connection, *PresentationRuleSet::CreateInstance(s_rulesetId), RulesetVariables(), *NavNodeKeyListContainer::Create());
    BlockECPresentationThreads();
    DoRequest(m_manager->GetContent(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor1)), false);

    // make second request
    ContentDescriptorCPtr descriptor2 = ContentDescriptor::Create(*connection2, *PresentationRuleSet::CreateInstance(s_rulesetId), RulesetVariables(), *NavNodeKeyListContainer::Create());
    auto req = m_manager->GetContent(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor2));

    // wait until first request gets blocked and abort blocking
    EnsureBlocked();
    Unblock();

    // let the second request finish
    req.wait();

    // verify
    VerifyCancelation(false, false, 2);

    static_cast<TestConnectionManager*>(m_connections.get())->NotifyConnectionClosed(*connection2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentSetSizeRequestWhenManagerIsTerminated)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetContentSetSizeHandler([&](auto const& params) -> size_t
            {
            if (started)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return 0;
                }
            m_hitCount.IncrementAtomicPre();
            return 0;
            });

        // request and verify
        ContentDescriptorCPtr descriptor = ContentDescriptor::Create(*m_connection, *PresentationRuleSet::CreateInstance(s_rulesetId), RulesetVariables(), *NavNodeKeyListContainer::Create());
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetContentSetSize(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor)));
        TerminateAndVerifyResult(started);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentSetSizeRequestWhenConnectionIsClosed)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetContentSetSizeHandler([&](auto const& params) -> size_t
            {
            if (started)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return 0;
                }
            m_hitCount.IncrementAtomicPre();
            return 0;
            });

        // request and verify
        ContentDescriptorCPtr descriptor = ContentDescriptor::Create(*m_connection, *PresentationRuleSet::CreateInstance(s_rulesetId), RulesetVariables(), *NavNodeKeyListContainer::Create());
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetContentSetSize(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor)));
        CloseConnectionAndVerifyResult(started);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, CancelsContentSetSizeRequestWhenRulesetIsDisposed)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetContentSetSizeHandler([&](auto const& params) -> size_t
            {
            if (started)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return 0;
                }
            m_hitCount.IncrementAtomicPre();
            return 0;
            });

        // request and verify
        ContentDescriptorCPtr descriptor = ContentDescriptor::Create(*m_connection, *PresentationRuleSet::CreateInstance(s_rulesetId), RulesetVariables(), *NavNodeKeyListContainer::Create());
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetContentSetSize(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor)));
        DisposeRulesetAndVerifyResult(started);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentSetSizeRequestCancelsOtherRequestsWithSameConnectionAndDisplayTypeAndDifferentSelectionTimestamps)
    {
    RunTestWithParams([this](bool started)
        {
        // set the request handler
        m_impl->SetContentSetSizeHandler([&](auto const& params) -> size_t
            {
            if (started && m_blockingState < BlockingState::Blocking)
                {
                // Wait till request gets canceled
                WaitTillStateGetsChanged(params.GetCancellationToken());
                if (params.GetCancellationToken() && params.GetCancellationToken()->IsCanceled())
                    return 0;
                }
            m_hitCount.IncrementAtomicPre();
            return 0;
            });

        // make first request
        ContentDescriptorPtr descriptor1 = ContentDescriptor::Create(*m_connection, *PresentationRuleSet::CreateInstance(s_rulesetId), RulesetVariables(), *NavNodeKeyListContainer::Create());
        descriptor1->SetSelectionInfo(*SelectionInfo::Create(BeTest::GetNameOfCurrentTest(), false, 1));
        if (!started)
            BlockECPresentationThreads();
        DoRequest(m_manager->GetContentSetSize(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor1)));
        EnsureBlocked();

        // make second request
        ContentDescriptorPtr descriptor2 = ContentDescriptor::Create(*m_connection, *PresentationRuleSet::CreateInstance(s_rulesetId), RulesetVariables(), *NavNodeKeyListContainer::Create());
        descriptor2->SetSelectionInfo(*SelectionInfo::Create(BeTest::GetNameOfCurrentTest(), false, 2));
        auto req = m_manager->GetContentSetSize(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor2));

        Unblock();
        req.wait();

        // verify
        VerifyCancelation(true, started, 1);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentSetSizeRequestDoesntCancelRequestsWithDifferentSelectionSources)
    {
    // set the request handler
    m_impl->SetContentSetSizeHandler([&](auto const& params) -> size_t
        {
        m_hitCount.IncrementAtomicPre();
        return 0;
        });

    // make first request
    ContentDescriptorPtr descriptor1 = ContentDescriptor::Create(*m_connection, *PresentationRuleSet::CreateInstance(s_rulesetId), RulesetVariables(), *NavNodeKeyListContainer::Create());
    descriptor1->SetSelectionInfo(*SelectionInfo::Create(Utf8PrintfString("%s:%d", BeTest::GetNameOfCurrentTest(), 1), false));
    BlockECPresentationThreads();
    DoRequest(m_manager->GetContentSetSize(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor1)), false);

    // make second request
    ContentDescriptorPtr descriptor2 = ContentDescriptor::Create(*m_connection, *PresentationRuleSet::CreateInstance(s_rulesetId), RulesetVariables(), *NavNodeKeyListContainer::Create());
    descriptor2->SetSelectionInfo(*SelectionInfo::Create(Utf8PrintfString("%s:%d", BeTest::GetNameOfCurrentTest(), 2), false));
    auto req = m_manager->GetContentSetSize(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor2));

    // wait until first request gets blocked and abort blocking
    EnsureBlocked();
    Unblock();

    // let the second request finish
    req.wait();

    // verify
    VerifyCancelation(false, false, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentSetSizeRequestDoesntCancelRequestsWithSameSelectionTimestamps)
    {
    // set the request handler
    m_impl->SetContentSetSizeHandler([&](auto const& params) -> size_t
        {
        m_hitCount.IncrementAtomicPre();
        return 0;
        });

    // make first request
    ContentDescriptorPtr descriptor = ContentDescriptor::Create(*m_connection, *PresentationRuleSet::CreateInstance(s_rulesetId), RulesetVariables(), *NavNodeKeyListContainer::Create());
    descriptor->SetSelectionInfo(*SelectionInfo::Create(BeTest::GetNameOfCurrentTest(), false));
    BlockECPresentationThreads();
    DoRequest(m_manager->GetContentSetSize(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor)), false);

    // make second request
    auto req = m_manager->GetContentSetSize(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor));

    // wait until first request gets blocked and abort blocking
    EnsureBlocked();
    Unblock();

    // let the second request finish
    req.wait();

    // verify
    VerifyCancelation(false, false, 2);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesDrivenECPresentationManagerRequestCancelationTests, ContentSetSizeRequestDoesntCancelRequestsWithDifferentConnections)
    {
    ECDbTestProject project2;
    project2.Create(BeTest::GetNameOfCurrentTest());
    RefCountedPtr<TestConnection> connection2 = static_cast<TestConnectionManager*>(m_connections.get())->NotifyConnectionOpened(project2.GetECDb());

    // set the request handler
    m_impl->SetContentSetSizeHandler([&](auto const& params) -> size_t
        {
        m_hitCount.IncrementAtomicPre();
        return 0;
        });

    // make the first request
    ContentDescriptorCPtr descriptor1 = ContentDescriptor::Create(*m_connection, *PresentationRuleSet::CreateInstance(s_rulesetId), RulesetVariables(), *NavNodeKeyListContainer::Create());
    BlockECPresentationThreads();
    DoRequest(m_manager->GetContentSetSize(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor1)), false);

    // make second request
    ContentDescriptorCPtr descriptor2 = ContentDescriptor::Create(*connection2, *PresentationRuleSet::CreateInstance(s_rulesetId), RulesetVariables(), *NavNodeKeyListContainer::Create());
    auto req = m_manager->GetContentSetSize(AsyncContentRequestParams::Create(s_project->GetECDb(), *descriptor2));

    // wait until first request gets blocked and abort blocking
    EnsureBlocked();
    Unblock();

    // let the second request finish
    req.wait();

    // verify
    VerifyCancelation(false, false, 2);

    static_cast<TestConnectionManager*>(m_connections.get())->NotifyConnectionClosed(*connection2);
    }
