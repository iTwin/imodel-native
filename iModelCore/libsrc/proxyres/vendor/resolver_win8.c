#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <windows.h>
#include <winhttp.h>

#include "config.h"
#include "event.h"
#include "log.h"
#include "resolver.h"
#include "resolver_i.h"
#include "resolver_winxp.h"
#include "util.h"
#include "util_win.h"

// WinHTTP proxy resolver function definitions
typedef DWORD(WINAPI *LPWINHTTPGETPROXYFORURLEX)(HINTERNET Resolver, PCWSTR Url,
                                                 WINHTTP_AUTOPROXY_OPTIONS *AutoProxyOptions, DWORD_PTR Context);
typedef DWORD(WINAPI *LPWINHTTPCREATEPROXYRESOLVER)(HINTERNET Session, HINTERNET *Resolver);
typedef DWORD(WINAPI *LPWINHTTPGETPROXYRESULT)(HINTERNET Resolver, WINHTTP_PROXY_RESULT *ProxyResult);
typedef VOID(WINAPI *LPWINHTTPFREEPROXYRESULT)(WINHTTP_PROXY_RESULT *ProxyResult);

typedef struct g_proxy_resolver_win8_s {
    // WinHTTP module handle
    HMODULE win_http;
    // WinHTTP session handle
    HINTERNET session;
    // WinHTTP proxy resolver functions
    LPWINHTTPGETPROXYFORURLEX winhttp_get_proxy_for_url_ex;
    LPWINHTTPCREATEPROXYRESOLVER winhttp_create_proxy_resolver;
    LPWINHTTPGETPROXYRESULT winhttp_get_proxy_result;
    LPWINHTTPFREEPROXYRESULT winhttp_free_proxy_result;
} g_proxy_resolver_win8_s;

g_proxy_resolver_win8_s g_proxy_resolver_win8;

typedef struct proxy_resolver_win8_s {
    // WinHTTP proxy resolver handle
    HINTERNET resolver;
    // Last system error
    int32_t error;
    // Complete event
    void *complete;
    // Proxy list
    char *list;
} proxy_resolver_win8_s;

