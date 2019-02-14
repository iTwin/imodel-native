/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStatementCache.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    BeMutexHolder lock(m_cache.m_mutex);

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
#ifndef NDEBUG
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
#endif //NDEBUG

//*************************************************************************************
// ECSqlStatementCache
//*************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2015
//---------------------------------------------------------------------------------------
ECSqlStatementCache::ECSqlStatementCache(uint32_t maxSize, Utf8CP name)
: m_maxSize(std::max<uint32_t> (maxSize, 1)), //a size of 0 doesn't make sense, so move it to the minimum size of 1
  m_name(name)
    {
    BeAssert(m_maxSize > 0);
#ifndef NDEBUG
    ECSqlStatementCacheDiagnostics::Log(GetName(), m_maxSize, ECSqlStatementCacheDiagnostics::EventType::CreatedCache, nullptr);
#endif //NDEBUG
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2015
//---------------------------------------------------------------------------------------
CachedECSqlStatementPtr ECSqlStatementCache::GetPreparedStatement(ECDbCR ecdb, Utf8CP ecsql, ECCrudWriteToken const* token) const
    {
    CachedECSqlStatementPtr stmt;
    GetPreparedStatement(stmt, ecdb, nullptr, token, ecsql);
    return stmt;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Affan.Khan      01/2018
//---------------------------------------------------------------------------------------
CachedECSqlStatementPtr ECSqlStatementCache::GetPreparedStatement(SchemaManagerCR schemaManager, DbCR datasource, Utf8CP ecsql) const
    {
    CachedECSqlStatementPtr stmt;
    GetPreparedStatement(stmt, schemaManager.GetDispatcher().Main().GetECDb(), &datasource, nullptr, ecsql);
    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlStatementCache::GetPreparedStatement(CachedECSqlStatementPtr& stmt, ECDbCR ecdb, DbCP datasource, ECCrudWriteToken const* token, Utf8CP ecsql) const
    {
    BeMutexHolder _v_v(m_mutex);

    stmt = FindEntry(ecdb, datasource, token, ecsql);
    if (stmt.IsValid())
        return;

    AddStatement(stmt, ecdb, datasource, token, ecsql);

    if (datasource == nullptr)
        {
        if (ECSqlStatus::Success != stmt->Prepare(ecdb, ecsql, token))
            stmt = nullptr;
        }
    else
        {
        BeAssert(token == nullptr);
        if (ECSqlStatus::Success != stmt->Prepare(ecdb.Schemas(), *datasource, ecsql))
            stmt = nullptr;
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2015
//---------------------------------------------------------------------------------------
CachedECSqlStatement* ECSqlStatementCache::FindEntry(ECDbCR ecdb, Db const* datasourceECDb, ECCrudWriteToken const* token, Utf8CP ecsql) const
    {
    std::list<CachedECSqlStatementPtr>::iterator foundIt = m_entries.end();
    for (auto it = m_entries.begin(), end = m_entries.end(); it != end; ++it)
        {
        CachedECSqlStatementPtr& stmt = *it;
        //ECSqlStatement::GetECSql returns nullptr if stmt is not prepared, so don't compare ECSQL string if not prepared
        if (stmt->IsPrepared() && 0 == strcmp(stmt->GetECSql(), ecsql) && &stmt->m_ecdb == &ecdb && stmt->m_dataSourceECDb == datasourceECDb && stmt->m_crudWriteToken == token)
            {
            // if statement > 1, the statement is currently in use, we can't share it
            if (stmt->GetRefCount() <= 1)
                {
#ifndef NDEBUG
                ECSqlStatementCacheDiagnostics::Log(GetName(), m_maxSize, ECSqlStatementCacheDiagnostics::EventType::GotFromCache, ecsql);
#endif //NDEBUG
                foundIt = it;
                break;
                }
            }
        }

    if (foundIt == m_entries.end())
        return nullptr;

    m_entries.splice(m_entries.begin(), m_entries, foundIt); // move this most-recently-accessed statement to front 
    return foundIt->get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2015
//---------------------------------------------------------------------------------------
void ECSqlStatementCache::AddStatement(CachedECSqlStatementPtr& newEntry, ECDbCR ecdb, DbCP datasource, ECCrudWriteToken const* token, Utf8CP ecsql) const
    {
    BeMutexHolder _v_v(m_mutex);
    if (((uint32_t) m_entries.size()) >= m_maxSize) // if cache is full, remove oldest entry
        {
        CachedECSqlStatementPtr& last = m_entries.back();
#ifndef NDEBUG        
        ECSqlStatementCacheDiagnostics::Log(GetName(), m_maxSize,  ECSqlStatementCacheDiagnostics::EventType::RemovedFromCache, last->GetECSql());
#endif //NDEBUG
        last->m_isInCache = false; // this statement is no longer managed by this cache, don't let Release method call Reset/ClearBindings anymore
        m_entries.pop_back();
        }

    newEntry = new CachedECSqlStatement(*this, ecdb, datasource, token);
    m_entries.push_front(newEntry);
#ifndef NDEBUG
    ECSqlStatementCacheDiagnostics::Log(GetName(), m_maxSize, ECSqlStatementCacheDiagnostics::EventType::AddedToCache, ecsql);
#endif //NDEBUG
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2015
//---------------------------------------------------------------------------------------
void ECSqlStatementCache::Empty()
    {
    BeMutexHolder lock(m_mutex);
    for (CachedECSqlStatementPtr& stmt : m_entries)
        {
        stmt->m_isInCache = false;
        }

    m_entries.clear();
#ifndef NDEBUG
    ECSqlStatementCacheDiagnostics::Log(GetName(), m_maxSize, ECSqlStatementCacheDiagnostics::EventType::ClearedCache, nullptr);
#endif //NDEBUG
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

#ifndef NDEBUG
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
#endif //NDEBUG

END_BENTLEY_SQLITE_EC_NAMESPACE
