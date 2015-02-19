/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/ECDb/ECSqlStatementCache_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <UnitTests/NonPublished/ECDb/ECDbTestProject.h>

USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

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
    cache.GetPreparedStatement (ecdb, ecsql2).get ();
    ASSERT_EQ (2, cache.Size ());

    //this should not add a new statement to the cache
    CachedECSqlStatementPtr stmt1B = cache.GetPreparedStatement (ecdb, ecsql1);
    ASSERT_TRUE (stmt1A == stmt1B.get ()) << "Statement is expected to still be in cache";

    //now first statement should be removed from cache
    cache.GetPreparedStatement (ecdb, ecsql3).get ();
    ASSERT_EQ (2, cache.Size ());

    ASSERT_EQ (1, stmt1B->GetRefCount ()) << "Statement was removed from cache, so only holder is expected to be stmt1B";
    }

END_ECDBUNITTESTS_NAMESPACE
