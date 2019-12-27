/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC
#include <ECDb/ConcurrentQueryManager.h>
#include <future>
#include <chrono>
#include <queue>
#include <thread>
#include <memory>
BEGIN_ECDBUNITTESTS_NAMESPACE
using namespace std::chrono_literals;
struct AsyncQuery
    {
    private:
    std::thread m_thread;
    std::promise<void> m_promise;
    Utf8String m_ecsql;
    ConcurrentQueryManager* m_mgr;
    ConcurrentQueryManager::Limit m_limit;
    ConcurrentQueryManager::Priority m_priority;
    std::chrono::time_point<std::chrono::steady_clock> m_requestTime;
    Json::Value m_rowset;
    int64_t m_rows;
    int m_id;
    static int inst;
    static Utf8CP GetPriortyStr(ConcurrentQueryManager::Priority p)
        {
        if (p == ConcurrentQueryManager::Priority::High)
            return "HIG";
        if (p == ConcurrentQueryManager::Priority::Normal)
            return "NOR";

        return "LOW";
        }
    static void Run(AsyncQuery* q)
        {
        TaskId taskId;
        ConcurrentQueryManager::PostStatus rcPost;
        Utf8String result;
        int64_t rows;
        ConcurrentQueryManager::PollStatus rcPoll;
        q->m_rowset = Json::Value(Json::ValueType::arrayValue);
    next:
        do
            {
            rcPost = q->m_mgr->PostQuery(taskId, q->m_ecsql.c_str(), nullptr, q->m_limit, ConcurrentQueryManager::Quota(), q->m_priority);
            if (rcPost != ConcurrentQueryManager::PostStatus::Done)
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            } while (rcPost != ConcurrentQueryManager::PostStatus::Done);
        std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
        do
            {
            rcPoll = q->m_mgr->PollQuery(result, rows, taskId);
            if (rcPoll == ConcurrentQueryManager::PollStatus::Pending)
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            } while (rcPoll == ConcurrentQueryManager::PollStatus::Pending);

        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);        
        if (rcPoll == ConcurrentQueryManager::PollStatus::Done)
            {
            q->m_rows += rows;
            // printf("[%d-%s] DONE %lld sec: total rows %lld\n", q->m_id, GetPriortyStr(q->m_priority), diff.count(), q->m_rows);
            auto t = Json::Value::From(result);
            for (Json::ArrayIndex f = 0; f < t.size(); f++)
                q->m_rowset.append(t[f]);
            // printf("[%d-%s] DONE %s\n", q->m_id, GetPriortyStr(q->m_priority), q->m_rowset.ToString().substr(0, 80).c_str());
            q->m_promise.set_value();
            return;
            }
        if (rcPoll == ConcurrentQueryManager::PollStatus::Error)
            {
            printf("[%d-%s] ERROR %lld sec\n", q->m_id, GetPriortyStr(q->m_priority), diff.count());
            q->m_promise.set_value();
            FAIL();
            return;
            }
        if (rcPoll == ConcurrentQueryManager::PollStatus::Timeout)
            {
            printf("[%d-%s] TIMEOUT  %lld sec\n", q->m_id, GetPriortyStr(q->m_priority), diff.count());
            FAIL();
            goto next;
            }
        if (rcPoll == ConcurrentQueryManager::PollStatus::Partial)
            {
            // printf("[%d-%s] PARTIAL  %lld sec : rows recieved %lld\n", q->m_id, GetPriortyStr(q->m_priority), diff.count(), q->m_rows);
            q->m_rows += rows;
            auto t = Json::Value::From(result);
            for (Json::ArrayIndex f = 0; f < t.size(); f++)
                q->m_rowset.append(t[f]);

            auto offset = q->m_limit.GetOffset() == -1 ? rows : q->m_limit.GetOffset() + rows;
            q->m_limit = ConcurrentQueryManager::Limit(-1, offset);
            goto next;
            }
        }
    public:
    AsyncQuery(ConcurrentQueryManager& mgr, Utf8CP ecsql, ConcurrentQueryManager::Priority priority)
        :m_ecsql(ecsql), m_mgr(&mgr), m_priority(priority), m_id(inst++),m_rows(0)
        {
        m_thread = std::thread(Run, this);
        }
    AsyncQuery(const AsyncQuery&) = delete;
    AsyncQuery& operator = (const AsyncQuery&) = delete;
    AsyncQuery(AsyncQuery&& rhs) :
        m_thread(std::move(m_thread)), m_promise(std::move(rhs.m_promise)), m_limit(std::move(rhs.m_limit)), m_priority(std::move(rhs.m_priority)), m_rowset(std::move(rhs.m_rowset)), m_mgr(std::move(rhs.m_mgr))
        , m_id(std::move(rhs.m_id)), m_rows(std::move(rhs.m_rows)), m_requestTime(std::move(rhs.m_requestTime))
        {}

    AsyncQuery& operator = (AsyncQuery&& rhs)
        {
        if (this != &rhs)
            {
            m_thread = std::move(m_thread);
            m_promise = std::move(rhs.m_promise);
            m_limit = std::move(rhs.m_limit);
            m_priority = std::move(rhs.m_priority);
            m_rowset = std::move(rhs.m_rowset);
            m_mgr = std::move(rhs.m_mgr);
            m_id = std::move(rhs.m_id);
            m_rows = std::move(rhs.m_rows);
            m_requestTime = std::move(rhs.m_requestTime);
            }

        return *this;
        }


    void Wait()
        {        
        m_promise.get_future().wait();
        m_thread.join();
        }
    };

