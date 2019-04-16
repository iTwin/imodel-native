/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <BeJavaScript/BeJavaScript.h>

static size_t s_initThreadCount;
static thread_local BeJsEnvironmentP t_jsenv;
static thread_local BeJsContextP t_jscontext;

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
struct DgnScriptContext : BeJsContext
{
    DgnScriptContext(BeJsEnvironmentR jsenv) : BeJsContext(jsenv, "DgnScript") {}
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

USING_NAMESPACE_BENTLEY_DGN

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptAdmin::ScriptAdmin()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::CheckCleanup()
    {
    bool anyLeft = false;
        {
        BeSystemMutexHolder _thread_safe_;
        anyLeft = (s_initThreadCount != 0);
        }

    if (!anyLeft)
        return;
    BeAssert(false && "s_initThreadCount != 0");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptAdmin::~ScriptAdmin()
    {
    CheckCleanup();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::InitializeOnThread()
    {
    //  Already initialized?
    if (nullptr != t_jsenv)
        {
        BeAssert(false && "Call ScriptAdmin::InitializeOnThread only once on a given thread");
        return;
        }

    ++s_initThreadCount;

    t_jsenv = new BeJsEnvironment;

    auto dgnScriptContext = new DgnScriptContext(*t_jsenv);
    t_jscontext = dgnScriptContext;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::TerminateOnThread()
    {
    if (nullptr == t_jsenv)
        return;

    --s_initThreadCount;

    if (nullptr != t_jscontext)
        {
        delete t_jscontext;
        t_jscontext = nullptr;
        }

    delete t_jsenv;
    t_jsenv = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
BeJsEnvironmentR DgnPlatformLib::Host::ScriptAdmin::GetBeJsEnvironment()
    {
    if (nullptr == t_jsenv)
        throw "InitializeOnThread was not called";

    return *t_jsenv;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
BeJsContextR DgnPlatformLib::Host::ScriptAdmin::GetDgnScriptContext()
    {
    if (nullptr == t_jscontext)
        throw "InitializeOnThread was not called";

    return *t_jscontext;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::_OnHostTermination(bool px)
    {
    delete this;
    }
