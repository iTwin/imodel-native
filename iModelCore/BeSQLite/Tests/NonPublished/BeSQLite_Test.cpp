/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "BeSQLiteNonPublishedTests.h"
#include <BeSQLite/ChangeSet.h>
#include <Bentley/BeDirectoryIterator.h>
#include <BeSQLite/Profiler.h>
#include <BeSQLite/VirtualTab.h>
using namespace MemorySize;

#define MEM_THRESHOLD (100 * MEG)

//=======================================================================================
// @bsistruct
//=======================================================================================
struct BeIdSetTests : public ::testing::Test
{
public:
    void ExpectRoundTrip(BeIdSet const& ids, Utf8CP expected);
    Utf8String ExpectCompressed(BeIdSet const& ids, Utf8CP expected);
    template<typename T> BeIdSet MakeIdSet(std::initializer_list<T> values)
        {
        BeIdSet ids;
        for (auto value : values)
            ids.insert(BeInt64Id(static_cast<int64_t>(value)));

        return ids;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeIdSetTests::ExpectRoundTrip(BeIdSet const& ids, Utf8CP expected)
    {
    Utf8String actual = ExpectCompressed(ids, expected);
    BeIdSet roundtripped;
    roundtripped.FromString(actual);
    EXPECT_TRUE(roundtripped == ids) << " Expected: " << ids.ToString().c_str() << " Actual: " << roundtripped.ToString().c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String BeIdSetTests::ExpectCompressed(BeIdSet const& ids, Utf8CP expected)
    {
    Utf8String actual = ids.ToCompactString();
    EXPECT_TRUE(actual.Equals(expected)) << "Expected: " << expected << " Actual: " << actual.c_str();
    return actual;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeIdSetTests, ToString)
    {
    ExpectRoundTrip(MakeIdSet({2}), "+2");
    ExpectRoundTrip(MakeIdSet({1,5}), "+1+4");
    ExpectRoundTrip(MakeIdSet({3,7,8,10}), "+3+4+1+2");
    ExpectRoundTrip(MakeIdSet({0xFF, 0x150}), "+FF+51");

    ExpectRoundTrip(MakeIdSet({1,2,3,4,5}), "+1*5");
    ExpectRoundTrip(MakeIdSet({2,4,6,8}), "+2*4");
    ExpectRoundTrip(MakeIdSet({1,2,3,4,8,12,16}), "+1*4+4*3");
    ExpectRoundTrip(MakeIdSet({1,2,3,4,8,12,16,17}), "+1*4+4*3+1");

    ExpectRoundTrip(MakeIdSet({100,200,300,400,500,600,700,800,900,1000,1100,1200,1300,1400,1500,1600,1700,1800,1900,2000,2100,2200,2300}), "+64*17");
    ExpectRoundTrip(MakeIdSet({1,10001,20001,30001,40001,50001,60001,70001,80001,90001,100001,110001,120001,130001,140001,150001,160001,170001,180001,190001,200001,210001,220001,230001, 230002}), "+1+2710*17+1");
    ExpectRoundTrip(MakeIdSet({0x21234567890, 0x31234567890, 0x41234567890, 0x61234567890}), "+21234567890+10000000000*2+20000000000");
    ExpectRoundTrip(MakeIdSet({0xabcdef0123456789, 0xabcdef1123456789}), "+ABCDEF0123456789+1000000000");
    ExpectRoundTrip(MakeIdSet({0xf0a0000000100, 0xf0a0000000120, 0xf0a0000000140, 0xf0a0000000202}), "+F0A0000000100+20*2+C2");

    ExpectRoundTrip(MakeIdSet({0xffffffffffffffff}), "+FFFFFFFFFFFFFFFF");
    ExpectRoundTrip(MakeIdSet({0x1ULL, 0xffffffffffffffffULL}), "+1+FFFFFFFFFFFFFFFE");
    ExpectRoundTrip(MakeIdSet({0x1000000000000001ULL, 0x4000000000000004ULL, 0x7000000000000007ULL, 0xa000007777777777ULL}), "+1000000000000001+3000000000000003*2+3000007777777770");

    ExpectCompressed(MakeIdSet({0}), "");
    ExpectCompressed(MakeIdSet({0, 0x1, 0x4, 0x5abc}), "+1+3+5AB8");
    }

    enum {
        TEST_DATA_SIZE = 160 * K,
    };

static void verifySeek(ChunkedArray const& array, int totalSize) {
    ChunkedArray::Reader reader(array);

    EXPECT_TRUE(reader.Seek(-2) == -1);
    EXPECT_TRUE(reader.Seek(totalSize + 1) == -1);

    int data;
    int size = sizeof(data);
    for (int i = 0; i < totalSize / size; ++i) {
        EXPECT_TRUE(reader.Seek(i * size) == 0);
        reader.Read((Byte*)&data, &size);
        EXPECT_TRUE(size == sizeof(int));
        EXPECT_TRUE(data == i);
    }
}

static void verifyRead(ChunkedArray const& array, int numInts, int totalSize) {
        bvector<int> bytes(numInts, 333);
        int* data = bytes.data();

        ChunkedArray::Reader reader(array);
        int index = 0;
        int total = 0;
        int size = 0;
        do {
            size = (int)bytes.size() * sizeof(int);
            reader.Read((Byte*)data, &size);
            total += size;
            for (int i = 0; i < (size / sizeof(int)); ++i)
                EXPECT_TRUE(data[i] == index++);
        } while (size > 0);

        EXPECT_TRUE(total == totalSize);
    }

static void verifyReadSizes(ChunkedArray const& array, int totalSize) {
    verifySeek(array, totalSize);
    verifyRead(array, 22, totalSize);
    verifyRead(array, 100, totalSize);
    verifyRead(array, 1024, totalSize);
    verifyRead(array, 16*K, totalSize);
    verifyRead(array, 2*MEG, totalSize);
}

static void fillArray(ChunkedArray& array, int const* from, int chunkSize, int numInts) {
    int curr = 0;
    while (curr < numInts) {
        array.Append((Byte const*)(from+curr), chunkSize*sizeof(int));
        curr += chunkSize;
    }
}
/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeIdSetTests, ChunkedArray) {
    int totalSize = TEST_DATA_SIZE * sizeof(int);
    auto ptr = std::make_unique<int[]>(totalSize);
    int* testData = ptr.get();
    for (int i = 0; i < TEST_DATA_SIZE; ++i)
        testData[i] = i;

    ChangeSet test1;
    fillArray(test1.m_data, testData, 1024, TEST_DATA_SIZE);
    verifyReadSizes(test1.m_data, totalSize);
    test1.Clear();
    fillArray(test1.m_data, testData, 16 * K, TEST_DATA_SIZE);
    verifyReadSizes(test1.m_data, totalSize);

    Byte* savePtr = test1.m_data.m_chunks[0].data();
    ChangeSet t2 = std::move(test1);
    verifyReadSizes(t2.m_data, totalSize);
    EXPECT_TRUE(test1._IsEmpty());
    EXPECT_EQ(test1.m_data.m_chunks.size(), 0);
    EXPECT_EQ(savePtr, t2.m_data.m_chunks[0].data()); // make sure we did a move, not a copy
    EXPECT_FALSE(t2.m_data.IsEmpty());
    t2.Clear();

    ChunkedArray test2(100);
    test2.Append((Byte*)testData, totalSize);
    verifyReadSizes(test2, totalSize);
    test2.Clear();
    fillArray(test2, testData, 1024, TEST_DATA_SIZE);
    verifyReadSizes(test2, totalSize);
    test2.Clear();

    DdlChanges schema;
    schema.AddDDL("Test1");
    schema.AddDDL("Test2");
    Utf8String ddl = schema.ToString();
    EXPECT_EQ(ddl, "Test1;Test2");

    DdlChanges schema1("This is a Test", 5);
    schema1.AddDDL("This is another test");
    schema1.AddDDL("This is a 3rd test");
    ddl = schema1.ToString();
    EXPECT_EQ(ddl, "This is a Test;This is another test;This is a 3rd test");
}

//=======================================================================================
// @bsiclass
//=======================================================================================
struct TestChangeSet : BeSQLite::ChangeSet
    {
    ConflictResolution _OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) override { BeAssert(false && "Unexpected conflict"); return ConflictResolution::Skip; }
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct TestChangeTracker : BeSQLite::ChangeTracker
    {
    TestChangeTracker(BeSQLite::DbR db) { SetDb(&db); }

    OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) override { return OnCommitStatus::Commit; }

    };

//=======================================================================================
// @bsistruct
//=======================================================================================
struct BeSQliteTestFixture : public ::testing::Test {
protected:
    static std::unique_ptr<Db> Create(Utf8CP fileName) {
        BeFileName outputPath;
        BeTest::GetHost().GetOutputRoot(outputPath);
        outputPath.AppendUtf8(fileName);
        if (outputPath.DoesPathExist())
                outputPath.BeDeleteFile();

        std::unique_ptr<Db> db = std::unique_ptr<Db>(new Db());
        if (db->CreateNewDb(outputPath) != BE_SQLITE_OK)
                return nullptr;

        db->SaveChanges();
        return db;
    }

