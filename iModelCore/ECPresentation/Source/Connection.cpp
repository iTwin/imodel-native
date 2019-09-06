/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/Connection.h>
#include <Bentley/BeThreadLocalStorage.h>
#include <Bentley/BeThread.h>
#include "ECDbBasedCache.h"

#define LOG_CONNECTIONS (*LoggingManager::GetLogger(LOGGER_NAMESPACE_ECPRESENTATION_CONNECTIONS))

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ConnectionEvent::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    return IECPresentationManager::GetSerializer().AsJson(*this, allocator);
    }

struct ProxyConnection;
//=======================================================================================
//! A connection for the specified ECDb. Tracks for when the ECDb is closed and notifies
//! ConnectionManager.
// @bsiclass                                    Grigas.Petraitis                10/2017
//=======================================================================================
struct PrimaryConnection : RefCounted<IConnection>, IECDbClosedListener
{
private:
    ConnectionManager& m_manager;
    Utf8String m_id;
    ECDbR m_ecdb;
    bool m_isPrimary;
    bool m_isOpen;
    bset<ProxyConnection const*> m_proxyConnections;

private:
    PrimaryConnection(ConnectionManager& manager, Utf8String id, ECDbR db, bool isPrimary)
        : m_manager(manager), m_id(id), m_ecdb(db), m_isPrimary(isPrimary)
        {
        ECDbClosedNotifier::Register(*this, m_ecdb, true);
        m_isOpen = m_ecdb.IsDbOpen();
        LOG_CONNECTIONS.infov("%p PrimaryConnection[%s] created on thread %d", this, m_id.c_str(), (int)BeThreadUtilities::GetCurrentThreadId());
        }

protected:
    Utf8StringCR _GetId() const override {return m_id;}
    ECDbR _GetECDb() const override {return m_ecdb;}
    BeSQLite::Db& _GetDb() const override {return m_ecdb;}
    bool _IsOpen() const override {return m_isOpen;}
    bool _IsReadOnly() const override {return m_ecdb.IsReadonly();}
    int _GetPriority() const override {return 100;}
    void _InterruptRequests() const override;
    void _OnConnectionClosed(ECDbCR db) override;
    void _OnConnectionReloaded(ECDbCR db) override;

public:
    static RefCountedPtr<PrimaryConnection> Create(ConnectionManager& manager, Utf8String id, ECDbR db, bool isPrimary) {return new PrimaryConnection(manager, id, db, isPrimary);}
    void NotifyProxyConnectionOpened(ProxyConnection const& proxy) {m_proxyConnections.insert(&proxy);}
    void NotifyProxyConnectionClosed(ProxyConnection const& proxy) {m_proxyConnections.erase(&proxy);}
};

//=======================================================================================
// @bsiclass                                    Grigas.Petraitis                11/2017
//=======================================================================================
struct IProxyConnectionsTracker
    {
    virtual ~IProxyConnectionsTracker() {}
    virtual void OnProxyConnectionAcquire(struct ProxyConnection const&) = 0;
    virtual void OnProxyConnectionRelease(struct ProxyConnection const&) = 0;
    };

