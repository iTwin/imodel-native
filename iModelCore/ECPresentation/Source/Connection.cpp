/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/Connection.h>
#include <Bentley/BeThreadLocalStorage.h>
#include <Bentley/BeThread.h>

#define STATEMENT_CACHE_SIZE 50

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ConnectionEvent::AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator) const
    {
    return ECPresentationManager::GetSerializer().AsJson(ctx, *this, allocator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ConnectionEvent::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    ECPresentationSerializerContext ctx;
    return AsJson(ctx, allocator);
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NotifyingAppData : Db::AppData
{
private:
    BeSQLite::Db::AppData::Key m_key;
    std::function<void()> m_onDestroy;

private:
    NotifyingAppData(std::function<void()> onDestroy) : m_onDestroy(onDestroy) {}

public:
    static RefCountedPtr<NotifyingAppData> Create(std::function<void()> onDestroy) {return new NotifyingAppData(onDestroy);}
    ~NotifyingAppData() {m_onDestroy();}
    BeSQLite::Db::AppData::Key const& GetKey() const {return m_key;}
};

struct ProxyConnection;
//=======================================================================================
//! A connection for the specified ECDb. Tracks for when the ECDb is closed and notifies
//! ConnectionManager.
// @bsiclass
//=======================================================================================
struct PrimaryConnection : RefCounted<IConnection>
{
private:
    ConnectionManager& m_manager;
    Utf8String m_id;
    ECDbR m_ecdb;
    ECSqlStatementCache m_statementCache;
    BeEventScope m_eventsScope;
    bool m_isOpen;
    int m_isSuspended;
    Db::AppData::Key const* m_connectionClosedNotifierKey;
    bset<ProxyConnection*> m_proxyConnections;

private:
    PrimaryConnection(ConnectionManager& manager, Utf8String id, ECDbR db)
        : m_manager(manager), m_id(id), m_ecdb(db), m_statementCache(STATEMENT_CACHE_SIZE, "ECPresentation: PrimaryConnection"), m_isSuspended(0)
        {
        auto onCloseNotifier = NotifyingAppData::Create(std::bind(&PrimaryConnection::OnConnectionClosed, this));
        m_connectionClosedNotifierKey = &onCloseNotifier->GetKey();
        m_ecdb.AddAppData(onCloseNotifier->GetKey(), onCloseNotifier.get());

        m_ecdb.Schemas().OnBeforeSchemaChanges().AddListener(m_eventsScope, [this](ECDbCR, SchemaChangeType){OnBeforeSchemaChanges();});
        m_ecdb.Schemas().OnAfterSchemaChanges().AddListener(m_eventsScope, [this](ECDbCR, SchemaChangeType){OnAfterSchemaChanges();});

        m_isOpen = m_ecdb.IsDbOpen();
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Connections, LOG_INFO, Utf8PrintfString("%p PrimaryConnection[%s] created on thread %" PRIu64, this, m_id.c_str(), (uint64_t)BeThreadUtilities::GetCurrentThreadId()));
        }
    void OnConnectionClosed();
    void OnBeforeSchemaChanges();
    void OnAfterSchemaChanges();

protected:
    Utf8StringCR _GetId() const override {return m_id;}
    ECDbR _GetECDb() const override {return m_ecdb;}
    BeSQLite::Db& _GetDb() const override {return m_ecdb;}
    ECSqlStatementCache const& _GetStatementCache() const override {return m_statementCache;}
    bool _IsOpen() const override {return m_isOpen;}
    bool _IsReadOnly() const override {return m_ecdb.IsReadonly();}
    void _InterruptRequests() const override;
    void _Reset() override;

public:
    static RefCountedPtr<PrimaryConnection> Create(ConnectionManager& manager, Utf8String id, ECDbR db)
        {
        RefCountedPtr<PrimaryConnection> connection = new PrimaryConnection(manager, id, db);
        return connection->IsOpen() ? connection : nullptr;
        }
    ~PrimaryConnection()
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Connections, LOG_INFO, Utf8PrintfString("%p PrimaryConnection[%s] destroyed on thread %" PRIu64, this, m_id.c_str(), (uint64_t)BeThreadUtilities::GetCurrentThreadId()));
        Close();
        }
    ConnectionManager const& GetManager() const {return m_manager;}
    void NotifyProxyConnectionOpened(ProxyConnection& proxy);
    void NotifyProxyConnectionClosed(ProxyConnection& proxy);
    void Close()
        {
        if (!m_isOpen)
            return;

        m_ecdb.DropAppData(*m_connectionClosedNotifierKey);
        }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct IProxyConnectionsTracker
    {
    virtual ~IProxyConnectionsTracker() {}
    virtual void OnProxyConnectionAcquire(struct ProxyConnection&) = 0;
    virtual void OnProxyConnectionRelease(struct ProxyConnection&) = 0;
    };

//=======================================================================================
//! A thread-local connection that's based on the supplied primary connection. It notifies
//! supplied IProxyConnectionsTracker so it can track when ProxyConnections are/aren't
//! being used.
// @bsiclass
//=======================================================================================
struct ProxyConnection : IConnection
{
private:
    IProxyConnectionsTracker* m_tracker;
    PrimaryConnection const& m_primaryConnection;
    mutable BeSQLite::Db m_db;
    mutable BeAtomic<uint32_t> m_refCount;
    mutable BeAtomic<int> m_disableThreadVerification;
    uint64_t m_threadId;
    ECSqlStatementCache m_statementCache;

private:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    ProxyConnection(IProxyConnectionsTracker* tracker, PrimaryConnection const& primaryConnection)
        : m_tracker(tracker), m_primaryConnection(primaryConnection), m_threadId((uint64_t)BeThreadUtilities::GetCurrentThreadId()), m_disableThreadVerification(0),
        m_statementCache(STATEMENT_CACHE_SIZE, Utf8PrintfString("ECPresentation: ProxyConnection(%" PRIu64 ")", m_threadId).c_str())
        {
        // By default we're going to retry 600 times, waiting 100 ms before each retry. This
        // gives us a total of 1 minute before we fail the query with BE_SQLITE_BUSY.
        auto const& busyTimeout = primaryConnection.GetManager().GetProps().GetBusyTimeout();
        uint64_t retriesCount = busyTimeout.IsValid() ? (busyTimeout.Value() / 100) : 600;
        RefCountedPtr<BeSQLite::BusyRetry> busyRetry = new BusyRetry(retriesCount, 100);

        DbResult openResult = m_primaryConnection.GetDb().OpenSecondaryConnection(m_db, Db::OpenParams(Db::OpenMode::Readonly, DefaultTxn::No, busyRetry.get()));
        if (BE_SQLITE_OK != openResult)
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Connections, Utf8PrintfString("Failed to open proxy SQLite connection with result `%s`", BeSQLite::Db::InterpretDbResult(openResult)));

        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Connections, LOG_INFO, Utf8PrintfString("%p ProxyConnection[%s] created on thread " PRIu64, this, m_primaryConnection.GetId().c_str(), m_threadId));

        uint64_t mmapSize = 0;
        if (primaryConnection.GetManager().GetProps().GetMmapFileSize().IsNull())
            {
            BeFileNameStatus fileSizeResult = BeFileName(m_db.GetDbFileName()).GetFileSize(mmapSize);
            if (BeFileNameStatus::Success != fileSizeResult)
                DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Connections, false, Utf8PrintfString("%p ProxyConnection[%s] failed to get file size, result = %d", this, m_primaryConnection.GetId().c_str(), (int)fileSizeResult));
            }
        else
            {
            mmapSize = primaryConnection.GetManager().GetProps().GetMmapFileSize().Value();
            }
        if (0 != mmapSize)
            {
            Savepoint txn(m_db, "set mmap_size");
            DbResult mmapResult = m_db.ExecuteSql(Utf8PrintfString("PRAGMA mmap_size = %" PRIu64, mmapSize).c_str());
            if (BE_SQLITE_OK == mmapResult)
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Connections, LOG_INFO, Utf8PrintfString("%p ProxyConnection[%s] set mmap_size = %" PRIu64, this, m_primaryConnection.GetId().c_str(), mmapSize))
            else
                DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Connections, false, Utf8PrintfString("Failed to set mmap_size = %" PRIu64 " with result `%s`" , mmapSize, BeSQLite::Db::InterpretDbResult(mmapResult)));
            }
        else
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Connections, LOG_INFO, Utf8PrintfString("%p ProxyConnection[%s] not using mmap", this, m_primaryConnection.GetId().c_str()));
            }

        auto const& memoryCacheSize = primaryConnection.GetManager().GetProps().GetMemoryCacheSize();
        if (!memoryCacheSize.IsNull())
            {
            Savepoint savepoint(m_db, "Set memory cache size");
            uint64_t cacheSizeInKb = memoryCacheSize.Value() / 1024;
            m_db.ExecuteSql(Utf8PrintfString("PRAGMA cache_size=-%" PRIu64, cacheSizeInKb).c_str());
            }
        }
    void ValidateCurrentThread() const
        {
        if (!IsOpen() || m_disableThreadVerification.load() > 0)
            {
            // allow accessing from a different thread when closing
            return;
            }
        if ((uint64_t)BeThreadUtilities::GetCurrentThreadId() != m_threadId)
            {
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Connections, Utf8PrintfString("%p ProxyConnection[%s] used on wrong thread. Expected: %" PRIu64 ". Got: %" PRIu64 ".",
                this, m_primaryConnection.GetId().c_str(), m_threadId, (uint64_t)BeThreadUtilities::GetCurrentThreadId()));
            }
        }

