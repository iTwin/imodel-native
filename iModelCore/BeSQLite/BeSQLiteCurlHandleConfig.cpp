/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#define _CRT_SECURE_NO_WARNINGS
#include "SQLite/sqlite3.h"
#include <curl/curl.h>
#include <assert.h>
#if !defined(ANDROID) && !defined(__linux__)
#define ENABLE_PROXYRES
#endif  // !ANDROID
#ifdef ENABLE_PROXYRES
#include <proxyres/proxyres.h>
#endif  // ENABLE_PROXYRES
#include <cstdlib>
#include <ctime>
#include <map>
#include <string>
#include <utility>
#ifndef ITWIN_DAEMON
#include <Bentley/Logging.h>
#endif  // ITWIN_DAEMON
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif  // __APPLE__

static std::string s_empty;

// RAII wrapper around a sqlite3_mutex.
// Note: this is designed only for use in this file, so is very minimal.
class SQLiteMutexLock {
    sqlite3_mutex* m_mutex;

   public:
    explicit SQLiteMutexLock(sqlite3_mutex* mutex) : m_mutex(mutex) {
        if (nullptr == m_mutex) {
            assert(false && "Mutex not initialized.");
            throw "Null mutex";
        }
        sqlite3_mutex_enter(m_mutex);
    }
    ~SQLiteMutexLock() {
        // Note: constructor throws if m_mutex is nullptr; if that happens, the destructor isn't called.
        sqlite3_mutex_leave(m_mutex);
    }
    // Prevent copying
    SQLiteMutexLock(const SQLiteMutexLock&) = delete;
    SQLiteMutexLock& operator=(const SQLiteMutexLock&) = delete;
};

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

#else  // ITWIN_DAEMON

void logTrace(const char* /*fmt*/, ...) {
    // NativeLogging isn't available daemon mode
}

void logError(const char* /*fmt*/, ...) {
    // NativeLogging isn't available daemon mode
}

#endif  // !ITWIN_DAEMON

#ifdef __APPLE__

static std::string s_certsPath;
static sqlite3_mutex* s_appleMutex = nullptr;

static const void initAppleMutex() {
    s_appleMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
}

static std::string& getCACertPath() {
    try {
        SQLiteMutexLock lock(s_appleMutex);
        if (s_certsPath.empty()) {
            uint32_t size = 0;
            _NSGetExecutablePath(nullptr, &size);  // get the size needed
            if (size == 0) {
                return s_empty;
            }
            std::string executablePath;
            executablePath.resize(size);
            if (_NSGetExecutablePath(&executablePath[0], &size) != 0) {
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
        return s_certsPath;
    } catch (...) {
        logError("getCACertPath: s_appleMutex not initialized.");
        return s_empty;
    }
}

void besqlite_bcv_set_cacert_path(const std::string& caFilename) {
    s_certsPath = caFilename;
}

#endif  // __APPLE__

#ifdef ENABLE_PROXYRES

static sqlite3_mutex* s_envMutex = nullptr;
static sqlite3_mutex* s_proxyMapMutex = nullptr;
static sqlite3_mutex* s_initMutex = nullptr;

static const void initProxyResMutexes() {
    s_envMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
    s_proxyMapMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
    s_initMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
}

static const std::string& getEnvProxy() {
    static bool envChecked = false;
    static std::string envProxy;

    try {
        SQLiteMutexLock lock(s_envMutex);
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
    } catch (...) {
        logError("getEnvProxy: s_envMutex not initialized");
    }
    return envProxy;
}

typedef std::pair<std::time_t, std::string> TimeStringPair;

static const std::string& getProxyForUrl(const std::string& url) {
    static std::map<std::string, TimeStringPair> proxyMap;
    const std::string& envProxy = getEnvProxy();

    try {
        SQLiteMutexLock lock(s_proxyMapMutex);
        if (!envProxy.empty()) {
            return envProxy;
        }
        auto schemeEnd = url.find("://");
        if (schemeEnd == std::string::npos) {
            logTrace("\nCannot determine proxy: invalid URL: %s\n", url.c_str());
            return s_empty;
        }
        auto hostEnd = url.find('/', schemeEnd + 3);
        std::string key = url.substr(0, hostEnd);
        std::time_t now = std::time(nullptr);
        auto it = proxyMap.find(key);
        if (it != proxyMap.end()) {
            if (now - it->second.first > 3600) {  // cache for 1 hour
                proxyMap.erase(it);
            } else {
                return it->second.second;
            }
        }
        void* pProxy = proxy_resolver_create();
        if (nullptr == pProxy) {
            return s_empty;
        }
        proxy_resolver_get_proxies_for_url(pProxy, url.c_str());
        proxy_resolver_wait(pProxy, -1);
        const char* list = proxy_resolver_get_list(pProxy);
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
        return proxyMap[key].second;
    } catch (...) {
        logError("getProxyForUrl: s_proxyMapMutex not initialized.");
        return s_empty;
    }
}

void setupCurlProxy(CURL* pCurl, const char* zUri) {
    static int s_initialized = 0;

    try {
        SQLiteMutexLock lock(s_initMutex);
        if (!s_initialized) {
            proxy_resolver_global_init();
            proxy_config_global_init();
            // proxy_config_set_auto_config_url_override("http://localhost:3001/pac.js"); // for testing
            s_initialized = 1;
        }
    } catch (...) {
        logError("setupCurlProxy: s_initMutex not initialized.");
        return;
    }
    const std::string& proxy = getProxyForUrl(zUri);
    if (!proxy.empty()) {
        curl_easy_setopt(pCurl, CURLOPT_PROXY, proxy.c_str());
    } else {
        curl_easy_setopt(pCurl, CURLOPT_PROXY, NULL);
    }
}

#endif  // ENABLE_PROXYRES

#ifdef __cplusplus
extern "C" {
#endif

int besqlite_bcv_custom_init() {
#ifdef ENABLE_PROXYRES
    initProxyResMutexes();
#endif  // ENABLE_PROXYRES
#ifdef __APPLE__
    initAppleMutex();
#endif  // __APPLE__
    return SQLITE_OK;
}

int besqlite_bcv_curl_handle_config(CURL* pCurl, int /*eMethod*/, const char* zUri) {
#ifdef ENABLE_PROXYRES
    setupCurlProxy(pCurl, zUri);
#endif  // ENABLE_PROXYRES
    curl_easy_setopt(pCurl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_REVOKE_BEST_EFFORT);
#ifdef __APPLE__
    const std::string& certsPath = getCACertPath();
    if (certsPath.empty()) {
        return SQLITE_ERROR;
    }
    curl_easy_setopt(pCurl, CURLOPT_CAINFO, certsPath.c_str());
#endif  // __APPLE__
    return SQLITE_OK;
}

#ifdef __cplusplus
} /* end of the extern "C" block */
#endif