    static BeFileNameStatus Clone(Utf8CP existingFile, Utf8CP out, bool override = true) {
        BeFileName existingFilePath;
        BeTest::GetHost().GetOutputRoot(existingFilePath);
        existingFilePath.AppendUtf8(existingFile);
        if (!existingFilePath.DoesPathExist())
                return BeFileNameStatus::FileNotFound;

        BeFileName outputPathB;
        BeTest::GetHost().GetOutputRoot(outputPathB);
        outputPathB.AppendUtf8(out);

        if (outputPathB.DoesPathExist())
                if (!override)
                    return BeFileNameStatus::AlreadyExists;
                else {
                    BeFileNameStatus r = outputPathB.BeDeleteFile();
                    if (r != BeFileNameStatus::Success)
                        return r;
                }

        return BeFileName::BeCopyFile(existingFilePath, outputPathB);
    }
    static std::unique_ptr<Db> Open(Utf8CP fileName, Db::OpenMode openMode) {
        BeFileName outputPath;
        BeTest::GetHost().GetOutputRoot(outputPath);
        outputPath.AppendUtf8(fileName);
        if (!outputPath.DoesPathExist())
                return nullptr;

        std::unique_ptr<Db> db = std::unique_ptr<Db>(new Db());
        if (db->OpenBeSQLiteDb(outputPath, Db::OpenParams(openMode)) != BE_SQLITE_OK)
                return nullptr;

        return db;
    }

    static std::unique_ptr<Db> OpenReadWrite(Utf8CP fileName) {
        return Open(fileName, Db::OpenMode::ReadWrite);
    }
    static std::unique_ptr<Db> OpenReadOnly(Utf8CP fileName) {
        return Open(fileName, Db::OpenMode::Readonly);
    }
    static int GetRowCount(DbR db, Utf8CP tableName) {
        auto stmt = db.GetCachedStatement(SqlPrintfString("SELECT COUNT(*) FROM %s", tableName));
        stmt->Step();
        return stmt->GetValueInt(0);
    }
    static std::unique_ptr<BeSQLite::ChangeSet> Capture(DbR db, std::function<bool(DbR, void*)> task, void* userObj) {
        TestChangeTracker tracker(db);
        tracker.EnableTracking(true);
        if (!task(db, userObj)) {
                tracker.EnableTracking(false);
                return nullptr;
        }

        if (!tracker.HasChanges())
                return nullptr;

        std::unique_ptr<BeSQLite::ChangeSet> changeset = std::unique_ptr<BeSQLite::ChangeSet>(new TestChangeSet());
        if (BE_SQLITE_OK != changeset->FromChangeTrack(tracker))
                return nullptr;

        return changeset;
    }

public:
    BeSQliteTestFixture() : ::testing::Test() {
        BeFileName tempDir;
        BeTest::GetHost().GetTempDir(tempDir);
        BeSQLiteLib::Initialize(tempDir);
    }
};

struct DisableAsserts {
    DisableAsserts() { BeTest::SetFailOnAssert(false); }
    ~DisableAsserts() { BeTest::SetFailOnAssert(true); }
};

TEST_F(BeSQliteTestFixture, sqlite_stmt) {
    auto db1 = Create("first.db");
    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("create table test(i)"));
    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("insert into test values (zeroblob(10))"));
    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("insert into test values (zeroblob(10))"));
    db1->SaveChanges();

    auto stmt = db1->GetCachedStatement("select * from test");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());

    Statement stmt2;
    ASSERT_EQ(BE_SQLITE_OK, stmt2.Prepare(*db1, "SELECT [sql] FROM [sqlite_stmt]"));


    ASSERT_EQ(BE_SQLITE_ROW, stmt2.Step());

    ASSERT_STREQ(stmt2.GetValueText(0), "SELECT [sql] FROM [sqlite_stmt]");
    ASSERT_EQ(BE_SQLITE_ROW, stmt2.Step());
    ASSERT_STREQ(stmt2.GetValueText(0), "select * from test");
    ASSERT_EQ(BE_SQLITE_ROW, stmt2.Step());
    ASSERT_STREQ(stmt2.GetValueText(0), "SELECT 1 FROM sqlite_master where type='table' AND name=?");
    ASSERT_EQ(BE_SQLITE_ROW, stmt2.Step());
    ASSERT_STREQ(stmt2.GetValueText(0), "INSERT OR REPLACE INTO be_Prop (Namespace,Name,Id,SubId,TxnMode,RawSize,Data,StrData) VALUES(?,?,?,?,?,?,?,?)");
    ASSERT_EQ(BE_SQLITE_DONE, stmt2.Step());
}

