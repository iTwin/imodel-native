/*--------------------------------------------------------------------------------------+
|
|     $Source: ServicesTierUvHost.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "iModelJsInternal.h"

extern "C" void imodeljs_addon_entry_point();

BEGIN_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct NapiAddonExtension : public ServicesTier::Extension
    {
    Utf8String m_name;
    Napi::ModuleRegisterCallback m_func;
    Utf8CP SupplyName() const override { return m_name.c_str(); }
    Napi::Value ExportJsModule (Js::RuntimeR runtime) override
        {
        auto env = runtime.Env();
        // NEEDS WORK: The (JS) caller should pass 'module' and 'exports' objects into me
        auto exports = Napi::Object::New(env);
        return m_func(env, exports);
        }
    NapiAddonExtension(Utf8StringCR name, Napi::ModuleRegisterCallback func) : m_name(name), m_func(func) {;}
    };

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

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
UvHost::UvHost()
    : m_jsRuntime (nullptr)
    {
    auto argv = GetSystemArgv();
    if (argv.find ("--inspect-brk") != std::string::npos || argv.find ("--debug-brk") != std::string::npos)
        GetConfig().waitForJsDebugger = true;

    auto createdThread = uv_thread_create (&m_thread, &EventLoopThreadEntry, this);
    BeAssert (createdThread >= 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
UvHost::~UvHost()
    {
    PostToEventLoop ([this]() { uv_stop (GetEventLoop()); });

    auto joinedThread = uv_thread_join (&m_thread);
    BeAssert (joinedThread >= 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
uv_loop_t* UvHost::SupplyEventLoop()
    {
    return uv_default_loop();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
Js::RuntimeR UvHost::SupplyJsRuntime()
    {
    return *m_jsRuntime;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
Napi::Function UvHost::SupplyJsRequireHandler (Napi::Env& env, Napi::Object initParams)
    {
    auto& runtime = GetJsRuntime();

    auto requireScriptEvaluation = runtime.EvaluateScript (RequireScript());
    BeAssert (requireScriptEvaluation.status == Js::EvaluateStatus::Success);
            
    Napi::Function func(Env(), requireScriptEvaluation.value);
    return func({initParams}).As<Napi::Function>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void UvHost::SupplyJsInfoValues (Napi::Env& env, Napi::Object info)
    {
    Host::SupplyJsInfoValues (env, info);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void UvHost::EventLoopThreadEntry (void* arg)
    {
    BeThreadUtilities::SetCurrentThreadName ("UvHostEventLoopThread");
    reinterpret_cast<UvHostP>(arg)->EventLoopThreadMain();
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void UvHost::EventLoopThreadMain()
    {
    auto& config = GetConfig();

    m_jsRuntime = new Js::Runtime("iModel.js Services Tier", config.enableJsDebugger, config.jsDebuggerPort);

    imodeljs_addon_entry_point(); // tell the addon (which is actually linked in) that now is a good time for it to install itself as a module.

    NotifyStarting();
    uv_run (GetEventLoop(), UV_RUN_DEFAULT);
    NotifyStop();

    size_t c = 0;
    while (c < 10 && uv_run (GetEventLoop(), UV_RUN_NOWAIT) != 0) { ++c; } 

    auto closed = uv_loop_close (GetEventLoop());
    BeAssert (closed >= 0);

    delete m_jsRuntime;
    }

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
    if (0==strcmp(addonName, "at_bentley_imodeljs_nodeaddon"))
        Extension::Install ([=]() { return new NapiAddonExtension("@bentley/imodeljs-mobile", regFunc); });
    }
