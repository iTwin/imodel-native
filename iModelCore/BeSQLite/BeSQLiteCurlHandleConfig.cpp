/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#define _CRT_SECURE_NO_WARNINGS
#include "SQLite/sqlite3.h"
#include <curl/curl.h>
#if !defined(ANDROID) && !defined(__linux__)
#define ENABLE_PROXYRES
#endif // !ANDROID
#ifdef ENABLE_PROXYRES
#include <assert.h>
#include <proxyres/proxyres.h>
#endif // ENABLE_PROXYRES
#include <cstdlib>
#include <ctime>
#include <map>
#include <string>
#include <utility>
#ifndef ITWIN_DAEMON
#include <Bentley/Logging.h>
#endif // ITWIN_DAEMON
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif // __APPLE__

static std::string s_empty;

#ifndef ITWIN_DAEMON

void logTrace(Utf8CP fmt, ...) {
    va_list args;
    va_start(args, fmt);
    NativeLogging::Logging::LogMessageVa("CloudSQLiteCurlConfig", NativeLogging::LOG_TRACE, fmt, args);
    va_end(args);
}

void logError(Utf8CP fmt, ...) {
    va_list args;
    va_start(args, fmt);
    NativeLogging::Logging::LogMessageVa("CloudSQLiteCurlConfig", NativeLogging::LOG_ERROR, fmt, args);
    va_end(args);
}

#else // ITWIN_DAEMON

void logTrace(const char* /*fmt*/, ...) {
    // Do nothing in daemon mode
}

void logError(const char* /*fmt*/, ...) {
    // Do nothing in daemon mode
}

#endif // !ITWIN_DAEMON

#ifdef __APPLE__

static std::string s_certsPath;
static sqlite3_mutex *s_appleMutex = nullptr;

static const void initAppleMutex() {
    s_appleMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
}

static std::string &getCACertPath() {
    if (s_appleMutex == NULL) {
        assert(false && "s_appleMutex not initialized");
        logError("getCACertPath: s_appleMutex not initialized");
        return s_empty;
    }
    sqlite3_mutex_enter(s_appleMutex);
    if (s_certsPath.empty()) {
        uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size); // get the size needed
        if (size == 0) {
            sqlite3_mutex_leave(s_appleMutex);
            return s_empty;
        }
        std::string executablePath;
        executablePath.resize(size);
        if (_NSGetExecutablePath(&executablePath[0], &size) != 0) {
            sqlite3_mutex_leave(s_appleMutex);
            return s_empty;
        }
        const std::string suffix = "/Assets/cacert.pem";
        size_t lastSlashIndex = executablePath.rfind('/');
        if (lastSlashIndex != std::string::npos) {
            s_certsPath = executablePath.substr(0, lastSlashIndex) + suffix;
        } else {
            // If there aren't any slashes in the executable path, just use the current directory.
            s_certsPath = std::string(".") + suffix;
        }
    }
    sqlite3_mutex_leave(s_appleMutex);
    return s_certsPath;
}

void besqlite_bcv_set_cacert_path(const std::string& caFilename) {
    s_certsPath = caFilename;
}

#endif // __APPLE__

#ifdef ENABLE_PROXYRES

static sqlite3_mutex *s_envMutex = nullptr;
static sqlite3_mutex *s_proxyMapMutex = nullptr;
static sqlite3_mutex *s_initMutex = nullptr;

static const void initProxyResMutexes() {
    s_envMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
    s_proxyMapMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
    s_initMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
}

static const std::string& getEnvProxy() {
    static bool envChecked = false;
    static std::string envProxy;

    if (nullptr == s_envMutex) {
        assert(false && "s_envMutex not initialized");
        logError("getEnvProxy: s_envMutex not initialized");
        return envProxy;
    }
    sqlite3_mutex_enter(s_envMutex);
    if (!envChecked) {
        const char* httpsProxy = std::getenv("https_proxy");
        if (httpsProxy != nullptr) {
            envProxy = httpsProxy;
        } else {
            const char* httpProxy = std::getenv("http_proxy");
            if (httpProxy != nullptr) {
                envProxy = httpProxy;
            }
        }
        if (!envProxy.empty()) {
            logTrace("Using proxy from environment: <%s>\n", envProxy.c_str());
        }
        envChecked = true;
    }
    sqlite3_mutex_leave(s_envMutex);
    return envProxy;
}

typedef std::pair<std::time_t, std::string> TimeStringPair;