//=======================================================================================
//! A thread-local connection that's based on the supplied primary connection. It notifies
//! supplied IProxyConnectionsTracker so it can track when ProxyConnections are/aren't
//! being used.
// @bsiclass                                    Grigas.Petraitis                10/2017
//=======================================================================================
struct ProxyConnection : IConnection
{
    struct BusyRetry : BeSQLite::BusyRetry
        {
        int _OnBusy(int count) const override
            {
            // just repeat indefinitely...
            BeThreadUtilities::BeSleep(1);
            return 1;
            }
        };

private:
    IProxyConnectionsTracker* m_tracker;
    PrimaryConnection const& m_primaryConnection;
    mutable BeSQLite::Db m_db;
    mutable BeAtomic<uint32_t> m_refCount;

private:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    ProxyConnection(IProxyConnectionsTracker* tracker, PrimaryConnection const& primaryConnection)
        : m_tracker(tracker), m_primaryConnection(primaryConnection)
        {
        m_db.OpenBeSQLiteDb(m_primaryConnection.GetDb().GetDbFileName(), Db::OpenParams(Db::OpenMode::Readonly, DefaultTxn::No, new BusyRetry()));
        LOG_CONNECTIONS.infov("%p ProxyConnection[%s] created on thread %d.", this, m_primaryConnection.GetId().c_str(), (int)BeThreadUtilities::GetCurrentThreadId());
        }

protected:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    Utf8StringCR _GetId() const override {return m_primaryConnection.GetId();}
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    ECDbR _GetECDb() const override {return m_primaryConnection.GetECDb();}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            01/2018
    +---------------+---------------+---------------+---------------+-----------+------*/
    BeSQLite::Db& _GetDb() const override {return m_db;}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    bool _IsOpen() const override {return m_primaryConnection.IsOpen();}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    bool _IsReadOnly() const override {return m_primaryConnection.IsReadOnly();}
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            01/2018
    +---------------+---------------+---------------+---------------+-----------+------*/
    void _InterruptRequests() const override {m_db.Interrupt();}

public:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    static RefCountedPtr<ProxyConnection> Create(IProxyConnectionsTracker* tracker, PrimaryConnection const& primaryConnection)
        {
        BeAssert(primaryConnection.IsOpen());
        return new ProxyConnection(tracker, primaryConnection);
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    ~ProxyConnection()
        {
        m_db.CloseDb();
        LOG_CONNECTIONS.infov("%p ProxyConnection[%s] closed on thread %d", this, GetId().c_str(), (int)BeThreadUtilities::GetCurrentThreadId());
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    void SetTracker(IProxyConnectionsTracker* tracker) {m_tracker = tracker;}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    uint32_t AddRef() const override
        {
        auto result = m_refCount.IncrementAtomicPre(std::memory_order_relaxed);
        if (nullptr != m_tracker)
            m_tracker->OnProxyConnectionAcquire(*this);
        return result;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    uint32_t Release() const override
        {
        uint32_t countWas = m_refCount.DecrementAtomicPost(std::memory_order_release);
        REFCOUNT_RELEASE_CHECK(countWas);
        if (nullptr != m_tracker)
            m_tracker->OnProxyConnectionRelease(*this);
        if (1 == countWas)
            {
            std::atomic_thread_fence(std::memory_order_acquire);
            delete this;
            }
        return countWas - 1;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    uint32_t GetRefCount() const {return m_refCount.load();}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimaryConnection::_InterruptRequests() const
    {
    for (ProxyConnection const* proxy : m_proxyConnections)
        proxy->InterruptRequests();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimaryConnection::_OnConnectionClosed(ECDbCR db)
    {
    BeAssert(&db == &m_ecdb);
    if (m_isOpen)
        {
        LOG_CONNECTIONS.infov("%p PrimaryConnection[%s] closed on thread %d", this, m_id.c_str(), (int)BeThreadUtilities::GetCurrentThreadId());
        m_isOpen = false;
        m_manager.NotifyConnectionClosed(m_id);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimaryConnection::_OnConnectionReloaded(ECDbCR db)
    {
    BeAssert(&db == &m_ecdb);
    if (m_isOpen)
        {
        LOG_CONNECTIONS.infov("%p PrimaryConnection[%s] reloaded on thread %d", this, m_id.c_str(), (int)BeThreadUtilities::GetCurrentThreadId());
        IConnectionPtr ref = this; // keep refcount of this instance so it doesnt get destroyed after NotifyConnectionClosed call
        m_manager.NotifyConnectionClosed(m_id);
        if (m_isPrimary)
            m_manager.NotifyPrimaryConnectionOpened(m_ecdb);
        else
            m_manager.NotifyConnectionOpened(m_ecdb);
        }
    }

//=======================================================================================
// @bsiclass                                    Grigas.Petraitis                10/2017
//=======================================================================================
struct ThreadLocalConnectionStore : IProxyConnectionsTracker, protected BeThreadLocalStorage
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

private:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            10/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    IConnection* GetConnectionPtr()
        {
        void* ptr = GetValueAsPointer();
        if (nullptr != ptr)
            return static_cast<IConnection*>(ptr);
        return nullptr;
        }

public:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            10/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    ThreadLocalConnectionStore(PrimaryConnection& primaryConnection)
        : m_primaryConnection(&primaryConnection)
        {
        SetValueAsPointer(m_primaryConnection.get());
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            10/2017
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
    * @bsimethod                                    Grigas.Petraitis            11/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    void OnProxyConnectionAcquire(ProxyConnection const&) override {}

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    void OnProxyConnectionRelease(ProxyConnection const& proxy) override
        {
        if (0 == proxy.GetRefCount())
            m_primaryConnection->NotifyProxyConnectionClosed(proxy);
        m_proxyConnectionsCV.notify_all();
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            10/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    IConnection* GetConnection()
        {
        IConnection* connection = GetConnectionPtr();
        if (nullptr != connection)
            return connection;

        RefCountedPtr<ProxyConnection> proxy = ProxyConnection::Create(this, *m_primaryConnection);
        m_proxyConnections.push_back(proxy);
        m_primaryConnection->NotifyProxyConnectionOpened(*proxy);
        SetValueAsPointer(proxy.get());
        return proxy.get();
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            10/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    PrimaryConnection const& GetPrimaryConnection() const {return *m_primaryConnection;}
};

//=======================================================================================
// @bsiclass                                    Grigas.Petraitis                10/2017
//=======================================================================================
struct ConnectionManager::ConnectionsStore
{
private:
    bmap<Utf8String, ThreadLocalConnectionStore*> m_connections;

public:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    ~ConnectionsStore()
        {
        for (auto pair : m_connections)
            delete pair.second;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            10/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    void Add(PrimaryConnection& connection)
        {
        if (m_connections.end() != m_connections.find(connection.GetId()))
            {
            BeAssert(false);
            return;
            }
        m_connections[connection.GetId()] = new ThreadLocalConnectionStore(connection);
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            10/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    bool Remove(Utf8StringCR connectionId)
        {
        auto iter = m_connections.find(connectionId);
        if (m_connections.end() == iter)
            return false;

        delete iter->second;
        m_connections.erase(iter);
        return true;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            10/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    IConnection* Get(Utf8StringCR id)
        {
        auto iter = m_connections.find(id);
        if (m_connections.end() == iter)
            return nullptr;

        return iter->second->GetConnection();
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    IConnection* Get(ECDbCR ecdb)
        {
        for (auto pair : m_connections)
            {
            if (&pair.second->GetPrimaryConnection().GetDb() == &ecdb || &pair.second->GetConnection()->GetDb() == &ecdb)
                return pair.second->GetConnection();
            }
        return nullptr;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectionManager::ConnectionManager()
    : m_activeConnections(new ConnectionsStore()), m_primaryThreadId(BeThreadUtilities::GetCurrentThreadId())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ConnectionManager::~ConnectionManager()
    {
    DELETE_AND_CLEAR(m_activeConnections);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectionManager::_AddListener(IConnectionsListener& listener) const
    {
    BeMutexHolder lock(m_listenersMutex);
    m_listeners.push_back(&listener);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectionManager::_DropListener(IConnectionsListener& listener) const
    {
    BeMutexHolder lock(m_listenersMutex);
    m_listeners.erase(std::find(m_listeners.begin(), m_listeners.end(), &listener));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectionManager::BroadcastEvent(ConnectionEvent const& evt) const
    {
    BeMutexHolder lock(m_listenersMutex);
    bvector<IConnectionsListener*> listeners = m_listeners;
    lock.unlock();

    for (IConnectionsListener* listener : listeners)
        listener->NotifyConnectionEvent(evt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IConnection* ConnectionManager::_GetConnection(Utf8CP connectionId) const
    {
    BeMutexHolder lock(m_connectionsMutex);
    return m_activeConnections->Get(connectionId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IConnection* ConnectionManager::_GetConnection(ECDbCR ecdb) const
    {
    BeMutexHolder lock(m_connectionsMutex);
    return m_activeConnections->Get(ecdb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectionPtr ConnectionManager::GetOrCreateConnection(ECDbR ecdb, bool isProjectPrimary)
    {
    Utf8PrintfString connectionId("%" PRIu64 ":%s:%s", (uint64_t)&ecdb, ecdb.GetDbGuid().ToString().c_str(), ecdb.GetDbFileName());

    BeMutexHolder lock(m_connectionsMutex);
    IConnectionPtr connection = m_activeConnections->Get(connectionId);
    if (connection.IsValid())
        return connection;

    RefCountedPtr<PrimaryConnection> primaryConnection = PrimaryConnection::Create(*this, connectionId, ecdb, isProjectPrimary);
    m_activeConnections->Add(*primaryConnection);
    lock.unlock();

    BroadcastEvent(ConnectionEvent(*primaryConnection, isProjectPrimary, ConnectionEventType::Opened));
    
    return primaryConnection;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
IConnectionPtr ConnectionManager::_CreateConnection(ECDbR ecdb)
    {
    return GetOrCreateConnection(ecdb, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectionManager::NotifyConnectionOpened(ECDbR connection)
    {
    GetOrCreateConnection(connection, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectionManager::NotifyPrimaryConnectionOpened(ECDbR connection)
    {
    GetOrCreateConnection(connection, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectionManager::NotifyConnectionClosed(Utf8StringCR connectionId)
    {
    BeMutexHolder lock(m_connectionsMutex);
    IConnectionPtr primaryConnection = m_activeConnections->Get(connectionId);
    if (primaryConnection.IsNull())
        {
        BeAssert(false);
        return;
        }
    lock.unlock();

    // must broadcast before removing the connection
    BroadcastEvent(ConnectionEvent(*primaryConnection, false, ConnectionEventType::Closed));

    // release the connection (it's still kept in m_activeConnections)
    primaryConnection = nullptr;

    // note: Remove will block this thread until all proxy connections
    // are finished being used on other threads
    m_activeConnections->Remove(connectionId);
    }
