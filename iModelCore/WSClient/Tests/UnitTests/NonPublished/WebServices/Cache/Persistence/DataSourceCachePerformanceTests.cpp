/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../Util/MockECDbSchemaChangeListener.h"
#include <Bentley/BeDebugLog.h>

using namespace ::testing;
USING_NAMESPACE_BENTLEY_WEBSERVICES

#include "BaseCacheTest.h"

struct DataSourceCachePerformanceTests : BaseCacheTest
    {
    size_t runs = 0;
    size_t iterations = 0;
    size_t instancesPerIteration = 0;

    IDataSourceCachePtr CreatePerformanceTestCache()
        {
        BeFileName filePath(":memory:");
        filePath = StubFilePath();
        TESTLOG.infov("Cache path: %s", filePath.GetNameUtf8().c_str());
        return CreateTestCache(filePath, StubCacheEnvironemnt());
        }

    void SetUp()
        {
        // Enable to run performance tests
        if (false)
            {
            runs = 5;
            iterations = 10;
            instancesPerIteration = 10000;
            }
        BaseCacheTest::SetUp();
        }

    void RunTest
        (
        std::function<void()> onResetForRun,
        std::function<void()> onNewIteration,
        std::function<void()> onIterationTest,
        std::function<size_t()> getTestedCount
        );
    };

void DataSourceCachePerformanceTests::RunTest
(
std::function<void()> onResetForRun,
std::function<void()> onNewIteration,
std::function<void()> onIterationTest,
std::function<size_t()> getTestedCount
)
    {
#ifdef NDEBUG
    TESTLOG.infov("Running NDEBUG build");
#else
    TESTLOG.infov("Running DEBUG build");
#endif

    bvector<bvector<std::tuple<double, size_t>>> allRuns;

    for (size_t run = 0; run < runs; run++)
        {
        TESTLOG.infov("------ Run %d -----", run);

        bvector<std::tuple<double, size_t>> collection;

        onResetForRun();

        TESTLOG.info("Iteration, TimeSec, InstancesCount");
        for (size_t i = 0; i < iterations; i++)
            {
            onNewIteration();

            int64_t start, end;
            DateTime::GetCurrentTime().ToUnixMilliseconds(start);
            onIterationTest();
            DateTime::GetCurrentTime().ToUnixMilliseconds(end);

            size_t testedCount = getTestedCount();

            double time = (double) (end - start) / 1000;
            TESTLOG.infov("%3d, %5.1f, %8d", i, time, testedCount);

            collection.push_back(std::tuple<double, size_t>{time, testedCount});
            }

        allRuns.push_back(collection);
        }

    TESTLOG.infov("------ Average results from %d runs -----", runs);

    TESTLOG.info("Iteration, AvgTimeSec, InstancesCount");
    for (size_t i = 0; i < iterations; i++)
        {
        double timeSum = 0;
        size_t testedCount = 0;
        for (size_t run = 0; run < runs; run++)
            {
            double time = std::get<0>(allRuns[run][i]);
            timeSum += time;
            testedCount = std::get<1>(allRuns[run][i]);
            }

        double time = timeSum / runs;
        TESTLOG.infov("%3d, %5.1f, %8d", i, time, testedCount);
        }
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(DataSourceCachePerformanceTests, CacheResponse_SameInstancesWithSameResponseKey_ConstantPerformance)
    {
    IDataSourceCachePtr cache;
    CachedResponseKey key;

    bset<ObjectId> rejected;
    WSQuery query("TestSchema", "TestClass");
    size_t totalInstances = 0;
    StubInstances instances;
    for (size_t j = 0; j < instancesPerIteration; j++)
        instances.Add({"TestSchema.TestClass", BeGuid(true).ToString()}, {{"TestProperty", BeGuid(true).ToString()}});
    totalInstances += instancesPerIteration;

    auto onResetForRun = [&]
        {
        cache = CreatePerformanceTestCache();
        key = StubCachedResponseKey(*cache, BeGuid(true).ToString());
        };
    auto onNewIteration = [&]
        {
        };
    auto onIterationTest = [&]
        {
        ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse(), &rejected, &query));
        ASSERT_TRUE(rejected.empty());
        };
    auto getTestedCount = [&]
        {
        return totalInstances;
        };
    RunTest(onResetForRun, onNewIteration, onIterationTest, getTestedCount);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(DataSourceCachePerformanceTests, CacheResponse_IncreasingAmountOfInstancesOnNewCache_ConstantPerformance)
    {
    IDataSourceCachePtr cache;
    CachedResponseKey key;

    bset<ObjectId> rejected;
    WSQuery query("TestSchema", "TestClass");
    size_t totalInstances = 0;
    StubInstances instances;

    auto onResetForRun = [&]
        {
        instances.Clear();
        totalInstances = 0;
        };
    auto onNewIteration = [&]
        {
        rejected.clear();
        for (size_t j = 0; j < instancesPerIteration; j++)
            instances.Add({"TestSchema.TestClass", BeGuid(true).ToString()}, {{"TestProperty", BeGuid(true).ToString()}});
        totalInstances += instancesPerIteration;
        cache = CreatePerformanceTestCache();
        key = StubCachedResponseKey(*cache, BeGuid(true).ToString());
        };
    auto onIterationTest = [&]
        {
        ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse(), &rejected, &query));
        ASSERT_TRUE(rejected.empty());
        };
    auto getTestedCount = [&]
        {
        return totalInstances;
        };
    RunTest(onResetForRun, onNewIteration, onIterationTest, getTestedCount);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(DataSourceCachePerformanceTests, CacheResponse_DifferentInstancesWithDifferentResponseKeysNoQuery_ConstantPerformance)
    {
    IDataSourceCachePtr cache;
    CachedResponseKey key;
    size_t totalInstances = 0;
    StubInstances instances;

    auto onResetForRun = [&]
        {
        totalInstances = 0;
        cache = CreatePerformanceTestCache();
        };
    auto onNewIteration = [&]
        {
        instances = StubInstances();
        for (size_t j = 0; j < instancesPerIteration; j++)
            instances.Add({"TestSchema.TestClass", BeGuid(true).ToString()}, {{"TestProperty", BeGuid(true).ToString()}});
        totalInstances += instancesPerIteration;
        key = StubCachedResponseKey(*cache, BeGuid(true).ToString());
        };
    auto onIterationTest = [&]
        {
        ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse(), nullptr, nullptr));
        };
    auto getTestedCount = [&]
        {
        return totalInstances;
        };
    RunTest(onResetForRun, onNewIteration, onIterationTest, getTestedCount);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(DataSourceCachePerformanceTests, CacheResponse_DifferentInstancesWithDifferentResponseKeys_ConstantPerformance)
    {
    IDataSourceCachePtr cache;
    CachedResponseKey key;
    bset<ObjectId> rejected;
    WSQuery query("TestSchema", "TestClass");
    size_t totalInstances = 0;
    StubInstances instances;

    auto onResetForRun = [&]
        {
        totalInstances = 0;
        cache = CreatePerformanceTestCache();
        };
    auto onNewIteration = [&]
        {
        instances = StubInstances();
        for (size_t j = 0; j < instancesPerIteration; j++)
            instances.Add({"TestSchema.TestClass", BeGuid(true).ToString()}, {{"TestProperty", BeGuid(true).ToString()}});
        totalInstances += instancesPerIteration;
        key = StubCachedResponseKey(*cache, BeGuid(true).ToString());
        };
    auto onIterationTest = [&]
        {
        ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, instances.ToWSObjectsResponse(), &rejected, &query));
        ASSERT_TRUE(rejected.empty());
        };
    auto getTestedCount = [&]
        {
        return totalInstances;
        };
    RunTest(onResetForRun, onNewIteration, onIterationTest, getTestedCount);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Julius.Senkus
