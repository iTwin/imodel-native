/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDb_Tests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE
TEST(BeSQLite, TestCachedStatement)
    {
    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);
    BeSQLite::BeSQLiteLib::Initialize(temporaryDir);
    Db db;
    ASSERT_EQ(db.CreateNewDb(BEDB_MemoryDb), BE_SQLITE_OK);
    ASSERT_EQ(db.ExecuteSql("CREATE TABLE Foo(Id INTEGER PRIMARY KEY, Str TEXT NOT NULL)"), BE_SQLITE_OK);
    ASSERT_EQ(db.ExecuteSql("INSERT INTO Foo(Id, Str) VALUES(1,'Test1')"), BE_SQLITE_OK);
    ASSERT_EQ(db.ExecuteSql("INSERT INTO Foo(Id, Str) VALUES(2,'Test2')"), BE_SQLITE_OK);
    ASSERT_EQ(db.ExecuteSql("INSERT INTO Foo(Id, Str) VALUES(3,'Test3')"), BE_SQLITE_OK);

    bvector<CachedStatementPtr> recursionStack(100);
    for (CachedStatementPtr& ptr : recursionStack)
        {
        ptr = db.GetCachedStatement("SELECT Id, Str FROM Foo ORDER BY Id");
        ASSERT_EQ(ptr->Step(), BE_SQLITE_ROW);
        ASSERT_EQ(ptr->GetValueInt64(0), 1);
        ASSERT_STREQ(ptr->GetValueText(1), "Test1");
        }

    for (CachedStatementPtr& ptr : recursionStack)
        {
        ASSERT_EQ(ptr->GetValueInt64(0), 1);
        ASSERT_STREQ(ptr->GetValueText(1), "Test1");
        ASSERT_EQ(ptr->Step(), BE_SQLITE_ROW);
        ASSERT_EQ(ptr->GetValueInt64(0), 2);
        ASSERT_STREQ(ptr->GetValueText(1), "Test2");
        ASSERT_EQ(ptr->Step(), BE_SQLITE_ROW);
        ASSERT_EQ(ptr->GetValueInt64(0), 3);
        ASSERT_STREQ(ptr->GetValueText(1), "Test3");
        ASSERT_EQ(ptr->Step(), BE_SQLITE_DONE);
        }
    }
//=======================================================================================
// @bsiclass                                     Krischan.Eberle                  01/15
//=======================================================================================
struct TestOtherConnectionECDb : ECDb
    {
    int m_changeCount;
    TestOtherConnectionECDb()
        : m_changeCount(0)
        {}

    virtual void _OnDbChangedByOtherConnection() override
        {
        m_changeCount++;
        ECDb::_OnDbChangedByOtherConnection();
        }
    };

