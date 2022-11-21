/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <regex>
#include <string>
#include <ECObjects/ECJsonUtilities.h>
#include <Bentley/Logging.h>
#include <GeomSerialization/GeomSerializationApi.h>
#include <GeomSerialization/GeomLibsSerialization.h>
#include <GeomSerialization/GeomLibsJsonSerialization.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
using namespace std::chrono_literals;
using namespace std::chrono;
#define LIMIT_VAR_COUNT "sys_ecdb_count"
#define LIMIT_VAR_OFFSET "sys_ecdb_offset"

static NativeLogging::CategoryLogger s_logger("ECDb.ConcurrentQuery");

#ifndef NDEBUG
    #define log_trace(...) s_logger.tracev(__VA_ARGS__)
    #define log_debug(...) s_logger.debugv(__VA_ARGS__)
#else
    #define log_trace(...)
    #define log_debug(...)
#endif

#define log_info(...)  s_logger.infov(__VA_ARGS__)
#define log_warn(...)  s_logger.warningv(__VA_ARGS__)
#define log_error(...) s_logger.errorv(__VA_ARGS__)

static Utf8String GetTimestamp() {
    return DateTime::GetCurrentTime().ToTimestampString();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int QueryRetryHandler::_OnBusy(int count) const {
    const auto kFailWithBusyError = 0;
    const auto kRetryOperation = 1;
    if (count <= 1) {
        m_start = std::chrono::steady_clock::now();
        return kRetryOperation;
    }
    if (m_maxTime != 0s) {
        const auto timeElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_start);
        if (timeElapsed > m_maxTime) {
            return kFailWithBusyError;
        }
    }
    std::this_thread::sleep_for(m_sleepTime);
    std::this_thread::yield();
    return kRetryOperation;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::shared_ptr<CachedQueryAdaptor> QueryAdaptorCache::TryGet(Utf8CP ecsql, bool usePrimaryConn, bool suppressLogError, ECSqlStatus& status, std::string& ecsql_error) {
    auto const hashCode = ECSqlStatement::GetHashCode(ecsql);
    auto iter = std::find_if(m_cache.begin(), m_cache.end(), [&ecsql,&hashCode,&usePrimaryConn] (std::shared_ptr<CachedQueryAdaptor>& entry) {
        return entry->GetUsePrimaryConn() == usePrimaryConn && entry->GetStatement().GetHashCode() == hashCode && strcmp(entry->GetStatement().GetECSql(), ecsql) == 0;
    });

    if (iter != m_cache.end()) {
        std::shared_ptr<CachedQueryAdaptor> entry = (*iter);
        if (m_cache.front().get() != entry.get()) {
            m_cache.erase(iter);
            m_cache.insert(m_cache.begin(), entry);
        }
        entry->GetStatement().Reset();
        entry->GetStatement().ClearBindings();
        return entry;
    }

    auto newCachedAdaptor = CachedQueryAdaptor::Make();
    newCachedAdaptor->SetWorkerConn(m_conn.GetDb());
    newCachedAdaptor->SetUsePrimaryConn(usePrimaryConn);
    ErrorListenerScope err_scope(const_cast<ECDb&>(m_conn.GetPrimaryDb()));
    if (usePrimaryConn)
        status = newCachedAdaptor->GetStatement().Prepare(m_conn.GetPrimaryDb(), ecsql, !suppressLogError);
    else
        status = newCachedAdaptor->GetStatement().Prepare(m_conn.GetPrimaryDb().Schemas(), m_conn.GetDb(), ecsql, !suppressLogError);

    if (status != ECSqlStatus::Success) {
        ecsql_error = err_scope.GetLastError();
        return nullptr;
    }
    while (m_cache.size() > m_maxEntries)
        m_cache.pop_back();

    m_cache.insert(m_cache.begin(), newCachedAdaptor);
    return newCachedAdaptor;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryJsonAdaptor& CachedQueryAdaptor::GetJsonAdaptor() {
    if (!m_adaptor) {
         m_adaptor = std::unique_ptr<QueryJsonAdaptor>(new QueryJsonAdaptor(*m_stmt.GetECDb()));
    }
    return *m_adaptor.get();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void CachedConnection::Execute(std::function<void(QueryAdaptorCache&,RunnableRequestBase&)> cb, std::unique_ptr<RunnableRequestBase> request) {
    recursive_guard_t lock(m_mutexReq);
    if (!m_isChangeSummaryCacheAttached) {
        BeFileName primaryChangeCacheFile;
        if (GetPrimaryDb().TryGetChangeCacheFileName(primaryChangeCacheFile)) {
            if (!m_db.IsChangeCacheAttached()) {
                if (BE_SQLITE_OK == m_db.AttachChangeCache(primaryChangeCacheFile)) {
                    m_isChangeSummaryCacheAttached = true;
                }
            }
        }
    }

    SetRequest(std::move(request));
    cb(m_adaptorCache, *m_request);
    ClearRequest();
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void CachedConnection::ClearRequest() {
    recursive_guard_t lock(m_mutexReq);
    m_request = nullptr;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void CachedConnection::SetRequest(std::unique_ptr<RunnableRequestBase> request) {
    recursive_guard_t lock(m_mutexReq);
    m_request= std::move(request);
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void CachedConnection::InterruptIf(std::function<bool(RunnableRequestBase const&)> cb, bool cancel) {
    recursive_guard_t lock(m_mutexReq);
    if (m_request != nullptr) {
        if (cancel) {
            m_request->Cancel();
        }
        if (cb(*m_request)) {
            Interrupt();
        }
    }
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
CachedConnection::~CachedConnection() {
    UpdateSqlFunctions(ConnectionAction::Closing);
    m_adaptorCache.Reset();
    m_db.CloseDb();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void CachedConnection::SetAdaptorCacheSize(uint32_t newSize) {
    recursive_guard_t lock(m_mutexReq);
    m_adaptorCache.SetMaxCacheSize(newSize);
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void CachedConnection::Reset(bool detachDbs) {
    recursive_guard_t lock(m_mutexReq);
    m_adaptorCache.Reset();
    m_db.ClearECDbCache();
    if (detachDbs) {
        m_db.DetachChangeCache();
        m_isChangeSummaryCacheAttached = false;
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void CachedConnection::UpdateSqlFunctions(ConnectionAction action) {
    if (action == ConnectionAction::Opening) {
        for (auto& info : GetPrimaryDbSqlFunctions()) {
            DbFunction *tmpFunc;
            if (!m_db.TryGetSqlFunction(tmpFunc, info.GetName().c_str(), info.GetNumArgs())) {
                if (auto rtreeFunc = dynamic_cast<RTreeMatchFunction*>(info.GetFunction()))
                    m_db.AddRTreeMatchFunction(*rtreeFunc);
                else
                    m_db.AddFunction(*info.GetFunction());
            }
        }
    } else { // closing
        for (auto& info : GetPrimaryDbSqlFunctions()) {
            DbFunction *tmpFunc;
            if (m_db.TryGetSqlFunction(tmpFunc, info.GetName().c_str(), info.GetNumArgs())) {
                if (tmpFunc == info.GetFunction() && dynamic_cast<RTreeMatchFunction*>(info.GetFunction()) == nullptr) {
                    m_db.RemoveFunction(*tmpFunc);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::vector<CachedConnection::FunctionInfo> CachedConnection::GetPrimaryDbSqlFunctions() const {
    std::vector<FunctionInfo> funcs;
    std::regex funcFilter("^imodel_\\w*", std::regex_constants::ECMAScript | std::regex_constants::icase);
    for(auto func : m_cache.GetPrimaryDb().GetSqlFunctions()) {
        FunctionInfo info(func->GetName(), func->GetNumArgs(), func);
        if (std::regex_match(info.GetName(), funcFilter)) {
            funcs.push_back(info);
        }
    }
    return funcs;
}


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::shared_ptr<CachedConnection> CachedConnection::Make(ConnectionCache& cache, uint16_t id) {
    auto newConn = std::make_shared<CachedConnection>(cache, id);
    if (id > 0) {
        if (BE_SQLITE_OK != cache.GetPrimaryDb().OpenSecondaryConnection(newConn->m_db,
        ECDb::OpenParams(Db::OpenMode::Readonly, DefaultTxn::No, newConn->m_retryHandler.get()))) {
            return nullptr;
        }
        newConn->UpdateSqlFunctions(ConnectionAction::Opening);
    }
    return newConn;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ConnectionCache::InterruptIf(std::function<bool(RunnableRequestBase const&)> predicate, bool cancel) {
    recursive_guard_t lock(m_mutex);
    for (auto& conn: m_conns) {
        conn->InterruptIf(predicate, cancel);
    }
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ConnectionCache::SetCacheStatementsPerWork(uint32_t newSize) {
    recursive_guard_t lock(m_mutex);
    for (auto& conn: m_conns) {
        conn->SetAdaptorCacheSize(newSize);
    }
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ConnectionCache::ConnectionCache(ECDb const& primaryDb, uint32_t pool_size): m_primaryDb(primaryDb), m_poolSize(pool_size) {
    if (!primaryDb.IsDbOpen())
        throw std::runtime_error("primary db connection must be open");

    if (m_poolSize < 1) {
        m_poolSize = ConcurrentQueryMgr::GetConfig(primaryDb).GetWorkerThreadCount();
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::shared_ptr<CachedConnection> ConnectionCache::GetConnection() {
    if (!m_primaryDb.IsDbOpen())
        throw std::runtime_error("primary db connection must be open");

    recursive_guard_t lock(m_mutex);
    for (auto& it : m_conns) {
        if (it.use_count() == 1)  {
            return it;
        }
    }
    if (m_conns.size() < m_poolSize) {
        m_conns.push_back(CachedConnection::Make(*this, (uint16_t)m_conns.size() + 1));
        return m_conns.back();
    }

    return nullptr;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
CachedConnection& ConnectionCache::GetSyncConnection() {
    if (m_syncConn == nullptr) {
        m_syncConn = CachedConnection::Make(*this, 0);
    }
    return *m_syncConn;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ConnectionCache::Interrupt(bool reset_conn, bool detach_dbs) {
    recursive_guard_t lock(m_mutex);
    uint32_t used_count = 0;
    while (used_count != m_conns.size()) {
        used_count = 0;
        for (auto& it : m_conns) {
            if (it.use_count() == 1)  {
                ++used_count;
            } else {
                it->Interrupt();
            }
        }
        std::this_thread::yield();
    }
    if (reset_conn) {
        for (auto& it : m_conns) {
            it->Reset(detach_dbs);
        }
    }
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ErrorListenerScope::_OnIssueReported(ECN::IssueSeverity severity, ECN::IssueCategory category, ECN::IssueType type, Utf8CP message) const{
    if (severity == ECN::IssueSeverity::Error)
        m_lastError = message;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ErrorListenerScope::ErrorListenerScope (ECDb& ecdb): m_ecdb(ecdb), m_lock(ecdb.GetImpl().GetMutex()) {
    m_ecdb.AddIssueListener(*this);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ErrorListenerScope::~ErrorListenerScope() {
     m_ecdb.RemoveIssueListener();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECDb const& CachedConnection::GetPrimaryDb() const { return m_cache.GetPrimaryDb(); }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void QueryLimit::ToJs(BeJsValue& v) const{
    v.toObject();
    v[kCount] = m_count;
    v[kOffset] = m_offset;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryLimit QueryLimit::FromJs(BeJsConst const& v) noexcept {
    QueryLimit limit;
    if (v.hasMember(kCount))
       limit. m_count = v[kCount].asInt64();

    if (v.hasMember(kOffset))
        limit.m_offset = v[kOffset].asInt64();

    return limit;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void QueryQuota::ToJs(BeJsValue& v) const{
    v.toObject();;
    v[kTime] = (int64_t)m_timeLimit.count();
    v[kMemory] = m_memoryLimit;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryQuota QueryQuota::FromJs(BeJsConst const& v) noexcept{
    QueryQuota quota(DEFAULT_QUOTA_MAX_TIME, DEFAULT_QUOTA_MAX_MEM);
    if (v.hasMember(kTime))
        quota.m_timeLimit = std::chrono::seconds(v[kTime].asUInt64());

    if (v.hasMember(kMemory))
        quota.m_memoryLimit = (uint32_t)v[kMemory].asUInt();
    return quota;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Ptr RunnableRequestBase::CreateQueueFullResponse() {
    return std::make_shared<QueryResponse>(
        QueryResponse::Kind::NoResult,
        QueryResponse::Stats(),
        QueryResponse::Status::QueueFull,
        ""
        );
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Ptr RunnableRequestBase::CreateErrorResponse(QueryResponse::Status status,std::string error) const {
    BeAssert((int)status >= (int)QueryResponse::Status::Error);
    if ((int)status < (int)QueryResponse::Status::Error) {
        return nullptr;
    }
    return std::make_shared<QueryResponse>(
        ECSqlResponse::Kind::NoResult,
        QueryResponse::Stats(GetCpuTime(), GetTotalTime(), 0, m_quota),
        status,
        error
        );
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Ptr RunnableRequestBase::CreateTimeoutResponse() const {
    return std::make_shared<QueryResponse>(
        ECSqlResponse::Kind::NoResult,
        QueryResponse::Stats(GetCpuTime(), GetTotalTime(), 0, m_quota),
        QueryResponse::Status::Timeout,
        ""
        );
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Ptr RunnableRequestBase::CreateCancelResponse() const {
    return std::make_shared<QueryResponse>(
        ECSqlResponse::Kind::NoResult,
        QueryResponse::Stats( GetCpuTime(), GetTotalTime(), 0, m_quota),
        QueryResponse::Status::Cancel,
        ""
        );
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Ptr RunnableRequestBase::CreateBlobIOResponse(std::vector<uint8_t>& buffer, bool done, uint32_t blobSize) const {
    const auto memUsed = (uint32_t)buffer.size();
    return std::make_shared<BlobIOResponse>(
        QueryResponse::Stats(GetCpuTime(), GetTotalTime(), memUsed, m_quota),
        done? QueryResponse::Status::Done:QueryResponse::Status::Partial,
        "",
        buffer,
        blobSize);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Ptr RunnableRequestBase::CreateECSqlResponse(std::string& resultJson, QueryProperty::List& meta, uint32_t rowCount, bool done) const {
    const auto memUsed = (uint32_t)(resultJson.size());
    return std::make_shared<ECSqlResponse>(
        QueryResponse::Stats(GetCpuTime(), GetTotalTime(), memUsed,m_quota),
        done? QueryResponse::Status::Done:QueryResponse::Status::Partial,
        "",
        resultJson,
        meta,
        rowCount);
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
RunnableRequestQueue::RunnableRequestQueue(ECDbCR ecdb): m_nextId(0), m_state(State::Running), m_lastDelayedQueryId(0),m_ecdb(ecdb) {
    auto env = ConcurrentQueryMgr::GetConfig(ecdb);
    m_quota = env.GetQuota();
    m_maxQueueSize = env.GetRequestQueueSize();
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void RunnableRequestQueue::SetMaxQuota(QueryQuota const& quota) {
    guard_t lock(m_mutex);
    m_quota = quota;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void RunnableRequestQueue::SetRequestQueueMaxSize(uint32_t size) {
    guard_t lock(m_mutex);
    if (size < kMinQueueSize )
        size = kMinQueueSize;

    m_maxQueueSize = size;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
uint32_t RunnableRequestQueue::GetNextId () {
    guard_t lock(m_mutex);
    constexpr auto kMax = std::numeric_limits<uint32_t>::max() - 0xff;
    constexpr auto kMin = std::numeric_limits<uint32_t>::min();
    if (m_nextId  >  kMax)
        m_nextId = kMin;
    else
        ++m_nextId;
    return m_nextId;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void RunnableRequestQueue::InsertSorted(ConnectionCache& conns, std::unique_ptr<RunnableRequestBase>&& request) {
    guard_t lock(m_mutex);
    log_trace("%s enqueuing request [id=%" PRIu32 "]", GetTimestamp().c_str(), request->GetId());
    auto& restartToken = request->GetRequest().GetRestartToken();
    if (!restartToken.empty()) {
        log_trace("%s request [id=%" PRIu32 "] has restart token '%s', attempting to cancel any existing request in queue.",GetTimestamp().c_str(), request->GetId(), restartToken.c_str());
        for (auto it = m_requests.begin(); it != m_requests.end();) {
            auto& existingRestartToken = (*it)->GetRequest().GetRestartToken();
            if (restartToken == existingRestartToken) {
                 log_trace("%s found request [id=%" PRIu32 "] with restart token '%s' and will be cancelled in response to request [id=%" PRIu32 "]",
                    GetTimestamp().c_str(),
                    (*it)->GetId(),
                    restartToken.c_str(),
                    request->GetId());
                (*it)->SetResponse((*it)->CreateCancelResponse());
                it = m_requests.erase(it);
            } else {
                ++it;
            }
        }
        log_trace("%s request [id=%" PRIu32 "] has restart token '%s', attempting to interrupt any running query.",GetTimestamp().c_str(), request->GetId(), restartToken.c_str());
        conns.InterruptIf([&](RunnableRequestBase const& rrb){
            if (rrb.GetRequest().GetRestartToken() == restartToken) {
                log_trace("%s found running request [id=%" PRIu32 "] with restart token '%s' and will be cancelled in response to request [id=%" PRIu32 "]",
                    GetTimestamp().c_str(),
                    rrb.GetId(),
                    restartToken.c_str(),
                    request->GetId());
                return true;
            }
            return false;
        }, true);
    }
    const auto priority = request->GetRequest().GetPriority();
    for (auto it = m_requests.begin(); it != m_requests.end(); ++ it) {
        auto cur_priority = it->get()->GetRequest().GetPriority();
        if (cur_priority > priority) {
             m_requests.insert( it, std::move(request));
             return;
        }
    }
    log_trace("%s enqueuing request [id=%" PRIu32 "] complete", GetTimestamp().c_str(), request->GetId());
    m_requests.insert( m_requests.end(), std::move(request));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::unique_ptr<RunnableRequestBase> RunnableRequestQueue::Dequeue() {
    if (m_requests.empty())
        return nullptr;

    auto req = std::move(m_requests.back());
    m_requests.pop_back();
    if (req->IsReady()) {
        log_trace("%s dequeued request [id=%" PRIu32 "]", GetTimestamp().c_str(), req->GetId());
        req->OnDequeued();
        return req;
    }
    if (m_lastDelayedQueryId != req->GetId()) {
        m_lastDelayedQueryId = req->GetId();
        log_trace("%s dequeued request [id=%" PRIu32 "] has delay and will be deferred and put back in queue.",GetTimestamp().c_str(), req->GetId());
    }
    if (m_requests.size() > 1)
        m_requests.insert(m_requests.end() - 1, std::move(req));
    else
        m_requests.push_back(std::move(req));

    std::this_thread::yield();
    return nullptr;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::unique_ptr<RunnableRequestBase> RunnableRequestQueue::WaitForDequeue() {
    unique_lock_t lock(m_mutex);
    m_cond.wait(lock, [&](){
        return !m_requests.empty() || m_state.load() != State::Running;
    });
    if (m_state.load() == State::Running)
        return Dequeue();

    return nullptr;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryQuota RunnableRequestQueue::AdjustQuota(QueryQuota const& requestedQuota) const {
    auto maxMem = requestedQuota.MaxMemoryAllowed();
    auto maxTime = requestedQuota.MaxTimeAllowed();
    if (m_quota.MaxMemoryAllowed() < maxMem || maxMem <=0)
        maxMem = m_quota.MaxMemoryAllowed() ;
    if (m_quota.MaxTimeAllowed() < maxTime || maxTime <= std::chrono::seconds(0))
        maxTime = m_quota.MaxTimeAllowed();

    return QueryQuota(maxTime, maxMem);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void RunnableRequestQueue::ExecuteSynchronously(ConnectionCache& conns, std::unique_ptr<RunnableRequestBase> runnableReq) {
    // sync connection must be called on main thread.
    log_trace("%s executing query synchronously [id=%" PRIu32 "] started.", GetTimestamp().c_str(), runnableReq->GetId());
    runnableReq->OnDequeued();
    conns.GetSyncConnection().Execute([](QueryAdaptorCache& adaptorCache, RunnableRequestBase& runnableQuery) {
        QueryHelper::Execute(adaptorCache, runnableQuery);
        }, std::move(runnableReq));
    log_trace("%s executing query synchronously [id=%" PRIu32 "] ended.",GetTimestamp().c_str(), runnableReq->GetId());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Future RunnableRequestQueue::Enqueue(ConnectionCache& conns, QueryRequest::Ptr request) {
    // Put a upper limit on query delay to make it safe.
    if (ConcurrentQueryMgr::GetConfig(m_ecdb).GetIgnoreDelay()) {
        request->SetDelay(0ms);
    } else {
        const auto maxDelayAllowed = (std::chrono::milliseconds)DEFAULT_QUERY_DELAY_MAX_TIME;
        if (request->GetDelay() >  maxDelayAllowed) {
            request->SetDelay(maxDelayAllowed);
        }
    }
    auto adjustedQuota = AdjustQuota(request->GetQuota());
    auto runnableReq = std::unique_ptr<RunnableRequestBase>(new RunnableRequestWithPromise(*this, std::move(request), adjustedQuota, GetNextId()));
    auto future = ((RunnableRequestWithPromise*)runnableReq.get())->GetFuture();
    if (m_requests.size() >= m_maxQueueSize) {
        log_warn("%s queue is full, rejecting request [id=%" PRIu32 "]", GetTimestamp().c_str(), runnableReq->GetId());
        runnableReq->SetResponse(RunnableRequestBase::CreateQueueFullResponse());
    } else  {
        if (m_state.load()==State::Stop) {
            log_error("%s concurrent query shuting down, rejecting request [id=%" PRIu32 "]", GetTimestamp().c_str(), runnableReq->GetId());
            runnableReq->SetResponse(runnableReq->CreateErrorResponse(QueryResponse::Status::Error, "concurrent query is shutting down"));
        } else {
            if (runnableReq->GetRequest().UsePrimaryConnection()) {
                ExecuteSynchronously(conns, std::move(runnableReq));
            } else {
                InsertSorted(conns, std::move(runnableReq));
                m_cond.notify_one();
            }
        }
    }
    return future;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void RunnableRequestQueue::Enqueue(ConnectionCache& conns, QueryRequest::Ptr request, ConcurrentQueryMgr::OnCompletion onComplete) {
    // Put a upper limit on query delay to make it safe.
    if (ConcurrentQueryMgr::GetConfig(m_ecdb).GetIgnoreDelay()) {
        request->SetDelay(0ms);
    } else {
        const auto maxDelayAllowed = (std::chrono::milliseconds)DEFAULT_QUERY_DELAY_MAX_TIME;
        if (request->GetDelay() >  maxDelayAllowed) {
            request->SetDelay(maxDelayAllowed);
        }
    }
    auto adjustedQuota = AdjustQuota(request->GetQuota());
    auto runnableReq = std::unique_ptr<RunnableRequestBase>(new RunnableRequestWithCallback(*this, std::move(request), adjustedQuota, GetNextId(), onComplete));
    if (m_requests.size() >= m_maxQueueSize) {
        log_warn("%s queue is full, rejecting request [id=%" PRIu32 "]", GetTimestamp().c_str(), runnableReq->GetId());
        runnableReq->SetResponse(RunnableRequestBase::CreateQueueFullResponse());
    } else  {
        if (m_state.load()==State::Stop) {
            log_error("%s concurrent query shuting down, rejecting request [id=%" PRIu32 "]", GetTimestamp().c_str(), runnableReq->GetId());
            runnableReq->SetResponse(runnableReq->CreateErrorResponse(QueryResponse::Status::Error,"concurrent query is shutting down"));
        } else {
            if (runnableReq->GetRequest().UsePrimaryConnection()) {
                ExecuteSynchronously(conns, std::move(runnableReq));
            } else {
                InsertSorted(conns, std::move(runnableReq));
                m_cond.notify_one();
            }
        }
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
uint32_t RunnableRequestQueue::Count() {
    guard_t lock(m_mutex);
    return (uint32_t)m_requests.size();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool RunnableRequestQueue::Suspend() {
    if (m_state.load() == State::Paused || m_state.load() == State::Stop)
        return false;

    log_trace("%s suspending request queue.", GetTimestamp().c_str());
    m_state.store(State::Paused);

    guard_t lock(m_mutex);
    m_cond.notify_all();
    log_trace("%s request queue suspended.", GetTimestamp().c_str());
    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool RunnableRequestQueue::Stop() {
    if (m_state.load() == State::Stop)
        return false;

    log_trace("%s stopping request queue.", GetTimestamp().c_str());
    m_state.store(State::Stop);

    guard_t lock(m_mutex);
    for(auto & request : m_requests) {
        request->SetResponse(request->CreateErrorResponse(QueryResponse::Status::Error,"concurrent query is shutting down"));
    }
    m_cond.notify_all();
    log_trace("%s request queue stopped.", GetTimestamp().c_str());
    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void RunnableRequestQueue::RemoveIf (std::function<bool(RunnableRequestBase&)> predicate) {
    guard_t lock(m_mutex);
    auto it = m_requests.begin();
    while(it != m_requests.end()) {
        if (predicate(*(*it))) {
            it = m_requests.erase(it);
        } else {
            ++it;
        }
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool RunnableRequestQueue::Resume() {
    if (m_state.load() == State::Running || m_state.load() == State::Stop)
        return false;

    log_trace("%s resuming request queue", GetTimestamp().c_str());
    m_state.store(State::Running);
    m_cond.notify_all();
    log_trace("%s request queue resumed.", GetTimestamp().c_str());
    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool RunnableRequestQueue::CancelRequest(uint32_t id) {
    guard_t lock(m_mutex);
    log_trace("%s request to cancel [id=%" PRIu32 "]", GetTimestamp().c_str(), id);
    auto it = std::find_if(std::begin(m_requests), std::end(m_requests), [id](std::unique_ptr<RunnableRequestBase>& v){
        return v->GetId() == id;
    });
    if (it != std::end(m_requests)) {
        log_trace("%s request [id=%" PRIu32 "] cancelled", GetTimestamp().c_str(), id);
        (*it)->SetResponse((*it)->CreateCancelResponse());
        m_requests.erase(it);
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void RunnableRequestBase::SetResponse(QueryResponse::Ptr response) {
    if (m_isCompleted)
        throw std::runtime_error("already responded");
    try { _SetResponse(response); } catch(std::exception) {}
    m_isCompleted = true;
}


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Future RunnableRequestWithPromise::GetFuture() {
   return QueryResponse::Future( new QueryResponse::Future::Impl([&](){
       GetQueue().CancelRequest(GetId());
   }, m_promise.get_future()));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Future::Future(Future&& rhs){
    m_impl = std::move(rhs.m_impl);
    rhs.m_impl= nullptr;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Future::~Future(){
    if (m_impl != nullptr)
        delete m_impl;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void QueryResponse::Future::Cancel(){
    if (m_impl == nullptr)
        throw std::runtime_error("query response future impl is null");

    return m_impl->Cancel();
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Ptr QueryResponse::Future::Get(){
    if (m_impl == nullptr)
        throw std::runtime_error("query response future impl is null");

    return m_impl->GetFuture().get();
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void QueryResponse::Future::Wait(){
    if (m_impl == nullptr)
        throw std::runtime_error("query response future impl is null");

    return m_impl->GetFuture().wait();;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool QueryResponse::Future::Valid(){
    if (m_impl == nullptr)
        throw std::runtime_error("query response future impl is null");

    return m_impl->GetFuture().valid();;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Future& QueryResponse::Future::operator =(Future&& rhs) {
    if(this != &rhs){
        m_impl = std::move(rhs.m_impl);
        rhs.m_impl = nullptr;
    }
    return *this;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::string QueryHelper::FormatQuery(const char* query) {
    Utf8String trimmedECSql = query;
    trimmedECSql.Trim();
    Utf8Char c;
    while (!trimmedECSql.empty() && (c = trimmedECSql[trimmedECSql.size() - 1])  && (c == ';' || isspace(c)))
        trimmedECSql.erase(trimmedECSql.size() - 1);
    if (trimmedECSql.StartsWithIAscii("with")) {
        std::regex rx("\\)\\s*select", std::regex_constants::ECMAScript | std::regex_constants::icase);
        std::match_results<Utf8String::const_iterator> matches;
        if (std::regex_search<Utf8String::const_iterator>(trimmedECSql.begin(), trimmedECSql.end(), matches, rx)) {
            auto n = matches.position();
            Utf8String prefix = trimmedECSql.substr(0, n + 1);
            Utf8String select = trimmedECSql.substr(n + 2);
            return Utf8PrintfString("%s select * from (%s) limit :" LIMIT_VAR_COUNT " offset :" LIMIT_VAR_OFFSET, prefix.c_str(), select.c_str());
        }
    }
    if (trimmedECSql.StartsWithIAscii("pragma")) {
        return trimmedECSql;
    }
    return Utf8PrintfString("select * from (%s) limit :" LIMIT_VAR_COUNT " offset :" LIMIT_VAR_OFFSET, trimmedECSql.c_str());
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void QueryHelper::BindLimits(ECSqlStatement& stmt, QueryLimit const& limit) {
    const auto idxCount = stmt.GetParameterIndex(LIMIT_VAR_COUNT);
    const auto idxOffset = stmt.GetParameterIndex(LIMIT_VAR_OFFSET);
    // bind limit
    stmt.BindInt64(idxCount, limit.GetCount());
    stmt.BindInt64(idxOffset, limit.GetOffset());
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryProperty const& QueryProperty::List::GetPropertyInfo(std::string const& name) const {
    static QueryProperty kNull;
    for(auto& info: *this) {
        if (info.GetJsonName() == name)
            return this->at(info.GetIndex());
    }
    return kNull;
}


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::string IJsSerializable::Stringify(StringifyFormat format) const {
    BeJsDocument doc;
    ToJs(doc);
    return doc.Stringify(format);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryProperty::List QueryHelper::GetMetaInfo(CachedQueryAdaptor& adp, bool classIdToClassNames) {
    QueryProperty::List props;

    return props;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void QueryProperty::ToJs(BeJsValue& val) const {
    val.toObject();
    val[JClass]=m_className;
    val[JGenerated]=m_isGenerated;
    val[JIndex]=m_index;
    val[JJsonName]=m_jsonName;
    val[JName]=m_name;
    val[JExtendedType]=m_extendedType;
    val[JType]=m_typeName;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void QueryProperty::List::ToJs(BeJsValue& val) const {
    val.toArray();
    for(auto it = begin(); it != end(); ++it) {
        auto newEl = val.appendValue();
        (*it).ToJs(newEl);
    }
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void QueryProperty::List::append(std::string className, std::string jsonName, std::string name, std::string typeName, bool generated, std::string extendedType, int index) {
    emplace_back(QueryProperty(className, jsonName,name, typeName, generated, extendedType, index));
}

//---------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------------------------------------------------------------------------------
Utf8CP QueryResponse::StatusToString(QueryResponse::Status status) {
    switch(status) {
        case QueryResponse::Status::Cancel:
            return "Cancel";
        case QueryResponse::Status::Done:
            return "Done";
        case QueryResponse::Status::Error:
            return "Error";
        case QueryResponse::Status::Error_BlobIO_OpenFailed:
            return "Error_BlobIO_OpenFailed";
        case QueryResponse::Status::Error_BlobIO_OutOfRange:
            return "Error_BlobIO_OutOfRange";
        case QueryResponse::Status::Error_ECSql_BindingFailed:
            return "Error_ECSql_BindingFailed";
        case QueryResponse::Status::Error_ECSql_PreparedFailed:
            return "Error_ECSql_PreparedFailed";
        case QueryResponse::Status::Error_ECSql_RowToJsonFailed:
            return "Error_ECSql_RowToJsonFailed";
        case QueryResponse::Status::Error_ECSql_StepFailed:
            return "Error_ECSql_StepFailed";
        case QueryResponse::Status::Partial:
            return "Partial";
        case QueryResponse::Status::QueueFull:
            return "QueueFull";
        case QueryResponse::Status::Timeout:
            return "Timeout";
    };
    return "Unknow QueryResponse::Status code";
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void QueryHelper::Execute(CachedQueryAdaptor& cachedAdaptor, RunnableRequestBase& runnableRequest) {
    enum class status { partial, done };
    auto& request= runnableRequest.GetRequest().GetAsConst<ECSqlRequest>();
    const auto abbreviateBlobs = request.GetAbbreviateBlobs();
    const auto includeMetaData= request.GetIncludeMetaData();
    const auto classIdToClassNames = request.GetConvertClassIdsToClassNames();
    auto& stmt = cachedAdaptor.GetStatement();
    auto& adaptor = cachedAdaptor.GetJsonAdaptor();
    QueryProperty::List props;
    if (includeMetaData) {
        adaptor.GetMetaData(props ,stmt);
    }
    adaptor.SetAbbreviateBlobs(abbreviateBlobs);
    adaptor.SetConvertClassIdsToClassNames(classIdToClassNames);
    adaptor.UseJsNames(request.GetValueFormat() == ECSqlRequest::ECSqlValueFormat::JsNames);
    uint32_t row_count = 0;
    std::string& result = cachedAdaptor.ClearAndGetCachedString();
    result.reserve(QUERY_WORKER_RESULT_RESERVE_BYTES);
    result.append("[");
    auto setResult = [&](status st) {
        result.append("]");
        if (runnableRequest.IsCancelled())
            runnableRequest.SetResponse(runnableRequest.CreateCancelResponse());
        else
            runnableRequest.SetResponse(runnableRequest.CreateECSqlResponse(result, props, row_count, st == status::done));
    };
    auto setError = [&] (QueryResponse::Status status, std::string err) {
        runnableRequest.SetResponse(runnableRequest.CreateErrorResponse(status, err));
        log_error("%s. (%s)", err.c_str(), QueryResponse::StatusToString(status));
    };

    // go over each row and serialize result
    auto rc = stmt.Step();
    while (rc == BE_SQLITE_ROW) {
        auto& rowsDoc = cachedAdaptor.ClearAndGetCachedXmlDocument();
        BeJsValue rows(rowsDoc);
        if (adaptor.RenderRow(rows, stmt) != SUCCESS) {
            setError(QueryResponse::Status::Error_ECSql_RowToJsonFailed, "failed to serialize ecsql statement row to json");
            return;
        } else {
            row_count = row_count + 1;
            if (row_count == 1) {
                result.append(rows.Stringify());
            } else {
                result.append(",").append(rows.Stringify());
            }
        }
        if (runnableRequest.IsTimeOrMemoryExceeded(result)) {
            log_trace("%s time or memory exceeded for request [id=%" PRIu32 "]",GetTimestamp().c_str(), runnableRequest.GetId());
            setResult(status::partial);
            return;
        }
        rc = stmt.Step();
    }

    if (rc == BE_SQLITE_INTERRUPT || rc == BE_SQLITE_BUSY) {
        setResult(status::partial);
    } else if (rc != BE_SQLITE_DONE) {
        DbResult lastError;
        std::string sqlStepError = cachedAdaptor.GetWorkerConn()->GetLastError(&lastError);
        if (lastError != BE_SQLITE_OK) {
            setError(QueryResponse::Status::Error_ECSql_StepFailed, SqlPrintfString("concurrent query step() failed: %s", sqlStepError.c_str()).GetUtf8CP());
        }
        else {
            setError(QueryResponse::Status::Error_ECSql_StepFailed, "concurrent query step() failed");
        }
    } else {
        setResult(status::done);
    }
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void QueryHelper::ReadBlob(ECDbCR conn, RunnableRequestBase& runnableRequest) {
    auto setError = [&] (QueryResponse::Status status, std::string err) {
        runnableRequest.SetResponse(runnableRequest.CreateErrorResponse(status, err));
      log_error("%s. (%s)", err.c_str(), QueryResponse::StatusToString(status));
    };
    auto& request  =runnableRequest.GetRequest().GetAsConst<BlobIORequest>();
    BlobIO blobIo;

    auto classCP = conn.Schemas().FindClass(request.GetClassName().c_str());
    if (classCP == nullptr) {
        setError(QueryResponse::Status::Error_BlobIO_OpenFailed, SqlPrintfString("BlobIO: unable to find classname '%s'", request.GetClassName().c_str()).GetUtf8CP());
        return;
    }
    if (conn.OpenBlobIO(blobIo, *classCP, request.GetAccessString().c_str(), request.GetInstanceId(), false) != SUCCESS) {
        setError(QueryResponse::Status::Error_BlobIO_OpenFailed, SqlPrintfString("BlobIO: unable to open blob for classname '%s' , accessString '%s' for instanceId '%s'",
            request.GetClassName().c_str(), request.GetAccessString().c_str(), request.GetInstanceId().ToHexStr().c_str()).GetUtf8CP());
        return;
    }
    const auto blobLength = (uint32_t)blobIo.GetNumBytes();
    std::vector<uint8_t> buffer;
    if (blobLength == 0) {
        runnableRequest.SetResponse(runnableRequest.CreateBlobIOResponse(buffer, true, blobLength));
        return;
    }

    const auto offset = (uint32_t)(request.GetRange().GetOffset() < 0 ? 0 : request.GetRange().GetOffset());
    auto length = (uint32_t)(request.GetRange().GetCount() <= 0 ? blobLength : request.GetRange().GetCount());
    if (offset > blobLength) {
        setError(QueryResponse::Status::Error_BlobIO_OutOfRange, "BlobIO: offset provided is greater then size of blob");
        return;
    }
    if ((offset + length) > blobLength) {
        setError(QueryResponse::Status::Error_BlobIO_OutOfRange,"BlobIO: offset + length provided is greater then size of blob");
        return;
    }
    const auto maxSize = request.GetQuota().MaxMemoryAllowed();
    auto numBytes = maxSize > 0 ? std::min(length, maxSize): length;
    buffer.resize(numBytes);
    auto rc = blobIo.Read(&buffer[0], (int)numBytes, offset);
    if (rc == BE_SQLITE_OK) {
        runnableRequest.SetResponse(runnableRequest.CreateBlobIOResponse(buffer, true, blobLength));
        return;
    }
    setError(QueryResponse::Status::Error, "BlobIO: unable to read blob due to sqlite error");
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void QueryHelper::ExecutePing(Json::Value const& pingJson, RunnableRequestBase& runnableRequest) {
    QueryProperty::List props;
    props.append("", "id", "id", "long", false, "", 0);

    const auto maxMem = (int64_t)ConcurrentQueryMgr::GetConfig(runnableRequest.GetQueue().GetECDb()).GetQuota().MaxMemoryAllowed();
    auto pingResultSize = pingJson["ping"]["resultSize"].asInt64();
    const auto pingSleepTime = std::chrono::milliseconds(pingJson["ping"]["sleepTime"].asUInt());
    const auto sleepUntil = std::chrono::steady_clock::now() + pingSleepTime;

    pingResultSize = pingResultSize < 0 ? 0 : pingResultSize;
    pingResultSize = pingResultSize > maxMem ? maxMem : pingResultSize;
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int64_t> dist(std::numeric_limits<uint64_t>::min(), std::numeric_limits<uint64_t>::max());
    std::string result = "[";
    result.reserve(pingResultSize);
    uint32_t rows = 0;
    while (result.size() < (size_t)(pingResultSize)) {
        if (rows > 0) {
            result.append(",");
        }
        result.append("\"").append(BeInt64Id(dist(mt)).ToHexStr()).append("\"");
        ++rows;
    }
    result.append("]");
    if (pingSleepTime.count() > 0 && sleepUntil > std::chrono::steady_clock::now()) {
        std::this_thread::sleep_until(sleepUntil);
    }
    runnableRequest.SetResponse(runnableRequest.CreateECSqlResponse(result, props, rows, true));
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void QueryHelper::Execute(QueryAdaptorCache& adaptorCache, RunnableRequestBase& runnableRequest) {
    auto setError = [&] (QueryResponse::Status status, std::string err) {
        runnableRequest.SetResponse(runnableRequest.CreateErrorResponse(status, err));
    };
    if (runnableRequest.IsTimeExceeded()) {
        runnableRequest.SetResponse(runnableRequest.CreateTimeoutResponse());
        return;
    }
    if (runnableRequest.GetRequest().GetKind() == QueryRequest::Kind::BlobIO) {
        auto& request = runnableRequest.GetRequest().GetAsConst<BlobIORequest>();
        auto& conn = request.UsePrimaryConnection() ? adaptorCache.GetConnection().GetPrimaryDb() : adaptorCache.GetConnection().GetDb();
        QueryHelper::ReadBlob(conn, runnableRequest);
    } else if (runnableRequest.GetRequest().GetKind() == QueryRequest::Kind::ECSql) {
        auto& request = runnableRequest.GetRequest().GetAsConst<ECSqlRequest>();
        // use for debugging performance issues.
        if (request.GetQuery().size()> 1 && request.GetQuery().at(0) == '{') {
            auto pingJson = Json::Value::From(request.GetQuery());
            if (!pingJson.isNull() && pingJson.hasMember("ping")) {
                QueryHelper::ExecutePing(pingJson, runnableRequest);
                return;
            }
        }
        std::string sql = QueryHelper::FormatQuery(request.GetQuery().c_str());
        ECSqlStatus status;
        std::string err;
        auto adaptor = adaptorCache.TryGet(sql.c_str(), request.UsePrimaryConnection(), request.GetSuppressLogErrors(), status, err);
        if (adaptor == nullptr) {
            if (status.IsSQLiteError()) {
                if (status.GetSQLiteError() == BE_SQLITE_INTERRUPT) {
                    if (runnableRequest.IsCancelled()) {
                        runnableRequest.SetResponse(runnableRequest.CreateCancelResponse());
                        return;
                    } else if (runnableRequest.IsTimeExceeded()){
                        runnableRequest.SetResponse(runnableRequest.CreateTimeoutResponse());
                        return;
                    }
                }
            }
            setError(QueryResponse::Status::Error_ECSql_PreparedFailed, err);
            return;
        }
        if (!request.GetArgs().TryBindTo(adaptor->GetStatement(), err)) {
            setError(QueryResponse::Status::Error_ECSql_BindingFailed, err);
            return;
        }
        BindLimits(adaptor->GetStatement(), request.GetLimit());
        QueryHelper::Execute(*adaptor, runnableRequest);
    } else {
        setError(QueryResponse::Status::Error, "unsupported kind of request");
    }
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void QueryExecutor::SetWorkerPoolSize(uint32_t newSize) {
    m_maxPoolSize = newSize;
    m_connCache.SetMaxPoolSize(newSize);
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryExecutor::QueryExecutor(RunnableRequestQueue& queue, ECDbCR primaryDb, uint32_t pool_size) :m_queue(queue), m_connCache(primaryDb, pool_size),m_maxPoolSize(pool_size),m_threadCount(0) {
    if (pool_size < 1) {
        pool_size = ConcurrentQueryMgr::GetConfig(primaryDb).GetWorkerThreadCount();
    }
    for (uint32_t i = 0; i < pool_size; ++i) {
        m_threads.emplace_back(std::thread([&](){
            thread_local const auto execId = m_threadCount.fetch_add(1);
            log_trace("%s executor started [id=%" PRIu32 "]",GetTimestamp().c_str(), execId);
            do {
                auto runnableQuery = m_queue.WaitForDequeue();
                if (runnableQuery != nullptr) {
                    log_trace("%s executor [id=%" PRIu32 "] dequeued request [id=%" PRIu32 "]", GetTimestamp().c_str(), execId, runnableQuery->GetId());
                    std::shared_ptr<CachedConnection> conn;
                    conn = m_connCache.GetConnection();
                    while (conn == nullptr) {
                        std::this_thread::yield();
                        std::this_thread::sleep_for(1s);
                        conn = m_connCache.GetConnection();
                    }
                    runnableQuery->SetExecutorContext(execId, conn->Id());
                    log_trace("%s executor [id=%" PRIu32 "] with request [id=%" PRIu32 "] is assigned connection [id=%" PRIu32 "]",
                        GetTimestamp().c_str(),
                        runnableQuery->GetExecutorId(),
                        runnableQuery->GetId(),
                        runnableQuery->GetConnectionId());

                    conn->Execute([](QueryAdaptorCache& adaptorCache, RunnableRequestBase& runnableQuery) {
                        log_trace("%s executing [exec_id=%" PRIu32 ", conn_id=%" PRIu32 ", req_id=%" PRIu32 "] started.",
                            GetTimestamp().c_str() ,
                            runnableQuery.GetExecutorId(),
                            runnableQuery.GetConnectionId(),
                            runnableQuery.GetId());

                        if (runnableQuery.GetRequest().UsePrimaryConnection()) {
                            QueryHelper::Execute(adaptorCache, runnableQuery);
                        } else {
                            Savepoint txn(adaptorCache.GetConnection().GetDbR(), "concurrent_query");
                            QueryHelper::Execute(adaptorCache, runnableQuery);
                        }
                        log_trace("%s executing [exec_id=%" PRIu32 ", conn_id=%" PRIu32 ", req_id=%" PRIu32 "] ended.",
                            GetTimestamp().c_str(),
                            runnableQuery.GetExecutorId(),
                            runnableQuery.GetConnectionId(),
                            runnableQuery.GetId());

                    },std::move(runnableQuery));
                }
            } while(m_queue.GetState() != RunnableRequestQueue::State::Stop);
            m_threadCount.fetch_sub(1);
            log_trace("%s executor stopped Id=%" PRIu32, GetTimestamp().c_str(), execId);
        }));
        // wait for atleast one thread to startup.
        while(m_threadCount.load() == 0) {
            std::this_thread::yield();
        }
    }
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryExecutor::~QueryExecutor() {
    for (auto& th : m_threads) {
        if (th.joinable())
            th.join();
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryMonitor::QueryMonitor(RunnableRequestQueue& queue, QueryExecutor& executor, std::chrono::milliseconds pollInterval)
    :m_stop(false), m_queue(queue),m_pollInterval(pollInterval),m_executor(executor) {
    auto notifyThreadHasStarted = std::make_unique<std::promise<void>>();
    log_trace("%s monitor started.", GetTimestamp().c_str());
    m_thread = std::thread([&](std::promise<void>* notifyWhenThreadStarted) {
        if (notifyWhenThreadStarted != nullptr) {
            notifyWhenThreadStarted->set_value();
        }
        do {
            m_queue.RemoveIf([&](RunnableRequestBase& request) {
                if (!request.IsTimeExceeded()) {
                    return false;
                }
                // send respond to client
                request.SetResponse(request.CreateTimeoutResponse());
                log_trace("%s monitor cancel query [id=%" PRIu32 "] with timeout", GetTimestamp().c_str(), request.GetId());
                return true;
                });
            m_executor.GetConnectionCache().InterruptIf([&](RunnableRequestBase const& request) {
                if (request.IsTimeExceeded() ){
                    log_trace("%s monitor cancel query [id=%" PRIu32 "] as it exceeded allowed time", GetTimestamp().c_str(), request.GetId());
                    return true;
                }
                return false;
            }, false);

            std::this_thread::sleep_for(m_pollInterval);
            std::this_thread::yield();
        } while (m_stop.load() == false);
        log_trace("%s monitor stopped.", GetTimestamp().c_str());
    }, notifyThreadHasStarted.get());
    notifyThreadHasStarted->get_future().get();
    notifyThreadHasStarted = nullptr;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ConcurrentQueryMgr::Impl::Suspend(ClearCacheOption clearCache, DetachAttachDbs detachDbs) {
    auto rc = m_queue.Suspend();
    if (rc)
        m_executor.GetConnectionCache().Interrupt(clearCache == ClearCacheOption::Yes, detachDbs == DetachAttachDbs::Yes);
    return rc;
}

ConcurrentQueryMgr::ConcurrentQueryMgr(ECDbCR ecdb){ m_impl = new Impl(ecdb);}
ConcurrentQueryMgr::~ConcurrentQueryMgr(){ delete m_impl;}
QueryResponse::Future ConcurrentQueryMgr::Enqueue(QueryRequest::Ptr request) { return m_impl->Enqueue(std::move(request)); }
void ConcurrentQueryMgr::Enqueue(QueryRequest::Ptr request, OnCompletion onCompletion){ m_impl->Enqueue(std::move(request), onCompletion); }
bool ConcurrentQueryMgr::Suspend(ClearCacheOption clearCache, DetachAttachDbs detachDbs) { return m_impl->Suspend(clearCache,detachDbs); }
bool ConcurrentQueryMgr::Resume() { return m_impl->Resume(); }
bool ConcurrentQueryMgr::IsSuspended() const { return m_impl->IsSuspended(); }
void ConcurrentQueryMgr::SetWorkerPoolSize(uint32_t newSize) {m_impl->SetWorkerPoolSize(newSize);}
void ConcurrentQueryMgr::SetRequestQueueMaxSize(uint32_t newSize) {m_impl->SetRequestQueueMaxSize(newSize);}
void ConcurrentQueryMgr::SetCacheStatementsPerWork(uint32_t newSize) {m_impl->SetCacheStatementsPerWork(newSize);}
void ConcurrentQueryMgr::SetMaxQuota(QueryQuota const& newQuota) {m_impl->SetMaxQuota(newQuota);}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------

ECSqlReader::ECSqlReader(ConcurrentQueryMgr& mgr, std::string ecsql,  ECSqlParams const& args)
    :m_mgr(mgr), m_ecsql(ecsql), m_args(args),m_globalOffset(-1),m_done(false),m_it(0) {
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ECSqlReader::Next() {
    if (m_it == m_rows.size()) {
        if (!m_done && Read() == 0) {
            return false;
        }
        m_it = 0;
    } else {
        ++m_it;
    }
    return m_it < m_rows.size();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
uint32_t ECSqlReader::Read() {
    if (m_done) {
        return 0;
    }
    const auto readMeta  = m_columns.empty();
    auto request = ECSqlRequest::MakeRequest(m_ecsql);
    request->SetArgs(m_args);
    request->SetIncludeMetaData(readMeta);
    request->SetConvertClassIdsToClassNames(true);
    m_globalOffset += m_rows.size();
    request->SetLimit(QueryLimit(-1, m_globalOffset));
    auto future = m_mgr.Enqueue(std::move(request));
    auto response = future.Get();
    if (response->IsError())
        throw std::runtime_error(response->GetError());

    auto& ecsqlResp = response->GetAsRef<ECSqlResponse>();
    m_rows = Json::Value::From(ecsqlResp.asJsonString());
    m_done = ecsqlResp.IsDone();
    if (readMeta) {
        m_columns = std::move(ecsqlResp.GetProperties());
    }
    return ecsqlResp.GetRowCount();
}


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Json::Value const& ECSqlReader::Row::GetValue(int index) const {
    static Json::Value kNull;
    if (index >= (int)m_row.size())
        return kNull;
    return m_row[index];
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Json::Value const& ECSqlReader::Row::GetValue(std::string const& name) const {
    static Json::Value kNull;
    const auto& col  = m_columns[name];
    if (!col.IsValid())
        return kNull;
    return GetValue(col.GetIndex());
}


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Json::Value ECSqlReader::Row::ToJson(Format fmt) const {
    Json::Value v(Json::ValueType::objectValue);
    for (auto& col : m_columns ){
        if (fmt== Format::UseJsonName)
            v[col.GetJsonName()] = GetValue(col.GetIndex());
        else
            v[col.GetName()] = GetValue(col.GetIndex());
    }
    return v;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void QueryResponse::Stats::ToJs(BeJsValue& v) const {
    v.toObject();
    v[kCpuTime] = (int64_t)m_cpuTime.count();
    v[kTotalTime] = (int64_t)m_totalTime.count();
    v[kTimeLimit] = (int64_t)m_timeLimit.count();
    v[kMemLimit] = m_memLimit;
    v[kMemUsed] = m_memUsed;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ConcurrentQueryMgr::Impl::_OnBeforeClearECDbCache() {
    Suspend(ClearCacheOption::Yes, DetachAttachDbs::Yes);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------

void ConcurrentQueryMgr::Impl::_OnAfterClearECDbCache() {
    Resume();
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ConcurrentQueryMgr::Impl::Impl(ECDbCR ecdb): m_executor(m_queue, ecdb),m_monitor(m_queue, m_executor),m_queue(ecdb) {
    auto removeBeforeSchemaChanges = ecdb.Schemas().OnBeforeSchemaChanges().AddListener([&](ECDbCR ecdb, SchemaChangeType type){
        Suspend(ClearCacheOption::Yes, DetachAttachDbs::Yes);
    });
    auto removeAfterSchemaChanges = ecdb.Schemas().OnAfterSchemaChanges().AddListener([&](ECDbCR ecdb, SchemaChangeType type){
        Resume();
    });
    m_removeEventHandlers = [=]() {
        removeBeforeSchemaChanges();
        removeAfterSchemaChanges();
    };
    const_cast<ECDbR>(ecdb).AddECDbCacheClearListener(*this);

}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ConcurrentQueryMgr::Impl::~Impl() {
    m_queue.Stop();
    m_removeEventHandlers();
    const_cast<ECDbR>(m_executor.GetConnectionCache().GetPrimaryDb()).RemoveECDbCacheClearListener(*this);
}

//---------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------------------------------------------------------------------------------
ConcurrentQueryMgr::Config const& ConcurrentQueryMgr::ResetConfig(ECDb const& ecdb, Config const& config) {
    if (!ecdb.IsDbOpen()) {
        throw std::runtime_error("ecdb is closed or not open");
    }

    BeMutexHolder lock (ecdb.GetImpl().GetMutex());
    auto& appKey = ConcurrentQueryConfigAppData::GetKey();
    auto appData = ecdb.FindAppDataOfType<ConcurrentQueryConfigAppData>(appKey);
    if (appData.IsNull()) {
        appData = ConcurrentQueryConfigAppData::Create();
        ecdb.AddAppData(appKey, appData.get());
    }
    appData->SetConfig(config);
    return appData->GetConfig();
}

//---------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------------------------------------------------------------------------------
ConcurrentQueryMgr::Config const& ConcurrentQueryMgr::GetConfig(ECDb const& ecdb) {
    if (!ecdb.IsDbOpen()) {
        throw std::runtime_error("ecdb is closed or not open");
    }

    BeMutexHolder lock (ecdb.GetImpl().GetMutex());
    auto& appKey = ConcurrentQueryConfigAppData::GetKey();
    auto appData = ecdb.FindAppDataOfType<ConcurrentQueryConfigAppData>(appKey);
    if (appData.IsNull()) {
        appData = ConcurrentQueryConfigAppData::Create();
        ecdb.AddAppData(appKey, appData.get());
    }
    return appData->GetConfig();
}
//---------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------------------------------------------------------------------------------
ConcurrentQueryMgr& ConcurrentQueryMgr::GetInstance(ECDbCR ecdb) {
    if (!ecdb.IsDbOpen()) {
        throw std::runtime_error("ecdb is closed or not open");
    }

    BeMutexHolder lock (ecdb.GetImpl().GetMutex());
    auto& appKey = ConcurrentQueryAppData::GetKey();
    auto appData = ecdb.FindAppDataOfType<ConcurrentQueryAppData>(appKey);
    if (appData.IsNull()) {
        appData = ConcurrentQueryAppData::Create(ecdb);
        ecdb.AddAppData(appKey, appData.get());
    }
    return appData->GetConcurrentQuery();
}
//---------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------------------------------------------------------------------------------
void ConcurrentQueryMgr::Shutdown(ECDbCR ecdb) {
    if (!ecdb.IsDbOpen()) {
        throw std::runtime_error("ecdb is closed or not open");
    }

    BeMutexHolder lock (ecdb.GetImpl().GetMutex());
    const auto& appKey = ConcurrentQueryAppData::GetKey();
    if (ecdb.FindAppDataOfType<ConcurrentQueryAppData>(appKey).IsValid()) {
        ecdb.DropAppData(appKey);
    }
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void QueryRequest::FromJs(BeJsConst const& val) {
    if (val.isObjectMember(JQuota)) {
        auto a = val[JQuota];
        m_quota.FromJs(a);
    }
    if (val.isNumericMember(JPriority)) {
        m_priority = val[JPriority].asInt();
    }
    if (val.isBoolMember(JUsePrimaryConn)) {
        m_usePrimaryConn = val[JUsePrimaryConn].asBool();
    }
    if (val.isNumericMember(JKind)) {
        m_kind = (Kind)val[JKind].asInt();
    }
    if (val.isStringMember(JRestartToken)) {
        m_restartToken = val[JRestartToken].asCString();
    }
    if (val.isNumericMember(JDelay)) {
        m_delay = std::chrono::milliseconds(val[JDelay].asInt());
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void BlobIORequest::FromJs(BeJsConst const& val) {
    QueryRequest::FromJs(val);
    if (GetKind() != QueryRequest::Kind::BlobIO) {
        throw std::runtime_error("expecting BlobIORequest");
    }
    if (val.isStringMember(JClassName)) {
        m_className = val[JClassName].asCString();
    }
    if (val.isStringMember(JAccessString)) {
        m_accessString = val[JAccessString].asCString();
    }
    if (val.isStringMember(JInstanceId)) {
        m_ecInstanceId = BeInt64Id::FromString(val[JInstanceId].asCString()).GetValueUnchecked();
    }
    if (val.isObjectMember(JRange)) {
        m_range = QueryLimit::FromJs(val[JRange]);
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ECSqlRequest::FromJs(BeJsConst const& val) {
    QueryRequest::FromJs(val);
    if (GetKind() != QueryRequest::Kind::ECSql) {
        throw std::runtime_error("expecting ECSqlRequest");
    }
    if (val.isStringMember(JQuery)) {
        m_query = val[JQuery].asCString();
    }
    if (val.isObjectMember(JArgs)) {
        Json::Value v;
        BeJsValue val2(v);
        val2.From(val[JArgs]);
        m_args.FromJs(v);
    }
    if (val.isBoolMember(JSuppressLogErrors)) {
        m_suppressLogErrors = val[JSuppressLogErrors].asBool();
    }
    if (val.isBoolMember(JIncludeMetaData)) {
        m_includeMetaData = val[JIncludeMetaData].asBool();
    }
    if (val.isBoolMember(JAbbreviateBlobs)) {
        m_abbreviateBlobs = val[JAbbreviateBlobs].asBool();
    }
    if (val.isBoolMember(JConvertClassIdsToClassNames)) {
        m_convertClassIdsToClassNames = val[JConvertClassIdsToClassNames].asBool();
    }
    if (val.isObjectMember(JLimit)) {
        m_limit = QueryLimit::FromJs(val[JLimit]);
    }
    if (val.isNumericMember(JValueFormat)) {
        m_valueFmt = (ECSqlValueFormat)val[JValueFormat].asInt();
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryRequest::Ptr QueryRequest::Deserialize(BeJsValue const& val) {
    Kind kind;
    if (val.isNumericMember(JKind)) {
        kind = (Kind)val[JKind].asInt();
    } else {
        throw std::runtime_error("concurrent query: unsupported query request type");
    }
    QueryRequest::Ptr ptr;
    if (kind == Kind::BlobIO) {
        ptr = BlobIORequest::MakeRequest("", "", 0);
    } else if (kind == Kind::ECSql) {
        ptr = ECSqlRequest::MakeRequest("");
    } else {
        throw std::runtime_error("concurrent query: unsupported query request type");
    }
    ptr->FromJs(val);
    return ptr;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void QueryResponse::ToJs(BeJsValue& v, bool includeData) const {
    v.toObject();
    v[JKind] = (int)m_kind;
    v[JError] = m_error;
    v[JStatus] = (int)m_status;
    auto stats = v[JStats];
    m_stats.ToJs(stats);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ECSqlResponse::ToJs(BeJsValue& v, bool includeData) const {
    QueryResponse::ToJs(v, includeData);
    v[JRowCount] = m_rowCount;
    if (includeData) {
        v[JData] = m_dataJson;
    }
    auto meta = v[JMeta];
    m_properties.ToJs(meta);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void BlobIOResponse::ToJs(BeJsValue& v, bool includeData) const {
    QueryResponse::ToJs(v, includeData);
    if (includeData) {
        v[JData].SetBinary(GetData(), GetLength());
    }
    v[JBlobSize]= GetRawBlobSize();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BeInt64Id ECSqlParams::ECSqlParam::GetValueId() const {
    BeAssert(m_type == Type::Id);
    return BeInt64Id::FromString(m_val.asCString());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::string ECSqlParams::ECSqlParam::GetValueString() const {
    BeAssert(m_type == Type::String);
    return m_val.asCString();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
double ECSqlParams::ECSqlParam::GetValueDouble() const {
    BeAssert(m_type == Type::Double);
    return m_val.asDouble();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int64_t ECSqlParams::ECSqlParam::GetValueLong() const {
    BeAssert(m_type == Type::Long);
    return m_val.asInt64();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int ECSqlParams::ECSqlParam::GetValueInt() const {
    BeAssert(m_type == Type::Integer);
    return m_val.asInt();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ECSqlParams::ECSqlParam::GetValueBool() const {
    BeAssert(m_type == Type::Boolean);
    return m_val.asBool();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DPoint2d ECSqlParams::ECSqlParam::GetValuePoint2d() const {
    BeAssert(m_type == Type::Point2d);
    return DPoint2d::From(m_val[Jx].asDouble(), m_val[Jy].asDouble());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DPoint3d ECSqlParams::ECSqlParam::GetValuePoint3d() const {
    BeAssert(m_type == Type::Point3d);
    return DPoint3d::From(m_val[Jx].asDouble(), m_val[Jy].asDouble(), m_val[Jz].asDouble() );
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bvector<Byte> ECSqlParams::ECSqlParam::GetValueBlob() const {
    bvector<Byte> byteArray;
    std::string base64 = m_val.asCString();
    Base64Utilities::Decode(byteArray, base64.c_str(), base64.size());
    return byteArray;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BeIdSet ECSqlParams::ECSqlParam::GetValueIdSet() const {
    BeAssert(m_type == Type::IdSet);
    BeIdSet set;
    set.FromString(m_val.asCString());
    return set;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlParams::ECSqlParam::ECSqlParam(std::string const& name, DPoint2d const& val): m_type(Type::Point2d),  m_name(name){
    m_val = Json::Value(Json::ValueType::objectValue);
    m_val[Jx]= val.x;
    m_val[Jy]= val.y;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlParams::ECSqlParam::ECSqlParam(std::string const& name, DPoint3d const& val): m_type(Type::Point3d), m_name(name){
    m_val = Json::Value(Json::ValueType::objectValue);
    m_val[Jx]= val.x;
    m_val[Jy]= val.y;
    m_val[Jz]= val.z;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlParams::ECSqlParam::ECSqlParam(std::string const& name, bvector<Byte> const& val): m_type(Type::Blob), m_name(name){
    Utf8String base64;
    Base64Utilities::Encode(base64, &val[0], val.size());
    m_val = Json::Value(base64);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int ECSqlParams::ECSqlParam::GetIndex() const {
    const intmax_t num = strtoimax(m_name.c_str(), nullptr, 10);
    if ((num == INTMAX_MAX && errno == ERANGE) || num == 0) {
        return -1;
    }
    return (int)num;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::vector<std::string> ECSqlParams::GetKeys() const {
    std::vector<std::string> keys;
    for(auto& p : m_params){
        keys.push_back(p.first);
    }
    return keys;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ECSqlParams::ToJs(Json::Value& val) {
    val = Json::Value(Json::ValueType::objectValue);
    for(auto& p : m_params) {
        auto& param = val[p.first];
        param = Json::Value(Json::ValueType::objectValue);
        param[JType] = (int)p.second.GetType();
        param[JValue] = p.second.GetValue();
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ECSqlParams::FromJs(Json::Value const& val) {
    m_params.clear();
    for(auto& name : val.getMemberNames()) {
        const auto& param = val[name];
        if (!param.hasMember(JType) || !param.hasMember(JValue))
            continue;

        const auto type = (ECSqlParam::Type)param[JType].asInt();
        const auto& paramVal = param[JValue];
        m_params[name] = ECSqlParam(name, type, paramVal);
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ECSqlParams::TryBindTo(ECSqlStatement& stmt, std::string& err) const {
    if (IsEmpty()) {
        return true;
    }
    for(auto& kp : m_params) {
        auto& param = kp.second;
        int index = param.IsIndexed() ? param.GetIndex() : stmt.GetParameterIndex(param.GetName().c_str());
        if (index < 1) {
            if (param.IsIndexed())
                err = "expecting binding index to be > 0";
            else
                err = SqlPrintfString("fail to find param index for named param '%s'", param.GetName().c_str()).GetUtf8CP();
            return false;
        }
        ECSqlStatus st;
        switch(param.GetType()) {
            case ECSqlParam::Type::Boolean:
                st = stmt.BindBoolean(index, param.GetValueBool()); break;
            case ECSqlParam::Type::Double:
                st = stmt.BindDouble(index, param.GetValueDouble()); break;
            case ECSqlParam::Type::Id:
                st = stmt.BindId(index, param.GetValueId());  break;
            case ECSqlParam::Type::IdSet: {
                IdSet<BeInt64Id> idSet(param.GetValueIdSet());
                st = stmt.BindIdSet(index, idSet);
                break;
            }
            case ECSqlParam::Type::Integer:
                st = stmt.BindInt(index, param.GetValueInt()); break;
            case ECSqlParam::Type::Long:
                st = stmt.BindInt64(index, param.GetValueLong()); break;
            case ECSqlParam::Type::Null:
                st = stmt.BindNull(index); break;
            case ECSqlParam::Type::Point2d:
                st = stmt.BindPoint2d(index, param.GetValuePoint2d()); break;
            case ECSqlParam::Type::Point3d:
                st = stmt.BindPoint3d(index, param.GetValuePoint3d()); break;
            case ECSqlParam::Type::String:
                st = stmt.BindText(index, param.GetValueString().c_str(), IECSqlBinder::MakeCopy::Yes); break;
            case ECSqlParam::Type::Blob: {
                auto buffer = param.GetValueBlob();
                st = stmt.BindBlob(index, &buffer[0], (int)buffer.size(), IECSqlBinder::MakeCopy::Yes); break;
            }
            case ECSqlParam::Type::Struct: {
                err = "Struct type binding not supported";
                return false;
            }
            default: {
                err = SqlPrintfString("unsupported parameter type '%d'", (int)param.GetType()).GetUtf8CP();
                return false;
            }
        };
        if (st != ECSqlStatus::Success) {
            // err = SqlPrintfString("fail to bind param '%s'. Make sure type are not missmatch", param.GetName().c_str()).GetUtf8CP();
        }
    }
    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlParams::ECSqlParam& ECSqlParams::ECSqlParam::operator = (ECSqlParam && rhs) {
    if (this != &rhs) {
        m_val = std::move(rhs.m_val);
        m_type = std::move(rhs.m_type);
        m_name = std::move(rhs.m_name);
    }
    return *this;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlParams::ECSqlParam& ECSqlParams::ECSqlParam::operator = (const ECSqlParam & rhs) {
    if (this != &rhs) {
        m_val = rhs.m_val;
        m_type = rhs.m_type;
        m_name = rhs.m_name;
    }
    return *this;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlParams& ECSqlParams::operator = (ECSqlParams && rhs) {
    if (this != &rhs) {
        m_params = std::move(rhs.m_params);
    }
    return *this;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlParams& ECSqlParams::operator = (const ECSqlParams & rhs) {
    if (this != &rhs) {
        m_params = rhs.m_params;
    }
    return *this;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ConcurrentQueryMgr::Config::Config():
    m_quota(DEFAULT_QUOTA_MAX_TIME, DEFAULT_QUOTA_MAX_MEM),
    m_workerThreadCount(DEFAULT_WORKER_THREAD_COUNT),
    m_requestQueueSize(DEFAULT_REQUEST_QUERY_SIZE),
    m_ignorePriority(DEFAULT_IGNORE_PRIORITY),
    m_ignoreDelay(DEFAULT_IGNORE_DELAY) {
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ConcurrentQueryMgr::Config::Equals(Config const& rhs) const {
    if (m_quota.MaxMemoryAllowed() != rhs.GetQuota().MaxMemoryAllowed())
        return false;
    if (m_quota.MaxTimeAllowed() != rhs.GetQuota().MaxTimeAllowed())
        return false;
    if (m_workerThreadCount != rhs.GetWorkerThreadCount())
        return false;
    if (m_requestQueueSize != rhs.GetRequestQueueSize())
        return false;
    if (m_ignorePriority != rhs.GetIgnorePriority())
        return false;
    if (m_ignoreDelay != rhs.GetIgnoreDelay())
        return false;        
    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ConcurrentQueryMgr::Config const& ConcurrentQueryMgr::Config::GetDefault() {
    static Config s_config;
    return s_config;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ConcurrentQueryMgr::Config ConcurrentQueryMgr::Config::From(std::string const& json) {
    if (json.empty()) {
        return Config::GetDefault();
    }
    LOG.infov("config from env: %s", json.c_str());
    Json::Value val = Json::Value::From(json);
    if (!val.isObject()) {
        return Config::GetDefault();
    }
    auto jsVal = BeJsConst(val);
    return From(val);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ConcurrentQueryMgr::Config::To(BeJsValue val) const {
    if (!val.isObject()) {
        val.toObject();
    }
    val[Config::JThreads] = GetWorkerThreadCount();
    val[Config::JQueueSize] = GetRequestQueueSize();
    val[Config::JIgnorePriority] = GetIgnorePriority();
    val[Config::JIgnoreDelay] = GetIgnoreDelay();
    auto quota = val[Config::JQuota];
    m_quota.ToJs(quota);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ConcurrentQueryMgr::Config ConcurrentQueryMgr::Config::From(BeJsValue val) {
    if (!val.isObject()) {
        return GetFromEnv();
    }
    auto defaultConfig = Config::GetFromEnv();
    Config config = defaultConfig;
    if (val.isNumericMember(Config::JThreads)) {
        auto threads = val[Config::JThreads].asUInt(defaultConfig.GetWorkerThreadCount());
        if (threads > std::thread::hardware_concurrency() || threads < MIN_WORKER_THREAD_COUNT) {
            threads = Config::GetDefault().GetWorkerThreadCount();
        }
        config.SetWorkerThreadCount(threads);
    }
    if (val.isNumericMember(Config::JQueueSize)) {
        auto queueSize = val[Config::JQueueSize].asUInt(defaultConfig.GetRequestQueueSize());
        if (queueSize < RunnableRequestQueue::kMinQueueSize || queueSize > MAX_REQUEST_QUERY_SIZE) {
            queueSize = Config::GetDefault().GetRequestQueueSize();
        }
        config.SetRequestQueueSize(queueSize);
    }
    if (val.isBoolMember(Config::JIgnorePriority)) {
        const auto ignorePriority = val[Config::JIgnorePriority].asBool(defaultConfig.GetIgnorePriority());
        config.SetIgnorePriority(ignorePriority);
    }
    if (val.isBoolMember(Config::JIgnoreDelay)) {
        const auto ignoreDelay = val[Config::JIgnoreDelay].asBool(defaultConfig.GetIgnoreDelay());
        config.SetIgnoreDelay(ignoreDelay);
    }    
    if (val.isObjectMember(Config::JQuota)) {
        auto quota = defaultConfig.GetQuota();
        quota = QueryQuota::FromJs(val[Config::JQuota]);
        config.SetQuota(quota);
    }
    return config;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ConcurrentQueryMgr::Config ConcurrentQueryMgr::Config::GetFromEnv() {
#if defined(BENTLEY_WIN32)
    char* buffer = 0;
    size_t count;
    // {"globalQuota": {"memory": 123, "time":123}, "workerThreads": 2, "requestQueueSize":233,"ignorePriority": false}
    _dupenv_s(&buffer, &count, "CONCURRENT_QUERY_CONFIG");
    if (buffer != nullptr || count == 0) {
        std::string str(buffer, count);
        free(buffer);
        return Config::From(str);
    }
#else
    char* buffer = std::getenv("CONCURRENT_QUERY_CONFIG");
    if (buffer != nullptr) {
        return Config::From(buffer);
    }
#endif
    return Config::GetDefault();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus QueryJsonAdaptor::RenderRow(BeJsValue rowJson, ECSqlStatement const& stmt) const {
    rowJson.SetEmptyArray();
    const int count = stmt.GetColumnCount();
    int consecutiveNulls = 0;
    for (int columnIndex = 0; columnIndex < count; columnIndex++) {
        IECSqlValue const& ecsqlValue = stmt.GetValue(columnIndex);
        if (ecsqlValue.IsNull()) {
            ++consecutiveNulls;
            continue;
        }

        while (consecutiveNulls > 0) {
            rowJson.appendValue().SetNull();
            --consecutiveNulls;
        }
        if (SUCCESS != RenderRootProperty(rowJson.appendValue(), ecsqlValue))
            return ERROR;
    }
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus QueryJsonAdaptor::RenderRootProperty(BeJsValue out, IECSqlValue const& in) const {
    return RenderProperty(out, in);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus QueryJsonAdaptor::RenderProperty(BeJsValue out, IECSqlValue const& in) const {
    auto prop = in.GetColumnInfo().GetProperty();
    if (prop == nullptr) {
        BeAssert(false && "property is null");
        return ERROR;
    }
    if (prop->GetIsPrimitive())
        return RenderPrimitiveProperty(out, in, nullptr);
    if (prop->GetIsStruct())
        return RenderStructProperty(out, in);
    if (prop->GetIsNavigation())
        return RenderNavigationProperty(out, in);
    if (prop->GetIsPrimitiveArray())
        return RenderPrimitiveArrayProperty(out, in);
    if (prop->GetIsStructArray())
        return RenderStructArrayProperty(out, in);
    BeAssert(false && "property type unsupported");
    return ERROR;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus QueryJsonAdaptor::RenderPrimitiveProperty(BeJsValue out, IECSqlValue const& in, ECN::PrimitiveType const* type) const {
    ECN::PrimitiveECPropertyCP prop = nullptr;
    ECN::PrimitiveType propType = Enum::FromInt<ECN::PrimitiveType>(0);
    if (type != nullptr) {
        propType = *type;
    } else {
        auto rootProp = in.GetColumnInfo().GetProperty();
        if (rootProp != nullptr) {
            prop = rootProp->GetAsPrimitiveProperty();
            propType = prop->GetType();
        } else {
            BeAssert("developer error");
            return ERROR;
        }
    }
    if (propType == ECN::PRIMITIVETYPE_Long) {
        return RenderLong(out, in, prop);
    }
    if (propType == ECN::PRIMITIVETYPE_String) {
        out = in.GetText();
        return SUCCESS;
    }
    if (propType == ECN::PRIMITIVETYPE_Double) {
        out = in.GetDouble();
        return SUCCESS;
    }
    if (propType == ECN::PRIMITIVETYPE_Integer) {
        out = std::trunc(in.GetDouble());
        return SUCCESS;
    }
    if (propType == ECN::PRIMITIVETYPE_Boolean) {
        out = in.GetBoolean();
        return SUCCESS;
    }
    if (propType == ECN::PRIMITIVETYPE_Binary) {
        return RenderBinaryProperty(out, in);
    }
    if (propType == ECN::PRIMITIVETYPE_DateTime) {
        out = in.GetDateTime().ToString();
        return SUCCESS;
    }
    if (propType == ECN::PRIMITIVETYPE_Point2d) {
        return RenderPoint2d(out, in);
    }
    if (propType == ECN::PRIMITIVETYPE_Point3d) {
         return RenderPoint3d(out, in);
    }
    if (propType == ECN::PRIMITIVETYPE_IGeometry){
        return RenderGeometryProperty(out, in);
    }
    BeAssert(false && "property type unsupported");
    return ERROR;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus QueryJsonAdaptor::RenderLong(BeJsValue out, IECSqlValue const& in, ECN::PrimitiveECPropertyCP prop) const {
    if (prop != nullptr) {
        const auto id = in.GetId<ECN::ECClassId>();
        const auto extendTypeId = ExtendedTypeHelper::GetExtendedType(prop->GetExtendedTypeName());
        const auto isClassId = Enum::Intersects<ExtendedTypeHelper::ExtendedType>(extendTypeId, ExtendedTypeHelper::ExtendedType::ClassIds);
        const auto isId = Enum::Intersects<ExtendedTypeHelper::ExtendedType>(extendTypeId, ExtendedTypeHelper::ExtendedType::Ids);
        if (isClassId) {
            if (!id.IsValid()) {
                return SUCCESS;
            }
            if(m_classIdToClassNames || m_useJsName) {
                auto classCP = m_ecdb.Schemas().GetClass(id);
                if (classCP != nullptr) {
                    ECN::ECJsonUtilities::ClassNameToJson(out, *classCP);
                    return SUCCESS;
                }
            }
            out = id.ToHexStr();
            return SUCCESS;
        } else if (isId) {
            if (!id.IsValid()) {
                return SUCCESS;
            }
            out = id.ToHexStr();
            return SUCCESS;
        }
    }
    out = std::trunc(in.GetDouble());
    return SUCCESS;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus QueryJsonAdaptor::RenderPoint3d(BeJsValue out, IECSqlValue const& in) const {
    const auto pt = in.GetPoint3d();
    out.SetEmptyObject();
    if (m_useJsName) {
        out[ECN::ECJsonSystemNames::Point::X()] = pt.x;
        out[ECN::ECJsonSystemNames::Point::Y()] = pt.y;
        out[ECN::ECJsonSystemNames::Point::Z()] = pt.z;
    } else {
        out[ECDBSYS_PROP_PointX] = pt.x;
        out[ECDBSYS_PROP_PointY] = pt.y;
        out[ECDBSYS_PROP_PointZ] = pt.z;
    }
    return SUCCESS;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus QueryJsonAdaptor::RenderPoint2d(BeJsValue out, IECSqlValue const& in) const {
    const auto pt = in.GetPoint2d();
    out.SetEmptyObject();
    if (m_useJsName) {
        out[ECN::ECJsonSystemNames::Point::X()] = pt.x;
        out[ECN::ECJsonSystemNames::Point::Y()] = pt.y;
    } else {
        out[ECDBSYS_PROP_PointX] = pt.x;
        out[ECDBSYS_PROP_PointY] = pt.y;
    }
    return SUCCESS;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus QueryJsonAdaptor::RenderGeometryProperty(BeJsValue out, IECSqlValue const& in) const {
    IGeometryPtr geom = in.GetGeometry();
    if (geom == nullptr)
        return ERROR;

    Utf8String jsonStr;
    if (!IModelJson::TryGeometryToIModelJsonString(jsonStr, *geom) || jsonStr.empty())
        return ERROR;

    rapidjson::Document jsonDoc;
    if (jsonDoc.Parse<0>(jsonStr.c_str()).HasParseError())
        return ERROR;

    out.From(jsonDoc);
    return SUCCESS;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus QueryJsonAdaptor::RenderBinaryProperty(BeJsValue out, IECSqlValue const& in) const {
    bool isGuid = false;
    if (in.GetColumnInfo().GetProperty() != nullptr) {
        const auto prop = in.GetColumnInfo().GetProperty()->GetAsPrimitiveProperty();
        isGuid = !prop->GetExtendedTypeName().empty() && prop->GetExtendedTypeName().EqualsIAscii("BeGuid");
    }
    int size = 0;
    Byte const* data = (Byte const*)in.GetBlob(&size);
    if (isGuid && size == sizeof(BeGuid)) {
        BeGuid guid;
        std::memcpy(&guid, data, sizeof(guid));
        out = guid.ToString().c_str();
        return SUCCESS;
    }
    if (m_abbreviateBlobs && size > 1)
        size = 1;

    out.SetBinary(data, (size_t)size);
    return SUCCESS;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus QueryJsonAdaptor::RenderNavigationProperty(BeJsValue out, IECSqlValue const& in) const {
    out.SetEmptyObject();
    const auto jsId = m_useJsName ? ECN::ECJsonSystemNames::Navigation::Id() : ECDBSYS_PROP_NavPropId;
    const auto jsClassId = m_useJsName ? ECN::ECJsonSystemNames::Navigation::RelClassName() : ECDBSYS_PROP_NavPropRelECClassId;
    auto const& navIdVal = in[ECDBSYS_PROP_NavPropId];
    if (navIdVal.IsNull())
        return SUCCESS;

    out[jsId] = navIdVal.GetId<ECInstanceId>().ToHexStr();
    auto const& relClassIdVal = in[ECDBSYS_PROP_NavPropRelECClassId];
    if (!relClassIdVal.IsNull()) {
        if (m_classIdToClassNames || m_useJsName) {
            const auto classId = relClassIdVal.GetId<ECN::ECClassId>();
            auto classCP = m_ecdb.Schemas().GetClass(classId);
            if (classCP != nullptr) {
                ECN::ECJsonUtilities::ClassNameToJson(out[jsClassId], *classCP);
                return SUCCESS;
            }
        }
        out[jsClassId] = relClassIdVal.GetId<ECN::ECClassId>().ToHexStr();
    }
    return SUCCESS;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus QueryJsonAdaptor::RenderStructProperty(BeJsValue out, IECSqlValue const& in) const {
    out.SetEmptyObject();
    for (IECSqlValue const& structMemberValue : in.GetStructIterable()) {
        if (structMemberValue.IsNull())
            continue;

        auto memberProp = structMemberValue.GetColumnInfo().GetProperty();
        if (m_useJsName) {
            Utf8String memberName = memberProp->GetName();
            ECN::ECJsonUtilities::LowerFirstChar(memberName);
            if (SUCCESS != RenderProperty(out[memberName], structMemberValue))
                return ERROR;
        } else {
            if (SUCCESS != RenderProperty(out[memberProp->GetName()], structMemberValue))
                return ERROR;
        }
    }
    return SUCCESS;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus QueryJsonAdaptor::RenderPrimitiveArrayProperty(BeJsValue out, IECSqlValue const& in) const {
    out.SetEmptyArray();
    auto elementType  = in.GetColumnInfo().GetProperty()->GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();
    for (IECSqlValue const& arrayElementValue : in.GetArrayIterable()) {
        if (arrayElementValue.IsNull())
            continue;

        if (SUCCESS != RenderPrimitiveProperty(out.appendValue(), arrayElementValue, &elementType))
            return ERROR;
    }
    return SUCCESS;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus QueryJsonAdaptor::RenderStructArrayProperty(BeJsValue out, IECSqlValue const& in) const {
    out.SetEmptyArray();
    for (IECSqlValue const& arrayElementValue : in.GetArrayIterable()) {
        if (arrayElementValue.IsNull())
            continue;

        if (SUCCESS != RenderStructProperty(out.appendValue(), arrayElementValue))
            return ERROR;
    }
    return SUCCESS;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void QueryJsonAdaptor::GetMetaData(QueryProperty::List& list, ECSqlStatement const& stmt) const {
    const int count = stmt.GetColumnCount();
    using ExtendedType = ExtendedTypeHelper::ExtendedType;
    std::map<std::string, int> uniqueJsMembers;
    for (int i = 0; i < count; i++) {
        Utf8String jsName;
        auto const& val = stmt.GetValue(i);
        auto const& col = val.GetColumnInfo();
        auto const prop = col.GetProperty();
        Utf8String extendType = prop->GetIsPrimitive() ? prop->GetAsPrimitiveProperty()->GetExtendedTypeName().c_str() : "";
        auto const extendedTypeId = ExtendedTypeHelper::GetExtendedType(extendType);
        auto const isSystem = Enum::Intersects<ExtendedType>(extendedTypeId, ExtendedType::All);
        std::string className = col.IsGeneratedProperty() || isSystem ? "" : prop->GetClass().GetFullName();
        std::string typeName = col.GetDataType().IsNavigation() ? "navigation" : prop->GetTypeFullName();
        std::string name = col.IsGeneratedProperty() ? prop->GetDisplayLabel() : prop->GetName();
        if (col.IsGeneratedProperty()) {
            jsName.assign(prop->GetDisplayLabel());
            if (jsName.empty()) {
                jsName.assign(prop->GetName());
            }
            ECN::ECJsonUtilities::LowerFirstChar(jsName);
        } else if (isSystem) {
            if (col.GetPropertyPath().Size() > 1)  {
                T_Utf8StringVector accessStringV;
                for (auto const* it : col.GetPropertyPath())
                    accessStringV.push_back(it->GetProperty()->GetName().c_str());
                Utf8String tmp = accessStringV.front() + ".";
                for (int j = 1; j < accessStringV.size() - 1; ++j)
                    tmp += accessStringV[j] + ".";

                auto &leafEntry = accessStringV.back();
                if (leafEntry == ECDBSYS_PROP_NavPropId)
                    tmp += ECN::ECJsonSystemNames::Id();
                else if (leafEntry == ECDBSYS_PROP_NavPropRelECClassId)
                    tmp += ECN::ECJsonSystemNames::Navigation::RelClassName();
                else if (leafEntry == ECDBSYS_PROP_PointX)
                    tmp += ECN::ECJsonSystemNames::Point::X();
                else if (leafEntry == ECDBSYS_PROP_PointY)
                    tmp += ECN::ECJsonSystemNames::Point::Y();
                else if (leafEntry == ECDBSYS_PROP_PointZ)
                    tmp += ECN::ECJsonSystemNames::Point::Z();
                else
                    tmp += leafEntry;

                jsName = tmp;
                ECN::ECJsonUtilities::LowerFirstChar(jsName);
            } else {
                switch(extendedTypeId) {
                    case ExtendedType::Id: jsName = ECN::ECJsonSystemNames::Id(); break;
                    case ExtendedType::ClassId: jsName = ECN::ECJsonSystemNames::ClassName(); break;
                    case ExtendedType::SourceId: jsName = ECN::ECJsonSystemNames::SourceId(); break;
                    case ExtendedType::SourceClassId:jsName = ECN::ECJsonSystemNames::SourceClassName(); break;
                    case ExtendedType::TargetId: jsName = ECN::ECJsonSystemNames::TargetId(); break;
                    case ExtendedType::TargetClassId: jsName = ECN::ECJsonSystemNames::TargetClassName(); break;
                    case ExtendedType::NavId: jsName = ECN::ECJsonSystemNames::Navigation::Id(); break;
                    case ExtendedType::NavRelClassId: jsName = ECN::ECJsonSystemNames::Navigation::RelClassName(); break;
                }
            }
        } else {
            jsName = col.GetPropertyPath().ToString();
            ECN::ECJsonUtilities::LowerFirstChar(jsName);
        }
        if (uniqueJsMembers.find(jsName) == uniqueJsMembers.end()) {
            uniqueJsMembers[jsName] = 0;
        } else {
            uniqueJsMembers[jsName]++;
            Utf8String suffix;
            suffix.Sprintf("_%d", uniqueJsMembers[jsName]);
            jsName.append(suffix);
        }
        list.append(className, jsName, name, typeName, col.IsGeneratedProperty(), extendType, i);
    }
}
END_BENTLEY_SQLITE_EC_NAMESPACE