protected:
    Utf8StringCR _GetId() const override {return m_primaryConnection.GetId();}
    ECDbR _GetECDb() const override {ValidateCurrentThread(); return m_primaryConnection.GetECDb();}
    BeSQLite::Db& _GetDb() const override {ValidateCurrentThread(); return m_db;}
    ECSqlStatementCache const& _GetStatementCache() const override {ValidateCurrentThread(); return m_statementCache;}
    bool _IsOpen() const override {return m_primaryConnection.IsOpen() && m_db.IsDbOpen();}
    bool _IsReadOnly() const override {return m_primaryConnection.IsReadOnly();}
    void _InterruptRequests() const override {ValidateCurrentThread(); m_db.Interrupt();}
    void _Reset() override {m_statementCache.Empty();}

public:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    static RefCountedPtr<ProxyConnection> Create(IProxyConnectionsTracker* tracker, PrimaryConnection const& primaryConnection)
        {
        RefCountedPtr<ProxyConnection> connection = new ProxyConnection(tracker, primaryConnection);
        return connection->IsOpen() ? connection : nullptr;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    ~ProxyConnection()
        {
        m_statementCache.Empty();
        m_db.CloseDb();
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Connections, LOG_INFO, Utf8PrintfString("%p ProxyConnection[%s] closed on thread %" PRIu64, this, GetId().c_str(), (uint64_t)BeThreadUtilities::GetCurrentThreadId()));
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    void SetTracker(IProxyConnectionsTracker* tracker) {m_tracker = tracker;}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    uint32_t AddRef() const override
        {
        auto result = m_refCount.IncrementAtomicPre(std::memory_order_relaxed);
        if (nullptr != m_tracker)
            m_tracker->OnProxyConnectionAcquire(const_cast<ProxyConnection&>(*this));
        return result;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    uint32_t Release() const override
        {
        uint32_t countWas = m_refCount.DecrementAtomicPost(std::memory_order_release);
        REFCOUNT_RELEASE_CHECK(countWas);
        if (nullptr != m_tracker)
            m_tracker->OnProxyConnectionRelease(const_cast<ProxyConnection&>(*this));
        if (1 == countWas)
            {
            std::atomic_thread_fence(std::memory_order_acquire);
            delete this;
            }
        return countWas - 1;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    uint32_t GetRefCount() const {return m_refCount.load();}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    void SetDisableThreadVerification(bool shouldDisable) const
        {
        if (shouldDisable)
            m_disableThreadVerification++;
        else
            m_disableThreadVerification--;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DisableProxyConnectionThreadVerification::DisableProxyConnectionThreadVerification(IConnectionCR connection)
    : m_connection(nullptr)
    {
    if (nullptr != dynamic_cast<ProxyConnection const*>(&connection))
        {
        m_connection = &connection;
        static_cast<ProxyConnection const*>(m_connection)->SetDisableThreadVerification(true);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DisableProxyConnectionThreadVerification::~DisableProxyConnectionThreadVerification()
    {
    if (nullptr != m_connection)
        static_cast<ProxyConnection const*>(m_connection)->SetDisableThreadVerification(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimaryConnection::_InterruptRequests() const
    {
    for (ProxyConnection const* proxy : m_proxyConnections)
        {
        DisableProxyConnectionThreadVerification noThreadVerification(*proxy);
        proxy->InterruptRequests();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimaryConnection::_Reset()
    {
    for (ProxyConnection* proxy : m_proxyConnections)
        proxy->Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimaryConnection::OnConnectionClosed()
    {
    if (!m_isOpen)
        return;

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Connections, LOG_INFO, Utf8PrintfString("%p PrimaryConnection[%s] closed on thread %" PRIu64, this, m_id.c_str(), (uint64_t)BeThreadUtilities::GetCurrentThreadId()));

    IConnectionPtr ref = this; // keep refcount of this instance so it doesnt get destroyed after NotifyConnectionClosed call
    m_statementCache.Empty();
    m_eventsScope.CancelAll();
    m_isOpen = false;
    m_manager.NotifyConnectionClosed(m_id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimaryConnection::OnBeforeSchemaChanges()
    {
    if (!m_isOpen)
        return;

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Connections, LOG_INFO, Utf8PrintfString("%p PrimaryConnection[%s] started schema changes on thread %" PRIu64, this, m_id.c_str(), (uint64_t)BeThreadUtilities::GetCurrentThreadId()));
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Connections, 0 == m_isSuspended, Utf8PrintfString("Expected `m_isSuspended == 0`, but got %d", m_isSuspended));
    ++m_isSuspended;
    m_manager.NotifyConnectionSuspended(m_id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimaryConnection::OnAfterSchemaChanges()
    {
    if (!m_isOpen)
        return;

    DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Connections, LOG_INFO, Utf8PrintfString("%p PrimaryConnection[%s] finished schema changes on thread %" PRIu64, this, m_id.c_str(), (uint64_t)BeThreadUtilities::GetCurrentThreadId()));
    DIAGNOSTICS_ASSERT_SOFT(DiagnosticsCategory::Connections, 1 == m_isSuspended, Utf8PrintfString("Expected `m_isSuspended == 1`, but got %d", m_isSuspended));
    --m_isSuspended;
    m_manager.NotifyConnectionResumed(m_id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimaryConnection::NotifyProxyConnectionOpened(ProxyConnection& proxy)
    {
    m_proxyConnections.insert(&proxy);
    m_manager.NotifyProxyConnectionOpened(proxy);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimaryConnection::NotifyProxyConnectionClosed(ProxyConnection& proxy)
    {
    m_proxyConnections.erase(&proxy);
    }

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ThreadLocalConnectionStore : RefCountedBase, IProxyConnectionsTracker, protected BeThreadLocalStorage
{
    struct ReleasedProxyConnectionsPredicate : IConditionVariablePredicate
        {
        bvector<RefCountedPtr<ProxyConnection>> const& m_connections;
        ReleasedProxyConnectionsPredicate(bvector<RefCountedPtr<ProxyConnection>> const& connections)
            : m_connections(connections)
            {}
        bool _TestCondition(BeConditionVariable&) override
            {
            for (RefCountedPtr<ProxyConnection> const& connection : m_connections)
                {
                if (connection->GetRefCount() > 1)
                    return false;
                }
            return true;
            }
        };

private:
    RefCountedPtr<PrimaryConnection> m_primaryConnection;
    bvector<RefCountedPtr<ProxyConnection>> m_proxyConnections;
    BeConditionVariable m_proxyConnectionsCV;

public:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    ThreadLocalConnectionStore(PrimaryConnection& primaryConnection)
        : m_primaryConnection(&primaryConnection)
        {
        SetValueAsPointer(m_primaryConnection.get());
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    ~ThreadLocalConnectionStore()
        {
        ReleasedProxyConnectionsPredicate predicate(m_proxyConnections);
        m_proxyConnectionsCV.WaitOnCondition(&predicate, BeConditionVariable::Infinite);
        SetValueAsPointer(nullptr);

        // we get here when ThreadLocalConnectionStore is the only holder of
        // m_proxyConnections - need to stop tracking their release before
        // finishing store desctruction
        for (RefCountedPtr<ProxyConnection> const& proxy : m_proxyConnections)
            proxy->SetTracker(nullptr);
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    void OnProxyConnectionAcquire(ProxyConnection&) override {}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    void OnProxyConnectionRelease(ProxyConnection& proxy) override
        {
        if (0 == proxy.GetRefCount())
            m_primaryConnection->NotifyProxyConnectionClosed(proxy);
        m_proxyConnectionsCV.notify_all();
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    IConnection* GetConnection()
        {
        void* ptr = GetValueAsPointer();
        if (nullptr != ptr)
            return static_cast<IConnection*>(ptr);
        return nullptr;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    IConnectionP GetOrCreateConnection()
        {
        IConnection* connection = GetConnection();
        if (nullptr != connection)
            return connection;

        RefCountedPtr<ProxyConnection> proxy = ProxyConnection::Create(this, *m_primaryConnection);
        if (proxy.IsNull())
            return nullptr;

        m_proxyConnections.push_back(proxy);
        m_primaryConnection->NotifyProxyConnectionOpened(*proxy);
        SetValueAsPointer(proxy.get());
        return proxy.get();
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    PrimaryConnection const& GetPrimaryConnection() const {return *m_primaryConnection;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ConnectionManager::ConnectionsStore
{
private:
    bmap<Utf8String, RefCountedPtr<ThreadLocalConnectionStore>> m_connections;

public:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    bvector<Utf8String> GetIds() const
        {
        bvector<Utf8String> ids;
        for (auto const& entry : m_connections)
            ids.push_back(entry.first);
        return ids;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    void Add(PrimaryConnection& connection)
        {
        if (m_connections.end() != m_connections.find(connection.GetId()))
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Connections, Utf8PrintfString("Trying to register connection, but connection with equal ID already exists: %s", connection.GetId().c_str()));

        m_connections[connection.GetId()] = new ThreadLocalConnectionStore(connection);
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    RefCountedPtr<ThreadLocalConnectionStore> Remove(Utf8StringCR connectionId)
        {
        auto iter = m_connections.find(connectionId);
        if (m_connections.end() == iter)
            return nullptr;

        auto store = iter->second;
        m_connections.erase(iter);
        return store;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    void Clear() {m_connections.clear();}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    IConnection* Get(Utf8StringCR id)
        {
        auto iter = m_connections.find(id);
        if (m_connections.end() == iter)
            return nullptr;

        return iter->second->GetOrCreateConnection();
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+-----------+------*/
    IConnection* Get(ECDbCR ecdb)
        {
        for (auto pair : m_connections)
            {
            BeSQLite::Db const* primaryDb = &pair.second->GetPrimaryConnection().GetDb();
            BeSQLite::Db const* proxyDb = pair.second->GetConnection() ? &pair.second->GetConnection()->GetDb() : nullptr;
            if (primaryDb == &ecdb || proxyDb == &ecdb)
                return pair.second->GetOrCreateConnection();
            }
        return nullptr;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectionManager::ConnectionManager(Props props)
    : m_activeConnections(new ConnectionsStore()), m_primaryThreadId(BeThreadUtilities::GetCurrentThreadId()), m_props(props)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectionManager::~ConnectionManager()
    {
    CloseConnections();
    DELETE_AND_CLEAR(m_activeConnections);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectionManager::_AddListener(IConnectionsListener& listener) const
    {
    BeMutexHolder lock(m_listenersMutex);
    auto iter = m_listeners.find(listener.GetPriority());
    if (m_listeners.end() == iter)
        iter = m_listeners.Insert(listener.GetPriority(), bset<IConnectionsListener*>()).first;
    iter->second.insert(&listener);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectionManager::_DropListener(IConnectionsListener& listener) const
    {
    BeMutexHolder lock(m_listenersMutex);
    auto iter = m_listeners.find(listener.GetPriority());
    if (m_listeners.end() != iter)
        iter->second.erase(&listener);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectionManager::BroadcastEvent(ConnectionEvent const& evt) const
    {
    BeMutexHolder lock(m_listenersMutex);
    bmap<int, bset<IConnectionsListener*>> listeners = m_listeners;
    lock.unlock();

    for (auto entry : listeners)
        {
        for (IConnectionsListener* listener : entry.second)
            listener->NotifyConnectionEvent(evt);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IConnection* ConnectionManager::_GetConnection(Utf8CP connectionId) const
    {
    BeMutexHolder lock(m_connectionsMutex);
    return m_activeConnections->Get(connectionId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IConnection* ConnectionManager::_GetConnection(ECDbCR ecdb) const
    {
    BeMutexHolder lock(m_connectionsMutex);
    return m_activeConnections->Get(ecdb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectionPtr ConnectionManager::GetOrCreateConnection(ECDbR ecdb)
    {
    Utf8PrintfString connectionId("%" PRIu64 ":%s:%s", (uint64_t)&ecdb, ecdb.GetDbGuid().ToString().c_str(), ecdb.GetDbFileName());

    BeMutexHolder lock(m_connectionsMutex);
    IConnectionPtr connection = m_activeConnections->Get(connectionId);
    if (connection.IsValid())
        return connection;

    RefCountedPtr<PrimaryConnection> primaryConnection = PrimaryConnection::Create(*this, connectionId, ecdb);
    if (primaryConnection.IsNull())
        return nullptr;

    m_activeConnections->Add(*primaryConnection);
    lock.unlock();

    BroadcastEvent(ConnectionEvent(*primaryConnection, ConnectionEventType::Opened));

    return primaryConnection;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectionPtr ConnectionManager::_CreateConnection(ECDbR ecdb)
    {
    return GetOrCreateConnection(ecdb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectionManager::_CloseConnections()
    {
    BeMutexHolder lock(m_connectionsMutex);
    auto connectionIds = m_activeConnections->GetIds();
    lock.unlock();
    for (Utf8StringCR connectionId : connectionIds)
        {
        IConnection* connection = GetConnection(connectionId.c_str());
        PrimaryConnection* primaryConnection = dynamic_cast<PrimaryConnection*>(connection);
        if (primaryConnection)
            primaryConnection->Close();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectionManager::NotifyConnectionOpened(ECDbR connection)
    {
    GetOrCreateConnection(connection);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectionManager::NotifyConnectionClosed(Utf8StringCR connectionId)
    {
    BeMutexHolder lock(m_connectionsMutex);
    IConnectionPtr primaryConnection = m_activeConnections->Get(connectionId);
    if (primaryConnection.IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Connections, Utf8PrintfString("Got notification about closed connection, but it's not registered: %s", connectionId.c_str()));

    lock.unlock();

    // broadcast the event with the connections still available
    BroadcastEvent(ConnectionEvent(*primaryConnection, ConnectionEventType::Closed));

    lock.lock();
    // remove the store from m_activeConnections within a mutex, but release it outside
    // note: destruction of ThreadLocalConnectionStore will block this thread until all
    // proxy connections are finished being used on other threads
    RefCountedPtr<ThreadLocalConnectionStore> tempStore = m_activeConnections->Remove(connectionId);
    lock.unlock();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectionManager::NotifyProxyConnectionOpened(IConnectionCR connection)
    {
    BroadcastEvent(ConnectionEvent(connection, ConnectionEventType::ProxyCreated));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectionManager::NotifyConnectionSuspended(Utf8StringCR connectionId)
    {
    BeMutexHolder lock(m_connectionsMutex);
    IConnectionPtr primaryConnection = m_activeConnections->Get(connectionId);
    if (primaryConnection.IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Connections, Utf8PrintfString("Got notification about suspended connection, but it's not registered: %s", connectionId.c_str()));

    lock.unlock();

    // broadcast the event with the connections still available
    BroadcastEvent(ConnectionEvent(*primaryConnection, ConnectionEventType::Suspended));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectionManager::NotifyConnectionResumed(Utf8StringCR connectionId)
    {
    BeMutexHolder lock(m_connectionsMutex);
    IConnectionPtr primaryConnection = m_activeConnections->Get(connectionId);
    if (primaryConnection.IsNull())
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Connections, Utf8PrintfString("Got notification about resumed connection, but it's not registered: %s", connectionId.c_str()));

    // reset the connection to clear statement cache
    primaryConnection->Reset();

    lock.unlock();

    // broadcast the event with the connections still available
    BroadcastEvent(ConnectionEvent(*primaryConnection, ConnectionEventType::Resumed));
    }
