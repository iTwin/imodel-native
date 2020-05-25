/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ECPresentationTest.h"
#include <ECPresentation/Connection.h>

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+===============+===============+===============+===============+===============+======*/
struct TestConnection : RefCounted<IConnection>
{
private:
    Utf8String m_id;
    ECDbR m_db;
    ECSqlStatementCache m_statementCache;
    std::function<void()> m_interruptHandler;
    std::function<void(Utf8StringCR)> m_usageListener;

private:
    void OnUsed(Utf8CP funcName) const
        {
        if (m_usageListener)
            m_usageListener(funcName);
        }

protected:
    Utf8StringCR _GetId() const override {OnUsed("id"); return m_id;}
    ECDbR _GetECDb() const override {OnUsed("ecdb"); return m_db;}
    BeSQLite::Db& _GetDb() const override {OnUsed("db"); return m_db;}
    ECSqlStatementCache const& _GetStatementCache() const override {OnUsed("statement cache"); return m_statementCache;}
    bool _IsOpen() const override {OnUsed("is open"); return m_db.IsDbOpen();}
    bool _IsReadOnly() const override {OnUsed("is readonly"); return m_db.IsReadonly();}
    void _InterruptRequests() const override {OnUsed("interrupt requests"); if(m_interruptHandler){m_interruptHandler();}}

public:
    TestConnection(ECDbR db)
        : m_db(db), m_id(Utf8PrintfString("ConnectionId:%" PRIu64, (uint64_t)&db)), m_statementCache(10, m_id.c_str())
        {}
    ~TestConnection() {m_statementCache.Empty();}
    void SetInterruptHandler(std::function<void()> handler) {m_interruptHandler = handler;}
    void NotifyConnectionReset() {m_statementCache.Empty();}
    void SetUsageListener(std::function<void(Utf8StringCR)> listener) {m_usageListener = listener;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct TestConnectionCache : IConnectionCache
{
    bmap<Utf8String, RefCountedPtr<TestConnection>> m_connections;
    IConnection* _GetConnection(Utf8CP connectionId) const override
        {
        auto iter = m_connections.find(connectionId);
        if (m_connections.end() == iter)
            return nullptr;
        return iter->second.get();
        }
    IConnection* _GetConnection(ECDbCR ecdb) const override
        {
        for (auto pair : m_connections)
            {
            if (&pair.second->GetDb() == &ecdb)
                return pair.second.get();
            }
        return nullptr;
        }
    RefCountedPtr<TestConnection> Cache(ECDbR db)
        {
        RefCountedPtr<TestConnection> connection = new TestConnection(db);
        m_connections[connection->GetId()] = connection;
        return connection;
        }
};

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                10/2017
+===============+===============+===============+===============+===============+======*/
struct TestConnectionManager : IConnectionManager
{
    bmap<Utf8String, RefCountedPtr<TestConnection>> m_connections;
    bmap<Utf8String, bvector<RefCountedPtr<TestConnection>>> m_secondaryConnections;
    mutable bvector<IConnectionsListener*> m_listeners;
    mutable BeMutex m_mutex;

    void _AddListener(IConnectionsListener& listener) const override
        {
        BeMutexHolder lock(m_mutex);
        m_listeners.push_back(&listener);
        }
    void _DropListener(IConnectionsListener& listener) const override
        {
        BeMutexHolder lock(m_mutex);
        m_listeners.erase(std::find(m_listeners.begin(), m_listeners.end(), &listener));
        }
    void _CloseConnections() override
        {
        BeMutexHolder lock(m_mutex);
        for (auto pair : m_connections)
            {
            for (IConnectionsListener* listener : m_listeners)
                listener->NotifyConnectionEvent(ConnectionEvent(*pair.second, true, ConnectionEventType::Closed));
            }
        m_connections.clear();
        m_secondaryConnections.clear();
        }
    RefCountedPtr<TestConnection> CreateConnection(ECDbR db)
        {
        BeMutexHolder lock(m_mutex);
        RefCountedPtr<TestConnection> connection = new TestConnection(db);
        m_connections[connection->GetId()] = connection;
        for (IConnectionsListener* listener : m_listeners)
            listener->NotifyConnectionEvent(ConnectionEvent(*connection, true, ConnectionEventType::Opened));
        return connection;
        }
    IConnectionPtr _CreateConnection(ECDbR db) override { return CreateConnection(db); }
    IConnection* _GetConnection(Utf8CP connectionId) const override
        {
        BeMutexHolder lock(m_mutex);
        auto iter = m_connections.find(connectionId);
        if (m_connections.end() == iter)
            return nullptr;
        return iter->second.get();
        }
    IConnection* _GetConnection(ECDbCR ecdb) const override
        {
        BeMutexHolder lock(m_mutex);
        for (auto pair : m_connections)
            {
            if (&pair.second->GetDb() == &ecdb)
                return pair.second.get();
            }
        return nullptr;
        }
    RefCountedPtr<TestConnection> NotifyConnectionOpened(ECDbR db)
        {
        BeMutexHolder lock(m_mutex);
        IConnection* existingConnection = nullptr;
        if (nullptr != (existingConnection = GetConnection(db)))
            {
            for (IConnectionsListener* listener : m_listeners)
                listener->NotifyConnectionEvent(ConnectionEvent(*existingConnection, true, ConnectionEventType::Opened));
            return dynamic_cast<TestConnection*>(existingConnection);
            }
        return dynamic_cast<TestConnection*>(CreateConnection(db).get());
        }
    void NotifyConnectionClosed(IConnectionCR connection)
        {
        BeMutexHolder lock(m_mutex);
        Utf8String connectionId = connection.GetId();
        for (IConnectionsListener* listener : m_listeners)
            listener->NotifyConnectionEvent(ConnectionEvent(connection, true, ConnectionEventType::Closed));
        m_connections.erase(connectionId);
        m_secondaryConnections.erase(connectionId);
        }
    RefCountedPtr<TestConnection> CreateSecondaryConnection(IConnectionCR primaryConnection)
        {
        BeMutexHolder lock(m_mutex);
        RefCountedPtr<TestConnection> connection = new TestConnection(primaryConnection.GetECDb());
        m_secondaryConnections[connection->GetId()].push_back(connection);
        return connection;
        }
};

END_ECPRESENTATIONTESTS_NAMESPACE