void CALLBACK proxy_resolver_win8_winhttp_status_callback(HINTERNET Internet, DWORD_PTR Context, DWORD InternetStatus,
                                                          LPVOID StatusInformation, DWORD StatusInformationLength) {
    proxy_resolver_win8_s *proxy_resolver = (proxy_resolver_win8_s *)Context;
    WINHTTP_PROXY_RESULT proxy_result = {0};

    UNUSED(StatusInformationLength);
    UNUSED(Internet);

    if (InternetStatus == WINHTTP_CALLBACK_FLAG_REQUEST_ERROR) {
        WINHTTP_ASYNC_RESULT *async_result = (WINHTTP_ASYNC_RESULT *)StatusInformation;

        // Failed to detect proxy auto configuration url so use DIRECT connection
        if (async_result->dwError == ERROR_WINHTTP_AUTODETECTION_FAILED) {
            log_debug("Proxy resolution returned code (%lu)", async_result->dwError);
            proxy_resolver->list = strdup("direct://");
            goto win8_async_done;
        }

        proxy_resolver->error = async_result->dwError;
        log_error("Unable to resolve proxy for url (%" PRId32 ")", proxy_resolver->error);
        goto win8_async_done;
    }

    proxy_resolver->error = g_proxy_resolver_win8.winhttp_get_proxy_result(proxy_resolver->resolver, &proxy_result);
    if (proxy_resolver->error != ERROR_SUCCESS) {
        log_error("Unable to retrieve proxy result (%" PRId32 ")", proxy_resolver->error);
        goto win8_async_done;
    }

    if (proxy_result.cEntries == 0) {
        // No support for FindProxyForURL return types HTTP/HTTPS
        proxy_resolver->list = strdup("direct://");
        goto win8_async_done;
    }

    // Allocate string to construct the proxy list into
    int32_t max_list = proxy_result.cEntries * MAX_PROXY_URL;
    proxy_resolver->list = (char *)calloc(max_list, sizeof(char));
    if (!proxy_resolver->list) {
        proxy_resolver->error = ERROR_OUTOFMEMORY;
        log_error("Unable to allocate memory for %s (%" PRId32 ")", "proxy list", proxy_resolver->error);
        goto win8_async_done;
    }

    size_t list_len = 0;

    // Construct proxy list string from WinHTTP proxy result
    for (uint32_t i = 0; i < proxy_result.cEntries; i++) {
        WINHTTP_PROXY_RESULT_ENTRY *entry = &proxy_result.pEntries[i];
        if (!entry)
            continue;

        // Prefix each url with the proxy scheme
        if (entry->fProxy) {
            switch (entry->ProxyScheme) {
            case INTERNET_SCHEME_HTTP:
                strncat(proxy_resolver->list, "http://", max_list - list_len - 1);
                list_len += 7;
                break;
            case INTERNET_SCHEME_HTTPS:
                strncat(proxy_resolver->list, "https://", max_list - list_len - 1);
                list_len += 8;
                break;
            case INTERNET_SCHEME_SOCKS:
                strncat(proxy_resolver->list, "socks://", max_list - list_len - 1);
                list_len += 8;
                break;
            case INTERNET_SCHEME_FTP:
                strncat(proxy_resolver->list, "ftp://", max_list - list_len - 1);
                list_len += 6;
                break;
            }

            // Convert wide char proxy url to UTF-8
            char *proxy_url = wchar_dup_to_utf8(entry->pwszProxy);
            if (proxy_url) {
                strncat(proxy_resolver->list, proxy_url, max_list - list_len - 1);
                list_len += strlen(proxy_url);
                free(proxy_url);
            }

            // Add proxy port to the end of the url
            if (entry->ProxyPort > 0) {
                snprintf(proxy_resolver->list + list_len, max_list - list_len, ":%d", entry->ProxyPort);
                proxy_resolver->list[max_list - 1] = 0;
                list_len = strlen(proxy_resolver->list);
            }
        } else {
            // No proxy
            strncat(proxy_resolver->list, "direct://", max_list - list_len - 1);
            list_len += 9;
        }

        // Separate each proxy url with a comma
        if (i != proxy_result.cEntries - 1) {
            strncat(proxy_resolver->list, ",", max_list - list_len - 1);
            list_len++;
        }
    }

win8_async_done:

    // Free proxy result
    if (proxy_result.cEntries)
        g_proxy_resolver_win8.winhttp_free_proxy_result(&proxy_result);

    event_set(proxy_resolver->complete);
}

