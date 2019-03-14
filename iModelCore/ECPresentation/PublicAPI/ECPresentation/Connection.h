/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/Connection.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECDb/ECDbApi.h>
#include <ECPresentation/ECPresentationTypes.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC

//=======================================================================================
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                10/2017
//=======================================================================================
struct IConnection : IRefCounted
{
protected:
    virtual Utf8StringCR _GetId() const = 0;
    virtual ECDbR _GetECDb() const = 0;
    virtual BeSQLite::Db& _GetDb() const = 0;
    virtual bool _IsOpen() const = 0;
    virtual bool _IsReadOnly() const = 0;
    virtual void _InterruptRequests() const = 0;
public:
    Utf8StringCR GetId() const {return _GetId();}
    ECDbR GetECDb() const {return _GetECDb();}
    BeSQLite::Db& GetDb() const {return _GetDb();}
    bool IsOpen() const {return _IsOpen();}
    bool IsReadOnly() const {return _IsReadOnly();}
    void InterruptRequests() const {_InterruptRequests();}
};

//=======================================================================================
//! A connection cache interface which provides connection (BeSQLite::EC::ECDb) by its id.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                09/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IConnectionCache
{
protected:
    virtual IConnection* _GetConnection(Utf8CP connectionId) const = 0;
    virtual IConnection* _GetConnection(ECDbCR ecdb) const = 0;
public:
    //! Virtual destructor.
    virtual ~IConnectionCache() {}

    //! Get the connection based on its id.
    //! @param[in] connectionId The ID of connection to get. ID is the serialized guid of the ECDb.
    IConnection* GetConnection(Utf8CP connectionId) const {return _GetConnection(connectionId);}

    //! Get the connection for specific ECDb.
    //! @param[in] ecdb The ECDb to get the connection for.
    IConnection* GetConnection(ECDbCR ecdb) const {return _GetConnection(ecdb);}
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
    RefCountedPtr<IConnection const> m_connection;
    ConnectionEventType m_eventType;
    bool m_isPrimaryConnection;
public:
    ConnectionEvent(IConnectionCR connection, bool isPrimary, ConnectionEventType eventType)
        : m_connection(&connection), m_eventType(eventType), m_isPrimaryConnection(isPrimary)
        {}
    IConnectionCR GetConnection() const {return *m_connection;}
    bool IsPrimaryConnection() const {return m_isPrimaryConnection;}
    ConnectionEventType GetEventType() const {return m_eventType;}
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;
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
//! An interface for connection manager which acts as IConnectionCache and broadcasts
//! connection-related events.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Saulius.Skliutas                10/2017
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IConnectionManager : IConnectionCache
{
protected:
    //! @see AddListener
    virtual void _AddListener(IConnectionsListener&) const = 0;

    //! @see DropListener
    virtual void _DropListener(IConnectionsListener&) const = 0;

    //! @see CreateConnection
    virtual IConnectionPtr _CreateConnection(ECDbR) = 0;

public:
    //! Add listener which listens for connection events.
    //! @param[in] listener The listener which will listen for connection events.
    void AddListener(IConnectionsListener& listener) const {_AddListener(listener);}

    //! Drop listener.
    //! @param[in] listener The listener which which should be dropped.
    void DropListener(IConnectionsListener& listener) const {_DropListener(listener);}

    //! Create a connection to the specified ECDb.
    IConnectionPtr CreateConnection(ECDbR ecdb) {return _CreateConnection(ecdb);}
};

//=======================================================================================
//! Manager that stores currently open connections and can get one of them by 
//! connection ID.
// @bsiclass                                    Grigas.Petraitis                11/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ConnectionManager : IConnectionManager
{
    struct ConnectionsStore;

private:
    uintptr_t m_primaryThreadId;
    mutable BeMutex m_connectionsMutex;
    ConnectionsStore* m_activeConnections;
    mutable BeMutex m_listenersMutex;
    mutable bvector<IConnectionsListener*> m_listeners;

private:
    void BroadcastEvent(ConnectionEvent const&) const;
    IConnectionPtr GetOrCreateConnection(ECDbR, bool isProjectPrimary);

protected:
    ECPRESENTATION_EXPORT IConnection* _GetConnection(Utf8CP connectionId) const override;
    ECPRESENTATION_EXPORT IConnection* _GetConnection(ECDbCR) const override;
    ECPRESENTATION_EXPORT void _AddListener(IConnectionsListener&) const override;
    ECPRESENTATION_EXPORT void _DropListener(IConnectionsListener&) const override;
    ECPRESENTATION_EXPORT IConnectionPtr _CreateConnection(ECDbR) override;

public:
    ECPRESENTATION_EXPORT ConnectionManager();
    ECPRESENTATION_EXPORT ~ConnectionManager();
    ECPRESENTATION_EXPORT void NotifyConnectionOpened(ECDbR);

//__PUBLISH_SECTION_END__
public:
    ECPRESENTATION_EXPORT void NotifyPrimaryConnectionOpened(ECDbR);
    void NotifyConnectionClosed(Utf8StringCR connectionId);
//__PUBLISH_SECTION_START__
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
