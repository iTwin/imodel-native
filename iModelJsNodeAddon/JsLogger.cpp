/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "JsLogger.h"
#include "IModelJsNative.h"

using namespace IModelJsNative;

/** call the JavaScript logger */
void JsLogger::logToJs(Utf8CP category, SEVERITY sev, Utf8CP msg) {
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
SEVERITY JsLogger::JsLevelToSeverity(Napi::Number jsLevel) {
    int logLevel = jsLevel == jsLevel.Env().Undefined() ? 4 : jsLevel.ToNumber().Int32Value();
    switch (logLevel) {
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

/** Synchronize the native std::map on this logger with the "_categoryFilter" map on the JavaScript object. */
void JsLogger::SyncLogLevels() {
    m_defaultSeverity = JsLevelToSeverity(m_loggerObj.Get("_minLevel").As<Napi::Number>());

    auto undefined = m_loggerObj.Env().Undefined();
    auto jsCategoryFilter = m_loggerObj.Get("_categoryFilter").As<Napi::Object>();
    if (jsCategoryFilter == undefined)
        BeNapi::ThrowJsException(JsInterop::Env(), "Invalid Logger");

    m_categoryFilter.clear();
    auto iterator = jsCategoryFilter.Get("entries").As<Napi::Function>().Call(jsCategoryFilter, {}).As<Napi::Object>();
    auto next = iterator.Get("next").As<Napi::Function>();
    while (true) {
        auto entry = next.Call(iterator, {}).As<Napi::Object>();
        if (entry.Get("done").ToBoolean())
            break;
        auto arr = entry.Get("value").As<Napi::Array>();
        if (arr == undefined)
            break;
        auto categoryName = arr.Get(0u).As<Napi::String>();
        auto logLevel = arr.Get(1u).As<Napi::Number>();
        if (categoryName != undefined && logLevel != undefined)
            m_categoryFilter[categoryName.Utf8Value()] = JsLevelToSeverity(logLevel);
    }
}

bool JsLogger::canUseJavaScript() {
    return !m_loggerObj.IsEmpty() && !JsInterop::IsJsExecutionDisabled();
}

bool JsLogger::IsSeverityEnabled(Utf8CP category, SEVERITY sev) {
    SEVERITY severity = m_defaultSeverity;
    auto it = m_categoryFilter.find(category);
    if (it != m_categoryFilter.end())
        severity = it->second;
    return (sev >= severity);
}

void JsLogger::deferLogging(Utf8CP category, SEVERITY sev, Utf8CP msg) {
    BeMutexHolder lock(m_deferredLogMutex);
    if (IsSeverityEnabled(category, sev))
       m_deferredLogging.push_back(LoggedMessage(category, sev, msg));
}

void JsLogger::processDeferred() {
    BeMutexHolder lock(m_deferredLogMutex);
    if (m_deferredLogging.empty())
       return;

    for (auto const& message : m_deferredLogging)
        logToJs(message.m_category.c_str(), message.m_severity, message.m_message.c_str());

    m_deferredLogging.clear();
}

void JsLogger::OnExit() {
    m_deferredLogging.clear();
    m_loggerObj.Reset();
}

void JsLogger::LogMessage(Utf8CP category, SEVERITY sev, Utf8CP msg) {
    if (m_loggerObj.IsEmpty())
        return; // before we've been initialized, skip

    if (!canUseJavaScript()) {
       deferLogging(category, sev, msg);
    } else {
       processDeferred();
       logToJs(category, sev, msg);
    }
}