//---------------------------------------------------------------------------------------
TEST_F(DataSourceCachePerformanceTests, CacheResponse_RelationshipInstances_ConstantPerformance)
    {
    IDataSourceCachePtr cache;
    CachedResponseKey key;
    bset<ObjectId> rejected;
    WSQuery query("TestSchema", "TestClassA");
    query.AddSelect("*,TestRelationshipClass!poly-forward-TestClass!poly.*");
    size_t totalInstances = 0;
    StubInstances rootInstances;

    auto onResetForRun = [&]
        {
        totalInstances = 0;
        cache = CreatePerformanceTestCache();
        };
    auto onNewIteration = [&]
        {
        rootInstances = StubInstances();
        StubInstances::StubRelationshipInstances relationship = 
            rootInstances.Add(ObjectId({ "TestSchema.TestClassA", BeGuid(true).ToString() }), { { "TestProperty", "RootFolder" } });

        for (size_t i = 0; i < instancesPerIteration; i++)
            {
            relationship.AddRelated({ "TestSchema.TestManyToOneRelationshipClass", "" },
                { "TestSchema.TestClassB", BeGuid(true).ToString() },
                { { "TestProperty", Utf8String("Child"+BeGuid(true).ToString()) } },
                ECRelatedInstanceDirection::Backward,
                {});
            }

        totalInstances += instancesPerIteration;
        key = StubCachedResponseKey(*cache, BeGuid(true).ToString());
        };
    auto onIterationTest = [&]
        {
        ASSERT_EQ(CacheStatus::OK, cache->CacheResponse(key, rootInstances.ToWSObjectsResponse(), &rejected, &query));
        ASSERT_TRUE(rejected.empty());
        };
    auto getTestedCount = [&]
        {
        return totalInstances;
        };
    RunTest(onResetForRun, onNewIteration, onIterationTest, getTestedCount);
    }
