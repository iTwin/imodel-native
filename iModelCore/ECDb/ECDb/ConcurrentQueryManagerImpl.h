/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ConcurrentQueryManager.h>
#include <queue>
#include <map>
#include <thread>
#include <future>
#include <random>
#include <chrono>

#define DEFAULT_QUERY_DELAY_MAX_TIME    std::chrono::seconds(10)
#define DEFAULT_QUOTA_MAX_TIME          std::chrono::seconds(60)
#define DEFAULT_QUOTA_MAX_MEM           0x800000
#define DEFAULT_REQUEST_QUERY_SIZE      2000
#define DEFAULT_IGNORE_PRIORITY         false
#define DEFAULT_IGNORE_DELAY            true
#define DEFAULT_WORKER_THREAD_COUNT     std::min(4u, std::thread::hardware_concurrency())
#define MAX_REQUEST_QUERY_SIZE          4000
#define MIN_WORKER_THREAD_COUNT         2
#define QUERY_WORKER_RESULT_RESERVE_BYTES 1024*4  // 4Kb and its cached buffer on for each thread.

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
using namespace std::chrono_literals;

using mutex_t = std::mutex;
using guard_t = std::lock_guard<mutex_t>;
using unique_lock_t = std::unique_lock<mutex_t>;
using recursive_mutex_t = std::recursive_mutex;
using recursive_guard_t = std::lock_guard<recursive_mutex_t>;

