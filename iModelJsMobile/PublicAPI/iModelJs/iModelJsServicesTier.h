/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <iModelJs/iModelJs.h>
#include <websocketpp/server.hpp>
#include <websocketpp/config/core.hpp>
#include <libuv/uv.h>
#include <atomic>
#include <functional>
#include <map>

#if defined(BENTLEYCONFIG_OS_APPLE_IOS) || defined(BENTLEYCONFIG_OS_APPLE_MACOS)
#include <JavaScriptCore/JavaScriptCore.h>
#endif

/** @namespace BentleyApi::iModelJs::ServicesTier Contains types used by the services tier of the iModel.js application framework. */
#define BEGIN_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE  BEGIN_BENTLEY_IMODELJS_NAMESPACE namespace ServicesTier {
#define END_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE    } END_BENTLEY_IMODELJS_NAMESPACE

#define IMODELJS_SERVICES_TIER_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE

#define IMODELJS_SERVICES_TIER_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE struct _sname_; DEFINE_REF_COUNTED_PTR(_sname_) END_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE

IMODELJS_SERVICES_TIER_TYPEDEFS (Extension)
IMODELJS_SERVICES_TIER_TYPEDEFS (Environment)
IMODELJS_SERVICES_TIER_TYPEDEFS (Host)
IMODELJS_SERVICES_TIER_TYPEDEFS (UvHost)
IMODELJS_SERVICES_TIER_TYPEDEFS (NodeHost)
IMODELJS_SERVICES_TIER_TYPEDEFS (Utilities)
IMODELJS_SERVICES_TIER_TYPEDEFS (Uv)
IMODELJS_SERVICES_TIER_TYPEDEFS (WebSockets)
IMODELJS_SERVICES_TIER_TYPEDEFS (MobileGateway)

IMODELJS_SERVICES_TIER_REF_COUNTED_PTR (Extension)
IMODELJS_SERVICES_TIER_REF_COUNTED_PTR (MobileGateway)
IMODELJS_SERVICES_TIER_REF_COUNTED_PTR (UvHost)

