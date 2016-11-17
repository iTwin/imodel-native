/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlWriteTokenTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
struct ECSqlWriteTokenTestFixture : public ECDbTestFixture
    {
protected:
    struct RestrictedECDb : ECDb
        {
        private:
            ECSqlWriteToken const* m_token;

        public:
            explicit RestrictedECDb() : ECDb()
                {
                m_token = &EnableECSqlWriteTokenValidation();
                }

            ECSqlWriteToken const* GetToken() const { return m_token; }
        };
    };


//---------------------------------------------------------------------------------------
// @bsimethod                                      Krischan.Eberle                  11/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlWriteTokenTestFixture, Test)
{
    BeFileName seedFilePath;
    {
    SetupECDb("ecsqlwritetoken_seed.ecdb", BeFileName(L"ECSqlTest.01.00.ecschema.xml"));
    ASSERT_TRUE(GetECDb().IsDbOpen());
    seedFilePath.AssignUtf8(GetECDb().GetDbFileName());
    GetECDb().CloseDb();
    }

    ECDb readonlyECDb, permittedECDb;
    ASSERT_EQ(BE_SQLITE_OK, CloneECDb(readonlyECDb, "ecsqlwritetoken_readonly.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(BE_SQLITE_OK, CloneECDb(permittedECDb, "ecsqlwritetoken_permitted.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));
    RestrictedECDb restrictedECDb;
    ASSERT_EQ(BE_SQLITE_OK, CloneECDb(restrictedECDb, "ecsqlwritetoken_restricted.ecdb", seedFilePath, ECDb::OpenParams(Db::OpenMode::ReadWrite)));

    bvector<Utf8CP> selectECSqls;
    selectECSqls.push_back("SELECT * FROM ecsql.PSA");
    selectECSqls.push_back("SELECT * FROM ecsql.PSAHasP");

    bvector<Utf8CP> nonSelectECSqls;
    nonSelectECSqls.push_back("INSERT INTO ecsql.PSA(L,S) VALUES(1000,'fasda')");
    nonSelectECSqls.push_back("UPDATE ecsql.PSA SET L=1123");
    nonSelectECSqls.push_back("DELETE FROM ecsql.PSA");

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

    auto runNonSelectECSql = [] (ECDbCR ecdb, Utf8CP ecsql, ECSqlWriteToken const* writeToken, bool expectedToSucceed)
        {
        ECSqlStatement stmt;
        ECSqlStatus stat = stmt.Prepare(ecdb, ecsql, writeToken);
        if (expectedToSucceed)
            {
            ASSERT_EQ(ECSqlStatus::Success, stat) << "ECDb: " << ecdb.GetDbFileName() << " ECSQL: " << ecsql;
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "ECDb: " << ecdb.GetDbFileName() << " ECSQL: " << ecsql;
            }
        else
            ASSERT_EQ(ECSqlStatus::Error, stat) << "ECDb: " << ecdb.GetDbFileName() << " ECSQL: " << ecsql;
        };

    for (Utf8CP nonSelectECSql : nonSelectECSqls)
        {
        runNonSelectECSql(readonlyECDb, nonSelectECSql, nullptr, false);
        runNonSelectECSql(permittedECDb, nonSelectECSql, nullptr, true);
        runNonSelectECSql(restrictedECDb, nonSelectECSql, nullptr, false);
        runNonSelectECSql(restrictedECDb, nonSelectECSql, restrictedECDb.GetToken(), true);
        }

    auto runNonSelectECSqlFromCache = [] (ECDbCR ecdb, ECSqlStatementCache& cache, Utf8CP ecsql, ECSqlWriteToken const* writeToken, bool expectedToSucceed)
        {
        Utf8CP writeTokenStr = writeToken != nullptr ? "yes" : "no";
        CachedECSqlStatementPtr stmt = cache.GetPreparedStatement(ecdb, ecsql, writeToken);
        if (expectedToSucceed)
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
        runNonSelectECSqlFromCache(readonlyECDb, readonlyCache, nonSelectECSql, nullptr, false);
        runNonSelectECSqlFromCache(permittedECDb, permittedCache, nonSelectECSql, nullptr, true);
        runNonSelectECSqlFromCache(restrictedECDb, restrictedCache, nonSelectECSql, nullptr, false);
        runNonSelectECSqlFromCache(restrictedECDb, restrictedCacheWithToken, nonSelectECSql, restrictedECDb.GetToken(), true);
        }


    auto runAdapters = [] (ECDbCR ecdb, ECSqlWriteToken const* writeToken, bool expectedToSucceed)
        {
        ECClassCP testClass = ecdb.Schemas().GetECClass("ECSqlTest", "PSA");
        ASSERT_TRUE(testClass != nullptr);
        Utf8CP writeTokenStr = writeToken != nullptr ? "yes" : "no";

        {
        ECInstanceInserter ecInserter(ecdb, *testClass, writeToken);
        ASSERT_EQ(expectedToSucceed, ecInserter.IsValid()) << "ECDb: " << ecdb.GetDbFileName() << " WriteToken: " << writeTokenStr;
        }

        {
        ECInstanceUpdater ecUpdater(ecdb, *testClass, writeToken);
        ASSERT_EQ(expectedToSucceed, ecUpdater.IsValid()) << "ECDb: " << ecdb.GetDbFileName() << " WriteToken: " << writeTokenStr;
        }

        {
        ECInstanceDeleter ecDeleter(ecdb, *testClass, writeToken);
        ASSERT_EQ(expectedToSucceed, ecDeleter.IsValid()) << "ECDb: " << ecdb.GetDbFileName() << " WriteToken: " << writeTokenStr;
        }

        {
        JsonInserter jsonInserter(ecdb, *testClass, writeToken);
        ASSERT_EQ(expectedToSucceed, jsonInserter.IsValid()) << "ECDb: " << ecdb.GetDbFileName() << " WriteToken: " << writeTokenStr;
        }

        {
        JsonUpdater jsonUpdater(ecdb, *testClass, writeToken);
        ASSERT_EQ(expectedToSucceed, jsonUpdater.IsValid()) << "ECDb: " << ecdb.GetDbFileName() << " WriteToken: " << writeTokenStr;
        }

        {
        JsonDeleter jsonDeleter(ecdb, *testClass, writeToken);
        ASSERT_EQ(expectedToSucceed, jsonDeleter.IsValid()) << "ECDb: " << ecdb.GetDbFileName() << " WriteToken: " << writeTokenStr;
        }

        };

    runAdapters(readonlyECDb, nullptr, false);
    runAdapters(permittedECDb, nullptr, true);
    runAdapters(restrictedECDb, nullptr, false);
    runAdapters(restrictedECDb, restrictedECDb.GetToken(), true);
    }

END_ECDBUNITTESTS_NAMESPACE
