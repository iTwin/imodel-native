/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECCrudWriteTokenTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
struct ECCrudWriteTokenTestFixture : public ECDbTestFixture
    {
protected:
    struct RestrictedECDb : ECDb
        {
        private:
            ECCrudWriteToken const* m_token;

        public:
            explicit RestrictedECDb() : ECDb()
                {
                m_token = &EnableECCrudWriteTokenValidation();
                }

            ECCrudWriteToken const* GetToken() const { return m_token; }
        };
    };


//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECCrudWriteTokenTestFixture, Test)
{
    enum class ExpectedResult
        {
        Success,
        Error
        };

    BeFileName seedFilePath;
    ECInstanceKey blobIoInstanceKey;
    {
    SetupECDb("eccrudwritetoken_seed.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());
    seedFilePath.AssignUtf8(GetECDb().GetDbFileName());

    //insert a row with zeroblob which is preprequisite for using blobio
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ecsql.PSA(Bi) VALUES(zeroblob(10))"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(blobIoInstanceKey)) << stmt.GetECSql();
    stmt.Finalize();
    GetECDb().SaveChanges();
    GetECDb().CloseDb();
    }

    ECDb readonlyECDb, permittedECDb;
    ASSERT_EQ(BE_SQLITE_OK, CloneECDb(readonlyECDb, "eccrudwritetoken_readonly.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(BE_SQLITE_OK, CloneECDb(permittedECDb, "eccrudwritetoken_permitted.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    RestrictedECDb restrictedECDb;
    ASSERT_EQ(BE_SQLITE_OK, CloneECDb(restrictedECDb, "eccrudwritetoken_restricted.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));

    bvector<Utf8CP> selectECSqls;
    selectECSqls.push_back("SELECT * FROM ecsql.P");
    selectECSqls.push_back("SELECT * FROM ecsql.PSAHasP");

    bvector<Utf8CP> nonSelectECSqls;
    nonSelectECSqls.push_back("INSERT INTO ecsql.P(L,S) VALUES(1000,'fasda')");
    nonSelectECSqls.push_back("UPDATE ecsql.P SET L=1123");
    nonSelectECSqls.push_back("DELETE FROM ecsql.P");

    auto runSelectECSql = [] (ECDbCR ecdb, Utf8CP ecsql)
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql)) << "ECDb: " << ecdb.GetDbFileName() << " ECSQL: " << ecsql;
        while (BE_SQLITE_ROW == stmt.Step())
            {
            }
        };
    
    for (Utf8CP selectECSql : selectECSqls)
        {
        runSelectECSql(readonlyECDb, selectECSql);
        runSelectECSql(permittedECDb, selectECSql);
        runSelectECSql(restrictedECDb, selectECSql);
        }

    auto runNonSelectECSql = [] (ECDbCR ecdb, Utf8CP ecsql, ECCrudWriteToken const* writeToken, ExpectedResult expected)
        {
        ECSqlStatement stmt;
        ECSqlStatus stat = stmt.Prepare(ecdb, ecsql, writeToken);
        if (ExpectedResult::Success == expected)
            {
            ASSERT_EQ(ECSqlStatus::Success, stat) << "ECDb: " << ecdb.GetDbFileName() << " ECSQL: " << ecsql;
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "ECDb: " << ecdb.GetDbFileName() << " ECSQL: " << ecsql;
            }
        else
            ASSERT_EQ(ECSqlStatus::Error, stat) << "ECDb: " << ecdb.GetDbFileName() << " ECSQL: " << ecsql;
        };

    for (Utf8CP nonSelectECSql : nonSelectECSqls)
        {
        runNonSelectECSql(readonlyECDb, nonSelectECSql, nullptr, ExpectedResult::Error);
        runNonSelectECSql(permittedECDb, nonSelectECSql, nullptr, ExpectedResult::Success);
        runNonSelectECSql(restrictedECDb, nonSelectECSql, nullptr, ExpectedResult::Error);
        runNonSelectECSql(restrictedECDb, nonSelectECSql, restrictedECDb.GetToken(), ExpectedResult::Success);
        }

    auto runNonSelectECSqlFromCache = [] (ECDbCR ecdb, ECSqlStatementCache& cache, Utf8CP ecsql, ECCrudWriteToken const* writeToken, ExpectedResult expected)
        {
        Utf8CP writeTokenStr = writeToken != nullptr ? "yes" : "no";
        CachedECSqlStatementPtr stmt = cache.GetPreparedStatement(ecdb, ecsql, writeToken);
        if (ExpectedResult::Success == expected)
            {
            ASSERT_TRUE(stmt != nullptr) << "ECDb: " << ecdb.GetDbFileName() << " ECSQL: " << ecsql << " WriteToken: " << writeTokenStr;
            ASSERT_EQ(BE_SQLITE_DONE, stmt->Step()) << "ECDb: " << ecdb.GetDbFileName() << " ECSQL: " << ecsql << " WriteToken: " << writeTokenStr;
            }
        else
            ASSERT_TRUE(stmt == nullptr) << "ECDb: " << ecdb.GetDbFileName() << " ECSQL: " << ecsql << " WriteToken: " << writeTokenStr;
        };

    ECSqlStatementCache readonlyCache(10);
    ECSqlStatementCache permittedCache(10);
    ECSqlStatementCache restrictedCache(10);
    ECSqlStatementCache restrictedCacheWithToken(10);
    for (Utf8CP nonSelectECSql : nonSelectECSqls)
        {
        runNonSelectECSqlFromCache(readonlyECDb, readonlyCache, nonSelectECSql, nullptr, ExpectedResult::Error);
        runNonSelectECSqlFromCache(permittedECDb, permittedCache, nonSelectECSql, nullptr, ExpectedResult::Success);
        runNonSelectECSqlFromCache(restrictedECDb, restrictedCache, nonSelectECSql, nullptr, ExpectedResult::Error);
        runNonSelectECSqlFromCache(restrictedECDb, restrictedCacheWithToken, nonSelectECSql, restrictedECDb.GetToken(), ExpectedResult::Success);
        }


    auto runAdapters = [] (ECDbCR ecdb, ECCrudWriteToken const* writeToken, ExpectedResult expected)
        {
        ECClassCP testClass = ecdb.Schemas().GetECClass("ECSqlTest", "P");
        ASSERT_TRUE(testClass != nullptr);
        Utf8CP writeTokenStr = writeToken != nullptr ? "yes" : "no";

        {
        ECInstanceInserter ecInserter(ecdb, *testClass, writeToken);
        ASSERT_EQ(expected == ExpectedResult::Success, ecInserter.IsValid()) << "ECDb: " << ecdb.GetDbFileName() << " WriteToken: " << writeTokenStr;
        }

        {
        ECInstanceUpdater ecUpdater(ecdb, *testClass, writeToken);
        ASSERT_EQ(expected == ExpectedResult::Success, ecUpdater.IsValid()) << "ECDb: " << ecdb.GetDbFileName() << " WriteToken: " << writeTokenStr;
        }

        {
        ECInstanceDeleter ecDeleter(ecdb, *testClass, writeToken);
        ASSERT_EQ(expected == ExpectedResult::Success, ecDeleter.IsValid()) << "ECDb: " << ecdb.GetDbFileName() << " WriteToken: " << writeTokenStr;
        }

        {
        JsonInserter jsonInserter(ecdb, *testClass, writeToken);
        ASSERT_EQ(expected == ExpectedResult::Success, jsonInserter.IsValid()) << "ECDb: " << ecdb.GetDbFileName() << " WriteToken: " << writeTokenStr;
        }

        {
        JsonUpdater jsonUpdater(ecdb, *testClass, writeToken);
        ASSERT_EQ(expected == ExpectedResult::Success, jsonUpdater.IsValid()) << "ECDb: " << ecdb.GetDbFileName() << " WriteToken: " << writeTokenStr;
        }

        {
        JsonDeleter jsonDeleter(ecdb, *testClass, writeToken);
        ASSERT_EQ(expected == ExpectedResult::Success, jsonDeleter.IsValid()) << "ECDb: " << ecdb.GetDbFileName() << " WriteToken: " << writeTokenStr;
        }

        };

    runAdapters(readonlyECDb, nullptr, ExpectedResult::Error);
    runAdapters(permittedECDb, nullptr, ExpectedResult::Success);
    runAdapters(restrictedECDb, nullptr, ExpectedResult::Error);
    runAdapters(restrictedECDb, restrictedECDb.GetToken(), ExpectedResult::Success);

    //BlobIO test
    enum class BlobIoOpenMode
        {
        Readonly,
        ReadWrite
        };
    auto openBlobIO = [] (ECDbCR ecdb, ECInstanceKey const& key, BlobIoOpenMode openMode, ECCrudWriteToken const* writeToken, ExpectedResult expected)
        {
        ECClassCP testClass = ecdb.Schemas().GetECClass("ECSqlTest", "PSA");
        ASSERT_TRUE(testClass != nullptr);
        Utf8CP writeTokenStr = writeToken != nullptr ? "yes" : "no";
        Utf8CP openModeStr = openMode == BlobIoOpenMode::Readonly ? "readonly" : "readwrite";

        BlobIO io;
        ASSERT_EQ(expected == ExpectedResult::Success, SUCCESS == ecdb.OpenBlobIO(io, *testClass, "Bi", key.GetECInstanceId(), openMode == BlobIoOpenMode::ReadWrite, writeToken)) << "ECDb: " << ecdb.GetDbFileName() << " openMode: " << openModeStr << " WriteToken: " << writeTokenStr;
        ASSERT_EQ(expected == ExpectedResult::Success, io.IsValid()) << "ECDb: " << ecdb.GetDbFileName() << " openMode: " << openModeStr << " WriteToken: " << writeTokenStr;
        };

    openBlobIO(readonlyECDb, blobIoInstanceKey, BlobIoOpenMode::ReadWrite, nullptr, ExpectedResult::Error);
    openBlobIO(readonlyECDb, blobIoInstanceKey, BlobIoOpenMode::Readonly, nullptr, ExpectedResult::Success);
    openBlobIO(permittedECDb, blobIoInstanceKey, BlobIoOpenMode::ReadWrite, nullptr, ExpectedResult::Success);
    openBlobIO(permittedECDb, blobIoInstanceKey, BlobIoOpenMode::Readonly, nullptr, ExpectedResult::Success);
    openBlobIO(restrictedECDb, blobIoInstanceKey, BlobIoOpenMode::ReadWrite, nullptr, ExpectedResult::Error);
    openBlobIO(restrictedECDb, blobIoInstanceKey, BlobIoOpenMode::Readonly, nullptr, ExpectedResult::Success);
    openBlobIO(restrictedECDb, blobIoInstanceKey, BlobIoOpenMode::ReadWrite, restrictedECDb.GetToken(), ExpectedResult::Success);
    openBlobIO(restrictedECDb, blobIoInstanceKey, BlobIoOpenMode::Readonly, restrictedECDb.GetToken(), ExpectedResult::Success);
    }

END_ECDBUNITTESTS_NAMESPACE