bool proxy_resolver_win8_get_proxies_for_url(void *ctx, const char *url) {
    proxy_resolver_win8_s *proxy_resolver = (proxy_resolver_win8_s *)ctx;
    WINHTTP_AUTOPROXY_OPTIONS options = {0};
    WINHTTP_PROXY_INFO proxy_info = {0};
    wchar_t *url_wide = NULL;
    wchar_t *auto_config_url_wide = NULL;
    char *auto_config_url = NULL;
    bool is_ok = false;
    int32_t error = 0;

    auto_config_url = proxy_config_get_auto_config_url();
    if (auto_config_url) {
        // Use auto configuration script specified by system
        auto_config_url_wide = utf8_dup_to_wchar(auto_config_url);
        if (!auto_config_url_wide) {
            proxy_resolver->error = ERROR_OUTOFMEMORY;
            log_error("Unable to allocate memory for %s (%" PRId32 ")", "auto config url", proxy_resolver->error);
            goto win8_done;
        }

        options.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
        options.lpszAutoConfigUrl = auto_config_url_wide;
    } else {
        // Use WPAD to automatically retrieve proxy auto-configuration and evaluate it
        options.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
        options.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
    }

    g_proxy_resolver_win8.winhttp_create_proxy_resolver(g_proxy_resolver_win8.session, &proxy_resolver->resolver);
    if (!proxy_resolver->resolver) {
        proxy_resolver->error = GetLastError();
        log_error("Unable to create proxy resolver (%" PRId32 ")", proxy_resolver->error);
        goto win8_done;
    }

    if (WinHttpSetStatusCallback(proxy_resolver->resolver,
                                 (WINHTTP_STATUS_CALLBACK)proxy_resolver_win8_winhttp_status_callback,
                                 WINHTTP_CALLBACK_FLAG_REQUEST_ERROR | WINHTTP_CALLBACK_FLAG_GETPROXYFORURL_COMPLETE,
                                 (DWORD_PTR)NULL) == WINHTTP_INVALID_STATUS_CALLBACK) {
        proxy_resolver->error = GetLastError();
        log_error("Unable to install status callback (%" PRId32 ")", proxy_resolver->error);
        goto win8_done;
    }

    // Convert url to wide char for WinHttpGetProxyForUrlEx
    url_wide = utf8_dup_to_wchar(url);
    if (!url_wide) {
        proxy_resolver->error = ERROR_OUTOFMEMORY;
        log_error("Unable to allocate memory for %s (%" PRId32 ")", "wide char url", proxy_resolver->error);
        goto win8_done;
    }

    // For performance reasons try fAutoLogonIfChallenged = false then try fAutoLogonIfChallenged = true
    // https://docs.microsoft.com/en-us/windows/win32/api/winhttp/ns-winhttp-winhttp_autoproxy_options

    error = g_proxy_resolver_win8.winhttp_get_proxy_for_url_ex(proxy_resolver->resolver, url_wide, &options,
                                                               (DWORD_PTR)proxy_resolver);
    if (error == ERROR_WINHTTP_LOGIN_FAILURE) {
        options.fAutoLogonIfChallenged = true;
        error = g_proxy_resolver_win8.winhttp_get_proxy_for_url_ex(proxy_resolver->resolver, url_wide, &options,
                                                                   (DWORD_PTR)proxy_resolver);
    }

    if (error != ERROR_IO_PENDING) {
        proxy_resolver->error = error;
        if (error != ERROR_WINHTTP_UNRECOGNIZED_SCHEME)
            log_error("Unable to get proxy for url %s (%" PRId32 ")", url, proxy_resolver->error);
        goto win8_done;
    }

    // WinHttpGetProxyForUrlEx always executes asynchronously
    is_ok = true;
    goto win8_cleanup;

win8_done:

    is_ok = proxy_resolver->list != NULL || proxy_resolver->error == 0;
    event_set(proxy_resolver->complete);

win8_cleanup:

    free(url_wide);
    free(auto_config_url_wide);
    free(auto_config_url);

    // Free proxy info
    if (proxy_info.lpszProxy)
        GlobalFree(proxy_info.lpszProxy);
    if (proxy_info.lpszProxyBypass)
        GlobalFree(proxy_info.lpszProxyBypass);

    return is_ok;
}

const char *proxy_resolver_win8_get_list(void *ctx) {
    proxy_resolver_win8_s *proxy_resolver = (proxy_resolver_win8_s *)ctx;
    if (!proxy_resolver)
        return NULL;
    return proxy_resolver->list;
}

int32_t proxy_resolver_win8_get_error(void *ctx) {
    proxy_resolver_win8_s *proxy_resolver = (proxy_resolver_win8_s *)ctx;
    return proxy_resolver->error;
}

bool proxy_resolver_win8_wait(void *ctx, int32_t timeout_ms) {
    proxy_resolver_win8_s *proxy_resolver = (proxy_resolver_win8_s *)ctx;
    if (!proxy_resolver)
        return false;
    return event_wait(proxy_resolver->complete, timeout_ms);
}

bool proxy_resolver_win8_cancel(void *ctx) {
    proxy_resolver_win8_s *proxy_resolver = (proxy_resolver_win8_s *)ctx;
    if (!proxy_resolver)
        return false;
    if (proxy_resolver->resolver) {
        WinHttpCloseHandle(proxy_resolver->resolver);
        proxy_resolver->resolver = NULL;
    }
    return true;
}

