/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "iModelJsInternal.h"

BEGIN_BENTLEY_IMODELJS_JS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void* Runtime::GetInitializationData()
    {
    BeAssert (s_initialized);

    return s_initializationData;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Runtime::RegisterCallbackHandler (CallbackHandler_T& handler) const
    {
    m_callbackHandlers.push_back (&handler);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Runtime::OnCreate()
    {
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Runtime::OnDestroy()
    {
    for (auto handler : m_callbackHandlers)
        delete handler;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Runtime::EvaluateResult Runtime::EvaluateScript (Napi::String script, Napi::String identifier)
    {
    EvaluateResult result;
    napi_value resvalue;
    auto nstatus = napi_run_script(Env(), script, &resvalue);
    if (napi_ok == nstatus)
        {
        result.status = EvaluateStatus::Success;
        result.value = Napi::Value(Env(), resvalue);
        }
    else 
        {
        napi_extended_error_info const* info;
        napi_get_last_error_info(Env(), &info);
        result.status = EvaluateStatus::RuntimeError;
        result.message = info->error_message;
        result.value = Env().Undefined();
        }

    return result;
    }

Runtime::EvaluateResult Runtime::EvaluateScript (Napi::String script) {return EvaluateScript(script, Napi::String::New(Env(), ""));}
Runtime::EvaluateResult Runtime::EvaluateScript (Utf8CP str, Utf8CP id) {return EvaluateScript(Napi::String::New(Env(), str), Napi::String::New(Env(), id));}

END_BENTLEY_IMODELJS_JS_NAMESPACE

#if defined(BENTLEYCONFIG_OS_APPLE_IOS) || defined(BENTLEYCONFIG_OS_APPLE_MACOS)
#include <JavaScriptCore/JavaScriptCore.h>

BEGIN_BENTLEY_IMODELJS_JS_NAMESPACE
struct RuntimeInternal
    {
    friend struct Runtime;

private:
    Napi::Env* m_initEnv {};

public:
    Napi::Env& Env() {return *m_initEnv;}
    static Runtime* s_runtime;
    JSContextRef GetContext() const;
    RuntimeInternal (RuntimeR runtime);
    };

RuntimeInternal::RuntimeInternal (RuntimeR runtime)
    {
    s_runtime = &runtime;
    auto contextGroup = JSContextGroupCreate();
    auto jscEnv = new napi_env__(contextGroup);
    m_initEnv = new Napi::Env((napi_env)jscEnv);
    }

JSContextRef RuntimeInternal::GetContext() const
{
    const napi_env__* jscEnv = m_initEnv->operator napi_env__ *();
    return jscEnv->GetContext();
    
}

void Runtime::Initialize()
    {
    if (s_initialized)
        return;

    s_initialized = true;
    }

Runtime::Runtime (Utf8CP name, bool startDebugger, uint16_t debuggerPort)
    : m_name     (name),
      m_engine   (Engine::JSC),
      m_debugger (nullptr)
    {
        m_impl = new RuntimeInternal(*this);
    }

Runtime::~Runtime()
    {
    }

Napi::Env& Runtime::Env()
    {
    return m_impl->Env();
    }

void Runtime::StartDebugger (uint16_t port)
    {
    }

void Runtime::NotifyIdle()
    {
    }

Runtime& Runtime::GetRuntime(Napi::Env const& env)
    {
    return *(RuntimeInternal::s_runtime);
    }

bool Runtime::s_initialized = false;
void* Runtime::s_initializationData = nullptr;
Runtime* RuntimeInternal::s_runtime = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                      6/18
//---------------------------------------------------------------------------------------
JSContextRef Runtime::GetContext() const
    {
    return m_impl->GetContext();
    }

END_BENTLEY_IMODELJS_JS_NAMESPACE

#elif defined(BENTLEYCONFIG_OS_WINRT)

#error WIP

#elif defined(BENTLEYCONFIG_OS_WINDOWS) || defined(BENTLEYCONFIG_OS_ANDROID) || defined(BENTLEYCONFIG_OS_LINUX)

#include <google_v8/v8.h>
#include <google_v8/v8-debug.h>
#include <google_v8/v8-inspector.h>
#include <google_v8/libplatform/libplatform.h>
#include <mutex>
#include <condition_variable>

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
static v8_inspector::StringView createV8StringView (Utf8CP value)
    {
    if (value == nullptr)
        return v8_inspector::StringView();

    return v8_inspector::StringView (reinterpret_cast<const uint8_t*>(value), strlen (value));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
static v8::Platform* getV8Platform()
    {
    return reinterpret_cast<v8::Platform*>(BentleyApi::iModelJs::Js::Runtime::GetInitializationData());
    }

BEGIN_BENTLEY_IMODELJS_JS_NAMESPACE

DEFINE_POINTER_SUFFIX_TYPEDEFS (V8InspectorDebugger)

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct RuntimeInternal
    {
    friend struct Runtime;

public:
    static constexpr uint32_t IsolateDataSlot = 0;

private:
    RuntimeR m_runtime;
    v8::Isolate* m_isolate;
    v8::Locker* m_locker;
    v8::Persistent<v8::Context> m_context;
    Napi::Env* m_initEnv {};

public:
    static v8::Isolate* GetIsolate (RuntimeCR runtime) { return runtime.m_impl->m_isolate; }
    static v8::Local<v8::Context> GetContext (RuntimeCR runtime) { return v8::Local<v8::Context>::New (GetIsolate (runtime), runtime.m_impl->m_context); }
    static void DispatchFunctionCallback (v8::FunctionCallbackInfo<v8::Value> const& info);
    static RuntimeR GetRuntime (v8::Isolate*);
    static RuntimeR GetRuntime (Napi::Env const&);

    Napi::Env& Env() {return *m_initEnv;}

    RuntimeInternal (RuntimeR runtime);
    ~RuntimeInternal();
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct V8InspectorDebugger : public Runtime::Debugger
    {
public:
    DEFINE_POINTER_SUFFIX_TYPEDEFS (Client)
    DEFINE_POINTER_SUFFIX_TYPEDEFS (Channel)
    DEFINE_POINTER_SUFFIX_TYPEDEFS (Connection)

    typedef websocketpp::server<websocketpp::config::core> websocketpp_server_t;

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct Client : public v8_inspector::V8InspectorClient
        {
    private:
        V8InspectorDebuggerR m_debugger;
        bool m_runningV8Loop;

    public:
        Client (V8InspectorDebuggerR debugger) : m_debugger (debugger), m_runningV8Loop (false) { ; }

        void runMessageLoopOnPause (int contextGroupId) override;
        void quitMessageLoopOnPause() override;

        V8InspectorDebuggerR GetDebugger() const { return m_debugger; }
        };

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct Channel : public v8_inspector::V8Inspector::Channel
        {
    private:
        V8InspectorDebuggerR m_debugger;

        void Send (v8_inspector::StringView const& view);

    public:
        Channel (V8InspectorDebuggerR debugger) : m_debugger (debugger) { ; }

        void sendResponse (int callId, std::unique_ptr<v8_inspector::StringBuffer> message) override;
        void sendNotification (std::unique_ptr<v8_inspector::StringBuffer> message) override;
        void flushProtocolNotifications() override;

        V8InspectorDebuggerR GetDebugger() const { return m_debugger; }
        };

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct Connection
        {
        friend struct Channel;
        friend struct V8InspectorDebugger;

    public:
        typedef websocketpp::connection<websocketpp::config::core> websocketpp_connection_t;

    private:
        //=======================================================================================
        // @bsiclass                                                    Steve.Wilson   7/17
        //=======================================================================================
        struct Relay : public std::streambuf
            {
        private:
            ConnectionR m_connection;

        protected:
            std::streamsize xsputn (const char_type* s, std::streamsize count) override;

        public:
            Relay (ConnectionR connection) : m_connection (connection) { ; }
            };

        //=======================================================================================
        // @bsiclass                                                    Steve.Wilson   7/17
        //=======================================================================================
        struct Forwarder
            {
        private:
            ConnectionR m_connection;

        public:
            Forwarder (ConnectionR connection) : m_connection (connection) { ; }

            void operator() (websocketpp::connection_hdl handle);
            void operator() (websocketpp::connection_hdl handle, websocketpp_server_t::message_ptr message);
            };

        V8InspectorDebuggerR m_debugger;
        uv_tcp_t m_connection;
        std::shared_ptr<websocketpp_connection_t> m_wsConnection;
        Relay m_outputRelay;
        std::ostream m_output;

        static void CloseHandler (uv_handle_t* handle);
        static void AllocHandler (uv_handle_t* handle, size_t suggestedSize, uv_buf_t* buf);
        static void ReadHandler (uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
        static void WriteHandler (uv_write_t* req, int status);

        void Write (const std::streambuf::char_type* s, std::streamsize count);
        void Write (Utf8StringCR message);

        void HandleOpen (websocketpp::connection_hdl handle);
        void HandleMessage (websocketpp::connection_hdl handle, websocketpp_server_t::message_ptr message);

    public:
        Connection (V8InspectorDebuggerR debugger, std::shared_ptr<websocketpp_connection_t> const& wsConnection);

        V8InspectorDebuggerR GetDebugger() const { return m_debugger; }
        void Close();
        };

private:
    uv_thread_t m_thread;
    uv_loop_t m_eventLoop;
    uv_idle_t m_idler;
    std::vector<std::string> m_connectionMessages;
    std::mutex m_connectionMessagesMutex;
    std::condition_variable m_connectionMessagesCV;
    Client m_client;
    Channel m_channel;
    std::unique_ptr<v8_inspector::V8Inspector> m_inspector;
    std::unique_ptr<v8_inspector::V8InspectorSession> m_session;
    uv_tcp_t m_listener;
    websocketpp_server_t m_server;
    ConnectionP m_connection;
    std::atomic<bool> m_ready;
    std::atomic<bool> m_eventLoopRunning;
    std::atomic<bool> m_terminating;

    static void ThreadEntry (void* arg);
    
    void ThreadMain();

    static void IdleHandler (uv_idle_t* handle);
    static void EventLoopCallbackAsyncHandler (uv_async_t* handle);
    static void EventLoopAsyncCloseHandler (uv_handle_t* handle);
    static void ListenHandler (uv_stream_t* stream, int status);

    void OnIdle();

    void HandleConnection();
    void NotifyConnectionClosed (ConnectionR connection);
    void NotifyReady() { m_ready = true; }
    void EnqueueConnectionMessage (std::string const& message);
    void ConsumeConnectionMessage (std::string const& message);
    void ConsumeConnectionMessages();
    void SendToConnection (Utf8StringCR message);

public:
    V8InspectorDebugger (RuntimeR runtime, uint16_t port);
    ~V8InspectorDebugger() override;

    bool IsReady() const override { return m_ready; }

    uv_loop_t* GetEventLoop() { return &m_eventLoop; }
    uv_loop_t const* GetEventLoop() const { return &m_eventLoop; }
    ClientCR GetClient() const { return m_client; }
    ClientR GetClient() { return m_client; }
    ChannelCR GetChannel() const { return m_channel; }
    ChannelR GetChannel() { return m_channel; }
    v8_inspector::V8Inspector& GetInspector() const { return *m_inspector; }
    v8_inspector::V8InspectorSession& GetSession() const { return *m_session; }
    uv_tcp_t* GetListener() { return &m_listener; }
    ConnectionP GetConnection() const { return m_connection; }
    bool PostToEventLoop (ServicesTier::Host::EventLoopCallback_T const& callback);
    void NotifyIdle() override;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Runtime::Initialize()
    {
    if (s_initialized)
        return;

    s_initialized = true;

    auto platform = v8::platform::CreateDefaultPlatform();
    s_initializationData = platform;

    v8::V8::InitializePlatform (platform);
    v8::V8::Initialize();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Runtime::Runtime (Utf8CP name, bool startDebugger, uint16_t debuggerPort)
    : m_name     (name),
      m_engine   (Engine::V8),
      m_debugger (nullptr)
    {
    Initialize();
    m_impl = new RuntimeInternal (*this);
    
    if (startDebugger)
        StartDebugger (debuggerPort);

    OnCreate();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Runtime::~Runtime()
    {
    OnDestroy();

    if (m_debugger != nullptr)
        delete m_debugger;

    delete m_impl;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/17
//---------------------------------------------------------------------------------------
Napi::Env& Runtime::Env() {return m_impl->Env();}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Runtime::StartDebugger (uint16_t port)
    {
    if (m_debugger == nullptr)
        m_debugger = new V8InspectorDebugger (*this, port);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Runtime::NotifyIdle()
    {
    auto platform = getV8Platform();
    auto isolate = RuntimeInternal::GetIsolate (*this);
    
    if (GetDebugger())
        GetDebugger()->NotifyIdle();
    while (v8::platform::PumpMessageLoop (platform, isolate)) { ; }
    v8::platform::RunIdleTasks (platform, isolate, 50.0 / 1000);
    isolate->RunMicrotasks();
    }

#ifdef OLD_WAY
//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Runtime::EvaluateResult Runtime::EvaluateScript (Utf8CP script, Utf8CP identifier)
    {
    EvaluateResult result;

    auto isolate = RuntimeInternal::GetIsolate (*this);

    v8::Local<v8::String> identifierHandle;
    if (identifier != nullptr)
        {
        auto maybeIdentifier = v8::String::NewFromUtf8 (isolate, identifier, v8::NewStringType::kNormal);
        if (!maybeIdentifier.IsEmpty())
            identifierHandle = maybeIdentifier.ToLocalChecked();
        }

    v8::Local<v8::String> scriptHandle;
    auto maybeScript = v8::String::NewFromUtf8 (isolate, script, v8::NewStringType::kNormal);
    if (!maybeScript.IsEmpty())
        scriptHandle = maybeScript.ToLocalChecked();

    v8::ScriptOrigin origin (identifierHandle);
    auto compiledScript = v8::Script::Compile (RuntimeInternal::GetContext (*this), scriptHandle, &origin);
    if (compiledScript.IsEmpty())
        {
        result.status = EvaluateStatus::ParseError;
        }
    else
        {
        v8::TryCatch tryCatch (isolate);
        v8::Local<v8::Value> runResult = compiledScript.ToLocalChecked()->Run();

        if (tryCatch.HasCaught() || tryCatch.HasTerminated())
            {
            result.status = EvaluateStatus::RuntimeError;
            result.message = *v8::String::Utf8Value (tryCatch.Exception());
            result.trace = *v8::String::Utf8Value (tryCatch.StackTrace());
            }
        else
            {
            result.status = EvaluateStatus::Success;
            result.value = *runResult;
            }
        }

    return result;
    }
#endif

// Keep this consistent with node_api.cc from GitHub
struct napi_env__ {
  explicit napi_env__(v8::Isolate* _isolate): isolate(_isolate),
      has_instance_available(true), last_error() {}
  ~napi_env__() {
    last_exception.Reset();
    has_instance.Reset();
    wrap_template.Reset();
    function_data_template.Reset();
    accessor_data_template.Reset();
  }
  v8::Isolate* isolate;
  v8::Persistent<v8::Value> last_exception;
  v8::Persistent<v8::Value> has_instance;
  v8::Persistent<v8::ObjectTemplate> wrap_template;
  v8::Persistent<v8::ObjectTemplate> function_data_template;
  v8::Persistent<v8::ObjectTemplate> accessor_data_template;
  bool has_instance_available;
  napi_extended_error_info last_error;
  int open_handle_scopes = 0;
};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
RuntimeInternal::RuntimeInternal (RuntimeR runtime)
    : m_runtime (runtime)
    {
    v8::Isolate::CreateParams isolateParams;
    isolateParams.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    m_isolate = v8::Isolate::New (isolateParams);
    m_isolate->SetData (IsolateDataSlot, &runtime);
    
    m_locker = new v8::Locker (m_isolate);
    m_isolate->Enter();

    v8::HandleScope scope (m_isolate);

    v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New();
    v8::Local<v8::Context> context = v8::Context::New (m_isolate, nullptr, global);
    m_context.Reset (m_isolate, context);

    context->Enter();

    // Set up the napi_env that is the wrapper for the isolate in the napi layer
    auto v8env = new napi_env__(m_isolate);
    m_initEnv = new Napi::Env((napi_env)v8env);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/17
//---------------------------------------------------------------------------------------
Runtime& RuntimeInternal::GetRuntime(Napi::Env const& env)
{
    napi_env__* v8env = *(napi_env__**)&env;
    return *(Runtime*)v8env->isolate->GetData(IsolateDataSlot);
}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      12/17
//---------------------------------------------------------------------------------------
Runtime& Runtime::GetRuntime(Napi::Env const& env)
{
    return RuntimeInternal::GetRuntime(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
RuntimeInternal::~RuntimeInternal()
    {
    //Nested to avoid destroying handle scope after isolate
        {
        v8::HandleScope scope (m_isolate);
        m_context.Get (m_isolate)->Exit();
        }

    m_context.Reset();
    m_isolate->ContextDisposedNotification();

    while (!m_isolate->IdleNotification (1000)) { ; }
    delete m_locker;
    m_isolate->Exit();
    m_isolate->Dispose();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
RuntimeR RuntimeInternal::GetRuntime (v8::Isolate* isolate)
    {
    return *reinterpret_cast<RuntimeP>(isolate->GetData (IsolateDataSlot));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
V8InspectorDebugger::V8InspectorDebugger (RuntimeR runtime, uint16_t port)
    : Debugger      (runtime, port),
      m_client      (*this),
      m_channel     (*this),
      m_connection  (nullptr),
      m_ready       (false),
      m_terminating (false)
    {
    m_server.clear_access_channels (websocketpp::log::alevel::all);
    m_server.clear_error_channels (websocketpp::log::elevel::all);

    Napi::HandleScope scope (GetRuntime().Env());
    m_inspector = v8_inspector::V8Inspector::create (RuntimeInternal::GetIsolate (GetRuntime()), &m_client);
    m_session = GetInspector().connect (1, &m_channel, v8_inspector::StringView());
    GetInspector().contextCreated (v8_inspector::V8ContextInfo (RuntimeInternal::GetContext (GetRuntime()), 1, createV8StringView (GetRuntime().GetName())));

    auto threadCreated = uv_thread_create (&m_thread, &ThreadEntry, this);
    BeAssert (threadCreated >= 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
V8InspectorDebugger::~V8InspectorDebugger()
    {
    PostToEventLoop ([this]() { uv_stop (GetEventLoop()); });
    m_terminating = true;

    auto joined = uv_thread_join (&m_thread);
    BeAssert (joined >= 0);

    v8::HandleScope scope (RuntimeInternal::GetIsolate (GetRuntime()));
    GetInspector().contextDestroyed (RuntimeInternal::GetContext (GetRuntime()));
    m_session.release();
    m_inspector.release();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::ThreadEntry (void* arg)
    {
    BeThreadUtilities::SetCurrentThreadName ("V8InspectorDebuggerThread");
    reinterpret_cast<V8InspectorDebuggerP>(arg)->ThreadMain();
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::ThreadMain()
    {
    m_eventLoop.data = this;
    m_idler.data     = this;
    m_listener.data  = this;

    auto status = uv_loop_init (&m_eventLoop);
    if (status < 0)
        {
        BeAssert (false);
        return;
        }

    status = uv_idle_init (GetEventLoop(), &m_idler);
    if (status < 0)
        {
        BeAssert (false);
        return;
        }

    status = uv_idle_start (&m_idler, &IdleHandler);
    if (status < 0)
        {
        BeAssert (false);
        return;
        }

    status = uv_tcp_init (GetEventLoop(), &m_listener);
    if (status < 0)
        {
        BeAssert (false);
        return;
        }

    uv_tcp_nodelay (&m_listener, true);

    struct sockaddr_in addr;
    status = uv_ip4_addr ("127.0.0.1", GetPort(), &addr);
    if (status < 0)
        {
        BeAssert (false);
        return;
        }

    status = uv_tcp_bind (&m_listener, reinterpret_cast<const sockaddr*>(&addr), 0);
    if (status < 0)
        {
        BeAssert (false);
        return;
        }

    status = uv_listen ((uv_stream_t*) &m_listener, 1024, &ListenHandler);
    if (status < 0)
        {
        BeAssert (false);
        return;
        }

    m_eventLoopRunning = true;
    uv_run (GetEventLoop(), UV_RUN_DEFAULT);

    status = uv_idle_stop (&m_idler);
    BeAssert (status >= 0);

    uv_close ((uv_handle_t*) &m_idler, nullptr);

    if (m_connection != nullptr)
        m_connection->Close();

    uv_close ((uv_handle_t*) &m_listener, nullptr);

    size_t c = 0;
    while (c < 10 && uv_run (GetEventLoop(), UV_RUN_NOWAIT) != 0) { ++c; } 

    status = uv_loop_close (GetEventLoop());
    BeAssert (status >= 0);

    m_eventLoopRunning = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool V8InspectorDebugger::PostToEventLoop (ServicesTier::Host::EventLoopCallback_T const& callback)
    {
    if (!m_terminating && m_eventLoopRunning)
        {
        uv_async_t* handle = (uv_async_t*) malloc (sizeof (uv_async_t));

        auto initializedHandle = uv_async_init (GetEventLoop(), handle, &EventLoopCallbackAsyncHandler);
        BeAssert (initializedHandle >= 0);

        if (initializedHandle >= 0)
            {
            auto callbackCopy = new ServicesTier::Host::EventLoopCallback_T (callback);
            handle->data = callbackCopy;

            auto sent = uv_async_send (handle);
            BeAssert (sent >= 0);

            if (sent >= 0)
                return true;
            }
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::NotifyIdle()
    {
    ConsumeConnectionMessages();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::EnqueueConnectionMessage (std::string const& message)
    {
    std::unique_lock<std::mutex> lock (m_connectionMessagesMutex);
    m_connectionMessages.push_back (message);
    lock.unlock();
    m_connectionMessagesCV.notify_one();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::ConsumeConnectionMessages()
    {
    std::unique_lock<std::mutex> lock (m_connectionMessagesMutex);

    for (auto& message : m_connectionMessages)
        ConsumeConnectionMessage (message);

    m_connectionMessages.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::ConsumeConnectionMessage (std::string const& message)
    {
    if (message.find ("Runtime.runIfWaitingForDebugger") != std::string::npos)
        NotifyReady();

    v8_inspector::StringView messageView (reinterpret_cast<const uint8_t*>(message.c_str()), message.size());
    GetSession().dispatchProtocolMessage (messageView);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::SendToConnection (Utf8StringCR message)
    {
    PostToEventLoop ([this, message]()
        {
        auto connection = GetConnection();
        BeAssert (connection != nullptr);

        connection->Write (message);
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::IdleHandler (uv_idle_t* handle)
    {
    BeAssert (handle->data != nullptr);

    auto& instance = *reinterpret_cast<V8InspectorDebuggerP>(handle->data);
    instance.OnIdle();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::OnIdle()
    {
    if (m_terminating)
        uv_stop (GetEventLoop()); //in case ~V8InspectorDebugger sets m_terminating before ThreadMain sets m_eventLoopRunning
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::EventLoopCallbackAsyncHandler (uv_async_t* handle)
    {
    auto callback = static_cast<ServicesTier::Host::EventLoopCallback_T*>(handle->data);
    (*callback)();
    delete callback;

    uv_close ((uv_handle_t*) handle, &EventLoopAsyncCloseHandler);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::EventLoopAsyncCloseHandler (uv_handle_t* handle)
    {
    free (handle);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::ListenHandler (uv_stream_t* stream, int status)
    {
    BeAssert (stream->data != nullptr);

    if (status < 0)
        {
        BeAssert (false);
        return;
        }

    auto& instance = *reinterpret_cast<V8InspectorDebuggerP>(stream->data);
    BeAssert (stream == reinterpret_cast<uv_stream_t*>(&instance.m_listener));
    instance.HandleConnection();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::HandleConnection()
    {
    if (m_connection != nullptr)
        return;

    m_connection = new Connection (*this, m_server.get_connection()); //deleted by Connection::CloseHandler
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::NotifyConnectionClosed (ConnectionR connection)
    {
    BeAssert (m_connection == &connection);
    
    m_connection = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::Client::runMessageLoopOnPause (int contextGroupId)
    {
    BeAssert (!m_runningV8Loop);

    m_runningV8Loop = true;
    while (m_runningV8Loop)
        {
        GetDebugger().ConsumeConnectionMessages();
        while (v8::platform::PumpMessageLoop (getV8Platform(), RuntimeInternal::GetIsolate (GetDebugger().GetRuntime()))) { ; }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::Client::quitMessageLoopOnPause()
    {
    BeAssert (m_runningV8Loop);

    m_runningV8Loop = false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::Channel::sendResponse (int callId, std::unique_ptr<v8_inspector::StringBuffer> message)
    {
    Send (message->string());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::Channel::sendNotification (std::unique_ptr<v8_inspector::StringBuffer> message)
    {
    Send (message->string());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::Channel::flushProtocolNotifications()
    {
    ;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::Channel::Send (v8_inspector::StringView const& view)
    {
    Utf8String message;

    if (view.is8Bit())
        {
        message.assign (reinterpret_cast<const char*>(view.characters8()), view.length());
        }
    else
        {
        WString messageW (reinterpret_cast<const wchar_t*>(view.characters16()), view.length());
        message = Utf8String (messageW);
        }

    GetDebugger().SendToConnection (message);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
std::streamsize V8InspectorDebugger::Connection::Relay::xsputn (const char_type* s, std::streamsize count)
    {
    m_connection.Write (s, count);

    return count;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::Connection::Forwarder::operator() (websocketpp::connection_hdl handle)
    {
    m_connection.HandleOpen (handle);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::Connection::Forwarder::operator() (websocketpp::connection_hdl handle, websocketpp_server_t::message_ptr message)
    {
    m_connection.HandleMessage (handle, message);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
V8InspectorDebugger::Connection::Connection (V8InspectorDebuggerR debugger, std::shared_ptr<websocketpp_connection_t> const& wsConnection)
    : m_debugger     (debugger),
      m_wsConnection (wsConnection),
      m_outputRelay  (*this),
      m_output       (&m_outputRelay)
    {
    m_connection.data = this;

    Forwarder forwarder (*this);
    m_wsConnection->set_message_handler (forwarder);
    m_wsConnection->set_open_handler (forwarder);
    
    m_wsConnection->register_ostream (&m_output);
    m_wsConnection->start();

    auto status = uv_tcp_init (GetDebugger().GetEventLoop(), &m_connection);
    auto closeOnError = status >= 0;

    if (status >= 0)
        {
        uv_tcp_nodelay (&m_connection, true);

        status = uv_accept (reinterpret_cast<uv_stream_t*>(GetDebugger().GetListener()), reinterpret_cast<uv_stream_t*>(&m_connection));
        if (status >= 0)
            {
            status = uv_read_start (reinterpret_cast<uv_stream_t*>(&m_connection), &AllocHandler, &ReadHandler);
            if (status >= 0)
                return;
            }
        }

    BeAssert (status < 0);
    BeAssert (false);

    if (closeOnError)
        Close();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::Connection::CloseHandler (uv_handle_t* handle)
    {
    BeAssert (handle->data != nullptr);

    auto instance = reinterpret_cast<ConnectionP>(handle->data);
    instance->GetDebugger().NotifyConnectionClosed (*instance);
    delete instance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::Connection::AllocHandler (uv_handle_t* handle, size_t suggestedSize, uv_buf_t* buf)
    {
    buf->base = (char*) malloc (suggestedSize);
    buf->len = (ULONG) suggestedSize;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::Connection::ReadHandler (uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
    {
    BeAssert (stream->data != nullptr);

    auto& instance = *reinterpret_cast<ConnectionP>(stream->data);

    if (nread <= 0)
        return;

    instance.m_wsConnection->read_all (buf->base, nread);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::Connection::WriteHandler (uv_write_t* req, int status)
    {
    free (req);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::Connection::HandleOpen (websocketpp::connection_hdl handle)
    {
    ;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::Connection::HandleMessage (websocketpp::connection_hdl handle, websocketpp_server_t::message_ptr message)
    {
    GetDebugger().EnqueueConnectionMessage (message->get_payload());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::Connection::Write (const std::streambuf::char_type* s, std::streamsize count)
    {
    auto req = reinterpret_cast<uv_write_t*>(malloc (sizeof (uv_write_t)));
    req->data = this;

    auto buffer = uv_buf_init ((char*) s, (unsigned int) count);

    auto status = uv_write (req, reinterpret_cast<uv_stream_t*>(&m_connection), &buffer, 1, &WriteHandler);
    BeAssert (status >= 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::Connection::Write (Utf8StringCR message)
    {
    m_wsConnection->send (message.c_str(), message.length(), websocketpp::frame::opcode::text);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void V8InspectorDebugger::Connection::Close()
    {
    uv_close (reinterpret_cast<uv_handle_t*>(&m_connection), &CloseHandler);
    }

bool Runtime::s_initialized = false;
void* Runtime::s_initializationData = nullptr;
        
END_BENTLEY_IMODELJS_JS_NAMESPACE

#endif
