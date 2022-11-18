/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
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

struct ConcurrentQueryFixture : ECDbTestFixture {
    void SetUp() override {
         // ConsoleLogger::SetSeverity("ECDb.ConcurrentQuery", BentleyApi::NativeLogging::LOG_TRACE);
        ECDbTestFixture::SetUp();
    }
};
struct SleepFunc : BeSQLite::ScalarFunction {
    SleepFunc() : ScalarFunction("imodel_sleep", -1){}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override {
        std::chrono::milliseconds t = 10ms;
        if (nArgs > 0) {
            t = std::chrono::milliseconds(args[0].GetValueInt());
        }
        std::this_thread::sleep_for(t);
        ctx.SetResultInt(1);
    }
    static SleepFunc& Instance() {static SleepFunc sleepFunc; return sleepFunc;}
};

struct CountFunc : BeSQLite::ScalarFunction {
    int _rowCount;
    CountFunc() : ScalarFunction("count_row", -1), _rowCount(0){}
    void _ComputeScalar(BeSQLite::DbFunction::Context& ctx, int nArgs, BeSQLite::DbValue* args) override { _rowCount++; }
    void Reset() { _rowCount = 0; }
    static CountFunc& Instance() {static CountFunc countFunc; return countFunc;}
};

struct StressTest {
    using query_request_t = ECSqlRequest::Ptr;
    using futures_t= std::vector<QueryResponse::Future>;
    using responses_t = std::queue<QueryResponse::Ptr>;
    struct QueryParam final {
        float _percentage_of_cacheable_queries;
        float _percentage_of_restartable_queries ;
        int _number_of_restartable_tokens;
        int _min_rows_to_generate ;
        int _max_rows_to_generate;
        int _concurrent_queries;
        std::chrono::milliseconds _sleep_per_request;

        static QueryParam Default() {
            QueryParam _;
            _._percentage_of_cacheable_queries = 0.5f;
            _._percentage_of_restartable_queries = 0.5f;
            _._number_of_restartable_tokens = 1;
            _._min_rows_to_generate = 100;
            _._max_rows_to_generate = 1000;
            _._concurrent_queries = 10;
            _._sleep_per_request = std::chrono::milliseconds(10);
            return _;
        }

        static QueryParam Aggressive() {
            auto _ = Default();
            _._percentage_of_cacheable_queries = 0.5f;
            _._percentage_of_restartable_queries = 0.6f;
            _._sleep_per_request = std::chrono::milliseconds(0);
            return _;
        }
        static QueryParam Slow() {
            QueryParam _ = Aggressive();
            _._sleep_per_request = std::chrono::milliseconds(100);
            return _;
        }
    };
    struct Client {
        static bool IsProb(float per = 0.5) { return ((float)rand() / RAND_MAX) < per; }
        static int Rand(int start = 0, int stop = 0) { return (int)((float)rand() / RAND_MAX)*(stop - start) + stop; }
        private:
            virtual query_request_t _MakeRequest(QueryParam const& param) const {
                const auto isCacheableQuery = IsProb(param._percentage_of_cacheable_queries);
                const auto isRestartableQuery = IsProb(param._percentage_of_restartable_queries);
                const auto restartTokenId = Rand(0, param._number_of_restartable_tokens);
                const auto maxRows = Rand(param._min_rows_to_generate, _param._max_rows_to_generate);
                const auto randomNoneZeroInt = isCacheableQuery ? rand() + 1 : 1;
                const std::string restartToken = SqlPrintfString("restart_token_%d", restartTokenId).GetUtf8CP();
                const std::string query = SqlPrintfString("with cnt(x) as (values(100000) union select x+1 from cnt where x < (? + 100000)) select x,x,x,x,x,x,x,x,x,x from cnt where %d",
                    randomNoneZeroInt).GetUtf8CP();
                auto req = ECSqlRequest::MakeRequest(query, ECSqlParams().BindInt(1, maxRows));
                if (isRestartableQuery) { req->SetRestartToken(restartToken); }
                if (param._sleep_per_request.count() > 0) {
                    std::this_thread::sleep_for(param._sleep_per_request);
                }
                return std::move(req);
            }
        private:
            std::thread _thread;
            std::atomic_bool _running;
            uint64_t _total_time;
            uint64_t _total_bytes;
            uint32_t _requestMade;
            uint32_t _maxRequest;
            QueryParam _param;
            ConcurrentQueryMgr& _mgr;
            query_request_t MakeRequest(QueryParam const& param) const { return _MakeRequest(param); }
            std::map<QueryResponse::Status, int> _statuses;