TEST_F(BeSQliteTestFixture, WAL_basic_test) {
    DisableAsserts _notused;
    auto getFileSize = [](Utf8CP name) {
        BeFileName fileName(name);
        uint64_t sz = 0;
        auto status = fileName.GetFileSize(sz);
        return BeFileNameStatus::Success == status ? sz : -1;
    };
    Utf8String dbFileName;
    auto db1 = Create("first.db");
    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("create table test(i)"));
    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("insert into test values (zeroblob(1024))"));
    dbFileName = db1->GetDbFileName();
    ASSERT_EQ(BE_SQLITE_OK, db1->SaveChanges());

    ASSERT_FALSE(db1->IsWalMode());

    ASSERT_EQ(BE_SQLITE_OK, db1->EnableWalMode(true));
    ASSERT_TRUE(db1->IsWalMode());

    // insert many rows to force auto checkpoint
    for (int i = 0; i < 5000; i++)
        ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("insert into test values (zeroblob(1024))"));

    // open a reader from read/write connection. SaveChanges() will not do auto checkpoint because
    // there is a reader from the same connection which writing to file.
    auto stmt = db1->GetCachedStatement("select * from test");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(BE_SQLITE_OK, db1->SaveChanges());

    // open a readonly second connection just to lock the WAL file from being checkpointed.
    auto db2 = Open("first.db", Db::OpenMode::Readonly);
    auto stmt2 = db2->GetCachedStatement("select * from test");
    for (int i = 0; i < 3000; i++)
        ASSERT_EQ(BE_SQLITE_ROW, stmt2->Step());

    // wal file must have grown larger after new inserted rows.
    Utf8String walFile = dbFileName + Utf8String("-wal");
    auto walFileSize = getFileSize(walFile.c_str());
    ASSERT_EQ(6892792, walFileSize);

    // main db size should small as the all the above data was written in wal file.
    auto dbSize = getFileSize(dbFileName.c_str());
    ASSERT_EQ(36864, dbSize);

    // auto checkpoint will not happen as there is active reader db1
    stmt = nullptr;
    db1->CloseDb();

    // closing reader later will ensure WAL file is not checkpointed
    stmt2 = nullptr;
    db2->CloseDb();

    dbSize = getFileSize(dbFileName.c_str());
    ASSERT_EQ(36864, dbSize);

    // Perform an explicit checkpoint. Note this will only work if there are no active readers
    int pnLog = -1, pnCkpt = -1;
    db2 = Open("first.db", Db::OpenMode::ReadWrite);
    ASSERT_EQ(BE_SQLITE_OK, db2->PerformCheckpoint(WalCheckpointMode::Truncate, &pnLog, &pnCkpt));
    ASSERT_EQ(0, pnLog);
    ASSERT_EQ(0, pnCkpt);
    db2->CloseDb();

    // After checkpoint the main db file must grow larger and WAL file is deleted
    dbSize = getFileSize(dbFileName.c_str());
    ASSERT_EQ(6881280, dbSize);

    walFileSize = getFileSize(walFile.c_str());
    ASSERT_EQ(-1, walFileSize);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, sqlite_stat1)
    {
    auto db1 = Create("first.db");
    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("create table foo1(id integer primary key, a,b)"));
    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("create table foo2(id integer primary key, a,b)"));
    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("create table foo3(id integer primary key, a,b)"));

    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("create index idx_foo1_a_b on foo1(a,b)"));
    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("create index idx_foo2_a_b on foo2(a,b)"));

    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("create unique index uidx_foo1_a_b on foo1(a)"));
    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("create unique index uidx_foo2_a_b on foo2(a)"));


    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("insert into foo1(id,a,b) values(1,'aa1','bb1')"));
    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("insert into foo1(id,a,b) values(2,'aa2','bb2')"));
    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("insert into foo1(id,a,b) values(3,'aa3','bb3')"));
    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("insert into foo1(id,a,b) values(4,'aa4','bb4')"));
    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("insert into foo1(id,a,b) values(5,'aa5','bb5')"));
    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("insert into foo2(id,a,b) values(1,'aa1','bb1')"));
    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("insert into foo2(id,a,b) values(2,'aa2','bb2')"));
    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("insert into foo2(id,a,b) values(3,'aa3','bb3')"));
    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("insert into foo3(id,a,b) values(1,'aa1','bb1')"));

    ASSERT_TRUE(db1->TableExists("sqlite_stat1"));
    ASSERT_FALSE(db1->TableExists("sqlite_stat2"));
    ASSERT_FALSE(db1->TableExists("sqlite_stat3"));
    ASSERT_FALSE(db1->TableExists("sqlite_stat4"));

    db1->SaveChanges();
    db1->CloseDb();
    ASSERT_EQ(BeFileNameStatus::Success, Clone("first.db", "second.db"));

    db1 = OpenReadWrite("first.db");
    auto db2 = OpenReadWrite("second.db");

    ASSERT_EQ(0, GetRowCount(*db1, "sqlite_stat1"));
    ASSERT_EQ(0, GetRowCount(*db2, "sqlite_stat1"));

    //Make sure change is that system table can be tracked.
    std::unique_ptr< BeSQLite::ChangeSet> cs = Capture(*db1, [] (DbR db, void*) {
        return db.ExecuteSql("analyze") == BE_SQLITE_OK;
        }, nullptr);


    const int expectedRowCount = GetRowCount(*db1, "sqlite_stat1");
    ASSERT_NE(expectedRowCount, 0);
    db1->SaveChanges();

    //apply the change set to a new db
    ASSERT_EQ(BE_SQLITE_OK, cs->ApplyChanges(*db2));
    db2->SaveChanges();
    ASSERT_EQ(expectedRowCount, GetRowCount(*db2, "sqlite_stat1"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, variable_limit)
    {
    auto db1 = Create("first.db");
    BeTest::SetFailOnAssert(false);
    ASSERT_EQ(BE_SQLITE_ERROR, db1->ExecuteSql("SELECT ?20002")) << db1->GetLastError();
    ASSERT_EQ(BE_SQLITE_ERROR, db1->ExecuteSql("SELECT ?20001")) << db1->GetLastError();
    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("SELECT ?20000")) << db1->GetLastError(); //MAX that can fit into 16bits
    ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql("SELECT ?19999")) << db1->GetLastError();
    BeTest::SetFailOnAssert(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, regexp_basic) {
    auto db = Create("regex.db");
    db->SaveChanges();
    auto test = [&] (Utf8CP exp) {
        auto stmt = db->GetCachedStatement(SqlPrintfString("select %s", exp));
        auto rc = stmt->Step();
        if (BE_SQLITE_ROW == rc)
            return stmt->GetValueInt(0);

        return -1;
    };

    ASSERT_EQ( 1, test("'test'   REGEXP  'test'      ")); // default case sensitive
    ASSERT_EQ( 1, test("'test'   REGEXP  '(?i:TEST)' ")); // case insensitive
    ASSERT_EQ( 1, test("'test'   REGEXP  '.est'      ")); // simple dot wild card
    ASSERT_EQ( 1, test("'1234'   REGEXP  '\\d+'      ")); // class specifier
    ASSERT_EQ(-1, test("'1234'   REGEXP  '(AA'       ")); // error
    ASSERT_STRCASEEQ("regex:missing ): (AA (BE_SQLITE_ERROR)", db->GetLastError().c_str());

    // use cached & compiled regexp pattern.
    for (int i =0;i< 10;++i) {
        ASSERT_EQ( 1, test("'test'   REGEXP  '(?i:TEST)' ")); // case insensitive
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, regexp_extract_basic) {
    auto db = Create("first.db");
    auto test = [&] (Utf8CP exp) {
        auto stmt = db->GetCachedStatement(SqlPrintfString("select %s", exp));
        auto rc = stmt->Step();
        if (BE_SQLITE_ROW == rc && !stmt->IsColumnNull(0))
            return Utf8String(stmt->GetValueText(0));

        return Utf8String();
    };

    ASSERT_STRCASEEQ( "test", test(R"(REGEXP_EXTRACT('test', '\w+', '\0'))").c_str());
    ASSERT_STRCASEEQ( "test", test(R"(REGEXP_EXTRACT('test', '\w+'))").c_str());
    ASSERT_STRCASEEQ( "villa, amiyah", test(R"(REGEXP_EXTRACT('amiyah, villa', '^(\w+)\s*,\s*(\w+)$', '\2, \1'))").c_str());

    // use cached & compiled regexp pattern.
    for (int i =0;i< 10;++i) {
        ASSERT_STRCASEEQ( "villa, amiyah", test(R"(REGEXP_EXTRACT('amiyah, villa', '^(\w+)\s*,\s*(\w+)$', '\2, \1'))").c_str());
    }
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, regexp_extract_boundary_cond) {
    auto db = Create("first.db");
    db->ExecuteSql("create table foo(strCol, intCol, binCol)") ;
    db->ExecuteSql("insert into foo values ('hello, world', 1023, X'123abc')") ;
    db->ExecuteSql("insert into foo values ('world', 2033, null)") ;
    db->ExecuteSql("insert into foo values ('every, one', 2445, null)") ;
    db->ExecuteSql("insert into foo values (null, null, null)") ;
    db->SaveChanges();

    // extract or return null, with all three parameters
    auto stmt = db->GetCachedStatement(R"(select REGEXP_EXTRACT(strCol, '^(\w+)\s*,\s*(\w+)$', '\2, \1' ) from foo)");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_STREQ("world, hello", stmt->GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_STREQ(NULL, stmt->GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_STREQ("one, every", stmt->GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_STREQ(NULL, stmt->GetValueText(0));
    ASSERT_EQ(BE_SQLITE_DONE, stmt->Step());

    // extract or return null with two parameter.
    stmt = db->GetCachedStatement(R"(select REGEXP_EXTRACT(strCol, '^(\w+)\s*,\s*(\w+)$' ) from foo)");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_STREQ("hello, world", stmt->GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_STREQ(NULL, stmt->GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_STREQ("every, one", stmt->GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_STREQ(NULL, stmt->GetValueText(0));
    ASSERT_EQ(BE_SQLITE_DONE, stmt->Step());

    // extract or return null with three paramter with rewriter set to default.
    stmt = db->GetCachedStatement(R"(select REGEXP_EXTRACT(strCol, '^(\w+)\s*,\s*(\w+)$' ,'\0') from foo)");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_STREQ("hello, world", stmt->GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_STREQ(NULL, stmt->GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_STREQ("every, one", stmt->GetValueText(0));
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_STREQ(NULL, stmt->GetValueText(0));
    ASSERT_EQ(BE_SQLITE_DONE, stmt->Step());

    // searching base on extract
    stmt = db->GetCachedStatement(R"(select COUNT(*) from foo WHERE REGEXP_EXTRACT(strCol, '^(\w+)\s*,\s*(\w+)$') = 'hello, world')");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(stmt->GetValueInt(0), 1);

    // error
    stmt = db->GetCachedStatement(R"(select COUNT(*) from foo WHERE REGEXP_EXTRACT(strCol, '^(\w+\s*,\s*(\w+)$') = 'hello, world')");
    ASSERT_EQ(BE_SQLITE_ERROR, stmt->Step());
    ASSERT_STREQ("regexp_extract:missing ): ^(\\w+\\s*,\\s*(\\w+)$ (BE_SQLITE_ERROR)", db->GetLastError().c_str());

    stmt = db->GetCachedStatement(R"(select COUNT(*) from foo WHERE REGEXP_EXTRACT(234234, '^(\w+)\s*,\s*(\w+)$') = 'hello, world')");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(stmt->GetValueInt(0), 0);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, regexp_boundary_cond) {
    auto db = Create("first.db");
    db->ExecuteSql("create table foo(strCol, intCol, binCol)") ;
    db->ExecuteSql("insert into foo values ('hello', 1023, X'123abc')") ;
    db->ExecuteSql("insert into foo values ('world', 2033, null)") ;
    db->ExecuteSql("insert into foo values ('every', 2445, null)") ;
    db->ExecuteSql("insert into foo values (null, null, null)") ;
    db->SaveChanges();
    // data or regex null return null
    auto stmt = db->GetCachedStatement("select count(*) from foo where regexp(null,strCol)");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());

    // data can be null but will return null.
    stmt = db->GetCachedStatement("select count(*) from foo where regexp('\\w+',null) is null");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(stmt->GetValueInt(0), 4);

    // data match can only be of type text. If not then null is returned.
    stmt = db->GetCachedStatement("select count(*) from foo where regexp('\\w+',87987) is null");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(stmt->GetValueInt(0), 4);

    // data match can only be of type text. If not then null is returned.
    stmt = db->GetCachedStatement("select count(*) from foo where regexp('\\w+',strCol)");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(stmt->GetValueInt(0), 3);

    // not text data is treated as null
    stmt = db->GetCachedStatement("select count(*) from foo where regexp('\\w+',binCol) is null");
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(stmt->GetValueInt(0), 4);

    // regex syntax error
    stmt = db->GetCachedStatement("select count(*) from foo where regexp('(w',strCol)");
    ASSERT_EQ(BE_SQLITE_ERROR, stmt->Step());
    ASSERT_STREQ("regex:missing ): (w (BE_SQLITE_ERROR)", db->GetLastError().c_str());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, base36) {
    auto db = Create("base36.db");
    db->SaveChanges();

    auto evaluate = [&](Utf8CP exp) {
        auto stmt = db->GetCachedStatement(SqlPrintfString("select %s", exp));
        auto rc = stmt->Step();
        return (rc == BE_SQLITE_ROW) ? stmt : nullptr;
    };

    auto s = evaluate("base36(NULL)");
    ASSERT_TRUE(s->IsColumnNull(0));

    s = evaluate("base36(0)");
    ASSERT_STREQ("0", s->GetValueText(0));

    s = evaluate("base36(123)");
    ASSERT_STREQ("3F", s->GetValueText(0));

    s = evaluate("base36(123456)");
    ASSERT_STREQ("2N9C", s->GetValueText(0));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, Trace)
    {
    auto db1 = Create("first.db");
    int nStmt = 0;
    int nProfile = 0;
    int nRow = 0;
    int nClose =0;
    auto cancel_profile = db1->GetTraceProfileEvent().AddListener([&](TraceContext const& ctx, int64_t nanoseconds) { nProfile++; });
    auto cancel_stmt = db1->GetTraceStmtEvent().AddListener([&](TraceContext const& ctx, Utf8CP sql) { nStmt++; });
    auto cancel_row = db1->GetTraceRowEvent().AddListener([&](TraceContext const& ctx) { nRow++; });
    auto cancel_close = db1->GetTraceCloseEvent().AddListener([&](SqlDbP, Utf8CP) { nClose++; });
    db1->ConfigTraceEvents(DbTrace::Profile| DbTrace::Stmt | DbTrace::Row | DbTrace::Close, true);

    db1->ExecuteSql("create table test(Id integer primary key, c0);");
    Statement stmt;
    stmt.Prepare(*db1, "insert into test (id,c0) values(?,?)");
    stmt.BindInt(1, 1000);
    stmt.BindText(2, "Hello World", Statement::MakeCopy::Yes);
    stmt.Step();
    stmt.Reset();
    stmt.ClearBindings();
    stmt.BindInt(1, 2000);
    stmt.BindText(2, "Hello Under World", Statement::MakeCopy::Yes);
    stmt.Step();
    stmt.Finalize();

    ASSERT_EQ(nStmt, 3);
    ASSERT_EQ(nProfile, 3);
    ASSERT_EQ(nRow, 1);

    db1->SaveChanges();
    db1->CloseDb();

    ASSERT_EQ(nClose, 1);

    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, TraceScope)
    {
    auto db1 = Create("first.db");
    SQLiteTraceScope scope(DbTrace::Stmt | DbTrace::Profile | DbTrace::Row, *db1, "SQLiteTrace");
    db1->ExecuteSql("create table test(Id integer primary key, c0);");
    Statement stmt;
    stmt.Prepare(*db1, "insert into test (id,c0) values(?,?)");
    stmt.BindInt(1, 1000);
    stmt.BindText(2, "Hello World", Statement::MakeCopy::Yes);
    stmt.Step();
    stmt.Finalize();
    db1->SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, TxnState)
    {
    auto db = Create("first.db");
    db->ExecuteSql("create table test(Id integer primary key, c0);");

    db->SaveChanges();
    ASSERT_EQ(db->GetDbFile()->GetTxnState(), BE_SQLITE_TXN_NONE);

    auto stmt = db->GetCachedStatement("select * from sqlite_master");
    ASSERT_EQ(stmt->Step(), BE_SQLITE_ROW);
    ASSERT_EQ(db->GetDbFile()->GetTxnState(), BE_SQLITE_TXN_READ);

    stmt = db->GetCachedStatement("insert into test (id,c0) values(?,?)");
    stmt->BindInt(1, 1000);
    stmt->BindText(2, "Hello World", Statement::MakeCopy::No);
    ASSERT_EQ(BE_SQLITE_DONE, stmt->Step());

    ASSERT_EQ(db->GetDbFile()->GetTxnState(), BE_SQLITE_TXN_WRITE);
    db->SaveChanges();
    ASSERT_EQ(db->GetDbFile()->GetTxnState(), BE_SQLITE_TXN_NONE);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, ReadonlyNoCommit)
    {
    auto db = Create("readonly_commit_test.db");
    db->SaveChanges();
    db = nullptr;

    db = OpenReadOnly("readonly_commit_test.db");
    auto stmt = db->GetCachedStatement("select * from sqlite_master");
    ASSERT_EQ(stmt->Step(), BE_SQLITE_ROW);
    ASSERT_EQ(db->GetDbFile()->GetTxnState(), BE_SQLITE_TXN_READ);
    ASSERT_EQ(db->IsReadonly(), true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, SerializeMainDb)
    {
    auto db = Create("first.db");

    db->ExecuteSql("create table test1(Id integer primary key, c0);");
    auto snapshot0 = db->Serialize();
    ASSERT_EQ(snapshot0.Size(), 36864);

    db->ExecuteSql("create table test2(Id integer primary key, c0);");
    auto snapshot1 = db->Serialize();
    ASSERT_EQ(snapshot1.Size(), 40960);

    db->AbandonChanges();
    db->CloseDb();

    Db db_snapshot0;
    ASSERT_EQ(Db::Deserialize(snapshot0, db_snapshot0), BE_SQLITE_OK);
    ASSERT_TRUE(db_snapshot0.TableExists("test1"));
    ASSERT_FALSE(db_snapshot0.TableExists("test2"));
    ASSERT_TRUE(snapshot0.Empty());

    Db db_snapshot1;
    ASSERT_EQ(Db::Deserialize(snapshot1, db_snapshot1), BE_SQLITE_OK);
    ASSERT_TRUE(db_snapshot1.TableExists("test1"));
    ASSERT_TRUE(db_snapshot1.TableExists("test2"));
    ASSERT_TRUE(snapshot1.Empty());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, SerializeTempDb)
    {
    auto db = Create("first.db");

    db->ExecuteSql("create table temp.test(Id integer primary key, c0);");
    db->ExecuteSql("create table test(Id integer primary key, c0);");

    auto snapshot0 = db->Serialize("temp");
    ASSERT_EQ(snapshot0.Size(), 8192);

    auto snapshot1 = db->Serialize("main");
    ASSERT_EQ(snapshot1.Size(), 36864);

    db->AbandonChanges();
    db->CloseDb();

    Db db_snapshot0;
    ASSERT_EQ(Db::Deserialize(snapshot0, db_snapshot0), BE_SQLITE_OK);
    ASSERT_TRUE(db_snapshot0.TableExists("test"));
    ASSERT_TRUE(snapshot0.Empty());

    Db db_snapshot1;
    ASSERT_EQ(Db::Deserialize(snapshot1, db_snapshot1), BE_SQLITE_OK);
    ASSERT_TRUE(db_snapshot1.TableExists("test"));
    ASSERT_TRUE(snapshot1.Empty());
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, WriteCommitWithAssert)
    {
    auto db = Create("first.db");
    db->ExecuteSql("create table test(Id integer primary key, c0);");

    ASSERT_EQ(db->GetDbFile()->GetTxnState(), BE_SQLITE_TXN_WRITE);
    ASSERT_EQ(db->IsReadonly(), false);
    BeTest::SetFailOnAssert(false);
    db = nullptr; // Cause assert

    BeTest::SetFailOnAssert(true);
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, Profiler)
    {
    auto db1 = Create("first.db");
    ASSERT_EQ(BE_SQLITE_OK, Profiler::InitScope(*db1, "test scope", "test", Profiler::Params()));
    db1->ExecuteSql("create table test(Id integer primary key, c0);");

    auto scope = Profiler::GetScope(*db1);
    ASSERT_EQ(BE_SQLITE_OK, scope->Start());

    Statement stmt;
    stmt.Prepare(*db1, "insert into test (id,c0) values(?,?)");
    for (int i=0; i< 100; ++i) {
        stmt.BindInt(1, i);
        stmt.BindText(2, "Hello World", Statement::MakeCopy::No);
        stmt.Step();
        stmt.ClearBindings();
        stmt.Reset();
        db1->SaveChanges();
    }
    stmt.Finalize();
    ASSERT_EQ(BE_SQLITE_OK, scope->Stop());

    ASSERT_GT(scope->GetElapsedTime(), 0);
    auto scopeId = scope->GetScopeId();
    auto file = scope->GetProfileDbFileName();

    Db profileDb;
    ASSERT_EQ(BE_SQLITE_OK, profileDb.OpenBeSQLiteDb(file, Db::OpenParams(Db::OpenMode::Readonly)));
    auto stats = profileDb.GetCachedStatement("SELECT row_count, sql from v_perf_sql where scope_id=? order by sql_id");
    stats->BindInt64(1, scopeId);

    ASSERT_EQ(BE_SQLITE_ROW, stats->Step());
    ASSERT_EQ(100, stats->GetValueInt(0));
    ASSERT_STREQ( "insert into test (id,c0) values(?,?)", stats->GetValueText(1));;

    ASSERT_EQ(BE_SQLITE_ROW, stats->Step());
    ASSERT_EQ(100, stats->GetValueInt(0));
    ASSERT_STREQ( "COMMIT", stats->GetValueText(1));;
    }

//=======================================================================================
//! Virtual Table to generate series
// @bsiclass
//=======================================================================================
struct SeriesModule : DbModule {
    struct SeriesTable : DbVirtualTable {
        struct SeriesCursor : DbCursor {
            enum class Columns{
                Value = 0,
                Start = 1,
                Stop = 2,
                Step = 3,
            };
            private:
                int m_isDesc = 0;
                int64_t m_iRowid = 0;
                int64_t m_iValue = 0;
                int64_t m_mnValue = 0;
                int64_t m_mxValue = 0;
                int64_t m_iStep = 0;
            public:
                SeriesCursor(SeriesTable& vt): DbCursor(vt){}
                bool Eof() final {
                    if (m_isDesc ) {
                        return m_iValue < m_mnValue;
                    }else{
                        return m_iValue > m_mxValue;
                    }
                }
                DbResult Next() final {
                    if (m_isDesc) {
                        m_iValue -= m_iStep;
                    } else {
                        m_iValue += m_iStep;
                    }
                    m_iRowid++;
                    return BE_SQLITE_OK;
                }
                DbResult GetColumn(int i, Context& ctx) final {
                    int64_t x = 0;
                    switch( (Columns)i ){
                        case Columns::Start: x = m_mnValue; break;
                        case Columns::Stop: x = m_mxValue; break;
                        case Columns::Step: x = m_iStep;   break;
                        default: x = m_iValue;  break;
                    }
                    ctx.SetResultInt64(x);
                    return BE_SQLITE_OK;
                }
                DbResult GetRowId(int64_t& rowId) final {
                    rowId = m_iRowid;
                    return BE_SQLITE_OK;
                }
                DbResult Filter(int idxNum, const char *idxStr, int argc, DbValue* argv) final {
                    int i = 0;
                    if( idxNum & 1 ){
                        m_mnValue = argv[i++].GetValueInt64();
                    }else{
                        m_mnValue = 0;
                    }
                    if( idxNum & 2 ){
                        m_mxValue = argv[i++].GetValueInt64();
                    }else{
                        m_mxValue = 0xff;
                    }
                    if( idxNum & 4 ){
                        m_iStep = argv[i++].GetValueInt64();
                        if( m_iStep==0 ){
                        m_iStep = 1;
                        }else if( m_iStep<0 ){
                        m_iStep = -m_iStep;
                        if( (idxNum & 16)==0 ) idxNum |= 8;
                        }
                    }else{
                        m_iStep = 1;
                    }
                    for(i=0; i<argc; i++){
                        if( argv[i].IsNull() ){
                        m_mnValue = 1;
                        m_mxValue =  0;
                        break;
                        }
                    }
                    if( idxNum & 8 ){
                        m_isDesc = 1;
                        m_iValue = m_mxValue;
                        if( m_iStep>0 ){
                        m_iValue -= (m_mxValue - m_mnValue)%m_iStep;
                        }
                    }else{
                        m_isDesc = 0;
                        m_iValue = m_mnValue;
                    }
                    m_iRowid = 1;
                    return BE_SQLITE_OK;
                }
        };
        public:
            SeriesTable(SeriesModule& module): DbVirtualTable(module) {}
            DbResult Open(DbCursor*& cur) override {
                cur = new SeriesCursor(*this);
                return BE_SQLITE_OK;
            }
             DbResult BestIndex(IndexInfo& indexInfo) final {
                int i, j;              /* Loop over constraints */
                int idxNum = 0;        /* The query plan bitmask */
                int unusableMask = 0;  /* Mask of unusable constraints */
                int nArg = 0;          /* Number of arguments that seriesFilter() expects */
                int aIdx[3];           /* Constraints on start, stop, and step */
                const int SQLITE_SERIES_CONSTRAINT_VERIFY = 0;
                aIdx[0] = aIdx[1] = aIdx[2] = -1;

                for(i=0; i<indexInfo.GetConstraintCount(); i++){
                    auto pConstraint = indexInfo.GetConstraint(i);
                    int iCol;    /* 0 for start, 1 for stop, 2 for step */
                    int iMask;   /* bitmask for those column */
                    if( pConstraint->GetColumn()< (int)SeriesCursor::Columns::Start ) continue;
                    iCol = pConstraint->GetColumn() - (int)SeriesCursor::Columns::Start;
                    iMask = 1 << iCol;
                    if (!pConstraint->IsUsable()){
                        unusableMask |=  iMask;
                        continue;
                    } else if (pConstraint->GetOp() == IndexInfo::Operator::EQ ){
                        idxNum |= iMask;
                        aIdx[iCol] = i;
                    }
                }
                for( i = 0; i < 3; i++) {
                    if( (j = aIdx[i])>=0 ) {
                        indexInfo.GetConstraintUsage(j)->SetArgvIndex(++nArg);
                        indexInfo.GetConstraintUsage(j)->SetOmit(!SQLITE_SERIES_CONSTRAINT_VERIFY);
                    }
                }
                if( (unusableMask & ~idxNum)!=0 ){
                    /* The start, stop, and step columns are inputs.  Therefore if there
                    ** are unusable constraints on any of start, stop, or step then
                    ** this plan is unusable */
                    return BE_SQLITE_CONSTRAINT;
                }
                if( (idxNum & 3) == 3){
                    /* Both start= and stop= boundaries are available.  This is the
                    ** the preferred case */
                    indexInfo.SetEstimatedCost((double)(2 - ((idxNum&4)!=0)));
                    indexInfo.SetEstimatedRows(1000);
                    if( indexInfo.GetIndexOrderByCount() >= 1 && indexInfo.GetOrderBy(0)->GetColumn() == 0 ){
                    if( indexInfo.GetOrderBy(0) ->GetDesc()){
                        idxNum |= 8;
                    }else{
                        idxNum |= 16;
                    }
                    indexInfo.SetOrderByConsumed(true);
                    }
                } else {
                    /* If either boundary is missing, we have to generate a huge span
                    ** of numbers.  Make this case very expensive so that the query
                    ** planner will work hard to avoid it. */
                    indexInfo.SetEstimatedRows(2147483647);
                }
                indexInfo.SetIdxNum(idxNum);
                return BE_SQLITE_OK;
             }
    };
    public:
        SeriesModule(DbR db): DbModule(db, "generate_series", "CREATE TABLE x(value,start hidden,stop hidden,step hidden)") {}
        DbResult Connect(DbVirtualTable*& out, Config& conf, int argc, const char* const* argv) final {
            out = new SeriesTable(*this);
            conf.SetTag(Config::Tags::Innocuous);
            return BE_SQLITE_OK;
        }
};
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, TableValueFunction_SeriesModule) {
    auto db1 = Create("first.db");
    (new SeriesModule(*db1))->Register();
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*db1, "SELECT value FROM generate_series(0,100,10)"));
    int expected[] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int i = 0;
    while(stmt.Step() == BE_SQLITE_ROW) {
        ASSERT_EQ(expected[i++], stmt.GetValueInt(0));
    }
    ASSERT_EQ(i, 11);
}


//=======================================================================================
//! Virtual Table to tokenize string
// @bsiclass
//=======================================================================================
struct TokenizeModule : DbModule {
    struct TokenizeTable : DbVirtualTable {
        struct TokenizeCursor : DbCursor {
            enum class Columns{
                Token = 0,
                Text = 1,
                Delimiter =2,
            };
            private:
                int64_t m_iRowid = 0;
                Utf8String m_text;
                Utf8String m_delimiter;
                bvector<Utf8String> m_tokens;

            public:
                TokenizeCursor(TokenizeTable& vt): DbCursor(vt){}
                bool Eof() final { return m_iRowid < 1 || m_iRowid > (int64_t)m_tokens.size() ; }
                DbResult Next() final {
                    ++m_iRowid;
                    return BE_SQLITE_OK;
                }
                DbResult GetColumn(int i, Context& ctx) final {
                    Utf8CP x = 0;
                    switch( (Columns)i ){
                        case Columns::Text: x = m_text.c_str(); break;
                        case Columns::Delimiter: x = m_delimiter.c_str(); break;
                        default: x = m_tokens[m_iRowid - 1].c_str(); break;
                    }
                    ctx.SetResultText(x, (int)strlen(x), Context::CopyData::Yes);
                    return BE_SQLITE_OK;
                }
                DbResult GetRowId(int64_t& rowId) final {
                    rowId = m_iRowid;
                    return BE_SQLITE_OK;
                }
                DbResult Filter(int idxNum, const char *idxStr, int argc, DbValue* argv) final {
                    int i = 0;
                    if( idxNum & 1 ){
                        m_text = argv[i++].GetValueText();
                    }else{
                        m_text = "";
                    }
                    if( idxNum & 2 ){
                        m_delimiter = argv[i++].GetValueText();
                    }else{
                        m_delimiter = ";";
                    }
                    m_tokens.clear();
                    BeStringUtilities::Split(m_text.c_str(), m_delimiter.c_str(), m_tokens);
                    if (idxNum & 8)
                        std:: sort(m_tokens.begin(), m_tokens.end(), std::greater <>());
                    else if (idxNum & 16)
                        std:: sort(m_tokens.begin(), m_tokens.end(), std::less <>());

                    m_iRowid = 1;
                    return BE_SQLITE_OK;
                }
        };
        public:
            TokenizeTable(TokenizeModule& module): DbVirtualTable(module) {}
            DbResult Open(DbCursor*& cur) override {
                cur = new TokenizeCursor(*this);
                return BE_SQLITE_OK;
            }
             DbResult BestIndex(IndexInfo& indexInfo) final {
                 int i, j;              /* Loop over constraints */
                int idxNum = 0;        /* The query plan bitmask */
                int unusableMask = 0;  /* Mask of unusable constraints */
                int nArg = 0;          /* Number of arguments that seriesFilter() expects */
                int aIdx[2];           /* Constraints on start, stop, and step */
                const int SQLITE_SERIES_CONSTRAINT_VERIFY = 0;
                aIdx[0] = aIdx[1] = -1;
                int nConstraint = indexInfo.GetConstraintCount();

                for(i=0; i<nConstraint; i++){
                    auto pConstraint = indexInfo.GetConstraint(i);
                    int iCol;    /* 0 for start, 1 for stop, 2 for step */
                    int iMask;   /* bitmask for those column */
                    if( pConstraint->GetColumn()< (int)TokenizeCursor::Columns::Text) continue;
                    iCol = pConstraint->GetColumn() - (int)TokenizeCursor::Columns::Text;
                    iMask = 1 << iCol;
                    if (!pConstraint->IsUsable()){
                        unusableMask |=  iMask;
                        continue;
                    } else if (pConstraint->GetOp() == IndexInfo::Operator::EQ ){
                        idxNum |= iMask;
                        aIdx[iCol] = i;
                    }
                }
                for( i = 0; i < 2; i++) {
                    if( (j = aIdx[i]) >= 0 ) {
                        indexInfo.GetConstraintUsage(j)->SetArgvIndex(++nArg);
                        indexInfo.GetConstraintUsage(j)->SetOmit(!SQLITE_SERIES_CONSTRAINT_VERIFY);
                    }
                }

                if ((unusableMask & ~idxNum)!=0 ){
                    return BE_SQLITE_CONSTRAINT;
                }

                indexInfo.SetEstimatedCost(2.0);
                indexInfo.SetEstimatedRows(1000);
                if( indexInfo.GetIndexOrderByCount() >= 1 && indexInfo.GetOrderBy(0)->GetColumn() == 0 ) {
                    if( indexInfo.GetOrderBy(0) ->GetDesc()){
                        idxNum |= 8;
                    } else {
                        idxNum |= 16;
                    }
                    indexInfo.SetOrderByConsumed(true);
                }
                indexInfo.SetIdxNum(idxNum);
                return BE_SQLITE_OK;
             }
    };
    public:
        TokenizeModule(DbR db): DbModule(db, "tokenize_text", "CREATE TABLE x(token,buffer hidden,delimiter hidden)") {}
        DbResult Connect(DbVirtualTable*& out, Config& conf, int argc, const char* const* argv) final {
            out = new TokenizeTable(*this);
            conf.SetTag(Config::Tags::Innocuous);
            return BE_SQLITE_OK;
        }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, TableValueFunction_TokenizeModule) {
    auto db1 = Create("first.db");
    (new TokenizeModule(*db1))->Register();
    if ("unsorted") {
        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*db1, "SELECT token FROM tokenize_text('The quick brown fox jumps over the lazy dog', ' ')"));
        auto expected = std::vector<std::string>{"The", "quick", "brown", "fox", "jumps", "over", "the", "lazy", "dog"};
        int i = 0;
        while(stmt.Step() == BE_SQLITE_ROW) {
            ASSERT_STREQ(expected[i++].c_str(), stmt.GetValueText(0));
        }
        ASSERT_EQ(i, 9);
    }
    if ("sorted ascending") {
        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*db1, "SELECT token FROM tokenize_text('the quick brown fox jumps over the lazy dog', ' ') ORDER BY token"));
        auto expected = std::vector<std::string>{"brown", "dog", "fox", "jumps", "lazy", "over", "quick", "the", "the"};
        int i = 0;
        while(stmt.Step() == BE_SQLITE_ROW) {
            ASSERT_STREQ(expected[i++].c_str(), stmt.GetValueText(0));
        }
        ASSERT_EQ(i, 9);
    }
    if ("sorted descending") {
        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*db1, "SELECT token FROM tokenize_text('the quick brown fox jumps over the lazy dog', ' ') ORDER BY token DESC"));
        auto expected = std::vector<std::string>{"the", "the", "quick", "over", "lazy", "jumps", "fox", "dog", "brown"};
        int i = 0;
        while(stmt.Step() == BE_SQLITE_ROW) {
            ASSERT_STREQ(expected[i++].c_str(), stmt.GetValueText(0));
        }
        ASSERT_EQ(i, 9);
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, IntegrityCheckShouldRunOnReadOnlyFileWithFTS5) {
    BeFileName testFileWithFts5;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(testFileWithFts5);
    testFileWithFts5.AppendUtf8("BeSQLiteTestData/test.bim");
    Db db;
    ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(testFileWithFts5, Db::OpenParams(Db::OpenMode::Readonly)));
    auto stmt = db.GetCachedStatement("PRAGMA integrity_check");
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(BE_SQLITE_ROW, stmt->Step());
    ASSERT_EQ(BE_SQLITE_DONE, stmt->Step());
    stmt = nullptr;
    db.CloseDb();
}

