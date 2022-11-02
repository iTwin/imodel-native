/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct ECCrudWriteTokenTestFixture : public ECDbTestFixture
    {
protected:
    struct RestrictedECDb : ECDb
        {
        public:
            explicit RestrictedECDb() : ECDb() { ApplyECDbSettings(true, false); }
            ECCrudWriteToken const* GetToken() const { return GetECDbSettingsManager().GetCrudWriteToken(); }
        };
    };


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECCrudWriteTokenTestFixture, Test)
{
    BeFileName seedFilePath;
    ECInstanceKey blobIoInstanceKey;
    {
    ASSERT_EQ(SUCCESS, SetupECDb("eccrudwritetoken_seed.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    seedFilePath.AssignUtf8(m_ecdb.GetDbFileName());

    //insert a row with zeroblob which is preprequisite for using blobio
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ecsql.PSA(Bi) VALUES(zeroblob(10))"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(blobIoInstanceKey)) << stmt.GetECSql();
    stmt.Finalize();
    m_ecdb.SaveChanges();
    m_ecdb.CloseDb();
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

    auto runNonSelectECSql = [] (ECDbCR ecdb, Utf8CP ecsql, ECCrudWriteToken const* writeToken, BentleyStatus expectedStat)
        {
        ECSqlStatement stmt;
        ECSqlStatus stat = stmt.Prepare(ecdb, ecsql, writeToken);
        if (SUCCESS == expectedStat)
            {
            ASSERT_EQ(ECSqlStatus::Success, stat) << "ECDb: " << ecdb.GetDbFileName() << " ECSQL: " << ecsql;
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "ECDb: " << ecdb.GetDbFileName() << " ECSQL: " << ecsql;
            }
        else
            ASSERT_EQ(ECSqlStatus::Error, stat) << "ECDb: " << ecdb.GetDbFileName() << " ECSQL: " << ecsql;
        };

    for (Utf8CP nonSelectECSql : nonSelectECSqls)
        {
        runNonSelectECSql(readonlyECDb, nonSelectECSql, nullptr, ERROR);
        runNonSelectECSql(permittedECDb, nonSelectECSql, nullptr, SUCCESS);
        runNonSelectECSql(restrictedECDb, nonSelectECSql, nullptr, ERROR);
        runNonSelectECSql(restrictedECDb, nonSelectECSql, restrictedECDb.GetToken(), SUCCESS);
        }

    auto runNonSelectECSqlFromCache = [] (ECDbCR ecdb, ECSqlStatementCache& cache, Utf8CP ecsql, ECCrudWriteToken const* writeToken, BentleyStatus expectedStat)
        {
        Utf8CP writeTokenStr = writeToken != nullptr ? "yes" : "no";
        CachedECSqlStatementPtr stmt = cache.GetPreparedStatement(ecdb, ecsql, writeToken);
        if (SUCCESS == expectedStat)
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
        runNonSelectECSqlFromCache(readonlyECDb, readonlyCache, nonSelectECSql, nullptr, ERROR);
        runNonSelectECSqlFromCache(permittedECDb, permittedCache, nonSelectECSql, nullptr, SUCCESS);
        runNonSelectECSqlFromCache(restrictedECDb, restrictedCache, nonSelectECSql, nullptr, ERROR);
        runNonSelectECSqlFromCache(restrictedECDb, restrictedCacheWithToken, nonSelectECSql, restrictedECDb.GetToken(), SUCCESS);
        }


    auto runAdapters = [] (ECDbCR ecdb, ECCrudWriteToken const* writeToken, bool expectedIsValid)
        {
        ECClassCP testClass = ecdb.Schemas().GetClass("ECSqlTest", "P");
        ASSERT_TRUE(testClass != nullptr);
        Utf8CP writeTokenStr = writeToken != nullptr ? "yes" : "no";

        {
        ECInstanceInserter ecInserter(ecdb, *testClass, writeToken);
        ASSERT_EQ(expectedIsValid, ecInserter.IsValid()) << "ECDb: " << ecdb.GetDbFileName() << " WriteToken: " << writeTokenStr;
        }

        {
        ECInstanceUpdater ecUpdater(ecdb, *testClass, writeToken);
        ASSERT_EQ(expectedIsValid, ecUpdater.IsValid()) << "ECDb: " << ecdb.GetDbFileName() << " WriteToken: " << writeTokenStr;
        }

        {
        ECInstanceDeleter ecDeleter(ecdb, *testClass, writeToken);
        ASSERT_EQ(expectedIsValid, ecDeleter.IsValid()) << "ECDb: " << ecdb.GetDbFileName() << " WriteToken: " << writeTokenStr;
        }

        {
        JsonInserter jsonInserter(ecdb, *testClass, writeToken);
        ASSERT_EQ(expectedIsValid, jsonInserter.IsValid()) << "ECDb: " << ecdb.GetDbFileName() << " WriteToken: " << writeTokenStr;
        }

        {
        JsonUpdater jsonUpdater(ecdb, *testClass, writeToken);
        ASSERT_EQ(expectedIsValid, jsonUpdater.IsValid()) << "ECDb: " << ecdb.GetDbFileName() << " WriteToken: " << writeTokenStr;
        }

        };

    runAdapters(readonlyECDb, nullptr, false);
    runAdapters(permittedECDb, nullptr, true);
    runAdapters(restrictedECDb, nullptr, false);
    runAdapters(restrictedECDb, restrictedECDb.GetToken(), true);

    //BlobIO test
    enum class BlobIoOpenMode
        {
        Readonly,
        ReadWrite
        };
    auto openBlobIO = [] (ECDbCR ecdb, ECInstanceKey const& key, BlobIoOpenMode openMode, ECCrudWriteToken const* writeToken, BentleyStatus expectedStat)
        {
        ECClassCP testClass = ecdb.Schemas().GetClass("ECSqlTest", "PSA");
        ASSERT_TRUE(testClass != nullptr);
        Utf8CP writeTokenStr = writeToken != nullptr ? "yes" : "no";
        Utf8CP openModeStr = openMode == BlobIoOpenMode::Readonly ? "readonly" : "readwrite";

        BlobIO io;
        ASSERT_EQ(expectedStat, ecdb.OpenBlobIO(io, *testClass, "Bi", key.GetInstanceId(), openMode == BlobIoOpenMode::ReadWrite, writeToken)) << "ECDb: " << ecdb.GetDbFileName() << " openMode: " << openModeStr << " WriteToken: " << writeTokenStr;
        ASSERT_EQ(expectedStat, io.IsValid() ? SUCCESS : ERROR) << "ECDb: " << ecdb.GetDbFileName() << " openMode: " << openModeStr << " WriteToken: " << writeTokenStr;
        };

    openBlobIO(readonlyECDb, blobIoInstanceKey, BlobIoOpenMode::ReadWrite, nullptr, ERROR);
    openBlobIO(readonlyECDb, blobIoInstanceKey, BlobIoOpenMode::Readonly, nullptr, SUCCESS);
    openBlobIO(permittedECDb, blobIoInstanceKey, BlobIoOpenMode::ReadWrite, nullptr, SUCCESS);
    openBlobIO(permittedECDb, blobIoInstanceKey, BlobIoOpenMode::Readonly, nullptr, SUCCESS);
    openBlobIO(restrictedECDb, blobIoInstanceKey, BlobIoOpenMode::ReadWrite, nullptr, ERROR);
    openBlobIO(restrictedECDb, blobIoInstanceKey, BlobIoOpenMode::Readonly, nullptr, SUCCESS);
    openBlobIO(restrictedECDb, blobIoInstanceKey, BlobIoOpenMode::ReadWrite, restrictedECDb.GetToken(), SUCCESS);
    openBlobIO(restrictedECDb, blobIoInstanceKey, BlobIoOpenMode::Readonly, restrictedECDb.GetToken(), SUCCESS);
    restrictedECDb.AbandonChanges();
    permittedECDb.AbandonChanges();
    }

END_ECDBUNITTESTS_NAMESPACE
