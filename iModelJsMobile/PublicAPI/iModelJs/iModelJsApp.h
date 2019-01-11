/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/iModelJs/iModelJsApp.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <iModelJs/iModelJs.h>

/** @namespace BentleyApi::iModelJs::App The iModel.js application framework. */
#define BEGIN_BENTLEY_IMODELJS_APP_NAMESPACE  BEGIN_BENTLEY_IMODELJS_NAMESPACE namespace App {
#define END_BENTLEY_IMODELJS_APP_NAMESPACE    } END_BENTLEY_IMODELJS_NAMESPACE

#define IMODELJS_APP_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_IMODELJS_APP_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_IMODELJS_APP_NAMESPACE

#define IMODELJS_APP_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_IMODELJS_APP_NAMESPACE struct _sname_; DEFINE_REF_COUNTED_PTR(_sname_) END_BENTLEY_IMODELJS_APP_NAMESPACE

BEGIN_BENTLEY_IMODELJS_APP_NAMESPACE





#ifdef WIP








































//=======================================================================================
//! A collection of application functionality in an environment.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct Activity
    {
    Activity() = delete;
    };

//=======================================================================================
//! An iModel.js application.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct Application
    {
    Application() = delete;
    };

//=======================================================================================
//! Acts on behalf of a service.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct Delegate
    {
    };

//=======================================================================================
//! An iModel.js environment.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct Environment
    {
private:
    static EnvironmentP s_instance;

    bool m_initialized;
    bool m_terminated;

    Environment (EnvironmentCR other) = delete;
    EnvironmentR operator= (EnvironmentCR other) = delete;
    Environment (Environment&& other) = delete;

protected:
    IMODELJS_EXPORT Environment();

    IMODELJS_EXPORT virtual void OnInitialized() { ; }
    IMODELJS_EXPORT virtual void OnTerminated() { ; }

public:
    IMODELJS_EXPORT static EnvironmentR GetInstance();

    IMODELJS_EXPORT virtual ~Environment();

    IMODELJS_EXPORT bool IsInitialized() const { return m_initialized; }
    IMODELJS_EXPORT bool IsTerminated() const { return m_terminated; }

    IMODELJS_EXPORT void Initialize();
    IMODELJS_EXPORT void Terminate();
    };

//=======================================================================================
//! An environment where the libuv framework is available.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
template <typename T>
struct ILibuvEnvironment : public T
    {
protected:
    ILibuvEnvironment() { ; }
    };

//=======================================================================================
//! An environment where Node.js is available.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
template <typename T>
struct INodeEnvironment : public ILibuvEnvironment<T>
    {
protected:
    INodeEnvironment() { ; }
    };

typedef INodeEnvironment<Environment> NodeEnvironmentSuper_T;

//=======================================================================================
//! The environment delivered by Node.js on a server host.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct NodeEnvironment : public NodeEnvironmentSuper_T
    {
    };

//=======================================================================================
//! An environment where web technologies are available.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
template <typename T>
struct IWebEnvironment : public T
    {
protected:
    IWebEnvironment() { ; }
    };

typedef IWebEnvironment<Environment> WebEnvironmentSuper_T;

//=======================================================================================
//! An environment where web rendering technologies like HTML, CSS, and WebGL are available.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct WebEnvironment : public WebEnvironmentSuper_T
    {
protected:
    IMODELJS_EXPORT WebEnvironment() { ; }
    };

//=======================================================================================
//! An environment where Web Workers technology is available.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct WebWorkersEnvironment : public WebEnvironmentSuper_T
    {
    WebWorkersEnvironment() = delete;
    };

//=======================================================================================
//! The environment delivered by a web browser host.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct WebBrowserEnvironment : public WebEnvironment
    {
    WebBrowserEnvironment() = delete;
    };

typedef INodeEnvironment<WebEnvironment> ElectronEnvironmentSuper_T;

//=======================================================================================
//! The environment delivered by the Electron framework on a desktop app host.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct ElectronEnvironment : public ElectronEnvironmentSuper_T
    {
    };

//=======================================================================================
//! The environment delivered by a web content control on a mobile app host.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct WebViewEnvironment : public WebEnvironment
    {
protected:
    IMODELJS_EXPORT WebViewEnvironment() { ; }
    };

//=======================================================================================
//! An Android android.webkit.WebView.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct AndroidWebViewEnvironment : public WebViewEnvironment
    {
    };

//=======================================================================================
//! An iOS WKWebView.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct iOSWebViewEnvironment : public WebViewEnvironment
    {
    };

