/*--------------------------------------------------------------------------------------+
|
|     $Source: ServicesTierUvHost.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
Js::Function UvHost::SupplyJsRequireHandler (Js::ScopeR scope, Js::ObjectCR initParams)
    {
    auto& runtime = GetJsRuntime();

    auto requireScriptEvaluation = runtime.EvaluateScript (RequireScript());
    BeAssert (requireScriptEvaluation.status == Js::EvaluateStatus::Success);

    return requireScriptEvaluation.value.AsFunction() (runtime.GetGlobal(), initParams).AsFunction();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void UvHost::SupplyJsInfoValues (Js::ScopeR scope, Js::ObjectR info)
    {
    Host::SupplyJsInfoValues (scope, info);
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

    m_jsRuntime = new Js::Runtime ("iModel.js Services Tier", config.enableJsDebugger, config.jsDebuggerPort);

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

