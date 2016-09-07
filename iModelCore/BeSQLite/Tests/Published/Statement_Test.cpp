/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/Statement_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
* Test to confirm ORDER BY semantics when NULL values are present.
* @bsimethod                                    Shaun.Sewall                    05/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(StatementTests, OrderBy)
    {
    initBeSQLiteLib();
    Db db;
    createDB(L"OrderBy.db", db);
    EXPECT_EQ(BE_SQLITE_OK, db.CreateTable("test_OrderBy", "[Id] INTEGER NOT NULL, [Label] TEXT COLLATE NOCASE, [Priority] INTEGER"));

    // Insert sample data that includes positive number, negative number, zero, and null
        {
        Statement statement;
        enum Column {Id=1, Label=2, Priority=3};
        EXPECT_EQ(BE_SQLITE_OK, statement.Prepare(db, "INSERT INTO test_OrderBy ([Id],[Label],[Priority]) VALUES (?,?,?)"));
    
        statement.BindInt(Column::Id, 1);
        statement.BindText(Column::Label, "Entry1", Statement::MakeCopy::No);
        statement.BindInt(Column::Priority, 100);
        EXPECT_EQ(BE_SQLITE_DONE, statement.Step());
        EXPECT_EQ(BE_SQLITE_OK, statement.Reset());

        statement.BindInt(Column::Id, 2);
        statement.BindText(Column::Label, "Entry2", Statement::MakeCopy::No);
        statement.BindInt(Column::Priority, -100);
        EXPECT_EQ(BE_SQLITE_DONE, statement.Step());
        EXPECT_EQ(BE_SQLITE_OK, statement.Reset());

        statement.BindInt(Column::Id, 3);
        statement.BindText(Column::Label, "Entry3", Statement::MakeCopy::No);
        statement.BindInt(Column::Priority, 0);
        EXPECT_EQ(BE_SQLITE_DONE, statement.Step());
        EXPECT_EQ(BE_SQLITE_OK, statement.Reset());

        statement.BindInt(Column::Id, 4);
        statement.BindText(Column::Label, "Entry4", Statement::MakeCopy::No);
        statement.BindNull(Column::Priority);
        EXPECT_EQ(BE_SQLITE_DONE, statement.Step());
        }

    // Verify ascending (which is the default) behavior
        {
        Statement statement;
        enum Column {Id=0, Priority=1};
        EXPECT_EQ(BE_SQLITE_OK, statement.Prepare(db, "SELECT [Id],[Priority] FROM test_OrderBy ORDER BY [Priority]"));

        EXPECT_EQ(BE_SQLITE_ROW, statement.Step());
        EXPECT_EQ(4, statement.GetValueInt(Column::Id));
        EXPECT_TRUE(statement.IsColumnNull(Column::Priority));

        EXPECT_EQ(BE_SQLITE_ROW, statement.Step());
        EXPECT_EQ(2, statement.GetValueInt(Column::Id));
        EXPECT_EQ(-100, statement.GetValueInt(Column::Priority));

        EXPECT_EQ(BE_SQLITE_ROW, statement.Step());
        EXPECT_EQ(3, statement.GetValueInt(Column::Id));
        EXPECT_EQ(0, statement.GetValueInt(Column::Priority));

        EXPECT_EQ(BE_SQLITE_ROW, statement.Step());
        EXPECT_EQ(1, statement.GetValueInt(Column::Id));
        EXPECT_EQ(100, statement.GetValueInt(Column::Priority));
        }

    // Verify descending behavior
        {
        Statement statement;
        enum Column {Id=0, Priority=1};
        EXPECT_EQ(BE_SQLITE_OK, statement.Prepare(db, "SELECT [Id],[Priority] FROM test_OrderBy ORDER BY [Priority] DESC"));

        EXPECT_EQ(BE_SQLITE_ROW, statement.Step());
        EXPECT_EQ(1, statement.GetValueInt(Column::Id));
        EXPECT_EQ(100, statement.GetValueInt(Column::Priority));

        EXPECT_EQ(BE_SQLITE_ROW, statement.Step());
        EXPECT_EQ(3, statement.GetValueInt(Column::Id));
        EXPECT_EQ(0, statement.GetValueInt(Column::Priority));

        EXPECT_EQ(BE_SQLITE_ROW, statement.Step());
        EXPECT_EQ(2, statement.GetValueInt(Column::Id));
        EXPECT_EQ(-100, statement.GetValueInt(Column::Priority));

        EXPECT_EQ(BE_SQLITE_ROW, statement.Step());
        EXPECT_EQ(4, statement.GetValueInt(Column::Id));
        EXPECT_TRUE(statement.IsColumnNull(Column::Priority));
        }
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

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 09/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST(StatementTests, InVirtualSet)
    {
    initBeSQLiteLib();
    Utf8String dbPath;
    {
    Db db;
    createDB(L"invirtualset.db", db);
    ASSERT_EQ(BE_SQLITE_OK, db.CreateTable("testVirtualSet", "Id INTEGER PRIMARY KEY, Name TEXT COLLATE NOCASE, Priority INTEGER"));
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db,"INSERT INTO testVirtualSet(Id,Name,Priority) VALUES(?,?,?)"));
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindUInt64(1, UINT64_C(1)));
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(2, "test1", Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindInt(3, 10));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
        
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindUInt64(1, UINT64_C(2)));
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(2, "test2", Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindInt(3, 20));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    db.SaveChanges();
    dbPath.assign(db.GetDbFileName());
    }

    Db db;
    ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(dbPath.c_str(), Db::OpenParams(Db::OpenMode::Readonly)));

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "SELECT count(*) FROM testVirtualSet WHERE InVirtualSet(?,Id)")) << db.GetLastError().c_str();
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "InVirtualSet without binding is expected to behave like binding an empty set.";
    ASSERT_EQ(0, stmt.GetValueInt(0)) << "InVirtualSet without binding is expected to behave like binding an empty set.";
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(BE_SQLITE_OK, stmt.BindNull(1)) << "InVirtualSet with binding NULL: " << db.GetLastError().c_str();
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "InVirtualSet with binding NULL is expected to behave like binding an empty set.";
    ASSERT_EQ(0, stmt.GetValueInt(0)) << "InVirtualSet with binding NULL is expected to behave like binding an empty set.";
    stmt.Reset();
    stmt.ClearBindings();

    IdSet<BeInt64Id> emptyIdset;
    ASSERT_EQ(BE_SQLITE_OK, stmt.BindVirtualSet(1, emptyIdset)) << "InVirtualSet with binding an empty virtual set: " << db.GetLastError().c_str();
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "InVirtualSet with binding an empty virtual set: " << db.GetLastError().c_str();
    ASSERT_EQ(0, stmt.GetValueInt(0)) << "InVirtualSet with binding an empty virtual set";
    }
