/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStatementCache.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*************************************************************************************
// CachedECSqlStatement
//*************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2015
//---------------------------------------------------------------------------------------
uint32_t CachedECSqlStatement::Release()
    {
    // Since statements can be referenced from multiple threads, and since we want to reset the statement 
    // when it is only held by the StatementCache, we need to hold the cache's mutex for the entire scope of this
    // method. However, the reference count member must still be atomic since we don't acquire the mutex for AddRef.
    BeDbMutexHolder holder(m_cache.m_mutex);

    const bool isInCache = m_isInCache; // hold this in a local before we decrement the refcount in case another thread deletes us
    const uint32_t countWas = m_refCount.DecrementAtomicPost();
    if (1 == countWas)
        {
        delete this;
        return 0;
        }

    // CachedECSqlStatements are always held in a cache, so the ref count will be 1 if no
    // one else is pointing to this instance. That means that the statement is no longer in use and
    // we should reset it so sqlite won't keep it in the list of active vdbe's. Also, clear its bindings so
    // the next user won't accidentally inherit them.
    if (isInCache && 2 == countWas && IsPrepared())
        {
        Reset(); // this is safe because we know this statement is now ONLY held by the cache.
        ClearBindings();
        }

    return countWas - 1;
    }

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      02/2015
//+===============+===============+===============+===============+===============+======
struct ECSqlStatementCacheDiagnostics
    {
    public:
        enum class EventType
            {
            AddedToCache,
            GotFromCache,
            RemovedFromCache,
            CreatedCache,
            ClearedCache
            };

    private:
        static const NativeLogging::SEVERITY LOG_SEVERITY = NativeLogging::LOG_TRACE;
        static BentleyApi::NativeLogging::ILogger* s_logger;

        ECSqlStatementCacheDiagnostics();
        ~ECSqlStatementCacheDiagnostics();

        static BentleyApi::NativeLogging::ILogger& GetLogger();

    public:
        static void Log(Utf8CP cacheName, uint32_t maxCacheSize, EventType, Utf8CP ecsql);
    };


