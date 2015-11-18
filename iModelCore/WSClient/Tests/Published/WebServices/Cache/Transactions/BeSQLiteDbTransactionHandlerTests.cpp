/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Transactions/BeSQLiteDbTransactionHandlerTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "BeSQLiteDbTransactionHandlerTests.h"

#include <WebServices/Cache/Transactions/BeSQLiteDbTransactionHandler.h>
#include <MobileDgn/Utils/Threading/WorkerThread.h>
#include "../CachingTestsHelper.h"

using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

void DebugLog(Utf8CP str)
    {
    //BeDebugLog (stc);
    };

struct TestBeSQLiteDbTransactionHandler : BeSQLiteDbTransactionHandler
    {
    public:
        uint64_t m_lastBusyCount;
        Utf8String m_name;

    public:
        TestBeSQLiteDbTransactionHandler(BeSQLite::Db& db, Utf8StringCR name) :
            BeSQLiteDbTransactionHandler(db),
            m_lastBusyCount(0),
            m_name(name)
            {};

        virtual ~TestBeSQLiteDbTransactionHandler()
            {};

        bool OnBusy(uint64_t count) override
            {
            DebugLog(Utf8PrintfString("%s BSY:%d", m_name.c_str(), count));
            m_lastBusyCount = count;
            //BeThreadUtilities::BeSleep (1);
            return true;
            }
    };

// DISABLED as asserts log to output
TEST_F(BeSQLiteDbTransactionHandlerTests, DISABLED_CommitTransaction_TransactionNotStarted_Error)
    {
    BeSQLite::Db::CreateParams createParams;
    createParams.SetStartDefaultTxn(BeSQLite::StartDefaultTransaction::DefaultTxn_No);
    BeSQLite::Db db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(StubFilePath(), BeDbGuid(), createParams));

    BeSQLiteDbTransactionHandler handler(db);
    BeTest::SetFailOnAssert(false);
    EXPECT_EQ(ERROR, handler.CommitTransaction());
    BeTest::SetFailOnAssert(true);
    }

TEST_F(BeSQLiteDbTransactionHandlerTests, CommitTransaction_ChangesDone_ChangesSaved)
    {
    BeSQLite::Db::CreateParams createParams;
    createParams.SetStartDefaultTxn(BeSQLite::StartDefaultTransaction::DefaultTxn_No);
    BeSQLite::Db db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(StubFilePath(), BeDbGuid(), createParams));

    BeSQLiteDbTransactionHandler handler(db);
    EXPECT_EQ(SUCCESS, handler.BeginTransaction());

    PropertySpec propertySpec("Prop", "DB");
    EXPECT_EQ(DbResult::BE_SQLITE_OK, db.SavePropertyString(propertySpec, "Foo"));
    EXPECT_TRUE(db.HasProperty(propertySpec));

    EXPECT_EQ(SUCCESS, handler.CommitTransaction());

    EXPECT_EQ(SUCCESS, handler.BeginTransaction());
    EXPECT_TRUE(db.HasProperty(propertySpec));
    EXPECT_EQ(SUCCESS, handler.CommitTransaction());
    }

TEST_F(BeSQLiteDbTransactionHandlerTests, CommitTransaction_ChangesDoneInSecondTransaction_ChangesSaved)
    {
    BeSQLite::Db::CreateParams createParams;
    createParams.SetStartDefaultTxn(BeSQLite::StartDefaultTransaction::DefaultTxn_No);
    BeSQLite::Db db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(StubFilePath(), BeDbGuid(), createParams));

    BeSQLiteDbTransactionHandler handler(db);
    EXPECT_EQ(SUCCESS, handler.BeginTransaction());
    EXPECT_EQ(SUCCESS, handler.CommitTransaction());

    EXPECT_EQ(SUCCESS, handler.BeginTransaction());

    PropertySpec propertySpec("Prop", "DB");
    EXPECT_EQ(DbResult::BE_SQLITE_OK, db.SavePropertyString(propertySpec, "Foo"));
    EXPECT_TRUE(db.HasProperty(propertySpec));

    EXPECT_EQ(SUCCESS, handler.CommitTransaction());

    EXPECT_EQ(SUCCESS, handler.BeginTransaction());
    EXPECT_TRUE(db.HasProperty(propertySpec));
    EXPECT_EQ(SUCCESS, handler.CommitTransaction());
    }

