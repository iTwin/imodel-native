/*--------------------------------------------------------------------------------------+
|
|     $Source: Utilities.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
    Scope scope (*this);

    m_promiseConstructor.Assign (*this, GetGlobal().Get ("Promise"));
    m_errorConstructor.Assign   (*this, GetGlobal().Get ("Error"));

    auto harvesterEvaluation = EvaluateScript (u8R"(
        (function() {
            let f = function (resolve, reject) {
                f.__bentley_imodeljs_lastResolve = resolve;
                f.__bentley_imodeljs_lastReject = reject;
            };
            
            return f;
        })();
    )");
    
    BeAssert (harvesterEvaluation.status == EvaluateStatus::Success);
    m_promiseHarvester.Assign (*this, harvesterEvaluation.value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Runtime::OnDestroy()
    {
    for (auto handler : m_callbackHandlers)
        delete handler;

    m_promiseConstructor.Clear();
    m_errorConstructor.Clear();
    m_promiseHarvester.Clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Object Runtime::CreatePromise (PromiseR destination) const
    {
    auto constructor = m_promiseConstructor.Get().AsFunction();
    auto harvester = m_promiseHarvester.Get().AsNoTypeCheck<Object>();
    
    auto instance = constructor.Construct (harvester);

    BeAssert (harvester.HasOwn ("__bentley_imodeljs_lastResolve") && harvester.HasOwn ("__bentley_imodeljs_lastReject"));
    destination.m_resolveFunction.Assign (*this, harvester.Get ("__bentley_imodeljs_lastResolve", true));
    destination.m_rejectFunction.Assign  (*this, harvester.Get ("__bentley_imodeljs_lastReject", true));

    return instance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Object Scope::CreateError (ValueCR value)
    {
    return m_runtime.GetErrorConstructor().Get().AsFunction().Construct (value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Object Promise::Initialize (ScopeR scope)
    {
    if (IsInitialized())
        {
        BeAssert (false);
        return scope.CreateObject();
        }
    
    return scope.GetRuntime().CreatePromise (*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Promise::Resolve (ValueCR value)
    {
    BeAssert (!m_resolveFunction.IsEmpty());

    m_resolveFunction.Get().AsFunction() (m_resolveFunction.GetRuntime()->GetGlobal(), value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Promise::Reject (ValueCR reason)
    {
    BeAssert (!m_rejectFunction.IsEmpty());

    m_rejectFunction.Get().AsFunction() (m_rejectFunction.GetRuntime()->GetGlobal(), reason);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
RuntimeCP Promise::GetRuntime() const
    {
    return m_resolveFunction.GetRuntime();
    }

END_BENTLEY_IMODELJS_JS_NAMESPACE

#ifdef BENTLEYCONFIG_OS_APPLE_IOS

#error WIP

#elifdef BENTLEYCONFIG_OS_WINRT

#error WIP

#else

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

static_assert (sizeof (Value) == sizeof (v8::Local<v8::Value>) && alignof (Value) == alignof (v8::Local<v8::Value>), "Js::Value does not match v8::Local<v8::Value>");

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

public:
    static v8::Isolate* GetIsolate (RuntimeCR runtime) { return runtime.m_impl->m_isolate; }
    static v8::Local<v8::Context> GetContext (RuntimeCR runtime) { return v8::Local<v8::Context>::New (GetIsolate (runtime), runtime.m_impl->m_context); }
    static void DispatchFunctionCallback (v8::FunctionCallbackInfo<v8::Value> const& info);
    static RuntimeR GetRuntime (v8::Isolate* isolate);
    static v8::FunctionCallbackInfo<v8::Value> const& GetInfo (CallbackInfoCR info);

    RuntimeInternal (RuntimeR runtime);
    ~RuntimeInternal();
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct ScopeInternal
    {
private:
    v8::HandleScope m_v8Scope;

public:
    static v8::Local<v8::Value> ToHandle (ValueCR value);

    ScopeInternal (ScopeR scope);
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

END_BENTLEY_IMODELJS_JS_NAMESPACE

BEGIN_BENTLEY_IMODELJS_JS_NAMESPACE

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
    
    GetDebugger()->NotifyIdle();
    while (v8::platform::PumpMessageLoop (platform, isolate)) { ; }
    v8::platform::RunIdleTasks (platform, isolate, 50.0 / 1000);
    isolate->RunMicrotasks();
    }

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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Object Runtime::GetGlobal() const
    {
    return *RuntimeInternal::GetContext (*this)->Global();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Runtime::ThrowException (ValueCR value)
    {
    auto isolate = RuntimeInternal::GetIsolate (*this);
    auto error = m_errorConstructor.Get().AsFunction().Construct (value);
    isolate->ThrowException (error.IsEmpty() ? v8::Local<v8::Value>::Cast (v8::Null (isolate)) : ScopeInternal::ToHandle (error));
    }

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
v8::FunctionCallbackInfo<v8::Value> const& RuntimeInternal::GetInfo (CallbackInfoCR info)
    {
    return *reinterpret_cast<v8::FunctionCallbackInfo<v8::Value> const*>(info.m_data);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void RuntimeInternal::DispatchFunctionCallback (v8::FunctionCallbackInfo<v8::Value> const& info)
    {
    auto& runtime = GetRuntime (info.GetIsolate());
    auto handler = reinterpret_cast<CallbackHandler_T*>(v8::Local<v8::External>::Cast (info.Data())->Value());
    
    CallbackInfo handlerInfo (runtime, (void*)&info);
    auto result = (*handler) (handlerInfo);

    info.GetReturnValue().Set (ScopeInternal::ToHandle (result));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Reference::Reference (RuntimeCR runtime, ValueCR value)
    : m_runtime (&runtime),
      m_data    (nullptr)
    {
    PerformAssign (value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Reference::Assign (ValueCR value)
    {
    BeAssert (m_runtime != nullptr);

    PerformAssign (value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Reference::Assign (RuntimeCR runtime, ValueCR value)
    {
    BeAssert (m_runtime == nullptr);

    m_runtime = &runtime;
    PerformAssign (value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Reference::PerformAssign (ValueCR value)
    {
    BeAssert (!value.IsEmpty());

    if (m_data != nullptr)
        {
        BeAssert (false);
        return;
        }

    auto handle = new v8::Persistent<v8::Value> (RuntimeInternal::GetIsolate (*m_runtime), ScopeInternal::ToHandle (value));
    m_data = handle;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Value Reference::Get() const
    {
    BeAssert (!IsEmpty());

    auto handle = reinterpret_cast<v8::Persistent<v8::Value>*>(m_data);
    return *v8::Local<v8::Value>::New (RuntimeInternal::GetIsolate (*m_runtime), *handle);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Reference::Clear()
    {
    if (m_data != nullptr)
        {
        auto handle = reinterpret_cast<v8::Persistent<v8::Value>*>(m_data);
        handle->Reset();
        delete handle;

        m_data = nullptr;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Reference::~Reference()
    {
    Clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Scope::Scope (RuntimeCR runtime, bool create)
    : m_runtime (runtime)
    {
    m_impl = create ? (new ScopeInternal (*this)) : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Scope::Scope (RuntimeCR runtime)
    : Scope (runtime, true)
    {
    ;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Scope::~Scope()
    {
    if (m_impl != nullptr)
        delete m_impl;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Undefined Scope::CreateUndefined()
    {
    return *v8::Undefined (RuntimeInternal::GetIsolate (m_runtime));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Null Scope::CreateNull()
    {
    return *v8::Null (RuntimeInternal::GetIsolate (m_runtime));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Boolean Scope::CreateBoolean (bool value)
    {
    return *v8::Boolean::New (RuntimeInternal::GetIsolate (m_runtime), value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Number Scope::CreateNumber (double value)
    {
    return *v8::Number::New (RuntimeInternal::GetIsolate (m_runtime), value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
String Scope::CreateString (Utf8CP value)
    {
    auto handle = v8::String::NewFromUtf8 (RuntimeInternal::GetIsolate (m_runtime), value, v8::NewStringType::kNormal);
    if (!handle.IsEmpty())
        return *handle.ToLocalChecked();

    return *v8::Local<v8::String>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Object Scope::CreateObject()
    {
    return *v8::Object::New (RuntimeInternal::GetIsolate (m_runtime));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Array Scope::CreateArray()
    {
    return *v8::Array::New (RuntimeInternal::GetIsolate (m_runtime));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
ArrayBuffer Scope::CreateArrayBuffer (void* data, size_t length)
    {
    return *v8::ArrayBuffer::New (RuntimeInternal::GetIsolate (m_runtime), data, length);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
ArrayBuffer Scope::CreateArrayBuffer (size_t length)
    {
    return *v8::ArrayBuffer::New (RuntimeInternal::GetIsolate (m_runtime), length);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Callback Scope::CreateCallback (CallbackHandler_T const& handler)
    {
    auto handlerCopy = new CallbackHandler_T (handler);
    m_runtime.RegisterCallbackHandler (*handlerCopy);

    auto handle = v8::Function::New (RuntimeInternal::GetContext (m_runtime),
                                     &RuntimeInternal::DispatchFunctionCallback,
                                     v8::External::New (RuntimeInternal::GetIsolate (m_runtime), handlerCopy));

    if (!handle.IsEmpty())
        return *handle.ToLocalChecked();
    else
        return *v8::Local<v8::Function>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
External Scope::CreateExternal (void* data)
    {
    auto isolate = RuntimeInternal::GetIsolate (m_runtime);

    auto tmpl = v8::ObjectTemplate::New (isolate);
    tmpl->SetInternalFieldCount (1);

    auto handle = tmpl->NewInstance (RuntimeInternal::GetContext (m_runtime));
    if (!handle.IsEmpty())
        {
        auto local = handle.ToLocalChecked();
        local->SetInternalField (0, v8::External::New (isolate, data));

        return *local;
        }
    else
        {
        return *v8::Local<v8::Object>();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Pointer Scope::CreatePointer (void* data)
    {
    return *v8::External::New (RuntimeInternal::GetIsolate (m_runtime), data);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
ScopeInternal::ScopeInternal (ScopeR scope)
    : m_v8Scope (RuntimeInternal::GetIsolate (scope.m_runtime))
    {
    ;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
v8::Local<v8::Value> ScopeInternal::ToHandle (ValueCR value)
    {
    v8::Local<v8::Value> v8Value;
    memcpy (&v8Value, &value, sizeof (value));

    return v8Value;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool Value::operator== (ValueCR b) const
    {
    return ScopeInternal::ToHandle (*this) == ScopeInternal::ToHandle (b);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Undefined Value::AsUndefined() const
    {
    BeAssert (!IsEmpty() && IsUndefined());

    return m_data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Null Value::AsNull() const
    {
    BeAssert (!IsEmpty() && IsNull());

    return m_data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Boolean Value::AsBoolean() const
    {
    BeAssert (!IsEmpty() && IsBoolean());

    return m_data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Number Value::AsNumber() const
    {
    BeAssert (!IsEmpty() && IsNumber());

    return m_data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
String Value::AsString() const
    {
    BeAssert (!IsEmpty() && IsString());

    return m_data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Object Value::AsObject() const
    {
    BeAssert (!IsEmpty() && IsObject());

    return m_data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Array Value::AsArray() const
    {
    BeAssert (!IsEmpty() && IsArray());

    return m_data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
ArrayBuffer Value::AsArrayBuffer() const
    {
    BeAssert (!IsEmpty() && IsArrayBuffer());

    return m_data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Function Value::AsFunction() const
    {
    BeAssert (!IsEmpty() && IsFunction());

    return m_data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Callback Value::AsCallback() const
    {
    BeAssert (!IsEmpty() && IsFunction());

    return m_data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
External Value::AsExternal() const
    {
    BeAssert (!IsEmpty() && IsExternal());

    return m_data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Pointer Value::AsPointer() const
    {
    BeAssert (!IsEmpty() && IsPointer());

    return m_data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool Value::IsUndefined() const
    {
    return ScopeInternal::ToHandle (*this)->IsUndefined();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool Value::IsNull() const
    {
    return ScopeInternal::ToHandle (*this)->IsNull();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool Value::IsBoolean() const
    {
    return ScopeInternal::ToHandle (*this)->IsBoolean();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool Value::IsNumber() const
    {
    return ScopeInternal::ToHandle (*this)->IsNumber();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool Value::IsString() const
    {
    return ScopeInternal::ToHandle (*this)->IsString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool Value::IsObject() const
    {
    return ScopeInternal::ToHandle (*this)->IsObject();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool Value::IsArray() const
    {
    return ScopeInternal::ToHandle (*this)->IsArray();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool Value::IsArrayBuffer() const
    {
    return ScopeInternal::ToHandle (*this)->IsArrayBuffer();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool Value::IsFunction() const
    {
    return ScopeInternal::ToHandle (*this)->IsFunction();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool Value::IsExternal() const
    {
    auto handle = ScopeInternal::ToHandle (*this);

    return handle->IsObject() && v8::Local<v8::Object>::Cast (handle)->InternalFieldCount() != 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool Value::IsPointer() const
    {
    return ScopeInternal::ToHandle (*this)->IsExternal();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool Boolean::GetValue() const
    {
    BeAssert (!IsEmpty());

    return v8::Local<v8::Boolean>::Cast (ScopeInternal::ToHandle (*this))->Value();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
double Number::GetValue() const
    {
    BeAssert (!IsEmpty());

    return v8::Local<v8::Number>::Cast (ScopeInternal::ToHandle (*this))->Value();
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Utf8String String::GetValue() const
    {
    BeAssert (!IsEmpty());

    auto handle = v8::Local<v8::String>::Cast (ScopeInternal::ToHandle (*this));
    v8::String::Utf8Value value (handle);

    return *value;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void* External::GetValue() const
    {
    BeAssert (!IsEmpty());

    auto handle = v8::Local<v8::Object>::Cast (ScopeInternal::ToHandle (*this));

    return v8::Local<v8::External>::Cast (handle->GetInternalField (0))->Value();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void* Pointer::GetValue() const
    {
    BeAssert (!IsEmpty());

    return v8::Local<v8::External>::Cast (ScopeInternal::ToHandle (*this))->Value();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
size_t ArrayBuffer::GetLength() const
    {
    BeAssert (!IsEmpty());

    return v8::Local<v8::ArrayBuffer>::Cast (ScopeInternal::ToHandle (*this))->GetContents().ByteLength();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void* ArrayBuffer::GetValue() const
    {
    BeAssert (!IsEmpty());

    return v8::Local<v8::ArrayBuffer>::Cast (ScopeInternal::ToHandle (*this))->GetContents().Data();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Value Object::Get (Utf8CP key, bool clear) const
    {
    BeAssert (!IsEmpty());

    auto handle = v8::Local<v8::Object>::Cast (ScopeInternal::ToHandle (*this));
    auto context = handle->CreationContext();
    auto isolate = context->GetIsolate();

    auto keyHandle = v8::String::NewFromUtf8 (isolate, key, v8::NewStringType::kNormal);
    if (!keyHandle.IsEmpty())
        {
        auto keyHandleLocal = keyHandle.ToLocalChecked();

        auto valueHandle = handle->Get (context, keyHandleLocal);
        if (!valueHandle.IsEmpty())
            {
            if (clear)
                {
                auto cleared = handle->Set (context, keyHandleLocal, v8::Null (isolate));
                BeAssert (!cleared.IsNothing() && cleared.ToChecked());
                }

            return *valueHandle.ToLocalChecked();
            }
        }

    return *v8::Local<v8::Value>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool Object::Has (Utf8CP key) const
    {
    BeAssert (!IsEmpty());

    auto handle = v8::Local<v8::Object>::Cast (ScopeInternal::ToHandle (*this));
    auto context = handle->CreationContext();
    auto isolate = context->GetIsolate();

    auto keyHandle = v8::String::NewFromUtf8 (isolate, key, v8::NewStringType::kNormal);
    if (!keyHandle.IsEmpty())
        {
        auto has = handle->Has (context, keyHandle.ToLocalChecked());

        return has.IsNothing() ? false : has.ToChecked();
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool Object::HasOwn (Utf8CP key) const
    {
    BeAssert (!IsEmpty());

    auto handle = v8::Local<v8::Object>::Cast (ScopeInternal::ToHandle (*this));
    auto context = handle->CreationContext();
    auto isolate = context->GetIsolate();

    auto keyHandle = v8::String::NewFromUtf8 (isolate, key, v8::NewStringType::kNormal);
    if (!keyHandle.IsEmpty())
        {
        auto has = handle->HasOwnProperty (context, keyHandle.ToLocalChecked());

        return has.IsNothing() ? false : has.ToChecked();
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Object Object::GetAsObject (Utf8CP key) const
    {
    return Get (key).AsObject();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool Object::Set (Utf8CP key, ValueCR value)
    {
    BeAssert (!IsEmpty());

    auto handle = v8::Local<v8::Object>::Cast (ScopeInternal::ToHandle (*this));
    auto context = handle->CreationContext();
    auto isolate = context->GetIsolate();

    auto keyHandle = v8::String::NewFromUtf8 (isolate, key, v8::NewStringType::kNormal);
    if (!keyHandle.IsEmpty())
        {
        auto set = handle->Set (context, keyHandle.ToLocalChecked(), ScopeInternal::ToHandle (value));

        return set.IsNothing() ? false : true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool Object::SetPrototype (ValueCR value)
    {
    BeAssert (!IsEmpty());

    auto handle = v8::Local<v8::Object>::Cast (ScopeInternal::ToHandle (*this));
    auto context = handle->CreationContext();

    auto set = handle->SetPrototype (context, ScopeInternal::ToHandle (value));

    return set.IsNothing() ? false : true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Value Object::GetPrototype() const
    {
    BeAssert (!IsEmpty());

    auto handle = v8::Local<v8::Object>::Cast (ScopeInternal::ToHandle (*this));

    return *handle->GetPrototype();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
uint32_t Array::GetLength() const
    {
    BeAssert (!IsEmpty());

    return v8::Local<v8::Array>::Cast (ScopeInternal::ToHandle (*this))->Length();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Value Array::Get (uint32_t key) const
    {
    BeAssert (!IsEmpty());

    auto handle = v8::Local<v8::Array>::Cast (ScopeInternal::ToHandle (*this));
    auto context = handle->CreationContext();
    
    auto valueHandle = handle->Get (context, key);
    if (!valueHandle.IsEmpty())
        return *valueHandle.ToLocalChecked();

    return *v8::Local<v8::Value>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool Array::Set (uint32_t key, ValueCR value)
    {
    BeAssert (!IsEmpty());

    auto handle = v8::Local<v8::Array>::Cast (ScopeInternal::ToHandle (*this));
    auto context = handle->CreationContext();

    auto set = handle->Set (context, key, ScopeInternal::ToHandle (value));

    return set.IsNothing() ? false : true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Value Function::CallInternal (uint32_t argc, ValueP argv, ObjectCP scope, bool construct) const
    {
    BeAssert (!IsEmpty());

    auto handle = v8::Local<v8::Function>::Cast (ScopeInternal::ToHandle (*this));
    auto context = handle->CreationContext();

    if (construct)
        {
        auto instanceHandle = handle->NewInstance (context, argc, reinterpret_cast<v8::Local<v8::Value>*>(argv));
        if (!instanceHandle.IsEmpty())
            return *instanceHandle.ToLocalChecked();
        }
    else
        {
        BeAssert (scope != nullptr);
        return *handle->Call (ScopeInternal::ToHandle (*scope), argc, reinterpret_cast<v8::Local<v8::Value>*>(argv));
        }

    return *v8::Local<v8::Value>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
CallbackInfo::CallbackInfo (RuntimeR runtime, void* data)
    : m_runtime (runtime),
      m_data    (data),
      m_scope   (runtime, false)
    {
    ;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
uint32_t CallbackInfo::GetArgumentCount() const
    {
    return RuntimeInternal::GetInfo (*this).Length();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Object CallbackInfo::GetThis() const
    {
    return *RuntimeInternal::GetInfo (*this).This();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool CallbackInfo::IsConstructCall() const
    {
    return RuntimeInternal::GetInfo (*this).IsConstructCall();
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
Value CallbackInfo::GetArgument (uint32_t index) const
    {
    return Value (*RuntimeInternal::GetInfo (*this) [index]);
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

    Js::Scope scope (GetRuntime());
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