int AsyncQuery::inst = 1000;

struct TimeoutFunc : BeSQLite::ScalarFunction
    {
    TimeoutFunc() : ScalarFunction("long_run", 1){}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override
        {
        std::this_thread::sleep_for(10ms);
        ctx.SetResultInt(args[0].GetValueInt());
        }
    };
constexpr uint32_t operator "" _mb(unsigned long long v)
    {
    return static_cast<uint32_t>(v * 1024 * 1024);
    }
constexpr uint32_t operator "" _kb(unsigned long long v)
    {
    return  static_cast<uint32_t>(v * 1024);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                     12/19
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, ConcurrentQueryManagerReadonly)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("one.ecdb"));
    ReopenECDb(ECDb::OpenParams(Db::OpenMode::Readonly));
    TimeoutFunc timeoutFunc;
    auto config = ConcurrentQueryManager::Config()
        .SetQuota(ConcurrentQueryManager::Quota(1s, 1_kb))
        .SetIdleCleanupTime(30s)
        .SetAfterConnectionOpenned([&timeoutFunc] (Db const& db) {
            db.AddFunction(timeoutFunc);
            });

    auto& mgr = m_ecdb.GetConcurrentQueryManager();  
    mgr.Initalize(config);
    std::async([&] ()
        {
        std::vector<std::unique_ptr<AsyncQuery>> queries;
        for (int i = 0; i < 100; i++)
            {
            queries.push_back(std::make_unique<AsyncQuery>(mgr, "select * from meta.ecclassdef; ;;", ConcurrentQueryManager::Priority::Low));
            queries.push_back(std::make_unique<AsyncQuery>(mgr, "select * from meta.ecclassdef ;;", ConcurrentQueryManager::Priority::Normal));
            queries.push_back(std::make_unique<AsyncQuery>(mgr, "select * from meta.ecpropertydef; ; ;", ConcurrentQueryManager::Priority::High));
            }
        for (auto& k : queries)
            {
            k->Wait();
            }
        }).wait();
    }