BEGIN_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Extension : public RefCountedBase
    {
    friend struct Host;
    friend struct Environment;

public:
    typedef std::function<ExtensionPtr()> InstallCallback_T;

private:
    Js::RuntimeP m_runtime;

protected:
    IMODELJS_EXPORT virtual Utf8CP SupplyName() const = 0;
    IMODELJS_EXPORT virtual Napi::Value ExportJsModule (Js::RuntimeR) = 0;

public:
    IMODELJS_EXPORT static void Install (InstallCallback_T const& callback);

    IMODELJS_EXPORT Js::RuntimeR GetRuntime() const { return *m_runtime; }
    IMODELJS_EXPORT Napi::Env& Env() const { return GetRuntime().Env(); }
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Environment
    {
    friend struct Host;
    friend struct Extension;

private:
    static EnvironmentP s_instance;

    std::map<Utf8String, ExtensionPtr> m_extensions;

    Environment();
    ~Environment();

    void Install (ExtensionPtr extension);
    void Shutdown();
    Napi::Value DeliverExtension (Js::RuntimeR, Utf8StringCR identifier);

    static void InstallCoreExtensions();

public:
    IMODELJS_EXPORT static EnvironmentR GetInstance();
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Host : public RefCountedBase
    {
    friend struct Extension;

public:
    DEFINE_POINTER_SUFFIX_TYPEDEFS (Config)

    typedef std::function<void()> EventLoopCallback_T;

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct Config
        {
        std::atomic<bool> enableJsDebugger;
        std::atomic<bool> waitForJsDebugger;
        std::atomic<uint16_t> jsDebuggerPort;

        Config() : enableJsDebugger  (true),
                   waitForJsDebugger (false),
                   jsDebuggerPort    (9229) { ; }
        };

private:
    static std::atomic<HostP> s_instance;
    
    std::atomic<uv_loop_t*> m_eventLoop;
    std::atomic<intptr_t> m_eventLoopThreadId;
    Js::RuntimeP m_jsRuntime;
    std::atomic<bool> m_ready;
    bool m_stopped;
    uv_idle_t m_idler;
    EnvironmentP m_environment;
    Napi::ObjectReference m_notifyIdle;
    Napi::ObjectReference m_notifyShutdown;
    Napi::ObjectReference m_notifyReady;

    static void IdleHandler (uv_idle_t* handle);
    static void EventLoopCallbackAsyncHandler (uv_async_t* handle);

    static void EventLoopAsyncCloseHandler (uv_handle_t* handle);
    
    static void DispatchExtensionCallback (Extension::InstallCallback_T const& callback);

    static Utf8CP InitScript();

    void HandleReady();
    void HandleIdle();
    
    void StartIdler();
    void StopIdler();
    void InitializeEnvironment();
    void TerminateEnvironment();
    void EmptyExtensionsQueue();
    void PerformInstall (Extension::InstallCallback_T const& callback);
    void SetupJsRuntime();
    void TeardownJsRuntime();

protected:
    IMODELJS_EXPORT Host();
    IMODELJS_EXPORT virtual ~Host();

    IMODELJS_EXPORT Utf8String GetSystemArgv() const;
    IMODELJS_EXPORT Utf8String GetSystemCwd() const;

    IMODELJS_EXPORT virtual uv_loop_t* SupplyEventLoop() = 0;
    IMODELJS_EXPORT virtual Js::RuntimeR SupplyJsRuntime() = 0;
    IMODELJS_EXPORT virtual Napi::Function SupplyJsRequireHandler (Napi::Env& env, Napi::Object initParams);
    IMODELJS_EXPORT virtual void SupplyJsInfoValues (Napi::Env& env, Napi::Object info);

    IMODELJS_EXPORT virtual void OnReady() { ; }
    IMODELJS_EXPORT virtual void OnIdle() { ; }
    IMODELJS_EXPORT virtual void OnStop() { ; }

    IMODELJS_EXPORT void NotifyStarting();
    IMODELJS_EXPORT void NotifyStop();

public:
    IMODELJS_EXPORT static bool Exists();
    IMODELJS_EXPORT static HostR GetInstance();
    IMODELJS_EXPORT static ConfigR GetConfig();

    IMODELJS_EXPORT uv_loop_t* GetEventLoop() const;
    IMODELJS_EXPORT Js::RuntimeR GetJsRuntime() const;
    Napi::Env& Env() {return GetJsRuntime().Env();}

    IMODELJS_EXPORT bool IsReady() const;
    IMODELJS_EXPORT bool IsStopped() const;
    IMODELJS_EXPORT bool IsEventLoopThread() const;
    IMODELJS_EXPORT bool PostToEventLoop (EventLoopCallback_T const& callback);
        

#if defined(BENTLEYCONFIG_OS_APPLE_IOS) || defined(BENTLEYCONFIG_OS_APPLE_MACOS)
    IMODELJS_EXPORT JSContextRef GetContext() const;
#endif
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct UvHost : public Host
    {
private:
    uv_thread_t m_thread;
    Js::RuntimeP m_jsRuntime;

    static void EventLoopThreadEntry (void* arg);
    static Utf8CP RequireScript();

    void EventLoopThreadMain();

protected:
    IMODELJS_EXPORT uv_loop_t* SupplyEventLoop() override;
    IMODELJS_EXPORT Js::RuntimeR SupplyJsRuntime() override;
    IMODELJS_EXPORT Napi::Function SupplyJsRequireHandler (Napi::Env& env, Napi::Object initParams) override;
    IMODELJS_EXPORT void SupplyJsInfoValues (Napi::Env& env, Napi::Object info) override;
    IMODELJS_EXPORT void OnStop() override;

public:
    IMODELJS_EXPORT UvHost();
    IMODELJS_EXPORT ~UvHost() override;
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct NodeHost : public Host
    {
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Utilities : public Extension
    {
public:
    DEFINE_POINTER_SUFFIX_TYPEDEFS (JsPrototypes)

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct JsPrototypes
        {
        Napi::ObjectReference uv_Handle;
        Napi::ObjectReference uv_Status;
        Napi::ObjectReference uv_io_Stream;
        Napi::ObjectReference uv_tcp_BindResult;
        Napi::ObjectReference uv_tcp_ConnectResult;
        Napi::ObjectReference uv_tcp_Handle;
        Napi::ObjectReference uv_tcp_Server;
        Napi::ObjectReference websocketpp_Base;
        Napi::ObjectReference websocketpp_ClientConnection;
        Napi::ObjectReference websocketpp_ServerEndpoint;
        };

private:
    JsPrototypes m_prototypes;

#ifdef COMMENT_OUT
    static Utf8CP InitScript();
    Napi::Function EvaluateInitScript(Js::RuntimeR);

    Napi::Function uv_Handle_close (Napi::Env& env);
    Napi::Function uv_io_shutdown (Napi::Env& env);
    Napi::Function uv_io_Stream_isReadable (Napi::Env& env);
    Napi::Function uv_io_Stream_isWritable (Napi::Env& env);
    Napi::Function uv_io_Stream_read (Napi::Env& env);
    Napi::Function uv_io_Stream_write (Napi::Env& env);
    Napi::Function uv_tcp_bind (Napi::Env& env);
    Napi::Function uv_tcp_connect (Napi::Env& env);
    Napi::Function uv_tcp_Handle_setNoDelay (Napi::Env& env);
    Napi::Function uv_tcp_Handle_setKeepAlive (Napi::Env& env);
    Napi::Function uv_tcp_Server_setSimultaneousAccepts (Napi::Env& env);
    Napi::Function uv_tcp_Server_listen (Napi::Env& env);
    Napi::Function uv_tcp_Server_accept (Napi::Env& env);
    Napi::Function websocketpp_Base_Dispose (Napi::Env& env);
    Napi::Function websocketpp_ClientConnection_process (Napi::Env& env);
    Napi::Function websocketpp_ClientConnection_send (Napi::Env& env);
    Napi::Function websocketpp_ServerEndpoint_constructor (Napi::Env& env);
    Napi::Function websocketpp_ServerEndpoint_createConnection (Napi::Env& env);
#endif

    static Utf8CP SimpleInitScript();
    Napi::Function EvaluateSimpleInitScript(Js::RuntimeR);
    Napi::Object CreateInitParams (Napi::Env& env);
    void FindPrototypes (Napi::Object exports);
    
    Napi::Function uv_fs_open (Napi::Env& env);
    Napi::Function uv_fs_realpath (Napi::Env& env);
    Napi::Function uv_fs_stat (Napi::Env& env);
    Napi::Function uv_fs_read (Napi::Env& env);
    Napi::Function uv_fs_close (Napi::Env& env);

protected:
    IMODELJS_EXPORT Utf8CP SupplyName() const override { return "@bentley/imodeljs-services-tier-utilities"; }
    IMODELJS_EXPORT Napi::Value ExportJsModule (Js::RuntimeR) override;

public:
    IMODELJS_EXPORT JsPrototypesCR GetPrototypes() const { return m_prototypes; }
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct WebSockets
    {
public:
    DEFINE_POINTER_SUFFIX_TYPEDEFS (ServerEndpoint)
    DEFINE_POINTER_SUFFIX_TYPEDEFS (ClientConnection)
    DEFINE_REF_COUNTED_PTR (ServerEndpoint)
    DEFINE_REF_COUNTED_PTR (ClientConnection)

    enum class Event { Open, Error, Message };
    typedef websocketpp::server<websocketpp::config::core> websocketpp_server_t;
    typedef websocketpp::connection<websocketpp::config::core> websocketpp_connection_t;
    typedef std::shared_ptr<websocketpp_connection_t> websocketpp_connection_ptr_t;
    typedef std::function<void(Event, websocketpp_server_t::message_ptr)> EventCallback_T;
    typedef std::function<std::streamsize(const std::streambuf::char_type*, std::streamsize)> TransportHandler_T;

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct ServerEndpoint : public RefCountedBase
        {
    private:
        websocketpp_server_t m_server;

    public:
        IMODELJS_EXPORT static ServerEndpointPtr Create();

        IMODELJS_EXPORT ServerEndpoint();
        IMODELJS_EXPORT websocketpp_server_t const& GetServer() const { return m_server; }
        IMODELJS_EXPORT websocketpp_server_t& GetServer() { return m_server; }
        IMODELJS_EXPORT ClientConnectionPtr CreateConnection (EventCallback_T const& callback, TransportHandler_T const& handler);
        };

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct ClientConnection : public RefCountedBase
        {
        friend struct ServerEndpoint;

    private:
        //=======================================================================================
        // @bsiclass                                                    Steve.Wilson   7/17
        //=======================================================================================
        struct Relay : public std::streambuf
            {
        private:
            ClientConnectionR m_connection;

        protected:
            std::streamsize xsputn (const char_type* s, std::streamsize count) override;

        public:
            Relay (ClientConnectionR connection) : m_connection (connection) { ; }
            };

        //=======================================================================================
        // @bsiclass                                                    Steve.Wilson   7/17
        //=======================================================================================
        struct Forwarder
            {
        private:
            ClientConnectionR m_connection;

        public:
            Forwarder (ClientConnectionR connection) : m_connection (connection) { ; }

            void operator() (websocketpp::connection_hdl handle);
            void operator() (websocketpp::connection_hdl handle, websocketpp_server_t::message_ptr message);
            };

        EventCallback_T m_callback;
        TransportHandler_T m_handler;
        websocketpp_connection_ptr_t m_connection;
        Relay m_relay;
        std::ostream m_output;

        ClientConnection (websocketpp_connection_ptr_t const& connection, EventCallback_T const& callback, TransportHandler_T const& handler);

    public:
        IMODELJS_EXPORT websocketpp_connection_t const& GetConnection() const { return *m_connection; }
        IMODELJS_EXPORT websocketpp_connection_t& GetConnection() { return *m_connection; }

        IMODELJS_EXPORT bool Process (Utf8CP input, size_t offset, size_t length);
        IMODELJS_EXPORT bool Send (Utf8CP message, size_t length, websocketpp::frame::opcode::value code);
        };

    WebSockets() = delete;
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Uv
    {
public:
    DEFINE_POINTER_SUFFIX_TYPEDEFS (Handle)
    DEFINE_POINTER_SUFFIX_TYPEDEFS (AddressDescriptor)
    DEFINE_POINTER_SUFFIX_TYPEDEFS (IoStream)
    DEFINE_POINTER_SUFFIX_TYPEDEFS (Request)
    DEFINE_POINTER_SUFFIX_TYPEDEFS (WriteRequest)
    DEFINE_POINTER_SUFFIX_TYPEDEFS (ConnectRequest)
    DEFINE_POINTER_SUFFIX_TYPEDEFS (ShutdownRequest)
    DEFINE_POINTER_SUFFIX_TYPEDEFS (TcpHandle)
    DEFINE_POINTER_SUFFIX_TYPEDEFS (Status)
    DEFINE_POINTER_SUFFIX_TYPEDEFS (BindResult)
    DEFINE_POINTER_SUFFIX_TYPEDEFS (ConnectResult)
    DEFINE_POINTER_SUFFIX_TYPEDEFS (WriteResult)
    DEFINE_POINTER_SUFFIX_TYPEDEFS (ShutdownResult)
    DEFINE_POINTER_SUFFIX_TYPEDEFS (Server)

    DEFINE_REF_COUNTED_PTR (WriteRequest)
    DEFINE_REF_COUNTED_PTR (ConnectRequest)
    DEFINE_REF_COUNTED_PTR (ShutdownRequest)
    DEFINE_REF_COUNTED_PTR (TcpHandle)
    DEFINE_REF_COUNTED_PTR (Server)

    enum class IP { V4, V6 };
    typedef std::function<void(StatusCR, TcpHandlePtr)> ConnectCallback_T;
    typedef std::function<bool(StatusCR, uv_buf_t const&, size_t nread)> Read_Callback_T;
    typedef std::function<void(StatusCR)> Write_Callback_T;
    typedef std::function<void(StatusCR)> Shutdown_Callback_T;

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct AddressDescriptor
        {
    private:
        IP m_protocol;
        struct sockaddr_in m_addr4;
        struct sockaddr_in6 m_addr6;

    public:
        IMODELJS_EXPORT AddressDescriptor (Utf8CP address, uint16_t port, Uv::IP protocol, int& status);

        IMODELJS_EXPORT IP GetProtocol() const { return m_protocol; }
        IMODELJS_EXPORT const sockaddr* GetPointer() const;
        };

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct Handle : public RefCountedBase
        {
    private:
        uv_handle_t* m_handle;

        static void CloseHandler (uv_handle_t* handle);

        Handle (HandleCR other) = delete;
        HandleR operator= (HandleCR other) = delete;

    protected:
        template <typename T>
        T* GetPointerUnchecked() const { return reinterpret_cast<T*>(m_handle); }

    public:
        IMODELJS_EXPORT explicit Handle (uv_handle_t* handle);
        IMODELJS_EXPORT Handle (Handle&& other);
        IMODELJS_EXPORT ~Handle();

        IMODELJS_EXPORT uv_handle_t* GetPointer() const { return m_handle; }
        IMODELJS_EXPORT void Close();
        };

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct Request : public RefCountedBase
        {
    private:
        uv_req_t* m_request;

        Request (RequestCR other) = delete;
        RequestR operator= (RequestCR other) = delete;

    protected:
        IMODELJS_EXPORT Request (uv_req_t* request);
        IMODELJS_EXPORT Request (Request&& other);

        template <typename T>
        T* GetPointerUnchecked() const { return reinterpret_cast<T*>(m_request); }

    public:
        IMODELJS_EXPORT ~Request();

        IMODELJS_EXPORT uv_req_t* GetPointer() const { return m_request; }
        };

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct WriteRequest : public Request
        {
    private:
        Write_Callback_T m_callback;
        std::string m_payload;
        static void Handler (uv_write_t* req, int status);

        WriteRequest (uv_stream_t* handle, unsigned char* data, size_t length, Write_Callback_T const& callback, int& status);

    public:
        IMODELJS_EXPORT static WriteRequestPtr Create (IoStreamCR handle, unsigned char* data, size_t length, Write_Callback_T const& callback, int& status);
        IMODELJS_EXPORT uv_write_t* GetPointer() const;
        };

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct IoStream : public Handle
        {
    private:
        Read_Callback_T m_callback;

        static void ReadHandler (uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
        static void AllocHandler (uv_handle_t* handle, size_t suggestedSize, uv_buf_t* buf);

    public:
        IMODELJS_EXPORT explicit IoStream (uv_stream_t* handle) : Handle (reinterpret_cast<uv_handle_t*>(handle)) { ; }
        IMODELJS_EXPORT IoStream (IoStream&& other) : Handle (std::move (other)) { ; }

        IMODELJS_EXPORT uv_stream_t* GetPointer() const;
        IMODELJS_EXPORT uv_handle_t* GetHandlePointer() const;
        IMODELJS_EXPORT Status Read (Read_Callback_T const& callback);
        IMODELJS_EXPORT WriteResult Write (unsigned char* data, size_t length, Write_Callback_T const& callback);
        IMODELJS_EXPORT WriteResult Write (const char* data, size_t length, Write_Callback_T const& callback);
        IMODELJS_EXPORT ShutdownResult Shutdown();
        IMODELJS_EXPORT bool IsReadable() const;
        IMODELJS_EXPORT bool IsWritable() const;
        };

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct ShutdownRequest : public Request
        {
    private:
        Shutdown_Callback_T m_callback;

        static void Handler (uv_shutdown_t* req, int status);

        ShutdownRequest (IoStreamCR handle, int& status);

    public:
        IMODELJS_EXPORT static ShutdownRequestPtr Create (IoStreamCR handle, int& status);

        IMODELJS_EXPORT uv_shutdown_t* GetPointer() const;
        };

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct TcpHandle : public IoStream
        {
    public:
        IMODELJS_EXPORT TcpHandle (int& status);
        IMODELJS_EXPORT explicit TcpHandle (uv_tcp_t* handle) : IoStream (reinterpret_cast<uv_stream_t*>(handle)) { ; }
        IMODELJS_EXPORT TcpHandle (TcpHandle&& other) : IoStream (std::move (other)) { ; }

        IMODELJS_EXPORT uv_tcp_t* GetPointer() const;
        IMODELJS_EXPORT uv_stream_t* GetStreamPointer() const;
        IMODELJS_EXPORT Status SetKeepAlive (bool enable, uint32_t delay);
        IMODELJS_EXPORT Status SetSimultaneousAccepts (bool enable);
        IMODELJS_EXPORT Status SetNoDelay (bool enable);
        };

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct ConnectRequest : public Request
        {
    private:
        TcpHandlePtr m_handle;
        ConnectCallback_T m_callback;
        
        static void Handler (uv_connect_t* req, int status);

        ConnectRequest (AddressDescriptorCR address, TcpHandle&& handle, int& status, ConnectCallback_T const& callback);

    public:
        IMODELJS_EXPORT static ConnectRequestPtr Create (AddressDescriptorCR address, TcpHandle&& handle, int& status, ConnectCallback_T const& callback);

        IMODELJS_EXPORT uv_connect_t* GetPointer() const;
        };

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct Status
        {
    private:
        int m_code;

    public:
        IMODELJS_EXPORT Status (int code) : m_code (code) { ; }

        IMODELJS_EXPORT int GetCode() const { return m_code; }
        IMODELJS_EXPORT bool IsError() const { return GetCode() < 0; }
        };

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct Server : public TcpHandle
        {
    public:
        typedef std::function<void(StatusCR, TcpHandlePtr)> ListenCallback_T;

    private:
        ListenCallback_T m_callback;

        static void ListenHandler (uv_stream_t* server, int status);

    public:
        IMODELJS_EXPORT Server (TcpHandle&& handle) : TcpHandle (std::move (handle)) { ; }

        IMODELJS_EXPORT Status Listen (uint32_t backlog, ListenCallback_T const& callback);
        IMODELJS_EXPORT Status Accept (TcpHandleR connection);
        };

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct BindResult : public Status
        {
    private:
        ServerPtr m_server;
        uint16_t m_port;

    public:
        IMODELJS_EXPORT BindResult (int status, ServerP server, uint16_t port) : Status (status), m_server (server), m_port (port) { ; }

        IMODELJS_EXPORT ServerPtr GetServer() const { return m_server; }
        IMODELJS_EXPORT uint16_t GetPort() const { return m_port; }
        };

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct ConnectResult : public Status
        {
    private:
        ConnectRequestPtr m_request;

    public:
        IMODELJS_EXPORT ConnectResult (int status, ConnectRequestP request) : Status (status), m_request (request) { ; }

        IMODELJS_EXPORT ConnectRequestCP GetRequest() const { return m_request.get(); }
        };

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct WriteResult : public Status
        {
    private:
        WriteRequestPtr m_request;

    public:
        IMODELJS_EXPORT WriteResult (int status, WriteRequestP request) : Status (status), m_request (request) { ; }

        IMODELJS_EXPORT WriteRequestCP GetRequest() const { return m_request.get(); }
        };

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct ShutdownResult : public Status
        {
    private:
        ShutdownRequestPtr m_request;

    public:
        IMODELJS_EXPORT ShutdownResult (int status, ShutdownRequestP request) : Status (status), m_request (request) { ; }

        IMODELJS_EXPORT ShutdownRequestP GetRequest() const { return m_request.get(); }
        };

    Uv() = delete;

    IMODELJS_EXPORT static BindResult Bind (Utf8CP address, uint16_t port, IP protocol);
    IMODELJS_EXPORT static ConnectResult Connect (Utf8CP address, uint16_t port, IP protocol, ConnectCallback_T const& callback);
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct MobileGateway : public Extension
    {
private:
    static MobileGatewayPtr s_instance;

    WebSockets::ServerEndpoint m_endpoint;
    Uv::ServerPtr m_server;
    Uv::TcpHandlePtr m_client;
    WebSockets::ClientConnectionPtr m_connection;
    uint16_t m_port;
    Napi::ObjectReference m_exports;

protected:
    IMODELJS_EXPORT Utf8CP SupplyName() const override { return "@bentley/imodeljs-mobilegateway"; }
    IMODELJS_EXPORT Napi::Value ExportJsModule (Js::RuntimeR) override;

public:
    IMODELJS_EXPORT static MobileGatewayR GetInstance();
    IMODELJS_EXPORT static void Terminate();

    IMODELJS_EXPORT MobileGateway();
    IMODELJS_EXPORT uint16_t GetPort() const { return m_port; }
    };

END_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE

//__PUBLISH_SECTION_END__