static const std::string& getProxyForUrl(const std::string& url) {
    static std::map<std::string, TimeStringPair> proxyMap;
    const std::string& envProxy = getEnvProxy();

    if (nullptr == s_proxyMapMutex) {
        assert(false && "s_proxyMapMutex not initialized");
        logError("getProxyForUrl: s_proxyMapMutex not initialized");
        return s_empty;
    }
    sqlite3_mutex_enter(s_proxyMapMutex);
    if (!envProxy.empty()) {
        sqlite3_mutex_leave(s_proxyMapMutex);
        return envProxy;
    }
    auto schemeEnd = url.find("://");
    if (schemeEnd == std::string::npos) {
        logTrace("\nCannot determine proxy: invalid URL: %s\n", url.c_str());
        sqlite3_mutex_leave(s_proxyMapMutex);
        return s_empty;
    }
    auto hostEnd = url.find('/', schemeEnd + 3);
    std::string key = url.substr(0, hostEnd);
    std::time_t now = std::time(nullptr);
    auto it = proxyMap.find(key);
    if (it != proxyMap.end()) {
        if (now - it->second.first > 3600) { // cache for 1 hour
            proxyMap.erase(it);
        } else {
            sqlite3_mutex_leave(s_proxyMapMutex);
            return it->second.second;
        }
    }
    void* pProxy = proxy_resolver_create();
    if (nullptr == pProxy) {
        sqlite3_mutex_leave(s_proxyMapMutex);
        return s_empty;
    }
    proxy_resolver_get_proxies_for_url(pProxy, url.c_str());
    proxy_resolver_wait(pProxy, -1);
    const char *list = proxy_resolver_get_list(pProxy);
    std::string proxy;
    if (list != nullptr) {
        proxy = list;
        auto commaSpot = proxy.find(',');
        if (commaSpot != std::string::npos) {
            proxy = proxy.substr(0, commaSpot);
        } else {
            proxy = list;
        }
        if (proxy == "direct://") {
            proxy = "";
        }
    }
    proxy_resolver_delete(&pProxy);
    if (!proxy.empty()) {
        logTrace("Resolved proxy for URL %s: <%s>\n", url.c_str(), proxy.c_str());
    }
    proxyMap[key] = std::make_pair(now, proxy);
    sqlite3_mutex_leave(s_proxyMapMutex);
    return proxyMap[key].second;
}

void setupCurlProxy(CURL * pCurl, const char * zUri) {
    static int s_initialized = 0;

    if (nullptr == s_initMutex) {
        assert(false && "s_initMutex not initialized");
        logError("setupCurlProxy: s_initMutex not initialized");
        return;
    }
    sqlite3_mutex_enter(s_initMutex);
    if (!s_initialized) {
        proxy_resolver_global_init();
        proxy_config_global_init();
        // proxy_config_set_auto_config_url_override("http://localhost:3001/pac.js"); // for testing
        s_initialized = 1;
    }
    sqlite3_mutex_leave(s_initMutex);
    const std::string& proxy = getProxyForUrl(zUri);
    if (!proxy.empty()) {
        curl_easy_setopt(pCurl, CURLOPT_PROXY, proxy.c_str());
    } else {
        curl_easy_setopt(pCurl, CURLOPT_PROXY, NULL);
    }
}

#endif // ENABLE_PROXYRES

#ifdef __cplusplus
extern "C" {
#endif

int besqlite_bcv_custom_init() {
#ifdef ENABLE_PROXYRES
    initProxyResMutexes();
#endif // ENABLE_PROXYRES
#ifdef __APPLE__
    initAppleMutex();
#endif // __APPLE__
    return SQLITE_OK;
}

int besqlite_bcv_curl_handle_config(CURL * pCurl, int /*eMethod*/, const char * zUri) {
#ifdef ENABLE_PROXYRES
    setupCurlProxy(pCurl, zUri);
#endif // ENABLE_PROXYRES
    curl_easy_setopt(pCurl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_REVOKE_BEST_EFFORT);
#ifdef __APPLE__
    const std::string& certsPath = getCACertPath();
    if (certsPath.empty()) {
        return SQLITE_ERROR;
    }
    curl_easy_setopt(pCurl, CURLOPT_CAINFO, certsPath.c_str());
#endif // __APPLE__
    return SQLITE_OK;
}

#ifdef __cplusplus
} /* end of the extern "C" block */
#endif
