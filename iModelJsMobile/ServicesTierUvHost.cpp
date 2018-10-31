/*--------------------------------------------------------------------------------------+
|
|     $Source: ServicesTierUvHost.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "iModelJsInternal.h"

BEGIN_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE

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

    Extension::Install ([]() { return &MobileGateway::GetInstance(); });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void UvHost::OnStop()
    {
    MobileGateway::Terminate();
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

        const char* threadPoolSize = "UV_THREADPOOL_SIZE=1";
    putenv(const_cast<char*>(threadPoolSize));
        
    m_jsRuntime = new Js::Runtime("iModel.js Services Tier", config.enableJsDebugger, config.jsDebuggerPort);

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

