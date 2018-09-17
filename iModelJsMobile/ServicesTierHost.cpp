/*--------------------------------------------------------------------------------------+
|
|     $Source: ServicesTierHost.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "iModelJsInternal.h"
#include <vector>
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/Desktop/FileSystem.h>

extern "C" void imodeljs_addon_entry_point();

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
        auto initialized = uv_mutex_init(&s_mutex);
        BeAssert(initialized >= 0);

        s_initialized = true;
        }

    return &s_mutex;
    }

BEGIN_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Sam.Wilson  02/18
//=======================================================================================
struct NapiAddonExtension // : public ServicesTier::Extension
    {
    Utf8String m_name;
    Napi::ModuleRegisterCallback m_func;
    // Utf8CP SupplyName() const override { return m_name.c_str(); }
    Napi::Value ExportJsModule (Js::RuntimeR runtime) // override
        {
        auto env = runtime.Env();
        // NEEDS WORK: The (JS) caller should pass 'module' and 'exports' objects into me
        auto exports = Napi::Object::New(env);
        return m_func(env, exports);
        }
    NapiAddonExtension(Utf8StringCR name, Napi::ModuleRegisterCallback func) : m_name(name), m_func(func) {;}
    };

static NapiAddonExtension* s_imodelJsNative;

/*
"@bentley/imodeljs-nodeaddon"

->

_register_
__napi_ ## regfunc


AddonUtils, registerModule


// Intercepts the Node-V8 module registration callback. Converts parameters
// to NAPI equivalents and then calls the registration callback specified
// by the NAPI module.
void napi_module_register_cb(v8::Local<v8::Object> exports,
                             v8::Local<v8::Value> module,
                             v8::Local<v8::Context> context,
                             void* priv) {
  napi_module* mod = static_cast<napi_module*>(priv);

  // Create a new napi_env for this module or reference one if a pre-existing
  // one is found.
  napi_env env = v8impl::GetEnv(context);

  napi_value _exports =
      mod->nm_register_func(env, v8impl::JsValueFromV8LocalValue(exports));

  // If register function returned a non-null exports object different from
  // the exports object we passed it, set that as the "exports" property of
  // the module.
  if (_exports != nullptr &&
      _exports != v8impl::JsValueFromV8LocalValue(exports)) {
    napi_value _module = v8impl::JsValueFromV8LocalValue(module);
    napi_set_named_property(env, _module, "exports", _exports);
  }
}

*/

//=======================================================================================
// Projects the ECSqlStatement class into JS.
//! @bsiclass
//=======================================================================================
struct IModelJsTimer : Napi::ObjectWrap<IModelJsTimer>
    {
    private:
        static Napi::FunctionReference s_constructor;
        uv_timer_t m_handle;
        int64_t m_timeout;
        Napi::FunctionReference m_function;

        static uv_loop_t* GetEventLoop() { return Host::GetInstance().GetEventLoop(); }

    public:
        static void Init(Napi::Env& env, Napi::Object exports)
            {
            Napi::HandleScope scope(env);
            Napi::Function t = DefineClass(env, "IModelJsTimer", {
              InstanceMethod("start", &IModelJsTimer::Start),
              InstanceMethod("stop", &IModelJsTimer::Stop),
            });

            exports.Set("IModelJsTimer", t);

            s_constructor = Napi::Persistent(t);
            s_constructor.SuppressDestruct();
            }

        IModelJsTimer(const Napi::CallbackInfo& info) : Napi::ObjectWrap<IModelJsTimer>(info)
            {
            m_timeout = info[0].ToNumber();
            m_function = Napi::Persistent(info[1].As<Napi::Function>());
            uv_timer_init(GetEventLoop(), &m_handle);
            Ref(); // keep this alive until the uv event is done with it. See OnUvHandleClose below.
            }

        static void OnUvHandleClose(uv_handle_t* handle)
            {
            auto wrap = static_cast<IModelJsTimer*>(handle->data);
            wrap->Unref(); // allow this object to be GC'd
            }

        static void CloseUvHandle(uv_timer_t* handle)
            {
            uv_close((uv_handle_t*)handle, OnUvHandleClose);
            }

        Napi::Value Start(const Napi::CallbackInfo& info)
            {
            int err = uv_timer_start(&m_handle, OnTimeout, m_timeout, 0);
            m_handle.data = this;
            return Napi::Number::New(Env(), err);
            }

        void Stop(const Napi::CallbackInfo& info)
            {
            uv_timer_stop(&m_handle);
            CloseUvHandle(&m_handle);
            }

        static void OnTimeout(uv_timer_t* handle)
            {
            auto wrap = static_cast<IModelJsTimer*>(handle->data);
            auto env = wrap->Env();
            if (!env.IsExceptionPending())
                {
                Napi::HandleScope handle_scope(env);
                try {
                    wrap->m_function({wrap->Value()});
                    }
                catch (Napi::Error err)
                    {
                    LOG(err.Message().c_str());
                    }
                }
            if (0 == uv_timer_get_repeat(handle))
                {
                CloseUvHandle(&wrap->m_handle);
                }
            }
    };