#ifdef ANALYZE_MEMORY_USAGE
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void SqliteMemoryTest(bool stmtjournalInMemory, bool useNestedTransaction, const int threadshold, bool testMustPass)
    {
    std::function<std::unique_ptr<Db>(Utf8CP)> create
        = [] (Utf8CP fileName)
        {
        BeFileName outputPath;
        BeTest::GetHost().GetOutputRoot(outputPath);
        outputPath.AppendUtf8(fileName);
        if (outputPath.DoesPathExist())
            outputPath.BeDeleteFile();

        std::unique_ptr<Db> db = std::unique_ptr<Db>(new Db());
        if (db->CreateNewDb(outputPath) != BE_SQLITE_OK)
            db = nullptr;

        return db;
        };

    std::function<std::unique_ptr< BeSQLite::ChangeSet>(DbR db, std::function<bool(DbR)>)> captureChangeset
        = [] (DbR db, std::function<bool(DbR db)> task)
        {
        struct MemmoryTestChangeTracker : BeSQLite::ChangeTracker
            {
            MemmoryTestChangeTracker(BeSQLite::DbR db) { SetDb(&db); }
            OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) override { return OnCommitStatus::Commit; }
            TrackChangesForTable _FilterTable(Utf8CP tableName) override
                {
                if (BeStringUtilities::StricmpAscii(tableName, "t1") == 0)
                    return TrackChangesForTable::No;

                if (BeStringUtilities::StricmpAscii(tableName, "t2") == 0)
                    return TrackChangesForTable::No;

                if (BeStringUtilities::StricmpAscii(tableName, "t3") == 0)
                    return TrackChangesForTable::No;

                return TrackChangesForTable::Yes;
                }
            };

        std::unique_ptr< BeSQLite::ChangeSet> changeset;
        MemmoryTestChangeTracker tracker(db);
        tracker.EnableTracking(true);
        if (!task(db))
            {
            tracker.EnableTracking(false);
            return changeset;
            }

        if (!tracker.HasChanges())
            return changeset;

        changeset = std::unique_ptr< BeSQLite::ChangeSet>(new TestChangeSet());
        if (BE_SQLITE_OK != changeset->FromChangeTrack(tracker))
            changeset = nullptr;

        return changeset;
        };

    Utf8CP schemaDb = R"(
        create table if not exists T0(Id integer primary key, c0, c1, c2, c3);
        create table if not exists T1(Id integer primary key, c0, c1, c2, c3);
        create table if not exists T2(Id integer primary key, c0, c1, c2, c3);
        create table if not exists T3(Id integer primary key, c0, c1, c2, c3);
        --
        create trigger if not exists t0_i after insert on t0
               when cast(new.c0 as real) between 0.0 and 0.8
        begin
             insert into t1
                    values(null, new.c0, new.c1, new.c2, new.c3);
        end;
        --
        create trigger if not exists t1_i after insert on t1
               when cast(new.c0 as real) between 0.0 and 0.6
        begin
             insert into t2
                    values(null, new.c0, new.c1, new.c2, new.c3);
        end;
        --
        create trigger if not exists t2_i after insert on t2
               when cast(new.c0 as real) between 0.0 and 0.4
        begin
             insert into t3
                    values(null, new.c0, new.c1, new.c2, new.c3);
        end; )";


    BeFileName changesetFile;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(changesetFile);
    changesetFile.AppendUtf8("BeSQLiteTestData\\mem_test.changeset");

    if (!changesetFile.DoesPathExist())
        {
        auto db1 = create("mem_check.db");
        ASSERT_EQ(BE_SQLITE_OK, db1->ExecuteSql(schemaDb));
        ASSERT_EQ(BE_SQLITE_OK, db1->SaveChanges());
        std::unique_ptr< BeSQLite::ChangeSet> cs = captureChangeset(*db1, [] (DbR db)
            {
            Statement stmt;
            EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "insert into t0 values(null, ?1, ?2, ?3, ?4)"));
            const int max = 10000;
            for (int i = 0; i < max; i++)
                {
                stmt.Reset();
                stmt.ClearBindings();
                stmt.BindDouble(1, (rand() / static_cast<float>(RAND_MAX)));
                stmt.BindDouble(2, (rand() / static_cast<float>(RAND_MAX)));
                stmt.BindDouble(3, (rand() / static_cast<float>(RAND_MAX)));
                stmt.BindDouble(4, (rand() / static_cast<float>(RAND_MAX)));
                EXPECT_EQ(BE_SQLITE_DONE, stmt.Step());
                }

            return true;
            });

        db1->CloseDb();
        LOG.debugv("Writing Changeset ... %s [size=%d bytes]", changesetFile.GetNameUtf8().c_str(), cs->GetSize());

        BeFile file;
        ASSERT_EQ(BeFileStatus::Success, file.Create(changesetFile.GetName()));
        for (auto& chunk : cs->m_data.m_chunks) {
            ASSERT_EQ(BeFileStatus::Success, file.Write(nullptr, chunk.data(), (uint32_t) chunk.size()));
        }
        ASSERT_EQ(BeFileStatus::Success, file.Flush());
        ASSERT_EQ(BeFileStatus::Success, file.Close());
        }

    int64_t currentMemBefore, highMemBefore;
    BeSQLiteLib::GetMemoryUsed(currentMemBefore, highMemBefore);
    LOG.debugv("Before  Current=%lld, High= %lld\n", currentMemBefore, highMemBefore);

    TestChangeSet cs;
    if (true)
        {
        BeFile file;
        ASSERT_EQ(BeFileStatus::Success, file.Open(changesetFile.GetName(), BeFileAccess::Read));
        ByteStream stream;
        ASSERT_EQ(BeFileStatus::Success, file.ReadEntireFile(stream));
        cs.m_data.Append(stream.GetData(), stream.GetSize());
        LOG.debugv("Reading Changeset ... %s [size=%d bytes]", changesetFile.GetNameUtf8().c_str(), cs.GetSize());
        }

    auto db2 = create("mem_check2.db");

    if (stmtjournalInMemory)
        {
        ASSERT_EQ(BE_SQLITE_OK, db2->ExecuteSql("PRAGMA temp_store=2;"));
        }

    ASSERT_EQ(BE_SQLITE_OK, db2->ExecuteSql(schemaDb));
    ASSERT_EQ(BE_SQLITE_OK, db2->SaveChanges());
    std::unique_ptr<Savepoint> sp;
    if (useNestedTransaction)
        {
        sp = std::unique_ptr<Savepoint>(new Savepoint(*db2, "foo"));
        }

    ASSERT_EQ(BE_SQLITE_OK, cs.ApplyChanges(*db2));
    if (useNestedTransaction)
        {
        ASSERT_EQ(BE_SQLITE_OK, sp->Commit());
        }

    int64_t currentMemAfter, highMemAfter;
    BeSQLiteLib::GetMemoryUsed(currentMemAfter, highMemAfter);
    LOG.debugv("After  Current=%lld, High= %lld\n", currentMemAfter - currentMemBefore, highMemAfter - highMemBefore, true /* reset stats*/);

    if (!stmtjournalInMemory && useNestedTransaction)
        {
        BeFileName tempDir;
        BeTest::GetHost().GetTempDir(tempDir);
        bvector<BeFileName> tempFiles;
        BeDirectoryIterator::WalkDirsAndMatch(tempFiles, tempDir, L"etilqs_*", false);
        ASSERT_EQ(tempFiles.size(), 1) << "We expect only one file in the temp folder";
        uint64_t fsz;
        ASSERT_EQ(BeFileNameStatus::Success, tempFiles.front().GetFileSize(fsz));
        LOG.debugv("Best Guess at 'Statement Journal File' : %s (%lld Bytes)", tempFiles.front().GetNameUtf8().c_str(), fsz);

        if (testMustPass)
            {
            ASSERT_GT(fsz, threadshold);
            ASSERT_LT(highMemAfter - highMemBefore, threadshold);
            }
        else
            {
            ASSERT_LT(fsz, threadshold);
            ASSERT_GT(highMemAfter - highMemBefore, threadshold);
            }
        }
    else
        {
        if (testMustPass)
            ASSERT_LT(highMemAfter - highMemBefore, threadshold);
        else
            ASSERT_GT(highMemAfter - highMemBefore, threadshold);
        }

    ASSERT_EQ(BE_SQLITE_OK, db2->SaveChanges());

    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, SQLiteMemCheck_UseDiskJournal_NoNestedTransaction)
    {
    SqliteMemoryTest(false, false, MEM_THRESHOLD, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, SQLiteMemCheck_UseInMemoryJournal_NoNestedTransaction)
    {
    SqliteMemoryTest(true, false, MEM_THRESHOLD, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, SQLiteMemCheck_UseInMemoryJournal_UseNestedTransaction)
    {
    SqliteMemoryTest(true, true, MEM_THRESHOLD, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, SQLiteMemCheck_UseInDiskJournal_UseNestedTransaction)
    {
    SqliteMemoryTest(false, true, MEM_THRESHOLD, true);
    }
#endif