void *proxy_resolver_win8_create(void) {
    proxy_resolver_win8_s *proxy_resolver = (proxy_resolver_win8_s *)calloc(1, sizeof(proxy_resolver_win8_s));
    if (!proxy_resolver)
        return NULL;
    proxy_resolver->complete = event_create();
    if (!proxy_resolver->complete) {
        free(proxy_resolver);
        return NULL;
    }
    return proxy_resolver;
}

bool proxy_resolver_win8_delete(void **ctx) {
    if (!ctx)
        return false;
    proxy_resolver_win8_s *proxy_resolver = (proxy_resolver_win8_s *)*ctx;
    if (!proxy_resolver)
        return false;
    proxy_resolver_win8_cancel(ctx);
    event_delete(&proxy_resolver->complete);
    free(proxy_resolver->list);
    free(proxy_resolver);
    return true;
}

bool proxy_resolver_win8_global_init(void) {
    // Dynamically load WinHTTP and CreateProxyResolver which is only avaialble on Windows 8 or higher
    g_proxy_resolver_win8.win_http = LoadLibraryExA("winhttp.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!g_proxy_resolver_win8.win_http)
        goto win8_init_error;

    g_proxy_resolver_win8.winhttp_create_proxy_resolver =
        (LPWINHTTPCREATEPROXYRESOLVER)GetProcAddress(g_proxy_resolver_win8.win_http, "WinHttpCreateProxyResolver");
    if (!g_proxy_resolver_win8.winhttp_create_proxy_resolver)
        goto win8_init_error;
    g_proxy_resolver_win8.winhttp_get_proxy_for_url_ex =
        (LPWINHTTPGETPROXYFORURLEX)GetProcAddress(g_proxy_resolver_win8.win_http, "WinHttpGetProxyForUrlEx");
    if (!g_proxy_resolver_win8.winhttp_get_proxy_for_url_ex)
        goto win8_init_error;
    g_proxy_resolver_win8.winhttp_get_proxy_result =
        (LPWINHTTPGETPROXYRESULT)GetProcAddress(g_proxy_resolver_win8.win_http, "WinHttpGetProxyResult");
    if (!g_proxy_resolver_win8.winhttp_get_proxy_result)
        goto win8_init_error;
    g_proxy_resolver_win8.winhttp_free_proxy_result =
        (LPWINHTTPFREEPROXYRESULT)GetProcAddress(g_proxy_resolver_win8.win_http, "WinHttpFreeProxyResult");
    if (!g_proxy_resolver_win8.winhttp_free_proxy_result)
        goto win8_init_error;

    if (g_proxy_resolver_win8.winhttp_create_proxy_resolver) {
        g_proxy_resolver_win8.session = WinHttpOpen(L"cproxyres", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                                    WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC);
    }

    if (!g_proxy_resolver_win8.session) {
    win8_init_error:
        if (g_proxy_resolver_win8.win_http)
            FreeLibrary(g_proxy_resolver_win8.win_http);
        g_proxy_resolver_win8.win_http = NULL;
        return false;
    }

    return true;
}

bool proxy_resolver_win8_global_cleanup(void) {
    if (g_proxy_resolver_win8.session)
        WinHttpCloseHandle(g_proxy_resolver_win8.session);
    if (g_proxy_resolver_win8.win_http)
        FreeLibrary(g_proxy_resolver_win8.win_http);

    memset(&g_proxy_resolver_win8, 0, sizeof(g_proxy_resolver_win8));
    return true;
}

const proxy_resolver_i_s *proxy_resolver_win8_get_interface(void) {
    static const proxy_resolver_i_s proxy_resolver_win8_i = {
        proxy_resolver_win8_get_proxies_for_url,
        proxy_resolver_win8_get_list,
        proxy_resolver_win8_get_error,
        proxy_resolver_win8_wait,
        proxy_resolver_win8_cancel,
        proxy_resolver_win8_create,
        proxy_resolver_win8_delete,
        true,   // get_proxies_for_url is handled asynchronously
        false,  // get_proxies_for_url does not take into account system config
        proxy_resolver_win8_global_init,
        proxy_resolver_win8_global_cleanup};
    return &proxy_resolver_win8_i;
}
