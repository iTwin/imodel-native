/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlStatementCache_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Muhammad Hassan                         05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (ECSqlStatementCacheTests, BindValuesToSameCachedStatementsMultipleTime)
    {
    ECDbTestProject testproject;
    auto& ecdb = testproject.Create ("ECSqlStatementCacheTest.ecdb", L"TestSchema.01.00.ecschema.xml", false);

    Utf8CP ecSqlInsert = "INSERT INTO TS.Settings (FormateName, FormateVersion) VALUES (?, ?)";
    Utf8CP ecSqlSelect = "SELECT FormateName, FormateVersion FROM TS.Settings";

    ECSqlStatementCache cache (3);
    ASSERT_TRUE (cache.IsEmpty ());

        {
        CachedECSqlStatementPtr stmt = cache.GetPreparedStatement (ecdb, ecSqlInsert);
        ASSERT_TRUE (stmt != nullptr);
        ASSERT_TRUE (stmt->IsPrepared ());
        ASSERT_EQ (stmt->BindText (1, "Hello", IECSqlBinder::MakeCopy::No), ECSqlStatus::Success) << "Binding string value failed";
        ASSERT_EQ (stmt->BindInt (2, 1), ECSqlStatus::Success) << "Binding Integer Value failed";
        ASSERT_TRUE (stmt->Step () == BE_SQLITE_DONE);
        }

    ASSERT_EQ (cache.Size (), 1);

        {
        CachedECSqlStatementPtr stmt = cache.GetPreparedStatement (ecdb, ecSqlSelect);
        ASSERT_EQ (stmt->Step (), BE_SQLITE_ROW);
        Utf8String formateName = stmt->GetValueText (0);
        ASSERT_EQ (formateName, "Hello");
        ASSERT_EQ (stmt->GetValueInt (1), 1);
        }

    ASSERT_EQ (cache.Size (), 2) << "Cache is expected to have two ECSqlStatements";

        {
        CachedECSqlStatementPtr stmt = cache.GetPreparedStatement (ecdb, ecSqlInsert);
        ASSERT_EQ (stmt->BindText (1, "World", IECSqlBinder::MakeCopy::No), ECSqlStatus::Success) << "Binding string value failed";
        ASSERT_EQ (stmt->BindInt (2, 2), ECSqlStatus::Success) << "Binding Integer Value failed";
        ASSERT_TRUE (stmt->Step () == BE_SQLITE_DONE);
        }
    //get existing cached statement so size of cache should remain the same i.e 2
    ASSERT_EQ (cache.Size (), 2) << "Cache is expected to have two ECSqlStatements";

    //Again accessing ECSqlSelect from cache and the size of cache should not change
        {
        size_t rowCount = 0;
        CachedECSqlStatementPtr stmt = cache.GetPreparedStatement (ecdb, ecSqlSelect);
        while (stmt->Step () != BE_SQLITE_DONE)
            {
            rowCount++;
            }
        ASSERT_EQ (rowCount, 2) << "row count returned doesn't match the no of rows inserted";
        }

    ASSERT_EQ (cache.Size (), 2) << "Cache is expected to have two ECSqlStatements";
    cache.Empty ();
    ASSERT_EQ (cache.Size (), 0) << "cache size should be 0 after it is cleared";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Muhammad Hassan                         05/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (ECSqlStatementCacheTests, VerifyCacheSizeMustNotExceedLimit)
    {
    ECDbTestProject testproject;
    auto& ecdb = testproject.Create ("ECSqlStatementCacheTest.ecdb", L"TestSchema.01.00.ecschema.xml", false);

    Utf8CP ecSql1 = "INSERT INTO TS.Settings VALUES (?, ?)";
    Utf8CP ecSql2 = "SELECT * FROM TS.Settings";
    Utf8CP ecSql3 = "SELECT * FROM TS.FileInfo";

    ECSqlStatementCache cache (2);
    CachedECSqlStatementPtr stmt = cache.GetPreparedStatement (ecdb, ecSql1);//insert first ECSql to cache
    ASSERT_TRUE (stmt != nullptr);
    ASSERT_EQ (cache.Size (), 1);

    stmt = cache.GetPreparedStatement (ecdb, ecSql2);//insert 2nd ECSql to cache
    ASSERT_TRUE (stmt != nullptr);
    ASSERT_EQ (cache.Size (), 2);

    stmt = cache.GetPreparedStatement (ecdb, ecSql3);//insert 3rd ECSql to cache
    ASSERT_TRUE (stmt != nullptr);
    ASSERT_EQ (cache.Size (), 2) << "cache size exceeded the limit";

    cache.Empty ();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  02/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlStatementCacheTests, GetPreparedStatement)
    {
    ECDbTestProject test;
    auto& ecdb = test.Create ("ecsqlstatementcachetest.ecdb", L"ECSqlTest.01.00.ecschema.xml", false);

    Utf8CP ecsql1 = "SELECT * FROM ecsql.PSA";
    Utf8CP ecsql2 = "SELECT * FROM ecsql.P";

    ECSqlStatementCache cache (10);
    ASSERT_TRUE (cache.IsEmpty ());

    //get cached statement and release it again
    {
    CachedECSqlStatementPtr stmt = cache.GetPreparedStatement (ecdb, ecsql1);
    ASSERT_TRUE (stmt != nullptr);
    ASSERT_EQ (2, stmt->GetRefCount ());
    ASSERT_TRUE (stmt->IsPrepared ());
    }

    ASSERT_EQ (1, cache.Size ());

    //get already existing statement and release it again
    {
    CachedECSqlStatementPtr stmt = cache.GetPreparedStatement (ecdb, ecsql1);
    ASSERT_TRUE (stmt != nullptr);
    ASSERT_EQ (2, stmt->GetRefCount ());
    ASSERT_TRUE (stmt->IsPrepared ());
        }

    ASSERT_EQ (1, cache.Size ()) << "Getting same statement should not add a new statement to the cache";

    //get other statement and release it again
    {
    CachedECSqlStatementPtr stmt = cache.GetPreparedStatement (ecdb, ecsql2);
    ASSERT_TRUE (stmt != nullptr);
    ASSERT_EQ (2, stmt->GetRefCount ());
    ASSERT_TRUE (stmt->IsPrepared ());
    }

    ASSERT_EQ (2, cache.Size ());

    cache.Empty ();
    ASSERT_EQ (0, cache.Size ()) << "Unexpected value after ECSqlCache::Empty";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  02/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlStatementCacheTests, GetAlreadyUsedPreparedStatement)
    {
    ECDbTestProject test;
    auto& ecdb = test.Create ("ecsqlstatementcachetest.ecdb", L"ECSqlTest.01.00.ecschema.xml", false);

    Utf8CP ecsql = "SELECT * FROM ecsql.PSA";

    ECSqlStatementCache cache (10);

    CachedECSqlStatementPtr stmt1 = cache.GetPreparedStatement (ecdb, ecsql);
    ASSERT_TRUE (stmt1 != nullptr);
    ASSERT_EQ (2, stmt1->GetRefCount ());
    ASSERT_TRUE (stmt1->IsPrepared ());

    ASSERT_EQ (1, cache.Size ());

    CachedECSqlStatementPtr stmt2 = cache.GetPreparedStatement (ecdb, ecsql);
    ASSERT_TRUE (stmt2 != nullptr);
    ASSERT_EQ (2, stmt2->GetRefCount ());
    ASSERT_TRUE (stmt2->IsPrepared ());

    ASSERT_FALSE (stmt1.get () == stmt2.get ()) << "Two different statement objects are expected for same ECSQL as first one was in use";
    ASSERT_EQ (2, cache.Size ());

    CachedECSqlStatementPtr stmt3 = cache.GetPreparedStatement (ecdb, ecsql);
    ASSERT_TRUE (stmt3 != nullptr);
    ASSERT_EQ (2, stmt3->GetRefCount ());
    ASSERT_TRUE (stmt3->IsPrepared ());

    ASSERT_FALSE (stmt1.get () == stmt3.get ()) << "Two different statement objects are expected for same ECSQL as first one was in use";
    ASSERT_EQ (3, cache.Size ());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  02/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlStatementCacheTests, PrepareFailure)
    {
    ECDbTestProject test;
    auto& ecdb = test.Create ("ecsqlstatementcachetest.ecdb", L"ECSqlTest.01.00.ecschema.xml", false);

    ECSqlStatementCache cache (10);

    //try get with wrong ECSQL
    CachedECSqlStatementPtr stmt = cache.GetPreparedStatement (ecdb, "SELECT * FROM blabla");
    ASSERT_TRUE (stmt == nullptr);
    ASSERT_TRUE (cache.IsEmpty ());

    //populate cache
    cache.GetPreparedStatement (ecdb, "SELECT * FROM ecsql.PSA");

    //try get with wrong ECSQL again
    ASSERT_EQ (1, cache.Size ());
    stmt = cache.GetPreparedStatement (ecdb, "SELECT * FROM blabla");
    ASSERT_TRUE (stmt == nullptr);
    ASSERT_EQ (1, cache.Size ());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  02/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST (ECSqlStatementCacheTests, CacheExcess)
    {
    ECDbTestProject test;
    auto& ecdb = test.Create ("ecsqlstatementcachetest.ecdb", L"ECSqlTest.01.00.ecschema.xml", false);

    Utf8CP ecsql1 = "SELECT * FROM ecsql.PSA";
    Utf8CP ecsql2 = "SELECT * FROM ecsql.P";
    Utf8CP ecsql3 = "SELECT * FROM ecsql.P WHERE ECInstanceId=1";

    ECSqlStatementCache cache (2);

    //populate cache
    CachedECSqlStatement* stmt1A = cache.GetPreparedStatement (ecdb, ecsql1).get ();
    cache.GetPreparedStatement (ecdb, ecsql2);
    ASSERT_EQ (2, cache.Size ());

    //this should not add a new statement to the cache
    CachedECSqlStatementPtr stmt1B = cache.GetPreparedStatement (ecdb, ecsql1);
    ASSERT_TRUE (stmt1A == stmt1B.get ()) << "Statement is expected to still be in cache";

    //now first statement should be removed from cache
    cache.GetPreparedStatement (ecdb, ecsql3);
    ASSERT_EQ (2, cache.Size ());

    ASSERT_EQ (1, stmt1B->GetRefCount ()) << "Statement was removed from cache, so only holder is expected to be stmt1B";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  02/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECSqlStatementCacheTests, DefaultEventHandlerAndCachedECSqlStatement)
    {
    const int rowsPerInstances = 3;
    ECDbTestProject test;
    auto& ecdb = test.Create("ecsqlstatementcachetest.ecdb", L"ECSqlTest.01.00.ecschema.xml", rowsPerInstances);

    ECSqlStatementCache cache(2);

    for (int i = 0; i < 10; i++)
        {
        CachedECSqlStatementPtr stmt = cache.GetPreparedStatement(ecdb, "UPDATE ONLY ecsql.PSA SET I=123");
        ASSERT_TRUE(stmt != nullptr);
        ASSERT_EQ(BE_SQLITE_DONE, stmt->Step ());
        }
    }

END_ECDBUNITTESTS_NAMESPACE