//=======================================================================================
//! A Universal Windows Platform Windows.UI.Xaml.Controls.Web​View.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct UwpWebViewEnvironment : public WebViewEnvironment
    {
    };

typedef ILibuvEnvironment<Environment> EmbeddedEnvironmentSuper_T;

//=======================================================================================
//! An environment delivered by an embedded instance of a JavaScript env.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct EmbeddedEnvironment : public EmbeddedEnvironmentSuper_T
    {
public:
    DEFINE_POINTER_SUFFIX_TYPEDEFS (Message)
    DEFINE_REF_COUNTED_PTR (Message)

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   6/17
    //=======================================================================================
    struct Message : public RefCountedBase
        {
        friend struct EmbeddedEnvironment;

    private:
        static void CallbackHandler (uv_async_t* handle);
        static void CloseHandler (uv_handle_t* handle);

        uv_async_t* m_handle;

    protected:
        IMODELJS_EXPORT Message();

        IMODELJS_EXPORT virtual void Callback (uv_async_t* handle) = 0;

    public:
        IMODELJS_EXPORT virtual ~Message();
        };

private:
    static void ThreadEntry (void* arg);
    static void IdleHandler (uv_idle_t* handle);
    static void LocalHandleCloseHandler (uv_handle_t* handle);

    uv_thread_t m_thread;
    
    void RunThread();
    void HandleUvIdleCallback (uv_idle_t* handle);

protected:
    IMODELJS_EXPORT EmbeddedEnvironment();

    IMODELJS_EXPORT void OnInitialized() override;
    IMODELJS_EXPORT void OnTerminated() override;

    IMODELJS_EXPORT virtual void OnStart() { ; }
    IMODELJS_EXPORT virtual void OnStop() { ; }

public:
    IMODELJS_EXPORT void Post (MessagePtr message);
    };

//=======================================================================================
//! A Google V8 engine environment.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct V8Environment : public EmbeddedEnvironment
    {
private:
    v8::Isolate* m_isolate;
    v8::Locker* m_locker;
    v8::Persistent<v8::Context> m_context;

protected:
    IMODELJS_EXPORT void OnStart() override;
    IMODELJS_EXPORT void OnStop() override;

public:
    IMODELJS_EXPORT V8Environment();

    IMODELJS_EXPORT v8::Isolate* GetJsIsolate() const { return m_isolate; }
    IMODELJS_EXPORT v8::Persistent<v8::Context> const& GetJsContext() const { return m_context; }
    };

//=======================================================================================
//! A Microsoft Chakra engine environment.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct ChakraEnvironment : public EmbeddedEnvironment
    {
    };

//=======================================================================================
//! A WebKit JavaScriptCore engine environment.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct JscEnvironment : public EmbeddedEnvironment
    {
    };

//=======================================================================================
//! A chronological sequence of activity instances in a session.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct History
    {
    History() = delete;
    };

//=======================================================================================
//! An iModel.js hosting context.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct Host
    {
private:
    static HostP s_instance;

    bool m_initialized;
    bool m_terminated;

    Host (HostCR other) = delete;
    HostR operator= (HostCR other) = delete;
    Host (Host&& other) = delete;

protected:
    IMODELJS_EXPORT Host();

    IMODELJS_EXPORT virtual void OnInitialized() { ; }
    IMODELJS_EXPORT virtual void OnTerminated() { ; }

public:
    IMODELJS_EXPORT static HostR GetInstance();

    IMODELJS_EXPORT virtual ~Host();

    IMODELJS_EXPORT bool IsInitialized() const { return m_initialized; }
    IMODELJS_EXPORT bool IsTerminated() const { return m_terminated; }

    IMODELJS_EXPORT void Initialize();
    IMODELJS_EXPORT void Terminate();
    };

//=======================================================================================
//! A web browser session.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct InteractiveHost : public Host
    {
protected:
    IMODELJS_EXPORT InteractiveHost() { ; }
    };

//=======================================================================================
//! A host where user interaction is possible.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct WebBrowserHost : public InteractiveHost
    {
    WebBrowserHost() = delete;
    };

//=======================================================================================
//! A mobile application.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct MobileAppHost : public InteractiveHost
    {
protected:
    IMODELJS_EXPORT MobileAppHost() { ; }
    };

//=======================================================================================
//! An iOS application.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct iOSAppHost : public MobileAppHost
    {
    };

