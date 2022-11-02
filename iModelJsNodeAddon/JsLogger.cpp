/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "JsLogger.h"
#include "IModelJsNative.h"

using namespace IModelJsNative;

void JsLogger::clearSeverities()  {
    BeSystemMutexHolder ___;
    m_categorySeverity.clear();
  }

/** call the JavaScript logger */
void JsLogger::logToJs(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg) {
    if (JsInterop::IsJsExecutionDisabled())
        return;

    auto env = m_loggerObj.Env();
    Napi::HandleScope scope(env);

    Utf8CP fname = (sev == NativeLogging::LOG_TRACE) ?   "logTrace":
                   (sev == NativeLogging::LOG_DEBUG) ?   "logTrace": // Logger does not distinguish between trace and debug
                   (sev == NativeLogging::LOG_INFO) ?    "logInfo":
                   (sev == NativeLogging::LOG_WARNING) ? "logWarning":
                                                        "logError"; // Logger does not distinguish between error and fatal

    auto method = m_loggerObj.Get(fname).As<Napi::Function>();
    if (method == env.Undefined())
        BeNapi::ThrowJsException(JsInterop::Env(), Utf8PrintfString("Invalid Logger -- missing %s function", fname).c_str());

    method({toJsString(env, category), toJsString(env, msg)});
    }

/** Get the native log level setting for a category */
SEVERITY JsLogger::getJsGetLogLevel(Utf8StringCR category) {
    if (!canUseJavaScript()) {
       BeAssert(false);
       return NativeLogging::LOG_TRACE;
    }

    auto env = m_loggerObj.Env();
    Napi::HandleScope scope(env);

    auto method = m_loggerObj.Get("getLevel").As<Napi::Function>();
    if (method == env.Undefined())
       BeNapi::ThrowJsException(env, "Invalid Logger");

    auto catJS = toJsString(env, category);
    auto jsLevelValue = method({catJS});
    int logLevel = (jsLevelValue == env.Undefined()) ? 4 : jsLevelValue.ToNumber().Int32Value();

    switch (logLevel) {
    case 0:
       return NativeLogging::LOG_TRACE;
    case 1:
       return NativeLogging::LOG_INFO;
    case 2:
       return NativeLogging::LOG_WARNING;
    case 3:
       return NativeLogging::LOG_ERROR;
    case 4:
       return (NativeLogging::SEVERITY)(NativeLogging::LOG_FATAL + 1); // LogLevel.None => Fake importance that is higher than the highest that could really be set.
    }
    BeAssert(false);
    return NativeLogging::LOG_TRACE;
}

bool JsLogger::canUseJavaScript() {
    return !m_loggerObj.IsEmpty() && !JsInterop::IsJsExecutionDisabled();
}

bool JsLogger::IsSeverityEnabled(Utf8CP category, NativeLogging::SEVERITY sev) {
    auto entry = m_categorySeverity.find(category);
    if (entry != m_categorySeverity.end())
       return sev >= entry->second;

    if (!canUseJavaScript())
       return true;

    auto level = getJsGetLogLevel(category);
    m_categorySeverity[category] = level;
    return sev >= level;
}

void JsLogger::deferLogging(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg) {
    BeSystemMutexHolder ___;
    if (IsSeverityEnabled(category, sev))
       m_deferredLogging.push_back(LoggedMessage(category, sev, msg));
}

void JsLogger::processDeferred() {
    BeSystemMutexHolder ___;
    if (m_deferredLogging.empty())
       return;

    Napi::HandleScope scope(m_loggerObj.Env());
    for (auto const& message : m_deferredLogging) {
       if (IsSeverityEnabled(message.m_category.c_str(), message.m_severity))
           logToJs(message.m_category.c_str(), message.m_severity, message.m_message.c_str());
    }

    m_deferredLogging.clear();
}

void JsLogger::OnExit() {
    m_deferredLogging.clear();
    if (!m_loggerObj.IsEmpty())
        m_loggerObj.Reset();
}

void JsLogger::LogMessage(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg) {
    if (m_loggerObj.IsEmpty())
        return; // before we've been initialized, skip

    if (!canUseJavaScript()) {
       deferLogging(category, sev, msg);
    } else {
       processDeferred();
       logToJs(category, sev, msg);
    }
}
