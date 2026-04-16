#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
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

typedef struct g_proxy_resolver_winxp_s {
    // WinHTTP module handle
    HMODULE win_http;
    // WinHTTP session handle
    HINTERNET session;
} g_proxy_resolver_winxp_s;

g_proxy_resolver_winxp_s g_proxy_resolver_winxp;

typedef struct proxy_resolver_winxp_s {
    // WinHTTP proxy resolver handle
    HINTERNET resolver;
    // Last system error
    int32_t error;
    // Complete event
    void *complete;
    // Proxy list
    char *list;
} proxy_resolver_winxp_s;

bool proxy_resolver_winxp_get_proxies_for_url(void *ctx, const char *url) {
    proxy_resolver_winxp_s *proxy_resolver = (proxy_resolver_winxp_s *)ctx;
    WINHTTP_AUTOPROXY_OPTIONS options = {0};
    WINHTTP_PROXY_INFO proxy_info = {0};
    wchar_t *url_wide = NULL;
    wchar_t *auto_config_url_wide = NULL;
    char *auto_config_url = NULL;
    char *proxy = NULL;
    char *bypass_list = NULL;
    bool is_ok = false;

    auto_config_url = proxy_config_get_auto_config_url();
    if (auto_config_url) {
        // Use auto configuration script specified by system
        auto_config_url_wide = utf8_dup_to_wchar(auto_config_url);
        if (!auto_config_url_wide) {
            proxy_resolver->error = ERROR_OUTOFMEMORY;
            log_error("Unable to allocate memory for %s (%" PRId32 ")", "auto config url", proxy_resolver->error);
            goto winxp_done;
        }

        options.dwFlags = WINHTTP_AUTOPROXY_CONFIG_URL;
        options.lpszAutoConfigUrl = auto_config_url_wide;
    } else {
        // Use WPAD to automatically retrieve proxy auto-configuration and evaluate it
        options.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
        options.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
    }

    // Convert url to wide char for WinHttpGetProxyForUrl
    url_wide = utf8_dup_to_wchar(url);
    if (!url_wide) {
        proxy_resolver->error = ERROR_OUTOFMEMORY;
        log_error("Unable to allocate memory for %s (%" PRId32 ")", "wide char url", proxy_resolver->error);
        goto winxp_done;
    }

    // For performance reasons try fAutoLogonIfChallenged = false then try fAutoLogonIfChallenged = true
    // https://docs.microsoft.com/en-us/windows/win32/api/winhttp/ns-winhttp-winhttp_autoproxy_options

    is_ok = WinHttpGetProxyForUrl(g_proxy_resolver_winxp.session, url_wide, &options, &proxy_info);
    if (!is_ok) {
        if (GetLastError() == ERROR_WINHTTP_LOGIN_FAILURE) {
            options.fAutoLogonIfChallenged = true;
            is_ok = WinHttpGetProxyForUrl(g_proxy_resolver_winxp.session, url_wide, &options, &proxy_info);
        }
        if (!is_ok) {
            int32_t error = GetLastError();

            // Failure to detect proxy auto configuration does not necessarily indicate an error
            if (error == ERROR_WINHTTP_AUTODETECTION_FAILED) {
                proxy_info.dwAccessType = WINHTTP_ACCESS_TYPE_NO_PROXY;
            } else {
                proxy_resolver->error = error;
                goto winxp_done;
            }
        }
    }

    switch (proxy_info.dwAccessType) {
    case WINHTTP_ACCESS_TYPE_NO_PROXY:
        // Use direct connection
        proxy_resolver->list = strdup("direct://");
        break;
    case WINHTTP_ACCESS_TYPE_NAMED_PROXY:
        // Using manually configured proxy
        if (proxy_info.lpszProxy)
            proxy = wchar_dup_to_utf8(proxy_info.lpszProxy);
        if (!proxy) {
            proxy_resolver->error = ERROR_OUTOFMEMORY;
            log_error("Unable to allocate memory for %s (%" PRId32 ")", "proxy", proxy_resolver->error);
            goto winxp_done;
        }

        if (proxy_info.lpszProxyBypass)
            bypass_list = wchar_dup_to_utf8(proxy_info.lpszProxyBypass);
        if (bypass_list) {
            // Check if we need to bypass the proxy for the url
            bool should_bypass = should_bypass_proxy(url, bypass_list);
            if (should_bypass) {
                // Bypass the proxy for the url
                log_info("Bypassing proxy for %s (%s)", url, bypass_list);
                proxy_resolver->list = strdup("direct://");
            }
        }

        // Convert proxy list to uri list
        if (proxy_resolver->list)
            proxy_resolver->list = convert_winhttp_proxy_list_to_uri_list(proxy);
        if (!proxy_resolver->list) {
            proxy_resolver->error = ERROR_OUTOFMEMORY;
            log_error("Unable to allocate memory for %s (%" PRId32 ")", "proxy list", proxy_resolver->error);
            goto winxp_done;
        }
        break;
    }

winxp_done:

    is_ok = proxy_resolver->list != NULL;
    event_set(proxy_resolver->complete);

    free(bypass_list);
    free(proxy);
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

const char *proxy_resolver_winxp_get_list(void *ctx) {
    proxy_resolver_winxp_s *proxy_resolver = (proxy_resolver_winxp_s *)ctx;
    if (!proxy_resolver)
        return NULL;
    return proxy_resolver->list;
}

int32_t proxy_resolver_winxp_get_error(void *ctx) {
    proxy_resolver_winxp_s *proxy_resolver = (proxy_resolver_winxp_s *)ctx;
    return proxy_resolver->error;
}

bool proxy_resolver_winxp_wait(void *ctx, int32_t timeout_ms) {
    proxy_resolver_winxp_s *proxy_resolver = (proxy_resolver_winxp_s *)ctx;
    if (!proxy_resolver)
        return false;
    return event_wait(proxy_resolver->complete, timeout_ms);
}

bool proxy_resolver_winxp_cancel(void *ctx) {
    proxy_resolver_winxp_s *proxy_resolver = (proxy_resolver_winxp_s *)ctx;
    if (!proxy_resolver)
        return false;
    if (proxy_resolver->resolver) {
        WinHttpCloseHandle(proxy_resolver->resolver);
        proxy_resolver->resolver = NULL;
    }
    return true;
}

void *proxy_resolver_winxp_create(void) {
    proxy_resolver_winxp_s *proxy_resolver = (proxy_resolver_winxp_s *)calloc(1, sizeof(proxy_resolver_winxp_s));
    if (!proxy_resolver)
        return NULL;
    proxy_resolver->complete = event_create();
    if (!proxy_resolver->complete) {
        free(proxy_resolver);
        return NULL;
    }
    return proxy_resolver;
}

bool proxy_resolver_winxp_delete(void **ctx) {
    if (!ctx)
        return false;
    proxy_resolver_winxp_s *proxy_resolver = (proxy_resolver_winxp_s *)*ctx;
    if (!proxy_resolver)
        return false;
    proxy_resolver_winxp_cancel(ctx);
    event_delete(&proxy_resolver->complete);
    free(proxy_resolver->list);
    free(proxy_resolver);
    return true;
}

bool proxy_resolver_winxp_global_init(void) {
    g_proxy_resolver_winxp.session =
        WinHttpOpen(L"cproxyres", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

    if (!g_proxy_resolver_winxp.session)
        return false;
    return true;
}

bool proxy_resolver_winxp_global_cleanup(void) {
    if (g_proxy_resolver_winxp.session)
        WinHttpCloseHandle(g_proxy_resolver_winxp.session);

    memset(&g_proxy_resolver_winxp, 0, sizeof(g_proxy_resolver_winxp));
    return true;
}

const proxy_resolver_i_s *proxy_resolver_winxp_get_interface(void) {
    static const proxy_resolver_i_s proxy_resolver_winxp_i = {
        proxy_resolver_winxp_get_proxies_for_url,
        proxy_resolver_winxp_get_list,
        proxy_resolver_winxp_get_error,
        proxy_resolver_winxp_wait,
        proxy_resolver_winxp_cancel,
        proxy_resolver_winxp_create,
        proxy_resolver_winxp_delete,
        false,  // get_proxies_for_url should be spooled to another thread
        false,  // get_proxies_for_url does not take into account system config
        proxy_resolver_winxp_global_init,
        proxy_resolver_winxp_global_cleanup};
    return &proxy_resolver_winxp_i;
}
