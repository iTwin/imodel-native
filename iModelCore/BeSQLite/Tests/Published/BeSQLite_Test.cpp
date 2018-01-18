/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/BeSQLite_Test.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BeSQLitePublishedTests.h"
#include <BeSQLite/ChangeSet.h>
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