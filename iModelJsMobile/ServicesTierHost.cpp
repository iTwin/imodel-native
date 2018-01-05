/*--------------------------------------------------------------------------------------+
|
|     $Source: ServicesTierHost.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "iModelJsInternal.h"
#include <vector>
#include <Bentley/BeFileName.h>

static std::vector<BentleyApi::iModelJs::ServicesTier::Extension::InstallCallback_T> s_extensionsQueue;

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
static uv_mutex_t* getExtensionsQueueMutex()
    {
    static uv_mutex_t s_mutex;
    static bool s_initialized;

    if (!s_initialized)
        {
        auto initialized = uv_mutex_init (&s_mutex);
        BeAssert (initialized >= 0);

        s_initialized = true;
        }

    return &s_mutex;
    }

BEGIN_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
Host::ConfigR Host::GetConfig()
    {
    static Config s_config;

    return s_config;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Host::EventLoopCallbackAsyncHandler (uv_async_t* handle)
    {
    auto callback = static_cast<EventLoopCallback_T*>(handle->data);
    (*callback)();
    delete callback;

    uv_close ((uv_handle_t*) handle, &EventLoopAsyncCloseHandler);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Host::EventLoopAsyncCloseHandler (uv_handle_t* handle)
    {
    free (handle);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
Host::Host()
    : m_eventLoop         (nullptr),
      m_eventLoopThreadId (0),
      m_jsRuntime         (nullptr),
      m_ready             (false),
      m_stopped           (false),
      m_environment       (nullptr)
    {
    BeAssert (s_instance.load() == nullptr);

    s_instance = this;
    m_idler.data = this;
    Environment::InstallCoreExtensions();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
Host::~Host()
    {
    BeAssert (s_instance == this);
    BeAssert (m_environment == nullptr);

    s_instance = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
HostR Host::GetInstance()
    {
    BeAssert (Exists());

    return *s_instance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
bool Host::Exists()
    {
    return s_instance.load() != nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
uv_loop_t* Host::GetEventLoop() const
    {
    return m_eventLoop;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
Js::RuntimeR Host::GetJsRuntime() const
    {
    BeAssert (IsEventLoopThread());

    return *m_jsRuntime;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
bool Host::IsReady() const
    {
    return m_ready;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
bool Host::IsStopped() const
    {
    return m_stopped;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
bool Host::IsEventLoopThread() const
    {
    return BeThreadUtilities::GetCurrentThreadId() == m_eventLoopThreadId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::NotifyStarting()
    {
    m_eventLoopThreadId = BeThreadUtilities::GetCurrentThreadId();

    m_eventLoop = SupplyEventLoop();
    BeAssert (m_eventLoop.load() != nullptr);

    m_jsRuntime = &SupplyJsRuntime();
    BeAssert (m_jsRuntime != nullptr);

    StartIdler();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::HandleReady()
    {
    BeAssert (!IsReady());

    auto debugger = GetJsRuntime().GetDebugger();
    if (debugger != nullptr && GetConfig().waitForJsDebugger && !debugger->IsReady())
        return;
    
    m_ready = true;

    SetupJsRuntime();
    InitializeEnvironment();
    EmptyExtensionsQueue();
    OnReady();

    auto& env = Env();
    Napi::HandleScope scope (env);
    try {
        m_notifyReady.Value().As<Napi::Function>()({});
        }
    catch (Napi::Error err)
        {
        fprintf(stderr, "%s\n", err.Message().c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::NotifyStop()
    {
    BeAssert (IsReady());

    OnStop();
    StopIdler();
    TerminateEnvironment();
    TeardownJsRuntime();

    m_stopped = true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::HandleIdle()
    {
    if (!IsReady())
        HandleReady();

    auto& runtime = GetJsRuntime();
    Napi::HandleScope scope (runtime.Env());

    runtime.NotifyIdle();
    OnIdle();

    if (!m_notifyIdle.IsEmpty())
        {
        try {
            m_notifyIdle.Value().As<Napi::Function>() ({});
            }
        catch (Napi::Error err)
            {
            fprintf(stderr, err.Message().c_str());
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::StartIdler()
    {
    auto initializedIdler = uv_idle_init (GetEventLoop(), &m_idler);
    BeAssert (initializedIdler >= 0);

    auto startedIdler = uv_idle_start (&m_idler, &IdleHandler);
    BeAssert (startedIdler >= 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::StopIdler()
    {
    auto stoppedIdler = uv_idle_stop (&m_idler);
    BeAssert (stoppedIdler >= 0);

    uv_close ((uv_handle_t*) &m_idler, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::InitializeEnvironment()
    {
    BeAssert (m_environment == nullptr);
    
    m_environment = new Environment;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::TerminateEnvironment()
    {
    BeAssert (m_environment != nullptr);

    m_environment->Shutdown();
    delete m_environment;
    m_environment = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::SetupJsRuntime()
    {
    auto& runtime = GetJsRuntime();
	auto& env = Env();
    Napi::HandleScope scope (env);

    try {   // must process Napi/JS errors before we destroy the HandleScope
        auto initScriptEvaluation = runtime.EvaluateScript (InitScript());
        BeAssert (initScriptEvaluation.status == Js::EvaluateStatus::Success);

        auto initParams = Napi::Object::New(env);

        initParams.Set ("deliverExtension", Napi::Function::New(env, [this](Napi::CallbackInfo const& info) -> Napi::Value
            {
            auto env = info.Env();

            if (info.Length() == 0)
                return env.Undefined();

            auto identifierArgument = info[0];
            if (!identifierArgument.IsString())
                return env.Undefined();

            return m_environment->DeliverExtension (Js::Runtime::GetRuntime(info.Env()), identifierArgument.As<Napi::String>().Utf8Value().c_str());
            }));

        initParams.Set ("evaluateScript", Napi::Function::New(env, [](Napi::CallbackInfo const& info) -> Napi::Value
            {
            auto env = info.Env();
            auto& runtime = Js::Runtime::GetRuntime(env);

            JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS(2);

            auto script = JS_CALLBACK_GET_STRING(0);
            auto identifier = JS_CALLBACK_GET_STRING(1);

            auto result = runtime.EvaluateScript (script, identifier);
            if (result.status == Js::EvaluateStatus::ParseError)
                Napi::Error::New(env, "Parse Error").ThrowAsJavaScriptException();
            else if (result.status == Js::EvaluateStatus::RuntimeError)
                Napi::Error::New(env, result.message.c_str()).ThrowAsJavaScriptException();
            else if (result.status == Js::EvaluateStatus::Success)
                return result.value;
        
            return JS_CALLBACK_UNDEFINED;
            }));

        initParams.Set ("createStringFromUtf8Buffer", Napi::Function::New(env, [](Napi::CallbackInfo const& info) -> Napi::Value
            {
            JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (1);

            auto buffer = JS_CALLBACK_GET_ARRAY_BUFFER (0);
        
            Utf8String string (static_cast<CharP>(buffer.Data()), buffer.ByteLength());

            return JS_CALLBACK_STRING (string.c_str());
            }));

        initParams.Set ("require", SupplyJsRequireHandler (env, initParams));

        auto info = Napi::Object::New(env);
        SupplyJsInfoValues (env, info);
        initParams.Set ("info", info);

        initScriptEvaluation.value.As<Napi::Function>() ({initParams});

        m_notifyIdle.Reset(initParams.Get("notifyIdle").As<Napi::Function>());
        m_notifyShutdown.Reset(initParams.Get("notifyShutdown").As<Napi::Function>());
        m_notifyReady.Reset(initParams.Get("notifyReady").As<Napi::Function>());

        env.Global().Set ("console_log", Napi::Function::New(env, [](Napi::CallbackInfo const& info) -> Napi::Value
            {
            JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS(1);
            auto msg = JS_CALLBACK_GET_STRING(0);
            printf("%s\n", msg.Utf8Value().c_str());
            return info.Env().Undefined();
            }));

        DefineNodeWorkAlikes();
        }
    catch (Napi::Error err) // JS threw an exception
        {
        fprintf(stderr, "%s\n", err.Message().c_str());
        return;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::DefineNodeWorkAlikes()
    {
	auto& env = Env();

    // -------------------------------------------------------------------
    //  process
    // -------------------------------------------------------------------
    auto process = Napi::Object::New(env);
    process["env"] = Napi::Object::New(env);       // this is a dummy. It's supposed to be the system environment, but that doesn't make sense on mobile devices.
                                                // We might try populating it with config variables ...
    process["on"] = Napi::Function::New(env, [] (Napi::CallbackInfo const& info) {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS(2);

        // *** TBD: support for event handlers?

    });

    process["exit"] = Napi::Function::New(env, [] (Napi::CallbackInfo const& info) {
        // *** TBD: support this?

    });

    env.Global()["process"] = process;

    // -------------------------------------------------------------------
    //  process
    // -------------------------------------------------------------------
    auto path = Napi::Object::New(env);
#ifdef _WIN32
    bool isWin32 = true;
    Utf8CP delim = ";";
#else
    bool isWin32 = false;
    Utf8CP delim = ":";
#endif
    path["posix"] = Napi::Boolean::New(env, !isWin32);
    path["win32"] = Napi::Boolean::New(env, isWin32);
    path["sep"] = Napi::String::New(env, DIR_SEPARATOR);
    path["delimiter"] = Napi::String::New(env, delim);
    path["basename"] = Napi::Function::New(env, [] (Napi::CallbackInfo const& info) -> Napi::Value {
            JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS(1);
            auto fnameS = JS_CALLBACK_GET_STRING(0);
            // TBD: optional 'ext' argument ...
            BeFileName fname(fnameS.Utf8Value().c_str(), true);
            return Napi::String::New(info.Env(), Utf8String(fname.GetBaseName()).c_str());
        });
    path["dirname"] = Napi::Function::New(env, [] (Napi::CallbackInfo const& info) -> Napi::Value {
            JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS(1);
            auto fnameS = JS_CALLBACK_GET_STRING(0);
            BeFileName fname(fnameS.Utf8Value().c_str(), true);
            return Napi::String::New(info.Env(), Utf8String(fname.GetDirectoryName()).c_str());
        });
    path["extname"] = Napi::Function::New(env, [] (Napi::CallbackInfo const& info) -> Napi::Value {
            JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS(1);
            auto fnameS = JS_CALLBACK_GET_STRING(0);
            BeFileName fname(fnameS.Utf8Value().c_str(), true);
            return Napi::String::New(info.Env(), Utf8String(fname.GetExtension()).c_str());
        });
    path["normalize"] = Napi::Function::New(env, [] (Napi::CallbackInfo const& info) -> Napi::Value {
            JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS(1);
            auto fnameS = JS_CALLBACK_GET_STRING(0);
            if (fnameS.Utf8Value().empty())
                {
                return Napi::String::New(info.Env(), "."); // as per node docs
                }
            BeFileName fname(fnameS.Utf8Value().c_str(), true);
            WString fixed;
            BeFileName::FixPathName(fixed, fname, true);
            return Napi::String::New(info.Env(), Utf8String(fixed).c_str());
        });
    path["join"] = Napi::Function::New(env, [] (Napi::CallbackInfo const& info) -> Napi::Value {
            JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS(1);
            // *** TBD: what will the arguments look like? An array?
            auto fnames = JS_CALLBACK_GET_ARRAY(0);
            if (fnames.Length() == 0)
                {
                Napi::Error::New(info.Env(), "not enough arguments").ThrowAsJavaScriptException();
                return info.Env().Undefined();
                }
            auto const& fnamesC = fnames;
            BeFileName path(fnamesC[(uint32_t)0].As<Napi::String>().Utf8Value().c_str(), true);
            for (uint32_t i = 1; i < fnamesC.Length(); ++i)
                {
                path.AppendToPath(BeFileName(fnamesC[i].As<Napi::String>().Utf8Value().c_str(), true));
                }
            return Napi::String::New(info.Env(), Utf8String(path).c_str());
        });
    path["isAbsolute"] = Napi::Function::New(env, [] (Napi::CallbackInfo const& info) -> Napi::Value {
            JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS(1);
            auto fnameS = JS_CALLBACK_GET_STRING(0);
            BeFileName fname(fnameS.Utf8Value().c_str(), true);
            return Napi::Boolean::New(info.Env(), fname.IsAbsolutePath());
        });

        /* TBD
path.format(pathObject)
path.parse(path)
path.relative(from, to)
path.resolve([...paths])
path.toNamespacedPath(path)
*/
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::TeardownJsRuntime()
    {
    auto& env = Env();
    Napi::HandleScope scope (env);

    m_notifyShutdown.Value().As<Napi::Function>() ({});
    
    m_notifyIdle.Reset();
    m_notifyShutdown.Reset();
    m_notifyReady.Reset();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
Napi::Function Host::SupplyJsRequireHandler (Napi::Env& env, Napi::Object initParams)
    {
    return Napi::Function::New(env, [](Napi::CallbackInfo const& info) { return JS_CALLBACK_UNDEFINED; });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::SupplyJsInfoValues (Napi::Env& env, Napi::Object info)
    {
#ifdef BENTLEYCONFIG_OS_WINDOWS
    info.Set ("isWindows", Napi::Boolean::New(env, true));
#else
    info.Set ("isWindows", Napi::Boolean::New(env, false));
#endif

    info.Set ("argv", Napi::String::New(env, GetSystemArgv().c_str()));
    info.Set ("cwd", Napi::String::New(env, GetSystemCwd().c_str()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
Utf8String Host::GetSystemArgv() const
    {
    Utf8String argv;

#ifdef BENTLEYCONFIG_OS_WINDOWS
    WString argvW (::GetCommandLineW());
    argv.Assign (argvW.GetWCharCP());
#else
    #error WIP
#endif

    return argv;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
Utf8String Host::GetSystemCwd() const
    {
#ifdef BENTLEYCONFIG_OS_WINDOWS
    char cwdBuffer [MAX_PATH * 4];
#else
    char cwdBuffer [PATH_MAX];
#endif

    size_t cwdLength = sizeof (cwdBuffer);
    auto result = uv_cwd (cwdBuffer, &cwdLength);
    BeAssert (result >= 0);

    return Utf8String ((result >= 0) ? cwdBuffer : "");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::IdleHandler (uv_idle_t* handle)
    {
    static_cast<HostP>(handle->data)->HandleIdle();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool Host::PostToEventLoop (EventLoopCallback_T const& callback)
    {
    if (!IsStopped())
        {
        uv_async_t* handle = (uv_async_t*) malloc (sizeof (uv_async_t));

        auto initializedHandle = uv_async_init (Host::GetInstance().GetEventLoop(), handle, &EventLoopCallbackAsyncHandler);
        BeAssert (initializedHandle >= 0);

        if (initializedHandle >= 0)
            {
            auto callbackCopy = new EventLoopCallback_T (callback);
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
void Host::DispatchExtensionCallback (Extension::InstallCallback_T const& callback)
    {
    auto mutex = getExtensionsQueueMutex();
        
    uv_mutex_lock (mutex);

    auto instance = Exists() ? &GetInstance() : nullptr;

    if (instance != nullptr && instance->IsReady())
        {
        if (instance->IsEventLoopThread())
            instance->PerformInstall (callback);
        else
            instance->PostToEventLoop ([=]() { GetInstance().PerformInstall (callback); });
        }
    else
        {
        s_extensionsQueue.push_back (callback);
        }

    uv_mutex_unlock (mutex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::EmptyExtensionsQueue()
    {
    auto mutex = getExtensionsQueueMutex();
        
    uv_mutex_lock (mutex);
    
    for (auto& cb : s_extensionsQueue)
        PerformInstall (cb);

    s_extensionsQueue.clear();

    uv_mutex_unlock (mutex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::PerformInstall (Extension::InstallCallback_T const& callback)
    {
    BeAssert (Host::GetInstance().IsEventLoopThread());

    auto extension = callback();
    BeAssert (extension != nullptr);

    extension->m_runtime = m_jsRuntime;

    Environment::GetInstance().Install (extension);
    }

std::atomic<HostP> Host::s_instance;

END_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE
