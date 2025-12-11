/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#define _CRT_SECURE_NO_WARNINGS
#include "SQLite/sqlite3.h"
#include <curl/curl.h>
#ifndef ANDROID
#define ENABLE_PROXYRES
#endif // !ANDROID
#ifdef ENABLE_PROXYRES
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
typedef std::pair<std::time_t, std::string> TimeStringPair;
static std::map<std::string, TimeStringPair> s_proxyMap;
static std::string s_envProxy;

#ifndef ITWIN_DAEMON

void logTrace(Utf8CP fmt, ...) {
    va_list args;
    va_start(args, fmt);
    NativeLogging::Logging::LogMessageVa("CloudSQLiteCurlConfig", NativeLogging::LOG_TRACE, fmt, args);
    va_end(args);
}

#else // ITWIN_DAEMON

void logTrace(const char* /*fmt*/, ...) {
    // Do nothing in daemon mode
}

#endif // !ITWIN_DAEMON

#ifdef __APPLE__

static std::string s_certsPath;

static std::string &getCACertPath() {
    if (s_certsPath.empty()) {
        uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size); // get the size needed
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
}

void besqlite_bcv_set_cacert_path(const std::string& caFilename) {
    s_certsPath = caFilename;
}

#endif // __APPLE__

#ifdef ENABLE_PROXYRES

static const std::string& getEnvProxy() {
    static bool envChecked = false;
    if (!envChecked) {
        const char* httpsProxy = std::getenv("https_proxy");
        if (httpsProxy != nullptr) {
            s_envProxy = httpsProxy;
        } else {
            const char* httpProxy = std::getenv("http_proxy");
            if (httpProxy != nullptr) {
                s_envProxy = httpProxy;
            }
        }
        if (!s_envProxy.empty()) {
            logTrace("Using proxy from environment: <%s>\n", s_envProxy.c_str());
        }
        envChecked = true;
    }
    return s_envProxy;
}

static const std::string& getProxyForUrl(const std::string& url) {
    const std::string& envProxy = getEnvProxy();
    if (!envProxy.empty()) {
        // printf("\nUsing proxy from environment: <%s>\n", envProxy.c_str());
        return envProxy;
    }
    auto schemeEnd = url.find("://");
    if (schemeEnd == std::string::npos) {
        logTrace("\nCannot determine proxy: invalid URL: %s\n", url.c_str());
        // printf("\nInvalid URL: %s\n", url.c_str());
        return s_empty;
    }
    auto hostEnd = url.find('/', schemeEnd + 3);
    std::string key = url.substr(0, hostEnd);
    std::time_t now = std::time(nullptr);
    auto it = s_proxyMap.find(key);
    if (it != s_proxyMap.end()) {
        if (now - it->second.first > 3600) { // cache for 1 hour
            s_proxyMap.erase(it);
        } else {
            // printf("\nUsing cached proxy for key %s: <%s>\n", key.c_str(), it->second.second.c_str());
            return it->second.second;
        }
    }
    void* pProxy = proxy_resolver_create();
    if (nullptr == pProxy) {
        // printf("\nFailed to create proxy resolver!\n");
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
    // printf("\nCaching proxy for key %s: <%s>\n", key.c_str(), proxy.c_str());
    s_proxyMap[key] = std::make_pair(now, proxy);
    return s_proxyMap[key].second;
}

void setupProxyResolver(CURL * pCurl, const char * zUri) {
    static int s_initialized = 0;
    if (!s_initialized) {
        proxy_resolver_global_init();
        proxy_config_global_init();
        // proxy_config_set_auto_config_url_override("http://localhost:3001/pac.js"); // for testing
        s_initialized = 1;
    }
    const std::string& proxy = getProxyForUrl(zUri);
    if (!proxy.empty()) {
        // printf("Setting proxy to %s for %s\n", proxy.c_str(), zUri);
        curl_easy_setopt(pCurl, CURLOPT_PROXY, proxy.c_str());
    } else {
        // printf("No proxy for %s\n", zUri);
        curl_easy_setopt(pCurl, CURLOPT_PROXY, NULL);
    }
}

#endif // ENABLE_PROXYRES

#ifdef __cplusplus
extern "C" {
#endif

int besqlite_bcv_curl_handle_config(CURL * pCurl, int /*eMethod*/, const char * zUri) {
#ifdef ENABLE_PROXYRES
    setupProxyResolver(pCurl, zUri);
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
