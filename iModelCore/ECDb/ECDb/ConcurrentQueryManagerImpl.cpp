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
#include <optional>

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
std::shared_ptr<CachedQueryAdaptor> QueryAdaptorCache::TryGet(Utf8CP ecsql, bool usePrimaryConn, bool suppressLogError, ECSqlStatus& status, std::string& ecsql_error, RunnableRequestQueue& queue, bool& isShutDownInProgress) {
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
    BeMutex& ecdbMutex = m_conn.GetPrimaryDb().GetImpl().GetMutex();
    while(!ecdbMutex.try_lock()) {
        if(queue.GetState() == RunnableRequestQueue::State::Stop) {
            status = ECSqlStatus::Error;
            isShutDownInProgress = true;
            ecsql_error = "Queue is shut down, cannot go ahead with the request.";
            return nullptr;
        }
        std::this_thread::yield();
    }
    ErrorListenerScope err_scope(const_cast<ECDb&>(m_conn.GetPrimaryDb()));
    if (usePrimaryConn)
        status = newCachedAdaptor->GetStatement().Prepare(m_conn.GetPrimaryDb(), ecsql, !suppressLogError);
    else {
        if (m_doNotUsePrimaryConnToPrepare) {
            status = newCachedAdaptor->GetStatement().Prepare(m_conn.GetDb(), ecsql, !suppressLogError);
        } else {
            status = newCachedAdaptor->GetStatement().Prepare(m_conn.GetPrimaryDb().Schemas(), m_conn.GetDb(), ecsql, !suppressLogError);
        }
    }
    if (status != ECSqlStatus::Success) {
        ecsql_error = err_scope.GetLastError();
        ecdbMutex.unlock();
        return nullptr;
    }
    while (m_cache.size() > m_maxEntries)
        m_cache.pop_back();

    m_cache.insert(m_cache.begin(), newCachedAdaptor);
    ecdbMutex.unlock();
    return newCachedAdaptor;


}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlRowAdaptor& QueryAdaptor::GetJsonAdaptor() {
    if (!m_adaptor) {
         m_adaptor = std::unique_ptr<ECSqlRowAdaptor>(new ECSqlRowAdaptor(*m_stmt.GetECDb()));
    }
    return *m_adaptor.get();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void CachedConnection::SyncAttachDbs() {
    recursive_guard_t lock(m_mutexReq);
    if (m_request != nullptr || !m_db.IsDbOpen()) {
        // cannot sync attach dbs if request is pending.
        return;
    }

    // simple fast way to verify if attach file have changed or not.
    const auto primaryAttachDbs = GetPrimaryDb().GetAttachedDbs();
    const auto thisAttachDbs = GetDb().GetAttachedDbs();
    std::once_flag cachedClearFlag;
    auto reset = [&]() {
        m_adaptorCache.Reset();
        m_db.ClearECDbCache();
    };

    // detach dbs that does not exist on primary connection
    for (auto& attachFile : thisAttachDbs) {
        if (attachFile.m_type == AttachFileType::Main || attachFile.m_type == AttachFileType::Temp) {
            continue;
        }

        BeAssert(attachFile.m_type != AttachFileType::SchemaSync);

        auto it = std::find_if(primaryAttachDbs.begin(), primaryAttachDbs.end(),
        [&attachFile](auto& primaryAttach) {
            return primaryAttach.m_alias.EqualsIAscii(attachFile.m_alias) &&
                primaryAttach.m_fileName.EqualsIAscii(attachFile.m_fileName);
        });

        if (it == primaryAttachDbs.end()) {
            std::call_once(cachedClearFlag, reset);
            if (attachFile.m_type == AttachFileType::ECChangeCache)
                m_db.DetachChangeCache();
            else
                m_db.DetachDb(attachFile.m_alias.c_str());
        }
    }

    // attach dbs that exist on primary connection but not on cached connection.
    for (auto& attachFile : primaryAttachDbs) {
        if (attachFile.m_type == AttachFileType::SchemaSync || attachFile.m_type == AttachFileType::Main || attachFile.m_type == AttachFileType::Temp) {
            continue;
        }

        auto it = std::find_if(thisAttachDbs.begin(), thisAttachDbs.end(),
        [&attachFile](auto& thisAttach) {
            return thisAttach.m_alias.EqualsIAscii(attachFile.m_alias) && thisAttach.m_fileName.EqualsIAscii(attachFile.m_fileName);
        });

        if (it == thisAttachDbs.end()) {
            std::call_once(cachedClearFlag, reset);
            if (attachFile.m_type == AttachFileType::ECChangeCache)
                m_db.AttachChangeCache(BeFileName(attachFile.m_fileName.c_str()));
            else
                m_db.AttachDb(attachFile.m_fileName.c_str(), attachFile.m_alias.c_str());
        }
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void CachedConnection::Execute(std::function<void(QueryAdaptorCache&,RunnableRequestBase&)> cb, std::unique_ptr<RunnableRequestBase> request) {
    if (m_db.IsDbOpen()) {
        // Check if the primary file data version has changed
        uint32_t dataVersion;
        if (BE_SQLITE_OK == GetPrimaryDb().GetFileDataVersion(dataVersion)) {
            if (dataVersion != m_primaryFileDataVer) {
                m_db.ClearECDbCache();
                m_db.ClearDbCache();
                m_primaryFileDataVer = dataVersion;
            }
        }
    }

    SyncAttachDbs();
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
        if (cb(*m_request)) {
            if (cancel) {
                m_request->Cancel();
            }
            m_request->Interrupt(*this);
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
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::vector<CachedConnection::FunctionInfo> CachedConnection::GetPrimaryDbSqlFunctions() const {
    std::vector<FunctionInfo> funcs;
    std::regex funcFilter("^imodel_\\w*", std::regex_constants::ECMAScript | std::regex_constants::icase);
    if (!m_cache.GetPrimaryDb().IsDbOpen())
        return funcs;

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
    const auto mmsize = ConcurrentQueryMgr::Config::Get().GetMemoryMapFileSize();
    if (mmsize > 0) {
        newConn->m_db.ExecuteSql(SqlPrintfString("PRAGMA mmap_size=%" PRIu32, mmsize));
    }

    cache.GetPrimaryDb().GetFileDataVersion(newConn->m_primaryFileDataVer);
    return newConn;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ConnectionCache::SyncAttachDbs() {
    FNV1HashBuilder builder;
    for (auto& file : GetPrimaryDb().GetAttachedDbs()) {
        if (file.m_type == AttachFileType::SchemaSync  || file.m_type == AttachFileType::Main || file.m_type == AttachFileType::Temp) {
            continue;
        }
        builder.UpdateString(file.m_alias);
        builder.UpdateString(file.m_fileName);
    }

    const auto hashCode = builder.GetHashCode();
    if (hashCode == m_primaryAttachFileHash) {
        return;
    }

    recursive_guard_t lock(m_mutex);
    m_primaryAttachFileHash = hashCode;
    for (auto& conn: m_conns) {
        if (conn != nullptr)
            conn->SyncAttachDbs();
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ConnectionCache::InterruptIf(std::function<bool(RunnableRequestBase const&)> predicate, bool cancel) {
    recursive_guard_t lock(m_mutex);
    for (auto& conn: m_conns) {
        if (conn != nullptr)
            conn->InterruptIf(predicate, cancel);
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ConnectionCache::ConnectionCache(ECDb const& primaryDb, uint32_t pool_size): m_primaryDb(primaryDb), m_poolSize(pool_size) {
    if (!primaryDb.IsDbOpen())
        throw std::runtime_error("primary db connection must be open");

    if (m_poolSize < 1) {
        m_poolSize = ConcurrentQueryMgr::Config::Get().GetWorkerThreadCount();
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
                it->InterruptIf(
                    [](RunnableRequestBase const& request) {
                        return true;
                    },
                    true);
            }
        }
        std::this_thread::yield();
    }
    if (reset_conn) {
        if(m_syncConn)
            m_syncConn->Reset(detach_dbs);

        for (auto& it : m_conns) {
            it->Reset(detach_dbs);
        }
    }
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ErrorListenerScope::_OnIssueReported(ECN::IssueSeverity severity, ECN::IssueCategory category, ECN::IssueType type, ECN::IssueId id, Utf8CP message) const{
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
QueryResponse::Ptr CreateResponseHelper::CreateQueueFullResponse() {
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
void RunnableRequestBase::Interrupt(CachedConnection& conn) {
    m_interrupted = true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Ptr CreateResponseHelper::CreateErrorResponse(QueryResponse::Status status,std::string error, std::chrono::microseconds cpuTime, std::chrono::milliseconds totalTime, const BentleyM0200::BeSQLite::EC::QueryQuota &quota, std::chrono::milliseconds prepareTime) const {
    BeAssert((int)status >= (int)QueryResponse::Status::Error);
    if ((int)status < (int)QueryResponse::Status::Error) {
        return nullptr;
    }
    return std::make_shared<QueryResponse>(
        ECSqlResponse::Kind::NoResult,
        QueryResponse::Stats(cpuTime, totalTime, 0, quota, prepareTime),
        status,
        error
        );
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Ptr CreateResponseHelper::CreateTimeoutResponse(std::chrono::microseconds cpuTime, std::chrono::milliseconds totalTime, const BentleyM0200::BeSQLite::EC::QueryQuota &quota, std::chrono::milliseconds prepareTime) const {
    return std::make_shared<QueryResponse>(
        ECSqlResponse::Kind::NoResult,
        QueryResponse::Stats(cpuTime, totalTime, 0, quota, prepareTime),
        QueryResponse::Status::Timeout,
        ""
        );
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Ptr CreateResponseHelper::CreateCancelResponse(std::chrono::microseconds cpuTime, std::chrono::milliseconds totalTime, const BentleyM0200::BeSQLite::EC::QueryQuota &quota, std::chrono::milliseconds prepareTime) const {
    return std::make_shared<QueryResponse>(
        ECSqlResponse::Kind::NoResult,
        QueryResponse::Stats(cpuTime, totalTime, 0, quota, prepareTime),
        QueryResponse::Status::Cancel,
        ""
        );
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Ptr CreateResponseHelper::CreateBlobIOResponse(std::vector<uint8_t>& buffer, bool done, uint32_t blobSize, std::chrono::microseconds cpuTime, std::chrono::milliseconds totalTime, const BentleyM0200::BeSQLite::EC::QueryQuota &quota, std::chrono::milliseconds prepareTime) const {
    const auto memUsed = (uint32_t)buffer.size();
    return std::make_shared<BlobIOResponse>(
        QueryResponse::Stats(cpuTime, totalTime, memUsed, quota, prepareTime),
        done? QueryResponse::Status::Done:QueryResponse::Status::Partial,
        "",
        buffer,
        blobSize);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Ptr CreateResponseHelper::CreateECSqlResponse(std::string& resultJson, ECSqlRowProperty::List& meta, uint32_t rowCount, bool done, std::chrono::microseconds cpuTime, std::chrono::milliseconds totalTime, const BentleyM0200::BeSQLite::EC::QueryQuota &quota, std::chrono::milliseconds prepareTime) const {
    const auto memUsed = (uint32_t)(resultJson.size());
    return std::make_shared<ECSqlResponse>(
        QueryResponse::Stats(cpuTime, totalTime, memUsed, quota, prepareTime),
        done? QueryResponse::Status::Done:QueryResponse::Status::Partial,
        "",
        resultJson,
        meta,
        rowCount);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Ptr CreateResponseHelper::CreateShutDownResponse(std::chrono::microseconds cpuTime, std::chrono::milliseconds totalTime, const BentleyM0200::BeSQLite::EC::QueryQuota &quota, std::chrono::milliseconds prepareTime) const {
    return std::make_shared<QueryResponse>(
        QueryResponse::Kind::NoResult,
        QueryResponse::Stats(cpuTime, totalTime, 0, quota, prepareTime),
        QueryResponse::Status::ShuttingDown,
        ""
        );
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
RunnableRequestQueue::RunnableRequestQueue(ECDbCR ecdb): m_nextId(0), m_state(State::Running), m_lastDelayedQueryId(0),m_ecdb(ecdb) {
    auto env = ConcurrentQueryMgr::Config::Get();
    m_quota = env.GetQuota();
    m_maxQueueSize = env.GetRequestQueueSize();
    m_shutdownWhenIdleFor = env.GetAutoShutdownWhenIdleForSeconds();
    m_lastDequeueTime = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::steady_clock::now());
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
uint32_t RunnableRequestQueue::GetNextId () {
    recursive_guard_t lock(m_mutex);
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
    recursive_guard_t lock(m_mutex);
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
void RunnableRequestQueue::IfReadyForAutoShutdown(std::function<void()> shutdownCb) {
    recursive_guard_t lock(m_mutex);
    if (m_shutdownWhenIdleFor == 0s || !m_requests.empty() || m_state.load() != State::Running)
        return;

    const auto elapsedSinceLastRequest = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - m_lastDequeueTime);
    if (elapsedSinceLastRequest > m_shutdownWhenIdleFor) {
        shutdownCb();
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
std::unique_ptr<RunnableRequestBase> RunnableRequestQueue::Dequeue() {
    if (m_requests.empty())
        return nullptr;

    auto req = std::move(m_requests.back());
    m_requests.pop_back();
    m_lastDequeueTime  = std::chrono::steady_clock::now();
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
    std::unique_lock<std::recursive_mutex> lock(m_mutex);
    m_cond.wait(lock, [&](){
        return !m_requests.empty() || m_state.load() != State::Running;
    });

    // If paused, wait until state changes to something else, if we return immediately, this would
    // cause the caller to immediately call again and loop infinitely.
    if (m_state.load() == State::Paused) {
        m_cond.wait(lock, [&](){ return m_state.load() != State::Paused; });
    }

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
    if (m_quota.MaxMemoryAllowed() < maxMem || maxMem <= 32000)
        maxMem = m_quota.MaxMemoryAllowed() ;
    if (m_quota.MaxTimeAllowed() < maxTime || maxTime <= std::chrono::seconds(10))
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
        log_trace("%s executing query synchronously [id=%" PRIu32 "] ended.",GetTimestamp().c_str(), runnableQuery.GetId());
    }, std::move(runnableReq));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Future RunnableRequestQueue::Enqueue(ConnectionCache& conns, QueryRequest::Ptr request) {
    // Put a upper limit on query delay to make it safe.
    const auto& conf = ConcurrentQueryMgr::Config::Get();
    if (conf.GetIgnoreDelay()) {
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
            runnableReq->SetResponse(runnableReq->CreateShutDownResponse());
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
    const auto& conf = ConcurrentQueryMgr::Config::Get();
    if (conf.GetIgnoreDelay()) {
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
            runnableReq->SetResponse(runnableReq->CreateShutDownResponse());
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
    recursive_guard_t lock(m_mutex);
    return (uint32_t)m_requests.size();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool RunnableRequestQueue::Stop() {
    if (m_state.load() == State::Stop)
        return false;

    log_trace("%s stopping request queue.", GetTimestamp().c_str());
    m_state.store(State::Stop);

    recursive_guard_t lock(m_mutex);
    for(auto & request : m_requests) {
        log_error("%s cancelling request [id=%" PRIu32 "] due to shutdown.", GetTimestamp().c_str(), request->GetId());
        request->SetResponse(request->CreateShutDownResponse());
    }
    m_cond.notify_all();
    log_trace("%s request queue stopped.", GetTimestamp().c_str());
    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void RunnableRequestQueue::RemoveIf (std::function<bool(RunnableRequestBase&)> predicate) {
    recursive_guard_t lock(m_mutex);
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
bool RunnableRequestQueue::CancelRequest(uint32_t id) {
    recursive_guard_t lock(m_mutex);
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
namespace {
    // This function has been brought over from delComment() in the parser which deletes the comments from an ECSql statement.
    // delComment has an early exit check when no comments are found.
    Utf8String RemoveCommentsFromECSql(Utf8String const& query) {
        if (query.find("--") == Utf8String::npos && query.find("//") == Utf8String::npos && query.find("/*") == Utf8String::npos)
            return query;

        const char* pCopy = query.c_str();
        size_t nQueryLen = query.size();
        bool bIsText1  = false;     // "text"
        bool bIsText2  = false;     // 'text'
        bool bComment2 = false;     // /* comment */
        bool bComment  = false;     // -- or // comment
        Utf8String result;
        result.reserve(nQueryLen);
        
        for (size_t i = 0; i < nQueryLen; ++i) {
            if (bComment2) {
                if ((i + 1) < nQueryLen) {
                    if (pCopy[i] == '*' && pCopy[i + 1] == '/') {
                        bComment2 = false;
                        ++i;
                    }
                }
                continue;
            }
            
            if (pCopy[i] == '\n')
                bComment = false;
            else if (!bComment) {
                if (pCopy[i] == '\"' && !bIsText2)
                    bIsText1 = !bIsText1;
                else if (pCopy[i] == '\'' && !bIsText1)
                    bIsText2 = !bIsText2;
                    
                if (!bIsText1 && !bIsText2 && (i + 1) < nQueryLen) {
                    if ((pCopy[i] == '-' && pCopy[i + 1] == '-') || (pCopy[i] == '/' && pCopy[i + 1] == '/'))
                        bComment = true;
                    else if (pCopy[i] == '/' && pCopy[i + 1] == '*')
                        bComment2 = true;
                }
            }
            
            if (!bComment && !bComment2)
                result.append(&pCopy[i], 1);
        }
        return result;
    }
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
        trimmedECSql = RemoveCommentsFromECSql(trimmedECSql);
        std::regex rx("\\)\\s*select", std::regex_constants::ECMAScript | std::regex_constants::icase);
        std::match_results<Utf8String::const_iterator> matches;
        if (std::regex_search<Utf8String::const_iterator>(trimmedECSql.begin(), trimmedECSql.end(), matches, rx)) {
            auto n = matches.position();
            Utf8String prefix = trimmedECSql.substr(0, n + 1);
            Utf8String select = trimmedECSql.substr(n + 2);
            return Utf8PrintfString("%s select * from (%s \n) limit :" LIMIT_VAR_COUNT " offset :" LIMIT_VAR_OFFSET, prefix.c_str(), select.c_str());
        }
    }
    if (trimmedECSql.StartsWithIAscii("pragma")) {
        return std::move(trimmedECSql);
    }
    return Utf8PrintfString("select * from (%s \n) limit :" LIMIT_VAR_COUNT " offset :" LIMIT_VAR_OFFSET, trimmedECSql.c_str());
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
ECSqlRowProperty const& ECSqlRowProperty::List::GetPropertyInfo(std::string const& name) const {
    static ECSqlRowProperty kNull;
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
ECSqlRowProperty::List QueryHelper::GetMetaInfo(CachedQueryAdaptor& adp, bool classIdToClassNames) {
    ECSqlRowProperty::List props;

    return props;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ECSqlRowProperty::ToJs(BeJsValue& val) const {
    val.toObject();
    val[JClass]=m_className;
    val[JAccessString]=m_accessString;
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
void ECSqlRowProperty::List::ToJs(BeJsValue& val) const {
    val.toArray();
    for(auto it = begin(); it != end(); ++it) {
        auto newEl = val.appendValue();
        (*it).ToJs(newEl);
    }
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ECSqlRowProperty::List::append(std::string className, std::string accessString, std::string jsonName, std::string name, std::string typeName, bool generated, std::string extendedType, int index) {
    emplace_back(ECSqlRowProperty(className, accessString, jsonName, name, typeName, generated, extendedType, index));
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
        case QueryResponse::Status::ShuttingDown:
            return "ShuttingDown";
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
    const auto doNotConvertClassIdsToClassNamesWhenAliased = request.GetDoNotConvertClassIdsToClassNamesWhenAliased();
    auto& stmt = cachedAdaptor.GetStatement();
    auto& adaptor = cachedAdaptor.GetJsonAdaptor();
    auto& options = adaptor.GetOptions();
    auto conn = cachedAdaptor.GetWorkerConn();
    options.SetAbbreviateBlobs(abbreviateBlobs);
    options.SetConvertClassIdsToClassNames(classIdToClassNames);
    options.SetUseJsNames(request.GetValueFormat() == ECSqlRequest::ECSqlValueFormat::JsNames);
    options.SetDoNotConvertClassIdsToClassNamesWhenAliased(doNotConvertClassIdsToClassNamesWhenAliased);
    ECSqlRowProperty::List props;
    if (includeMetaData) {
        adaptor.GetMetaData(props ,stmt);
    }
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
    
        if (conn->IsDbOpen())
            conn->SetProgressHandler(nullptr);
    };
    auto setError = [&] (QueryResponse::Status status, std::string err) {
        runnableRequest.SetResponse(runnableRequest.CreateErrorResponse(status, err));
        log_error("%s. (%s)", err.c_str(), QueryResponse::StatusToString(status));
    };

    if(conn->IsDbOpen()) {
        conn->SetProgressHandler([&](){
            if (runnableRequest.IsTimeExceeded()) {
                log_trace("%s time exceeded for query [id=%" PRIu32 "]", GetTimestamp().c_str(), runnableRequest.GetId());
                return DbProgressAction::Interrupt;
            }

            if(runnableRequest.IsInterrupted()) {
                log_trace("%s query cancelled [id=%" PRIu32 "]", GetTimestamp().c_str(), runnableRequest.GetId());
                return DbProgressAction::Interrupt;
            }
            return DbProgressAction::Continue;
        }, static_cast<int>(ConcurrentQueryMgr::Config::Get().GetProgressOpCount()));
    }
    // go over each row and serialize result
    auto rc = stmt.Step();
    while (rc == BE_SQLITE_ROW) {
        auto& rowsDoc = cachedAdaptor.ClearAndGetCachedJsonDocument();
        BeJsValue rows(rowsDoc);
        if (adaptor.RenderRowAsArray(rows, ECSqlStatementRow(stmt)) != SUCCESS) {
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
        std::string sql = QueryHelper::FormatQuery(request.GetQuery().c_str());
        ECSqlStatus status;
        std::string err;
        bool isShutDownInProgress = false;
        const auto prepareTimeStart = std::chrono::steady_clock::now();
        auto recordPrepareTime = [&]() {
            runnableRequest.SetPrepareTime(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - prepareTimeStart));
        };

        auto adaptor = adaptorCache.TryGet(sql.c_str(), request.UsePrimaryConnection(), request.GetSuppressLogErrors(), status, err, runnableRequest.GetQueue(), isShutDownInProgress);
        if (adaptor == nullptr) {
            recordPrepareTime();
            if(isShutDownInProgress) {
                log_error("%s cancelling request [id=%" PRIu32 "] because %s.", GetTimestamp().c_str(), runnableRequest.GetId(), err.c_str());
                runnableRequest.SetResponse(runnableRequest.CreateShutDownResponse());
                return;
            }
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
            recordPrepareTime();
            setError(QueryResponse::Status::Error_ECSql_BindingFailed, err);
            return;
        }
        BindLimits(adaptor->GetStatement(), request.GetLimit());
        recordPrepareTime();
        QueryHelper::Execute(*adaptor, runnableRequest);
    } else {
        setError(QueryResponse::Status::Error, "unsupported kind of request");
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryExecutor::QueryExecutor(RunnableRequestQueue& queue, ECDbCR primaryDb, uint32_t pool_size) :m_queue(queue), m_connCache(primaryDb, pool_size),m_maxPoolSize(pool_size),m_threadCount(0) {
    if (pool_size < 1) {
        pool_size = ConcurrentQueryMgr::Config::Get().GetWorkerThreadCount();
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
QueryMonitor::QueryMonitor(RunnableRequestQueue& queue, QueryExecutor& executor)
    :m_stop(false), m_queue(queue), m_executor(executor) {
    const auto& config = ConcurrentQueryMgr::Config::Get();
    m_pollInterval = config.GetMonitorPollInterval();
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
                log_trace("%s monitor interrupt query [id=%" PRIu32 "] with timeout", GetTimestamp().c_str(), request.GetId());
                return true;
                });

            m_executor.GetConnectionCache().SyncAttachDbs();
            m_queue.IfReadyForAutoShutdown([&](){
                m_queue.Stop();
                log_trace("%s monitor invoking autoshutdown.", GetTimestamp().c_str());
                std::thread([&]() {
                    ConcurrentQueryMgr::Shutdown(m_queue.GetECDb());
                }).detach();
            });

            std::unique_lock<std::mutex> lock(m_queryMonitorMutex);
            m_queryMonitorCv.wait_for(lock,m_pollInterval,[&]{ return m_stop.load() == true; });
            m_queryMonitorCv.notify_all();
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
QueryMonitor::~QueryMonitor() {
    m_stop.store(true);
    m_queryMonitorCv.notify_all();
    if (m_thread.joinable())
        m_thread.join();
}

ConcurrentQueryMgr::ConcurrentQueryMgr(ECDbCR ecdb){ m_impl = new Impl(ecdb);}
ConcurrentQueryMgr::~ConcurrentQueryMgr(){ delete m_impl;}
QueryResponse::Future ConcurrentQueryMgr::Enqueue(QueryRequest::Ptr request) { return m_impl->Enqueue(std::move(request)); }
void ConcurrentQueryMgr::Enqueue(QueryRequest::Ptr request, OnCompletion onCompletion){ m_impl->Enqueue(std::move(request), onCompletion); }
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
    v[kPrepareTime] = (int64_t)m_prepareTime.count();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ConcurrentQueryMgr::Impl::Impl(ECDbCR ecdb): m_executor(m_queue, ecdb),m_monitor(m_queue, m_executor),m_queue(ecdb) {

}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ConcurrentQueryMgr::Impl::~Impl() {
    m_queue.Stop();
}

ConcurrentQueryMgr::Config ConcurrentQueryMgr::Config::s_config = ConcurrentQueryMgr::Config::GetFromEnv();

//---------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------------------------------------------------------------------------------
const ConcurrentQueryMgr::Config& ConcurrentQueryMgr::Config::Get() {
    return s_config;
}

//---------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------------------------------------------------------------------------------
const ConcurrentQueryMgr::Config& ConcurrentQueryMgr::Config::Reset(std::optional<Config> conf) {
    s_config = conf.value_or(ConcurrentQueryMgr::Config::GetFromEnv());
    return s_config;
}


//---------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------------------------------------------------------------------------------
void ConcurrentQueryMgr::WithInstance(ECDbCR ecdb, std::function<void(ConcurrentQueryMgr&)> cb) {
    if (!ecdb.IsDbOpen()) {
        throw std::runtime_error("ecdb is closed or not open");
    }

    auto& appKey = ConcurrentQueryAppData::GetKey();
    RefCountedPtr<ConcurrentQueryAppData> appData;
    if(true) {
        BeMutexHolder lock (ecdb.GetImpl().GetMutex());
        appData = ecdb.FindAppDataOfType<ConcurrentQueryAppData>(appKey);
        if (appData.IsNull()) {
            appData = ConcurrentQueryAppData::Create(ecdb);
            ecdb.AddAppData(appKey, appData.get());
        }
    }
    cb (appData->GetConcurrentQuery());
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
        m_quota = QueryQuota::FromJs(a);
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
    if (val.isBoolMember(JsReadOptions::JDoNotConvertClassIdsToClassNamesWhenAliased)) {
        m_doNotConvertClassIdsToClassNamesWhenAliased = val[JsReadOptions::JDoNotConvertClassIdsToClassNamesWhenAliased].asBool();
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
                BinderInfo const& binderInfo = stmt.GetBinderInfo(index);
                if(binderInfo.GetType() == BinderInfo::BinderType::VirtualSet) {
                    std::shared_ptr<IdSet<BeInt64Id>> idSet = std::make_shared<IdSet<BeInt64Id>>(param.GetValueIdSet());
                    st = stmt.BindVirtualSet(index, idSet);
                }
                else if(binderInfo.GetType() == BinderInfo::BinderType::Array && binderInfo.IsForIdSet()) {
                    bool allElementsAdded = true;
                    IECSqlBinder& binder = stmt.GetBinder(index);
                    IdSet<BeInt64Id> set(param.GetValueIdSet());
                    for(auto& ids: set) {
                        if(!ids.IsValid()) {
                            allElementsAdded = false;
                            break;
                        }
                        st = binder.AddArrayElement().BindInt64((int64_t) ids.GetValue());
                        if(!st.IsSuccess()) {
                            allElementsAdded = false;
                            break;
                        }
                    }
                    if(allElementsAdded) // If even one array element has failed to be added we set the status for the entire operation as ECSqlStatus::Error although for the time being we don't do anything with status even if it fails
                        st = ECSqlStatus::Success;
                    else
                        st = ECSqlStatus::Error;
                }
                else
                    st = ECSqlStatus::Error;

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
    m_ignoreDelay(DEFAULT_IGNORE_DELAY),
    m_doNotUsePrimaryConnToPrepare(DEFAULT_DONOT_USE_PRIMARY_CONN_TO_PREPARE),
    m_autoShutdownWhenIdleForSeconds(DEFAULT_SHUTDOWN_WHEN_IDLE_FOR_SECONDS),
    m_monitorPollInterval(std::chrono::milliseconds(DEFAULT_MONITOR_POLL_INTERVAL)),
    m_progressOpCount(DEFAULT_PROGRESS_OP_COUNT),
    m_statementCacheSizePerWorker(DEFAULT_STATEMENT_CACHE_SIZE_PER_WORKER), m_memoryMapFileSize(0){
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
    if (m_doNotUsePrimaryConnToPrepare != rhs.GetDoNotUsePrimaryConnToPrepare())
        return false;
    if (m_autoShutdownWhenIdleForSeconds != rhs.GetAutoShutdownWhenIdleForSeconds())
        return false;
    if (m_statementCacheSizePerWorker != rhs.GetStatementCacheSizePerWorker())
        return false;
    if (m_monitorPollInterval != rhs.GetMonitorPollInterval())
        return false;
    if (m_memoryMapFileSize != rhs.GetMemoryMapFileSize())
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
    val[Config::JDoNotUsePrimaryConnToPrepare] = GetDoNotUsePrimaryConnToPrepare();
    val[Config::JAutoShutdownWhenIdleForSeconds] =static_cast<uint32_t>(GetAutoShutdownWhenIdleForSeconds().count());
    val[Config::JStatementCacheSizePerWorker] = GetStatementCacheSizePerWorker();
    val[Config::JMonitorPollInterval] = static_cast<uint32_t>(GetMonitorPollInterval().count());
    val[Config::JMemoryMapFileSize] = GetMemoryMapFileSize();
    val[Config::JProgressOpCount] = GetProgressOpCount();
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
    if(val.isNumericMember(Config::JProgressOpCount)){
        auto progressOpCount = val[Config::JProgressOpCount].asUInt(defaultConfig.GetProgressOpCount());
        if (progressOpCount < MIN_PROGRESS_OP_COUNT || progressOpCount > MAX_PROGRESS_OP_COUNT) {
            progressOpCount = Config::GetDefault().GetProgressOpCount();
        }
        config.SetProgressOpCount(progressOpCount);
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
    if (val.isBoolMember(Config::JDoNotUsePrimaryConnToPrepare)) {
        const auto doNotUsePrimaryConnToPrepare = val[Config::JDoNotUsePrimaryConnToPrepare].asBool(defaultConfig.GetDoNotUsePrimaryConnToPrepare());
        config.SetDoNotUsePrimaryConnToPrepare(doNotUsePrimaryConnToPrepare);
    }
    if (val.isNumericMember(Config::JAutoShutdownWhenIdleForSeconds)) {
        const auto autoShutdownWhenIdleForSeconds = val[Config::JAutoShutdownWhenIdleForSeconds].asUInt64(defaultConfig.GetAutoShutdownWhenIdleForSeconds().count());
        config.SetAutoShutdownWhenIdleForSeconds(std::chrono::seconds(autoShutdownWhenIdleForSeconds));
    }
    if (val.isNumericMember(Config::JStatementCacheSizePerWorker)) {
        auto statementCacheSizePerWorker = val[Config::JStatementCacheSizePerWorker].asUInt(defaultConfig.GetStatementCacheSizePerWorker());
        if (statementCacheSizePerWorker> MAX_STATEMENT_CACHE_SIZE_PER_WORKER)
            statementCacheSizePerWorker = MAX_STATEMENT_CACHE_SIZE_PER_WORKER;
        config.SetStatementCacheSizePerWorker(statementCacheSizePerWorker);
    }
    if (val.isNumericMember(Config::JMonitorPollInterval)) {
        auto pollInterval = val[Config::JMonitorPollInterval].asInt64(defaultConfig.GetMonitorPollInterval().count());
        if (pollInterval < MIN_MONITOR_POLL_INTERVAL)
            pollInterval = MIN_MONITOR_POLL_INTERVAL;

        const auto maxTimeAllowed = std::chrono::duration_cast<std::chrono::milliseconds>(config.GetQuota().MaxTimeAllowed()).count();
        if (pollInterval > maxTimeAllowed)
            pollInterval = maxTimeAllowed;

        config.SetMonitorPollInterval(std::chrono::milliseconds(pollInterval));
    }
    if (val.isNumericMember(Config::JMemoryMapFileSize)) {
        uint32_t memoryMapFileSize = (uint32_t)val[Config::JMemoryMapFileSize].asUInt(defaultConfig.GetMemoryMapFileSize());
        config.SetMemoryMapFileSize(memoryMapFileSize);
    }
    return config;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------

QueryAdaptorCache::QueryAdaptorCache(CachedConnection& conn):m_conn(conn){
    auto config = ConcurrentQueryMgr::Config::Get();
    m_maxEntries = config.GetStatementCacheSizePerWorker();
    m_doNotUsePrimaryConnToPrepare = config.GetDoNotUsePrimaryConnToPrepare();
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

ECSqlRowReader::ECSqlRowReader(ECDbCR ecdb){ m_impl = new Impl(ecdb);}
ECSqlRowReader::~ECSqlRowReader(){ delete m_impl;}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Ptr ECSqlRowReader::Step(ECSqlRequest::Ptr request) {
    return m_impl->Step(std::move(request));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Ptr ECSqlRowReader::Impl::Step(ECSqlRequest::Ptr request) {
    RunnableRequestStatsHelper helper(request->GetQuota());
    helper.OnDequeued(); // In sync mode there is no queue involved but we want to have the stats populated correctly so we call OnDequeued here to set the start time for the request processing
    auto response = TryExecute(std::move(request), helper);
    return response;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Ptr ECSqlRowReader::Impl::TryExecute(ECSqlRequest::Ptr request, RunnableRequestStatsHelper& runnableRequestHelper) {
    // Validating ecsql
    CreateResponseHelper responseHelper;
    const auto prepareTimeStart = std::chrono::steady_clock::now();
    auto recordPrepareTime = [&]() {
        runnableRequestHelper.SetPrepareTime(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - prepareTimeStart));
    };

    const auto setError = [&responseHelper, &runnableRequestHelper](std::string errMsg, QueryResponse::Status status) -> QueryResponse::Ptr {
        log_error("%s. (%s)", errMsg.c_str(), QueryResponse::StatusToString(status));
        return responseHelper.CreateErrorResponse(status, errMsg, runnableRequestHelper.GetCpuTime(), runnableRequestHelper.GetTotalTime(), runnableRequestHelper.GetQuota(), runnableRequestHelper.GetPrepareTime());
    };

    auto ecsql = QueryHelper::FormatQuery(request->GetQuery().c_str());
    auto const hashCode = ECSqlStatement::GetHashCode(ecsql.c_str());
    if(!m_adaptor.GetStatement().IsPrepared() || m_adaptor.GetStatement().GetHashCode() != hashCode || strcmp(m_adaptor.GetStatement().GetECSql(), ecsql.c_str()) != 0) {
        std::string error;
        if (!PrepareStmt(ecsql, request->GetSuppressLogErrors(), error)) { // Prepare statement if not prepared or the prepared statement is different from the current request's ecsql
            recordPrepareTime();
            return setError(SqlPrintfString("failed to prepare ecsql: %s. error: %s", ecsql.c_str(), error.c_str()).GetUtf8CP(), QueryResponse::Status::Error_ECSql_PreparedFailed);
        }
        if (!BindParams(request->GetArgs(), request->GetLimit(), error)) { // Bind parameters
            recordPrepareTime();
            return setError(SqlPrintfString("failed to bind params to ecsql: %s. error: %s", ecsql.c_str(), error.c_str()).GetUtf8CP(), QueryResponse::Status::Error_ECSql_BindingFailed);
        }
        recordPrepareTime();
    }
    if(!(m_args == request->GetArgs() && m_limit == request->GetLimit())) {
        std::string error;
        m_adaptor.GetStatement().Reset();
        m_adaptor.GetStatement().ClearBindings();
        if (!BindParams(request->GetArgs(), request->GetLimit(), error)) { // Bind parameters
            recordPrepareTime();
            return setError(SqlPrintfString("failed to bind params to ecsql: %s. error: %s", ecsql.c_str(), error.c_str()).GetUtf8CP(), QueryResponse::Status::Error_ECSql_BindingFailed);
        }
        recordPrepareTime();
    }
    return Execute(std::move(request), runnableRequestHelper);

}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ECSqlRowReader::Impl::BindParams(ECSqlParams const& params,  QueryLimit const& limit, std::string& error) {
    
    if(!params.TryBindTo(m_adaptor.GetStatement(), error)) {
        log_error("failed to bind parameters. error: %s", error.c_str());
        return false;
    }
    QueryHelper::BindLimits(m_adaptor.GetStatement(), limit);
    m_args = params;
    m_limit = limit;
    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ECSqlRowReader::Impl::PrepareStmt(std:: string const& ecsql,  bool suppressLogError, std::string& ecsql_error) {
    ErrorListenerScope err_scope(const_cast<ECDb&>(m_ecdb));
    
    ECSqlStatus status = m_adaptor.GetStatement().Prepare(m_ecdb, ecsql.c_str(), !suppressLogError);

    if (status != ECSqlStatus::Success) {
        ecsql_error = err_scope.GetLastError();
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
QueryResponse::Ptr ECSqlRowReader::Impl::Execute(ECSqlRequest::Ptr request, RunnableRequestStatsHelper& runnableRequestHelper) {
    CreateResponseHelper responseHelper;
    enum class status { partial, done };
    const auto abbreviateBlobs = request->GetAbbreviateBlobs();
    const auto includeMetaData= request->GetIncludeMetaData();
    const auto classIdToClassNames = request->GetConvertClassIdsToClassNames();
    const auto doNotConvertClassIdsToClassNamesWhenAliased = request->GetDoNotConvertClassIdsToClassNamesWhenAliased();
    auto& stmt = m_adaptor.GetStatement();
    auto& adaptor = m_adaptor.GetJsonAdaptor();
    auto& options = adaptor.GetOptions();
    options.SetAbbreviateBlobs(abbreviateBlobs);
    options.SetConvertClassIdsToClassNames(classIdToClassNames);
    options.SetUseJsNames(request->GetValueFormat() == ECSqlRequest::ECSqlValueFormat::JsNames);
    options.SetDoNotConvertClassIdsToClassNamesWhenAliased(doNotConvertClassIdsToClassNamesWhenAliased);
    ECSqlRowProperty::List props;
    if (includeMetaData) {
        adaptor.GetMetaData(props ,stmt);
    }
    uint32_t row_count = 0;
    std::string& result = m_adaptor.ClearAndGetCachedString();
    result.reserve(QUERY_WORKER_RESULT_RESERVE_BYTES);
    result.append("[");
    auto setResult = [&](status st) {
        result.append("]");

        if (m_ecdb.IsDbOpen())
            m_ecdb.SetProgressHandler(nullptr);
        return responseHelper.CreateECSqlResponse(result, props, row_count, st == status::done, runnableRequestHelper.GetCpuTime(), runnableRequestHelper.GetTotalTime(), runnableRequestHelper.GetQuota(), runnableRequestHelper.GetPrepareTime());
    };
    auto setError = [&] (QueryResponse::Status status, std::string err) {
        log_error("%s. (%s)", err.c_str(), QueryResponse::StatusToString(status));
        return responseHelper.CreateErrorResponse(status, err, runnableRequestHelper.GetCpuTime(), runnableRequestHelper.GetTotalTime(), runnableRequestHelper.GetQuota(), runnableRequestHelper.GetPrepareTime());
    };

    if(m_ecdb.IsDbOpen()) {
        m_ecdb.SetProgressHandler([&](){
            if (runnableRequestHelper.IsTimeExceeded()) {
                log_trace("%s time exceeded for sync query", GetTimestamp().c_str());
                return DbProgressAction::Interrupt;
            }
            return DbProgressAction::Continue;
        }, 1);
    }
    // go over each row and serialize result
    auto rc = stmt.Step();
    if (rc == BE_SQLITE_ROW) {
        auto& rowsDoc = m_adaptor.ClearAndGetCachedJsonDocument();
        BeJsValue rows(rowsDoc);
        if (adaptor.RenderRowAsArray(rows, ECSqlStatementRow(stmt)) != SUCCESS) {
            return setError(QueryResponse::Status::Error_ECSql_RowToJsonFailed, "failed to serialize ecsql statement row to json");
        } else {
            row_count = row_count + 1;
            result.append(rows.Stringify());
        }
        if (runnableRequestHelper.IsTimeOrMemoryExceeded(result)) {
            log_trace("%s time or memory exceeded for sync request",GetTimestamp().c_str());
            return setResult(status::partial);
        }
        return setResult(status::done);
    }
    else if(rc == BE_SQLITE_DONE) {
        return setResult(status::done);
    }
    else if (rc == BE_SQLITE_INTERRUPT || rc == BE_SQLITE_BUSY) {
        return setResult(status::partial);
    } else {
        DbResult lastError;
        std::string sqlStepError = m_ecdb.GetLastError(&lastError);
        if (lastError != BE_SQLITE_OK) {
            return setError(QueryResponse::Status::Error_ECSql_StepFailed, SqlPrintfString("concurrent query step() failed: %s", sqlStepError.c_str()).GetUtf8CP());
        }
        return setError(QueryResponse::Status::Error_ECSql_StepFailed, "concurrent query step() failed");
    }
}

END_BENTLEY_SQLITE_EC_NAMESPACE
