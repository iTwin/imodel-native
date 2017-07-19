/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnScript/DgnScript.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <BeJavaScript/BeJavaScript.h>

static intptr_t s_homeThreadId;
static size_t s_nonHomeThreadCount;
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
    s_homeThreadId = BeThreadUtilities::GetCurrentThreadId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::CheckCleanup()
    {
    bool anyLeft = false;
        {
        BeSystemMutexHolder _thread_safe_;
        anyLeft = (s_nonHomeThreadCount != 0);
        }

    if (!anyLeft)
        return;
    LOG.error("Some thread initialized script support on a thread and then forgot to call TerminateOnThread");
    BeAssert(false && "Some thread initialized script support on a thread and then forgot to call TerminateOnThread");
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
        if (BeThreadUtilities::GetCurrentThreadId() == s_homeThreadId)      // To make this a little more foolproof, tolerate redundant calls to InitializeOnThread on home thread
/*<=*/      return;

        BeAssert(false && "Call ScriptAdmin::InitializeOnThread only once on a given thread");
        return;
        }

    //  Try to detect missing calls to Terminate
    bool tooMany = false;
        {
        BeSystemMutexHolder _thread_safe_;
        if (BeThreadUtilities::GetCurrentThreadId() != s_homeThreadId)
            {
            ++s_nonHomeThreadCount;
            tooMany = (s_nonHomeThreadCount > 20);
            }
        }

    if (tooMany)
        {
        LOG.warningv("ScriptAdmin::InitializeOnThread called %lld times. Did you forget to call TerminateOnThread?", s_nonHomeThreadCount);
        BeDataAssert(false && "ScriptAdmin::InitializeOnThread called many times. Did you forget to call TerminateOnThread?");
        }

    //  *********************************************
    //  Initialize the BeJsEnvironment
    t_jsenv = new BeJsEnvironment;

    //  *********************************************
    //  Initialize the DgnScriptContext
    auto dgnScriptContext = new DgnScriptContext(*t_jsenv);
    t_jscontext = dgnScriptContext;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::TerminateOnThread()
    {
    if (nullptr == t_jsenv)
        {
        BeAssert(false && "Call ScriptAdmin::TerminateOnThread only once on a given thread");
        return;
        }

    if (BeThreadUtilities::GetCurrentThreadId() != s_homeThreadId)
        {
        BeSystemMutexHolder _thread_safe_;
        --s_nonHomeThreadCount;
        }

    // Free the BeJsEnvironment and BeJsContext that were set up on this thread
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
    if ((nullptr == t_jsenv) && (BeThreadUtilities::GetCurrentThreadId() == s_homeThreadId))        // only the home thread auto-initializes
        InitializeOnThread();

    if (nullptr == t_jsenv)
        throw "InitializeOnThread was not called";

    return *t_jsenv;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
BeJsContextR DgnPlatformLib::Host::ScriptAdmin::GetDgnScriptContext()
    {
    if ((nullptr == t_jsenv) && (BeThreadUtilities::GetCurrentThreadId() == s_homeThreadId))        // only the home thread auto-initializes
        InitializeOnThread();

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
