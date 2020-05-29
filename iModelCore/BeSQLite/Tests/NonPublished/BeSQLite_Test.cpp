/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "BeSQLiteNonPublishedTests.h"
#include <BeSQLite/ChangeSet.h>
#include <Bentley/BeDirectoryIterator.h>

using namespace MemorySize;

#define MEM_THRESHOLD (100 * MEG)

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   06/16
//=======================================================================================
struct BeIdSetTests : public ::testing::Test
{
public:
    void ExpectRoundTrip(BeIdSet const& ids, Utf8CP expected);
    BeIdSet MakeIdSet(std::initializer_list<int> values)
        {
        BeIdSet ids;
        for (auto value : values)
            ids.insert(BeInt64Id(static_cast<int64_t>(value)));

        return ids;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void BeIdSetTests::ExpectRoundTrip(BeIdSet const& ids, Utf8CP expected)
    {
    Utf8String actual = ids.ToCompactString();
    EXPECT_TRUE(actual.Equals(expected)) << "Expected: " << expected << " Actual: " << actual.c_str();
    BeIdSet roundtripped;
    roundtripped.FromString(actual);
    EXPECT_TRUE(roundtripped == ids) << " Expected: " << ids.ToString().c_str() << " Actual: " << roundtripped.ToString().c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
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
    }

    enum {
        TEST_DATA_SIZE = 160 * K,
    };

static void verifyRead(ChunkedArray const& array, int numInts, int totalSize) {
    bvector<int> bytes(numInts, 333);
    int* data =  bytes.data();

    ChunkedArray::Reader reader(array);
    int index = 0;
    int total = 0;
    int size = 0;
    do {
        size = (int) bytes.size() * sizeof(int);
        reader.Read((Byte*)data, &size);
        total += size;
        for (int i=0; i<(size/sizeof(int)); ++i)
            EXPECT_TRUE(data[i] == index++);
    } while (size > 0);

    EXPECT_TRUE(total == totalSize);
}

static void verifyReadSizes(ChunkedArray const& array, int totalSize) {
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
 @bsimethod                                    Keith.Bentley                    05/20
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeIdSetTests, ChunkedArray) {
    int totalSize = TEST_DATA_SIZE * sizeof(int);
    int* testData = (int*)malloc(totalSize);
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

    DbSchemaChangeSet schema;
    schema.AddDDL("Test1");
    schema.AddDDL("Test2");
    Utf8String ddl = schema.ToString();
    EXPECT_EQ(ddl, "Test1;Test2");

    DbSchemaChangeSet schema1("This is a Test", 5);
    schema1.AddDDL("This is another test");
    schema1.AddDDL("This is a 3rd test");
    ddl = schema1.ToString();
    EXPECT_EQ(ddl, "This is a Test;This is another test;This is a 3rd test");
}


//=======================================================================================
// @bsiclass                                                  Affan.Khan   01/18
//=======================================================================================
struct TestChangeSet : BeSQLite::ChangeSet
    {
    ConflictResolution _OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) override { BeAssert(false && "Unexpected conflict"); return ConflictResolution::Skip; }
    };

//=======================================================================================
// @bsiclass                                                 Affan.Khan   01/18
//=======================================================================================
struct TestChangeTracker : BeSQLite::ChangeTracker
    {
    TestChangeTracker(BeSQLite::DbR db) { SetDb(&db); }

    OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) override { return OnCommitStatus::Continue; }

    };

