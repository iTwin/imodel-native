/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
using namespace std::chrono_literals;
using namespace std::chrono;
#define LIMIT_VAR_COUNT "sys_ecdb_count"
#define LIMIT_VAR_OFFSET "sys_ecdb_offset"
static constexpr Json::StaticString json_autoExpireTimeForCompletedQuery() { return Json::StaticString("autoExpireTimeForCompletedQuery"); }
static constexpr Json::StaticString json_concurrent() { return Json::StaticString("concurrent"); }
static constexpr Json::StaticString json_cachedStatementsPerWorker() { return Json::StaticString("cachedStatementsPerWorker"); }
static constexpr Json::StaticString json_maxQueueSize() { return Json::StaticString("maxQueueSize"); }
static constexpr Json::StaticString json_minMonitorInterval() { return Json::StaticString("minMonitorInterval"); }
static constexpr Json::StaticString json_idolCleanupTime() { return Json::StaticString("idolCleanupTime"); }
static constexpr Json::StaticString json_completedTaskExpires() { return Json::StaticString("completedTaskExpires"); }
static constexpr Json::StaticString json_quota_timeLimit() { return Json::StaticString("timeLimit"); }
static constexpr Json::StaticString json_quota_memoryLimit() { return Json::StaticString("memoryLimit"); }
static constexpr Json::StaticString json_quota() { return Json::StaticString("quota"); }
static constexpr Json::StaticString json_limit_offset() { return Json::StaticString("offset"); }
static constexpr Json::StaticString json_limit_count() { return Json::StaticString("count"); }


