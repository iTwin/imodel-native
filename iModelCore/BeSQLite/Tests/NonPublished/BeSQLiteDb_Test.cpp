/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "BeSQLiteNonPublishedTests.h"
#include "BeSQLite/ChangeSet.h"
#include <map>
#include <vector>

//---------------------------------------------------------------------------------------
// Creating a new Db for the test
// @bsimethod                                    Krischan.Eberle                   01/14
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SetupDb(Db& db, WCharCP dbName)
    {
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir(tempDir);
    BeSQLiteLib::Initialize(tempDir);

    BeFileName dbFileName;
    BeTest::GetHost().GetOutputRoot(dbFileName);
    dbFileName.AppendToPath(dbName);

    if (BeFileName::DoesPathExist(dbFileName))
        BeFileName::BeDeleteFile(dbFileName);

    DbResult result = db.CreateNewDb(dbFileName.GetNameUtf8().c_str());
    EXPECT_EQ(BE_SQLITE_OK, result) << "Db Creation failed";
    if (result == BE_SQLITE_OK)
        db.SaveChanges();

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeSQLiteDb, BeBriefcaseBasedIdTest)
    {
    BeBriefcaseId bc(0x103);
    BeBriefcaseBasedId id1(bc, 0x108d7de7e);
    EXPECT_TRUE(bc == id1.GetBriefcaseId());
    EXPECT_TRUE(0x108d7de7e == id1.GetLocalId());

    Utf8String val=id1.ToHexStr();
    EXPECT_TRUE(val == "0x1030108d7de7e");

    BeBriefcaseBasedId id2 = BeBriefcaseBasedId::CreateFromJson(Json::Value(val));
    EXPECT_TRUE(id2 == id1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     11/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST(BeSQLiteDb, CheckProfileVersion)
    {
    ProfileVersion expectedProfileVersion(2, 4, 5, 3);
    ProfileVersion minimumUpgradeProfileVersion(2, 3, 0, 0);

    std::map<ProfileVersion, ProfileState> testDataset {{ProfileVersion(0, 0, 0, 0), ProfileState::Older(ProfileState::CanOpen::No, false)},
    {ProfileVersion(1, 0, 0, 0), ProfileState::Older(ProfileState::CanOpen::No, false)},
    {ProfileVersion(1, 9, 0, 0), ProfileState::Older(ProfileState::CanOpen::No, false)},
    {ProfileVersion(2, 3, 0, 0), ProfileState::Older(ProfileState::CanOpen::Readonly, true)},
    {ProfileVersion(2, 4, 0, 0), ProfileState::Older(ProfileState::CanOpen::Readwrite, true)},
    {ProfileVersion(2, 4, 2, 3), ProfileState::Older(ProfileState::CanOpen::Readwrite, true)},
    {ProfileVersion(2, 4, 5, 0), ProfileState::Older(ProfileState::CanOpen::Readwrite, true)},
    {ProfileVersion(2, 4, 5, 2), ProfileState::Older(ProfileState::CanOpen::Readwrite, true)},

    {ProfileVersion(2, 4, 5, 3), ProfileState::UpToDate()},

    {ProfileVersion(2, 4, 5, 4), ProfileState::Newer(ProfileState::CanOpen::Readwrite)},
    {ProfileVersion(2, 4, 5, 33), ProfileState::Newer(ProfileState::CanOpen::Readwrite)},

    {ProfileVersion(2, 4, 6, 0), ProfileState::Newer(ProfileState::CanOpen::Readwrite)},
    {ProfileVersion(2, 4, 6, 99), ProfileState::Newer(ProfileState::CanOpen::Readwrite)},
    {ProfileVersion(2, 4, 99, 0), ProfileState::Newer(ProfileState::CanOpen::Readwrite)},
    {ProfileVersion(2, 4, 99, 99), ProfileState::Newer(ProfileState::CanOpen::Readwrite)},

    {ProfileVersion(2, 5, 0, 0), ProfileState::Newer(ProfileState::CanOpen::Readonly)},
    {ProfileVersion(2, 5, 0, 1), ProfileState::Newer(ProfileState::CanOpen::Readonly)},
    {ProfileVersion(2, 99, 0, 1), ProfileState::Newer(ProfileState::CanOpen::Readonly)},
    {ProfileVersion(3, 0, 0, 0), ProfileState::Newer(ProfileState::CanOpen::No)},
    {ProfileVersion(99, 99, 99, 99), ProfileState::Newer(ProfileState::CanOpen::No)}};

    for (std::pair<ProfileVersion, ProfileState> const& testItem : testDataset)
        {
        ProfileVersion const& actualProfileVersion = testItem.first;
        ProfileState const& expectedState = testItem.second;

        ProfileState actualState = Db::CheckProfileVersion(expectedProfileVersion, actualProfileVersion, minimumUpgradeProfileVersion, "Test");

        EXPECT_EQ(expectedState, actualState) <<  "Expected version: " << expectedProfileVersion.ToJson().c_str() << " - Actual version: " << actualProfileVersion.ToJson().c_str();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(BeSQLiteDb, AssignBriefcaseIdInReadonlyMode)
    {
    Utf8String dbPath;

    //prepare test dgn db
    {
    Db db;
    auto stat = SetupDb(db, L"changerepoid.ibim");
    ASSERT_EQ(BE_SQLITE_OK, stat) << "Creation of test BeSQLite DB failed.";
    dbPath.assign(db.GetDbFileName());
    db.CloseDb();
    }

    Db db;
    DbResult stat = db.OpenBeSQLiteDb(dbPath.c_str(), Db::OpenParams(Db::OpenMode::Readonly));
    ASSERT_EQ(BE_SQLITE_OK, stat) << L"Reopening test Bim '" << dbPath.c_str() << L"' failed.";

    BeTest::SetFailOnAssert(false);
    stat = db.ResetBriefcaseId(BeBriefcaseId(12345));
    BeTest::SetFailOnAssert(true);
    ASSERT_EQ(BE_SQLITE_READONLY, stat) << L"Calling ResetBriefcaseId on readonly Bim file is expected to fail.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   12/12
//+---------------+---------------+---------------+---------------+---------------+------
TEST(BeSQLiteDb, AssignBriefcaseId)
    {
    Utf8String dbPath;

    //prepare test dgn db
    {
    Db db;
    auto stat = SetupDb(db, L"assignbriefcaseid.db");
    ASSERT_EQ(BE_SQLITE_OK, stat) << "Creation of test BeSQLite DB failed.";
    dbPath.assign(db.GetDbFileName());

    std::vector<int> localValues = {1234, 111, -111, 0};
    std::vector<Utf8String> localValueNames = {"key0", "key1", "key2", "key3"};
    const size_t valueCount = localValues.size();
    for (size_t i = 0; i < valueCount; i++)
        {
        int val = localValues[i];
        size_t keyIndex = 0;
        ASSERT_EQ(BE_SQLITE_OK, db.GetBLVCache().Register(keyIndex, localValueNames[i].c_str())) << "Registration of RLV " << localValueNames[i].c_str() << " is expected to succeed.";
        auto result = db.GetBLVCache().SaveValue(keyIndex, val);
        ASSERT_EQ(BE_SQLITE_OK, result) << "Saving test BLV '" << localValueNames[i].c_str() << "=" << val << "' failed";
        }

    ASSERT_EQ(BE_SQLITE_OK, db.SaveChanges()) << "Committing briefcase local values failed.";
    db.CloseDb();
    }

    //reopen Bim again, change briefcase id and close again (to avoid that caches linger around)
    BeBriefcaseId expectedBriefcaseId;
    expectedBriefcaseId.Invalidate();

    {
    Db db;
    DbResult stat = db.OpenBeSQLiteDb(dbPath.c_str(), Db::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_EQ(BE_SQLITE_OK, stat) << "Reopening test Bim '" << dbPath.c_str() << "' failed.";

    //now change briefcase id. This should truncate be_local and reinsert the new briefcase id
    stat = db.ResetBriefcaseId(BeBriefcaseId(BeBriefcaseId::FirstValidBriefcaseId()));
    ASSERT_EQ(BE_SQLITE_OK, stat) << "Changing the briefcase id is not expected to fail.";
    }

    //now reopen from scratch
    Db db;
    DbResult stat = db.OpenBeSQLiteDb(dbPath.c_str(), Db::OpenParams(Db::OpenMode::Readonly));
    ASSERT_EQ(BE_SQLITE_OK, stat) << "Reopening test Bim '" << dbPath.c_str() << "' failed.";

    //query be_local to check that there is only one row (the repo id)
    Statement statement;
    ASSERT_EQ(BE_SQLITE_OK, statement.Prepare(db, "SELECT Name, Val from " BEDB_TABLE_Local)) << "Preparing SQL statement to retrieve content of be_local failed";
    int rowCount = 0;
    while (statement.Step() == BE_SQLITE_ROW)
        {
        rowCount++;
        Utf8CP name = statement.GetValueText(0);
        // NB: "repository" here really means "briefcase", but we don't want to break existing DgnDbs.
        ASSERT_STREQ("be_repositoryid", name) << "be_local after a briefcase id change should only contain the briefcase id.";
        //don't mimick the blob deserialization here. Just test that the column is not null. Use the API to check the actual repo id later
        ASSERT_FALSE(statement.IsColumnNull(1)) << "Val column of briefcase id row in be_local after a briefcase id change must not be null.";
        }

    ASSERT_EQ(1, rowCount) << "be_local after a briefcase id change should only contain one row (the repo id).";
    }

// #include "BeSQLitePublishedTests.h"
// #include "BeSQLite/ChangeSet.h"
// #include <vector>
// #include <limits>
// #include <string>
// #include <initializer_list>

/*---------------------------------------------------------------------------------**//**
* Test fixture for testing Db
* @bsimethod                                    Majd.Uddin                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct BeSQLiteDbTests : public ::testing::Test
    {
    public:
        Db              m_db;
        DbResult        m_result;

        static DbResult SetupDb(Db& db, WCharCP dbName, BeGuidCR dbGuid=BeGuid(), Db::CreateParams const& createParams=Db::CreateParams());
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
DbResult BeSQLiteDbTests::SetupDb(Db& db, WCharCP dbName, BeGuidCR dbGuid, Db::CreateParams const& createParams)
    {
    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);
    BeSQLiteLib::Initialize(temporaryDir);

    BeFileName dbFullName = getDbFilePath(dbName);
    if (BeFileName::DoesPathExist(dbFullName))
        BeFileName::BeDeleteFile(dbFullName);
    DbResult result = db.CreateNewDb(dbFullName.GetNameUtf8().c_str(), dbGuid, createParams);
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

/*---------------------------------------------------------------------------------**//**
* Create an encrypted Db and then test opening it.
* @bsimethod                                    Shaun.Sewall                    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, CreateEncryptedDb)
    {
    // Verify that we can create an encrypted database
    Db db;
    WCharCP dbName = L"createEncrypted.db";
    uint64_t encryptionKey = 0x1234567890abcdef;
    Db::CreateParams createParams;
    createParams.GetEncryptionParamsR().SetKey(&encryptionKey, static_cast<uint32_t>(sizeof(encryptionKey)));
    DbResult result = SetupDb(db, dbName, BeGuid(), createParams);
    BeFileName dbFileName(db.GetDbFileName(), BentleyCharEncoding::Utf8);
    ASSERT_EQ(BE_SQLITE_OK, result) << "Db Creation failed";
    ASSERT_FALSE(db.IsReadonly());
    ASSERT_TRUE(db.IsDbOpen());
    db.CloseDb();
    ASSERT_FALSE(db.IsDbOpen());
    ASSERT_TRUE(Db::IsEncryptedDb(dbFileName)) << "Expect ENCRYPTED_BESQLITE_FORMAT_SIGNATURE in file";

    // Opening an encrypted database without supplying the key should fail
    Db::OpenParams openParams(Db::OpenMode::Readonly);
    result = db.OpenBeSQLiteDb(getDbFilePath(dbName), openParams);
    ASSERT_EQ(BE_SQLITE_NOTADB, result) << "Expect OpenBeSQLiteDb to fail because encryption key was not provided";
    ASSERT_FALSE(db.IsDbOpen());

    // Opening an encrypted database with the key should succeed
    openParams.GetEncryptionParamsR().SetKey(&encryptionKey, static_cast<uint32_t>(sizeof(encryptionKey)));
    result = db.OpenBeSQLiteDb(getDbFilePath(dbName), openParams);
    ASSERT_EQ(BE_SQLITE_OK, result) << "Expect OpenBeSQLiteDb to succeed because encryption key was provided";
    ASSERT_TRUE(db.IsDbOpen());
    db.CloseDb();
    ASSERT_FALSE(db.IsDbOpen());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, EncryptDb)
    {
    // Create an unencrypted database
    Db db;
    WCharCP dbName = L"encrypt.db";
    SetupDb(db, dbName);
    ASSERT_TRUE(db.IsDbOpen());
    BeFileName dbFileName(db.GetDbFileName(), BentleyCharEncoding::Utf8);
    db.CloseDb();
    ASSERT_FALSE(db.IsDbOpen());
    ASSERT_FALSE(Db::IsEncryptedDb(dbFileName)) << "Should not have ENCRYPTED_BESQLITE_FORMAT_SIGNATURE in file";

    DbResult result = Db::EncryptDb(dbFileName, Db::EncryptionParams());
    ASSERT_EQ(BE_SQLITE_MISUSE, result) << "Expect failure because invalid key was passed";

    uint64_t encryptionKey = 0x1234567890abcdef;
    result = Db::EncryptDb(dbFileName, Db::EncryptionParams(&encryptionKey, static_cast<uint32_t>(sizeof(encryptionKey))));
    ASSERT_EQ(BE_SQLITE_OK, result);
    ASSERT_TRUE(Db::IsEncryptedDb(dbFileName)) << "Expect ENCRYPTED_BESQLITE_FORMAT_SIGNATURE in file";

    Db::OpenParams openParams(Db::OpenMode::Readonly);
    openParams.GetEncryptionParamsR().SetKey(&encryptionKey, static_cast<uint32_t>(sizeof(encryptionKey)));
    result = db.OpenBeSQLiteDb(dbFileName, openParams);
    ASSERT_EQ(BE_SQLITE_OK, result) << "Expect OpenBeSQLiteDb to succeed because encryption key was provided";
    ASSERT_TRUE(db.IsDbOpen());
    db.CloseDb();

    encryptionKey = 0x111222333444;
    result = Db::EncryptDb(dbFileName, Db::EncryptionParams(&encryptionKey, static_cast<uint32_t>(sizeof(encryptionKey))));
    ASSERT_EQ(BE_SQLITE_MISUSE, result) << "Expect failure because database is already encrypted";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, EncryptDbWithExtra)
    {
    // Create an unencrypted database
    Db db;
    WCharCP dbName = L"encryptWithExtra.db";
    SetupDb(db, dbName);
    ASSERT_TRUE(db.IsDbOpen());
    BeFileName dbFileName(db.GetDbFileName(), BentleyCharEncoding::Utf8);
    db.CloseDb();
    ASSERT_FALSE(db.IsDbOpen());
    ASSERT_FALSE(Db::IsEncryptedDb(dbFileName)) << "Should not have ENCRYPTED_BESQLITE_FORMAT_SIGNATURE in file";

    uint64_t encryptionKey = 0x1234567890abcdef;
    Json::Value extraData("This is extra data");
    Db::EncryptionParams encryptionParams(&encryptionKey, static_cast<uint32_t>(sizeof(encryptionKey)), EncryptionKeySource::NotSpecified, Json::FastWriter::ToString(extraData));
    DbResult result = Db::EncryptDb(dbFileName, encryptionParams);
    ASSERT_EQ(BE_SQLITE_OK, result);
    ASSERT_TRUE(Db::IsEncryptedDb(dbFileName)) << "Expect ENCRYPTED_BESQLITE_FORMAT_SIGNATURE in file";

    Db::OpenParams openParams(Db::OpenMode::Readonly);
    openParams.GetEncryptionParamsR().SetKey(&encryptionKey, static_cast<uint32_t>(sizeof(encryptionKey)));
    result = db.OpenBeSQLiteDb(dbFileName, openParams);
    ASSERT_EQ(BE_SQLITE_OK, result) << "Expect OpenBeSQLiteDb to succeed because encryption key was provided";
    ASSERT_TRUE(db.IsDbOpen());
    db.CloseDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, PasswordProtectDb)
    {
    // Create an unencrypted database
    Db db;
    WCharCP dbName = L"password.db";
    SetupDb(db, dbName);
    ASSERT_TRUE(db.IsDbOpen());
    BeFileName dbFileName(db.GetDbFileName(), BentleyCharEncoding::Utf8);
    db.CloseDb();
    ASSERT_FALSE(db.IsDbOpen());
    ASSERT_FALSE(Db::IsEncryptedDb(dbFileName)) << "Should not have ENCRYPTED_BESQLITE_FORMAT_SIGNATURE in file";

    Utf8CP password = "password";
    DbResult result = Db::PasswordProtectDb(dbFileName, password);
    ASSERT_EQ(BE_SQLITE_OK, result);
    ASSERT_TRUE(Db::IsEncryptedDb(dbFileName)) << "Expect ENCRYPTED_BESQLITE_FORMAT_SIGNATURE in file";

    Db::OpenParams openParams(Db::OpenMode::Readonly);
    openParams.GetEncryptionParamsR().SetPassword("wrongPassword");
    result = db.OpenBeSQLiteDb(getDbFilePath(dbName), openParams);
    ASSERT_EQ(BE_SQLITE_NOTADB, result) << "Expect OpenBeSQLiteDb to fail because wrong password was passed";
    ASSERT_FALSE(db.IsDbOpen());

    openParams.GetEncryptionParamsR().SetPassword(password);
    result = db.OpenBeSQLiteDb(dbFileName, openParams);
    ASSERT_EQ(BE_SQLITE_OK, result) << "Expect OpenBeSQLiteDb to succeed because password was properly provided";
    ASSERT_TRUE(db.IsDbOpen());
    db.CloseDb();
    }

int GetPageSize(BeFileName dbFile)
    {
    Db db;
    Db::OpenParams openParams(Db::OpenMode::Readonly);
    DbResult result = db.OpenBeSQLiteDb(dbFile, openParams);
    EXPECT_EQ(BE_SQLITE_OK, result);
    Statement stmt;
    EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "pragma page_size"));
    stmt.Step();
    return stmt.GetValueInt(0);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeSQLiteDbTests, VacuumWithPageSize)
    {
    // Create an unencrypted database
    Db db;
    WCharCP dbName = L"smalldb.db";
    SetupDb(db, dbName);
    ASSERT_TRUE(db.IsDbOpen());
    BeFileName dbFileName(db.GetDbFileName(), BentleyCharEncoding::Utf8);
    db.CloseDb();

    ASSERT_EQ((int)Db::PageSize::PAGESIZE_4K, GetPageSize(dbFileName));
    auto pageSizes = std::vector<int>{
        (int)Db::PageSize::PAGESIZE_512,
        (int)Db::PageSize::PAGESIZE_1K,
        (int)Db::PageSize::PAGESIZE_2K,
        (int)Db::PageSize::PAGESIZE_4K,
        (int)Db::PageSize::PAGESIZE_8K,
        (int)Db::PageSize::PAGESIZE_16K,
        (int)Db::PageSize::PAGESIZE_32K,
        (int)Db::PageSize::PAGESIZE_64K};

    for (const int ps : pageSizes)
        {
        ASSERT_EQ((int)BE_SQLITE_OK, (int)Db::Vacuum(dbFileName.GetNameUtf8().c_str(), ps));
        ASSERT_EQ(ps, GetPageSize(dbFileName));
        }
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/14
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/14
+---------------+---------------+---------------+---------------+---------------+------*/
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
    BeGuid projGuid(true);
    m_db.SaveProjectGuid(projGuid);
    BeGuid projGuidOut = m_db.QueryProjectGuid();

    EXPECT_TRUE (projGuidOut.IsValid());
    EXPECT_TRUE (projGuidOut==projGuid);

    // try round-tripping a GUID through a string and back
    Utf8String guidstr(projGuid.ToString());
    EXPECT_EQ (SUCCESS, projGuidOut.FromString(guidstr.c_str()));
    EXPECT_TRUE (projGuidOut==projGuid);

    // roundtrip a GUID that started as a string
    Utf8CP strGuid = "c69ec318-60d6-4c54-8f58-e34b78fb8110";
    BeGuid guid;
    EXPECT_FALSE(guid.IsValid());
    guid.FromString(strGuid);
    EXPECT_TRUE(guid.IsValid());
    EXPECT_STREQ(strGuid, guid.ToString().c_str());

    //get the BeGUID
    BeSQLite::BeGuid dbGuid = m_db.GetDbGuid();
    EXPECT_TRUE (dbGuid.IsValid());

    //create a new Db with explicit BeSQLite::BeGuid value
    Db db2;
    BeSQLite::BeGuid dbGuid2(100, 400), dbGuid3(false);

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
    EXPECT_TRUE(BE_SQLITE_OK == m_db.RenameTable(testTableName, newTableName));
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

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(BeSQLiteDbTests, BigUInt64Test)
    {
    SetupDb(L"biguint64test.db");

    Utf8CP testTableName = "testtable";
    ASSERT_EQ(BE_SQLITE_OK, m_db.CreateTable(testTableName, "id INTEGER PRIMARY KEY, val INTEGER")) << "Creating test table '" << testTableName << "' failed.";

    const uint64_t bigNumber = ((uint64_t) std::numeric_limits<int64_t>::max()) + 100;
    ASSERT_TRUE(static_cast<int64_t>(bigNumber) < 0) << "Ensure the uint64 is larger than the max of int64";

    auto assertUInt64 = [] (Db const& db, int64_t id, uint64_t expected)
        {
        CachedStatementPtr selectStmt = db.GetCachedStatement("SELECT val FROM testtable WHERE id=?");
        ASSERT_TRUE(selectStmt != nullptr);
        ASSERT_EQ(BE_SQLITE_OK, selectStmt->BindInt64(1, id));
        ASSERT_EQ(BE_SQLITE_ROW, selectStmt->Step());
        ASSERT_EQ(expected, selectStmt->GetValueUInt64(0));
        ASSERT_EQ(expected, (uint64_t) selectStmt->GetValueInt64(0));
        };

    {
    Statement insertStmt;
    ASSERT_EQ(BE_SQLITE_OK, insertStmt.Prepare(m_db, "INSERT INTO testtable(val) VALUES (?)"));

    ASSERT_EQ(BE_SQLITE_OK, insertStmt.BindUInt64(1, bigNumber));
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());
    assertUInt64(m_db, m_db.GetLastInsertRowId(), bigNumber);
    }


    {
    Utf8String sql;
    sql.Sprintf("INSERT INTO testtable(val) VALUES (%lld)", bigNumber);
    Statement insertStmt;
    ASSERT_EQ(BE_SQLITE_OK, insertStmt.Prepare(m_db, sql.c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, insertStmt.Step());
    assertUInt64(m_db, m_db.GetLastInsertRowId(), bigNumber);
    }

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
    ASSERT_EQ (BE_SQLITE_OK, m_db.GetBLVCache().Register(rlvIndex, testPropValueName));
    m_result = m_db.GetBLVCache().SaveValue(rlvIndex, val);
    EXPECT_EQ (BE_SQLITE_OK, m_result) << "SaveBriefcaseLocalValue failed";

    uint64_t actualVal = -1LL;
    m_result = m_db.GetBLVCache().QueryValue(actualVal, rlvIndex);
    EXPECT_EQ (BE_SQLITE_OK, m_result);
    EXPECT_EQ (val, (int) actualVal);

    m_db.SaveChanges();

    actualVal = -1LL;
    m_result = m_db.GetBLVCache().QueryValue(actualVal, rlvIndex);
    EXPECT_EQ (BE_SQLITE_OK, m_result);
    EXPECT_EQ (val, (int) actualVal);

    ASSERT_TRUE (m_db.GetBLVCache().TryGetIndex(rlvIndex, testPropValueName));
    ASSERT_FALSE (m_db.GetBLVCache().TryGetIndex(rlvIndex, "GarbageProp"));

    ASSERT_EQ (BE_SQLITE_ERROR, m_db.GetBLVCache().Register(rlvIndex, testPropValueName));

    //Work with RLVs directly
    Utf8CP testProp2 = "TestProp2";
    m_result = m_db.SaveBriefcaseLocalValue(testProp2, "Test Value");
    EXPECT_EQ(BE_SQLITE_DONE, m_result);

    Utf8String val2 = "None";
    m_result = m_db.QueryBriefcaseLocalValue(val2, testProp2);
    EXPECT_EQ(BE_SQLITE_ROW, m_result);
    EXPECT_STREQ("Test Value", val2.c_str());

    m_db.CloseDb();
    }


/*---------------------------------------------------------------------------------**//**
* Simulate a LineStyle bim case
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
    expiredFileNameW.AppendToPath(L"Bim");
    expiredFileNameW.AppendToPath(L"expired.ibim");
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
        BeFileName dbFullName = BeSQLiteDbTests::getDbFilePath(L"expired2.ibim");
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
    Utf8CP testFileNameOld = "Bentley_Standard_CustomAttributes.01.13.ecschema.xml";
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
    Utf8CP testFileName = "Bentley_Standard_CustomAttributes.01.13.ecschema.xml";
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

    Utf8CP NewFileName = "Copy_Bentley_Standard_CustomAttributes.01.13.ecschema.xml";
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

struct MyChangeTracker : ChangeTracker
    {
    MyChangeTracker(DbR db) : ChangeTracker("Test") { SetDb(&db); }
    virtual OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) override { BeAssert(false); return OnCommitStatus::Abort; }
    };

struct MyChangeSet : ChangeSet
    {
    virtual ConflictResolution _OnConflict(ConflictCause clause, Changes::Change iter) { BeAssert(false); return ConflictResolution::Abort; }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                   10/16
//---------------------------------------------------------------------------------------
TEST_F(BeSQLiteDbTests, RealUpdateTest)
    {
    SetupDb(L"RealTest.db");

    DbResult result = m_db.ExecuteSql("CREATE TABLE TestTable ([Id] INTEGER PRIMARY KEY, [ZeroReal] REAL, [IntegralReal] REAL, [FractionalReal] REAL)");
    ASSERT_TRUE(result == BE_SQLITE_OK);

    /* Baseline: Entry with just null-s */
    result = m_db.ExecuteSql("INSERT INTO TestTable (ZeroReal,IntegralReal,FractionalReal) values (null, 1.0, 1.1)");
    ASSERT_TRUE(result == BE_SQLITE_OK);

    int64_t rowId = m_db.GetLastInsertRowId();
    ASSERT_EQ(1, rowId);

    MyChangeTracker changeTracker(m_db);
    changeTracker.EnableTracking(true);

    /* Test 1: Update null with 0.0 */
    result = m_db.ExecuteSql("UPDATE TestTable SET ZeroReal=0.0, IntegralReal=1.0, FractionalReal=1.1  WHERE ROWID=1");
    ASSERT_TRUE(result == BE_SQLITE_OK);
    ASSERT_TRUE(changeTracker.HasChanges());

    MyChangeSet changeSet;
    changeSet.FromChangeTrack(changeTracker);
    int size = changeSet.GetSize();
    ASSERT_TRUE(size > 0);

    changeTracker.EndTracking();
    changeSet.Free();
    changeTracker.EnableTracking(true);

    /* Test 2: Update with no changes to integral values
    * Note: SQlite fixed a bug where this got reported as a change: https://www.sqlite.org/src/info/5f3e602831ba2eca */
    result = m_db.ExecuteSql("UPDATE TestTable SET ZeroReal=0.0, IntegralReal=1.0, FractionalReal=1.1 WHERE ROWID=1");
    ASSERT_TRUE(result == BE_SQLITE_OK);
    ASSERT_TRUE(changeTracker.HasChanges());

    changeSet.FromChangeTrack(changeTracker);
    size = changeSet.GetSize();
    ASSERT_TRUE(size == 0);

    changeTracker.EndTracking();
    changeSet.Free();
    m_db.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                   10/16
//---------------------------------------------------------------------------------------
TEST_F(BeSQLiteDbTests, InsertMismatchedColumns)
    {
    SetupDb(L"MismatchedColumnsTest.db");

    // Create a change set with 3 columns
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE TABLE TestTable ([Column1] INTEGER PRIMARY KEY, [Column2] INTEGER, [Column3] INTEGER)"));

    MyChangeTracker changeTracker(m_db);
    changeTracker.EnableTracking(true);

    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("INSERT INTO TestTable Values(1, 2, 3)"));

    MyChangeSet changeSet;
    changeSet.FromChangeTrack(changeTracker);
    int size = changeSet.GetSize();
    ASSERT_TRUE(size > 0);
    changeTracker.EndTracking();

    // Add a column, and attempt to apply the change set
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("ALTER TABLE TestTable ADD COLUMN [Column4]"));
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("DELETE FROM TestTable"));

    DbResult result = changeSet.ApplyChanges(m_db);
    EXPECT_TRUE(result == BE_SQLITE_OK); // SQLite lets this through, and that's good!

    // Drop a column, and attempt to apply the change set
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("DROP TABLE TestTable"));
    EXPECT_EQ(BE_SQLITE_OK, m_db.ExecuteSql("CREATE TABLE TestTable ([Column1] INTEGER PRIMARY KEY, [Column2] INTEGER)"));

    result = changeSet.ApplyChanges(m_db);
    EXPECT_TRUE(result == BE_SQLITE_OK); // SQLite should ideally fail here - we have reported this to them 8/31/2017
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Taslim.Murad                   05/17
//---------------------------------------------------------------------------------------
TEST_F (BeSQLiteDbTests, GetColumn)
{
    SetupDb (L"test1.db");
    EXPECT_TRUE (m_db.IsDbOpen ());

    ASSERT_EQ (BE_SQLITE_OK, m_db.CreateTable ("TestTable1", "id NUMERIC, name TEXT")) << "Creating table failed.";
    EXPECT_EQ (BE_SQLITE_OK, m_db.ExecuteSql ("INSERT INTO TestTable1 Values(1, 'test')"));

    bvector<Utf8String> buff;
    bool res1= m_db.GetColumns (buff ,"TestTable1");
    EXPECT_EQ (res1, true);

    bool IdStatus = false;
    IdStatus = buff[0].Equals(Utf8String ("id"));
    EXPECT_EQ (IdStatus, true);

    bool nameStatus = false;
    nameStatus = buff[1].Equals(Utf8String ("name"));
    EXPECT_EQ (nameStatus, true);

    EXPECT_EQ (BE_SQLITE_OK, m_db.AbandonChanges ());
    m_db.CloseDb ();
}

//---------------------------------------------------------------------------------------
// @bsimethod                                Taslim.Murad                   05/17
//---------------------------------------------------------------------------------------
TEST_F (BeSQLiteDbTests, SaveCreationDate)
{
    bool result=false;
    SetupDb (L"test2.db");
    EXPECT_TRUE (m_db.IsDbOpen ());

    BentleyM0200::DateTime currentDate = BentleyM0200::DateTime::GetCurrentTimeUtc ();
    EXPECT_EQ (BE_SQLITE_OK, m_db.SaveCreationDate ());

    BentleyM0200::DateTime newDate;
    EXPECT_EQ(BE_SQLITE_ROW, m_db.QueryCreationDate(newDate));

    if ( (newDate.GetYear()==currentDate.GetYear()) && (newDate.GetMonth()==currentDate.GetMonth()) && (newDate.GetDay () == currentDate.GetDay ()) && (newDate.GetHour()==currentDate.GetHour()) && (newDate.GetMinute()==currentDate.GetMinute()) && (newDate.GetSecond()==currentDate.GetSecond()) )
    {
        result=true;
    }
    EXPECT_TRUE (result);

    m_db.CloseDb ();
}

//---------------------------------------------------------------------------------------
// @bsimethod                                Taslim.Murad                   05/17
//---------------------------------------------------------------------------------------
TEST_F (BeSQLiteDbTests, AddColumnToTable)
{
    SetupDb (L"test1.db");
    EXPECT_TRUE (m_db.IsDbOpen ());

    ASSERT_EQ (BE_SQLITE_OK, m_db.CreateTable ("TestTable2", "id NUMERIC, name TEXT")) << "Creating table failed.";
    EXPECT_EQ (BE_SQLITE_OK, m_db.AddColumnToTable ("TestTable2", "TitleId", "INTEGER"));

    bvector<Utf8String> buff;
    bool res1 = m_db.GetColumns (buff, "TestTable2");
    EXPECT_EQ (res1, true);

    bool IdStatus = false;
    IdStatus = buff[2].Equals (Utf8String ("TitleId"));
    EXPECT_EQ (IdStatus, true);

    EXPECT_EQ (BE_SQLITE_OK, m_db.CreateIndex("newInd", "TestTable2",true,"id",nullptr));

    m_db.CloseDb ();
}

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                   05/17
//---------------------------------------------------------------------------------------
TEST_F(BeSQLiteDbTests, CreateChangeSetWithSchemaAndDataChanges)
    {
    /* Tests that
     * 1. Schema and data changes are not allowed to the same tables in the same change set.
     * This validates our assumptions in storing transactions and creating revisions
     * 2. Schema and data changes can be made to different tables in the same change set.
     * This validates our assumptions in allowing this for bridge-framework workflows when
     * importing v8 legacy schemas.
     * */

    SetupDb(L"RealTest.db");

    DbResult result = m_db.ExecuteSql("CREATE TABLE TestTable1 ([Id] INTEGER PRIMARY KEY, [Column1] REAL)");
    ASSERT_TRUE(result == BE_SQLITE_OK);
    result = m_db.ExecuteSql("CREATE TABLE TestTable2 ([Id] INTEGER PRIMARY KEY, [Column1] REAL)");
    ASSERT_TRUE(result == BE_SQLITE_OK);

    MyChangeTracker changeTracker(m_db);
    changeTracker.EnableTracking(true);

    // Add row to TestTable1
    result = m_db.ExecuteSql("INSERT INTO TestTable1 (Column1) values (1.1)");
    ASSERT_TRUE(result == BE_SQLITE_OK);

    // Add column to TestTable2
    result = m_db.AddColumnToTable("TestTable2", "Column2", "REAL");
    ASSERT_TRUE(result == BE_SQLITE_OK);

    // Add row to TestTable2
    result = m_db.ExecuteSql("INSERT INTO TestTable2 (Column1,Column2) values (1.1,2.2)");
    ASSERT_TRUE(result == BE_SQLITE_OK);

    // Create change set - succeeds!
    MyChangeSet changeSet;
    result = changeSet.FromChangeTrack(changeTracker);
    ASSERT_TRUE(result == BE_SQLITE_OK);

    // Add column to TestTable1
    result = m_db.AddColumnToTable("TestTable1", "Column2", "REAL");
    ASSERT_TRUE(result == BE_SQLITE_OK);

    // Create change set - fails!
    changeSet.Free();
    result = changeSet.FromChangeTrack(changeTracker);
    ASSERT_TRUE(result == BE_SQLITE_SCHEMA); // Failure!

    // Add row to TestTable 1
    result = m_db.ExecuteSql("INSERT INTO TestTable1 (Column1,Column2) values (3.3,4.4)");
    ASSERT_TRUE(result == BE_SQLITE_OK);

    // Create change set - fails!
    changeSet.Free();
    result = changeSet.FromChangeTrack(changeTracker);
    ASSERT_TRUE(result == BE_SQLITE_SCHEMA); // Failure!

    changeTracker.EndTracking();
    changeSet.Free();
    m_db.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                   05/17
//---------------------------------------------------------------------------------------
TEST_F(BeSQLiteDbTests, ApplyChangeSetAfterSchemaChanges)
    {
    /* Tests that you can create a change set, make a schema change, and then apply that change
     * set to the Db. This validates our assumption that we can merge a schema revision when
     * there are local changes */

    SetupDb(L"RealTest.db");

    DbResult result = m_db.ExecuteSql("CREATE TABLE TestTable ([Id] INTEGER PRIMARY KEY, [Column1] REAL, [Column2] REAL)");
    ASSERT_TRUE(result == BE_SQLITE_OK);

    MyChangeTracker changeTracker(m_db);
    changeTracker.EnableTracking(true);

    // Add row
    result = m_db.ExecuteSql("INSERT INTO TestTable (Column1,Column2) values (1.1,2.2)");
    EXPECT_TRUE(result == BE_SQLITE_OK);

    // Create change set
    MyChangeSet changeSet;
    result = changeSet.FromChangeTrack(changeTracker);
    EXPECT_TRUE(result == BE_SQLITE_OK);
    changeTracker.EndTracking();

    changeSet.Dump("ChangeSet", m_db);

    // Delete all rows
    result = m_db.ExecuteSql("DELETE FROM TestTable");
    EXPECT_TRUE(result == BE_SQLITE_OK);

    // Add column
    result = m_db.AddColumnToTable("TestTable", "Column3", "REAL");
    EXPECT_TRUE(result == BE_SQLITE_OK);

    // Apply change set
    result = changeSet.ApplyChanges(m_db);
    EXPECT_TRUE(result == BE_SQLITE_OK);

    // Validate
    Statement stmt;
    result = stmt.Prepare(m_db, "SELECT Column1 FROM TestTable WHERE Column2=2.2");
    EXPECT_TRUE(result == BE_SQLITE_OK);
    result = stmt.Step();
    EXPECT_TRUE(result == BE_SQLITE_ROW);
    EXPECT_EQ(1.1, stmt.GetValueDouble(0));

    changeSet.Free();
    stmt.Finalize();
    m_db.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Taslim.Murad                   05/17
//---------------------------------------------------------------------------------------
TEST_F (BeSQLiteDbTests, SaveQueryDelBreifCaselocalValue)
{
    SetupDb (L"testb.db");
    EXPECT_TRUE (m_db.IsDbOpen ());

    Utf8CP name = "test";
    EXPECT_EQ (BE_SQLITE_DONE, m_db.SaveBriefcaseLocalValue (name, 2));

    uint64_t value;
    EXPECT_EQ (BE_SQLITE_ROW, m_db.QueryBriefcaseLocalValue (value, name));
    ASSERT_EQ (value, 2);

    EXPECT_EQ (BE_SQLITE_DONE, m_db.DeleteBriefcaseLocalValue (name));
    EXPECT_NE (BE_SQLITE_ROW, m_db.QueryBriefcaseLocalValue (value, name));
}

//---------------------------------------------------------------------------------------
// @bsimethod                                Taslim.Murad                   05/17
//---------------------------------------------------------------------------------------
TEST_F (BeSQLiteDbTests, DelProperties)
{
    SetupDb (L"Props3.db");

    PropertySpec spec1 ("TestSpec", "TestApplication");
    m_result = m_db.SaveProperty (spec1, L"Any Value", 10, 400, 10);
    EXPECT_EQ (BE_SQLITE_OK, m_result) << "SaveProperty failed";
    EXPECT_TRUE (m_db.HasProperty (spec1, 400, 10));

    uint64_t * mid = 0;
    m_result = m_db.DeleteProperties (spec1, mid);
    EXPECT_EQ (BE_SQLITE_DONE, m_result) << "DeleteProperty failed";
    EXPECT_FALSE (m_db.HasProperty (spec1, 400, 10));
}
