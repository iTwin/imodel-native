/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/Published/ECDb_Tests.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbTests, getAndChangeRepositoryIdForDb)
    {
    ECDbTestProject testProject;
    ECDbR ecdbr = testProject.Create("EcdbRepositoryIdTest.ecdb", L"StartupCompany.02.00.ecschema.xml", true);
    BeRepositoryId id = ecdbr.GetRepositoryId();
    ASSERT_TRUE(id.IsValid());
    Int32 previousRepositoryId = id.GetValue();
    BeRepositoryId nextid = id.GetNextRepositoryId();
    ASSERT_EQ(BE_SQLITE_OK, ecdbr.ChangeRepositoryId(nextid));
    Int32 changedRepositoryId = ecdbr.GetRepositoryId().GetValue();
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
