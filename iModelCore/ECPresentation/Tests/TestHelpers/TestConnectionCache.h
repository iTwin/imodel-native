/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/TestHelpers/TestConnectionCache.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECPresentationTest.h"
#include <ECPresentation/Connection.h>

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2017
+===============+===============+===============+===============+===============+======*/
struct TestConnectionCache : IConnectionCache
{
    bmap<Utf8String, ECDb*> m_connections;
    ECDb* _GetConnection(Utf8CP connectionId) const override
        {
        auto iter = m_connections.find(connectionId);
        if (m_connections.end() == iter)
            return nullptr;
        return iter->second;
        }
    void Cache(ECDbR connection) {m_connections[connection.GetDbGuid().ToString()] = &connection;}
};

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                10/2017
+===============+===============+===============+===============+===============+======*/
struct TestConnectionManager : IConnectionManager
{
    bmap<Utf8String, ECDb*> m_connections;
    bvector<IConnectionsListener*> m_listeners;

    void _AddListener(IConnectionsListener& listener) override {m_listeners.push_back(&listener);}
    void _DropListener(IConnectionsListener& listener) override {m_listeners.erase(std::find(m_listeners.begin(), m_listeners.end(), &listener));}
    ECDb* _GetConnection(Utf8CP connectionId) const override
        {
        auto iter = m_connections.find(connectionId);
        if (m_connections.end() == iter)
            return nullptr;
        return iter->second;
        }
    void NotifyConnectionOpened(ECDbR connection) 
        {
        m_connections[connection.GetDbGuid().ToString()] = &connection;
        for (IConnectionsListener* listener : m_listeners)
            listener->NotifyConnectionEvent(ConnectionEvent(connection, true, ConnectionEventType::Opened));
        }
};

END_ECPRESENTATIONTESTS_NAMESPACE