struct RetryHandler final : BeSQLite::BusyRetry
    {
    private:
        virtual int _OnBusy(int count) const override 
            {
            printf("Main %d\n", count);
            return 1; 
            }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                     12/19
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, ConcurrentQueryManagerReadWrite)
    {
    // BeSQLiteLib::EnableSharedCache(true);
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("one.ecdb"));
    m_ecdb.CreateTable("test_foo", "col");
    RefCountedPtr<RetryHandler> handler = new RetryHandler();
    ReopenECDb(ECDb::OpenParams(Db::OpenMode::ReadWrite, DefaultTxn::Yes, handler.get()));
    auto config = ConcurrentQueryManager::Config()
        .SetQuota(ConcurrentQueryManager::Quota(60s, 2_mb))
        .SetIdleCleanupTime(1s)
        .SetConcurrent(8)
        .SetUseSharedCache(true)
        .SetUseImmutableDb(false)
        .SetUseUncommitedRead(true);

    auto& mgr = m_ecdb.GetConcurrentQueryManager();
    mgr.Initalize(config);
    std::async([&] ()
        {
        std::vector<std::unique_ptr<AsyncQuery>> queries;
        for (int i = 0; i < 100; i++)
            {
            // make sure ecsql is not cachable
            queries.push_back(std::make_unique<AsyncQuery>(mgr, SqlPrintfString("select * from meta.ecclassdef where ecinstanceid<> %d", i), ConcurrentQueryManager::Priority::Low));
            queries.push_back(std::make_unique<AsyncQuery>(mgr, SqlPrintfString("select * from meta.ecpropertydef where ecinstanceid<> %d", i), ConcurrentQueryManager::Priority::High));

            // simulate write to primary connection and call save changes to trigger RetryBusy handler 
            m_ecdb.ExecuteSql("insert into test_foo(col) values (1)");

            }
        for (auto& k : queries)
            {
            k->Wait();
            }

        }).wait();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     09/13
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECInstanceId, Conversion)
    {
    //ToString
    ECInstanceId ecInstanceId(UINT64_C(123456789));
    Utf8CP expectedInstanceId = "123456789";
    Utf8Char actualInstanceId[BeInt64Id::ID_STRINGBUFFER_LENGTH];
    ecInstanceId.ToString(actualInstanceId);
    EXPECT_STREQ(expectedInstanceId, actualInstanceId) << "Unexpected InstanceId generated from ECInstanceId " << ecInstanceId.GetValue();

    ecInstanceId = ECInstanceId(UINT64_C(0));
    expectedInstanceId = "0";
    actualInstanceId[0] = '\0';
    ecInstanceId.ToString(actualInstanceId);
    EXPECT_STREQ(expectedInstanceId, actualInstanceId) << "Unexpected InstanceId generated from ECInstanceId " << ecInstanceId.GetValueUnchecked();

    //FromString
    Utf8CP instanceId = "123456789";
    ECInstanceId expectedECInstanceId(UINT64_C(123456789));
    ECInstanceId actualECInstanceId;
    EXPECT_EQ(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId));
    EXPECT_EQ(expectedECInstanceId.GetValue(), actualECInstanceId.GetValue()) << "Unexpected ECInstanceId parsed from InstanceId " << instanceId;

    instanceId = "0";
    expectedECInstanceId = ECInstanceId(UINT64_C(0));
    actualECInstanceId = ECInstanceId();
    EXPECT_NE(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId));

    instanceId = "0000";
    expectedECInstanceId = ECInstanceId(UINT64_C(0));
    actualECInstanceId = ECInstanceId();
    EXPECT_NE(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId));

    instanceId = "-123456";
    EXPECT_NE(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId)) << "InstanceId with negative number '" << instanceId << "' is not expected to be supported by ECInstanceId::FromString";

    instanceId = "-12345678901234";
    EXPECT_NE(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId)) << "InstanceId with negative number '" << instanceId << "' is not expected to be supported by ECInstanceId::FromString";

    //now test with invalid instance ids
    ScopedDisableFailOnAssertion disableFailOnAssertion;

    instanceId = "0x75BCD15";
    expectedECInstanceId = ECInstanceId(UINT64_C(123456789));
    actualECInstanceId = ECInstanceId();
    EXPECT_EQ(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId)) << "InstanceId with hex formatted number '" << instanceId << "' is expected to be supported by ECInstanceId::FromString";
    EXPECT_TRUE(actualECInstanceId == ECInstanceId(UINT64_C(0x75BCD15)));

    instanceId = "i-12345";
    EXPECT_NE(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId)) << "InstanceId starting with i- '" << instanceId << "' is not expected to be supported by ECInstanceId::FromString";

    instanceId = "1234a123";
    EXPECT_NE(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId)) << "Non-numeric InstanceId '" << instanceId << "' is not expected to be supported by ECInstanceId::FromString";

    instanceId = "blabla";
    EXPECT_NE(SUCCESS, ECInstanceId::FromString(actualECInstanceId, instanceId)) << "Non-numeric InstanceId '" << instanceId << "' is not expected to be supported by ECInstanceId::FromString";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  08/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST(SQLiteRegression, Test)
    {
    BeFileName testFilePath(L"D:\\temp\\sqliteselectissue.db");
    if (!testFilePath.DoesPathExist())
        return;

    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);
    BeSQLiteLib::Initialize(temporaryDir);

    Db db;
    ASSERT_EQ(BE_SQLITE_OK, db.OpenBeSQLiteDb(testFilePath, Db::OpenParams(Db::OpenMode::Readonly)));

    Utf8CP problematicSql = R"sql(
    SELECT 1 FROM test_ClassA, (SELECT Id,ECClassId,SourceId,TargetId FROM test_Rel2) rel, test_ClassB
       WHERE test_ClassA.Id=? AND test_ClassA.Id=rel.SourceId AND test_ClassB.Id=rel.TargetId
                )sql";

    printf("Query plan: %s\r\n", db.ExplainQuery(problematicSql, true).c_str());
    printf("Query: %s\r\n", db.ExplainQuery(problematicSql, false).c_str());

    Statement verifyStmt;
    ASSERT_EQ(BE_SQLITE_OK, verifyStmt.Prepare(db, problematicSql));


    ASSERT_EQ(BE_SQLITE_OK, verifyStmt.BindInt64(1, 1));
    ASSERT_EQ(BE_SQLITE_ROW, verifyStmt.Step());
    verifyStmt.Reset();
    verifyStmt.ClearBindings();
    ASSERT_EQ(BE_SQLITE_OK, verifyStmt.BindText(1, "1", Statement::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_ROW, verifyStmt.Step()) << "This currently returns BE_SQLITE_DONE, possible due to a SQLite regression";
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Affan.Khan                      03/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST(ECDbInit, Initialize)
    {
    BeFileName applicationSchemaDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(applicationSchemaDir);
    BeFileName temporaryDir;
    BeTest::GetHost().GetOutputRoot(temporaryDir);
    ECDb::Initialize(temporaryDir, &applicationSchemaDir);
    ASSERT_EQ(true, ECDb::IsInitialized());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, Settings)
    {
    EXPECT_FALSE(ECDb().GetECDbSettings().RequiresECCrudWriteToken());
    EXPECT_FALSE(ECDb().GetECDbSettings().RequiresECSchemaImportToken());

    struct RestrictableECDb final : ECDb
        {
        RestrictableECDb(bool requireCrudToken, bool requireSchemaImportToken) : ECDb()
            {
            ApplyECDbSettings(requireCrudToken, requireSchemaImportToken);
            }

        ~RestrictableECDb() {}

        ECCrudWriteToken const* GetCrudToken() const { return GetECDbSettingsManager().GetCrudWriteToken(); }
        SchemaImportToken const* GetImportToken() const { return GetECDbSettingsManager().GetSchemaImportToken(); }
        };

    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("settings.ecdb"));
    BeFileName testFilePath(m_ecdb.GetDbFileName());
    CloseECDb();

    auto createSchemaV1 = [] () 
        {
        ECSchemaPtr schema = nullptr;
        EXPECT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "Test", "t", 1, 0, 0));
        ECEntityClassP testClass = nullptr;
        EXPECT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(testClass, "Foo1"));
        PrimitiveECPropertyP testProp = nullptr;
        EXPECT_EQ(ECObjectsStatus::Success, testClass->CreatePrimitiveProperty(testProp, "Name"));
        testClass = nullptr;
        EXPECT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(testClass, "Foo2"));
        EXPECT_EQ(ECObjectsStatus::Success, testClass->CreatePrimitiveProperty(testProp, "Name"));
        return schema;
        };

    //creates V2 of the test schema that contains changes (a class is deleted) that are not supported by changeset merging 
    auto createSchemaV2 = [] ()
        {
        ECSchemaPtr schema = nullptr;
        EXPECT_EQ(ECObjectsStatus::Success, ECSchema::CreateSchema(schema, "Test", "t", 2, 0, 0));
        ECEntityClassP testClass = nullptr;
        EXPECT_EQ(ECObjectsStatus::Success, schema->CreateEntityClass(testClass, "Foo2"));
        PrimitiveECPropertyP testProp = nullptr;
        EXPECT_EQ(ECObjectsStatus::Success, testClass->CreatePrimitiveProperty(testProp, "Name"));
        return schema;
        };


    for (bool requiresECCrudWriteToken : {false, true})
        {
        for (bool requiresSchemaImportToken : {false, true})
            {
            RestrictableECDb ecdb(requiresECCrudWriteToken, requiresSchemaImportToken);
            EXPECT_EQ(requiresECCrudWriteToken, ecdb.GetECDbSettings().RequiresECCrudWriteToken());
            EXPECT_EQ(requiresSchemaImportToken, ecdb.GetECDbSettings().RequiresECSchemaImportToken());

            ASSERT_EQ(BE_SQLITE_OK, ecdb.OpenBeSQLiteDb(testFilePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)));
            Utf8CP ecsql = "INSERT INTO ecdbf.ExternalFileInfo(Name) VALUES('foofile.txt')";
            if (requiresECCrudWriteToken)
                {
                ECSqlStatement stmt;
                ASSERT_TRUE(ecdb.GetCrudToken() != nullptr) << "RequiresECCrudWriteToken == true";
                ASSERT_EQ(ECSqlStatus::Error, stmt.Prepare(ecdb, ecsql, nullptr)) << "RequiresECCrudWriteToken == true";
                ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql, ecdb.GetCrudToken())) << "RequiresECCrudWriteToken == true";
                ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "RequiresECCrudWriteToken == true";
                }
            else
                {
                ECSqlStatement stmt;
                ASSERT_TRUE(ecdb.GetCrudToken() == nullptr) << "RequiresECCrudWriteToken == false";
                ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql, nullptr)) << "RequiresECCrudWriteToken == false";
                ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "RequiresECCrudWriteToken == false";
                }

            if (requiresSchemaImportToken)
                {
                ASSERT_TRUE(ecdb.GetImportToken() != nullptr) << "RequiresSchemaImportToken == true";
                ECSchemaPtr schema = createSchemaV1();
                ASSERT_EQ(ERROR, ecdb.Schemas().ImportSchemas({schema.get()}, nullptr)) << "RequiresSchemaImportToken == true";
                ecdb.AbandonChanges();
                schema = createSchemaV1();
                ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportSchemas({schema.get()}, ecdb.GetImportToken())) << "RequiresSchemaImportToken == true";
                ASSERT_TRUE(ecdb.Schemas().GetClass("Test", "Foo1") != nullptr) << "RequiresSchemaImportToken == true";
                ASSERT_TRUE(ecdb.Schemas().GetClass("Test", "Foo2") != nullptr) << "RequiresSchemaImportToken == true";
                }
            else
                {
                ECSchemaPtr schema = createSchemaV1();
                ASSERT_TRUE(ecdb.GetImportToken() == nullptr) << "RequiresSchemaImportToken == false";
                ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportSchemas({schema.get()}, nullptr)) << "RequiresSchemaImportToken == false";
                ASSERT_TRUE(ecdb.Schemas().GetClass("Test", "Foo1") != nullptr) << "RequiresSchemaImportToken == false";
                ASSERT_TRUE(ecdb.Schemas().GetClass("Test", "Foo2") != nullptr) << "RequiresSchemaImportToken == false";
                }

            ecdb.AbandonChanges();
            }
        }
    }