//*************************************************************************************
// ECSqlStatementCache
//*************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2015
//---------------------------------------------------------------------------------------
ECSqlStatementCache::ECSqlStatementCache(uint32_t maxSize, Utf8CP name)
: m_mutex(BeDbMutex::MutexType::Recursive), m_maxSize(std::max<uint32_t> (maxSize, 1)), //a size of 0 doesn't make sense, so move it to the minimum size of 1
  m_name(name)
    {
    BeAssert(m_maxSize > 0);
    ECSqlStatementCacheDiagnostics::Log(GetName(), m_maxSize, ECSqlStatementCacheDiagnostics::EventType::CreatedCache, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2015
//---------------------------------------------------------------------------------------
CachedECSqlStatementPtr ECSqlStatementCache::GetPreparedStatement(ECDbCR ecdb, Utf8CP ecsql) const
    {
    CachedECSqlStatement* existingStmt = FindEntry(ecsql);
    if (existingStmt != nullptr)
        return existingStmt;

    CachedECSqlStatementPtr newStmt = nullptr;
    AddStatement(newStmt, ecdb, ecsql);

    if (ECSqlStatus::Success != newStmt->Prepare(ecdb, ecsql))
        return nullptr;

    return newStmt;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2015
//---------------------------------------------------------------------------------------
CachedECSqlStatement* ECSqlStatementCache::FindEntry(Utf8CP ecsql) const
    {
    for (CachedECSqlStatementPtr& stmt : m_entries)
        {
        //ECSqlStatement::GetECSql returns nullptr if stmt is not prepared, so don't compare ECSQL string if not prepared
        if (stmt->IsPrepared() && 0 == strcmp(stmt->GetECSql(), ecsql))
            {
            // if statement > 1, the statement is currently in use, we can't share it
            if (stmt->GetRefCount() <= 1)
                {
                ECSqlStatementCacheDiagnostics::Log(GetName(), m_maxSize, ECSqlStatementCacheDiagnostics::EventType::GotFromCache, ecsql);
                return stmt.get();
                }
            }
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2015
//---------------------------------------------------------------------------------------
void ECSqlStatementCache::AddStatement(CachedECSqlStatementPtr& newEntry, ECDbCR ecdb, Utf8CP ecsql) const
    {
    BeDbMutexHolder _v_v(m_mutex);

    if (((uint32_t) m_entries.size()) >= m_maxSize) // if cache is full, remove oldest entry
        {
        CachedECSqlStatementPtr& last = m_entries.back();
        ECSqlStatementCacheDiagnostics::Log(GetName(), m_maxSize,  ECSqlStatementCacheDiagnostics::EventType::RemovedFromCache, last->GetECSql());
        last->m_isInCache = false; // this statement is no longer managed by this cache, don't let Release method call Reset/ClearBindings anymore
        m_entries.pop_back();
        }

    newEntry = new CachedECSqlStatement(*this);
    m_entries.push_front(newEntry);
    ECSqlStatementCacheDiagnostics::Log(GetName(), m_maxSize, ECSqlStatementCacheDiagnostics::EventType::AddedToCache, ecsql);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2015
//---------------------------------------------------------------------------------------
void ECSqlStatementCache::Empty()
    {
    BeDbMutexHolder _v_v(m_mutex);
    for (CachedECSqlStatementPtr& stmt : m_entries)
        {
        stmt->m_isInCache = false;
        }

    m_entries.clear();
    ECSqlStatementCacheDiagnostics::Log(GetName(), m_maxSize, ECSqlStatementCacheDiagnostics::EventType::ClearedCache, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2015
//---------------------------------------------------------------------------------------
void ECSqlStatementCache::Log() const
    {
    LOG.debugv("%s (max size: %" PRIu32 ")", GetName(), m_maxSize);
    for (auto& stmt : m_entries)
        LOG.debugv("\t%s", stmt->GetECSql());
    }

//*************************************************************************************
// ECSqlStatementCacheDiagnostics
//*************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2015
//---------------------------------------------------------------------------------------
//static member initialization
BentleyApi::NativeLogging::ILogger* ECSqlStatementCacheDiagnostics::s_logger = nullptr;


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2015
//---------------------------------------------------------------------------------------
//static
void ECSqlStatementCacheDiagnostics::Log(Utf8CP cacheName, uint32_t maxCacheSize, EventType eventType, Utf8CP ecsql)
    {
    if (GetLogger().isSeverityEnabled(LOG_SEVERITY))
        {
        cacheName = !Utf8String::IsNullOrEmpty(cacheName) ? cacheName : "unnamed cache";
        Utf8CP eventStr = nullptr;
        switch (eventType)
            {
                case EventType::AddedToCache:
                    eventStr = "Added";
                    break;
                case EventType::RemovedFromCache:
                    eventStr = "Removed";
                    break;
                case EventType::GotFromCache:
                    eventStr = "Got";
                    break;
                case EventType::CreatedCache:
                    eventStr = "Created cache";
                    break;
                case EventType::ClearedCache:
                    eventStr = "Cleared cache";
                    break;
                default:
                    BeAssert(false && "Programmer error: Adjust ECSqlStatementCacheDiagnostics::Log to new value in EventType enum.");
                    break;
            }

        Utf8String message("[%s (max size: %" PRIu32 ")] %s");
        if (!Utf8String::IsNullOrEmpty(ecsql))
            message.append(" | ").append(ecsql);

        GetLogger().messagev(LOG_SEVERITY, message.c_str(), cacheName, maxCacheSize, eventStr);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2015
//---------------------------------------------------------------------------------------
//static
BentleyApi::NativeLogging::ILogger& ECSqlStatementCacheDiagnostics::GetLogger()
    {
    if (s_logger == nullptr)
        s_logger = NativeLogging::LoggingManager::GetLogger(L"Diagnostics.ECSqlStatement.Cache");

    return *s_logger;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
