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

/** Synchronize the native std::map on this logger with the "_categoryFilter" map on the JavaScript logger object. */
void JsLogger::SyncLogLevels() {
    // first sync the default severity level for categories not specified
    m_defaultSeverity = JsLevelToSeverity(m_loggerObj.Get("_minLevel").As<Napi::Number>());

    auto undefined = m_loggerObj.Env().Undefined();
    // the category mask is stored in a JavaScript map object called "_categoryFilter". Note: it is a private member
    // in the TypeScript class, but that doesn't stop us from accessing it in JavaScript.
    auto jsCategoryFilter = m_loggerObj.Get("_categoryFilter").As<Napi::Object>();
    if (jsCategoryFilter == undefined)
        BeNapi::ThrowJsException(JsInterop::Env(), "Invalid Logger");

    m_categoryFilter.clear();
    // iterate over the entries in the category filter map by calling the "entries" method
    auto iterator = jsCategoryFilter.Get("entries").As<Napi::Function>().Call(jsCategoryFilter, {}).As<Napi::Object>();
    auto next = iterator.Get("next").As<Napi::Function>();
    while (true) {
        auto entry = next.Call(iterator, {}).As<Napi::Object>(); // request the next entry in the map
        if (entry.Get("done").ToBoolean()) // are we at the end of the map?
            break;
        // the entry is in a member called "value" that is a array: [categoryName: string, logLevel: number]
        auto arr = entry.Get("value").As<Napi::Array>();
        if (arr == undefined)
            break; // should never happen
        auto categoryName = arr.Get(0u).As<Napi::String>();
        auto logLevel = arr.Get(1u).As<Napi::Number>();
        if (categoryName != undefined && logLevel != undefined)
            m_categoryFilter[categoryName.Utf8Value()] = JsLevelToSeverity(logLevel);
    }
}

/** see if it is safe to use the JavaScript logger. That will be true if the logger has been initialized and we're on the main thread. */
bool JsLogger::canUseJavaScript() {
    return !m_loggerObj.IsEmpty() && !JsInterop::IsJsExecutionDisabled();
}

/** Given a category and severity level, return true if it should be logged.
 * @note Must be called with deferredMutex held
 */
bool JsLogger::isEnabled(Utf8CP category, SEVERITY sev) {
    SEVERITY severity = m_defaultSeverity;
    auto it = m_categoryFilter.find(category);
    if (it != m_categoryFilter.end())
        severity = it->second;
    return (sev >= severity);
}

/** Send all deferred log messages to the JavaScript logger
 * @note Must be called with deferredMutex held
 */
void JsLogger::processDeferred() {
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
    BeMutexHolder lock(m_deferredLogMutex);
    if (!isEnabled(category, sev))
        return;

    if (canUseJavaScript()) {
        processDeferred();
        logToJs(category, sev, msg);
    } else {
        // save this message in memory so it can be logged later on the JavaScript thread
        m_deferredLogging.push_back(LoggedMessage(category, sev, msg));
    }
}

bool JsLogger::IsSeverityEnabled(Utf8CP category, SEVERITY sev) {
    BeMutexHolder lock(m_deferredLogMutex);
    return isEnabled(category, sev);
}

void JsLogger::FlushDeferred() {
    BeMutexHolder lock(m_deferredLogMutex);
    processDeferred();
}
