/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/MultiFileCommit_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BeSQLitePublishedTests.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                   09/14
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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                   09/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void openDB (WCharCP dbName, Db& db, Db::OpenMode openMode)
    {
    BeFileName dbFileName;
    BeTest::GetHost().GetOutputRoot (dbFileName);
    dbFileName.AppendToPath (dbName);

    Db::OpenParams params (openMode);
    DbResult result = db.OpenBeSQLiteDb (dbFileName.GetNameUtf8().c_str(), params);
    ASSERT_EQ (BE_SQLITE_OK, result) << "Db Creation failed";
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
* @bsimethod                                    Sam.Wilson                   09/14
+---------------+---------------+---------------+---------------+---------------+------*/
//TEST(MultiFileCommit, Test1)
//    {
//    initBeSQLiteLib();
//
//    Db db1, db2;
//    createDB (L"db1.db", db1);
//
//    createDB (L"db2.db", db2);
//    Utf8String db2FileName = db2.GetDbFileName();
//    db2.CloseDb();
//
//    ASSERT_EQ (BE_SQLITE_OK, db1.AttachDb (db2FileName.c_str(), "DB2") );
//
//    openDB (L"db2.db", db2, Db::OPEN_ReadWrite);
//
//    ASSERT_EQ (BE_SQLITE_OK, db1.CreateTable ("Table1", "COL11 NUMERIC, COL12 TEXT"));
//    ASSERT_EQ (BE_SQLITE_OK, db1.ExecuteSql ("INSERT INTO Table1 (COL11, COL12) values (10, 'ONE')"));
//
//    ASSERT_EQ (BE_SQLITE_OK, db2.CreateTable ("Table2", "COL21 NUMERIC, COL22 TEXT"));
//    ASSERT_EQ (BE_SQLITE_OK, db2.ExecuteSql ("INSERT INTO Table2 (COL21, COL22) values (20, 'TWO')"));
//
//    db2.CloseDb();
//
//    ASSERT_EQ (BE_SQLITE_OK, db1.SaveChanges());
//
//    db2.AbandonChanges(); // should be nop
//    db1.AbandonChanges(); // should be nop
//
//    db1.CloseDb();
//    db2.CloseDb();
//
//    openDB (L"db1.db", db1, Db::OPEN_Readonly);
//    openDB (L"db2.db", db2, Db::OPEN_Readonly);
//
//    Statement stmt1;
//    stmt1.Prepare (db1, "SELECT * FROM Table1 WHERE (COL11=10)");
//    ASSERT_EQ (BE_SQLITE_ROW, stmt1.Step());
//
//    Statement stmt2;
//    stmt1.Prepare (db2, "SELECT * FROM Table2 WHERE (COL21=20)");
//    ASSERT_EQ (BE_SQLITE_ROW, stmt2.Step());
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                   09/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MultiFileCommit, Test2)
    {
    initBeSQLiteLib();

    Db db1;
    
    //  Create both dbs
    Utf8String db2FileName;
    if (true)
        {
        Db db2;
        createDB (L"db1.db", db1);
        createDB (L"db2.db", db2);

        db2FileName = db2.GetDbFileName();

        ASSERT_EQ (BE_SQLITE_OK, db1.CreateTable ("Table1", "COL11 NUMERIC, COL12 TEXT"));
        ASSERT_EQ (BE_SQLITE_OK, db2.CreateTable ("Table2", "COL21 NUMERIC, COL22 TEXT"));

        db1.CloseDb();
        db2.CloseDb();
        }

    //  Open db1 
    if (true)
        {
        openDB (L"db1.db", db1, Db::OpenMode::ReadWrite);
    
        //      Attach db2 to it
        ASSERT_EQ (BE_SQLITE_OK, db1.AttachDb (db2FileName.c_str(), "DB2") );

        //      Write to both db1 and db2 through db1
        ASSERT_EQ (BE_SQLITE_OK, db1.ExecuteSql ("INSERT INTO Table1 (COL11, COL12) values (10, 'ONE')"));

        ASSERT_EQ (BE_SQLITE_OK, db1.ExecuteSql ("INSERT INTO DB2.Table2 (COL21, COL22) values (20, 'TWO')"));

        ASSERT_EQ (BE_SQLITE_OK, db1.SaveChanges());

        db1.CloseDb();
        }

    //  Open db1 again
    if (true)
        {
        openDB (L"db1.db", db1, Db::OpenMode::Readonly);

        //      Attach db2 to it
        ASSERT_EQ (BE_SQLITE_OK, db1.AttachDb (db2FileName.c_str(), "DB2") );

        //      Access both db1 and db2 through db1
        Statement stmt1;
        stmt1.Prepare (db1, "SELECT * FROM Table1 WHERE (COL11=10)");
        ASSERT_EQ (BE_SQLITE_ROW, stmt1.Step());

        Statement stmt2;
        stmt2.Prepare (db1, "SELECT * FROM DB2.Table2 WHERE (COL21=20)");
        ASSERT_EQ (BE_SQLITE_ROW, stmt2.Step());
        }
    }