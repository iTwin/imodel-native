/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/Connection.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/Connection.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ConnectionEvent::AsJson(rapidjson::Document::AllocatorType* allocator) const
    {
    rapidjson::Document json(allocator);
    json.SetObject();
    json.AddMember("EventType", (int)m_eventType, json.GetAllocator());

    rapidjson::Value connectionJson(rapidjson::kObjectType);
    connectionJson.AddMember("ConnectionId", rapidjson::Value(m_connection.GetDbGuid().ToString().c_str(), json.GetAllocator()), json.GetAllocator());
    if (m_eventType == ConnectionEventType::Opened)
        connectionJson.AddMember("IsProjectPrimary", m_isPrimaryConnection, json.GetAllocator());
    else
        connectionJson.AddMember("IsProjectPrimary", rapidjson::Value(), json.GetAllocator());
    json.AddMember("Connection", connectionJson, json.GetAllocator());

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECDb* ConnectionManager::_GetConnection(Utf8CP connectionId) const
    {
    auto iter = m_activeConnections.find(connectionId);
    if (m_activeConnections.end() != iter)
        return iter->second;
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectionManager::BroadcastEvent(ConnectionEvent const& evt) const
    {
    for (IConnectionsListener* listener : m_listeners)
        listener->NotifyConnectionEvent(evt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectionManager::NotifyOpened(ECDbR connection, bool isProjectPrimary)
    {
    BeAssert(m_activeConnections.end() == m_activeConnections.find(connection.GetDbGuid().ToString()));
    m_activeConnections[connection.GetDbGuid().ToString()] = &connection;
    BroadcastEvent(ConnectionEvent(connection, isProjectPrimary, ConnectionEventType::Opened));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectionManager::NotifyConnectionOpened(ECDbR connection)
    {
    NotifyOpened(connection, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectionManager::NotifyConnectionClosed(ECDbR connection)
    {
    auto iter = m_activeConnections.find(connection.GetDbGuid().ToString());
    if (m_activeConnections.end() == iter)
        {
        BeAssert(false);
        return;
        }

    m_activeConnections.erase(iter);
    BroadcastEvent(ConnectionEvent(connection, false, ConnectionEventType::Closed));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ConnectionManager::NotifyPrimaryConnectionOpened(ECDbR connection)
    {
    NotifyOpened(connection, true);
    }