//=======================================================================================
//! An Android application.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct AndroidAppHost : public MobileAppHost
    {
    };

//=======================================================================================
//! A Universal Windows Platform application.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct UwpAppHost : public MobileAppHost
    {
    };

//=======================================================================================
//! A desktop application.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct DesktopAppHost : public InteractiveHost
    {
protected:
    IMODELJS_EXPORT DesktopAppHost() { ; }
    };

//=======================================================================================
//! A Windows application.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct WindowsDesktopAppHost : public InteractiveHost
    {
    IMODELJS_EXPORT WindowsDesktopAppHost();
    };

//=======================================================================================
//! A Mac application.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct MacDesktopAppHost : public InteractiveHost
    {
    };

//=======================================================================================
//! An HTTP server.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct ServerHost : public Host
    {
protected:
    IMODELJS_EXPORT ServerHost() { ; }
    };

//=======================================================================================
//! A Windows server.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct WindowsServerHost : public ServerHost
    {
    };

//=======================================================================================
//! A Linux server.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct LinuxServerHost : public ServerHost
    {
    };

//=======================================================================================
//! An iModel.js service.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct Service
    {
    Service() = delete;
    };

//=======================================================================================
//! A period of application usage on a host.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct Session
    {
    Session() = delete;
    };

//=======================================================================================
//! A mechanism for exchanging data between environments.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct Transport
    {
protected:
    IMODELJS_EXPORT Transport() { ; }
    };

//=======================================================================================
//! A transport that uses WebSockets technology.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct WebSocketsTransport : public Transport
    {
public:
    typedef websocketpp::server<websocketpp::config::core> websocketpp_server_t;

private:
    uv_tcp_t m_listener;
    websocketpp_server_t m_server;
    
    static void ListenerHandler (uv_stream_t* stream, int status);
    
    void HandleUvListenerCallback (uv_stream_t* stream, int status);
    
public:
    IMODELJS_EXPORT WebSocketsTransport();
    };

//=======================================================================================
//! A mechanism for exchanging data between environments.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct Connection : public RefCountedBase
    {
protected:
    IMODELJS_EXPORT Connection() { ; }
    };

//=======================================================================================
//! A transport that uses WebSockets technology.
// @bsiclass                                                    Steve.Wilson   6/17
//=======================================================================================
struct WebSocketsConnection : public Connection
    {
public:
    typedef websocketpp::connection<websocketpp::config::core> websocketpp_connection_t;

private:
    struct Relay : public std::streambuf
        {
    private:
        WebSocketsConnectionR m_connection;

    protected:
        std::streamsize xsputn (const char_type* s, std::streamsize count) override;

    public:
        Relay (WebSocketsConnectionR connection) : m_connection (connection) { ; }
        };

    struct Forwarder
        {
    private:
        WebSocketsConnectionR m_connection;

    public:
        Forwarder (WebSocketsConnectionR connection) : m_connection (connection) { ; }

        void operator() (websocketpp::connection_hdl handle);
        void operator() (websocketpp::connection_hdl handle, WebSocketsTransport::websocketpp_server_t::message_ptr message);
        };

    WebSocketsTransportR m_transport;
    uv_tcp_t m_connection;
    std::shared_ptr<websocketpp_connection_t> m_wsConnection;
    Relay m_outputRelay;
    std::ostream m_output;

    static void ConnectionAllocHandler (uv_handle_t* handle, size_t suggestedSize, uv_buf_t* buf);
    static void ClientConnectionHandleCloseHandler (uv_handle_t* handle);
    static void ConnectionReadHandler (uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
    static void ConnectionWriteHandler (uv_write_t* req, int status);

    void HandleUvConnectionReadCallback (uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
    void HandleUvConnectionAllocCallback (uv_handle_t* handle, size_t suggestedSize, uv_buf_t* buf);
    void HandleUvConnectionWriteCallback (uv_write_t* req, int status);
    void HandleWebsocketppOpenCallback (websocketpp::connection_hdl handle);
    void HandleWebsocketppMessageCallback (websocketpp::connection_hdl handle, WebSocketsTransport::websocketpp_server_t::message_ptr message);
    void Write (const std::streambuf::char_type* s, std::streamsize count);

public:
    IMODELJS_EXPORT WebSocketsConnection (WebSocketsTransportR transport, std::shared_ptr<websocketpp_connection_t> const& wsConnection, uv_stream_t* stream);
    };

#endif

END_BENTLEY_IMODELJS_APP_NAMESPACE

//__PUBLISH_SECTION_END__
