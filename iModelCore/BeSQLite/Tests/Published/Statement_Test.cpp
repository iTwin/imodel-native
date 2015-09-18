/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/Statement_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BeSQLitePublishedTests.h"

/*---------------------------------------------------------------------------------**//**
* Creating a new Db for the test
* @bsimethod                                    Majd.Uddin                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void createDB (WCharCP dbName, Db& db)
    {
    BeFileName dbFileName;
    BeTest::GetHost().GetOutputRoot (dbFileName);
    dbFileName.AppendToPath (dbName);
    if (BeFileName::DoesPathExist (dbFileName))
        BeFileName::BeDeleteFile (dbFileName);

    DbResult result = db.CreateNewDb (dbFileName.GetNameUtf8().c_str());
    ASSERT_EQ (BE_SQLITE_OK, result) << "Db Creation failed";

    //add some data
    EXPECT_EQ (BE_SQLITE_OK, db.CreateTable ("linestyles", "lsId NUMERIC, lsName TEXT"));
    ASSERT_EQ (BE_SQLITE_OK, db.ExecuteSql ("INSERT INTO linestyles (lsId, lsName) values (10, 'ARROW')"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void initBeSQLiteLib()
    {
    BeFileName temporaryDir;
    BeTest::GetHost ().GetOutputRoot (temporaryDir);
    BeSQLiteLib::Initialize (temporaryDir);
    }

/*---------------------------------------------------------------------------------**//**
* Building and running a statement
* @bsimethod                                    Majd.Uddin                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(StatementTests, NewStatement)
    {
    initBeSQLiteLib();

    Statement stat1;

    //initially it should not be prepared
    EXPECT_FALSE( stat1.IsPrepared());

    //create a DB
    Db db;
    createDB (L"stat.db", db);

    DbResult result = stat1.Prepare (db, "SELECT * FROM linestyles");
    EXPECT_EQ ( result, BE_SQLITE_OK);
    EXPECT_TRUE (stat1.IsPrepared());

    //now run this statement
    stat1.DumpResults(); //It dumps the SQL and not the results?
    EXPECT_EQ (BE_SQLITE_ROW, stat1.Step());
    //Verify data
    EXPECT_EQ (2, stat1.GetColumnCount()); //there are 2 columns in linestyles table
    EXPECT_EQ (DbValueType::IntegerVal, stat1.GetColumnType(0));
    EXPECT_EQ (DbValueType::TextVal, stat1.GetColumnType(1));
    EXPECT_TRUE (stat1.IsColumnNull(2));
    EXPECT_FALSE (stat1.IsColumnNull(1));
    EXPECT_STREQ ("lsId", stat1.GetColumnName(0));
    EXPECT_STREQ ("lsName", stat1.GetColumnName(1));
    EXPECT_STREQ ("SELECT * FROM linestyles", stat1.GetSql());
    EXPECT_STREQ ("ARROW", stat1.GetValueText(1));
    WString value;
    value.AssignUtf8 (stat1.GetValueText(1));
    EXPECT_STREQ (L"ARROW", value.c_str());

    EXPECT_EQ (10, stat1.GetValueInt(0));
    EXPECT_EQ (10, stat1.GetValueInt64(0));
    EXPECT_EQ (10.0, stat1.GetValueDouble(0));

    stat1.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* Try invlaid values for statement
* @bsimethod                                    Majd.Uddin                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(StatementTests, BadDb)
    {
    initBeSQLiteLib();
    Statement stat1;
    //create a DB
    Db db;
    createDB ( L"stat.db", db);
    db.CloseDb();

    //It crashes here. Issue is reported
    //DbResult result = stat1.TryPrepare (db, "SELECT * linestyles");
    stat1.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* Try invlaid values for statement
* @bsimethod                                    Majd.Uddin                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(StatementTests, BadSQL)
    {
    initBeSQLiteLib();
    Statement stat1;
    //create a DB
    Db db;
    createDB ( L"stat.db", db);

    //For an incorrect SQL, it should return an error
    DbResult result = stat1.TryPrepare (db, "SELECT * FakeTableName");
    EXPECT_EQ (BE_SQLITE_ERROR, result);

    stat1.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* Statement reset
* @bsimethod                                    Majd.Uddin                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(StatementTests, ResetStatement)
    {
    initBeSQLiteLib();
    Statement stat1;
    EXPECT_FALSE( stat1.IsPrepared());

    //create a DB
    Db db;
    createDB ( L"reset.db", db);
    DbResult result = stat1.Prepare (db, "SELECT * FROM linestyles");
    EXPECT_EQ ( result, BE_SQLITE_OK);
    EXPECT_TRUE (stat1.IsPrepared());

    EXPECT_EQ (BE_SQLITE_ROW, stat1.Step());
    EXPECT_EQ (BE_SQLITE_DONE, stat1.Step());
    //This is where it is not sure if it should work as there is only one row. Issue reported
    EXPECT_EQ (BE_SQLITE_ROW, stat1.Step());
    EXPECT_EQ (BE_SQLITE_DONE, stat1.Step());

    //now reset this
    EXPECT_EQ (BE_SQLITE_OK, stat1.Reset());
    EXPECT_EQ (BE_SQLITE_ROW, stat1.Step());
    EXPECT_EQ (BE_SQLITE_DONE, stat1.Step());
    EXPECT_EQ (BE_SQLITE_ROW, stat1.Step());
    EXPECT_EQ (BE_SQLITE_DONE, stat1.Step());
    stat1.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* Bind parameters for SELECT and INSERT
* @bsimethod                                    Majd.Uddin                   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(StatementTests, BindToStatement)
    {
    initBeSQLiteLib();
    Statement stat1, stat2, stat3;
    EXPECT_FALSE( stat1.IsPrepared());

    Db db;
    createDB ( L"bind.db", db);
    DbResult result = stat1.Prepare (db, "SELECT * FROM linestyles WHERE lsId = ?");
    EXPECT_EQ ( result, BE_SQLITE_OK);
    EXPECT_TRUE (stat1.IsPrepared());

    result = stat1.BindInt (1, 10);
    EXPECT_EQ (result, BE_SQLITE_OK) << " The result is: " << result;
    EXPECT_EQ (BE_SQLITE_ROW, stat1.Step());
    EXPECT_STREQ ("ARROW", stat1.GetValueText(1));

    //INSERT command with Bind values
    result = stat2.Prepare (db, "INSERT INTO linestyles (lsId, lsName) values (?, ?)");
    
    result = stat2.BindInt (1, 20);
    EXPECT_EQ (result, BE_SQLITE_OK) << " The result is: " << result;
    result = stat2.BindText (2, "BLOCK", Statement::MakeCopy::No);
    EXPECT_EQ (result, BE_SQLITE_OK) << " The result is: " << result;
    //Execute command and verify data
    EXPECT_EQ (BE_SQLITE_DONE, stat2.Step());

    result = stat3.Prepare (db, "SELECT * FROM linestyles WHERE lsId = '20'");
    EXPECT_EQ ( BE_SQLITE_ROW, stat3.Step());
    EXPECT_STREQ ("BLOCK", stat3.GetValueText(1));

    stat1.Finalize();
    stat2.Finalize();
    stat3.Finalize();
    }
/*---------------------------------------------------------------------------------**//**
* Clearing Bindings for statement
* @bsimethod                                    Majd.Uddin                   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(StatementTests, ClearBinding)
    {
    initBeSQLiteLib();
    Statement stat1;
    EXPECT_FALSE( stat1.IsPrepared());

    Db db;
    createDB ( L"bind2.db", db);
    DbResult result = stat1.Prepare (db, "SELECT * FROM linestyles WHERE lsId = ?");
    EXPECT_EQ ( result, BE_SQLITE_OK);
    EXPECT_TRUE (stat1.IsPrepared());

    result = stat1.BindInt (1, 10);
    EXPECT_EQ (result, BE_SQLITE_OK) << " The result is: " << result;
    EXPECT_EQ (BE_SQLITE_ROW, stat1.Step());
    EXPECT_STREQ ("ARROW", stat1.GetValueText(1));

    //Now Clear this binding and run statement again.
    EXPECT_EQ (BE_SQLITE_OK, stat1.ClearBindings());
    //This does not return OK, rather returns BE_SQLITE_MISUSE ??
    stat1.BindInt (1, 20);
    EXPECT_EQ (BE_SQLITE_DONE, stat1.Step());
    EXPECT_EQ (NULL, stat1.GetValueText(1)); //There should be no value

    stat1.Finalize();
    }

/*---------------------------------------------------------------------------------**//**
* Provisional test to ensure sql json functions automatically made available when
* a new DB connection is created, and superficially test a couple of them.
* NEEDSWORK: More comprehensive tests of the json stuff.
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(StatementTests, VerifyJsonExtensionEnabled)
    {
    initBeSQLiteLib();
    Db db;
    createDB(L"json.db", db);
    EXPECT_EQ (BE_SQLITE_OK, db.ExecuteSql("INSERT INTO linestyles (lsId, lsName) values (20, json('{\"X\":123}'))"));

    Statement stmt;
    EXPECT_EQ (BE_SQLITE_OK, stmt.Prepare(db, "SELECT count(*) FROM linestyles WHERE 1 = json_valid(lsName)"));
    EXPECT_EQ (BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ (1, stmt.GetValueInt(0));
    }