//=======================================================================================
// @bsistruct                                                   Affan.Khan   01/18
//=======================================================================================
struct BeSQliteTestFixture : public ::testing::Test
    {
    protected:

        static std::unique_ptr<Db> Create(Utf8CP fileName)
            {
            BeFileName outputPath;
            BeTest::GetHost().GetOutputRoot(outputPath);
            outputPath.AppendUtf8(fileName);
            if (outputPath.DoesPathExist())
                outputPath.BeDeleteFile();

            std::unique_ptr<Db> db = std::unique_ptr<Db>(new Db());
            if (db->CreateNewDb(outputPath) != BE_SQLITE_OK)
                return nullptr;

            return db;
            }

        static BeFileNameStatus Clone(Utf8CP existingFile, Utf8CP out, bool override =true)
            {
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
                else
                    {
                    BeFileNameStatus r = outputPathB.BeDeleteFile();
                    if (r != BeFileNameStatus::Success)
                        return r;
                    }

            return BeFileName::BeCopyFile(existingFilePath, outputPathB);
            }
        static std::unique_ptr<Db> Open(Utf8CP fileName, Db::OpenMode openMode)
            {
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

        static std::unique_ptr<Db> OpenReadWrite(Utf8CP fileName)
            {
            return Open(fileName, Db::OpenMode::ReadWrite);
            }
        static std::unique_ptr<Db> OpenReadOnly(Utf8CP fileName)
            {
            return Open(fileName, Db::OpenMode::Readonly);
            }
        static int GetRowCount(DbR db, Utf8CP tableName)
            {
            auto stmt = db.GetCachedStatement(SqlPrintfString("SELECT COUNT(*) FROM %s", tableName));
            stmt->Step();
            return  stmt->GetValueInt(0);
            }
        static std::unique_ptr< BeSQLite::ChangeSet> Capture(DbR db, std::function<bool(DbR,void*)> task, void* userObj)
            {
            TestChangeTracker tracker(db);
            tracker.EnableTracking(true);
            if (!task(db, userObj))
                {
                tracker.EnableTracking(false);
                return nullptr;
                }

            if (!tracker.HasChanges())
                return nullptr;

            std::unique_ptr< BeSQLite::ChangeSet> changeset = std::unique_ptr< BeSQLite::ChangeSet>(new TestChangeSet());
            if (BE_SQLITE_OK != changeset->FromChangeTrack(tracker))
                return nullptr;

            return changeset;
            }
    public:
        BeSQliteTestFixture():
            ::testing::Test()
            {
            BeFileName tempDir;
            BeTest::GetHost().GetTempDir(tempDir);
            BeSQLiteLib::Initialize(tempDir);
            }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                             1/18
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
// @bsimethod                                Affan.Khan                             1/18
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
// @bsimethod                                Affan.Khan                             3/20
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, Trace)
    {
    auto db1 = Create("first.db");
    int nStmt = 0;
    int nProfile = 0;
    int nRow = 0;
    db1->ConfigureTrace(
        (DbTrace)(DbTrace::BE_SQLITE_TRACE_STMT |
                  DbTrace::BE_SQLITE_TRACE_PROFILE |
                  DbTrace::BE_SQLITE_TRACE_ROW),
        [&](TraceContext const& ctx, Utf8CP sql) {
            nStmt++;
        },
        [&](TraceContext const& ctx, int64_t nanoseconds) {
            nProfile++;
        },
        [&](TraceContext const& ctx) {
            nRow++;
        });
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
    db1->CloseDb();

    ASSERT_EQ(nStmt, 3);
    ASSERT_EQ(nProfile, 3);
    ASSERT_EQ(nRow, 1);
    }

    //---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                             3/20
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, TraceScope)
    {
    auto db1 = Create("first.db");
    SQLiteTraceScope scope((DbTrace)(DbTrace::BE_SQLITE_TRACE_STMT |
                  DbTrace::BE_SQLITE_TRACE_PROFILE |
                  DbTrace::BE_SQLITE_TRACE_ROW), *db1, "SQLiteTrace");
    db1->ExecuteSql("create table test(Id integer primary key, c0);");
    Statement stmt;
    stmt.Prepare(*db1, "insert into test (id,c0) values(?,?)");
    stmt.BindInt(1, 1000);
    stmt.BindText(2, "Hello World", Statement::MakeCopy::Yes);
    stmt.Step();
    stmt.Finalize();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                             4/18
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
            OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) override { return OnCommitStatus::Continue; }
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
                stmt.BindDouble(1, (rand() / (RAND_MAX*1.0f)));
                stmt.BindDouble(2, (rand() / (RAND_MAX*1.0f)));
                stmt.BindDouble(3, (rand() / (RAND_MAX*1.0f)));
                stmt.BindDouble(4, (rand() / (RAND_MAX*1.0f)));
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
// @bsimethod                                Affan.Khan                             4/18
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, SQLiteMemCheck_UseDiskJournal_NoNestedTransaction)
    {
    SqliteMemoryTest(false, false, MEM_THRESHOLD, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                             4/18
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, SQLiteMemCheck_UseInMemoryJournal_NoNestedTransaction)
    {
    SqliteMemoryTest(true, false, MEM_THRESHOLD, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                             4/18
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, SQLiteMemCheck_UseInMemoryJournal_UseNestedTransaction)
    {
    SqliteMemoryTest(true, true, MEM_THRESHOLD, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                             4/18
//---------------------------------------------------------------------------------------
TEST_F(BeSQliteTestFixture, SQLiteMemCheck_UseInDiskJournal_UseNestedTransaction)
    {
    SqliteMemoryTest(false, true, MEM_THRESHOLD, true);
    }