Napi::FunctionReference IModelJsTimer::s_constructor;

//=======================================================================================
// Projects the ECSqlStatement class into JS.
//! @bsiclass
//=======================================================================================
struct IModelJsFs : Napi::ObjectWrap<IModelJsFs>
    {
    IModelJsFs(const Napi::CallbackInfo& info) : Napi::ObjectWrap<IModelJsFs>(info)
        {}

    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "IModelJsFs", {
          StaticMethod("existsSync", &IModelJsFs::ExistsSync),
          StaticMethod("unlinkSync", &IModelJsFs::UnlinkSync),
          StaticMethod("removeSync", &IModelJsFs::RemoveSync),
          StaticMethod("mkdirSync", &IModelJsFs::MkdirSync),
          StaticMethod("lstatSync", &IModelJsFs::LStatSync),
          StaticMethod("rmdirSync", &IModelJsFs::RmdirSync),
          StaticMethod("readdirSync", &IModelJsFs::ReaddirSync),
          StaticMethod("writeFileSync", &IModelJsFs::WriteFileSync),
          StaticMethod("copySync", &IModelJsFs::CopySync),
        });

        exports.Set("IModelJsFs", t);
        }

    static Napi::Value ExistsSync(const Napi::CallbackInfo& info)
        {
        BeFileName fn(info[0].ToString().Utf8Value().c_str(), true);
        return Napi::Boolean::New(info.Env(), fn.DoesPathExist());
        }

    static Napi::Value UnlinkSync(const Napi::CallbackInfo& info)
        {
        BeFileName fn(info[0].ToString().Utf8Value().c_str(), true);
        return Napi::Number::New(info.Env(), (int)fn.BeDeleteFile());
        }

    static Napi::Value RemoveSync(const Napi::CallbackInfo& info)
        {
        BeFileName fn(info[0].ToString().Utf8Value().c_str(), true);
        if (fn.IsDirectory())
            return Napi::Number::New(info.Env(), (int)BeFileName::EmptyAndRemoveDirectory(fn.c_str()));
        return Napi::Number::New(info.Env(), (int)fn.BeDeleteFile());
        }

    static Napi::Value MkdirSync(const Napi::CallbackInfo& info)
        {
        BeFileName fn(info[0].ToString().Utf8Value().c_str(), true);
        return Napi::Number::New(info.Env(), (int)BeFileName::CreateNewDirectory(fn.c_str()));
        }

    static Napi::Value RmdirSync(const Napi::CallbackInfo& info)
        {
        BeFileName fn(info[0].ToString().Utf8Value().c_str(), true);
        return Napi::Number::New(info.Env(), (int)BeFileName::EmptyAndRemoveDirectory(fn.c_str()));
        }

    static Napi::Value WriteFileSync(const Napi::CallbackInfo& info)
        {
        auto fp = fopen(info[0].ToString().Utf8Value().c_str(), "w+");
        int n = fputs(info[1].ToString().Utf8Value().c_str(), fp);
        fclose(fp);
        return Napi::Number::New(info.Env(), n);
        }

    static Napi::Value CopySync(const Napi::CallbackInfo& info)
        {
        BeFileName src(info[0].ToString().Utf8Value().c_str(), true);
        BeFileName dst(info[1].ToString().Utf8Value().c_str(), true);
        return Napi::Number::New(info.Env(), (int)BeFileName::BeCopyFile(src, dst));
        }

    static Napi::Value ReaddirSync(const Napi::CallbackInfo& info)
        {
        BeFileName topDir(info[0].ToString().Utf8Value().c_str(), true);

        bvector<Utf8String> files;
        BeFileName entryName;
        bool        isDir;
        for (BeDirectoryIterator dirs (topDir); dirs.GetCurrentEntry (entryName, isDir) == SUCCESS; dirs.ToNext())
            {
            files.push_back(Utf8String(entryName.c_str()));
            }

        auto dirs = Napi::Array::New(info.Env(), files.size());
        int i=0;
        for (auto const& fn : files)
            dirs[i++] = Napi::String::New(info.Env(), fn.c_str());

        return dirs;
        }

    static Napi::Value LStatSync(const Napi::CallbackInfo& info)
        {
        auto env = info.Env();

        BeFileName fn(info[0].ToString().Utf8Value().c_str(), true);

        auto stats = Napi::Object::New(env);
        if (!fn.DoesPathExist())
            return env.Undefined();

        auto isDir = fn.IsDirectory();
        stats.Set("IsDirectory", Napi::Boolean::New(env, isDir));
        stats.Set("IsFile", Napi::Boolean::New(env, !isDir));
        stats.Set("IsSymbolicLink", Napi::Boolean::New(env, fn.IsSymbolicLink()));
        stats.Set("IsSocket", Napi::Boolean::New(env, false));
        time_t ctime, mtime, atime;
        fn.GetFileTime(&ctime, &atime, &mtime);
        stats.Set("birthTimeMs", Napi::Number::New(env, (double)ctime/1000));
        stats.Set("atimeMs", Napi::Number::New(env, (double)atime/1000));
        stats.Set("mtimeMs", Napi::Number::New(env, (double)mtime/1000));
        return stats;
        }

    };

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
void Host::EventLoopCallbackAsyncHandler(uv_async_t* handle)
    {
    auto callback = static_cast<EventLoopCallback_T*>(handle->data);
    (*callback)();
    delete callback;

    uv_close((uv_handle_t*)handle, &EventLoopAsyncCloseHandler);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Host::EventLoopAsyncCloseHandler(uv_handle_t* handle)
    {
    free(handle);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
Host::Host()
    : m_eventLoop(nullptr),
    m_eventLoopThreadId(0),
    m_jsRuntime(nullptr),
    m_ready(false),
    m_stopped(false),
    m_environment(nullptr)
    {
    BeAssert(s_instance.load() == nullptr);

    s_instance = this;
    m_idler.data = this;
    Environment::InstallCoreExtensions();
    }


#if defined(BENTLEYCONFIG_OS_APPLE_IOS) || defined(BENTLEYCONFIG_OS_APPLE_MACOS)
//---------------------------------------------------------------------------------------
// @bsimethod                                Affan.Khan                     7/2018
//---------------------------------------------------------------------------------------
JSContextRef Host::GetContext() const
    {
    return m_jsRuntime->GetContext();
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
Host::~Host()
    {
    BeAssert(s_instance == this);
    BeAssert(m_environment == nullptr);

    s_instance = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
HostR Host::GetInstance()
    {
    BeAssert(Exists());

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
    BeAssert(IsEventLoopThread());

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
    BeAssert(m_eventLoop.load() != nullptr);

    m_jsRuntime = &SupplyJsRuntime();
    BeAssert(m_jsRuntime != nullptr);

    StartIdler();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::HandleReady()
    {
    BeAssert(!IsReady());

    auto debugger = GetJsRuntime().GetDebugger();
    if (debugger != nullptr && GetConfig().waitForJsDebugger && !debugger->IsReady())
        return;

    m_ready = true;

    SetupJsRuntime();
    InitializeEnvironment();
    EmptyExtensionsQueue();
    OnReady();

    auto& env = Env();
    Napi::HandleScope scope(env);
    try
        {
        m_notifyReady.Value().As<Napi::Function>()({});
        }
    catch (Napi::Error err)
        {
        LOG(err.Message().c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::NotifyStop()
    {
    BeAssert(IsReady());

    OnStop();
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
    Napi::HandleScope scope(runtime.Env());

    runtime.NotifyIdle();
    OnIdle();

    if (!m_notifyIdle.IsEmpty())
        {
        try
            {
            m_notifyIdle.Value().As<Napi::Function>() ({});
            }
        catch (Napi::Error err)
            {
            LOG(err.Message().c_str());
            }
        }

    StopIdler();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::StartIdler()
    {
    auto initializedIdler = uv_idle_init(GetEventLoop(), &m_idler);
    BeAssert(initializedIdler >= 0);

    auto startedIdler = uv_idle_start(&m_idler, &IdleHandler);
    BeAssert(startedIdler >= 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::StopIdler()
    {
    auto stoppedIdler = uv_idle_stop(&m_idler);
    BeAssert(stoppedIdler >= 0);

    uv_close((uv_handle_t*)&m_idler, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::InitializeEnvironment()
    {
    BeAssert(m_environment == nullptr);

    m_environment = new Environment;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::TerminateEnvironment()
    {
    BeAssert(m_environment != nullptr);

    m_environment->Shutdown();
    delete m_environment;
    m_environment = nullptr;
    }

//#define TO_UTF8CP(WSTR) Utf8String(WSTR).c_str()

//---------------------------------------------------------------------------------------
// @bsimethod                                Sam.Wilson                     1/2018
//---------------------------------------------------------------------------------------
static Utf8String newDir(BeFileNameCR baseDir, WCharCP subDirName, bool deleteIfExists)
    {
    BeFileName dirName(baseDir);
    dirName.AppendToPath(subDirName);
    if (deleteIfExists)
        BeFileName::EmptyAndRemoveDirectory(dirName.c_str());
    BeFileName::CreateNewDirectory(dirName.c_str());
    return Utf8String(dirName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::SetupJsRuntime()
    {
    auto& runtime = GetJsRuntime();
    auto& env = Env();
    Napi::HandleScope scope(env);

    imodeljs_addon_entry_point(); // tell the addon (which is actually linked in) to install itself as a module.

    try
        {   // must process Napi/JS errors before we destroy the HandleScope

        env.Global().Set("self", env.Global()); // make this platform look like a WebWorker

        // Set up the knownLocations globals
        auto knownLocations = Napi::Object::New(env);

#if (defined(BENTLEYCONFIG_OS_WINDOWS) && !defined(BENTLEYCONFIG_OS_WINRT)) || defined(BENTLEYCONFIG_OS_LINUX) || defined(BENTLEYCONFIG_OS_APPLE_MACOS)
        // Define these globals on desktop platforms just for testing purposes ...
        BeFileName cwd;
        Desktop::FileSystem::GetCwd(cwd);
        knownLocations.Set("packageAssetsDir", Napi::String::New(env, newDir(cwd, L"packageAssetsDir", false).c_str()));
        knownLocations.Set("assetsDir", Napi::String::New(env, newDir(cwd, L"assetsDir", false).c_str()));
        //BeFileName tmpDir;
        //Desktop::FileSystem::BeGetTempPath(tmpDir);
        knownLocations.Set("tempDir", Napi::String::New(env, newDir(cwd, L"tempDir", true).c_str()));
#elif defined(BENTLEYCONFIG_OS_APPLE_IOS)
        BeFileName cwd;
        Desktop::FileSystem::GetCwd(cwd);
        knownLocations.Set("packageAssetsDir", Napi::String::New(env, newDir(cwd, L"packageAssetsDir", false).c_str()));
        knownLocations.Set("assetsDir", Napi::String::New(env, newDir(cwd, L"assetsDir", false).c_str()));
        //BeFileName tmpDir;
        //Desktop::FileSystem::BeGetTempPath(tmpDir);
        knownLocations.Set("tempDir", Napi::String::New(env, newDir(cwd, L"tempDir", true).c_str()));
#else 
        #error implement known locations for mobile platforms ...
#endif

        auto imodeljsMobile = Napi::Object::New(env);

        imodeljsMobile.Set("knownLocations", knownLocations);

        if (s_imodelJsNative)
            imodeljsMobile.Set("imodeljsNative", s_imodelJsNative->ExportJsModule(runtime));
        else
            fprintf(stderr, "no addon??\n");

        env.Global().Set("imodeljsMobile", imodeljsMobile);

        IModelJsTimer::Init(env, env.Global());
        IModelJsFs::Init(env, env.Global());

        auto initScriptEvaluation = runtime.EvaluateScript(InitScript());
        BeAssert(initScriptEvaluation.status == Js::EvaluateStatus::Success);

        auto initParams = Napi::Object::New(env);

        initParams.Set("deliverExtension", Napi::Function::New(env, [this](Napi::CallbackInfo const& info) -> Napi::Value
            {
            auto env = info.Env();

            if (info.Length() == 0)
                return env.Undefined();

            auto identifierArgument = info[0];
            if (!identifierArgument.IsString())
                return env.Undefined();

            return m_environment->DeliverExtension(Js::Runtime::GetRuntime(info.Env()), identifierArgument.As<Napi::String>().Utf8Value().c_str());
            }));

        initParams.Set("evaluateScript", Napi::Function::New(env, [](Napi::CallbackInfo const& info) -> Napi::Value
            {
            auto env = info.Env();
            auto& runtime = Js::Runtime::GetRuntime(env);

            JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS(2);

            auto script = JS_CALLBACK_GET_STRING(0);
            auto identifier = JS_CALLBACK_GET_STRING(1);

            auto result = runtime.EvaluateScript(script, identifier);
            if (result.status == Js::EvaluateStatus::ParseError)
                Napi::Error::New(env, "Parse Error").ThrowAsJavaScriptException();
            else if (result.status == Js::EvaluateStatus::RuntimeError)
                Napi::Error::New(env, result.message.c_str()).ThrowAsJavaScriptException();
            else if (result.status == Js::EvaluateStatus::Success)
                return result.value;

            return JS_CALLBACK_UNDEFINED;
            }));

        initParams.Set("createStringFromUtf8Buffer", Napi::Function::New(env, [](Napi::CallbackInfo const& info) -> Napi::Value
            {
            JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS(1);

            auto buffer = JS_CALLBACK_GET_ARRAY_BUFFER(0);

            Utf8String string(static_cast<CharP>(buffer.Data()), buffer.ByteLength());

            return JS_CALLBACK_STRING(string.c_str());
            }));

        initParams.Set("require", SupplyJsRequireHandler(env, initParams));

        auto info = Napi::Object::New(env);
        SupplyJsInfoValues(env, info);
        initParams.Set("info", info);

        initScriptEvaluation.value.As<Napi::Function>() ({ initParams });

        m_notifyIdle.Reset(initParams.Get("notifyIdle").As<Napi::Function>());
        m_notifyShutdown.Reset(initParams.Get("notifyShutdown").As<Napi::Function>());
        m_notifyReady.Reset(initParams.Get("notifyReady").As<Napi::Function>());

        env.Global().Set("console_log", Napi::Function::New(env, [](Napi::CallbackInfo const& info) -> Napi::Value
            {
            JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS(1);
            auto msg = JS_CALLBACK_GET_STRING(0);
            LOG(msg.Utf8Value().c_str());
            return info.Env().Undefined();
            }));
        }
    catch (Napi::Error err) // JS threw an exception
        {
        LOG(err.Message().c_str());
        return;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::TeardownJsRuntime()
    {
    auto& env = Env();
    Napi::HandleScope scope(env);

    m_notifyShutdown.Value().As<Napi::Function>() ({});

    m_notifyIdle.Reset();
    m_notifyShutdown.Reset();
    m_notifyReady.Reset();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
Napi::Function Host::SupplyJsRequireHandler(Napi::Env& env, Napi::Object initParams)
    {
    return Napi::Function::New(env, [](Napi::CallbackInfo const& info) { return JS_CALLBACK_UNDEFINED; });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::SupplyJsInfoValues(Napi::Env& env, Napi::Object info)
    {
#ifdef BENTLEYCONFIG_OS_WINDOWS
    info.Set("isWindows", Napi::Boolean::New(env, true));
#else
    info.Set("isWindows", Napi::Boolean::New(env, false));
#endif

    info.Set("argv", Napi::String::New(env, GetSystemArgv().c_str()));
    info.Set("cwd", Napi::String::New(env, GetSystemCwd().c_str()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
Utf8String Host::GetSystemArgv() const
    {
    Utf8String argv;

#ifdef BENTLEYCONFIG_OS_WINDOWS
    WString argvW(::GetCommandLineW());
    argv.Assign(argvW.GetWCharCP());
#else
// TODO: GetSystemArgv
// BeAssert(false && "TBD");
#endif

    return argv;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
Utf8String Host::GetSystemCwd() const
    {
#ifdef BENTLEYCONFIG_OS_WINDOWS
    char cwdBuffer[MAX_PATH * 4];
#else
    char cwdBuffer[PATH_MAX];
#endif

    size_t cwdLength = sizeof(cwdBuffer);
    auto result = uv_cwd(cwdBuffer, &cwdLength);
    BeAssert(result >= 0);

    return Utf8String((result >= 0) ? cwdBuffer : "");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::IdleHandler(uv_idle_t* handle)
    {
    static_cast<HostP>(handle->data)->HandleIdle();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
bool Host::PostToEventLoop(EventLoopCallback_T const& callback)
    {
    if (!IsStopped())
        {
        uv_async_t* handle = (uv_async_t*)malloc(sizeof(uv_async_t));

        auto initializedHandle = uv_async_init(Host::GetInstance().GetEventLoop(), handle, &EventLoopCallbackAsyncHandler);
        BeAssert(initializedHandle >= 0);

        if (initializedHandle >= 0)
            {
            auto callbackCopy = new EventLoopCallback_T(callback);
            handle->data = callbackCopy;

            auto sent = uv_async_send(handle);
            BeAssert(sent >= 0);

            if (sent >= 0)
                return true;
            }
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/17
//---------------------------------------------------------------------------------------
void Host::DispatchExtensionCallback(Extension::InstallCallback_T const& callback)
    {
    auto mutex = getExtensionsQueueMutex();

    uv_mutex_lock(mutex);

    auto instance = Exists() ? &GetInstance() : nullptr;

    if (instance != nullptr && instance->IsReady())
        {
        if (instance->IsEventLoopThread())
            instance->PerformInstall(callback);
        else
            instance->PostToEventLoop([=]() { GetInstance().PerformInstall(callback); });
        }
    else
        {
        s_extensionsQueue.push_back(callback);
        }

    uv_mutex_unlock(mutex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::EmptyExtensionsQueue()
    {
    auto mutex = getExtensionsQueueMutex();

    uv_mutex_lock(mutex);

    for (auto& cb : s_extensionsQueue)
        PerformInstall(cb);

    s_extensionsQueue.clear();

    uv_mutex_unlock(mutex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Host::PerformInstall(Extension::InstallCallback_T const& callback)
    {
    BeAssert(Host::GetInstance().IsEventLoopThread());

    auto extension = callback();
    BeAssert(extension != nullptr);

    extension->m_runtime = m_jsRuntime;

    Environment::GetInstance().Install(extension);
    }

std::atomic<HostP> Host::s_instance;

END_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE

using namespace BentleyApi::iModelJs::ServicesTier;

//---------------------------------------------------------------------------------------
//
// The addon (which is linked into this library) calls this to register itself as a module.
//
// @bsimethod                                Sam.Wilson                     01/2018
//---------------------------------------------------------------------------------------
extern "C" void imodeljs_register_addon(char const* addonName, Napi::ModuleRegisterCallback regFunc)
    {
    //if (0==strcmp(addonName, "at_bentley_imodeljs_nodeaddon"))
    //    Extension::Install ([=]() { return new NapiAddonExtension("@bentley/imodeljs-mobile", regFunc); });
    s_imodelJsNative = new NapiAddonExtension("@bentley/imodeljs-mobile", regFunc);
    }