struct CachedConnection;
struct ConnectionCache;
struct QueryJsonAdaptor;
//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ErrorListenerScope final: ECN::IIssueListener {
    private:
        mutable std::string m_lastError;
        ECDb& m_ecdb;
        BeMutexHolder m_lock;
        void _OnIssueReported(ECN::IssueSeverity severity, ECN::IssueCategory category, ECN::IssueType type, Utf8CP message) const override;
    public:
        explicit ErrorListenerScope (ECDb& ecdb);
        ~ErrorListenerScope();
        std::string const& GetLastError() { return m_lastError; }
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct CachedQueryAdaptor final: std::enable_shared_from_this<CachedQueryAdaptor> {
    private:
        ECSqlStatement m_stmt;
        std::unique_ptr<QueryJsonAdaptor> m_adaptor;
        std::string m_cachedString;
        rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> m_allocator;
        rapidjson::CrtAllocator m_stackAllocator;
        rapidjson::Document m_cachedXmlDoc;
        Db const* m_conn;
        bool m_usePrimaryConn;
    public:
        CachedQueryAdaptor() :m_cachedXmlDoc(&m_allocator, 1024, &m_stackAllocator), m_usePrimaryConn(false) { m_cachedXmlDoc.SetArray(); }
        ECSqlStatement& GetStatement() { return m_stmt; }
        QueryJsonAdaptor& GetJsonAdaptor();
        rapidjson::Document& ClearAndGetCachedXmlDocument() { m_cachedXmlDoc.Clear();  return m_cachedXmlDoc; }
        std::string& ClearAndGetCachedString() { m_cachedString.clear(); return m_cachedString; }
        bool GetUsePrimaryConn() const { return m_usePrimaryConn; }
        void SetUsePrimaryConn(bool val) { m_usePrimaryConn = val; }
        Db const* GetWorkerConn() const { return m_conn; }
        void SetWorkerConn(Db const& conn) { m_conn = &conn; }
        std::shared_ptr<CachedQueryAdaptor> Shared() { return shared_from_this(); }
        static std::shared_ptr<CachedQueryAdaptor> Make() {
            return std::make_shared<CachedQueryAdaptor>();
        }
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct QueryJsonAdaptor {
private:
    bool m_abbreviateBlobs;
    bool m_classIdToClassNames;
    bool m_useJsName;
    ECDbCR m_ecdb;

private:
    BentleyStatus RenderRootProperty(BeJsValue out, IECSqlValue const& in) const;
    BentleyStatus RenderProperty(BeJsValue out, IECSqlValue const& in) const;
    BentleyStatus RenderPrimitiveProperty(BeJsValue out, IECSqlValue const& in, ECN::PrimitiveType const* prop) const;
    BentleyStatus RenderNavigationProperty(BeJsValue out, IECSqlValue const& in) const;
    BentleyStatus RenderPoint2d(BeJsValue out, IECSqlValue const& in) const;
    BentleyStatus RenderPoint3d(BeJsValue out, IECSqlValue const& in) const;
    BentleyStatus RenderLong(BeJsValue out, IECSqlValue const& in, ECN::PrimitiveECPropertyCP prop) const;
    BentleyStatus RenderGeometryProperty(BeJsValue out, IECSqlValue const& in) const;
    BentleyStatus RenderBinaryProperty(BeJsValue out, IECSqlValue const& in) const;
    BentleyStatus RenderStructProperty(BeJsValue out, IECSqlValue const& in) const;
    BentleyStatus RenderPrimitiveArrayProperty(BeJsValue out, IECSqlValue const& in) const;
    BentleyStatus RenderStructArrayProperty(BeJsValue out, IECSqlValue const& in) const;

public:
    QueryJsonAdaptor(ECDbCR ecdb):m_ecdb(ecdb),m_abbreviateBlobs(true), m_classIdToClassNames(false),m_useJsName(false){}
    QueryJsonAdaptor& SetAbbreviateBlobs(bool v) { m_abbreviateBlobs = v; return *this;}
    QueryJsonAdaptor& SetConvertClassIdsToClassNames(bool v) { m_classIdToClassNames = v; return *this; }
    QueryJsonAdaptor& UseJsNames(bool v) { m_useJsName = v; return *this; }
    BentleyStatus RenderRow(BeJsValue rowJson, ECSqlStatement const& stmt, bool asArray = true) const;
    void GetMetaData(QueryProperty::List& list, ECSqlStatement const& stmt) const;
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct QueryAdaptorCache final {
    const static uint32_t kDefaultCacheSize =40;
    private:
            std::vector<std::shared_ptr<CachedQueryAdaptor>> m_cache;
            recursive_mutex_t m_mutex;
            CachedConnection& m_conn;
            uint32_t m_maxEntries;
    public:
        QueryAdaptorCache(CachedConnection& conn, uint32_t maxCacheEntries = kDefaultCacheSize):m_conn(conn), m_maxEntries(maxCacheEntries){}
        ~QueryAdaptorCache(){}
        std::shared_ptr<CachedQueryAdaptor> TryGet(Utf8CP ecsql, bool usePrimaryConn, bool suppressLogError, ECSqlStatus& status, std::string& ecsql_error);
        void Reset() { m_cache.clear(); }
        void SetMaxCacheSize(uint32_t n) { if (n < QueryAdaptorCache::kDefaultCacheSize) return; m_maxEntries = n; }
        CachedConnection& GetConnection() {return m_conn;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================

struct QueryRetryHandler final : BeSQLite::BusyRetry {
    using Ptr = RefCountedPtr<QueryRetryHandler>;
    private:
        std::chrono::milliseconds m_maxTime;
        std::chrono::milliseconds m_sleepTime;
        mutable std::chrono::steady_clock::time_point m_start;
        QueryRetryHandler(std::chrono::milliseconds maxTime, std::chrono::milliseconds sleepTime):
            m_sleepTime(sleepTime),m_maxTime(maxTime) {}
        virtual int _OnBusy(int count) const;
    public:
        static Ptr Create(std::chrono::milliseconds maxTime, std::chrono::milliseconds sleepTime= 100ms) { return new QueryRetryHandler(maxTime, sleepTime); }
};

struct RunnableRequestBase;
//=======================================================================================
//! @bsiclass
//=======================================================================================
struct CachedConnection final : std::enable_shared_from_this<CachedConnection> {
    friend struct QueryAdaptorCache;
    private:
        enum ConnectionAction {
            Opening,
            Closing
        };
        struct FunctionInfo {
            private:
                std::string m_name;
                int m_numArgs;
                DbFunction *m_func;
            public:
                FunctionInfo(std::string name, int numArgs, DbFunction* func): m_name(name), m_numArgs(numArgs), m_func(func) {}
                std::string const& GetName() const {return m_name;}
                int GetNumArgs() const {return m_numArgs;}
                DbFunction* GetFunction() const {return m_func;}
        };

        ConnectionCache& m_cache;
        ECDb m_db;
        recursive_mutex_t m_mutexReq;
        bool m_isChangeSummaryCacheAttached;
        std::unique_ptr<RunnableRequestBase> m_request;
        uint16_t m_id;
        QueryAdaptorCache m_adaptorCache;
        QueryRetryHandler::Ptr m_retryHandler;
        void UpdateSqlFunctions(ConnectionAction);
        CachedConnection(const CachedConnection&)=delete;
        CachedConnection(CachedConnection&&)=delete;
        CachedConnection& operator = (const CachedConnection&)=delete;
        CachedConnection& operator = (CachedConnection&&)=delete;
        std::vector<FunctionInfo> GetPrimaryDbSqlFunctions() const;
        void SetRequest(std::unique_ptr<RunnableRequestBase> request);
        void ClearRequest();
    public:
        CachedConnection(ConnectionCache& cache, uint16_t id):m_cache(cache), m_id(id), m_adaptorCache(*this),m_isChangeSummaryCacheAttached(false),m_retryHandler(QueryRetryHandler::Create(60s)){}
        ~CachedConnection();
        void Interrupt() const { m_db.Interrupt();}
        void Execute(std::function<void(QueryAdaptorCache&,RunnableRequestBase&)>, std::unique_ptr<RunnableRequestBase>);
        void Reset(bool detachDbs);
        void InterruptIf(std::function<bool(RunnableRequestBase const&)>,bool cancel);
        bool IsSync() const { return m_id == 0; }
        ECDb const& GetPrimaryDb() const;
        ECDb const& GetDb() const {return m_db; }
        ECDb& GetDbR() {return m_db; }
        uint16_t Id() const { return m_id; }
        std::shared_ptr<CachedConnection> Shared() { return  shared_from_this(); }
        static std::shared_ptr<CachedConnection> Make(ConnectionCache&,uint16_t);
        void SetAdaptorCacheSize(uint32_t newSize);
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ConnectionCache final {
    private:
        std::vector<std::shared_ptr<CachedConnection>> m_conns;
        std::shared_ptr<CachedConnection> m_syncConn;
        ECDb const& m_primaryDb;
        recursive_mutex_t m_mutex;
        uint32_t m_poolSize;

    public:
        ConnectionCache(ECDb const& primaryDb, uint32_t pool_size);
        ECDb const& GetPrimaryDb() const { return m_primaryDb; }
        std::shared_ptr<CachedConnection> GetConnection();
        CachedConnection& GetSyncConnection();
        void Interrupt(bool reset_conn, bool detachDbs);
        void InterruptIf(std::function<bool(RunnableRequestBase const&)> predicate, bool cancel);
        void SetCacheStatementsPerWork(uint32_t);
        void SetMaxPoolSize(uint32_t newSize)  {m_poolSize = newSize; }
};

struct RunnableRequestQueue;
//=======================================================================================
//! @bsiclass
//=======================================================================================
struct RunnableRequestBase {
    private:
        QueryRequest::Ptr m_request;
        uint32_t m_id;
        bool m_isCompleted;
        std::chrono::time_point<std::chrono::steady_clock> m_dequeuedOn;
        std::chrono::time_point<std::chrono::steady_clock> m_submittedOn;
        bool m_requestCancel;
        RunnableRequestQueue& m_queue;
        QueryQuota m_quota;
        std::atomic_bool m_cancelled;
        uint32_t m_executorId;
        uint32_t m_connId;
        virtual void _SetResponse(QueryResponse::Ptr response) = 0;
    public:
        RunnableRequestBase(RunnableRequestQueue& queue, QueryRequest::Ptr request, QueryQuota quota, uint32_t id)
            :m_queue(queue), m_request(std::move(request)), m_id(id), m_isCompleted(false),m_dequeuedOn(0s),
             m_quota(quota), m_submittedOn(std::chrono::steady_clock::now()), m_cancelled(false), m_executorId(0), m_connId(0){}
        virtual ~RunnableRequestBase(){}
        QueryRequest const& GetRequest() const {return *m_request;}
        uint32_t GetId() const {return m_id; }
        void SetResponse(QueryResponse::Ptr response);
        bool IsCompleted() const {return m_isCompleted; }
        RunnableRequestQueue& GetQueue() { return m_queue;}
        void Cancel() { m_cancelled.store(true); }
        bool IsReady() const { return GetTotalTime() > m_request->GetDelay(); }
        bool IsCancelled () const {return m_cancelled.load(); }
        bool IsTimeExceeded() const { return m_quota.MaxTimeAllowed() == 0s ? false : std::chrono::duration_cast<std::chrono::seconds>(GetTotalTime()) >  m_quota.MaxTimeAllowed();}
        bool IsMemoryExceeded(std::string const& result) const { return m_quota.MaxMemoryAllowed() == 0 ? false : result.size() > m_quota.MaxMemoryAllowed(); }
        bool IsTimeOrMemoryExceeded(std::string const& result) const { return IsTimeExceeded() || IsMemoryExceeded(result);}
        void OnDequeued()  { m_dequeuedOn = std::chrono::steady_clock::now(); }
        uint32_t GetExecutorId() const {return m_executorId; }
        uint32_t GetConnectionId() const {return m_connId; }
        void SetExecutorContext(uint32_t executorId, uint32_t connId) { m_executorId = executorId;  m_connId= connId;}
        std::chrono::milliseconds GetTotalTime() const { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_submittedOn);}
        std::chrono::microseconds GetCpuTime() const { return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_dequeuedOn) ;}
        QueryResponse::Ptr CreateErrorResponse(QueryResponse::Status status, std::string error) const;
        QueryResponse::Ptr CreateTimeoutResponse() const;
        QueryResponse::Ptr CreateCancelResponse() const;
        QueryResponse::Ptr CreateBlobIOResponse(std::vector<uint8_t>& meta, bool done, uint32_t rawBlobSize) const;
        QueryResponse::Ptr CreateECSqlResponse(std::string& result, QueryProperty::List& meta, uint32_t rowcount, bool done) const;
        static QueryResponse::Ptr CreateQueueFullResponse() ;

};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct RunnableRequestWithPromise final : RunnableRequestBase  {
    private:
        std::promise<QueryResponse::Ptr> m_promise;
        virtual void _SetResponse(QueryResponse::Ptr response) override { m_promise.set_value(std::move(response)); }
    public:
        RunnableRequestWithPromise(RunnableRequestQueue& queue, QueryRequest::Ptr request, QueryQuota quota, uint32_t id): RunnableRequestBase(queue, std::move(request), quota, id){}
        virtual ~RunnableRequestWithPromise(){}
        QueryResponse::Future GetFuture();
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct RunnableRequestWithCallback final : RunnableRequestBase {
    private:
        ConcurrentQueryMgr::OnCompletion m_callback;
        virtual void _SetResponse(QueryResponse::Ptr response) override { m_callback(response); }
    public:
        RunnableRequestWithCallback(RunnableRequestQueue& queue, QueryRequest::Ptr request, QueryQuota quota, uint32_t id, ConcurrentQueryMgr::OnCompletion& callback): RunnableRequestBase(queue, std::move(request), quota, id), m_callback(callback){}
        virtual ~RunnableRequestWithCallback(){}
};

struct QueryExecutor;
//=======================================================================================
//! @bsiclass
//=======================================================================================
struct RunnableRequestQueue final {
    const static uint32_t kDefaultQueueSize = 1024;
    const static uint32_t kMinQueueSize = 40;
    friend struct QueryExecutor;
    enum class State {
        Paused,
        Stop,
        Running
    };

    private:
        mutex_t m_mutex;
        std::condition_variable m_cond;
        std::atomic<State> m_state;
        uint32_t m_maxQueueSize;
        uint32_t m_nextId;
        uint32_t m_lastDelayedQueryId;
        QueryQuota m_quota;
        std::vector<std::unique_ptr<RunnableRequestBase>> m_requests;
        ECDbCR m_ecdb;

    private:
        void InsertSorted(ConnectionCache&,std::unique_ptr<RunnableRequestBase>&& request);
        uint32_t GetNextId ();
        std::unique_ptr<RunnableRequestBase> Dequeue();
        std::unique_ptr<RunnableRequestBase> WaitForDequeue();
        QueryQuota AdjustQuota(QueryQuota const& quota) const;
        void ExecuteSynchronously(ConnectionCache&, std::unique_ptr<RunnableRequestBase>);
    public:
        explicit RunnableRequestQueue(ECDbCR ecdb);
        ~RunnableRequestQueue() { Stop();}
        bool CancelRequest(uint32_t id);
        void SetRequestQueueMaxSize(uint32_t size);
        void SetMaxQuota(QueryQuota const&);
        bool Suspend();
        bool Resume();
        ECDbCR GetECDb() const { return m_ecdb; }
        void RemoveIf (std::function<bool(RunnableRequestBase&)> predicate);
        State GetState() const { return m_state.load(); }
        QueryResponse::Future Enqueue(ConnectionCache&,QueryRequest::Ptr);
        void Enqueue(ConnectionCache&,QueryRequest::Ptr, ConcurrentQueryMgr::OnCompletion onComplete);
        uint32_t Count();
        bool Stop();
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct QueryExecutor final {
    private:
        RunnableRequestQueue& m_queue;
        ConnectionCache m_connCache;
        std::vector<std::thread> m_threads;
        uint32_t m_maxPoolSize;
        std::atomic<uint32_t> m_threadCount;
    public:
        QueryExecutor(RunnableRequestQueue& queue, ECDbCR primaryDb, uint32_t pool_size = 0);
        ~QueryExecutor();
        void SetWorkerPoolSize(uint32_t);
        ConnectionCache& GetConnectionCache() {return m_connCache; }
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct QueryHelper final {
    private:
        static std::string FormatQuery(const char* query);
        static void BindLimits(ECSqlStatement& stmt, QueryLimit const& limit);
        static QueryProperty::List GetMetaInfo(CachedQueryAdaptor&,bool);
        static void Execute(CachedQueryAdaptor& cachedAdaptor, RunnableRequestBase& request);
        static void ReadBlob(ECDbCR conn, RunnableRequestBase& request);
        static void ExecutePing(Json::Value const& pingJson, RunnableRequestBase& runnableRequest);
    public:
        static void Execute(QueryAdaptorCache& adaptorCache, RunnableRequestBase& request);
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct QueryResponse::Future::Impl {
    private:
        std::future<QueryResponse::Ptr> m_future;
        std::function<void()> m_cancelCallback;
    public:
        Impl(std::function<void()> cancelCallback, std::future<QueryResponse::Ptr> future): m_cancelCallback(std::move(cancelCallback)), m_future(std::move(future)){}
        void Cancel() { m_cancelCallback();}
        std::future<QueryResponse::Ptr>& GetFuture() {return m_future;}
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct QueryMonitor {
    private:
        std::thread m_thread;
        std::atomic_bool m_stop;
        RunnableRequestQueue& m_queue;
        QueryExecutor& m_executor;
        std::chrono::milliseconds m_pollInterval;
        cancel_callback_type m_cancelBeforeSchemaChanges;
    public:
        QueryMonitor(RunnableRequestQueue& queue, QueryExecutor& executor, std::chrono::milliseconds pollInterval = 1000ms);
        ~QueryMonitor() { m_stop.store(true); if (m_thread.joinable()) m_thread.join(); }
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ConcurrentQueryMgr::Impl : ECDb::IECDbCacheClearListener {
    private:
        RunnableRequestQueue m_queue;
        QueryExecutor m_executor;
        QueryMonitor m_monitor;
        cancel_callback_type m_removeEventHandlers;
        void _OnBeforeClearECDbCache() override;
        void _OnAfterClearECDbCache() override;
    public:
        Impl(ECDbCR ecdb);
        ~Impl();
        QueryResponse::Future Enqueue(QueryRequest::Ptr request) { return m_queue.Enqueue(m_executor.GetConnectionCache(), std::move(request)); }
        void Enqueue(QueryRequest::Ptr request, OnCompletion completion) { m_queue.Enqueue(m_executor.GetConnectionCache(), std::move(request), completion); }
        bool Suspend(ClearCacheOption clearCache, DetachAttachDbs detachDbs);
        bool Resume() {return m_queue.Resume();}
        bool IsSuspended() const { return m_queue.GetState() == RunnableRequestQueue::State::Paused;}
        // change config
        void SetWorkerPoolSize(uint32_t newSize) { m_executor.SetWorkerPoolSize(newSize); }
        void SetRequestQueueMaxSize(uint32_t newSize) { m_queue.SetRequestQueueMaxSize(newSize); }
        void SetCacheStatementsPerWork(uint32_t newSize) { m_executor.GetConnectionCache().SetCacheStatementsPerWork(newSize); }
        void SetMaxQuota(QueryQuota const& newQuota) {m_queue.SetMaxQuota(newQuota); }
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ConcurrentQueryAppData : Db::AppData {
    private:
        mutable ConcurrentQueryMgr m_concurrentQuery;
    public:
        ConcurrentQueryAppData(ECDbCR ecdb):m_concurrentQuery(ecdb){}
        static Db::AppData::Key& GetKey() {
            static Db::AppData::Key s_key;
            return s_key;
        }
        ConcurrentQueryMgr& GetConcurrentQuery() const { return m_concurrentQuery; }
        static RefCountedPtr<ConcurrentQueryAppData> Create(ECDbCR ecdb) {
            return new ConcurrentQueryAppData(ecdb);
        }
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ConcurrentQueryConfigAppData : Db::AppData {
    private:
        mutable ConcurrentQueryMgr::Config m_config;
    public:
        ConcurrentQueryConfigAppData(){
            m_config = ConcurrentQueryMgr::Config::GetFromEnv();
        }
        static Db::AppData::Key& GetKey() {
            static Db::AppData::Key s_key;
            return s_key;
        }
        ConcurrentQueryMgr::Config const& GetConfig() const { return m_config; }
        void SetConfig(ConcurrentQueryMgr::Config const& config) { m_config = config; }
        static RefCountedPtr<ConcurrentQueryConfigAppData> Create() {
            return new ConcurrentQueryConfigAppData();
        }
};
END_BENTLEY_SQLITE_EC_NAMESPACE
