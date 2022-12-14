/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentation/Connection.h>
#include <folly/Executor.h>
#include <UnitTests/ECPresentation/TestConnectionCache.h>
#include "../Helpers/ECDbTestProject.h"

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

#define WAIT_UNTIL(beatomicBool) \
    while (!beatomicBool) \
        BeThreadUtilities::BeSleep(1);

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(ConnectionManagerTests, WaitsForProxyConnectionsToCloseBeforeClosingPrimaryConnection)
    {
    m_manager.NotifyConnectionOpened(s_project->GetECDb());
    IConnectionPtr primaryConnection = m_manager.GetConnection(s_project->GetECDb());
    ASSERT_TRUE(primaryConnection.IsValid());

    BeAtomic<bool> didWorkerThreadStart(false);
    BeAtomic<bool> didClosePrimaryECDb(false);
    BeAtomic<bool> didWorkerThreadFinish(false);
    std::thread([&]()
        {
        // request a connection on a different thread
        IConnectionPtr proxyConnection = m_manager.GetConnection(primaryConnection->GetId().c_str());
        ASSERT_TRUE(proxyConnection.IsValid());
        didWorkerThreadStart.store(true);

        // sleep for 100 ms before releasing the proxy connection to
        // get the primary thread to block
        BeThreadUtilities::BeSleep(100);
        proxyConnection = nullptr;

        // wait for primary thread to finish closing the primary connection
        WAIT_UNTIL(didClosePrimaryECDb);
        didWorkerThreadFinish.store(true);
        }).detach();

    // wait for worker thread to create the proxy connection
    WAIT_UNTIL(didWorkerThreadStart);

    // the proxy connection is created, now attempt to close the primary ECDb
    // this thread gets blocked until the proxy connection is not used anymore
    s_project->GetECDb().CloseDb();
    didClosePrimaryECDb.store(true);

    // wait for worker thread to finish its verifications
    WAIT_UNTIL(didWorkerThreadFinish);

    // verify the primary connection is closed and removed from cache
    EXPECT_FALSE(primaryConnection->IsOpen());

    // verify we can't get another proxy connection
    std::thread([&]()
        {
        IConnectionPtr proxyConnection = m_manager.GetConnection(primaryConnection->GetId().c_str());
        ASSERT_TRUE(proxyConnection.IsNull());
        }).join();
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