/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "IModelBank.h"
#include <Napi/napi.h>

namespace IModelBank
{
struct LogMessage
{
    Utf8String m_category;
    Utf8String m_message;
    NativeLogging::SEVERITY m_severity;
};

static bvector<LogMessage> *s_deferredLogging;

extern Napi::ObjectReference &jsInterop_getLogger();

//=======================================================================================
// @bsistruct                                   Sam.Wilson                  02/18
//=======================================================================================
struct NativeLoggingShim : NativeLogging::Provider::ILogProvider
{
    int STDCALL_ATTRIBUTE Initialize() override { return SUCCESS; }

    int STDCALL_ATTRIBUTE Uninitialize() override { return SUCCESS; }

    int STDCALL_ATTRIBUTE CreateLogger(WCharCP nameSpace, NativeLogging::Provider::ILogProviderContext **ppContext) override
    {
        *ppContext = reinterpret_cast<NativeLogging::Provider::ILogProviderContext *>(new WString(nameSpace));
        return SUCCESS;
    }

    int STDCALL_ATTRIBUTE DestroyLogger(NativeLogging::Provider::ILogProviderContext *pContext) override
    {
        WString *ns = reinterpret_cast<WString *>(pContext);
        if (nullptr != ns)
            delete ns;
        return SUCCESS;
    }

    int STDCALL_ATTRIBUTE SetOption(WCharCP attribName, WCharCP attribValue) override
    {
        BeAssert(false);
        return SUCCESS;
    }

    int STDCALL_ATTRIBUTE GetOption(WCharCP attribName, WCharP attribValue, uint32_t valueSize) override { return ERROR; }

    void STDCALL_ATTRIBUTE LogMessage(NativeLogging::Provider::ILogProviderContext *context, NativeLogging::SEVERITY sev, WCharCP msg) override
    {
        LogMessage(context, sev, Utf8String(msg).c_str());
    }

    int STDCALL_ATTRIBUTE SetSeverity(WCharCP nameSpace, NativeLogging::SEVERITY severity) override
    {
        BeAssert(false && "only the app (in TypeScript) sets severities");
        return ERROR;
    }

    void STDCALL_ATTRIBUTE LogMessage(NativeLogging::Provider::ILogProviderContext *context, NativeLogging::SEVERITY sev, Utf8CP msg) override
    {
        WString *ns = reinterpret_cast<WString *>(context);
        JsInterop::LogMessage(Utf8String(*ns).c_str(), sev, msg);
    }

    bool STDCALL_ATTRIBUTE IsSeverityEnabled(NativeLogging::Provider::ILogProviderContext *context, NativeLogging::SEVERITY sev) override
    {
        WString *ns = reinterpret_cast<WString *>(context);
        return JsInterop::IsSeverityEnabled(Utf8String(*ns).c_str(), sev);
    }
};

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void logMessageToJs(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg)
{
    if (JsInterop::IsJsExecutionDisabled())
        {
        BeAssert(false);
        return;
        }

    auto env = jsInterop_getLogger().Env();
    Napi::HandleScope scope(env);

    Utf8CP fname = (sev == NativeLogging::LOG_TRACE) ? "logTrace" : (sev == NativeLogging::LOG_DEBUG) ? "logTrace" :                                                                    // Logger does not distinguish between trace and debug
                                                                        (sev == NativeLogging::LOG_INFO) ? "logInfo" : (sev == NativeLogging::LOG_WARNING) ? "logWarning" : "logError"; // Logger does not distinguish between error and fatal

    auto method = jsInterop_getLogger().Get(fname).As<Napi::Function>();
    if (method == env.Undefined())
    {
        JsInterop::ThrowJsException("Invalid Logger");
        return;
    }

    auto catJS = Napi::String::New(env, category);
    auto msgJS = Napi::String::New(env, msg);

    method({catJS, msgJS});
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static bool callIsLogLevelEnabledJs(Utf8CP category, NativeLogging::SEVERITY sev)
{
    if (JsInterop::IsJsExecutionDisabled())
        {
        BeAssert(false);
        return false;
        }

    auto env = jsInterop_getLogger().Env();
    Napi::HandleScope scope(env);

    auto method = jsInterop_getLogger().Get("isEnabled").As<Napi::Function>();
    if (method == env.Undefined())
    {
        JsInterop::ThrowJsException("Invalid Logger");
        return true;
    }

    auto catJS = Napi::String::New(env, category);

    int llevel = (sev <= NativeLogging::LOG_TRACE) ? 0 : (sev <= NativeLogging::LOG_DEBUG) ? 0 :                                                 // Logger does not distinguish between trace and debug
                                                             (sev == NativeLogging::LOG_INFO) ? 1 : (sev == NativeLogging::LOG_WARNING) ? 2 : 3; // Logger does not distinguish between error and fatal

    auto levelJS = Napi::Number::New(env, llevel);

    return method({catJS, levelJS}).ToBoolean();
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void deferLogging(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg)
{
    BeSystemMutexHolder ___;

    if (!s_deferredLogging)
        s_deferredLogging = new bvector<LogMessage>();

    LogMessage lm;
    lm.m_category = category;
    lm.m_message = msg;
    lm.m_severity = sev;
    s_deferredLogging->push_back(lm);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void doDeferredLogging()
{
    if (JsInterop::IsJsExecutionDisabled())
        {
        BeAssert(false);
        return;
        }
        
    BeSystemMutexHolder ___;
    if (!s_deferredLogging)
        return;

    for (auto const &lm : *s_deferredLogging)
    {
        logMessageToJs(lm.m_category.c_str(), lm.m_severity, lm.m_message.c_str());
    }

    delete s_deferredLogging;
    s_deferredLogging = nullptr;
}

} // namespace IModelBank

using namespace IModelBank;

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelBank::JsInterop::LogMessage(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg)
{
    if (IModelBank::jsInterop_getLogger().IsEmpty() || IsJsExecutionDisabled())
    {
        IModelBank::deferLogging(category, sev, msg);
        return;
    }

    IModelBank::doDeferredLogging();
    IModelBank::logMessageToJs(category, sev, msg);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool IModelBank::JsInterop::IsSeverityEnabled(Utf8CP category, NativeLogging::SEVERITY sev)
{
    if (IModelBank::jsInterop_getLogger().IsEmpty() || IsJsExecutionDisabled())
        return true;

    IModelBank::doDeferredLogging();
    return IModelBank::callIsLogLevelEnabledJs(category, sev);
}

//---------------------------------------------------------------------------------------
// @bsimethod                               Ramanujam.Raman                 07/17
//---------------------------------------------------------------------------------------
void JsInterop::InitLogging()
{
    NativeLogging::LoggingConfig::ActivateProvider(new NativeLoggingShim());
}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  05/17
//---------------------------------------------------------------------------------------
NativeLogging::ILogger &JsInterop::GetNativeLogger()
{
    static NativeLogging::ILogger *s_nativeLogger;
    if (nullptr == s_nativeLogger)
        s_nativeLogger = NativeLogging::LoggingManager::GetLogger("imodel-bank"); // This is thread-safe. The assignment is atomic, and GetLogger will always return the same value for a given key anyway.
    return *s_nativeLogger;
}