//---------------------------------------------------------------------------------------
// Test to ensure that PRAGMA data_version changes when another connection changes a database
// @bsiclass                                     Krischan.Eberle                  01/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, TwoConnections)
    {
    BeFileName testECDbPath;
    {
    ECDbR ecdb = SetupECDb("one.ecdb");
    testECDbPath = BeFileName(ecdb.GetDbFileName());
    ecdb.CloseDb();
    }

    TestOtherConnectionECDb ecdb1, ecdb2;
    ASSERT_EQ(BE_SQLITE_OK, ecdb1.OpenBeSQLiteDb(testECDbPath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite, DefaultTxn::No)));
    ASSERT_EQ(BE_SQLITE_OK, ecdb2.OpenBeSQLiteDb(testECDbPath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite, DefaultTxn::No)));

    { // make a change to the database from the first connection
    Savepoint t1(ecdb1, "tx1");
    // the first transaction should not call _OnDbChangedByOtherConnection
    ASSERT_EQ(0, ecdb1.m_changeCount);
    ecdb1.CreateTable("TEST", "Col1 INTEGER");
    }

    { // make a change to the database from the second connection
    Savepoint t2(ecdb2, "tx2");
    // the first transaction on the second connection should not call _OnDbChangedByOtherConnection
    ASSERT_EQ(0, ecdb2.m_changeCount);
    ecdb2.ExecuteSql("INSERT INTO TEST(Col1) VALUES(3)");
    }

    { // start another transaction on the first connection. This should notice that the second connection changed the db.
    Savepoint t3(ecdb1, "tx1");
    ASSERT_EQ(1, ecdb1.m_changeCount);
    ecdb1.ExecuteSql("INSERT INTO TEST(Col1) VALUES(4)");
    }

    { // additional changes from the same connection should not trigger additional calls to _OnDbChangedByOtherConnection
    Savepoint t3(ecdb1, "tx1");
    ASSERT_EQ(1, ecdb1.m_changeCount);
    ecdb1.ExecuteSql("INSERT INTO TEST(Col1) VALUES(5)");
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/15
//+---------------+---------------+---------------+---------------+---------------+------
struct TestBusyRetry : BusyRetry
    {
    int m_retryCount;
    mutable int m_actualRetryCount;

    Savepoint* m_savepointToCommitDuringRetry;
    int m_retryCountBeforeCommit;


    explicit TestBusyRetry(int retryCount)
        : m_retryCount(retryCount), m_actualRetryCount(0), m_savepointToCommitDuringRetry(nullptr), m_retryCountBeforeCommit(-1)
        {}

    virtual int _OnBusy(int count) const override
        {
        //count is 0-based
        if (count >= m_retryCount)
            return 0;

        m_actualRetryCount = count + 1;

        if (m_savepointToCommitDuringRetry != nullptr && m_retryCountBeforeCommit == count)
            {
            if (BE_SQLITE_OK != m_savepointToCommitDuringRetry->Commit())
                {
                BeAssert(false && "Cannot commit write transaction in retry loop.");
                return 1;
                }
            }

        return 1;
        }

    void Reset() { m_actualRetryCount = 0; }
    void SetSavepointToCommitDuringRetry(Savepoint& savepoint, int retryCountBeforeCommit)
        {
        m_savepointToCommitDuringRetry = &savepoint;
        m_retryCountBeforeCommit = retryCountBeforeCommit;
        }
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, TwoConnectionsWithBusyRetryHandler)
    {
    BeFileName testECDbPath;
    {
    ECDbR ecdb = SetupECDb("one.ecdb");
    testECDbPath = BeFileName(ecdb.GetDbFileName());
    ecdb.CloseDb();
    }

    ECDb ecdb1;
    DbResult result = ecdb1.OpenBeSQLiteDb(testECDbPath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite, DefaultTxn::No));
    ASSERT_EQ(BE_SQLITE_OK, result);
    TestBusyRetry retry(3);
    retry.AddRef();
    ECDb ecdb2;
    result = ecdb2.OpenBeSQLiteDb(testECDbPath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite, DefaultTxn::No, &retry));
    ASSERT_EQ(BE_SQLITE_OK, result);

    {
    // make a change to the database from the first connection
    Savepoint s1(ecdb1, "ecdb1");
    ecdb1.CreateTable("TEST", "Col1 INTEGER");

    // try to make a change to the database from the second connection which should fail

    Savepoint s2(ecdb2, "ecdb2", false, BeSQLiteTxnMode::Immediate);
    result = s2.Begin();
    ASSERT_EQ(BE_SQLITE_BUSY, result);
    ASSERT_EQ(retry.m_retryCount, retry.m_actualRetryCount);
    }

    //at this point all locks should be cleared again.

    // now try to make a change to the database if the retry handler commits the open transaction on the first connection
    {
    // make a change to the database from the first connection
    Savepoint s1(ecdb1, "ecdb1");
    ecdb1.CreateTable("TEST2", "Col1 INTEGER");

    retry.Reset();
    Savepoint s2(ecdb2, "ecdb2", false, BeSQLiteTxnMode::Immediate);
    int retryAttemptsBeforeCommitting = 2;
    retry.SetSavepointToCommitDuringRetry(s1, 2);
    result = s2.Begin();
    ASSERT_EQ(BE_SQLITE_OK, result) << "Change on first conn gets committed during retry, so Savepoint.Begin should succeed";
    //after commit one more retry will be done (therefore + 1)
    ASSERT_EQ(retryAttemptsBeforeCommitting + 1, retry.m_actualRetryCount);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, GetAndChangeBriefcaseIdForDb)
    {
    ECDbR ecdb = SetupECDb("ecdbbriefcaseIdtest.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml"), 3);
    
    BeBriefcaseId id = ecdb.GetBriefcaseId();
    ASSERT_TRUE(id.IsValid());
    int32_t previousBriefcaseId = id.GetValue();
    BeBriefcaseId nextid = id.GetNextBriefcaseId();
    ASSERT_EQ(BE_SQLITE_OK, ecdb.ChangeBriefcaseId(nextid));
    int32_t changedBriefcaseId = ecdb.GetBriefcaseId().GetValue();
    ASSERT_NE(previousBriefcaseId, changedBriefcaseId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, GetAndChangeGUIDForDb)
    {
    ECDbR ecdb = SetupECDb("ecdbbriefcaseIdtest.ecdb", BeFileName(L"StartupCompany.02.00.ecschema.xml"), 3);

    BeGuid guid = ecdb.GetDbGuid();
    ASSERT_TRUE(guid.IsValid());

    BeGuid oldGuid = guid;
    guid.Create();
    if (guid.IsValid())
        ecdb.ChangeDbGuid(guid);
    BeGuid newGuid = ecdb.GetDbGuid();
    ASSERT_TRUE(newGuid.IsValid());
    ASSERT_TRUE(oldGuid != newGuid);

    guid.Invalidate();
    ASSERT_FALSE(guid.IsValid());
    }

END_ECDBUNITTESTS_NAMESPACE
