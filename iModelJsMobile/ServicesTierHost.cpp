/*--------------------------------------------------------------------------------------+
|
|     $Source: ServicesTierHost.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "iModelJsInternal.h"
#include <vector>

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

    auto& runtime = GetJsRuntime();
    Js::Scope scope (runtime);
    m_notifyReady.Get().AsFunction() (runtime.GetGlobal());
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
    Js::Scope scope (runtime);

    runtime.NotifyIdle();
    OnIdle();

    if (!m_notifyIdle.IsEmpty())
        m_notifyIdle.Get().AsFunction() (runtime.GetGlobal());
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
    Js::Scope scope (runtime);

    auto initScriptEvaluation = runtime.EvaluateScript (InitScript());
    BeAssert (initScriptEvaluation.status == Js::EvaluateStatus::Success);

    auto initParams = scope.CreateObject();

    initParams.Set ("deliverExtension", scope.CreateCallback ([this](Js::CallbackInfoCR info) -> Js::Value
        {
        auto& scope = info.GetScope();

        if (info.GetArgumentCount() == 0)
            return scope.CreateUndefined();

        auto identifierArgument = info.GetArgument (0);
        if (!identifierArgument.IsString())
            return scope.CreateUndefined();

        return m_environment->DeliverExtension (scope, identifierArgument.AsString().GetValue());
        }));

    initParams.Set ("evaluateScript", scope.CreateCallback ([](Js::CallbackInfoCR info) -> Js::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (2);

        auto script = JS_CALLBACK_GET_STRING (0);
        auto identifier = JS_CALLBACK_GET_STRING (1);

        auto identifierS = identifier.GetValue();

        auto result = info.GetRuntime().EvaluateScript (script.GetValue().c_str(), (identifierS == "") ? nullptr : identifierS.c_str());
        if (result.status == Js::EvaluateStatus::ParseError)
            info.GetRuntime().ThrowException (JS_CALLBACK_STRING ("Parse Error"));
        else if (result.status == Js::EvaluateStatus::RuntimeError)
            info.GetRuntime().ThrowException (JS_CALLBACK_STRING (result.message.c_str()));
        else if (result.status == Js::EvaluateStatus::Success)
            return result.value;
        
        return JS_CALLBACK_UNDEFINED;
        }));

    initParams.Set ("createStringFromUtf8Buffer", scope.CreateCallback ([](Js::CallbackInfoCR info) -> Js::Value
        {
        JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS (1);

        auto buffer = JS_CALLBACK_GET_ARRAY_BUFFER (0);
        
        Utf8String string (static_cast<CharP>(buffer.GetValue()), buffer.GetLength());

        return JS_CALLBACK_STRING (string.c_str());
        }));

    initParams.Set ("require", SupplyJsRequireHandler (scope, initParams));

    auto info = scope.CreateObject();
    SupplyJsInfoValues (scope, info);
    initParams.Set ("info", info);

    initScriptEvaluation.value.AsFunction() (runtime.GetGlobal(), initParams);

    m_notifyIdle.Assign (runtime, initParams.Get ("notifyIdle"));
    m_notifyShutdown.Assign (runtime, initParams.Get ("notifyShutdown"));
    m_notifyReady.Assign (runtime, initParams.Get ("notifyReady"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::TeardownJsRuntime()
    {
    auto& runtime = GetJsRuntime();
    Js::Scope scope (runtime);

    m_notifyShutdown.Get().AsFunction() (runtime.GetGlobal());
    
    m_notifyIdle.Clear();
    m_notifyShutdown.Clear();
    m_notifyReady.Clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
Js::Function Host::SupplyJsRequireHandler (Js::ScopeR scope, Js::ObjectCR initParams)
    {
    return scope.CreateCallback ([](Js::CallbackInfoCR info) { return JS_CALLBACK_UNDEFINED; });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::SupplyJsInfoValues (Js::ScopeR scope, Js::ObjectR info)
    {
#ifdef BENTLEYCONFIG_OS_WINDOWS
    info.Set ("isWindows", scope.CreateBoolean (true));
#else
    info.Set ("isWindows", scope.CreateBoolean (false));
#endif

    info.Set ("argv", scope.CreateString (GetSystemArgv().c_str()));
    info.Set ("cwd", scope.CreateString (GetSystemCwd().c_str()));
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
