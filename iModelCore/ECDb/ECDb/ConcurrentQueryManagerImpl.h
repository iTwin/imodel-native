/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECDb/ConcurrentQueryManager.h>
#include <queue>
#include <map>
#include <thread>
#include <future>
#include <random>

#define QUERY_WORKER_RESULT_RESERVE_BYTES 1024*4 
#define QUERY_MONITOR_SLEEP_TIME 1 // milliseconds
#define BASE64_HEADER "encoding=base64;"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
struct QueryWorker;
struct QueryWorkerPool;
struct QueryTask;
//=======================================================================================
//! @bsiclass                                                        05/2019
//=======================================================================================
struct ConcurrentQueryManager::Impl final
    {
    private:
        std::mutex m_mutex;
        std::map<TaskId, std::unique_ptr<QueryTask>> m_tasks;
        Config m_config;
        std::unique_ptr<QueryWorkerPool> m_workerPool;
        std::thread m_monitor;
        std::vector<TaskId> m_autoCleanupTask;
        std::chrono::time_point<std::chrono::steady_clock> m_lastTimeActive;
        ECDb const& m_ecdb;
        bool m_stop;
        bool m_initalized;
        TaskId NextId() const;
        static void Monitor(ConcurrentQueryManager::Impl* mgr);
        mutable std::random_device m_rd;
        mutable std::uniform_int_distribution<TaskId> dist;
        mutable std::mt19937 m_mt;

    public:
        explicit Impl(ECDb const& ecdb);
        ~Impl();
        bool IsStopped() const { return m_stop; }
        QueryWorkerPool& GetWorkerPool() { return *m_workerPool; }
        Config const& GetConfig() const { return m_config; }
        ECDbCR GetECDb() const { return m_ecdb; }
        PostStatus PostQuery(TaskId& taskId, Utf8CP ecsql, Utf8CP bindings, Limit limit, Quota quota, Priority priority);
        PollStatus PollQuery(Utf8StringR resultJson, int64_t& rows, TaskId taskId);
        bool IsInitalized() const;
        bool Initalize(Config config);
    };

//=======================================================================================
//! @bsiclass                                                        05/2019
//=======================================================================================
struct JsonAdaptorCache final
    {
    struct CacheEntry final
        {
        private:
            std::unique_ptr<ECSqlStatement> m_stmt;
            std::unique_ptr<JsonECSqlSelectAdapter> m_adaptor;
        public:
            CacheEntry(const CacheEntry&) = delete;
            CacheEntry& operator =(const CacheEntry&) = delete;
            CacheEntry() : m_stmt(new ECSqlStatement()) {}
            CacheEntry(CacheEntry&& rhs)
                :m_stmt(std::move(rhs.m_stmt)), m_adaptor(std::move(rhs.m_adaptor))
                {}
            CacheEntry& operator =(CacheEntry&& rhs)
                {
                if (this != &rhs)
                    {
                    m_stmt = std::move(rhs.m_stmt);
                    m_adaptor = std::move(rhs.m_adaptor);
                    }
                return *this;
                }
            ECSqlStatement* GetStatement() { return m_stmt.get(); }
            JsonECSqlSelectAdapter* GetJsonAdaptor()
                {
                if (!m_adaptor)
                    {
                    auto options = JsonECSqlSelectAdapter::FormatOptions();
                    options.SetRowFormat(JsonECSqlSelectAdapter::RowFormat::IModelJs);
                    m_adaptor = std::unique_ptr<JsonECSqlSelectAdapter>(new JsonECSqlSelectAdapter(*m_stmt, options));
                    }

                return m_adaptor.get();
                }
        };
    private:
        std::vector<CacheEntry> m_stmt;
        unsigned int m_cacheSize;
        QueryWorker& m_worker;
    public:
        JsonAdaptorCache(const JsonAdaptorCache& rhs) = delete;
        JsonAdaptorCache& operator = (const JsonAdaptorCache& rhs) = delete;

        explicit JsonAdaptorCache(QueryWorker& worker, unsigned int cacheSize) :m_cacheSize(cacheSize), m_worker(worker) {}
        ~JsonAdaptorCache(){}
        CacheEntry* Get(Utf8CP ecsql);
        void Clear() { m_stmt.clear(); }
    };

//=======================================================================================
//! @bsiclass                                                        05/2019
//=======================================================================================
struct QueryInterruptor final
    {
    private:
        Db& m_db;
    public:
        QueryInterruptor(Db& db)
            :m_db(db)
            {}
        ~QueryInterruptor() {}
        void Interrupt() const
            {
            m_db.Interrupt();
            }
        Utf8String GetLastError() const { return m_db.GetLastError(); }
    };

//=======================================================================================
//! @bsiclass                                                        05/2019
//=======================================================================================
struct QueryTask final
    {
    enum class State
        {
        Inqueue,
        Running,
        Timeout,
        Partial, // partial result was sent
        Done,
        Error
        };
    struct ComparePriority { bool operator()(QueryTask* rhs, QueryTask* lhs) { return (int) rhs->m_priority > (int) lhs->m_priority; } };
    private:
        ConcurrentQueryManager::Priority m_priority;
        Utf8String m_ecsql;
        Utf8String m_result;
        std::atomic<State> m_state;
        std::atomic<QueryInterruptor*> m_interruptor;
        std::chrono::time_point<std::chrono::steady_clock> m_createdOn;
        std::chrono::time_point<std::chrono::steady_clock> m_completedOn;
        ConcurrentQueryManager::Limit m_limit;
        TaskId m_id;
        int64_t m_rows;
        Utf8String m_bindings;
        ConcurrentQueryManager::Quota m_quota;
        void SetError(Utf8CP error);
        void SetPartial();
        void SetDone();
        BentleyStatus BindPrimitive(ECSqlStatement* stmt, Json::Value const& v, int index);
    public:
        QueryTask(const QueryTask& rhs) = delete;
        QueryTask& operator = (const QueryTask& rhs) = delete;
        explicit QueryTask(TaskId id, Utf8CP ecsql, Utf8CP bindings, ConcurrentQueryManager::Limit limit, ConcurrentQueryManager::Quota quota, ConcurrentQueryManager::Priority priority) :
            m_ecsql(ecsql), m_priority(priority), m_createdOn(std::chrono::steady_clock::now()), m_state(State::Inqueue), m_id(id), m_interruptor(nullptr), m_limit(limit), m_rows(0), m_quota(quota), m_bindings(bindings) {}
        ~QueryTask() {}
        TaskId GetId() const { return m_id; }
        std::chrono::time_point<std::chrono::steady_clock> CreatedOn() const noexcept { return m_createdOn; }
        std::chrono::time_point<std::chrono::steady_clock> CompletedOn() const noexcept { return m_completedOn; }
        Utf8StringCR GetQuery() const noexcept { return m_ecsql; }
        ConcurrentQueryManager::Priority GetPriority() const noexcept { return m_priority; }
        State GetState() const noexcept { return m_state; }
        int64_t GetRowCount() const noexcept { return m_rows; }
        std::chrono::seconds GetTimeElapsed() const { return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_createdOn); }
        size_t GetMemoryUsed() const { return m_result.SizeInBytes(); }
        bool ExceededQuota() const noexcept;
        void SetState(State state) noexcept { m_state = state; }
        void Run(JsonAdaptorCache& cache, QueryInterruptor& interruptor);
        Utf8StringR GetResult() { return m_result; }
        QueryInterruptor* GetInterruptor() const { return m_interruptor; }
    };

struct QueryWorkerPool;
//=======================================================================================
//! @bsiclass                                                        05/2019
//=======================================================================================
struct QueryWorker final
    {
    private:
        struct RetryHandler final : BeSQLite::BusyRetry
            {
            private:
                virtual int _OnBusy(int count) const
                    {
                    if (count > 10)
                        return 0;

                    BeThreadUtilities::BeSleep(1000); return 1;
                    }
            };

        JsonAdaptorCache m_stmtCache;
        QueryWorkerPool& m_workerPool;
        std::thread m_thread;
        std::promise<void> m_started;
        RefCountedPtr<RetryHandler> m_retryHandler;
        Db m_db;
        void Init();
        static void Run(QueryWorker* worker);
    public:
        QueryWorkerPool& GetPool() { return m_workerPool; }
        Db const& GetConnection() const { return m_db; }
        explicit QueryWorker(QueryWorkerPool& workerPool);
        void Join() { m_thread.join(); }
        void FreeMemory();
        bool IsDbOpen() const { return m_db.IsDbOpen(); }
        ~QueryWorker();
    };

//=======================================================================================
//! @bsiclass                                                        05/2019
//=======================================================================================
struct QueryWorkerPool final
    {
    friend struct QueryWorker;
     class task_priority_queue : public std::priority_queue<QueryTask*, std::vector<QueryTask*>, QueryTask::ComparePriority>
        {
        public:
            bool remove(const QueryTask* value)
                {
                auto it = std::find(this->c.begin(), this->c.end(), value);
                if (it != this->c.end())
                    {
                    this->c.erase(it);
                    std::make_heap(this->c.begin(), this->c.end(), this->comp);
                    return true;
                    }
                 return false;
                }
        };
    private:
        task_priority_queue m_queue;
        std::condition_variable m_taskVar;
        mutable std::mutex m_mutex;
        std::vector <std::unique_ptr<QueryWorker>> m_workers;
        bool m_stop;
        unsigned int m_maxWorkers;
        ConcurrentQueryManager::Impl* m_mgr;
        bool m_isInitialized;
        void Shutdown();

    public:
        explicit QueryWorkerPool(ConcurrentQueryManager::Impl* mgr);
        ConcurrentQueryManager::Impl* GetMgr() const { return m_mgr; }
        bool Enqueue(QueryTask* task);
        void Remove(QueryTask* task);
        void FreeMemory();
        size_t GetUnSafeSize() const { return m_queue.size(); }
        bool IsInitialized() const { return m_isInitialized; }
        bool Empty() const { std::lock_guard<std::mutex> lock(m_mutex); return m_queue.empty(); }
        ~QueryWorkerPool();
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