//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
void ConcurrentQueryManager::Limit::ToJson(Json::Value& v) const
    {
    v = Json::Value(Json::ValueType::objectValue);
    v[json_limit_offset()] = static_cast<int>(m_offset);
    v[json_limit_count()] = static_cast<int>(m_count);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
ConcurrentQueryManager::Limit& ConcurrentQueryManager::Limit::FromJson(Json::Value const& v)
    {
    if (v.isMember(json_limit_offset()))
        m_offset = v[json_limit_offset()].asInt();

    if (v.isMember(json_limit_count()))
        m_count = v[json_limit_count()].asInt();

    return *this;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
void ConcurrentQueryManager::Quota::ToJson(Json::Value& v) const
    {
    v = Json::Value(Json::ValueType::objectValue);
    v[json_quota_timeLimit()] = static_cast<int>(m_timeLimit.count());
    v[json_quota_memoryLimit()] = m_memoryLimit;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
ConcurrentQueryManager::Quota& ConcurrentQueryManager::Quota::FromJson(Json::Value const& v)
    {
    if (v.isMember(json_quota_timeLimit()))
        m_timeLimit = seconds(v[json_quota_timeLimit()].asInt());

    if (v.isMember(json_quota_memoryLimit()))
        m_memoryLimit = v[json_quota_memoryLimit()].asInt();

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
void ConcurrentQueryManager::Config::ToJson(Json::Value& v) const
    {
    v = Json::Value(Json::ValueType::objectValue);
    v[json_idolCleanupTime()] = static_cast<int>(m_idolCleanupTime.count());
    v[json_minMonitorInterval()] = static_cast<int>(m_minMonitorInterval.count());
    v[json_completedTaskExpires()] = static_cast<int>(m_completedTaskExpires.count());
    v[json_cachedStatementsPerWorker()] = static_cast<int>(m_cacheStatementsPerThread);
    v[json_maxQueueSize()] = static_cast<int>(m_maxQueueSize);
    v[json_concurrent()] = static_cast<int>(m_concurrent);
    v[json_autoExpireTimeForCompletedQuery()] = static_cast<int>(m_completedTaskExpires.count());
    m_quota.ToJson(v[json_quota()]);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
ConcurrentQueryManager::Config& ConcurrentQueryManager::Config::FromJson(Json::Value const& v)
    {
    if (v.isMember(json_idolCleanupTime()))
        m_idolCleanupTime = seconds(v[json_idolCleanupTime()].asInt());

    if (v.isMember(json_minMonitorInterval()))
        m_minMonitorInterval = seconds(v[json_minMonitorInterval()].asInt());

    if (v.isMember(json_completedTaskExpires()))
        m_completedTaskExpires = seconds(v[json_completedTaskExpires()].asInt());

    if (v.isMember(json_cachedStatementsPerWorker()))
        m_cacheStatementsPerThread = static_cast<unsigned int>(v[json_cachedStatementsPerWorker()].asInt());

    if (v.isMember(json_maxQueueSize()))
        m_maxQueueSize = static_cast<unsigned int>(v[json_maxQueueSize()].asInt());

    if (v.isMember(json_concurrent()))
        m_concurrent = static_cast<unsigned int>(v[json_concurrent()].asInt());

    if (v.isMember(json_autoExpireTimeForCompletedQuery()))
        m_completedTaskExpires = seconds(v[json_autoExpireTimeForCompletedQuery()].asInt());

    return *this;
    }
//****************************ConcurrentQueryManager::Config*****************************
//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
ConcurrentQueryManager::Config::Config(const Config&& rhs)
    :m_afterConnectionOpenned(std::move(rhs.m_afterConnectionOpenned)),
    m_beforeConnectionClosed(std::move(rhs.m_beforeConnectionClosed)),
    m_concurrent(std::move(rhs.m_concurrent)),
    m_cacheStatementsPerThread(std::move(rhs.m_cacheStatementsPerThread)),
    m_maxQueueSize(std::move(rhs.m_maxQueueSize)),
    m_quota(std::move(rhs.m_quota)),
    m_idolCleanupTime(std::move(rhs.m_idolCleanupTime)),
    m_minMonitorInterval(std::move(rhs.m_minMonitorInterval)),
    m_completedTaskExpires(std::move(rhs.m_completedTaskExpires))
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
ConcurrentQueryManager::Config::Config(const Config& rhs)
    :m_afterConnectionOpenned(rhs.m_afterConnectionOpenned),
    m_beforeConnectionClosed(rhs.m_beforeConnectionClosed),
    m_concurrent(rhs.m_concurrent),
    m_cacheStatementsPerThread(rhs.m_cacheStatementsPerThread),
    m_maxQueueSize(rhs.m_maxQueueSize),
    m_quota(rhs.m_quota),
    m_idolCleanupTime(rhs.m_idolCleanupTime),
    m_minMonitorInterval(rhs.m_minMonitorInterval),
    m_completedTaskExpires(rhs.m_completedTaskExpires)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
ConcurrentQueryManager::Config& ConcurrentQueryManager::Config::operator= (const Config&& rhs)
    {
    if (this != &rhs)
        {
        m_afterConnectionOpenned = std::move(rhs.m_afterConnectionOpenned);
        m_beforeConnectionClosed = std::move(rhs.m_beforeConnectionClosed);
        m_concurrent = std::move(rhs.m_concurrent);
        m_cacheStatementsPerThread = std::move(rhs.m_cacheStatementsPerThread);
        m_maxQueueSize = std::move(rhs.m_maxQueueSize);
        m_quota = std::move(rhs.m_quota);
        m_idolCleanupTime = rhs.m_idolCleanupTime;
        m_minMonitorInterval = rhs.m_minMonitorInterval;
        m_completedTaskExpires = rhs.m_completedTaskExpires;
        }
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
ConcurrentQueryManager::Config& ConcurrentQueryManager::Config::operator= (const Config& rhs)
    {
    if (this != &rhs)
        {
        m_afterConnectionOpenned = rhs.m_afterConnectionOpenned;
        m_beforeConnectionClosed = rhs.m_beforeConnectionClosed;
        m_concurrent = rhs.m_concurrent;
        m_cacheStatementsPerThread = rhs.m_cacheStatementsPerThread;
        m_maxQueueSize = rhs.m_maxQueueSize;
        m_quota = rhs.m_quota;
        m_idolCleanupTime = rhs.m_idolCleanupTime;
        m_minMonitorInterval = rhs.m_minMonitorInterval;
        m_completedTaskExpires = rhs.m_completedTaskExpires;
        }
    return *this;
    }

//************************************QueryTask**************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
bool QueryTask::ExceededQuota() const noexcept
    {
    const auto timeQuotaExceeded = m_quota.MaxTimeAllowed() == seconds(0) ? false : GetTimeElapsed() >= m_quota.MaxTimeAllowed();
    const auto memoryQuotaExceeded = m_quota.MaxMemoryAllowed() == 0 ? false : GetMemoryUsed() >= m_quota.MaxMemoryAllowed();
    return timeQuotaExceeded || memoryQuotaExceeded;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
void QueryTask::SetError(Utf8CP error)
    {
    m_completedOn = steady_clock::now();
    m_interruptor = nullptr;
    m_result = error;
    m_state.store(State::Error);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
void QueryTask::SetPartial()
    {
    m_result.append("]");
    m_completedOn = steady_clock::now();
    m_interruptor = nullptr;
    m_state.store(State::Partial);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
void QueryTask::SetDone()
    {
    m_result.append("]");
    m_completedOn = steady_clock::now();
    m_interruptor = nullptr;
    m_state.store(State::Done);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
void QueryTask::Run(JsonAdaptorCache& cache, QueryInterruptor& interruptor)
    {
    if (m_state != State::Inqueue)
        return;

    m_state = State::Running;
    if (ExceededQuota())
        {
        m_state.store(State::Partial);
        m_interruptor = nullptr;
        return;
        }
   
    m_interruptor = &interruptor;
    auto entry = cache.Get(m_ecsql.c_str());
    if (entry == nullptr)
        {
        m_state.store(State::Error);
        return;
        }

    if (!m_bindings.empty())
        {
        const auto userBindings = Json::Value::From(m_bindings);
        if (userBindings.type() != Json::ValueType::arrayValue)
            {
            SetError("Expecting primitive json array for binding params"); return;
            }
        for (Json::ArrayIndex i = 0; i < userBindings.size(); i++)
            {
            const auto type = userBindings[i].type();
            if (type == Json::ValueType::objectValue ||
                type == Json::ValueType::arrayValue ||
                type == Json::ValueType::nullValue)
                {
                SetError("Json object, array and null is not acceptable values for binding"); return;
                }
            else if (type == Json::ValueType::booleanValue)
                entry->GetStatement()->BindBoolean(i + 1, userBindings[i].asBool());
            else if (type == Json::ValueType::intValue)
                entry->GetStatement()->BindInt(i + 1, userBindings[i].asInt());
            else if (type == Json::ValueType::realValue)
                entry->GetStatement()->BindDouble(i + 1, userBindings[i].asDouble());
            else if (type == Json::ValueType::stringValue)
                entry->GetStatement()->BindText(i + 1, userBindings[i].asCString(), EC::IECSqlBinder::MakeCopy::Yes);
            else if (type == Json::ValueType::uintValue)
                entry->GetStatement()->BindInt(i + 1, userBindings[i].asUInt());
            }
        }

    const auto idxCount = entry->GetStatement()->GetParameterIndex(LIMIT_VAR_COUNT);
    const auto idxOffset = entry->GetStatement()->GetParameterIndex(LIMIT_VAR_OFFSET);
    // bind limit
    entry->GetStatement()->BindInt64(idxCount, m_limit.GetCount());
    entry->GetStatement()->BindInt64(idxOffset, m_limit.GetOffset());
    Json::Value rows(Json::ValueType::objectValue);
    bool firstRow = true;
    m_result.reserve(QUERY_WORKER_RESULT_RESERVE_BYTES);
    m_result = "[";
    DbResult rc = entry->GetStatement()->Step();
    while (rc == BE_SQLITE_ROW)
        {
        if (entry->GetJsonAdaptor()->GetRow(rows) != SUCCESS)
            {
            SetError("Fail to convert row to json");
            return;
            }
        else
            {
            m_rows = m_rows + 1;
            if (firstRow)
                {
                firstRow = false;
                m_result.append(rows.ToString());
                }
            else
                m_result.append(",\n").append(rows.ToString());
            }

        if (ExceededQuota())
            {            
            SetPartial();
            return;
            }

        rc = entry->GetStatement()->Step();
        }

    if (rc == BE_SQLITE_INTERRUPT)
        {
        SetPartial();
        return;
        }
    else if (rc != BE_SQLITE_DONE)
        {
        SetError(interruptor.GetLastError().c_str());
        return;
        }
    SetDone();
    }

//************************************JsonAdaptorCache**************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
JsonAdaptorCache::CacheEntry* JsonAdaptorCache::Get(Utf8CP ecsql)
    {
    auto itor = std::find_if(m_stmt.begin(), m_stmt.end(), [&ecsql] (CacheEntry& entry)
        {
        return  strcmp(entry.GetStatement()->GetECSql(), ecsql) == 0;
        });

    if (itor != m_stmt.end())
        { 
        CacheEntry entry = std::move((*itor));
        m_stmt.erase(itor);
        m_stmt.insert(m_stmt.begin(), std::move(entry));
        auto stmt = m_stmt.front().GetStatement();
        stmt->Reset();
        stmt->ClearBindings();
        return &m_stmt.front();
        }
    auto& ecdb = m_worker.GetPool().GetMgr()->GetECDb();
    auto& workerConn = m_worker.GetConnection();

    CacheEntry entry;    
    ecdb.GetImpl().GetMutex().Enter();
    auto rc = entry.GetStatement()->Prepare(ecdb.Schemas(), workerConn, ecsql);
    ecdb.GetImpl().GetMutex().Leave();
    if (rc != ECSqlStatus::Success)
        return nullptr;

    while (m_stmt.size() >= m_cacheSize - 1)
        m_stmt.pop_back();

    auto it = m_stmt.insert(m_stmt.begin(), std::move(entry));
    return &(*it);
    }

//*************************************QueryWorker**************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
QueryWorker::QueryWorker(QueryWorkerPool& workerPool)
    :m_workerPool(workerPool), m_stmtCache(*this, workerPool.m_mgr->GetConfig().GetCacheStatementPerThread())
    {
    Init();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
void QueryWorker::FreeMemory()
    {
    m_stmtCache.Clear();
    if (m_db.IsDbOpen())
        m_db.FreeMemory();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
void QueryWorker::Run(QueryWorker* worker)
    {
    worker->m_started.set_value();
    for (;;)
        {
        QueryTask* task = nullptr;
        if (true)
            {
            auto& pool = worker->m_workerPool;
            std::unique_lock<std::mutex> lock(pool.m_mutex);
            pool.m_taskVar.wait(lock, [&] ()
                {
                return (pool.m_stop || !pool.m_queue.empty());
                });
            
            if (pool.m_stop)
                return;

            // find task
            while (!pool.m_queue.empty() && task == nullptr)
                {
                auto nextTask = pool.m_queue.top();
                pool.m_queue.pop();
                if (nextTask->GetState() == QueryTask::State::Inqueue)
                    task = nextTask;                    
                }
            }

        // execute query
        if (task != nullptr)
            {
            QueryInterruptor interruptor(worker->m_db);
            task->Run(worker->m_stmtCache, interruptor);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
void QueryWorker::Init()
    {
    m_retryHandler = new RetryHandler();
    auto rc = m_db.OpenBeSQLiteDb(m_workerPool.GetMgr()->GetECDb().GetDbFileName(), Db::OpenParams((Db::OpenMode)((int) Db::OpenMode::Readonly), DefaultTxn::Yes, m_retryHandler.get()));

    if (rc != BE_SQLITE_OK)
        {
        BeAssert(rc != BE_SQLITE_OK);
        }

    auto& cb = m_workerPool.m_mgr->GetConfig().GetAfterConnectionOpenedCallback();
    if (cb)
        cb(m_db);

    m_thread = std::thread(Run, this);
    m_started.get_future().wait();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
QueryWorker::~QueryWorker()
    {
    if (m_db.IsDbOpen())
        {
        auto& cb = m_workerPool.m_mgr->GetConfig().GetBeforeConnectionClosedCallback();
        if (cb)
            cb(m_db);
        }
    m_stmtCache.Clear();
    m_db.CloseDb();
    }

//**********************************QueryWorkerPool**************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
QueryWorkerPool::QueryWorkerPool(ConcurrentQueryManager::Impl* mgr)
    :m_mgr(mgr),m_stop(false)
    {
    m_maxWorkers = std::max(1u, std::min(m_mgr->GetConfig().GetConcurrent(), std::thread::hardware_concurrency()));
    for (unsigned int i = 0; i < m_maxWorkers; i++)
        m_workers.push_back(std::unique_ptr<QueryWorker>(new QueryWorker(*this)));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
QueryWorkerPool::~QueryWorkerPool()
    {
    if (true)
        {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_queue.empty())
            m_queue.pop();
        m_stop = true;
        m_taskVar.notify_all();
        }

    for (auto& worker : m_workers)
        worker->Join();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
void QueryWorkerPool::FreeMemory()
    {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty())
        {
        for (auto& worker : m_workers)
            worker->FreeMemory();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
bool QueryWorkerPool::Enqueue(QueryTask* task)
    {
    std::lock_guard<std::mutex> lock(m_mutex);
    const auto maxQueueSize = m_mgr->GetConfig().GetMaxQueueSize();
    if (maxQueueSize && m_queue.size() >= maxQueueSize)
        return false;
    
    m_queue.push(task);
    m_taskVar.notify_one();
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
void QueryWorkerPool::Remove(QueryTask* task)
    {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.remove(task);
    }

//************************************ConcurrentQueryManager::Impl****************************
//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
uint32_t ConcurrentQueryManager::Impl::NextId() const
    {    
    TaskId i;
    do
        {
        i = dist(m_mt);
        } while (m_tasks.find(i) != m_tasks.end());
        return i;
    }

//************************************ConcurrentQueryManager****************************
//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
ConcurrentQueryManager::ConcurrentQueryManager(ECDbCR ecdb)
    {
    m_pimpl = new ConcurrentQueryManager::Impl(ecdb);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
bool ConcurrentQueryManager::IsInitalized() const
    {
    return this->m_pimpl->IsInitalized();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
bool ConcurrentQueryManager::Initalize(Config config)
    {
    return this->m_pimpl->Initalize(config);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
ConcurrentQueryManager::~ConcurrentQueryManager() 
    {
    delete this->m_pimpl;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
ConcurrentQueryManager::PostStatus ConcurrentQueryManager::PostQuery(TaskId& taskId, Utf8CP ecsql, Utf8CP bindings, Limit limit, ConcurrentQueryManager::Quota quota,  ConcurrentQueryManager::Priority priority)
    {
    return m_pimpl->PostQuery(taskId, ecsql, bindings, limit, quota, priority);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
ConcurrentQueryManager::PollStatus ConcurrentQueryManager::PollQuery(Utf8StringR resultJson, int64_t& rows, TaskId taskId)
    {
    return m_pimpl->PollQuery(resultJson, rows, taskId);
    }

//********************************ConcurrentQueryManager::Impl***************************
//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
void ConcurrentQueryManager::Impl::Monitor(ConcurrentQueryManager::Impl* mgr)
    {
    const auto minIntervalTime = mgr->GetConfig().GetMinMonitorInterval();
    for (;;)
        {
        if (mgr->m_stop)
            return;
        
        auto nextTimeoutToWakeUp = mgr->GetConfig().GetMinMonitorInterval();
        //check if any inqueue task has expired
        if (true)
            {
            std::lock_guard<std::mutex> lock(mgr->m_mutex);
            seconds timeElapsed = duration_cast<seconds>(steady_clock::now() - mgr->m_lastTimeActive);
            if (timeElapsed > mgr->m_config.GetIdolCleanupTime())
                {
                 mgr->GetWorkerPool().FreeMemory();
                 mgr->m_lastTimeActive = steady_clock::now();
                }

            // House keeping
            std::this_thread::yield();
            const auto houseKeepingStart = steady_clock::now();
            for (auto& kp : mgr->m_tasks)
                {
                QueryTask* task = kp.second.get();
                if (task->GetState() == QueryTask::State::Inqueue || task->GetState() == QueryTask::State::Running)
                    {
                    if (task->ExceededQuota())
                        {
                        if (auto itp = task->GetInterruptor())
                            itp->Interrupt();
                        else if (task->GetState() == QueryTask::State::Inqueue)
                            task->SetState(QueryTask::State::Timeout);
                        }
                    }
                else if (mgr->m_config.GetAutoExpireTimeForCompletedQuery() > seconds(0))
                    {
                    auto timeElaspedSinceCompleted = duration_cast<seconds>(steady_clock::now() - task->CompletedOn());
                    if (timeElaspedSinceCompleted > mgr->m_config.GetAutoExpireTimeForCompletedQuery())
                        {
                        mgr->m_autoCleanupTask.push_back(task->GetId());
                        }
                    }
                }

            // clean up task
            std::this_thread::yield();
            if (!mgr->m_autoCleanupTask.empty())
                {
                for (auto& id : mgr->m_autoCleanupTask)
                    {
                    auto it = mgr->m_tasks.find(id);
                    if (it != mgr->m_tasks.end())
                        mgr->m_tasks.erase(it);
                    }
                mgr->m_autoCleanupTask.clear();
                }

            //next time to wake up depend on how muct time it took to run this time
            const auto houseKeepingElapsed = duration_cast<seconds>(steady_clock::now() - houseKeepingStart);
            if (houseKeepingElapsed < minIntervalTime)
                nextTimeoutToWakeUp = minIntervalTime;
            else
                nextTimeoutToWakeUp = houseKeepingElapsed + minIntervalTime;
            }
        std::this_thread::sleep_for(nextTimeoutToWakeUp);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
bool ConcurrentQueryManager::Impl::IsInitalized() const
    {
    return m_initalized;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
bool ConcurrentQueryManager::Impl::Initalize(Config config)
    {
    if (m_initalized)
        return false;

    m_config = config;
    m_workerPool = std::make_unique< QueryWorkerPool>(this);
    m_monitor = std::thread(Monitor, this);
    m_initalized = true;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
ConcurrentQueryManager::Impl::Impl(ECDbCR ecdb)
    :m_ecdb(ecdb), m_initalized(false), m_stop(false), m_mt(m_rd())
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
ConcurrentQueryManager::Impl::~Impl() 
    { 
    m_stop = true; 
    if (m_initalized)
        m_monitor.join(); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
ConcurrentQueryManager::PostStatus ConcurrentQueryManager::Impl::PostQuery(TaskId& taskId, Utf8CP ecsql, Utf8CP bindings, Limit limit, ConcurrentQueryManager::Quota quota, ConcurrentQueryManager::Priority priority)
    {
    if (!m_initalized)
        return PostStatus::NotInitalized;

    if (quota.IsEmpty())
        quota = m_config.GetQuota();
    else
        {
        // constraint user provide quota limit by global limit set.
        std::chrono::seconds resolvedTimeAllowed = m_config.GetQuota().MaxTimeAllowed();
        if (quota.MaxTimeAllowed() != std::chrono::seconds(0))
            resolvedTimeAllowed = std::min(quota.MaxTimeAllowed(), m_config.GetQuota().MaxTimeAllowed());

        uint32_t memoryLimit = m_config.GetQuota().MaxMemoryAllowed();
        if (memoryLimit != 0)
            memoryLimit = std::min(quota.MaxMemoryAllowed(), m_config.GetQuota().MaxMemoryAllowed());

        quota = Quota(resolvedTimeAllowed, memoryLimit);
        }

    std::lock_guard<std::mutex> guard(m_mutex);
    auto id = NextId();
    const Utf8String limitQuery = Utf8PrintfString("select * from (%s) limit :" LIMIT_VAR_COUNT " offset :" LIMIT_VAR_OFFSET , ecsql);
    auto itor = m_tasks.insert(std::make_pair(id, std::make_unique<QueryTask>(id, limitQuery.c_str(), bindings, limit, quota, priority)));
    auto task = itor.first->second.get();
    if (m_workerPool->Enqueue(task))
        {
        taskId = task->GetId();
        m_lastTimeActive = steady_clock::now();
        return PostStatus::Done;
        }
   
    m_tasks.erase(task->GetId());
    return PostStatus::QueueSizeExceded;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
ConcurrentQueryManager::PollStatus ConcurrentQueryManager::Impl::PollQuery(Utf8StringR resultJson, int64_t& rows, TaskId taskId)
    {
    if (!m_initalized)
        return PollStatus::NotInitalized;

    std::lock_guard<std::mutex> guard(m_mutex);   
    auto it = m_tasks.find(taskId);
    if (it == m_tasks.end())
        return PollStatus::NotFound;

    m_lastTimeActive = steady_clock::now();
    auto task = it->second.get();
    if (task->GetState() == QueryTask::State::Error)
        {
        m_workerPool->Remove(it->second.get());
        m_tasks.erase(it);
        return PollStatus::Error;
        }
    if (task->GetState() == QueryTask::State::Timeout)
        {
        m_workerPool->Remove(it->second.get());
        m_tasks.erase(it);
        return PollStatus::Timeout;
        }

    if (task->GetState() == QueryTask::State::Done)
        {
        auto result = PollStatus::Done;
        rows = task->GetRowCount();
        resultJson = std::move(task->GetResult());
        m_workerPool->Remove(it->second.get());
        m_tasks.erase(it);
        return result;
        }

    if (task->GetState() == QueryTask::State::Partial)
        {
        auto result = PollStatus::Partial;
        rows = task->GetRowCount();
        resultJson = std::move(task->GetResult());
        m_workerPool->Remove(it->second.get());
        m_tasks.erase(it);
        return result;
        }

    return PollStatus::Pending;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