        public:
            std::map<QueryResponse::Status, int> const& GetStatusCount() const { return _statuses; }
            QueryParam const& GetQueryParam() const { return _param; }
            uint64_t GetTotalTime() const { return _total_time;  }
            uint64_t GetTotalResultSize() const { return _total_bytes;  }
            bool IsRunning() const { return _running.load(); }
            uint32_t GetRequestMade() const { return _requestMade; }
            Client (ConcurrentQueryMgr& mgr, uint32_t maxRequest= 1000, QueryParam param = QueryParam::Default())
                :_running(true),_total_time(0),_total_bytes(0),_mgr(mgr),_requestMade(0),_maxRequest(maxRequest),_param(param)  {
                _thread = std::thread([&]() {
                    while(_running.load()) {
                        futures_t futures;
                        for (int i = 0; i < _param._concurrent_queries; ++i) {
                            futures.push_back(_mgr.Enqueue(MakeRequest(_param)));
                            ++_requestMade;
                            if (_maxRequest > 0) {
                                if (_requestMade > _maxRequest) {
                                    _running = false;
                                }
                            }
                        }

                        for (auto& future : futures) {
                            auto resp = future.Get();
                            _total_time += resp->GetStats().TotalTime().count();
                            _total_bytes += resp->GetStats().MemUsed();
                            if (_statuses.find(resp->GetStatus()) == _statuses.end()) {
                                _statuses[resp->GetStatus()] = 1;
                            } else {
                                ++_statuses[resp->GetStatus()];
                            }
                            if ((int)resp->GetStatus() >= (int)QueryResponse::Status::Error){
                                BeAssert(false);
                                LOG.errorv("%s: %s", QueryResponse::StatusToString(resp->GetStatus()), resp->GetError().c_str());
                            }
                            if (_maxRequest > 0) {
                                if (_requestMade > _maxRequest) {
                                    _running = false;
                                }
                            }
                            std::this_thread::yield();
                        }
                    }
                });
            }
            void AppendStatus(std::map<QueryResponse::Status, int>& statuses) {
                for (auto& entry : _statuses) {
                    if (statuses.find(entry.first) == statuses.end()) {
                        statuses[entry.first] = entry.second;
                    } else {
                        statuses[entry.first] += entry.second;
                    }
                }
            }
            void Stop() { _running.store(false); }
            ~Client() { _thread.join(); }
            static std::unique_ptr<Client> Make(ConcurrentQueryMgr& mgr, uint32_t maxRequest= 1000, QueryParam param = QueryParam::Default()) { return std::make_unique<Client>(mgr, maxRequest, param); }
    };
private:
    std::vector<std::unique_ptr<Client>> m_clients;

public:
    StressTest() {}
    static std::vector<std::unique_ptr<Client>> Make(ConcurrentQueryMgr& mgr, uint32_t maxRequest= 1000, int aggressiveClients = 10, int normalClients = 10, int slowClients = 10) {
        std::vector<std::unique_ptr<Client>> clients;
        for (int i = 0; i < slowClients; ++i) {
            clients.push_back(Client::Make(mgr, maxRequest, QueryParam::Slow()));
        }
        for (int i = 0; i < normalClients; ++i) {
            clients.push_back(Client::Make(mgr, maxRequest, QueryParam::Default()));
        }
        for (int i = 0; i < aggressiveClients; ++i) {
            clients.push_back(Client::Make(mgr, maxRequest, QueryParam::Aggressive()));
        }
        return std::move(clients);
    }
    static void WaitOrStop(std::vector<std::unique_ptr<Client>>& clients,  uint32_t maxRequest= 1000) {
        for (auto& client : clients) {
            if (client->GetRequestMade() >= maxRequest) {
                if (client->IsRunning()) {
                    client->Stop();
                    while(client->IsRunning()) {
                        std::this_thread::yield();
                    }
                }
            }
        }
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
#if 0
TEST_F(ConcurrentQueryFixture, StressTest) {
    GTEST_SKIP() << "This test take 15 sec and is meant for stress testing concurrent query for failures under load";

    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("conn_query.ecdb"));
    m_ecdb.AddFunction(SleepFunc::Instance());
    ConcurrentQueryMgr::Config::GetInstance().SetQuota(QueryQuota(std::chrono::seconds(10),1024*1025*1));
    ConcurrentQueryMgr::Config::GetInstance().SetWorkerThreadCount(std::thread::hardware_concurrency());

    auto& mgr = ConcurrentQueryMgr::GetInstance(m_ecdb);
    const uint32_t maxRequest = 100;
    const int aggressiveClients = 10;
    const int normalClients = 10;
    const int slowClients = 10;

    auto clients = StressTest::Make(mgr, maxRequest, aggressiveClients, normalClients, slowClients);
    StressTest::WaitOrStop(clients, maxRequest);
}
#endif

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, InterruptCheck_Timeout) {

    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("conn_query.ecdb"));
    auto config = ConcurrentQueryMgr::GetConfig(m_ecdb);
    config.SetQuota(QueryQuota(std::chrono::seconds(2), 1024));
    config.SetIgnoreDelay(false);
    ConcurrentQueryMgr::ResetConfig(m_ecdb, config);

    const auto delay = std::chrono::milliseconds(5000);
    auto& mgr = ConcurrentQueryMgr::GetInstance(m_ecdb);
    auto req = ECSqlRequest::MakeRequest("with cnt(x) as (values(0) union select x+1 from cnt where x < ? ) select x from cnt", ECSqlParams().BindInt(1, 1));
    req->SetDelay(delay);
    auto r = mgr.Enqueue(std::move(req)).Get();
    ASSERT_EQ(r->GetStatus(), QueryResponse::Status::Timeout);
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, InterruptCheck_MemoryLimitExceeded) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("conn_query.ecdb"));

    auto config = ConcurrentQueryMgr::GetConfig(m_ecdb);
    config.SetQuota(QueryQuota(std::chrono::seconds(10), 1000));
    ConcurrentQueryMgr::ResetConfig(m_ecdb, config);

    auto& mgr = ConcurrentQueryMgr::GetInstance(m_ecdb);
    auto req = ECSqlRequest::MakeRequest(
        "with cnt(x) as (values(0) union select x+1 from cnt where x < ? ) select x, CAST(randomblob(1000) AS BINARY) from cnt",
        ECSqlParams().BindInt(1, 3));

    auto r = mgr.Enqueue(std::move(req)).Get();
    ASSERT_EQ(r->GetStatus(), QueryResponse::Status::Partial);
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, InterruptCheck_TimeLimitExceeded) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("conn_query.ecdb"));

    auto config = ConcurrentQueryMgr::GetConfig(m_ecdb);
    config.SetQuota(QueryQuota(std::chrono::seconds(1), 1000));
    ConcurrentQueryMgr::ResetConfig(m_ecdb, config);

    m_ecdb.AddFunction(SleepFunc::Instance());
    auto& mgr = ConcurrentQueryMgr::GetInstance(m_ecdb);
    auto req = ECSqlRequest::MakeRequest(
        "with cnt(x) as (values(0) union select x+1 from cnt where x < ? ) select x,imodel_sleep(500, x)  from cnt",
        ECSqlParams().BindInt(1, 10));

    auto r = mgr.Enqueue(std::move(req)).Get();
    ASSERT_EQ(r->GetStatus(), QueryResponse::Status::Partial);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, ECSqlParams) {
    int i = 111;
    int64_t i64 = 1111;
    double d = 1111.1111;
    auto p2d = DPoint2d::From(122.332, 344.455);
    auto p3d = DPoint3d::From(432.453, 231.453, 332.334);
    bvector<Byte> bin = {0x22,0xfa, 0x33, 0x1a, 0x33, 0xe2, 0x39, 0xef, 0xcf, 0xd4};
    auto str ="hello, world";
    auto b = true;
    auto id = BeInt64Id(0xeeddff);
    BeIdSet idset;

    idset.insert(BeInt64Id(0xeeddf1));
    idset.insert(BeInt64Id(0xeeddf2));
    idset.insert(BeInt64Id(0xeeddf3));
    idset.insert(BeInt64Id(0xeeddf4));
    idset.insert(BeInt64Id(0xeeddf5));
    idset.insert(BeInt64Id(0xeeddf6));
    idset.insert(BeInt64Id(0xeeddf7));
    idset.insert(BeInt64Id(0xeeddf8));
    idset.insert(BeInt64Id(0xeeddf9));


    std::string expectedStr =  R"({
        "1"  : {
            "type" : 0,
            "value" : true
        },
        "10" : {
             "type" : 7,
            "value" : {
                "x" : 122.33199999999999,
                "y" : 344.45499999999998
            }
        },
        "11" : {
             "type" : 8,
            "value" : {
                "x" : 432.45299999999997,
                "y" : 231.4530,
                "z" : 332.3340
            }
        },
        "12" : {
             "type" : 9,
            "value" : "hello, world"
        },
        "2" : {
             "type" : 10,
            "value" : "IvozGjPiOe/P1A=="
        },
        "3" : {
             "type" : 1,
            "value" : 1111.1111000000001
        },
        "4" : {
             "type" : 2,
            "value" : "0xeeddff"
        },
        "6" : {
             "type" : 3,
            "value" : "+EEDDF1+1*8"
        },
        "7" : {
             "type" : 4,
            "value" : 111
        },
        "8" : {
             "type" : 5,
            "value" : 1111
        },
        "9" : {
             "type" : 6,
            "value" : null
        },
        "b" : {
             "type" : 0,
            "value" : true
        },
        "bin" : {
             "type" : 10,
            "value" : "IvozGjPiOe/P1A=="
        },
        "d" : {
             "type" : 1,
            "value" : 1111.1111000000001
        },
        "i" : {
             "type" : 4,
            "value" : 111
        },
        "i64" : {
             "type" : 5,
            "value" : 1111
        },
        "id" : {
             "type" : 2,
            "value" : "0xeeddff"
        },
        "idset" : {
             "type" : 3,
            "value" : "+EEDDF1+1*8"
        },
        "nul" : {
             "type" : 6,
            "value" : null
        },
        "p2d" : {
             "type" : 7,
            "value" : {
                "x" : 122.33199999999999,
                "y" : 344.45499999999998
            }
        },
        "p3d" : {
            "type" : 8,
            "value" : {
                "x" : 432.45299999999997,
                "y" : 231.4530,
                "z" : 332.3340
            }
        },
        "str" : {
            "type" : 9,
            "value" : "hello, world"
        }
    })";

    auto expectedJson = Json::Value::From(expectedStr);

    ECSqlParams params;
    params.BindBool(1, b);
    params.BindBlob(2, bin);
    params.BindDouble(3, d);
    params.BindId(4, id);
    params.BindIdSet(6, idset);
    params.BindInt(7, i);
    params.BindLong(8, i64);
    params.BindNull(9);
    params.BindPoint2d(10, p2d);
    params.BindPoint3d(11, p3d);
    params.BindString(12, str);

    params.BindBool("b", b);
    params.BindBlob("bin", bin);
    params.BindDouble("d", d);
    params.BindId("id", id);
    params.BindIdSet("idset", idset);
    params.BindInt("i", i);
    params.BindLong("i64", i64);
    params.BindNull("nul");
    params.BindPoint2d("p2d", p2d);
    params.BindPoint3d("p3d", p3d);
    params.BindString("str", str);

    Json::Value v;
    params.ToJs(v);
    EXPECT_STREQ(v.toStyledString().c_str(), expectedJson.toStyledString().c_str());

    ECSqlParams params1;
    params1.FromJs(v);
    Json::Value v2;
    params1.ToJs(v2);
    EXPECT_STREQ(v2.toStyledString().c_str(), expectedJson.toStyledString().c_str());

    ASSERT_EQ( params1.GetParam(1).GetType(), ECSqlParams::ECSqlParam::Type::Boolean);
    ASSERT_EQ( params1.GetParam(2).GetType(), ECSqlParams::ECSqlParam::Type::Blob);
    ASSERT_EQ( params1.GetParam(3).GetType(), ECSqlParams::ECSqlParam::Type::Double);
    ASSERT_EQ( params1.GetParam(4).GetType(), ECSqlParams::ECSqlParam::Type::Id);
    ASSERT_EQ( params1.GetParam(6).GetType(), ECSqlParams::ECSqlParam::Type::IdSet);
    ASSERT_EQ( params1.GetParam(7).GetType(), ECSqlParams::ECSqlParam::Type::Integer);
    ASSERT_EQ( params1.GetParam(8).GetType(), ECSqlParams::ECSqlParam::Type::Long);
    ASSERT_EQ( params1.GetParam(9).GetType(), ECSqlParams::ECSqlParam::Type::Null);
    ASSERT_EQ( params1.GetParam(10).GetType(), ECSqlParams::ECSqlParam::Type::Point2d);
    ASSERT_EQ( params1.GetParam(11).GetType(), ECSqlParams::ECSqlParam::Type::Point3d);
    ASSERT_EQ( params1.GetParam(12).GetType(), ECSqlParams::ECSqlParam::Type::String);

}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, sqlite_only_eval_function_with_no_arg_or_constant_arg_only_once) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("conn_query.ecdb"));
    m_ecdb.AddFunction(CountFunc::Instance());

    const auto kRowCount = 10;
    if ("function with no arg") {
        CountFunc::Instance().Reset();
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
            "with cnt(x) as (values(1) union select x+1 from cnt where x < ? ) select COUNT_ROW() from cnt"));
        stmt.BindInt(1, kRowCount);
        int idx = 0;
        while(stmt.Step() == BE_SQLITE_ROW) {
            ++idx;
        }
        ASSERT_EQ(idx, kRowCount);
        ASSERT_EQ(CountFunc::Instance()._rowCount, 1); // sql function is only called once
    }
    if ("function with const arg") {
        CountFunc::Instance().Reset();
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
            "with cnt(x) as (values(1) union select x+1 from cnt where x < ? ) select COUNT_ROW(100) from cnt"));
        stmt.BindInt(1, kRowCount);
        int idx = 0;
        while(stmt.Step() == BE_SQLITE_ROW) {
            ++idx;
        }
        ASSERT_EQ(idx, kRowCount);
        ASSERT_EQ(CountFunc::Instance()._rowCount, 1); // sql function is only called once
    }
    if ("function with var arg") {
        CountFunc::Instance().Reset();
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
            "with cnt(x) as (values(1) union select x+1 from cnt where x < ? ) select COUNT_ROW(X) from cnt"));
        stmt.BindInt(1, kRowCount);
        int idx = 0;
        while(stmt.Step() == BE_SQLITE_ROW) {
            ++idx;
        }
        ASSERT_EQ(idx, kRowCount);
        ASSERT_EQ(CountFunc::Instance()._rowCount, kRowCount); // sql function is called for each row
    }
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, DelayRequest) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("conn_query.ecdb"));
    ConcurrentQueryMgr::Config conf = ConcurrentQueryMgr::GetConfig(m_ecdb);
    conf.SetIgnoreDelay(false);
    ConcurrentQueryMgr::ResetConfig(m_ecdb, conf);

    auto& mgr = ConcurrentQueryMgr::GetInstance(m_ecdb);
    auto req = ECSqlRequest::MakeRequest("with cnt(x) as (values(0) union select x+1 from cnt where x < ? ) select x from cnt", ECSqlParams().BindInt(1, 1));
    const auto delay = std::chrono::milliseconds(2000);
    req->SetDelay(delay);
    auto r = mgr.Enqueue(std::move(req)).Get();
    ASSERT_EQ(r->GetStatus(), QueryResponse::Status::Done);
    ASSERT_GT(r->GetStats().TotalTime(), delay);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, RestartToken) {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("conn_query.ecdb"));
    ConcurrentQueryMgr::Config conf = ConcurrentQueryMgr::GetConfig(m_ecdb);
    conf.SetIgnoreDelay(false);
    ConcurrentQueryMgr::ResetConfig(m_ecdb, conf);
        
    auto& mgr = ConcurrentQueryMgr::GetInstance(m_ecdb);
    const auto sql = "with cnt(x) as (values(0) union select x+1 from cnt where x < ? ) select x from cnt";
    auto req0 = ECSqlRequest::MakeRequest(sql, ECSqlParams().BindInt(1, 5));
    req0->SetRestartToken("test token");
    req0->SetDelay(5000ms);
    auto req1 = ECSqlRequest::MakeRequest(sql, ECSqlParams().BindInt(1, 5));
    req1->SetRestartToken("test token");
    auto r0 = mgr.Enqueue(std::move(req0));
    auto r1 = mgr.Enqueue(std::move(req1));

    auto f0 = r0.Get();
    auto f1 = r1.Get();

    ASSERT_EQ(f0->GetStatus(), QueryResponse::Status::Cancel);
    ASSERT_EQ(f1->GetStatus(), QueryResponse::Status::Done);
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, FutureAndCallback) {
    auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Foo" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    </ECCustomAttributes>
                <ECProperty propertyName="I" typeName="int" />
                <ECProperty propertyName="S" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml");

    auto populateDb = [&](int rows, int strMaxLength) {
        ECSqlStatement stmt;
        stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(I,S) VALUES(?,?)");
        for (int i = 0; i < rows;i++) {
            stmt.BindInt(1, rand());
            stmt.BindText(2, Utf8String((rand() % strMaxLength) + 1, 'X').c_str(), IECSqlBinder::MakeCopy::Yes);
            stmt.Step();
            stmt.Reset();
            stmt.ClearBindings();
        };
    };
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("ConcurrentQuery_Simple.ecdb", testSchema));
    const auto rowsInserted = 100;
    populateDb(rowsInserted, 512);

    ReopenECDb(ECDb::OpenParams(Db::OpenMode::Readonly));
    auto& mgr = ConcurrentQueryMgr::GetInstance(m_ecdb);
    auto future = mgr.Enqueue(ECSqlRequest::MakeRequest("select * from ts.foo"));
    std::promise<void> e;
    mgr.Enqueue(ECSqlRequest::MakeRequest("select * from ts.foo"), [&](QueryResponse::Ptr r){
        EXPECT_TRUE(r->IsSuccess());
        e.set_value();
    });
    auto response = future.Get();
    ASSERT_TRUE(response->IsSuccess());
    e.get_future().get();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, ReaderSchema) {
    auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Foo" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    </ECCustomAttributes>
                <ECProperty propertyName="I" typeName="int" />
                <ECProperty propertyName="S" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml");

    auto populateDb = [&](int rows, int strMaxLength) {
        ECSqlStatement stmt;
        stmt.Prepare(m_ecdb, "INSERT INTO ts.Foo(I,S) VALUES(?,?)");
        for (int i = 0; i < rows;i++) {
            stmt.BindInt(1, rand());
            stmt.BindText(2, Utf8String((rand() % strMaxLength) + 1, 'X').c_str(), IECSqlBinder::MakeCopy::Yes);
            stmt.Step();
            stmt.Reset();
            stmt.ClearBindings();
        };
    };
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("ConcurrentQuery_Simple.ecdb", testSchema));
    const auto rowsInserted = 100;
    populateDb(rowsInserted, 512);
    ReopenECDb(ECDb::OpenParams(Db::OpenMode::Readonly));
    auto& mgr = ConcurrentQueryMgr::GetInstance(m_ecdb);
    ECSqlReader reader(mgr, "select * from ts.foo");
    int rowCount = 0;

    BeJsDocument expectedMeta;
    expectedMeta.Parse(R"(
        [
        {
            "className": "",
            "generated": false,
            "index": 0,
            "jsonName": "id",
            "name": "ECInstanceId",
            "extendedType": "Id",
            "typeName": "long"
        },
        {
            "className": "",
            "generated": false,
            "index": 1,
            "jsonName": "className",
            "name": "ECClassId",
            "extendedType": "ClassId",
            "typeName": "long"
        },
        {
            "className": "TestSchema:Foo",
            "generated": false,
            "index": 2,
            "jsonName": "i",
            "name": "I",
            "extendedType": "",
            "typeName": "int"
        },
        {
            "className": "TestSchema:Foo",
            "generated": false,
            "index": 3,
            "jsonName": "s",
            "name": "S",
            "extendedType": "",
            "typeName": "string"
        }
        ]
    )");
    reader.Next();
    ++rowCount;
    ASSERT_STREQ(reader.GetColumns().Stringify().c_str(), expectedMeta.Stringify().c_str());
    while(reader.Next()) {
        ++rowCount;
    }
    ASSERT_EQ(rowCount, rowsInserted);
    ECSqlReader reader1(mgr, "select COUNT(*) from ts.foo");
    reader1.Next();
    BeJsDocument expectedMeta2;
    expectedMeta2.Parse(R"json(
        [
        {
            "className" : "",
            "generated" : true,
            "index" : 0,
            "jsonName" : "cOUNT(*)",
            "name" : "COUNT(*)",
            "extendedType" : "",
            "typeName" : "long"
        }
        ]
    )json");
    ASSERT_STREQ(reader1.GetColumns().Stringify().c_str(), expectedMeta2.Stringify().c_str());
    auto cntJson0 = Json::Value::From(R"j({"cOUNT(*)" : 100.0})j");
    auto cntJson1 = Json::Value::From(R"j({"COUNT(*)" : 100.0})j");
    ASSERT_STREQ(reader1.GetRow().ToJson(ECSqlReader::Row::Format::UseJsonName).toStyledString().c_str(), cntJson0.toStyledString().c_str());
    ASSERT_STREQ(reader1.GetRow().ToJson(ECSqlReader::Row::Format::UseName).toStyledString().c_str(), cntJson1.toStyledString().c_str());
    ASSERT_EQ(reader1.GetRow()[0].asInt(), 100);
    ASSERT_EQ(reader1.GetRow()["cOUNT(*)"].asInt(), 100);

}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, ReaderBinding) {
    auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Foo" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    </ECCustomAttributes>
                <ECProperty propertyName="I" typeName="int" />
                <ECProperty propertyName="S" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml");

    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("ConcurrentQuery_Simple.ecdb", testSchema));

    auto& mgr = ConcurrentQueryMgr::GetInstance(m_ecdb);
    ECSqlReader  classReader(mgr, "select * from meta.ECClassDef where name=?",
        ECSqlParams().BindString(1, "Foo"));
    int cols = 0;
    while(classReader.Next()) {
        auto classRow = classReader.GetRow();
        for (int i =0; i < classReader.GetColumns().size(); ++i, ++cols) {
            auto& col = classReader.GetColumns()[i];

            auto& v0 = classRow[i];
            auto& v1 = classRow[col];
            auto& v2 = classRow[col.GetJsonName()];
            ASSERT_STREQ(v0.ToString().c_str(), v1.ToString().c_str());
            ASSERT_STREQ(v1.ToString().c_str(), v2.ToString().c_str());
            ASSERT_STREQ(v2.ToString().c_str(), v0.ToString().c_str());
        }
    }

    ASSERT_EQ(cols,11);
    // cte
    ECSqlReader  cteReader(mgr, "with cnt(x) as (values(1) union select x+1 from cnt where x < 1000 ) select * from cnt");
    int cntRowCount = 0;
    while(cteReader.Next()) {
       cntRowCount++;
    }
    ASSERT_EQ(cntRowCount,1000);

    BeIdSet idSet;
    idSet.insert(BeInt64Id(10));
    idSet.insert(BeInt64Id(20));
    idSet.insert(BeInt64Id(30));
    idSet.insert(BeInt64Id(40));
    int vsRowCount = 0;
    ECSqlReader  vsReader(mgr, "with cnt(x) as (values(1) union select x+1 from cnt where x < 1000 ) select * from cnt where invirtualset(?, x)",
        ECSqlParams().BindIdSet(1, idSet));

    while(vsReader.Next()) {
       vsRowCount++;
    }
    ASSERT_EQ(vsRowCount,4);
}
//---------------------------------------------------------------------------------------
//@bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConcurrentQueryFixture, BlobIO) {
    auto testSchema = SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Foo" >
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    </ECCustomAttributes>
                <ECProperty propertyName="B" typeName="binary" />
            </ECEntityClass>
        </ECSchema>)xml");

    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("ConcurrentQuery_Simple.ecdb", testSchema));

    auto createBuff = [](int size) {
        std::vector<uint8_t> buffer;
        for(auto i =0; i< size; ++i) {
            buffer.push_back((uint8_t)(((float)rand()/RAND_MAX)*94+32));
        }
        buffer.shrink_to_fit();
        return buffer;
    };
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "insert into ts.Foo(ECInstanceId, B) VALUES(?, ?)"));
    std::map<int , std::vector<uint8_t>> buffers;
    const auto kSize = 1024*4;

    for(auto i =0; i< 2; ++i) {
        buffers[i]=createBuff(kSize);
        stmt.ClearBindings();
        stmt.Reset();
        stmt.BindInt(1, i  + 1);
        stmt.BindBlob(2, &buffers[i][0], (int)(buffers[i].size()), IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(stmt.Step(), BE_SQLITE_DONE);
    }

    m_ecdb.SaveChanges();
    auto& mgr = ConcurrentQueryMgr::GetInstance(m_ecdb);

    if ("read a full blob") {
        auto ecId = 1;
        auto freq = mgr.Enqueue(BlobIORequest::MakeRequest("ts.Foo", "B", ecId));
        auto resp = freq.Get();
        ASSERT_TRUE(resp->IsSuccess());
        auto data = resp->GetAsConst<BlobIOResponse>().GetData();
        auto len = resp->GetAsConst<BlobIOResponse>().GetLength();
        auto& buff = buffers[ecId-1];
        ASSERT_EQ(len, buff.size());
        ASSERT_EQ(memcmp(data, &buff[0], len), 0);
    }
    if ("read a partial blob") {
        auto ecId = 1;
        auto freq = mgr.Enqueue(BlobIORequest::MakeRequest("ts.Foo", "B", ecId, QueryLimit(10, 10)));
        auto resp = freq.Get();
        ASSERT_TRUE(resp->IsSuccess());
        auto data = resp->GetAsConst<BlobIOResponse>().GetData();
        auto len = resp->GetAsConst<BlobIOResponse>().GetLength();
        auto& buff = buffers[ecId-1];
        ASSERT_EQ(len, 10);
        ASSERT_EQ(memcmp(data, &buff[0]+10, len), 0);
    }
    if ("wrong class fail with error") {
        auto freq = mgr.Enqueue(BlobIORequest::MakeRequest("ts.UnknowClass", "B", 1));
        auto resp = freq.Get();
        ASSERT_TRUE(resp->IsError());
        ASSERT_STREQ(resp->GetError().c_str(), "BlobIO: unable to find classname 'ts.UnknowClass'");
    }
    if ("wrong property fail with error") {
        auto freq = mgr.Enqueue(BlobIORequest::MakeRequest("ts.Foo", "UnknownProperty", 1));
        auto resp = freq.Get();
        ASSERT_TRUE(resp->IsError());
        ASSERT_STREQ(resp->GetError().c_str(), "BlobIO: unable to open blob for classname 'ts.Foo' , accessString 'UnknownProperty' for instanceId '0x1'");
    }
    if ("wrong ec instance id fail with error") {
        auto freq = mgr.Enqueue(BlobIORequest::MakeRequest("ts.Foo", "B", 0xffffff));
        auto resp = freq.Get();
        ASSERT_TRUE(resp->IsError());
        printf("%s\n", resp->GetError().c_str());
        ASSERT_STREQ(resp->GetError().c_str(), "BlobIO: unable to open blob for classname 'ts.Foo' , accessString 'B' for instanceId '0xffffff'");
    }

    if ("wrong offset/length fail with error") {
        auto freq = mgr.Enqueue(BlobIORequest::MakeRequest("ts.Foo", "B", 1, QueryLimit(kSize+1024, 1024)));
        auto resp = freq.Get();
        ASSERT_TRUE(resp->IsError());
        ASSERT_STREQ(resp->GetError().c_str(), "BlobIO: offset + length provided is greater then size of blob");
    }
}



END_ECDBUNITTESTS_NAMESPACE
