/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECDb/ECDbApi.h>
#include <ECPresentation/ECPresentationTypes.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC

//=======================================================================================
//! @ingroup GROUP_Presentation
// @bsiclass
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
    virtual void _Reset() = 0;
    virtual ECSqlStatementCache const& _GetStatementCache() const = 0;
public:
    Utf8StringCR GetId() const {return _GetId();}
    ECDbR GetECDb() const {return _GetECDb();}
    BeSQLite::Db& GetDb() const {return _GetDb();}
    bool IsOpen() const {return _IsOpen();}
    bool IsReadOnly() const {return _IsReadOnly();}
    void InterruptRequests() const {_InterruptRequests();}
    void Reset() {_Reset();}
    ECSqlStatementCache const& GetStatementCache() const {return _GetStatementCache();}
};

//=======================================================================================
//! A connection cache interface which provides connection (BeSQLite::EC::ECDb) by its id.
//! @ingroup GROUP_Presentation
// @bsiclass
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
// @bsiclass
//=======================================================================================
enum class ConnectionEventType
    {
    Opened,
    Closed,
    Suspended,
    Resumed,
    ProxyCreated,
    };

//=======================================================================================
//! A structure that contains information about a connection-related event.
// @bsiclass
//=======================================================================================
struct ConnectionEvent
{
private:
    RefCountedPtr<IConnection const> m_connection;
    ConnectionEventType m_eventType;
public:
    ConnectionEvent(IConnectionCR connection, ConnectionEventType eventType)
        : m_connection(&connection), m_eventType(eventType)
        {}
    IConnectionCR GetConnection() const {return *m_connection;}
    ConnectionEventType GetEventType() const {return m_eventType;}
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* allocator = nullptr) const;
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* allocator = nullptr) const;
};

//=======================================================================================
//! An interface for a listener which listens for connection-related events.
// @bsiclass
//=======================================================================================
struct IConnectionsListener
{
protected:
    virtual void _OnConnectionEvent(ConnectionEvent const&) = 0;
    // note: higher priority listeners are called later
    virtual int _GetPriority() const {return 1000;}
public:
    virtual ~IConnectionsListener() {}
    void NotifyConnectionEvent(ConnectionEvent const& evt) {_OnConnectionEvent(evt);}
    int GetPriority() const {return _GetPriority();}
};

//=======================================================================================
//! An interface for connection manager which acts as IConnectionCache and broadcasts
//! connection-related events.
//! @ingroup GROUP_Presentation
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IConnectionManager : IConnectionCache
{
protected:
    virtual void _AddListener(IConnectionsListener&) const = 0;
    virtual void _DropListener(IConnectionsListener&) const = 0;
    virtual IConnectionPtr _CreateConnection(ECDbR) = 0;
    virtual void _CloseConnections() = 0;

public:
    //! Add listener which listens for connection events.
    //! @param[in] listener The listener which will listen for connection events.
    void AddListener(IConnectionsListener& listener) const {_AddListener(listener);}

    //! Drop listener.
    //! @param[in] listener The listener which which should be dropped.
    void DropListener(IConnectionsListener& listener) const {_DropListener(listener);}

    //! Create a connection to the specified ECDb.
    IConnectionPtr CreateConnection(ECDbR ecdb) {return _CreateConnection(ecdb);}

    //! Force close all connections
    void CloseConnections() {_CloseConnections();}
};

//=======================================================================================
//! Manager that stores currently open connections and can get one of them by
//! connection ID.
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ConnectionManager : IConnectionManager
{
    struct ConnectionsStore;

    struct Props
    {
    private:
        Nullable<uint64_t> m_mmapFileSize;
        Nullable<uint64_t> m_memoryCacheSize;
        Nullable<uint64_t> m_busyTimeout;
    public:
        Props(Nullable<uint64_t> mmapFileSize = (uint64_t)0)
            : m_mmapFileSize(mmapFileSize)
            {}
        Nullable<uint64_t> const& GetMmapFileSize() const {return m_mmapFileSize;}
        void SetMmapFileSize(Nullable<uint64_t> value) {m_mmapFileSize = value;}
        Nullable<uint64_t> const& GetMemoryCacheSize() const {return m_memoryCacheSize;}
        void SetMemoryCacheSize(Nullable<uint64_t> value) {m_memoryCacheSize = value;}
        Nullable<uint64_t> const& GetBusyTimeout() const {return m_busyTimeout;}
        void SetBusyTimeout(Nullable<uint64_t> value) {m_busyTimeout = value;}
    };

private:
    Props m_props;
    uintptr_t m_primaryThreadId;
    mutable BeMutex m_connectionsMutex;
    ConnectionsStore* m_activeConnections;
    mutable BeMutex m_listenersMutex;
    mutable bmap<int, bset<IConnectionsListener*>> m_listeners;

private:
    void BroadcastEvent(ConnectionEvent const&) const;
    IConnectionPtr GetOrCreateConnection(ECDbR);

protected:
    ECPRESENTATION_EXPORT IConnection* _GetConnection(Utf8CP connectionId) const override;
    ECPRESENTATION_EXPORT IConnection* _GetConnection(ECDbCR) const override;
    ECPRESENTATION_EXPORT void _AddListener(IConnectionsListener&) const override;
    ECPRESENTATION_EXPORT void _DropListener(IConnectionsListener&) const override;
    ECPRESENTATION_EXPORT IConnectionPtr _CreateConnection(ECDbR) override;
    ECPRESENTATION_EXPORT void _CloseConnections() override;

public:
    ECPRESENTATION_EXPORT ConnectionManager(Props props = Props());
    ECPRESENTATION_EXPORT ~ConnectionManager();
    ECPRESENTATION_EXPORT void NotifyConnectionOpened(ECDbR);
    Props const& GetProps() const {return m_props;}

public:
    ECPRESENTATION_EXPORT void NotifyProxyConnectionOpened(IConnectionCR);
    void NotifyConnectionClosed(Utf8StringCR connectionId);
    void NotifyConnectionSuspended(Utf8StringCR connectionId);
    void NotifyConnectionResumed(Utf8StringCR connectionId);
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct DisableProxyConnectionThreadVerification
{
private:
    IConnectionCP m_connection;
public:
    DisableProxyConnectionThreadVerification(IConnectionCR);
    virtual ~DisableProxyConnectionThreadVerification();
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
