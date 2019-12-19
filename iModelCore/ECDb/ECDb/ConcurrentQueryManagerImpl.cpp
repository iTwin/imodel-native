/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
using namespace std::chrono_literals;
using namespace std::chrono;
#define LIMIT_VAR_COUNT "sys_ecdb_count"
#define LIMIT_VAR_OFFSET "sys_ecdb_offset"
#define BUSY_RETRY_TIMEOUT 100 // ms
static BentleyApi::NativeLogging::ILogger *s_logger = NativeLogging::LoggingManager::GetLogger(L"ECDb.ConcurrentQuery");
#define CQLOG (*s_logger)

//****************************ConcurrentQueryManager::Config*****************************
//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
ConcurrentQueryManager::Config::Config():
    m_concurrent(std::thread::hardware_concurrency()),
    m_cacheStatementsPerThread(20),
    m_maxQueueSize(std::thread::hardware_concurrency() * 1000),
    m_idleCleanupTime(20min),
    m_minMonitorInterval(1s),
    m_completedTaskExpires(1min),
    m_quota(),
    m_useSharedCache(false),
    m_useUncommitedRead(false),
    m_useImmutableDb(false)
    {}


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
    m_idleCleanupTime(std::move(rhs.m_idleCleanupTime)),
    m_minMonitorInterval(std::move(rhs.m_minMonitorInterval)),
    m_completedTaskExpires(std::move(rhs.m_completedTaskExpires)),
    m_useSharedCache(std::move(rhs.m_useSharedCache)),
    m_useUncommitedRead(std::move(rhs.m_useUncommitedRead)),
    m_useImmutableDb(std::move(rhs.m_useImmutableDb))
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
    m_idleCleanupTime(rhs.m_idleCleanupTime),
    m_minMonitorInterval(rhs.m_minMonitorInterval),
    m_completedTaskExpires(rhs.m_completedTaskExpires),
    m_useSharedCache(rhs.m_useSharedCache),
    m_useUncommitedRead(rhs.m_useUncommitedRead),
    m_useImmutableDb(rhs.m_useImmutableDb)
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
        m_idleCleanupTime = rhs.m_idleCleanupTime;
        m_minMonitorInterval = rhs.m_minMonitorInterval;
        m_completedTaskExpires = rhs.m_completedTaskExpires;
        m_useSharedCache = rhs.m_useSharedCache;
        m_useUncommitedRead = rhs.m_useUncommitedRead;
        m_useImmutableDb = rhs.m_useImmutableDb;
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
        m_idleCleanupTime = rhs.m_idleCleanupTime;
        m_minMonitorInterval = rhs.m_minMonitorInterval;
        m_completedTaskExpires = rhs.m_completedTaskExpires;
        m_useSharedCache = std::move(rhs.m_useSharedCache);
        m_useUncommitedRead = std::move(rhs.m_useUncommitedRead);
        m_useImmutableDb = std::move(rhs.m_useImmutableDb);
        }
    return *this;
    }