//=======================================================================================
// @bsiclass                                     Krischan.Eberle                  01/15
//=======================================================================================
struct TestOtherConnectionECDb : ECDb
    {
    int m_changeCount;
    TestOtherConnectionECDb()
        : m_changeCount(0)
        {}

    void _OnDbChangedByOtherConnection() override
        {
        m_changeCount++;
        ECDb::_OnDbChangedByOtherConnection();
        }
    };

//---------------------------------------------------------------------------------------
// Test to ensure that PRAGMA data_version changes when another connection changes a database
// @bsiclass                                     Krischan.Eberle                  01/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, TwoConnections)
    {
    BeFileName testECDbPath;
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("one.ecdb"));
    testECDbPath = BeFileName(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();
    }

    TestOtherConnectionECDb ecdb1, ecdb2;
    ASSERT_EQ(BE_SQLITE_OK, ecdb1.OpenBeSQLiteDb(testECDbPath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite, DefaultTxn::No)));
    ASSERT_EQ(BE_SQLITE_OK, ecdb2.OpenBeSQLiteDb(testECDbPath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite, DefaultTxn::No)));

    { // make a change to the database from the first connection
    Savepoint t1(ecdb1, "tx1");
    // the first transaction should not call _OnDbChangedByOtherConnection
    ASSERT_EQ(0, ecdb1.m_changeCount);
    ecdb1.CreateTable("TEST", "Col1 INTEGER");
    }

    { // make a change to the database from the second connection
    Savepoint t2(ecdb2, "tx2");
    // the first transaction on the second connection should not call _OnDbChangedByOtherConnection
    ASSERT_EQ(0, ecdb2.m_changeCount);
    ecdb2.ExecuteSql("INSERT INTO TEST(Col1) VALUES(3)");
    }

    { // start another transaction on the first connection. This should notice that the second connection changed the db.
    Savepoint t3(ecdb1, "tx1");
    ASSERT_EQ(1, ecdb1.m_changeCount);
    ecdb1.ExecuteSql("INSERT INTO TEST(Col1) VALUES(4)");
    }

    { // additional changes from the same connection should not trigger additional calls to _OnDbChangedByOtherConnection
    Savepoint t3(ecdb1, "tx1");
    ASSERT_EQ(1, ecdb1.m_changeCount);
    ecdb1.ExecuteSql("INSERT INTO TEST(Col1) VALUES(5)");
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/15
//+---------------+---------------+---------------+---------------+---------------+------
struct TestBusyRetry : BusyRetry
    {
    int m_retryCount;
    mutable int m_actualRetryCount;

    Savepoint* m_savepointToCommitDuringRetry;
    int m_retryCountBeforeCommit;


    explicit TestBusyRetry(int retryCount)
        : m_retryCount(retryCount), m_actualRetryCount(0), m_savepointToCommitDuringRetry(nullptr), m_retryCountBeforeCommit(-1)
        {}

    int _OnBusy(int count) const override
        {
        //count is 0-based
        if (count >= m_retryCount)
            return 0;

        m_actualRetryCount = count + 1;

        if (m_savepointToCommitDuringRetry != nullptr && m_retryCountBeforeCommit == count)
            {
            if (BE_SQLITE_OK != m_savepointToCommitDuringRetry->Commit())
                {
                BeAssert(false && "Cannot commit write transaction in retry loop.");
                return 1;
                }
            }

        return 1;
        }

    void Reset() { m_actualRetryCount = 0; }
    void SetSavepointToCommitDuringRetry(Savepoint& savepoint, int retryCountBeforeCommit)
        {
        m_savepointToCommitDuringRetry = &savepoint;
        m_retryCountBeforeCommit = retryCountBeforeCommit;
        }
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, TwoConnectionsWithBusyRetryHandler)
    {
    BeFileName testECDbPath;
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("one.ecdb"));
    testECDbPath = BeFileName(m_ecdb.GetDbFileName());
    m_ecdb.CloseDb();
    }

    ECDb::OpenParams openParams(ECDb::OpenMode::ReadWrite, DefaultTxn::No);
    ECDb ecdb1;
    DbResult result = ecdb1.OpenBeSQLiteDb(testECDbPath, openParams);
    ASSERT_EQ(BE_SQLITE_OK, result);
    TestBusyRetry retry(3);
    retry.AddRef();
    ECDb ecdb2;
    openParams = ECDb::OpenParams(ECDb::OpenMode::ReadWrite, DefaultTxn::No);
    openParams.SetBusyRetry(&retry);
    result = ecdb2.OpenBeSQLiteDb(testECDbPath, openParams);
    ASSERT_EQ(BE_SQLITE_OK, result);

    {
    // make a change to the database from the first connection
    Savepoint s1(ecdb1, "ecdb1");
    ecdb1.CreateTable("TEST", "Col1 INTEGER");

    // try to make a change to the database from the second connection which should fail

    Savepoint s2(ecdb2, "ecdb2", false, BeSQLiteTxnMode::Immediate);
    result = s2.Begin();
    ASSERT_EQ(BE_SQLITE_BUSY, result);
    ASSERT_EQ(retry.m_retryCount, retry.m_actualRetryCount);
    }

    //at this point all locks should be cleared again.

    // now try to make a change to the database if the retry handler commits the open transaction on the first connection
    {
    // make a change to the database from the first connection
    Savepoint s1(ecdb1, "ecdb1");
    ecdb1.CreateTable("TEST2", "Col1 INTEGER");

    retry.Reset();
    Savepoint s2(ecdb2, "ecdb2", false, BeSQLiteTxnMode::Immediate);
    int retryAttemptsBeforeCommitting = 2;
    retry.SetSavepointToCommitDuringRetry(s1, 2);
    result = s2.Begin();
    ASSERT_EQ(BE_SQLITE_OK, result) << "Change on first conn gets committed during retry, so Savepoint.Begin should succeed";
    //after commit one more retry will be done (therefore + 1)
    ASSERT_EQ(retryAttemptsBeforeCommitting + 1, retry.m_actualRetryCount);
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                10/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, ResetInstanceIdSequence)
    {
    struct TestECDb final : ECDb
        {
        TestECDb() : ECDb() {}
        ~TestECDb() {}

        BentleyStatus CallResetInstanceIdSequence(BeBriefcaseId newBriefcaseId, IdSet<ECClassId> const* ecClassIgnoreList) { return ResetInstanceIdSequence(newBriefcaseId, ecClassIgnoreList); }

        BeBriefcaseBasedId GetInstanceIdSequenceValue()
            {
            BriefcaseLocalValueCache& cache = GetBLVCache();
            size_t ix = 0;
            if (!cache.TryGetIndex(ix, "ec_instanceidsequence"))
                return BeBriefcaseBasedId();

            uint64_t val = 0;
            if (BE_SQLITE_OK != cache.QueryValue(val, ix))
                return BeBriefcaseBasedId();

            return BeBriefcaseBasedId(val);
            }
        };

    ASSERT_EQ(SUCCESS, SetupECDb("ResetInstanceIdSequence.ecdb", SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
                                                                        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                                                                            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                                                                            <ECEntityClass typeName="A" >
                                                                                <ECCustomAttributes>
                                                                                   <ClassMap xmlns="ECDbMap.02.00">
                                                                                        <MapStrategy>TablePerHierarchy</MapStrategy>
                                                                                    </ClassMap>
                                                                                 </ECCustomAttributes>
                                                                                <ECProperty propertyName="Prop1" typeName="int" />
                                                                            </ECEntityClass>
                                                                            <ECEntityClass typeName="A1" >
                                                                                <BaseClass>A</BaseClass>
                                                                                <ECProperty propertyName="Prop2" typeName="int" />
                                                                            </ECEntityClass>
                                                                            <ECEntityClass typeName="B" >
                                                                                <ECCustomAttributes>
                                                                                   <ClassMap xmlns="ECDbMap.02.00">
                                                                                        <MapStrategy>TablePerHierarchy</MapStrategy>
                                                                                    </ClassMap>
                                                                                 </ECCustomAttributes>
                                                                                <ECProperty propertyName="Prop1" typeName="int" />
                                                                            </ECEntityClass>
                                                                            <ECEntityClass typeName="B1" >
                                                                                <BaseClass>B</BaseClass>
                                                                                <ECProperty propertyName="Prop2" typeName="int" />
                                                                            </ECEntityClass>
                                                                            <ECEntityClass typeName="C" >
                                                                                <ECCustomAttributes>
                                                                                   <ClassMap xmlns="ECDbMap.02.00">
                                                                                        <MapStrategy>TablePerHierarchy</MapStrategy>
                                                                                    </ClassMap>
                                                                                 </ECCustomAttributes>
                                                                                <ECProperty propertyName="Prop1" typeName="int" />
                                                                            </ECEntityClass>
                                                                            <ECEntityClass typeName="C1" >
                                                                                <BaseClass>C</BaseClass>
                                                                                <ECProperty propertyName="Prop2" typeName="int" />
                                                                            </ECEntityClass>
                                                                            <ECEntityClass typeName="C2" >
                                                                                <BaseClass>C</BaseClass>
                                                                                <ECProperty propertyName="Prop3" typeName="int" />
                                                                            </ECEntityClass>
                                                                            <ECEntityClass typeName="C21" >
                                                                                <BaseClass>C2</BaseClass>
                                                                                <ECProperty propertyName="Prop4" typeName="int" />
                                                                            </ECEntityClass>
                                                                        </ECSchema>)xml")));
    
    BeFileName filePath(m_ecdb.GetDbFileName());

    BeBriefcaseId masterBriefcaseId(BeBriefcaseId::Master());
    BeBriefcaseId briefcaseAId(3);
    BeBriefcaseId briefcaseBId(111);

    std::map<uint32_t, uint64_t> sequenceValuesPerBriefcase {{masterBriefcaseId.GetValue(), 0},
                                        {briefcaseAId.GetValue(), 0}, {briefcaseBId.GetValue(), 0}};

    ASSERT_EQ(BE_SQLITE_OK, PopulateECDb(5));
    sequenceValuesPerBriefcase[masterBriefcaseId.GetValue()] = UINT64_C(40);

    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SetAsBriefcase(briefcaseAId));
    ASSERT_EQ(BE_SQLITE_OK, PopulateECDb(5));
    sequenceValuesPerBriefcase[briefcaseAId.GetValue()] = UINT64_C(40);

    m_ecdb.CloseDb();

    TestECDb testDb;
    ASSERT_EQ(BE_SQLITE_OK, testDb.OpenBeSQLiteDb(filePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite)));

    ASSERT_EQ(sequenceValuesPerBriefcase[testDb.GetBriefcaseId().GetValue()], testDb.GetInstanceIdSequenceValue().GetLocalId()) << "Briefcase Id: " << testDb.GetBriefcaseId().GetValue();

    for (std::pair<uint32_t, uint64_t> const& kvPair : sequenceValuesPerBriefcase)
        {
        BeBriefcaseId briefcaseId(kvPair.first);
        uint64_t expectedMaxId = kvPair.second;
        BeBriefcaseBasedId expectedId(briefcaseId, expectedMaxId);
        ASSERT_EQ(SUCCESS, testDb.CallResetInstanceIdSequence(briefcaseId, nullptr)) << briefcaseId.GetValue();
        ASSERT_EQ(expectedId, testDb.GetInstanceIdSequenceValue()) << "Briefcase Id: " << briefcaseId.GetValue() << " Expected sequence value (lower 40 bits): " << expectedMaxId;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, GetAndAssignBriefcaseIdForDb)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbbriefcaseIdtest.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.00.ecschema.xml")));
    
    BeBriefcaseId id = m_ecdb.GetBriefcaseId();
    ASSERT_TRUE(id.IsValid());
    int32_t previousBriefcaseId = id.GetValue();
    BeBriefcaseId nextid = id.GetNextBriefcaseId();
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SetAsBriefcase(nextid));
    int32_t changedBriefcaseId = m_ecdb.GetBriefcaseId().GetValue();
    ASSERT_NE(previousBriefcaseId, changedBriefcaseId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Muhammad Hassan                  11/14
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbTestFixture, GetAndChangeGUIDForDb)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ecdbbriefcaseIdtest.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.00.ecschema.xml")));

    BeGuid guid = m_ecdb.GetDbGuid();
    ASSERT_TRUE(guid.IsValid());

    BeGuid oldGuid = guid;
    guid.Create();
    if (guid.IsValid())
        m_ecdb.ChangeDbGuid(guid);
    BeGuid newGuid = m_ecdb.GetDbGuid();
    ASSERT_TRUE(newGuid.IsValid());
    ASSERT_TRUE(oldGuid != newGuid);

    guid.Invalidate();
    ASSERT_FALSE(guid.IsValid());
    }


END_ECDBUNITTESTS_NAMESPACE
