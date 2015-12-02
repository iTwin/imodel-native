/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/BeSQLiteDb_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BeSQLitePublishedTests.h"

#include <vector>

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing Db
* @bsimethod                                    Majd.Uddin                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct BeSQLiteDbTests : public ::testing::Test
    {
    public:
        Db              m_db;
        DbResult        m_result;

        static DbResult SetupDb(Db& db, WCharCP dbName);
        void SetupDb(WCharCP dbName);
        static BeFileName getDbFilePath(WCharCP dbName);

    };

/*---------------------------------------------------------------------------------**//**
* Creating a new Db for the test
* @bsimethod                                    Majd.Uddin                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName BeSQLiteDbTests::getDbFilePath(WCharCP dbName)
    {
    BeFileName dbFileName;
    BeTest::GetHost().GetOutputRoot(dbFileName);
    dbFileName.AppendToPath(dbName);
    return dbFileName;
    }

//---------------------------------------------------------------------------------------
// Creating a new Db for the test
// @bsimethod                                    Krischan.Eberle                   12/12
//+---------------+---------------+---------------+---------------+---------------+------
DbResult BeSQLiteDbTests::SetupDb(Db& db, WCharCP dbName)
    {
    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);
    BeSQLiteLib::Initialize(temporaryDir);

    BeFileName dbFullName = getDbFilePath(dbName);
    if (BeFileName::DoesPathExist(dbFullName))
        BeFileName::BeDeleteFile(dbFullName);
    DbResult result = db.CreateNewDb(dbFullName.GetNameUtf8().c_str());
    EXPECT_EQ (BE_SQLITE_OK, result) << "Db Creation failed";
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* Creating a new Db for the test
* @bsimethod                                    Majd.Uddin                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
void BeSQLiteDbTests::SetupDb(WCharCP dbName)
    {
    m_result = SetupDb(m_db, dbName);
    ASSERT_EQ (BE_SQLITE_OK, m_result) << "Db Creation failed";
    }

/*---------------------------------------------------------------------------------**//**
* Creating a new Db
* @bsimethod                                    Majd.Uddin                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, CreateNewDb)
    {
    SetupDb(L"blank");
    EXPECT_FALSE (m_db.IsReadonly());
    EXPECT_TRUE (m_db.IsDbOpen());

    //Verifying it's name
    Utf8CP dbName = m_db.GetDbFileName();
    WString strName(dbName, true);
    WString shortName = strName.substr(strName.length() - 5, strName.length());
    EXPECT_STREQ (L"blank", shortName.c_str()) << L"The returned DbFileName is not correct. it is: " << shortName.c_str();
    m_db.CloseDb();
    EXPECT_FALSE (m_db.IsDbOpen());
    }

/*---------------------------------------------------------------------------------**//**
* Opening an existing Db
* @bsimethod                                    Majd.Uddin                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, OpenDb)
    {
    SetupDb(L"one.db");
    EXPECT_TRUE (m_db.IsDbOpen());

    //now close and re-open it
    m_db.CloseDb();
    EXPECT_FALSE (m_db.IsDbOpen());
    m_result = m_db.OpenBeSQLiteDb(getDbFilePath(L"one.db"), Db::OpenParams(Db::OpenMode::Readonly, DefaultTxn::Yes));
    EXPECT_EQ (BE_SQLITE_OK, m_result);
    EXPECT_TRUE (m_db.IsDbOpen());
    }


//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/14
//=======================================================================================
struct TestOtherConnectionDb : Db
    {
    int m_changeCount;
    TestOtherConnectionDb() {m_changeCount=0;}
    virtual void _OnDbChangedByOtherConnection() override {++m_changeCount; Db::_OnDbChangedByOtherConnection();}
    };

/*---------------------------------------------------------------------------------**//**
* Test to ensure that PRAGMA data_version changes when another connection changes a database
* @bsimethod                                    Keith.Bentley                   12/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, TwoConnections)
    {
    SetupDb(L"one.db");
    EXPECT_TRUE (m_db.IsDbOpen());
    m_db.CloseDb();

    TestOtherConnectionDb db1, db2;
    DbResult result = db1.OpenBeSQLiteDb(getDbFilePath(L"one.db"), Db::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::No));

    EXPECT_EQ (BE_SQLITE_OK, result);
    EXPECT_TRUE (db1.IsDbOpen());
    
    result = db2.OpenBeSQLiteDb(getDbFilePath(L"one.db"), Db::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::No));
    EXPECT_EQ (BE_SQLITE_OK, result);
    EXPECT_TRUE (db2.IsDbOpen());

    { // make a change to the database from the first connection
    Savepoint t1(db1, "tx1");
    // the first transaction should not call _OnDbChangedByOtherConnection
    EXPECT_EQ (0, db1.m_changeCount);
    db1.CreateTable("TEST", "Col1 INTEGER");
    }

    { // make a change to the database from the second connection
    Savepoint t2(db2, "tx2");
    // the first transaction on the second connection should not call _OnDbChangedByOtherConnection
    EXPECT_EQ (0, db2.m_changeCount);
    db2.ExecuteSql("INSERT INTO TEST(Col1) VALUES(3)");
    }

    { // start another transaction on the first connection. This should notice that the second connection changed the db.
    Savepoint t3(db1, "tx1");
    EXPECT_EQ (1, db1.m_changeCount);
    db1.ExecuteSql("INSERT INTO TEST(Col1) VALUES(4)");
    }

    { // additional changes from the same connnection should not trigger additional calls to _OnDbChangedByOtherConnection
    Savepoint t3(db1, "tx1");
    EXPECT_EQ (1, db1.m_changeCount);
    db1.ExecuteSql("INSERT INTO TEST(Col1) VALUES(5)");
    }

    }

struct TestBusyRetry : BusyRetry
    {
    int m_maxCount;
    mutable int m_onBusyCalls;

    TestBusyRetry() : m_maxCount(2), m_onBusyCalls(0) {}
    void Reset() {m_onBusyCalls = 0;}

    virtual int _OnBusy(int count) const
        {
        m_onBusyCalls++;
        if (count >= m_maxCount)
            return 0;

        return 1;
        }
    };


TEST_F(BeSQLiteDbTests, Concurrency_UsingSavepoints)
    {
    SetupDb(L"one.db");
    m_db.CloseDb();
    BeFileName dbname = getDbFilePath(L"one.db");

    // TEST 2 CONNECTIONS
    TestBusyRetry retry1;
    TestBusyRetry retry2;
    retry1.AddRef();
    retry2.AddRef();

    Db::OpenParams openParams1(Db::OpenMode::ReadWrite, DefaultTxn::No, &retry1);
    Db::OpenParams openParams2(Db::OpenMode::ReadWrite, DefaultTxn::No, &retry2);

    Db db1, db2;
    DbResult result = db1.OpenBeSQLiteDb(dbname, openParams1);
    EXPECT_EQ (BE_SQLITE_OK, result);
    result = db2.OpenBeSQLiteDb(dbname, openParams2);
    EXPECT_EQ (BE_SQLITE_OK, result);

    {
    Savepoint sp1(db1, "DB1", false);
    Savepoint sp2(db2, "DB2", false, BeSQLiteTxnMode::Immediate);

    result = sp1.Begin();
    EXPECT_EQ (BE_SQLITE_OK, result);
    result = sp2.Begin();
    EXPECT_EQ (BE_SQLITE_OK, result); 

    result = db2.SavePropertyString(PropertySpec("Foo", "DB2"), "Test2");
    EXPECT_EQ (BE_SQLITE_OK, result);

    result = sp2.Commit();
    EXPECT_EQ (BE_SQLITE_BUSY, result);
    EXPECT_EQ (3, retry2.m_onBusyCalls);

    result = sp1.Commit();
    EXPECT_EQ (BE_SQLITE_OK, result);

    result = sp2.Commit();
    EXPECT_EQ (BE_SQLITE_OK, result);
    }

    {
    retry2.Reset();
    Savepoint sp1(db1, "DB1", false, BeSQLiteTxnMode::Immediate);
    Savepoint sp2(db2, "DB2", false, BeSQLiteTxnMode::Immediate);

    result = sp1.Begin();
    EXPECT_EQ (BE_SQLITE_OK, result);
    result = sp2.Begin();
    EXPECT_EQ (BE_SQLITE_BUSY, result); 
    EXPECT_EQ (3, retry2.m_onBusyCalls);
    }
}

/*---------------------------------------------------------------------------------**//**
* Setting ProjectGuid
* @bsimethod                                    Majd.Uddin                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, SetGuids)
    {
    SetupDb(L"SetGuids.db");

    EXPECT_FALSE (m_db.QueryProjectGuid().IsValid());
    BeGuid projGuid;
    m_db.SaveProjectGuid(projGuid);
    BeGuid projGuidOut = m_db.QueryProjectGuid();

    EXPECT_TRUE (projGuidOut.IsValid());
    EXPECT_TRUE (projGuidOut==projGuid);

    // try round-tripping a GUID through a string and back
    Utf8String guidstr(projGuid.ToString());
    EXPECT_EQ (SUCCESS, projGuidOut.FromString(guidstr.c_str()));
    EXPECT_TRUE (projGuidOut==projGuid);

    //get the BeGUID
    BeSQLite::BeGuid dbGuid = m_db.GetDbGuid();
    EXPECT_TRUE (dbGuid.IsValid());

    //create a new Db with explicit BeSQLite::BeGuid value
    Db db2;
    BeSQLite::BeGuid dbGuid2(false), dbGuid3(false);
    dbGuid2.Init(400, 100);

    BeFileName dbName2 = getDbFilePath(L"new.db");
    if (BeFileName::DoesPathExist(dbName2))
        BeFileName::BeDeleteFile(dbName2);
    m_result = db2.CreateNewDb(dbName2.GetNameUtf8().c_str(), dbGuid2);
    dbGuid3 = db2.GetDbGuid();
    EXPECT_TRUE (dbGuid3.IsValid());
    //get the BriefcaseId
    BeBriefcaseId repId = m_db.GetBriefcaseId();
    EXPECT_TRUE (repId.IsValid());
    EXPECT_EQ (0, repId.GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* Attach and then Detach Db
* @bsimethod                                    Majd.Uddin                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, AttachDb)
    {
    SetupDb(L"Main.db");
    Db db2;
    BeFileName dbName2 = getDbFilePath(L"sub.db");
    if (BeFileName::DoesPathExist(dbName2))
        BeFileName::BeDeleteFile(dbName2);
    m_result = db2.CreateNewDb(dbName2.GetNameUtf8().c_str());
    ASSERT_EQ (BE_SQLITE_OK, m_result) << "Db Creation failed";

    m_result = m_db.AttachDb(db2.GetDbFileName(), "Attachment1");
    EXPECT_EQ (BE_SQLITE_OK, m_result) << "AttachDb() failed";
    m_result = m_db.DetachDb("Attachment1");
    EXPECT_EQ (BE_SQLITE_OK, m_result) << "DetachDb() failed";

    //crash on DetachDb on incorrect alias
    //m_result = m_db.DetachDb ("Aaaaa");

    //AttachDb() passes for any values
    m_result = m_db.AttachDb(getDbFilePath(L"dummy.db").GetNameUtf8().c_str(), "dummy");
    EXPECT_EQ (BE_SQLITE_OK, m_result);

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Majd.Uddin                   07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BeSQLiteDbTests, RenameAndDropTable)
{
    SetupDb(L"RenameAndDropTable.db");

    Utf8CP testTableName = "TestTable";
    EXPECT_FALSE(m_db.TableExists(testTableName)) << "Table '" << testTableName << "' is expected to not exist.";
    EXPECT_EQ(BE_SQLITE_OK, m_db.CreateTable(testTableName, "id NUMERIC, name TEXT")) << "Creating test table '" << testTableName << "' failed.";
    EXPECT_TRUE(m_db.TableExists(testTableName)) << "Table '" << testTableName << "' is expected to exist as it was created right before this check.";

    // Now rename the table
    Utf8CP newTableName = "TestTable2";
    EXPECT_TRUE(m_db.RenameTable(testTableName, newTableName));
    EXPECT_FALSE(m_db.TableExists(testTableName)) << "Table '" << testTableName << "' is expected to not exist.";
    EXPECT_TRUE(m_db.TableExists(newTableName)) << "Table '" << testTableName << "' is expected to exist as it was created right before this check.";

    // Now let's drop it
    EXPECT_EQ(BE_SQLITE_OK, m_db.DropTable(newTableName));
    EXPECT_FALSE(m_db.TableExists(newTableName)) << "Table '" << testTableName << "' is expected to not exist.";

    m_db.CloseDb();
}

//---------------------------------------------------------------------------------------
// @bsimethod                                    Majd.Uddin                   07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BeSQLiteDbTests, GetLastRowId)
{
    SetupDb(L"explainQuery.db");

    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable("TestTable", "id NUMERIC, name TEXT")) << "Creating table failed.";

    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO TestTable Values(1, 'test')"));
    int64_t rowId = m_db.GetLastInsertRowId();
    EXPECT_EQ(1, rowId);

    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO TestTable Values(2, 'test2')"));
    rowId = m_db.GetLastInsertRowId();
    EXPECT_EQ(2, rowId);

    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable("TestTable2", "id NUMERIC, name TEXT")) << "Creating table failed.";
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO TestTable2 Values(1, 'test')"));
    rowId = m_db.GetLastInsertRowId();
    EXPECT_EQ(1, rowId);
    
    m_db.CloseDb();
}

//---------------------------------------------------------------------------------------
// @bsimethod                                    Majd.Uddin                   07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BeSQLiteDbTests, QueryCreationDate)
{
    SetupDb(L"CreationDate.db");
    
    DateTime creationDate;
    //CreationDate is only inserted by Publishers/Convertors? And always return an error
    EXPECT_EQ(BE_SQLITE_ERROR, m_db.QueryCreationDate(creationDate));

    m_db.CloseDb();
}
//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   01/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BeSQLiteDbTests, TableExists)
    {
    SetupDb(L"tableexists.db");

    Utf8CP testTableName = "testtable";

    EXPECT_FALSE (m_db.TableExists(testTableName)) << "Table '" << testTableName << "' is expected to not exist.";

    EXPECT_EQ (BE_SQLITE_OK, m_db.CreateTable(testTableName, "id NUMERIC, name TEXT")) << "Creating test table '" << testTableName << "' failed.";

    EXPECT_TRUE (m_db.TableExists(testTableName)) << "Table '" << testTableName << "' is expected to exist as it was created right before this check.";

    //now test with closed connection
    m_db.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (BeSQLiteDbTests, BlobTest)
    {
    SetupDb(L"blobtest.db");

    Utf8CP testTableName = "testtable";
    ASSERT_EQ (BE_SQLITE_OK, m_db.CreateTable(testTableName, "val BLOB")) << "Creating test table '" << testTableName << "' failed.";

    const int64_t expectedValue = 123456789LL;
    Statement insertStmt;
    ASSERT_EQ (BE_SQLITE_OK, insertStmt.Prepare(m_db, "INSERT INTO testtable (val) VALUES (?)"));

    ASSERT_EQ (BE_SQLITE_OK, insertStmt.BindBlob(1, &expectedValue, sizeof (expectedValue), Statement::MakeCopy::Yes));
    ASSERT_EQ (BE_SQLITE_DONE, insertStmt.Step());

    Statement selectStmt;
    ASSERT_EQ (BE_SQLITE_OK, selectStmt.Prepare(m_db, "SELECT val FROM testtable LIMIT 1"));
    ASSERT_EQ (BE_SQLITE_ROW, selectStmt.Step());

    void const* actualBlob = selectStmt.GetValueBlob(0);
    int64_t actualValue = -1LL;
    memcpy(&actualValue, actualBlob, sizeof (actualValue));
    ASSERT_EQ (expectedValue, actualValue);

    int actualBlobSize = selectStmt.GetColumnBytes(0);
    ASSERT_EQ ((int) sizeof(int64_t), actualBlobSize);

    //now read Int64 directly
    selectStmt.Reset();
    ASSERT_EQ (BE_SQLITE_ROW, selectStmt.Step());
    ASSERT_EQ (0LL, selectStmt.GetValueInt64(0)) << "is expected to not convert the blob implicitly to the expected int64_t";
    }

/*---------------------------------------------------------------------------------**//**
* Save and Query PropertyString
* @bsimethod                                    Majd.Uddin                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, PropertyString)
    {
    SetupDb(L"props.db");

    PropertySpec spec1("TestSpec", "TestApplication");
    Utf8String stringValue("This is test value");
    
    m_result = m_db.SavePropertyString(spec1, stringValue); 
    EXPECT_EQ (BE_SQLITE_OK, m_result) << "SavePropertyString failed";
    EXPECT_TRUE (m_db.HasProperty(spec1));

    Utf8String stringValue2;
    m_result = m_db.QueryProperty(stringValue2, spec1);
    EXPECT_EQ (BE_SQLITE_ROW, m_result);
    
    EXPECT_STREQ (stringValue.c_str(), stringValue2.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* Save and Query Property
* @bsimethod                                    Majd.Uddin                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, Property)
    {
    SetupDb(L"Props2.db");

    PropertySpec spec1("TestSpec", "TestApplication");
    m_result = m_db.SaveProperty(spec1, L"Any Value", 10, 400, 10); 
    EXPECT_EQ (BE_SQLITE_OK, m_result) << "SaveProperty failed";
    
    EXPECT_TRUE (m_db.HasProperty(spec1, 400, 10));

    Utf8CP buffer[10];
    m_result = m_db.QueryProperty(buffer, 10, spec1, 400, 10);
    EXPECT_EQ (BE_SQLITE_ROW, m_result);
    //EXPECT_TRUE (false) << buffer;

    m_result = m_db.DeleteProperty(spec1, 400, 10);
    EXPECT_EQ (BE_SQLITE_DONE, m_result) << "DeleteProperty failed";
    EXPECT_FALSE (m_db.HasProperty(spec1, 400, 10));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, CachedProperties)
    {
    SetupDb(L"Props3.db");

    Byte values[] = {1,2,3,4,5,6};
    PropertySpec spec1("TestSpec", "CachedProp", PropertySpec::Mode::Cached);
    m_result = m_db.SaveProperty(spec1, values, sizeof(values), 400, 10); 
    EXPECT_TRUE (m_db.HasProperty(spec1, 400, 10));

    PropertySpec spec2("TestSpec", "CachedProp2", PropertySpec::Mode::Cached);
    m_result = m_db.SaveProperty(spec2, values, sizeof(values));
    EXPECT_TRUE (m_db.HasProperty(spec2));

    Utf8CP spec3Val="Spec 3 value";
    PropertySpec spec3("Spec3", "CachedProp", PropertySpec::Mode::Cached);
    m_result = m_db.SavePropertyString(spec3, spec3Val);
    EXPECT_TRUE (m_db.HasProperty(spec3));

    Utf8String origStr = "String value";
    m_result = m_db.SavePropertyString(spec1, origStr, 400, 10);

    Byte buffer[10];
    m_result = m_db.QueryProperty(buffer, sizeof(values), spec1, 400, 10);
    EXPECT_EQ (BE_SQLITE_ROW, m_result);
    EXPECT_TRUE(0==memcmp(values, buffer, sizeof(values)));

    Byte values2[] = {10,20};
    m_result = m_db.SaveProperty(spec1, values2, sizeof(values2), 400, 10); 

    m_db.SaveChanges();

    Utf8String strval;
    m_result = m_db.QueryProperty(strval, spec1, 400, 10);
    EXPECT_EQ (BE_SQLITE_ROW, m_result);
    EXPECT_TRUE(0==strval.CompareTo(origStr));

    m_result = m_db.QueryProperty(strval, spec2);
    EXPECT_EQ (BE_SQLITE_ROW, m_result);
    EXPECT_TRUE(0==strval.CompareTo(""));

    m_result = m_db.QueryProperty(buffer, sizeof(values), spec2);
    EXPECT_EQ (BE_SQLITE_ROW, m_result);
    EXPECT_TRUE(0==memcmp(values, buffer, sizeof(values)));

    m_result = m_db.QueryProperty(strval, spec3);
    EXPECT_EQ (BE_SQLITE_ROW, m_result);
    EXPECT_TRUE(0==strval.CompareTo(spec3Val));
    //EXPECT_TRUE (false) << buffer;

    m_result = m_db.DeleteProperty(spec1, 400, 10);
    EXPECT_EQ (BE_SQLITE_DONE, m_result) << "DeleteProperty failed";
    EXPECT_FALSE (m_db.HasProperty(spec1, 400, 10));

    Utf8String val2Str = "value 2";
    m_result = m_db.SavePropertyString(spec1, val2Str, 400, 10);

    if (true)
        {
        Savepoint savepoint(m_db, "intermediate");
        m_result = m_db.SavePropertyString(spec1, "ChangedStr", 400, 10);
        savepoint.Cancel();
        }

    m_result = m_db.QueryProperty(strval, spec1, 400, 10);
    EXPECT_EQ (BE_SQLITE_ROW, m_result);
    EXPECT_TRUE(0==strval.CompareTo(val2Str));
    EXPECT_TRUE (m_db.HasProperty(spec1, 400, 10));
    }

/*---------------------------------------------------------------------------------**//**
* Save and Query BriefcaseLocal Values
* @bsimethod                                    Majd.Uddin                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, BriefcaseLocalValues)
    {
    SetupDb(L"local.db");
    
    //Working with RLVs through RLVCache
    int val = -1345;
    Utf8CP testPropValueName = "TestProp";
    size_t rlvIndex = 0;
    ASSERT_EQ (BE_SQLITE_OK, m_db.GetRLVCache().Register(rlvIndex, testPropValueName));
    m_result = m_db.GetRLVCache().SaveValue(rlvIndex, val);
    EXPECT_EQ (BE_SQLITE_OK, m_result) << "SaveBriefcaseLocalValue failed";

    uint64_t actualVal = -1LL;
    m_result = m_db.GetRLVCache().QueryValue(actualVal, rlvIndex);
    EXPECT_EQ (BE_SQLITE_OK, m_result);
    EXPECT_EQ (val, (int) actualVal);

    m_db.SaveChanges();

    actualVal = -1LL;
    m_result = m_db.GetRLVCache().QueryValue(actualVal, rlvIndex);
    EXPECT_EQ (BE_SQLITE_OK, m_result);
    EXPECT_EQ (val, (int) actualVal);

    ASSERT_TRUE (m_db.GetRLVCache().TryGetIndex(rlvIndex, testPropValueName));
    ASSERT_FALSE (m_db.GetRLVCache().TryGetIndex(rlvIndex, "GarbageProp"));

    ASSERT_EQ (BE_SQLITE_ERROR, m_db.GetRLVCache().Register(rlvIndex, testPropValueName));

    //Work with RLVs directly
    Utf8CP testProp2 = "TestProp2";
    m_result = m_db.SaveBriefcaseLocalValue(testProp2, "Test Value");
    EXPECT_EQ(BE_SQLITE_DONE, m_result);

    Utf8String val2 = "None";
    m_result = m_db.QueryBriefcaseLocalValue(testProp2, val2);
    EXPECT_EQ(BE_SQLITE_ROW, m_result);
    EXPECT_STREQ("Test Value", val2.c_str());
    
    m_db.CloseDb();
    }


/*---------------------------------------------------------------------------------**//**
* Simulate a LineStyle dgndb case
* @bsimethod                                    Majd.Uddin                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, linestyleDB)
    {
    SetupDb(L"linestyle.db");

    //creating a table
    EXPECT_EQ (BE_SQLITE_OK, m_db.CreateTable("linestyles", "lsId NUMERIC, lsName TEXT"));
    EXPECT_TRUE (m_db.TableExists("linestyles"));
    EXPECT_TRUE (m_db.ColumnExists("linestyles", "lsId"));

    //Add data
    ASSERT_EQ (BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO linestyles (lsId, lsName) values (10, 'ARROW')"));

    //Dump the result for display
    m_db.DumpSqlResults("SELECT * from linestyles");
    }

#ifdef PUBLISHER_WONT_PUBLISH_EXPIRED_FILES
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeSQLiteDbOpenTest, Expired)
    {
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    BeSQLiteLib::Initialize(tempDir);
    
    BeFileName docDir;
    BeTest::GetHost().GetDocumentsRoot(docDir);

    BeFileName expiredFileNameW (docDir);
    expiredFileNameW.AppendToPath(L"DgnDb");
    expiredFileNameW.AppendToPath(L"expired.idgndb");
    ASSERT_TRUE( BeFileName::DoesPathExist(expiredFileNameW) );

    Utf8String expiredFileName(expiredFileNameW);

    BeSQLite::Db::OpenParams parms(BeSQLite::Db::OpenMode::Readonly);

    BeSQLite::Db db;
    ASSERT_TRUE( db.OpenBeSQLiteDb(expiredFileName.c_str(), parms) == BE_SQLITE_OK );
    ASSERT_TRUE( db.IsDbOpen() );
    ASSERT_TRUE( db.IsExpired() );
    DateTime xdate;
    ASSERT_TRUE( db.GetExpirationDate(xdate) == BE_SQLITE_OK );
    ASSERT_TRUE( DateTime::Compare(DateTime::GetCurrentTimeUtc(), xdate) != DateTime::CompareResult::EarlierThan );

    db.CloseDb();
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/14
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeSQLiteDbOpenTest, Expired2)
    {
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    BeSQLiteLib::Initialize(tempDir);
    
    Utf8String expiredFileName;
    if (true)
        {
        BeFileName dbFullName = BeSQLiteDbTests::getDbFilePath(L"expired2.idgndb");
        if (BeFileName::DoesPathExist(dbFullName))
            BeFileName::BeDeleteFile(dbFullName);
        BeSQLite::Db db;
        BeSQLite::Db::CreateParams createParms;
        createParms.SetExpirationDate(DateTime::GetCurrentTimeUtc());
        ASSERT_EQ( BE_SQLITE_OK, db.CreateNewDb(dbFullName.GetNameUtf8().c_str(), BeSQLite::BeGuid(), createParms) );
        ASSERT_TRUE( db.IsDbOpen() );
        ASSERT_TRUE( db.IsExpired() );
        expiredFileName.assign(db.GetDbFileName());
        }    

    BeSQLite::Db::OpenParams parms(BeSQLite::Db::OpenMode::Readonly);

    BeSQLite::Db db;
    ASSERT_TRUE( db.OpenBeSQLiteDb(expiredFileName.c_str(), parms) == BE_SQLITE_OK );
    ASSERT_TRUE( db.IsDbOpen() );
    ASSERT_TRUE( db.IsExpired() );
    DateTime xdate;
    ASSERT_TRUE( db.QueryExpirationDate(xdate) == BE_SQLITE_ROW );
    ASSERT_TRUE( DateTime::Compare(DateTime::GetCurrentTimeUtc(), xdate) != DateTime::CompareResult::EarlierThan );

    db.CloseDb();
    }

struct BeSQLiteEmbeddedFileTests : BeSQLiteDbTests
    {
protected:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                     Muhammad Hassan                  5/15
    //+---------------+---------------+---------------+---------------+---------------+------
    void deleteExistingFile(BeFileName filePath)
        {
        if (BeFileName::DoesPathExist(filePath.GetName()))
            {
            // Delete any previously exported file
            BeFileNameStatus fileDeleteStatus = BeFileName::BeDeleteFile(filePath.GetName());
            ASSERT_EQ(BeFileNameStatus::Success, fileDeleteStatus);
            }
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  12/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BeSQLiteEmbeddedFileTests, ReplaceExistingEmbeddedFile)
    {
    SetupDb(L"embeddedfiles.db");

    //test file
    //  Using a much larger file so I could check that the embedded blobs were removed from the BE_Prop table.
    Utf8CP testFileNameOld = "Bentley_Standard_CustomAttributes.01.12.ecschema.xml";
    WString testFileNameOldW(testFileNameOld, BentleyCharEncoding::Utf8);

    BeFileName testFilePathOld;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(testFilePathOld);
    testFilePathOld.AppendToPath(L"ECSchemas");
    testFilePathOld.AppendToPath(L"Standard");
    testFilePathOld.AppendToPath(testFileNameOldW.c_str());

    //INSERT scenario
    DbEmbeddedFileTable& embeddedFileTable = m_db.EmbeddedFiles();
    DbResult stat = BE_SQLITE_OK;
    DateTime expectedLastModified = DateTime::GetCurrentTimeUtc();

    BeBriefcaseBasedId embeddedFileId = embeddedFileTable.Import(&stat, testFileNameOld, testFilePathOld.GetNameUtf8().c_str(), ".xml", nullptr, &expectedLastModified);
    ASSERT_EQ(BE_SQLITE_OK, stat);
    ASSERT_TRUE(embeddedFileId.IsValid());

    BeFileName testFilePath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(testFilePath);
    testFilePath.AppendToPath(L"ECSchemas");
    testFilePath.AppendToPath(L"Standard");
    testFilePath.AppendToPath(testFileNameOldW.c_str());
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Replace(testFileNameOld, testFilePath.GetNameUtf8().c_str()));

    //Query to check that embedded blobs were removed form BE_Prop
    Statement stmt;
    DbResult dbr = stmt.Prepare(m_db, "SELECT * FROM " BEDB_TABLE_Property " WHERE Id=? AND SubId>0");
    ASSERT_EQ(BE_SQLITE_OK, dbr);
    stmt.BindId(1, embeddedFileId);
    dbr = stmt.Step();
    ASSERT_EQ(BE_SQLITE_DONE, dbr);

    BeFileName exportFilePath;
    BeTest::GetHost().GetOutputRoot(exportFilePath);
    exportFilePath.AppendToPath(testFileNameOldW.c_str());
    deleteExistingFile(exportFilePath);
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Export(exportFilePath.GetNameUtf8().c_str(), testFileNameOld));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  12/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BeSQLiteEmbeddedFileTests, ReadAddNewEntrySaveEmbeddedFile)
    {
    SetupDb(L"embeddedfiles.db");

    //test file
    //  Used a fairly large file for this to verify that it correctly handles files that are larger than one blob.
    Utf8CP testFileName = "Bentley_Standard_CustomAttributes.01.12.ecschema.xml";
    WString testFileNameW(testFileName, BentleyCharEncoding::Utf8);

    BeFileName testFilePath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(testFilePath);
    testFilePath.AppendToPath(L"ECSchemas");
    testFilePath.AppendToPath(L"Standard");
    testFilePath.AppendToPath(testFileNameW.c_str());

    //INSERT scenario
    DbEmbeddedFileTable& embeddedFileTable = m_db.EmbeddedFiles();
    DbResult stat = BE_SQLITE_OK;
    DateTime expectedLastModified = DateTime::GetCurrentTimeUtc();
    double expectedLastModifiedJd = 0.0;
    ASSERT_EQ(SUCCESS, expectedLastModified.ToJulianDay(expectedLastModifiedJd));

    BeBriefcaseBasedId embeddedFileId = embeddedFileTable.Import(&stat, testFileName, testFilePath.GetNameUtf8().c_str(), ".xml", nullptr, &expectedLastModified);
    ASSERT_EQ(BE_SQLITE_OK, stat);
    ASSERT_TRUE(embeddedFileId.IsValid());

    Utf8CP NewFileName = "Copy_Bentley_Standard_CustomAttributes.01.12.ecschema.xml";
    WString NewFileNameW(NewFileName, BentleyCharEncoding::Utf8);
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.AddEntry(NewFileName, "xml"));

    uint64_t size = 0;
    ASSERT_EQ(embeddedFileId, embeddedFileTable.QueryFile(testFileName, &size));
    ASSERT_TRUE(size > 0);

    bvector<Byte> buffer;
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Read(buffer, testFileName));
    ASSERT_TRUE(size == buffer.size());
    //Save the data with compression and than read again to verify that the data is unchanged.
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Save(buffer.data(), size, NewFileName));
    bvector<Byte> buffer2;
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Read(buffer2, NewFileName));
    ASSERT_TRUE(buffer.size() == buffer2.size());
    ASSERT_EQ(0, memcmp(&buffer[0], &buffer2[0], buffer.size()));

    //Now save data without compression and read it again and read it again to verify that the data is unchanged.
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Save(buffer.data(), size, NewFileName, nullptr, false));
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Read(buffer2, NewFileName));
    ASSERT_TRUE(buffer.size() == buffer2.size());
    ASSERT_EQ(0, memcmp(&buffer[0], &buffer2[0], buffer.size()));

    BeFileName exportFilePathOld;
    BeTest::GetHost().GetOutputRoot(exportFilePathOld);
    exportFilePathOld.AppendToPath(testFileNameW.c_str());
    deleteExistingFile(exportFilePathOld);

    BeFileName exportFilePath;
    BeTest::GetHost().GetOutputRoot(exportFilePath);
    exportFilePath.AppendToPath(NewFileNameW.c_str());
    deleteExistingFile(exportFilePath);

    //NewFileName now refers to a file that was embedded without compression. Verify that Export works for this file and for the original file aswell.
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Export(exportFilePathOld.GetNameUtf8().c_str(), testFileName));
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Export(exportFilePath.GetNameUtf8().c_str(), NewFileName));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  12/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BeSQLiteEmbeddedFileTests, ImportExportEmptyFile)
    {
    SetupDb(L"embeddedfiles.db");

    //test file
    Utf8CP testFileName = "EmptyFile.txt";
    BeFileName testFilePath;
    BeTest::GetHost().GetOutputRoot(testFilePath);
    testFilePath.AppendToPath(WString(testFileName, BentleyCharEncoding::Utf8).c_str());
    deleteExistingFile(testFilePath);

    BeFile testFile;
    ASSERT_EQ(BeFileStatus::Success, testFile.Create(testFilePath.c_str()));

    //INSERT scenario
    DbEmbeddedFileTable& embeddedFileTable = m_db.EmbeddedFiles();
    DbResult stat = BE_SQLITE_OK;
    DateTime expectedLastModified = DateTime::GetCurrentTimeUtc();
    double expectedLastModifiedJd = 0.0;
    ASSERT_EQ(SUCCESS, expectedLastModified.ToJulianDay(expectedLastModifiedJd));

    BeBriefcaseBasedId embeddedFileId = embeddedFileTable.Import(&stat, testFileName, testFilePath.GetNameUtf8().c_str(), "txt", nullptr, &expectedLastModified);
    ASSERT_EQ(BE_SQLITE_OK, stat);
    ASSERT_TRUE(embeddedFileId.IsValid());

    BeFileName testFileOutPath;
    BeTest::GetHost().GetOutputRoot(testFileOutPath);
    testFileOutPath.AppendToPath(L"EmptyFileOut.txt");
    deleteExistingFile(testFileOutPath);
    ASSERT_EQ(BE_SQLITE_OK, embeddedFileTable.Export(testFileOutPath.GetNameUtf8().c_str(), testFileName));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  12/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BeSQLiteEmbeddedFileTests, EmbedFileWithInvalidPath)
    {
    SetupDb(L"embeddedfiles.db");

    //test file
    Utf8CP testFileName = "StartupCompany.json";
    WString testFileNameW(testFileName, BentleyCharEncoding::Utf8);

    //Test File Path
    BeFileName testFilePath;
    testFilePath.AppendToPath(testFileNameW.c_str());

    //INSERT scenario
    DbEmbeddedFileTable& embeddedFileTable = m_db.EmbeddedFiles();
    DbResult stat = BE_SQLITE_OK;
    DateTime expectedLastModified = DateTime::GetCurrentTimeUtc();
    double expectedLastModifiedJd = 0.0;
    ASSERT_EQ(SUCCESS, expectedLastModified.ToJulianDay(expectedLastModifiedJd));

    BeBriefcaseBasedId embeddedFileId = embeddedFileTable.Import(&stat, testFileName, testFilePath.GetNameUtf8().c_str(), "JSON", nullptr, &expectedLastModified);
    ASSERT_EQ(BE_SQLITE_ERROR_FileNotFound, stat);
    ASSERT_FALSE(embeddedFileId.IsValid());
    }
