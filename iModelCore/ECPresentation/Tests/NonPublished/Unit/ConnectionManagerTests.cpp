/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentation/Connection.h>
#include <folly/Executor.h>
#include <UnitTests/ECPresentation/TestConnectionCache.h>
#include "../Helpers/ECDbTestProject.h"
#include <chrono>
#include <future>

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ConnectionManagerTests : ECPresentationTest
    {
    static ECDbTestProject* s_project;
    ConnectionManager m_manager;

    static void SetUpTestCase()
        {
        s_project = new ECDbTestProject();
        s_project->Create("ConnectionManagerTests");
        }

    static void TearDownTestCase()
        {
        DELETE_AND_CLEAR(s_project);
        }

    void SetUp() override
        {
        ECPresentationTest::SetUp();
        if (!s_project->GetECDb().IsDbOpen())
            s_project->Open("ConnectionManagerTests");
        }
    };
ECDbTestProject* ConnectionManagerTests::s_project = nullptr;

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ConnectionManagerTests, ReturnsNullWhenNotNotified)
    {
    ASSERT_TRUE(nullptr == m_manager.GetConnection(s_project->GetECDb()));
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ConnectionManagerTests, ReturnsCachedConnectionWhenRequestedByPrimaryECDb)
    {
    m_manager.NotifyConnectionOpened(s_project->GetECDb());
    IConnectionP connection = m_manager.GetConnection(s_project->GetECDb());
    ASSERT_TRUE(nullptr != connection);
    EXPECT_TRUE(connection->IsOpen());
    EXPECT_EQ(&s_project->GetECDb(), &connection->GetDb());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ConnectionManagerTests, ReturnsCachedConnectionWhenRequestedByConnectionId)
    {
    m_manager.NotifyConnectionOpened(s_project->GetECDb());
    IConnectionP connectionByDb = m_manager.GetConnection(s_project->GetECDb());
    ASSERT_TRUE(nullptr != connectionByDb);

    IConnectionP connectionById = m_manager.GetConnection(connectionByDb->GetId().c_str());
    ASSERT_TRUE(nullptr != connectionById);
    EXPECT_TRUE(connectionById->IsOpen());
    EXPECT_EQ(&s_project->GetECDb(), &connectionById->GetDb());
    EXPECT_EQ(connectionByDb->GetId(), connectionById->GetId());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ConnectionManagerTests, ReturnsProxyConnectionWhenRequestedByPrimaryECDbOnDifferentThread)
    {
    m_manager.NotifyConnectionOpened(s_project->GetECDb());
    IConnectionPtr primaryConnection = m_manager.GetConnection(s_project->GetECDb());
    ASSERT_TRUE(primaryConnection.IsValid());

    std::thread([&]()
        {
        // need to request by id first to get proxy created
        m_manager.GetConnection(primaryConnection->GetId().c_str());
        // request a connection on a different thread
        IConnectionPtr proxyConnection = m_manager.GetConnection(s_project->GetECDb());
        ASSERT_TRUE(proxyConnection.IsValid());
        EXPECT_TRUE(proxyConnection->IsOpen());
        // expect connections to be based on the same datasource but different sqlite connections
        EXPECT_EQ(primaryConnection->GetId(), proxyConnection->GetId());
        EXPECT_EQ(&s_project->GetECDb(), &proxyConnection->GetECDb());
        EXPECT_NE(&proxyConnection->GetECDb(), &proxyConnection->GetDb());
        }).join();
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ConnectionManagerTests, ReturnsProxyConnectionWhenRequestedByConnectionIdOnDifferentThread)
    {
    m_manager.NotifyConnectionOpened(s_project->GetECDb());
    IConnectionPtr primaryConnection = m_manager.GetConnection(s_project->GetECDb());
    ASSERT_TRUE(primaryConnection.IsValid());

    std::thread([&]()
        {
        // request a connection on a different thread
        IConnectionPtr proxyConnection = m_manager.GetConnection(primaryConnection->GetId().c_str());
        ASSERT_TRUE(proxyConnection.IsValid());
        EXPECT_TRUE(proxyConnection->IsOpen());
        // expect connections to be based on the same datasource but different sqlite connections
        EXPECT_EQ(primaryConnection->GetId(), proxyConnection->GetId());
        EXPECT_EQ(&s_project->GetECDb(), &proxyConnection->GetECDb());
        EXPECT_NE(&proxyConnection->GetECDb(), &proxyConnection->GetDb());
        }).join();
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ConnectionManagerTests, WaitsForProxyConnectionsToCloseBeforeClosingPrimaryConnection)
    {
    m_manager.NotifyConnectionOpened(s_project->GetECDb());
    IConnectionPtr primaryConnection = m_manager.GetConnection(s_project->GetECDb());
    ASSERT_TRUE(primaryConnection.IsValid());

    std::promise<bool> proxyCreated;
    auto proxyCreatedFuture = proxyCreated.get_future();
    std::promise<void> closeNotified;
    auto closeNotifiedFuture = closeNotified.get_future();
    std::promise<void> closeFinished;
    auto closeFinishedFuture = closeFinished.get_future();
    bool receivedCloseNotification = false;
    bool closedBeforeProxyRelease = false;

    TestConnectionsListener listener([&](ConnectionEvent const& event)
        {
        if (ConnectionEventType::Closed == event.GetEventType())
            closeNotified.set_value();
        });
    m_manager.AddListener(listener);

    Utf8String connectionId = primaryConnection->GetId();
    std::thread worker([&]()
        {
        IConnectionPtr proxyConnection = m_manager.GetConnection(connectionId.c_str());
        proxyCreated.set_value(proxyConnection.IsValid());
        if (proxyConnection.IsNull())
            return;

        // Keep the proxy until close has started, then check that close is still waiting for it.
        receivedCloseNotification = (std::future_status::ready == closeNotifiedFuture.wait_for(std::chrono::seconds(5)));
        if (receivedCloseNotification)
            closedBeforeProxyRelease = (std::future_status::ready == closeFinishedFuture.wait_for(std::chrono::milliseconds(200)));
        proxyConnection = nullptr;
        });

    bool createdInTime = (std::future_status::ready == proxyCreatedFuture.wait_for(std::chrono::seconds(5)));
    bool hasProxy = createdInTime && proxyCreatedFuture.get();
    if (hasProxy)
        s_project->GetECDb().CloseDb();
    else
        closeNotified.set_value();
    closeFinished.set_value();
    worker.join();
    m_manager.DropListener(listener);

    ASSERT_TRUE(hasProxy);
    EXPECT_TRUE(receivedCloseNotification);
    EXPECT_FALSE(closedBeforeProxyRelease);
    EXPECT_FALSE(primaryConnection->IsOpen());

    std::thread([&]()
        {
        IConnectionPtr proxyConnection = m_manager.GetConnection(connectionId.c_str());
        ASSERT_TRUE(proxyConnection.IsNull());
        }).join();
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ConnectionManagerTests, ClosesWhenProxyIsReleasedDuringCloseNotification)
    {
    m_manager.NotifyConnectionOpened(s_project->GetECDb());
    IConnectionPtr primaryConnection = m_manager.GetConnection(s_project->GetECDb());
    ASSERT_TRUE(primaryConnection.IsValid());

    std::promise<bool> proxyCreated;
    auto proxyCreatedFuture = proxyCreated.get_future();
    std::promise<void> releaseProxy;
    auto releaseProxyFuture = releaseProxy.get_future();
    std::promise<void> proxyReleased;
    auto proxyReleasedFuture = proxyReleased.get_future();
    bool releasedDuringNotification = false;

    TestConnectionsListener listener([&](ConnectionEvent const& event)
        {
        if (ConnectionEventType::Closed != event.GetEventType())
            return;

        // Finish the release callback before close proceeds to the store's wait.
        releaseProxy.set_value();
        releasedDuringNotification = (std::future_status::ready == proxyReleasedFuture.wait_for(std::chrono::seconds(5)));
        });
    m_manager.AddListener(listener);

    Utf8String connectionId = primaryConnection->GetId();
    std::thread worker([&]()
        {
        IConnectionPtr proxy = m_manager.GetConnection(connectionId.c_str());
        proxyCreated.set_value(proxy.IsValid());
        if (proxy.IsNull() || std::future_status::ready != releaseProxyFuture.wait_for(std::chrono::seconds(5)))
            return;

        proxy = nullptr;
        proxyReleased.set_value();
        });

    bool createdInTime = (std::future_status::ready == proxyCreatedFuture.wait_for(std::chrono::seconds(5)));
    bool hasProxy = createdInTime && proxyCreatedFuture.get();
    if (hasProxy)
        s_project->GetECDb().CloseDb();
    else
        releaseProxy.set_value();
    worker.join();
    m_manager.DropListener(listener);

    ASSERT_TRUE(hasProxy);
    EXPECT_TRUE(releasedDuringNotification);
    EXPECT_FALSE(primaryConnection->IsOpen());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ConnectionManagerTests, NotifiesListenersWhenClosingECDb)
    {
    m_manager.NotifyConnectionOpened(s_project->GetECDb());
    IConnectionPtr primaryConnection = m_manager.GetConnection(s_project->GetECDb());
    ASSERT_TRUE(primaryConnection.IsValid());

    int callCount = 0;
    TestConnectionsListener listener([&](ConnectionEvent const&)
        {
        ++callCount;
        });
    m_manager.AddListener(listener);

    s_project->GetECDb().CloseDb();
    EXPECT_EQ(1, callCount);
    EXPECT_FALSE(primaryConnection->IsOpen());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ConnectionManagerTests, NotifiesListenersWhenClosingConnections)
    {
    m_manager.NotifyConnectionOpened(s_project->GetECDb());
    IConnectionPtr primaryConnection = m_manager.GetConnection(s_project->GetECDb());
    ASSERT_TRUE(primaryConnection.IsValid());

    int callCount = 0;
    TestConnectionsListener listener([&](ConnectionEvent const&)
        {
        ++callCount;
        });
    m_manager.AddListener(listener);
    m_manager.CloseConnections();
    EXPECT_EQ(1, callCount);
    EXPECT_FALSE(primaryConnection->IsOpen());
    EXPECT_TRUE(s_project->GetECDb().IsDbOpen());

    s_project->GetECDb().CloseDb();
    EXPECT_EQ(1, callCount);
    EXPECT_FALSE(primaryConnection->IsOpen());
    EXPECT_FALSE(s_project->GetECDb().IsDbOpen());
    }