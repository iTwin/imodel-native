/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/Published/ECDb_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
void AssertCloseDb (ECDbR ecdb, bool expectedSuccess)
    {
    void* sqliteDbHandle = Backdoor::ECDb::GetSqliteDb (ecdb);

    //CloseDb always returns OK even if the DB could not be closed properly
    ecdb.CloseDb();

    //So attempting to close via SQLite directly
    const auto actualStat = BeSQLiteLib::CloseSqlDb (sqliteDbHandle);
    if (expectedSuccess)
        {
        //in this case CloseDb is expected to have succeeded and another call to CloseDb is expected to fail
        ASSERT_EQ (BE_SQLITE_MISUSE, actualStat) << "Closing the ECDb file is expected to succeed.";
        }
    else
        {
        //in this case CloseDb is expected to have stayed open and a new attempt to close it should return the same error.
        ASSERT_EQ (BE_SQLITE_BUSY, actualStat) << "Closing the ECDb file is expected to fail because of unfinalized statements.";
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbTests, CloseECDbWithUnfinalizedStatements)
    {
    Utf8String ecdbPath;
        {
        ECDbTestProject project;
        project.Create ("ecdbclosetests.ecdb", L"StartupCompany.02.00.ecschema.xml", true);
        ecdbPath = Utf8String (project.GetECDbPath ());
        }

    BeTest::SetFailOnAssert (false);
        //Test with unfinalized BeSQLite statement
        {
        ECDb ecdb;
        ASSERT_EQ (BE_SQLITE_OK, ecdb.OpenBeSQLiteDb (ecdbPath.c_str (), ECDb::OpenParams (ECDb::OPEN_Readonly)));

        BeSQLite::Statement stmt;
        ASSERT_EQ (BE_SQLITE_OK, stmt.Prepare (ecdb, "SELECT * FROM ec_Schema"));

        AssertCloseDb (ecdb, false);
        }

        //Test with finalized BeSQLite statement
        {
        ECDb ecdb;
        ASSERT_EQ (BE_SQLITE_OK, ecdb.OpenBeSQLiteDb (ecdbPath.c_str (), ECDb::OpenParams (ECDb::OPEN_Readonly)));

        BeSQLite::Statement stmt;
        ASSERT_EQ (BE_SQLITE_OK, stmt.Prepare (ecdb, "SELECT * FROM ec_Schema"));

        stmt.Finalize ();
        AssertCloseDb (ecdb, true);
        }

        //Test with unfinalized ECSqlStatement
        {
        ECDb ecdb;
        ASSERT_EQ (BE_SQLITE_OK, ecdb.OpenBeSQLiteDb (ecdbPath.c_str (), ECDb::OpenParams (ECDb::OPEN_Readonly)));

        ECSqlStatement stmt;
        ASSERT_TRUE (ECSqlStatus::Success == stmt.Prepare (ecdb, "SELECT FirstName FROM stco.Employee"));

        AssertCloseDb (ecdb, false);
        }

        //Test with finalized ECSqlStatement
        {
        ECDb ecdb;
        ASSERT_EQ (BE_SQLITE_OK, ecdb.OpenBeSQLiteDb (ecdbPath.c_str (), ECDb::OpenParams (ECDb::OPEN_Readonly)));

        ECSqlStatement stmt;
        ASSERT_TRUE (ECSqlStatus::Success == stmt.Prepare (ecdb, "SELECT FirstName FROM stco.Employee"));

        stmt.Finalize ();
        AssertCloseDb (ecdb, true);
        }

    BeTest::SetFailOnAssert (true);
    }


//=======================================================================================
// @bsiclass                                     Krischan.Eberle                  01/15
//=======================================================================================
struct TestOtherConnectionECDb : ECDb
    {
    int m_changeCount;
    TestOtherConnectionECDb ()
        : m_changeCount (0)
        {}

    virtual void _OnDbChangedByOtherConnection () override
        {
        m_changeCount++; 
        ECDb::_OnDbChangedByOtherConnection ();
        }
    };


//---------------------------------------------------------------------------------------
// Test to ensure that PRAGMA data_version changes when another connection changes a database
// @bsiclass                                     Krischan.Eberle                  01/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbTests, TwoConnections)
    {
    BeFileName testECDbPath;
        {
        ECDbTestProject testProject;
        ECDbR ecdb = testProject.Create ("one.ecdb");
        testECDbPath = BeFileName (ecdb.GetDbFileName ());
        }

    TestOtherConnectionECDb ecdb1, ecdb2;
    DbResult result = ecdb1.OpenBeSQLiteDb (testECDbPath, ECDb::OpenParams (ECDb::OPEN_ReadWrite, DefaultTxn_No));
    ASSERT_EQ (BE_SQLITE_OK, result);
    result = ecdb2.OpenBeSQLiteDb (testECDbPath, ECDb::OpenParams (ECDb::OPEN_ReadWrite, DefaultTxn_No));
    ASSERT_EQ (BE_SQLITE_OK, result);

        { // make a change to the database from the first connection
        Savepoint t1 (ecdb1, "tx1");
        // the first transaction should not call _OnDbChangedByOtherConnection
        ASSERT_EQ (0, ecdb1.m_changeCount);
        ecdb1.CreateTable ("TEST", "Col1 INTEGER");
        }

        { // make a change to the database from the second connection
        Savepoint t2 (ecdb2, "tx2");
        // the first transaction on the second connection should not call _OnDbChangedByOtherConnection
        ASSERT_EQ (0, ecdb2.m_changeCount);
        ecdb2.ExecuteSql ("INSERT INTO TEST(Col1) VALUES(3)");
        }

        { // start another transaction on the first connection. This should notice that the second connection changed the db.
        Savepoint t3 (ecdb1, "tx1");
        ASSERT_EQ (1, ecdb1.m_changeCount);
        ecdb1.ExecuteSql ("INSERT INTO TEST(Col1) VALUES(4)");
        }

        { // additional changes from the same connnection should not trigger additional calls to _OnDbChangedByOtherConnection
        Savepoint t3 (ecdb1, "tx1");
        ASSERT_EQ (1, ecdb1.m_changeCount);
        ecdb1.ExecuteSql ("INSERT INTO TEST(Col1) VALUES(5)");
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


    explicit TestBusyRetry (int retryCount)
        : m_retryCount (retryCount), m_actualRetryCount (0), m_savepointToCommitDuringRetry (nullptr), m_retryCountBeforeCommit (-1)
        {}

    virtual int _OnBusy (int count) const override
        {
        //count is 0-based
        if (count >= m_retryCount)
            return 0;

        m_actualRetryCount = count + 1;

        if (m_savepointToCommitDuringRetry != nullptr && m_retryCountBeforeCommit == count)
            {
            if (BE_SQLITE_OK != m_savepointToCommitDuringRetry->Commit ())
                {
                BeAssert (false && "Cannot commit write transaction in retry loop.");
                return 1;
                }
            }

        return 1;
        }

    void Reset () {m_actualRetryCount = 0;}
    void SetSavepointToCommitDuringRetry (Savepoint& savepoint, int retryCountBeforeCommit)
        {
        m_savepointToCommitDuringRetry = &savepoint;
        m_retryCountBeforeCommit = retryCountBeforeCommit;
        }
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbTests, TwoConnectionsWithBusyRetryHandler)
    {
    BeFileName testECDbPath;
        {
        ECDbTestProject testProject;
        ECDbR ecdb = testProject.Create ("one.ecdb");
        testECDbPath = BeFileName (ecdb.GetDbFileName ());
        }

    ECDb ecdb1;
    DbResult result = ecdb1.OpenBeSQLiteDb (testECDbPath, ECDb::OpenParams (ECDb::OPEN_ReadWrite, DefaultTxn_No));
    ASSERT_EQ (BE_SQLITE_OK, result);
    TestBusyRetry retry (3);
    retry.AddRef ();
    ECDb ecdb2;
    result = ecdb2.OpenBeSQLiteDb (testECDbPath, ECDb::OpenParams (ECDb::OPEN_ReadWrite, DefaultTxn_No, &retry));
    ASSERT_EQ (BE_SQLITE_OK, result);

        {
        // make a change to the database from the first connection
        Savepoint s1 (ecdb1, "ecdb1");
        ecdb1.CreateTable ("TEST", "Col1 INTEGER");

        // try to make a change to the database from the second connection which should fail
        
        Savepoint s2 (ecdb2, "ecdb2", false, BeSQLiteTxnMode::Immediate);
        result = s2.Begin ();
        ASSERT_EQ (BE_SQLITE_BUSY, result);
        ASSERT_EQ (retry.m_retryCount, retry.m_actualRetryCount);
        }

    //at this point all locks should be cleared again.

    // now try to make a change to the database if the retry handler commits the open transaction on the first connection
        {
        // make a change to the database from the first connection
        Savepoint s1 (ecdb1, "ecdb1");
        ecdb1.CreateTable ("TEST2", "Col1 INTEGER");

        retry.Reset ();
        Savepoint s2 (ecdb2, "ecdb2", false, BeSQLiteTxnMode::Immediate);
        int retryAttemptsBeforeCommitting = 2;
        retry.SetSavepointToCommitDuringRetry (s1, 2);
        result = s2.Begin ();
        ASSERT_EQ (BE_SQLITE_OK, result) << "Change on first conn gets committed during retry, so Savepoint.Begin should succeed";
        //after commit one more retry will be done (therefore + 1)
        ASSERT_EQ (retryAttemptsBeforeCommitting + 1, retry.m_actualRetryCount);
        }
    }



//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbTests, getAndChangeRepositoryIdForDb)
    {
    ECDbTestProject testProject;
    ECDbR ecdbr = testProject.Create("EcdbRepositoryIdTest.ecdb", L"StartupCompany.02.00.ecschema.xml", true);
    BeRepositoryId id = ecdbr.GetRepositoryId();
    ASSERT_TRUE(id.IsValid());
    int32_t previousRepositoryId = id.GetValue();
    BeRepositoryId nextid = id.GetNextRepositoryId();
    ASSERT_EQ(BE_SQLITE_OK, ecdbr.ChangeRepositoryId(nextid));
    int32_t changedRepositoryId = ecdbr.GetRepositoryId().GetValue();
    ASSERT_FALSE(previousRepositoryId == changedRepositoryId);
    }

TEST(ECDbTests, getAndChangeGUIDForDb)
    {
    ECDbTestProject testProject;
    ECDbR ecdbr = testProject.Create("EcdbRepositoryIdTest.ecdb", L"StartupCompany.02.00.ecschema.xml", true);

    BeGuid guid = ecdbr.GetDbGuid();
    ASSERT_TRUE(guid.IsValid());

    BeGuid oldGuid = guid;
    guid.Create();
    if (guid.IsValid())
        ecdbr.ChangeDbGuid(guid);
    BeGuid newGuid = ecdbr.GetDbGuid();
    ASSERT_TRUE(newGuid.IsValid());
    ASSERT_TRUE(oldGuid != newGuid);

    guid.Invalidate();
    ASSERT_FALSE(guid.IsValid());
    }

TEST(ECDbTests, createEmptyProject)
    {
    ECDbTestProject testproject;
    ECDbR ecdbr = testproject.Create("emptyecdb.ecdb");
    BeRepositoryId repositotyId = ecdbr.GetRepositoryId();
    ASSERT_TRUE(repositotyId.IsValid());
    ECSchemaP ecschema;
    ASSERT_EQ(SUCCESS, ecdbr.GetEC().GetSchemaManager().GetECSchema(ecschema, "ECDbSystem", true));
    }

END_ECDBUNITTESTS_NAMESPACE
