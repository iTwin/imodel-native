/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStatementCache.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
uint32_t CachedECSqlStatement::Release ()
    {
    if (0 == --m_refCount)
        {
        delete this;
        return  0;
        }

    // CachedECSqlStatements are always held in a ECSqlStatementCache, so the ref count will be 1 if no
    // one else is pointing to this instance. That means that the statement is no longer in use and
    // we should reset it so sqlite won't keep it in the list of active vdbe's. Also, clear its bindings so
    // the next user won't accidentally inherit them.
    if (1 == m_refCount && IsPrepared ())
        {
        Reset ();
        ClearBindings ();
        }

    return m_refCount;
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

    ECSqlStatementCacheDiagnostics ();
    ~ECSqlStatementCacheDiagnostics ();

    static BentleyApi::NativeLogging::ILogger& GetLogger ();

public:
    static void Log (Utf8CP cacheName, size_t maxCacheSize, EventType, Utf8CP ecsql);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2015
//---------------------------------------------------------------------------------------
//static member initialization
BentleyApi::NativeLogging::ILogger* ECSqlStatementCacheDiagnostics::s_logger = nullptr;


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2015
//---------------------------------------------------------------------------------------
//static
void ECSqlStatementCacheDiagnostics::Log (Utf8CP cacheName, size_t maxCacheSize, EventType eventType, Utf8CP ecsql)
    {
    if (GetLogger ().isSeverityEnabled (LOG_SEVERITY))
        {
        cacheName = !Utf8String::IsNullOrEmpty (cacheName) ? cacheName : "unnamed cache";
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
                    BeAssert (false && "Programmer error: Adjust ECSqlStatementCacheDiagnostics::Log to new value in EventType enum.");
                    break;
            }

        Utf8String message ("[%s (max size: %d)] %s");
        if (!Utf8String::IsNullOrEmpty (ecsql))
            message.append (" | ").append (ecsql);

        GetLogger ().messagev (LOG_SEVERITY, message.c_str (), cacheName, maxCacheSize, eventStr);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    02/2015
//---------------------------------------------------------------------------------------
//static
BentleyApi::NativeLogging::ILogger& ECSqlStatementCacheDiagnostics::GetLogger ()
    {
    if (s_logger == nullptr)
        s_logger = NativeLogging::LoggingManager::GetLogger (L"Diagnostics.ECSqlStatement.Cache");

    return *s_logger;
    }

//*************************************************************************************
// ECSqlStatementCache
//*************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2015
//---------------------------------------------------------------------------------------
ECSqlStatementCache::ECSqlStatementCache (size_t maxSize, Utf8CP name)
: m_maxSize (std::max<size_t> (maxSize, 1)), //a size of 0 doesn't make sense, so move it to the minimum size of 1
  m_name (name)
    {
    BeAssert (m_maxSize > 0); m_entries.reserve (m_maxSize);
    ECSqlStatementCacheDiagnostics::Log (GetName (), m_maxSize, ECSqlStatementCacheDiagnostics::EventType::CreatedCache, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2015
//---------------------------------------------------------------------------------------
CachedECSqlStatementPtr ECSqlStatementCache::GetPreparedStatement (ECDbCR ecdb, Utf8CP ecsql) const
    {
    BeDbMutexHolder _v_v (m_mutex);
    CachedECSqlStatement* stmt = FindEntry (ecsql);
    if (stmt != nullptr)
        return stmt;

    return AddStatement (ecdb, ecsql);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2015
//---------------------------------------------------------------------------------------
CachedECSqlStatement* ECSqlStatementCache::FindEntry (Utf8CP ecsql) const
    {
    for (auto& stmt : m_entries)
        {
        if (0 == strcmp (stmt->GetECSql (), ecsql))
            {
            // if statement > 1, the statement is currently in use, we can't share it
            if (stmt->GetRefCount () <= 1)
                {
                ECSqlStatementCacheDiagnostics::Log (GetName (), m_maxSize, ECSqlStatementCacheDiagnostics::EventType::GotFromCache, ecsql);
                return stmt.get ();
                }
            }
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2015
//---------------------------------------------------------------------------------------
CachedECSqlStatement* ECSqlStatementCache::AddStatement (ECDbCR ecdb, Utf8CP ecsql) const
    {
    if (m_entries.size () >= m_maxSize) // if cache is full, remove oldest entry
        {
        auto first = m_entries.begin ();
        ECSqlStatementCacheDiagnostics::Log (GetName (), m_maxSize,  ECSqlStatementCacheDiagnostics::EventType::RemovedFromCache, (*first)->GetECSql ());
        m_entries.erase (first);
        }

    auto newEntry = new CachedECSqlStatement ();
    if (ECSqlStatus::Success != newEntry->Prepare (ecdb, ecsql))
        return nullptr;

    m_entries.push_back (newEntry);
    ECSqlStatementCacheDiagnostics::Log (GetName (), m_maxSize, ECSqlStatementCacheDiagnostics::EventType::AddedToCache, ecsql);
    return newEntry;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2015
//---------------------------------------------------------------------------------------
void ECSqlStatementCache::Empty ()
    {
    BeDbMutexHolder _v_v (m_mutex);
    m_entries.clear ();
    ECSqlStatementCacheDiagnostics::Log (GetName (), m_maxSize, ECSqlStatementCacheDiagnostics::EventType::ClearedCache, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      02/2015
//---------------------------------------------------------------------------------------
void ECSqlStatementCache::Log () const
    {
    LOG.debugv ("%s (max size: %d)", GetName (), m_maxSize);
    for (auto& stmt : m_entries)
        LOG.debugv ("\t%s", stmt->GetECSql ());
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
