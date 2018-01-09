/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/TestHelpers/TestConnectionCache.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
    Utf8String m_id;
    ECDbR m_db;
    Utf8StringCR _GetId() const override {return m_id;}
    ECDbR _GetDb() const override {return m_db;}
    bool _IsOpen() const override {return m_db.IsDbOpen();}
    bool _IsReadOnly() const override {return m_db.IsReadonly();}
    TestConnection(ECDbR db) : m_db(db), m_id(Utf8PrintfString("ConnectionId:%" PRIu64, (uint64_t)&db)) {}
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct TestConnectionCache : IConnectionCache
{
    bmap<Utf8String, IConnectionPtr> m_connections;
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
    IConnectionPtr Cache(ECDbR db)
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
    bmap<Utf8String, IConnectionPtr> m_connections;
    mutable bvector<IConnectionsListener*> m_listeners;

    void _AddListener(IConnectionsListener& listener) const override {m_listeners.push_back(&listener);}
    void _DropListener(IConnectionsListener& listener) const override {m_listeners.erase(std::find(m_listeners.begin(), m_listeners.end(), &listener));}
    IConnectionPtr _CreateConnection(ECDbR db) override
        {
        IConnectionPtr connection = new TestConnection(db);
        m_connections[connection->GetId()] = connection;
        return connection;
        }
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
    IConnectionPtr NotifyConnectionOpened(ECDbR db)
        {
        IConnectionPtr connection = CreateConnection(db);
        for (IConnectionsListener* listener : m_listeners)
            listener->NotifyConnectionEvent(ConnectionEvent(*connection, true, ConnectionEventType::Opened));
        return connection;
        }
    void NotifyConnectionClosed(IConnectionCR connection) 
        {
        for (IConnectionsListener* listener : m_listeners)
            listener->NotifyConnectionEvent(ConnectionEvent(connection, true, ConnectionEventType::Closed));
        m_connections.erase(connection.GetId());
        }
    void ClearConnections()
        {
        for (auto pair : m_connections)
            {
            for (IConnectionsListener* listener : m_listeners)
                listener->NotifyConnectionEvent(ConnectionEvent(*pair.second, true, ConnectionEventType::Closed));
            }
        m_connections.clear();
        }
};

END_ECPRESENTATIONTESTS_NAMESPACE