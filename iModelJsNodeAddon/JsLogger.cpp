/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "JsLogger.h"
#include "IModelJsNative.h"

using namespace IModelJsNative;

/** call the JavaScript logger */
void JsLogger::logToJs(Utf8CP category, SEVERITY sev, Utf8CP msg)
    {
    Utf8CP fname = (sev == LOG_TRACE) ?   "logTrace" :
                   (sev == LOG_INFO) ?    "logInfo" :
                   (sev == LOG_WARNING) ? "logWarning" :
                   "logError";

    auto method = m_loggerObj.Get(fname).As<Napi::Function>();
    auto env = m_loggerObj.Env();
    if (method != env.Undefined())
        method({toJsString(env, category), toJsString(env, msg)});
    }

/** convert a JavaScript log level value to a C++ log level value */
SEVERITY JsLogger::JsLevelToSeverity(Napi::Number jsLevel)
    {
    int logLevel = jsLevel == jsLevel.Env().Undefined() ? 4 : jsLevel.ToNumber().Int32Value();
    switch (logLevel)
        {
        case 0:
            return LOG_TRACE;
        case 1:
            return LOG_INFO;
        case 2:
            return LOG_WARNING;
        case 3:
            return LOG_ERROR;
        }
    return LOG_NEVER;
    }

/** Synchronize the native std::map on this logger with the "_categoryFilter" map on the JavaScript logger object. */
void JsLogger::SyncLogLevels()
    {
    BeMutexHolder lock(m_mutex);

    // first sync the default severity level for categories not specified
    m_defaultSeverity = JsLevelToSeverity(m_loggerObj.Get("minLevel").As<Napi::Number>());

    auto undefined = m_loggerObj.Env().Undefined();
    // the category mask is stored in a JavaScript map object called "_categoryFilter". Note: it is a private member
    // in the TypeScript class, but that doesn't stop us from accessing it in JavaScript.
    auto jsCategoryFilter = m_loggerObj.Get("categoryFilter").As<Napi::Object>();
    if (jsCategoryFilter == undefined)
        THROW_JS_IMODEL_NATIVE_EXCEPTION(JsInterop::Env(), "Invalid Logger", IModelJsNativeErrorKey::BadArg);

    m_categoryFilter.clear();

    Napi::HandleScope scope(jsCategoryFilter.Env());
    auto names = jsCategoryFilter.GetPropertyNames();
    for (uint32_t i = 0; i < names.Length(); ++i)
        {
        Utf8String name = names.Get(i).As<Napi::String>().Utf8Value();
        Napi::Value value = jsCategoryFilter[name];
        m_categoryFilter[name] = JsLevelToSeverity(value.As<Napi::Number>());
        }
    }

/** see if it is safe to use the JavaScript logger. That will be true if the logger has been initialized and we're on the main thread. */
bool JsLogger::canUseJavaScript()
    {
    return !m_loggerObj.IsEmpty() && !JsInterop::IsJsExecutionDisabled();
    }

/** Given a category and severity level, return true if it should be logged.
 * @note Must be called with mutex held
 */
bool JsLogger::isEnabled(Utf8CP catIn, SEVERITY sev)
    {
    SEVERITY severity = m_defaultSeverity;
    Utf8String category(catIn);
    auto it = m_categoryFilter.find(category);
    if (it != m_categoryFilter.end())
        {
        severity = it->second;
        }
    else
        {
        // check for a namespace (delimited by ".") within the category.
        auto dot = category.find_last_of('.');
        if (std::string::npos != dot)
            return isEnabled(category.substr(0, dot).c_str(), sev);
        }
    return (sev >= severity);
    }

void JsLogger::Cleanup()
    {
    BeMutexHolder lock(m_mutex);
    m_loggerObj.Reset();
    if (m_processLogsOnMainThread)
        m_processLogsOnMainThread.Release();
    }

void JsLogger::LogMessage(Utf8CP category, SEVERITY sev, Utf8CP msg)
    {
    if (!IsSeverityEnabled(category, sev))
        return;

    if (canUseJavaScript())
        {
        logToJs(category, sev, msg);
        return;
        }

    BeMutexHolder lock(m_mutex);
    if (m_processLogsOnMainThread)
        {
        m_processLogsOnMainThread.NonBlockingCall([this, category = Utf8String(category), sev, msg = Utf8String(msg)](Napi::Env, Napi::Function)
            {
            try {
                logToJs(category.c_str(), sev, msg.c_str());
                }
            catch (...)
                {
                // logToJs triggers the backend JavaScript logging functionality. That functionality
                // allows for custom log functions. Those custom log functions may throw an exception,
                // and if they do so, that exception gets propagated to here. Re-throwing it here kills
                // the node process, so we just silently ignore it.
                }
            });
        }
    }

bool JsLogger::IsSeverityEnabled(Utf8CP category, SEVERITY sev)
    {
    BeMutexHolder lock(m_mutex);
    return isEnabled(category, sev);
    }

Napi::Value JsLogger::GetJsLogger()
    {
    return m_loggerObj.Value();
    }

void JsLogger::SetJsLogger(Napi::CallbackInfo const& info)
    {
    BeMutexHolder lock(m_mutex);
    Cleanup();
    m_processLogsOnMainThread = Napi::ThreadSafeFunction::New(info.Env(), Napi::Function::New(info.Env(), [](Napi::CallbackInfo const&){}), "JsLogger", 0, 1);
    m_processLogsOnMainThread.Unref(info.Env());
    m_loggerObj = Napi::ObjectReference::New(info[0].ToObject(), 1);
    m_loggerObj.SuppressDestruct();
    SyncLogLevels();
    }