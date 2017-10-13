/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/Connection.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECDb/ECDbApi.h>
#include <ECPresentation/ECPresentationTypes.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//=======================================================================================
//! A connection cache interface which provides connection (BeSQLite::EC::ECDb) by its id (guid).
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                09/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IConnectionCache
{
protected:
    //! @see GetConnection
    virtual BeSQLite::EC::ECDb* _GetConnection(Utf8CP connectionId) const = 0;
public:
    //! Virtual destructor.
    virtual ~IConnectionCache() {}

    //! Get the connection based on its id.
    //! @param[in] connectionId The ID of connection to get. ID is the serialized guid of the ECDb.
    BeSQLite::EC::ECDb* GetConnection(Utf8CP connectionId) const {return _GetConnection(connectionId);}
};

//=======================================================================================
//! Lists possible types of connection events.
// @bsiclass                                    Grigas.Petraitis                11/2016
//=======================================================================================
enum class ConnectionEventType
    {
    Opened,
    Closed,
    };

//=======================================================================================
//! A structure that contains information about a connection-related event.
// @bsiclass                                    Grigas.Petraitis                08/2017
//=======================================================================================
struct ConnectionEvent
{
private:
    BeSQLite::EC::ECDbCR m_connection;
    ConnectionEventType m_eventType;
    bool m_isPrimaryConnection;
public:
    ConnectionEvent(BeSQLite::EC::ECDbCR connection, bool isPrimary, ConnectionEventType eventType)
        : m_connection(connection), m_eventType(eventType), m_isPrimaryConnection(isPrimary)
        {}
    BeSQLite::EC::ECDbCR GetConnection() const {return m_connection;}
    bool IsPrimaryConnection() const {return m_isPrimaryConnection;}
    ConnectionEventType GetEventType() const {return m_eventType;}
    rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;
};

//=======================================================================================
//! An interface for a listener which listens for connection-related events.
// @bsiclass                                    Grigas.Petraitis                08/2017
//=======================================================================================
struct IConnectionsListener
{
protected:
    virtual void _OnConnectionEvent(ConnectionEvent const&) = 0;
public:
    virtual ~IConnectionsListener() {}
    void NotifyConnectionEvent(ConnectionEvent const& evt) {_OnConnectionEvent(evt);}
};

//=======================================================================================
//! A connection manager interface which provides connection (BeSQLite::EC::ECDb) by its id (guid)
//! and manages connections listeners.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Saulius.Skliutas                10/2017
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IConnectionManager : IConnectionCache
{
protected:
    //! @see AddListener
    virtual void _AddListener(IConnectionsListener&) = 0;

    //! @see DropListener
    virtual void _DropListener(IConnectionsListener&) = 0;

public:
    //! Add listener which listens for connection events.
    //! @param[in] listener The listener which will listen for connection events.
    void AddListener(IConnectionsListener& listener) { _AddListener(listener); }

    //! Drop listener.
    //! @param[in] listener The listener which which should be dropped.
    void DropListener(IConnectionsListener& listener) { _DropListener(listener); }
};

//=======================================================================================
//! Manager that stores currently open connections and can get one of them by 
//! connection ID.
// @bsiclass                                    Grigas.Petraitis                11/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ConnectionManager : IConnectionManager
{
private:
    bmap<Utf8String, BeSQLite::EC::ECDb*> m_activeConnections;
    bvector<IConnectionsListener*> m_listeners;

private:
    void BroadcastEvent(ConnectionEvent const&) const;
    void NotifyOpened(BeSQLite::EC::ECDbR, bool isProjectPrimary);

protected:
    ECPRESENTATION_EXPORT BeSQLite::EC::ECDb* _GetConnection(Utf8CP connectionId) const override;
    void _AddListener(IConnectionsListener& listener) override { m_listeners.push_back(&listener); }
    void _DropListener(IConnectionsListener& listener) override { m_listeners.erase(std::find(m_listeners.begin(), m_listeners.end(), &listener)); }

public:
    ECPRESENTATION_EXPORT void NotifyConnectionOpened(BeSQLite::EC::ECDbR connection);
    ECPRESENTATION_EXPORT void NotifyConnectionClosed(BeSQLite::EC::ECDbR connection);

//__PUBLISH_SECTION_END__
public:
    ECPRESENTATION_EXPORT void NotifyPrimaryConnectionOpened(BeSQLite::EC::ECDbR connection);
//__PUBLISH_SECTION_START__
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