//************************************QueryTask**************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
bool QueryTask::ExceededQuota(QuotaType type) const noexcept
    {
    if (((int)type & (int)QuotaType::Time) > 0)
        {
        const auto timeQuotaExceeded = m_quota.MaxTimeAllowed() == seconds(0) ? false : GetTimeElapsed() >= m_quota.MaxTimeAllowed();
        if (timeQuotaExceeded)
            return true;
        }

    if (((int) type & (int)QuotaType::Memory) > 0)
        {
        const auto memoryQuotaExceeded = m_quota.MaxMemoryAllowed() == 0 ? false : GetMemoryUsed() >= m_quota.MaxMemoryAllowed();
        if (memoryQuotaExceeded)
            return true;
        }

    return false;
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
void QueryTask::SetPartialWithNoResults()
    {
    m_result.clear();
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
BentleyStatus QueryTask::BindPrimitive(ECSqlStatement* stmt, Json::Value const& v, int index)
    {
    const auto type = v.type();
    if (type == Json::ValueType::objectValue ||
        type == Json::ValueType::arrayValue ||
        type == Json::ValueType::nullValue)
        {
        SetError("Json object, array and null is not acceptable values for binding"); return ERROR;
        }
    else if (type == Json::ValueType::booleanValue)
        stmt->BindBoolean(index, v.asBool());
    else if (type == Json::ValueType::intValue)
        stmt->BindInt(index, v.asInt());
    else if (type == Json::ValueType::realValue)
        stmt->BindDouble(index, v.asDouble());
    else if (type == Json::ValueType::stringValue)
        {
        const size_t headerSize = strlen(BASE64_HEADER);
        const size_t stringSize = strlen(v.asCString());
        if (stringSize > headerSize)
            {
            if (strncmp(BASE64_HEADER, v.asCString(), headerSize) == 0)
                {
                const auto payLoadLen = stringSize - headerSize;
                ByteStream stream;
                Base64Utilities::Decode(stream, v.asCString() + headerSize, payLoadLen);
                stmt->BindBlob(index, stream.GetData(), stream.GetSize(), EC::IECSqlBinder::MakeCopy::Yes);
                return SUCCESS;
                }
            }
        stmt->BindText(index, v.asCString(), EC::IECSqlBinder::MakeCopy::Yes);
        }
    else if (type == Json::ValueType::uintValue)
        stmt->BindInt(index, v.asUInt());

    return SUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
void QueryTask::Run(JsonAdaptorCache& cache, QueryInterruptor& interruptor)
    {
    if (m_state != State::Inqueue)
        return;

    m_requestContext.OnTaskStart();
    m_state = State::Running;
    if (ExceededQuota())
        {
        SetPartialWithNoResults();
        return;
        }
   
    m_interruptor = &interruptor;
    ECSqlStatus prepareStatus;
    auto entry = cache.Get(m_ecsql.c_str(), &prepareStatus);
    if (entry == nullptr)
        {
        if (prepareStatus.IsSQLiteError() && prepareStatus.GetSQLiteError() == BE_SQLITE_BUSY)
            SetPartialWithNoResults();
        else
            SetError(interruptor.GetLastError().c_str());
        return;
        }
    ECSqlStatement* pstmt = entry->GetStatement();
    if (!m_bindings.empty())
        {
        const auto userBindings = Json::Value::From(m_bindings);
        if (userBindings.type() == Json::ValueType::objectValue)
            {
            for (auto& mb : userBindings.getMemberNames())
                {
                const int idx = pstmt->GetParameterIndex(mb.c_str());
                if (idx < 1)
                    {
                    SetError(SqlPrintfString("Failed to find parameter '%s'", mb.c_str()));
                    return;
                    }

                auto& v = userBindings[mb];
                if (BindPrimitive(pstmt, v, idx) != SUCCESS)
                    return;
                }
            }
        else if (userBindings.type() == Json::ValueType::arrayValue)
            {
            for (Json::ArrayIndex i = 0; i < userBindings.size(); i++)
                {
                auto& v = userBindings[i];
                const auto type = v.type();
                if (type == Json::ValueType::objectValue)
                    {
                    SetError("Object is not a acceptable value as parameter unless its top level object");
                    return;
                    }
                else
                    {
                    if (BindPrimitive(pstmt, v, i + 1) != SUCCESS)
                        return;
                    }
                }
            }
        else
            {
            SetError("Only object or array is acceptable binding top level types");
            return;
            }
        }

    const auto idxCount = pstmt->GetParameterIndex(LIMIT_VAR_COUNT);
    const auto idxOffset = pstmt->GetParameterIndex(LIMIT_VAR_OFFSET);
    // bind limit
    pstmt->BindInt64(idxCount, m_limit.GetCount());
    pstmt->BindInt64(idxOffset, m_limit.GetOffset());
    Json::Value rows(Json::ValueType::objectValue);
    bool firstRow = true;
    m_result.reserve(QUERY_WORKER_RESULT_RESERVE_BYTES);
    m_result = "[";
    DbResult rc = pstmt->Step();
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
                m_result.append(",").append(rows.ToString());
            }

        if (ExceededQuota())
            {
            SetPartial();
            return;
            }

        rc = pstmt->Step();
        }

    if (rc == BE_SQLITE_INTERRUPT || rc == BE_SQLITE_BUSY)
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
JsonAdaptorCache::CacheEntry* JsonAdaptorCache::Get(Utf8CP ecsql, ECSqlStatus* status)
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
    // ecdb.GetImpl().GetMutex().Enter();
    auto rc = entry.GetStatement()->Prepare(ecdb.Schemas(), workerConn, ecsql);
    // ecdb.GetImpl().GetMutex().Leave();
    if (rc != ECSqlStatus::Success)
        {
        if (status)
            *status = rc;
        return nullptr;
        }

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
    :m_workerPool(workerPool), m_stmtCache(*this, workerPool.m_mgr->GetConfig().GetCacheStatementPerThread()), m_retryHandler(RetryHandler::Create())
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
            RetryHandler::Scope scope(*worker->m_retryHandler, *task);
            if (worker->GetPool().GetMgr()->GetConfig().GetUseImmutableDb() && worker->m_db.IsReadonly())
                {
                task->Run(worker->m_stmtCache, interruptor);
                }
            else
                {
                Savepoint txn(worker->m_db, "QueryWorker::Run");
                task->Run(worker->m_stmtCache, interruptor);
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
int QueryWorker::RetryHandler::_OnBusy(int count) const 
    {
    printf("Worker %d\n", count);
    if (m_task && m_task->ExceededQuota(QueryTask::QuotaType::Time))
        return 0;

    BeThreadUtilities::BeSleep(BUSY_RETRY_TIMEOUT);
    return 1;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
void QueryWorker::Init()
    {
    // m_retryHandler = RetryHandler::Create();
    const auto isErrorEnabled = CQLOG.isSeverityEnabled(NativeLogging::LOG_ERROR);
    const auto useImmutableDb = m_workerPool.GetMgr()->GetConfig().GetUseImmutableDb() && m_workerPool.GetMgr()->GetECDb().IsReadonly();
    const auto useUncommitedRead = m_workerPool.GetMgr()->GetConfig().GetUseUncommitedRead();
    const auto useSharedCache = m_workerPool.GetMgr()->GetConfig().GetUseSharedCache();
    const auto txn = useImmutableDb ? DefaultTxn::Yes : DefaultTxn::No;
    const auto openMode = (Db::OpenMode)(useSharedCache && !useImmutableDb ? 
        (int)Db::OpenMode::Readonly | (int)Db::OpenMode::SharedCache : (int)Db::OpenMode::Readonly);

    const auto openParam = Db::OpenParams(openMode, txn, m_retryHandler.get());
    if (useImmutableDb)
        {
        openParam.SetQueryParam("immutable", "true");
        CQLOG.info("ConncurrentQuery initialized with immutable connection to SQLiteDb");
        }
        
    auto rc = m_db.OpenBeSQLiteDb(m_workerPool.GetMgr()->GetECDb().GetDbFileName(), openParam);
    if (rc != BE_SQLITE_OK)
        {
        if (isErrorEnabled) 
            CQLOG.errorv("Failed to open SQLite connection: %s", m_db.GetLastError().c_str());

        BeAssert(rc != BE_SQLITE_OK);
        return;
        }
    if (useUncommitedRead && !useImmutableDb)
        {
        rc = m_db.TryExecuteSql("PRAGMA read_uncommitted=true");
        if (rc != BE_SQLITE_OK)
            {
            if (isErrorEnabled)
                CQLOG.errorv("Failed to apply read uncommitted flag: %s", m_db.GetLastError().c_str());
            m_db.CloseDb();
            BeAssert(rc != BE_SQLITE_OK);
            return;
            }
        }
    auto& cb = m_workerPool.m_mgr->GetConfig().GetAfterConnectionOpenedCallback();
    if (cb)
        {
        cb(m_db);
        }

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
    :m_mgr(mgr),m_stop(false), m_isInitialized(false)
    {
    m_maxWorkers = std::max(1u, std::min(m_mgr->GetConfig().GetConcurrent(), std::thread::hardware_concurrency()));
    const auto isTraceEnabled = CQLOG.isSeverityEnabled(NativeLogging::LOG_TRACE);
    if (isTraceEnabled)
        CQLOG.tracev("Starting WorkerPool %" PRIu32, m_maxWorkers);

    for (unsigned int i = 0; i < m_maxWorkers; i++) 
        {
        m_workers.push_back(std::unique_ptr<QueryWorker>(new QueryWorker(*this)));
        if (!m_workers.back()->IsDbOpen()) 
            {
            const auto isErrorEnabled = CQLOG.isSeverityEnabled(NativeLogging::LOG_ERROR);
            if (isErrorEnabled)
                CQLOG.errorv("Failed to start query worker [id=%" PRIu32 "]", i);

            Shutdown();
            return;
            }
        }
    m_isInitialized = true;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
void QueryWorkerPool::Shutdown()
    {
    const auto isTraceEnabled = CQLOG.isSeverityEnabled(NativeLogging::LOG_TRACE);
    if (isTraceEnabled)
        CQLOG.trace("Shutting down WorkerPool");

    if (true)
        {
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_queue.empty())
            m_queue.pop();
        m_stop = true;
        m_taskVar.notify_all();
        }

    for (auto& worker : m_workers)
        {
        if (worker->IsDbOpen())
            worker->Join();
        }    
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                             Affan.Khan             05/2019
//---------------------------------------------------------------------------------------
QueryWorkerPool::~QueryWorkerPool()
    {
    Shutdown();
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
ConcurrentQueryManager::PostStatus ConcurrentQueryManager::PostQuery(TaskId& taskId, Utf8CP ecsql, Utf8CP bindings, Limit limit, ConcurrentQueryManager::Quota quota,  ConcurrentQueryManager::Priority priority, ConcurrentQueryManager::RequestContext requestContext)
    {
    return m_pimpl->PostQuery(taskId, ecsql, bindings, limit, quota, priority, requestContext);
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
    const auto isTraceEnabled = CQLOG.isSeverityEnabled(NativeLogging::LOG_TRACE);
    if (isTraceEnabled)
        CQLOG.trace("Monitor started");

    const auto minIntervalTime = mgr->GetConfig().GetMinMonitorInterval();
    for (;;)
        {
        if (mgr->m_stop)
            break;
        
        auto nextTimeoutToWakeUp = mgr->GetConfig().GetMinMonitorInterval();
        //check if any inqueue task has expired
        if (true)
            {
            std::lock_guard<std::mutex> lock(mgr->m_mutex);
            seconds timeElapsed = duration_cast<seconds>(steady_clock::now() - mgr->m_lastTimeActive);
            if (timeElapsed > mgr->m_config.GetIdleCleanupTime())
                {
                if (isTraceEnabled)
                    CQLOG.trace("Idle Cleanup");

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
    if (isTraceEnabled)
        CQLOG.trace("Monitor shutdown");
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

    if (!m_ecdb.IsDbOpen())
        {
        if (CQLOG.isSeverityEnabled(NativeLogging::LOG_ERROR))
            CQLOG.error("Primary connection state must be open");

        return false;
        }
    m_config = config;
    m_workerPool = std::make_unique< QueryWorkerPool>(this);
    m_initalized = m_workerPool->IsInitialized();
    if (!m_initalized) 
        {
        if (CQLOG.isSeverityEnabled(NativeLogging::LOG_ERROR))
            CQLOG.error("Failed to initialize worker pool");

        m_config = Config();
        m_workerPool = nullptr;
        return false;
        }
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
ConcurrentQueryManager::PostStatus ConcurrentQueryManager::Impl::PostQuery(TaskId& taskId, Utf8CP ecsql, Utf8CP bindings, Limit limit, ConcurrentQueryManager::Quota quota, ConcurrentQueryManager::Priority priority, ConcurrentQueryManager::RequestContext requestContext)
    {
    if (!m_initalized)
        return PostStatus::NotInitalized;

    if (CQLOG.isSeverityEnabled(NativeLogging::LOG_ERROR))
        CQLOG.trace("Failed to initialize");

    // remove semicolon in the end so this query can be but as subquery of another
    Utf8String trimedECSql = ecsql;
    if (!trimedECSql.empty())
        {
        for (size_t i = trimedECSql.size() - 1; i > 0; --i)
            {
            if (!isspace(trimedECSql[i]))
                {
                if (trimedECSql[i] != ';')
                    break;
                else
                    trimedECSql[i] = ' ';
                }
            }
        trimedECSql.TrimEnd();
        }

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
    const Utf8String limitQuery = Utf8PrintfString("select * from (%s) limit :" LIMIT_VAR_COUNT " offset :" LIMIT_VAR_OFFSET , trimedECSql.c_str());
    auto itor = m_tasks.insert(std::make_pair(id, std::make_unique<QueryTask>(id, limitQuery.c_str(), bindings, limit, quota, priority, requestContext)));
    auto task = itor.first->second.get();
    if (m_workerPool->Enqueue(task))
        {
        taskId = task->GetId();
        m_lastTimeActive = steady_clock::now();
        return PostStatus::Done;
        }
   
    m_tasks.erase(task->GetId());
    if (CQLOG.isSeverityEnabled(NativeLogging::LOG_ERROR))
        CQLOG.errorv("QueueSizeExceded [current=%" PRIu64 "] [max=%" PRIu64 "]",m_workerPool->GetUnSafeSize(), m_config.GetMaxQueueSize());

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
