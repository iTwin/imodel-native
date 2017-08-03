/*--------------------------------------------------------------------------------------+
|
|     $Source: ServicesTierUtilities.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "iModelJsInternal.h"

BEGIN_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE

namespace {

DEFINE_POINTER_SUFFIX_TYPEDEFS (UvNetAddressDescriptor)
DEFINE_POINTER_SUFFIX_TYPEDEFS (UvBuffer)
DEFINE_POINTER_SUFFIX_TYPEDEFS (UvHandle)
DEFINE_POINTER_SUFFIX_TYPEDEFS (UvIoStream)
DEFINE_POINTER_SUFFIX_TYPEDEFS (UvIoStreamShutdownRequest)
DEFINE_POINTER_SUFFIX_TYPEDEFS (UvIoStreamWriteRequest)
DEFINE_POINTER_SUFFIX_TYPEDEFS (UvRequest)
DEFINE_POINTER_SUFFIX_TYPEDEFS (UvTcpHandle)
DEFINE_POINTER_SUFFIX_TYPEDEFS (UvTcpConnectRequest)

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct UvNetAddressDescriptor
    {
public:
    enum class IP { V4, V6 };

private:
    IP m_protocol;
    struct sockaddr_in m_addr4;
    struct sockaddr_in6 m_addr6;

public:
    UvNetAddressDescriptor (Js::StringCR address, Js::NumberCR port, Js::NumberCR protocol, int& status);

    IP GetProtocol() const { return m_protocol; }
    const sockaddr* GetPointer() const;
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct UvHandle
    {
private:
    uv_handle_t* m_handle;

    static void CloseHandler (uv_handle_t* handle);

    UvHandle (UvHandleCR other) = delete;
    UvHandleR operator= (UvHandleCR other) = delete;

public:
    UvHandle (uv_handle_t* handle): m_handle (handle) { ; }
    UvHandle (UvHandle&& other);
    ~UvHandle();

    uv_handle_t* GetPointer() const { return m_handle; }
    void Close();
    void ClearAndClose();
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct UvBuffer
    {
private:
    uv_buf_t m_buffer;

public:
    UvBuffer (char* base, unsigned int length);
    UvBuffer (Js::ArrayBufferCR buffer);

    uv_buf_t const* GetPointer() const { return &m_buffer; }
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct UvRequest
    {
private:
    uv_req_t* m_request;

    UvRequest (UvRequestCR other) = delete;
    UvRequestR operator= (UvRequestCR other) = delete;

protected:
    UvRequest (uv_req_t* request) : m_request (request) { ; }
    UvRequest (UvRequest&& other);

public:
    ~UvRequest();

    uv_req_t* GetPointer() const { return m_request; }
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct UvIoStreamShutdownRequest : public UvRequest
    {
public:
    UvIoStreamShutdownRequest (UvIoStreamCR handle, uv_shutdown_cb callback, int& status);
    UvIoStreamShutdownRequest (UvIoStreamShutdownRequest&& other) : UvRequest (std::move (other)) { ; }

    uv_shutdown_t* GetPointer() const;
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct UvIoStreamWriteRequest : public UvRequest
    {
public:
    UvIoStreamWriteRequest (UvIoStreamCR handle, UvBufferCR buffer, uv_write_cb callback, int& status);
    UvIoStreamWriteRequest (UvIoStreamWriteRequest&& other) : UvRequest (std::move (other)) { ; }

    uv_write_t* GetPointer() const;
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct UvIoStream : public UvHandle
    {
public:
    UvIoStream (uv_stream_t* handle) : UvHandle (reinterpret_cast<uv_handle_t*>(handle)) { ; }
    UvIoStream (UvIoStream&& other) : UvHandle (std::move (other)) { ; }

public:
    uv_stream_t* GetPointer() const;
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct UvTcpHandle : public UvIoStream
    {
public:
    UvTcpHandle (uv_tcp_t* handle) : UvIoStream (reinterpret_cast<uv_stream_t*>(handle)) { ; }
    UvTcpHandle (int& status);
    UvTcpHandle (UvTcpHandle&& other) : UvIoStream (std::move (other)) { ; }

    uv_tcp_t* GetPointer() const;
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct UvTcpConnectRequest : public UvRequest
    {
public:
    UvTcpConnectRequest (UvNetAddressDescriptorCR address, UvTcpHandleCR handle, uv_connect_cb callback, int& status);
    UvTcpConnectRequest (UvTcpConnectRequest&& other) : UvRequest (std::move (other)) { ; }

    uv_connect_t* GetPointer() const;
    };

namespace JsApi {

DEFINE_POINTER_SUFFIX_TYPEDEFS (Base)
DEFINE_POINTER_SUFFIX_TYPEDEFS (ObjectBase)
DEFINE_POINTER_SUFFIX_TYPEDEFS (uv_Status)
DEFINE_POINTER_SUFFIX_TYPEDEFS (uv_Handle)
DEFINE_POINTER_SUFFIX_TYPEDEFS (uv_io_Stream)
DEFINE_POINTER_SUFFIX_TYPEDEFS (uv_io_Stream_write_Promise)
DEFINE_POINTER_SUFFIX_TYPEDEFS (uv_io_shutdown_Promise)
DEFINE_POINTER_SUFFIX_TYPEDEFS (uv_tcp_connect_Promise)
DEFINE_POINTER_SUFFIX_TYPEDEFS (uv_tcp_ConnectResult)
DEFINE_POINTER_SUFFIX_TYPEDEFS (uv_tcp_BindResult)
DEFINE_POINTER_SUFFIX_TYPEDEFS (uv_tcp_Handle)
DEFINE_POINTER_SUFFIX_TYPEDEFS (uv_tcp_Server)
DEFINE_POINTER_SUFFIX_TYPEDEFS (websocketpp_ClientConnection)
DEFINE_POINTER_SUFFIX_TYPEDEFS (websocketpp_ServerEndpoint)

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Promise
    {
public:
    static Js::Object CreateAndReject (Js::ScopeR scope, Js::ValueCR result);
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
enum class uv_net_IP : uint32_t
    {
    V4 = 0,
    V6 = 1
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct uv_StatusCode
    {
    static uint32_t Create (int uvResult)
        {
        if (uvResult < 0)
            {
            switch (uvResult)
                {
                case UV_E2BIG:           return 1;
                case UV_EACCES:          return 2;
                case UV_EADDRINUSE:      return 3;
                case UV_EADDRNOTAVAIL:   return 4;
                case UV_EAFNOSUPPORT:    return 5;
                case UV_EAGAIN:          return 6;
                case UV_EAI_ADDRFAMILY:  return 7;
                case UV_EAI_AGAIN:       return 8;
                case UV_EAI_BADFLAGS:    return 9;
                case UV_EAI_BADHINTS:    return 10;
                case UV_EAI_CANCELED:    return 11;
                case UV_EAI_FAIL:        return 12;
                case UV_EAI_FAMILY:      return 13;
                case UV_EAI_MEMORY:      return 14;
                case UV_EAI_NODATA:      return 15;
                case UV_EAI_NONAME:      return 16;
                case UV_EAI_OVERFLOW:    return 17;
                case UV_EAI_PROTOCOL:    return 18;
                case UV_EAI_SERVICE:     return 19;
                case UV_EAI_SOCKTYPE:    return 20;
                case UV_EALREADY:        return 21;
                case UV_EBADF:           return 22;
                case UV_EBUSY:           return 23;
                case UV_ECANCELED:       return 24;
                case UV_ECHARSET:        return 25;
                case UV_ECONNABORTED:    return 26;
                case UV_ECONNREFUSED:    return 27;
                case UV_ECONNRESET:      return 28;
                case UV_EDESTADDRREQ:    return 29;
                case UV_EEXIST:          return 30;
                case UV_EFAULT:          return 31;
                case UV_EFBIG:           return 32;
                case UV_EHOSTUNREACH:    return 33;
                case UV_EINTR:           return 34;
                case UV_EINVAL:          return 35;
                case UV_EIO:             return 36;
                case UV_EISCONN:         return 37;
                case UV_EISDIR:          return 38;
                case UV_ELOOP:           return 39;
                case UV_EMFILE:          return 40;
                case UV_EMSGSIZE:        return 41;
                case UV_ENAMETOOLONG:    return 42;
                case UV_ENETDOWN:        return 43;
                case UV_ENETUNREACH:     return 44;
                case UV_ENFILE:          return 45;
                case UV_ENOBUFS:         return 46;
                case UV_ENODEV:          return 47;
                case UV_ENOENT:          return 48;
                case UV_ENOMEM:          return 49;
                case UV_ENONET:          return 50;
                case UV_ENOPROTOOPT:     return 51;
                case UV_ENOSPC:          return 52;
                case UV_ENOSYS:          return 53;
                case UV_ENOTCONN:        return 54;
                case UV_ENOTDIR:         return 55;
                case UV_ENOTEMPTY:       return 56;
                case UV_ENOTSOCK:        return 57;
                case UV_ENOTSUP:         return 58;
                case UV_EPERM:           return 59;
                case UV_EPIPE:           return 60;
                case UV_EPROTO:          return 61;
                case UV_EPROTONOSUPPORT: return 62;
                case UV_EPROTOTYPE:      return 63;
                case UV_ERANGE:          return 64;
                case UV_EROFS:           return 65;
                case UV_ESHUTDOWN:       return 66;
                case UV_ESPIPE:          return 67;
                case UV_ESRCH:           return 68;
                case UV_ETIMEDOUT:       return 69;
                case UV_ETXTBSY:         return 70;
                case UV_EXDEV:           return 71;
                case UV_UNKNOWN:         return 72;
                case UV_EOF:             return 73;
                case UV_ENXIO:           return 74;
                case UV_EMLINK:          return 75;
                }
            }

        return 0;
        }
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Base
    {
private:
    UtilitiesCR m_owner;

    Base (BaseCR other) = delete;
    BaseR operator= (BaseCR other) = delete;

protected:
    Base (UtilitiesCR owner) : m_owner (owner) { ; }

public:
    UtilitiesCR GetOwner() const { return m_owner; }
    Js::Object CreateStatus (Js::ScopeR scope, int uvResult) const;
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct ObjectBase : public Base
    {
private:
    Js::Reference m_object;

protected:
    ObjectBase (UtilitiesCR owner) : Base (owner) { ; }

    Js::ReferenceR GetObject() { return m_object; }
    Js::ReferenceCR GetObject() const { return m_object; }

public:
    virtual ~ObjectBase() { ; }
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct uv_Status
    {
public:
    static Js::Object Create (Js::ScopeR scope, int uvResult, Js::ValueCR prototype);
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct uv_Handle : public ObjectBase
    {
    friend struct Utilities;

private:
    static void CloseHandler (uv_handle_t* handle);

protected:
    uv_Handle (UtilitiesCR owner) : ObjectBase (owner) { ; }

public:
    virtual UvHandleCR GetUvHandle() const = 0;
    virtual UvHandleR GetUvHandle() = 0;
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct uv_io_Stream : public uv_Handle
    {
    friend struct Utilities;

private:
    Js::Reference m_allocator;
    Js::Reference m_readCallback;
    std::map<void*, Js::ReferenceP> m_buffers;

    static void AllocHandler (uv_handle_t* handle, size_t suggestedSize, uv_buf_t* buf);
    static void ReadHandler (uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

protected:
    uv_io_Stream (UtilitiesCR owner) : uv_Handle (owner) { ; }

public:
    ~uv_io_Stream() override;
            
    virtual UvIoStreamCR GetUvIoStream() const = 0;
    virtual UvIoStreamR GetUvIoStream() = 0;

    Js::ReferenceCR GetAllocator() const { return m_allocator; }
    Js::ReferenceCR GetReadCallback() const { return m_readCallback; }

    void SetHandlers (Js::ObjectCR allocator, Js::FunctionCR readCallback);

    void StoreBuffer (Js::ArrayBufferCR buffer);
    Js::ArrayBuffer GetBuffer (void* identifier);
    void ReleaseBuffer (void* identifier);
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct uv_io_shutdown_Promise : public Base
    {
    friend struct Utilities;

private:
    UvIoStreamShutdownRequest m_request;
    Js::Promise m_promise;

    static void Handler (uv_shutdown_t* req, int status);

    uv_io_shutdown_Promise (UtilitiesCR owner, Js::ScopeR scope, UvIoStreamShutdownRequest&& request);

public:
    static Js::Object Create (UtilitiesCR owner, Js::ScopeR scope, UvIoStreamShutdownRequest&& request);
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct uv_io_Stream_write_Promise : public Base
    {
    friend struct Utilities;

private:
    UvIoStreamWriteRequest m_request;
    Js::Promise m_promise;

    uv_io_Stream_write_Promise (UtilitiesCR owner, Js::ScopeR scope, UvIoStreamWriteRequest&& request);

    static void Handler (uv_write_t* req, int status);

public:
    static Js::Object Create (UtilitiesCR owner, Js::ScopeR scope, UvIoStreamWriteRequest&& request);
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct uv_tcp_connect_Promise : public Base
    {
    friend struct Utilities;

private:
    UvTcpHandle m_handle;
    UvTcpConnectRequest m_request;
    Js::Promise m_promise;

    static void Handler (uv_connect_t* req, int status);

    uv_tcp_connect_Promise (UtilitiesCR owner, UvTcpHandle&& handle, UvTcpConnectRequest&& request);

public:
    static Js::Object Create (UtilitiesCR owner, Js::ScopeR scope, UvTcpHandle&& handle, UvTcpConnectRequest&& request);

    UvTcpHandleCR GetUvTcpHandle() const { return m_handle; }
    UvTcpHandleR GetUvTcpHandle() { return m_handle; }
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct uv_tcp_ConnectResult
    {
public:
    static Js::Object Create (UtilitiesCR owner, Js::ScopeR scope, int uvResult, Js::ValueCR connection);
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct uv_tcp_BindResult
    {
public:
    static Js::Object Create (UtilitiesCR owner, Js::ScopeR scope, int uvResult, Js::ValueCR server);
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct uv_tcp_Handle : public uv_io_Stream
    {
private:
    UvTcpHandle m_handle;

protected:
    uv_tcp_Handle (UtilitiesCR owner, Js::ScopeR scope, UvTcpHandle&& handle);

public:
    static Js::Object Create (UtilitiesCR owner, Js::ScopeR scope, UvTcpHandle&& handle);

    UvHandleCR GetUvHandle() const override { return m_handle; }
    UvHandleR GetUvHandle() override { return m_handle; }
    UvIoStreamCR GetUvIoStream() const override { return m_handle; }
    UvIoStreamR GetUvIoStream() override { return m_handle; }
    virtual UvTcpHandleCR GetUvTcpHandle() const { return m_handle; }
    virtual UvTcpHandleR GetUvTcpHandle() { return m_handle; }
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct uv_tcp_Server : public uv_tcp_Handle
    {
    friend struct Utilities;

private:
    Js::Reference m_listenCallback;

    static void ListenHandler (uv_stream_t* server, int status);

    uv_tcp_Server (UtilitiesCR owner, Js::ScopeR scope, UvTcpHandle&& handle) : uv_tcp_Handle (owner, scope, std::move (handle)) { ; }

public:
    static Js::Object Create (UtilitiesCR owner, Js::ScopeR scope, UvTcpHandle&& handle);

    Js::ReferenceCR GetListenCallback() const { return m_listenCallback; }
    void SetListenCallback (Js::FunctionCR value);
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct websocketpp_ServerEndpoint : public ObjectBase
    {
public:
    typedef websocketpp::server<websocketpp::config::core> websocketpp_server_t;

private:
    websocketpp_server_t m_server;

    websocketpp_ServerEndpoint (UtilitiesCR owner, Js::ScopeR scope);

public:
    static Js::Object Create (UtilitiesCR owner, Js::ScopeR scope);

    websocketpp_server_t const& GetServer() const { return m_server; }
    websocketpp_server_t& GetServer() { return m_server; }
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct websocketpp_ClientConnection : public ObjectBase
    {
public:
    typedef websocketpp::connection<websocketpp::config::core> websocketpp_connection_t;
    typedef std::shared_ptr<websocketpp_connection_t> websocketpp_connection_ptr_t;

private:
    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct Relay : public std::streambuf
        {
    private:
        websocketpp_ClientConnectionR m_connection;

    protected:
        std::streamsize xsputn (const char_type* s, std::streamsize count) override;

    public:
        Relay (websocketpp_ClientConnectionR connection) : m_connection (connection) { ; }
        };

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct Forwarder
        {
    private:
        websocketpp_ClientConnectionR m_connection;

    public:
        Forwarder (websocketpp_ClientConnectionR connection) : m_connection (connection) { ; }

        void operator() (websocketpp::connection_hdl handle);
        void operator() (websocketpp::connection_hdl handle, websocketpp_ServerEndpoint::websocketpp_server_t::message_ptr message);
        };

    websocketpp_connection_ptr_t m_connection;
    Relay m_relay;
    std::ostream m_output;

    websocketpp_ClientConnection (UtilitiesCR owner, Js::ScopeR scope, websocketpp_connection_ptr_t const& connection);

public:
    static Js::Object Create (UtilitiesCR owner, Js::ScopeR scope, websocketpp_connection_ptr_t const& connection);

    websocketpp_connection_t const& GetConnection() const { return *m_connection; }
    websocketpp_connection_t& GetConnection() { return *m_connection; }
    };

}

}

END_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE
