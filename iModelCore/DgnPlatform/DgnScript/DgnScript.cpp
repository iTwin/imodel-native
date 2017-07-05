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
static thread_local DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler* t_jsnotificationHandler;
static DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler* s_defaultNotificationHandler;

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
    if (BeThreadUtilities::GetCurrentThreadId() == s_homeThreadId)  // To make this a little more foolproof, don't terminate the JS environment on the home thread.
        return;

    if (nullptr == t_jsenv)
        {
        BeAssert(false && "Call ScriptAdmin::TerminateOnThread only once on a given thread");
        return;
        }

    if (true)
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

    if (nullptr != t_jsnotificationHandler)
        t_jsnotificationHandler = nullptr;
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
// @bsimethod                                   Sam.Wilson                      07/17
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler* DgnPlatformLib::Host::ScriptAdmin::RegisterScriptNotificationHandler(ScriptNotificationHandler& h)
    {
    ScriptNotificationHandler* was = t_jsnotificationHandler;
    t_jsnotificationHandler = &h;
    return was;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/17
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler* DgnPlatformLib::Host::ScriptAdmin::GetScriptNotificationHandler()
    {
    if (nullptr != t_jsnotificationHandler)
        return t_jsnotificationHandler;

    BeSystemMutexHolder _thread_safe_;

    if (nullptr == s_defaultNotificationHandler)
        s_defaultNotificationHandler = new ScriptNotificationHandler; // we should always have a default notification handler

    return s_defaultNotificationHandler;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::HandleScriptError (ScriptNotificationHandler::Category category, Utf8CP description, Utf8CP details)
    {
    auto nh = GetScriptNotificationHandler();
    if (nullptr == nh)
        return;
    nh->_HandleScriptError(GetDgnScriptContext(), category, description, details);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler::_HandleScriptError(BeJsContextR, Category category, Utf8CP description, Utf8CP details)
    {
    NativeLogging::LoggingManager::GetLogger("DgnScript")->errorv("Script error category: %x, description: %s, details: %s", (int)category, description, details);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::HandleLogMessage(Utf8CP category, LoggingSeverity sev, Utf8CP message)
    {
    auto nh = GetScriptNotificationHandler();
    if (nullptr == nh)
        return;
    nh->_HandleLogMessage(category, sev, message);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler::_HandleLogMessage(Utf8CP category, LoggingSeverity severity, Utf8CP message)
    {
    NativeLogging::LoggingManager::GetLogger(category)->message(ToNativeLoggingSeverity(severity), message);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::_OnHostTermination(bool px)
    {
    delete this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void/*Json::Value*/ DgnPlatformLib::Host::ScriptAdmin::EvaluateScript(Utf8CP script)
    {
    BeJsContext::EvaluateStatus evstatus;
    BeJsContext::EvaluateException evexception;
    /*auto res = */GetDgnScriptContext().EvaluateScript(script, "file:///DgnScriptContext.js", &evstatus, &evexception);
    //m_jsContext->
    if (BeJsContext::EvaluateStatus::Success != evstatus)
        {
        if (BeJsContext::EvaluateStatus::ParseError==evstatus)
            HandleScriptError(ScriptNotificationHandler::Category::ParseError, "", "");
        else
            HandleScriptError(ScriptNotificationHandler::Category::Exception, evexception.message.c_str(), evexception.trace.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      02/16
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::_ThrowException(Utf8CP exname, Utf8CP details)
    {
#ifdef WIP_BEJAVASCRIPT // *** BeJsContext is not quite ready to return values while JsRT is in an exception state
    BeJsContext::EvaluateStatus status; // We do have to ask for the status. Otherwise BeJsContext will assert.
    // BeJsContext::EvaluateException exception; *** NB: Don't request the exception info. If you do, then BeJsContext will clear the exception, prevent it from being propagated to the caller.
    GetDgnScriptContext().EvaluateScript(Utf8PrintfString("Bentley.Dgn.ThrowException('%s', '%s')", exname, details? details: ""),
                                            "file://DgnJsApi.js", &status, nullptr);
#else
    T_HOST.GetScriptAdmin().HandleScriptError(DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler::Category::Exception, exname, details);
#endif
    }

