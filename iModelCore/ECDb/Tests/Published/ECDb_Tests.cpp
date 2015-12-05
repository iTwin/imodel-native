/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDb_Tests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "../BackDoor/PublicAPI/BackDoor/ECDb/ECDbTestProject.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECDbTests, CloseECDbWithFinalizedStatements)
    {
    Utf8String ecdbPath;
        {
        ECDbTestProject project;
        project.Create ("ecdbclosetests.ecdb", L"StartupCompany.02.00.ecschema.xml", true);
        ecdbPath = Utf8String (project.GetECDbPath ());
        }

        //Test with finalized BeSQLite statement
        {
        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbPath.c_str(), ECDb::OpenParams(ECDb::OpenMode::Readonly)));

        BeSQLite::Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(ecdb, "SELECT * FROM ec_Schema"));

        stmt.Finalize();
        ecdb.CloseDb();
        }

        //Test with finalized ECSqlStatement
        {
        ECDb ecdb;
        ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(ecdbPath.c_str(), ECDb::OpenParams(ECDb::OpenMode::Readonly)));

        ECSqlStatement stmt;
        ASSERT_TRUE(ECSqlStatus::Success == stmt.Prepare(ecdb, "SELECT FirstName FROM stco.Employee"));

        stmt.Finalize();
        ecdb.CloseDb();
        }

        //Testing to close the db with unfinalized statements is hard to do as the database remains open. We don't want that to happen as artefacts of the ATPs
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
    DbResult result = ecdb1.OpenBeSQLiteDb (testECDbPath, ECDb::OpenParams (ECDb::OpenMode::ReadWrite, DefaultTxn::No));
    ASSERT_EQ (BE_SQLITE_OK, result);
    result = ecdb2.OpenBeSQLiteDb (testECDbPath, ECDb::OpenParams (ECDb::OpenMode::ReadWrite, DefaultTxn::No));
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
    DbResult result = ecdb1.OpenBeSQLiteDb (testECDbPath, ECDb::OpenParams (ECDb::OpenMode::ReadWrite, DefaultTxn::No));
    ASSERT_EQ (BE_SQLITE_OK, result);
    TestBusyRetry retry (3);
    retry.AddRef ();
    ECDb ecdb2;
    result = ecdb2.OpenBeSQLiteDb (testECDbPath, ECDb::OpenParams (ECDb::OpenMode::ReadWrite, DefaultTxn::No, &retry));
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
TEST(ECDbTests, getAndChangeBriefcaseIdForDb)
    {
    ECDbTestProject testProject;
    ECDbR ecdbr = testProject.Create("EcdbBriefcaseIdTest.ecdb", L"StartupCompany.02.00.ecschema.xml", true);
    BeBriefcaseId id = ecdbr.GetBriefcaseId();
    ASSERT_TRUE(id.IsValid());
    int32_t previousBriefcaseId = id.GetValue();
    BeBriefcaseId nextid = id.GetNextBriefcaseId();
    ASSERT_EQ(BE_SQLITE_OK, ecdbr.ChangeBriefcaseId(nextid));
    int32_t changedBriefcaseId = ecdbr.GetBriefcaseId().GetValue();
    ASSERT_FALSE(previousBriefcaseId == changedBriefcaseId);
    }

TEST(ECDbTests, getAndChangeGUIDForDb)
    {
    ECDbTestProject testProject;
    ECDbR ecdbr = testProject.Create("EcdbBriefcaseIdTest.ecdb", L"StartupCompany.02.00.ecschema.xml", true);

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
    BeBriefcaseId repositotyId = ecdbr.GetBriefcaseId();
    ASSERT_TRUE(repositotyId.IsValid());
    ASSERT_TRUE(ecdbr.Schemas().GetECSchema("ECDb_System", true) != nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                  12/14
//+---------------+---------------+---------------+---------------+---------------+------
//this test might fail if changes made to the classes Furniture and Employee in the ECsql schema.
TEST(ECDbTests, checkPrimaryKeyForTablesIsBasedOnECInstanceId)
{
    Utf8CP sc_Furniture = "CREATE TABLE [sc_Furniture] ([ECInstanceId] INTEGER not null , [ECClassId] INTEGER not null , [AssetID] CHAR, [AssetOwner] CHAR, [BarCode] CHAR, [AssetUserID] CHAR, [Cost] DOUBLE, [Room] CHAR, [AssetRecordKey] CHAR, [Condition] INTEGER, [Material] INTEGER, [Weight] DOUBLE, [ChairFootPrint] BLOB, [Type] CHAR, [Color] CHAR, [DeskFootPrint] CHAR, [NumberOfCabinets] INTEGER, [Size] CHAR, [Length] DOUBLE, [Breadth] DOUBLE, [Employee__src_01_id] INTEGER, PRIMARY KEY  (ECInstanceId)) ";
    Utf8CP sc_Employee = "CREATE TABLE [sc_Employee] ([ECInstanceId] INTEGER not null , [EmployeeID] INTEGER, [FirstName] CHAR, [JobTitle] CHAR, [LastName] CHAR, [ManagerID] INTEGER, [Room] CHAR, [SSN] INTEGER, [Project] CHAR, [WorkPhone] BLOB, [MobilePhone] BLOB, [FullName] CHAR, Location_Coordinate_X DOUBLE, [Location_Coordinate_Y] DOUBLE, [Location_Coordinate_Z] DOUBLE, [Location_Street] CHAR, [Location_City] CHAR, [Location_State] CHAR, [Location_Country] CHAR, [Location_Zip] CHAR, [EmployeeType] INTEGER, [Address_Coordinate_X] DOUBLE, [Address_Coordinate_Y] DOUBLE, [Address_Coordinate_Z] DOUBLE, [Address_Street] CHAR, [Address_City] CHAR, [Address_State] CHAR, [Address_Country] CHAR, [Address_Zip] CHAR, [EmployeeRecordKey] TIMESTAMP, [Company__trg_11_id] INTEGER, [Phone__trg_01_id] INTEGER, PRIMARY KEY  (ECInstanceId)) ";
    ECDbTestProject testProject;
    ECDbR ecdbr = testProject.Create("EcdbBriefcaseIdTest.ecdb", L"StartupCompany.02.00.ecschema.xml", true);
    Statement sqlstmt0;
    ASSERT_EQ(BE_SQLITE_OK, sqlstmt0.Prepare(ecdbr, "SELECT sql FROM sqlite_master WHERE type='table' AND tbl_name='sc_Furniture'"));
    sqlstmt0.Step();
    Utf8CP sqlValue0 = sqlstmt0.GetValueText(0);
    ASSERT_TRUE(strcmp(sc_Furniture, sqlValue0) > 0);

    Statement sqlstmt;
    ASSERT_EQ(BE_SQLITE_OK, sqlstmt.Prepare(ecdbr, "SELECT sql FROM sqlite_master WHERE type='table' AND tbl_name='sc_Employee'"));
    sqlstmt.Step();
    Utf8CP sqlValue = sqlstmt.GetValueText(0);
    ASSERT_TRUE(strcmp(sc_Employee, sqlValue) > 0);
}
    
END_ECDBUNITTESTS_NAMESPACE
