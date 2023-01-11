/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct ECSqlStatementCacheTests : ECDbTestFixture
    {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementCacheTests, BindValuesToSameCachedStatementsMultipleTime)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementCacheTest.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    Utf8CP ecSqlInsert = "INSERT INTO ecsql.PSA(S,I) VALUES (?, ?)";
    Utf8CP ecSqlSelect = "SELECT S,I FROM ecsql.PSA";

    ECSqlStatementCache cache(3);
    ASSERT_TRUE(cache.IsEmpty());

    {
    CachedECSqlStatementPtr stmt = cache.GetPreparedStatement(m_ecdb, ecSqlInsert);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_TRUE(stmt->IsPrepared());
    ASSERT_EQ(stmt->BindText(1, "Hello", IECSqlBinder::MakeCopy::No), ECSqlStatus::Success) << "Binding string value failed";
    ASSERT_EQ(stmt->BindInt(2, 1), ECSqlStatus::Success) << "Binding Integer Value failed";
    ASSERT_TRUE(stmt->Step() == BE_SQLITE_DONE);
    }

    ASSERT_EQ(cache.Size(), 1);
    cache.Log();

    {
    CachedECSqlStatementPtr stmt = cache.GetPreparedStatement(m_ecdb, ecSqlSelect);
    ASSERT_EQ(stmt->Step(), BE_SQLITE_ROW);
    Utf8String formateName = stmt->GetValueText(0);
    ASSERT_EQ(formateName, "Hello");
    ASSERT_EQ(stmt->GetValueInt(1), 1);
    }

    ASSERT_EQ(cache.Size(), 2) << "Cache is expected to have two ECSqlStatements";
    cache.Log();

    {
    CachedECSqlStatementPtr stmt = cache.GetPreparedStatement(m_ecdb, ecSqlInsert);
    ASSERT_EQ(stmt->BindText(1, "World", IECSqlBinder::MakeCopy::No), ECSqlStatus::Success) << "Binding string value failed";
    ASSERT_EQ(stmt->BindInt(2, 2), ECSqlStatus::Success) << "Binding Integer Value failed";
    ASSERT_TRUE(stmt->Step() == BE_SQLITE_DONE);
    }
    //get existing cached statement so size of cache should remain the same i.e 2
    ASSERT_EQ(cache.Size(), 2) << "Cache is expected to have two ECSqlStatements";

    //Again accessing ECSqlSelect from cache and the size of cache should not change
    {
    size_t rowCount = 0;
    CachedECSqlStatementPtr stmt = cache.GetPreparedStatement(m_ecdb, ecSqlSelect);
    while (stmt->Step() != BE_SQLITE_DONE)
        {
        rowCount++;
        }
    ASSERT_EQ(rowCount, 2) << "row count returned doesn't match the no of rows inserted";
    }

    ASSERT_EQ(cache.Size(), 2) << "Cache is expected to have two ECSqlStatements";
    cache.Empty();
    ASSERT_EQ(cache.Size(), 0) << "cache size should be 0 after it is cleared";
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementCacheTests, VerifyCachedStatementIsKeyedCorrectly)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementCacheTest.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));
    BeFileName seedFile(m_ecdb.GetDbFileName(), true);

    Db datasource1, datasource2;
    ASSERT_EQ(BE_SQLITE_OK, datasource1.OpenBeSQLiteDb(seedFile, Db::OpenParams(Db::OpenMode::Readonly)));
    ASSERT_EQ(BE_SQLITE_OK, datasource2.OpenBeSQLiteDb(seedFile, Db::OpenParams(Db::OpenMode::Readonly)));

    ECDbR ecdb1 = m_ecdb;
    ECDb ecdb2;
    ASSERT_EQ(BE_SQLITE_OK, ecdb2.OpenBeSQLiteDb(seedFile, Db::OpenParams(Db::OpenMode::Readonly)));


    Utf8CP ecSql1 = "SELECT * FROM ecsql.PSA";
    Utf8CP ecSql2 = "SELECT * FROM ecsql.P";

    ECSqlStatementCache cache(20);
    CachedECSqlStatementPtr stmt;

    stmt = cache.GetPreparedStatement(ecdb1, ecSql1);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 1);
   
    stmt = cache.GetPreparedStatement(ecdb1, ecSql2);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 2);

    stmt = cache.GetPreparedStatement(ecdb2, ecSql1);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 3);

    stmt = cache.GetPreparedStatement(ecdb2, ecSql2);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 4);

    stmt = cache.GetPreparedStatement(ecdb1.Schemas(), datasource1, ecSql1);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 5);

    stmt = cache.GetPreparedStatement(ecdb1.Schemas(), datasource1, ecSql2);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 6);

    stmt = cache.GetPreparedStatement(ecdb1.Schemas(), datasource2, ecSql1);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 7);

    stmt = cache.GetPreparedStatement(ecdb1.Schemas(), datasource2, ecSql2);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 8);

    stmt = cache.GetPreparedStatement(ecdb2.Schemas(), datasource1, ecSql1);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 9);

    stmt = cache.GetPreparedStatement(ecdb2.Schemas(), datasource1, ecSql2);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 10);

    stmt = cache.GetPreparedStatement(ecdb2.Schemas(), datasource2, ecSql1);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 11);

    stmt = cache.GetPreparedStatement(ecdb2.Schemas(), datasource2, ecSql2);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 12);

    //Re run and make sure cache size does not change
    //--------------------------------------------------------------------

    stmt = cache.GetPreparedStatement(ecdb1, ecSql1);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 12);

    stmt = cache.GetPreparedStatement(ecdb1, ecSql2);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 12);

    stmt = cache.GetPreparedStatement(ecdb2, ecSql1);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 12);

    stmt = cache.GetPreparedStatement(ecdb2, ecSql2);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 12);

    stmt = cache.GetPreparedStatement(ecdb1.Schemas(), datasource1, ecSql1);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 12);

    stmt = cache.GetPreparedStatement(ecdb1.Schemas(), datasource1, ecSql2);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 12);

    stmt = cache.GetPreparedStatement(ecdb1.Schemas(), datasource2, ecSql1);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 12);

    stmt = cache.GetPreparedStatement(ecdb1.Schemas(), datasource2, ecSql2);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 12);

    stmt = cache.GetPreparedStatement(ecdb2.Schemas(), datasource1, ecSql1);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 12);

    stmt = cache.GetPreparedStatement(ecdb2.Schemas(), datasource1, ecSql2);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 12);

    stmt = cache.GetPreparedStatement(ecdb2.Schemas(), datasource2, ecSql1);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 12);

    stmt = cache.GetPreparedStatement(ecdb2.Schemas(), datasource2, ecSql2);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 12);

    cache.Empty();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECSqlStatementCacheTests, VerifyCacheSizeMustNotExceedLimit)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementCacheTest.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    Utf8CP ecSql1 = "INSERT INTO ecsql.PSA(I,D) VALUES (?, ?)";
    Utf8CP ecSql2 = "SELECT * FROM ecsql.PSA";
    Utf8CP ecSql3 = "SELECT * FROM ecsql.P";

    ECSqlStatementCache cache(2);
    CachedECSqlStatementPtr stmt = cache.GetPreparedStatement(m_ecdb, ecSql1);//insert first ECSql to cache
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 1);

    stmt = cache.GetPreparedStatement(m_ecdb, ecSql2);//insert 2nd ECSql to cache
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 2);

    stmt = cache.GetPreparedStatement(m_ecdb, ecSql3);//insert 3rd ECSql to cache
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(cache.Size(), 2) << "cache size exceeded the limit";

    cache.Empty();
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementCacheTests, GetPreparedStatement)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementCacheTest.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    Utf8CP ecsql1 = "SELECT * FROM ecsql.PSA";
    Utf8CP ecsql2 = "SELECT * FROM ecsql.P";

    ECSqlStatementCache cache(10);
    ASSERT_TRUE(cache.IsEmpty());

    //get cached statement and release it again
    {
    CachedECSqlStatementPtr stmt = cache.GetPreparedStatement(m_ecdb, ecsql1);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(2, stmt->GetRefCount());
    ASSERT_TRUE(stmt->IsPrepared());
    }

    ASSERT_EQ(1, cache.Size());

    //get already existing statement and release it again
    {
    CachedECSqlStatementPtr stmt = cache.GetPreparedStatement(m_ecdb, ecsql1);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(2, stmt->GetRefCount());
    ASSERT_TRUE(stmt->IsPrepared());
    }

    ASSERT_EQ(1, cache.Size()) << "Getting same statement should not add a new statement to the cache";

    //get other statement and release it again
    {
    CachedECSqlStatementPtr stmt = cache.GetPreparedStatement(m_ecdb, ecsql2);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(2, stmt->GetRefCount());
    ASSERT_TRUE(stmt->IsPrepared());
    }

    ASSERT_EQ(2, cache.Size());

    cache.Empty();
    ASSERT_EQ(0, cache.Size()) << "Unexpected value after ECSqlCache::Empty";
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementCacheTests, GetAlreadyUsedPreparedStatement)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementCacheTest.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    Utf8CP ecsql = "SELECT * FROM ecsql.PSA";

    ECSqlStatementCache cache(10);

    CachedECSqlStatementPtr stmt1 = cache.GetPreparedStatement(m_ecdb, ecsql);
    ASSERT_TRUE(stmt1 != nullptr);
    ASSERT_EQ(2, stmt1->GetRefCount());
    ASSERT_TRUE(stmt1->IsPrepared());

    ASSERT_EQ(1, cache.Size());

    CachedECSqlStatementPtr stmt2 = cache.GetPreparedStatement(m_ecdb, ecsql);
    ASSERT_TRUE(stmt2 != nullptr);
    ASSERT_EQ(2, stmt2->GetRefCount());
    ASSERT_TRUE(stmt2->IsPrepared());

    ASSERT_FALSE(stmt1.get() == stmt2.get()) << "Two different statement objects are expected for same ECSQL as first one was in use";
    ASSERT_EQ(2, cache.Size());

    CachedECSqlStatementPtr stmt3 = cache.GetPreparedStatement(m_ecdb, ecsql);
    ASSERT_TRUE(stmt3 != nullptr);
    ASSERT_EQ(2, stmt3->GetRefCount());
    ASSERT_TRUE(stmt3->IsPrepared());

    ASSERT_FALSE(stmt1.get() == stmt3.get()) << "Two different statement objects are expected for same ECSQL as first one was in use";
    ASSERT_EQ(3, cache.Size());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementCacheTests, PrepareFailure)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementCacheTest.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    ECSqlStatementCache cache(10);
    CachedECSqlStatementPtr stmt;
    ECSqlStatus status;

    //try get with wrong ECSQL
    stmt = cache.GetPreparedStatement(m_ecdb, "SELECT * FROM blabla", false, &status);
    ASSERT_TRUE(stmt == nullptr);
    ASSERT_EQ(1, cache.Size()) << "ECSqlStatements are added to the cache even if preparation fails";
    ASSERT_EQ(ECSqlStatus::InvalidECSql, status);

    //get with a valid ECSQL
    stmt = cache.GetPreparedStatement(m_ecdb, "SELECT * FROM ecsql.PSA", false, &status);
    ASSERT_TRUE(stmt != nullptr);
    ASSERT_EQ(2, cache.Size());
    ASSERT_EQ(ECSqlStatus::Success, status);

    //try get with wrong ECSQL again -> adds a new statement to the cache
    stmt = cache.GetPreparedStatement(m_ecdb, "SELECT * FROM blabla", false, &status);
    ASSERT_TRUE(stmt == nullptr);
    ASSERT_EQ(3, cache.Size()); 
    ASSERT_EQ(ECSqlStatus::InvalidECSql, status);
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlStatementCacheTests, CacheExcess)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ECSqlStatementCacheTest.ecdb", SchemaItem::CreateForFile("ECSqlTest.01.00.00.ecschema.xml")));

    Utf8CP ecsql1 = "SELECT * FROM ecsql.PSA";
    Utf8CP ecsql2 = "SELECT * FROM ecsql.P";
    Utf8CP ecsql3 = "SELECT * FROM ecsql.P WHERE ECInstanceId=1";

    ECSqlStatementCache cache(2);

    //populate cache
    //hold raw pointers of the statement for testing purposes to not increment ref count
    CachedECSqlStatement* stmt1A = cache.GetPreparedStatement(m_ecdb, ecsql1).get();
    CachedECSqlStatementPtr stmt2 = cache.GetPreparedStatement(m_ecdb, ecsql2);
    ASSERT_EQ(2, cache.Size());

    //this should not add a new statement to the cache
    //but will move it to the top of the cache.
    CachedECSqlStatementPtr stmt1B = cache.GetPreparedStatement(m_ecdb, ecsql1);
    ASSERT_TRUE(stmt1A == stmt1B.get()) << "Statement is expected to still be in cache";

    //now second statement should be removed from cache, as stmt1 was moved to top of cache
    cache.GetPreparedStatement(m_ecdb, ecsql3);
    ASSERT_EQ(2, cache.Size());

    ASSERT_EQ(1, stmt2->GetRefCount()) << "Statement was removed from cache, so only holder is expected to be stmt1B";
    }

END_ECDBUNITTESTS_NAMESPACE