TEST_F(BeSQLiteDbTransactionHandlerTests, RollbackTransaction_ChangesDone_ChangesRemoved)
    {
    BeSQLite::Db::CreateParams createParams;
    createParams.SetStartDefaultTxn(BeSQLite::StartDefaultTransaction::DefaultTxn_No);
    BeSQLite::Db db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(StubFilePath(), BeDbGuid(), createParams));

    BeSQLiteDbTransactionHandler handler(db);
    EXPECT_EQ(SUCCESS, handler.BeginTransaction());

    PropertySpec propertySpec("Prop", "DB");
    EXPECT_EQ(DbResult::BE_SQLITE_OK, db.SavePropertyString(propertySpec, "Foo"));
    EXPECT_TRUE(db.HasProperty(propertySpec));

    EXPECT_EQ(SUCCESS, handler.RollbackTransaction());

    EXPECT_EQ(SUCCESS, handler.BeginTransaction());
    EXPECT_FALSE(db.HasProperty(propertySpec));
    EXPECT_EQ(SUCCESS, handler.CommitTransaction());
    }

TEST_F(BeSQLiteDbTransactionHandlerTests, StartTransaction_TwoConnections_SecondConnectionIsBlockedUntilFirstFinishesTransaction)
    {
    auto thread1 = WorkerThread::Create("thread1");
    auto thread2 = WorkerThread::Create("thread2");

    BeSQLite::Db::CreateParams params;
    params.SetStartDefaultTxn(BeSQLite::StartDefaultTransaction::DefaultTxn_No);

    BeFileName path = StubFilePath();

    BeSQLite::Db db;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, db.CreateNewDb(path, BeDbGuid(), params));
    db.CloseDb();

    for (int i = 0; i < 10; i++)
        {
        DebugLog(">>> \n\n------------------------- StartTransaction_TwoConnections_SecondConnectionIsBlockedUntilFirstFinishesTransaction -----------\n\n");

        BeSQLite::Db db1, db2;
        ASSERT_EQ(DbResult::BE_SQLITE_OK, db1.OpenBeSQLiteDb(path, params));
        ASSERT_EQ(DbResult::BE_SQLITE_OK, db2.OpenBeSQLiteDb(path, params));

        AsyncTestCheckpoint db1Start, db2Start, db1InTransaction, db2InTransaction;

        TestBeSQLiteDbTransactionHandler handler1(db1, "DB1");
        TestBeSQLiteDbTransactionHandler handler2(db2, "DB2");

        // Run async tests
        thread1->ExecuteAsync([&]
            {
            db1Start.CheckinAndWait();

            DebugLog(">>> DB1 St");
            EXPECT_EQ(SUCCESS, handler1.BeginTransaction());
            DebugLog(">>> DB1 Std");

            db1InTransaction.CheckinAndWait();

            // EXPECT_EQ ("", "Next line fails randomly");
            DebugLog(">>> DB1 Cmt");
            EXPECT_EQ(SUCCESS, handler1.CommitTransaction());
            DebugLog(">>> DB1 Cmtd");
            });

        thread2->ExecuteAsync([&]
            {
            db2Start.CheckinAndWait();

            DebugLog(">>> DB2 St");
            EXPECT_EQ(SUCCESS, handler2.BeginTransaction());
            DebugLog(">>> DB2 Std");

            db2InTransaction.CheckinAndWait();

            DebugLog(">>> DB2 Cmt");
            EXPECT_EQ(SUCCESS, handler2.CommitTransaction());
            DebugLog(">>> DB2 Cmtd");
            });

        // Control async tests
        db1Start.WaitUntilReached();
        db2Start.WaitUntilReached();
        db1Start.Continue();
        db1InTransaction.WaitUntilReached();
        db2Start.Continue();

        while (handler2.m_lastBusyCount == 0); // Wait until handler2 gets busy error and retries
        EXPECT_FALSE(db2InTransaction.WasReached()); // Check if still blocked

        db1InTransaction.Continue();
        db2InTransaction.WaitUntilReached();
        db2InTransaction.Continue();

        thread1->OnEmpty()->Wait();
        thread2->OnEmpty()->Wait();
        }
    }
