#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include <errno.h>
#ifdef _WIN32
#  include <windows.h>
#  include <winsock2.h>
#endif

#include "config.h"
#if defined(__linux__) || defined(HAVE_DUKTAPE)
#  include "execute.h"
#endif
#include "log.h"
#include "resolver.h"
#include "resolver_i.h"
#if defined(__APPLE__)
#  if defined(PROXYRES_EXECUTE) && defined(HAVE_DUKTAPE)
#    include "resolver_posix.h"
#  else
#    include "resolver_mac.h"
#  endif
#elif defined(__linux__)
// #  include "resolver_gnome3.h"
#  ifdef PROXYRES_EXECUTE
#    include "resolver_posix.h"
#  endif
#elif defined(_WIN32)
#  if defined(PROXYRES_EXECUTE) && defined(HAVE_DUKTAPE)
#    include "resolver_posix.h"
#  elif WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP
#    include "resolver_winxp.h"
#    include "resolver_win8.h"
#  elif WINAPI_FAMILY == WINAPI_FAMILY_PC_APP
#    include "resolver_winrt.h"
#  endif
#endif
#include "threadpool.h"
#include "util.h"

#ifdef __cplusplus
#  define delete f_delete
#endif

typedef struct g_proxy_resolver_s {
    // Library reference count
    int32_t ref_count;
    // Proxy resolver interface
    const proxy_resolver_i_s *proxy_resolver_i;
    // Thread pool
    void *threadpool;
} g_proxy_resolver_s;

g_proxy_resolver_s g_proxy_resolver;

typedef struct proxy_resolver_s {
    // Base proxy resolver instance
    void *base;
    // Async job
    char *url;
    // Proxy list from system config
    char *list;
    // Next proxy pointer
    const char *listp;
} proxy_resolver_s;

static void proxy_resolver_get_proxies_for_url_threadpool(void *arg) {
    proxy_resolver_s *proxy_resolver = (proxy_resolver_s *)arg;
    if (!proxy_resolver)
        return;
    g_proxy_resolver.proxy_resolver_i->get_proxies_for_url(proxy_resolver->base, proxy_resolver->url);
}

static bool proxy_resolver_get_proxies_for_url_from_system_config(void *ctx, const char *url) {
    proxy_resolver_s *proxy_resolver = (proxy_resolver_s *)ctx;
    char *auto_config_url = NULL;
    char *proxy = NULL;
    char *scheme = NULL;

    // Skip if auto-config url evaluation is required for proxy resolution
    auto_config_url = proxy_config_get_auto_config_url();
    if (auto_config_url)
        goto config_done;

    // Use scheme associated with the URL when determining proxy
    scheme = get_url_scheme(url, "http");
    if (!scheme) {
        log_error("Unable to allocate memory for scheme");
        goto config_done;
    }

    // Check if manually configured proxy is specified in system config
    proxy = proxy_config_get_proxy(scheme);
    if (proxy) {
        // Check if we need to bypass the proxy for the url
        char *bypass_list = proxy_config_get_bypass_list();
        bool should_bypass = should_bypass_proxy(url, bypass_list);
        if (should_bypass) {
            // Bypass the proxy for the url
            log_info("Bypassing proxy for %s (%s)", url, bypass_list ? bypass_list : "null");
            proxy_resolver->list = strdup("direct://");
        } else {
            // Construct proxy list url using scheme associated with proxy's port if available,
            // otherwise continue to use scheme associated with the url.
            const uint16_t proxy_port = get_host_port(proxy, strlen(proxy), 0);
            const char *proxy_scheme = proxy_port ? get_port_scheme(proxy_port, scheme) : scheme;

            // Use proxy from settings
            proxy_resolver->list = get_url_from_host(proxy_scheme, proxy);
        }
        free(bypass_list);
    } else if (!proxy_config_get_auto_discover()) {
        // Use DIRECT connection since proxy auto-discovery is not necessary
        proxy_resolver->list = strdup("direct://");
    }

config_done:

    free(scheme);
    free(proxy);
    free(auto_config_url);

    return proxy_resolver->list != NULL;
}

bool proxy_resolver_get_proxies_for_url(void *ctx, const char *url) {
    proxy_resolver_s *proxy_resolver = (proxy_resolver_s *)ctx;
    if (!proxy_resolver || !g_proxy_resolver.proxy_resolver_i)
        return false;

    proxy_resolver->listp = NULL;
    free(proxy_resolver->list);
    proxy_resolver->list = NULL;

    // Check if OS resolver already takes into account system configuration
    if (!g_proxy_resolver.proxy_resolver_i->uses_system_config) {
        // Check if auto-discovery is necessary
        if (proxy_resolver_get_proxies_for_url_from_system_config(ctx, url)) {
            // Use system proxy configuration if no auto-discovery mechanism is necessary
            return true;
        }
    }

    // Discover proxy auto-config asynchronously if supported, otherwise spool to thread pool
    if (g_proxy_resolver.proxy_resolver_i->is_async)
        return g_proxy_resolver.proxy_resolver_i->get_proxies_for_url(proxy_resolver->base, url);

    free(proxy_resolver->url);
    proxy_resolver->url = strdup(url);

    return threadpool_enqueue(g_proxy_resolver.threadpool, proxy_resolver,
                              proxy_resolver_get_proxies_for_url_threadpool);
}

const char *proxy_resolver_get_list(void *ctx) {
    proxy_resolver_s *proxy_resolver = (proxy_resolver_s *)ctx;
    if (!proxy_resolver || !g_proxy_resolver.proxy_resolver_i)
        return NULL;
    if (proxy_resolver->list)
        return proxy_resolver->list;
    return g_proxy_resolver.proxy_resolver_i->get_list(proxy_resolver->base);
}

char *proxy_resolver_get_next_proxy(void *ctx) {
    proxy_resolver_s *proxy_resolver = (proxy_resolver_s *)ctx;
    if (!proxy_resolver || !g_proxy_resolver.proxy_resolver_i)
        return NULL;
    if (!proxy_resolver->listp)
        return NULL;

    // Get the next proxy to connect through
    char *proxy = str_sep_dup(&proxy_resolver->listp, ",");
    if (!proxy) {
        if (proxy_resolver->list)
            proxy_resolver->listp = proxy_resolver->list;
        else
            proxy_resolver->listp = g_proxy_resolver.proxy_resolver_i->get_list(proxy_resolver->base);
    }
    return proxy;
}

int32_t proxy_resolver_get_error(void *ctx) {
    proxy_resolver_s *proxy_resolver = (proxy_resolver_s *)ctx;
    if (!proxy_resolver || !g_proxy_resolver.proxy_resolver_i)
        return -1;
    return g_proxy_resolver.proxy_resolver_i->get_error(proxy_resolver->base);
}

bool proxy_resolver_wait(void *ctx, int32_t timeout_ms) {
    proxy_resolver_s *proxy_resolver = (proxy_resolver_s *)ctx;
    if (!proxy_resolver || !g_proxy_resolver.proxy_resolver_i)
        return false;
    if (proxy_resolver->list) {
        proxy_resolver->listp = proxy_resolver->list;
        return true;
    }
    if (g_proxy_resolver.proxy_resolver_i->wait(proxy_resolver->base, timeout_ms)) {
        proxy_resolver->listp = g_proxy_resolver.proxy_resolver_i->get_list(proxy_resolver->base);
        return true;
    }
    return false;
}

bool proxy_resolver_cancel(void *ctx) {
    proxy_resolver_s *proxy_resolver = (proxy_resolver_s *)ctx;
    if (!proxy_resolver || !g_proxy_resolver.proxy_resolver_i)
        return false;
    if (proxy_resolver->list)
        return true;
    return g_proxy_resolver.proxy_resolver_i->cancel(proxy_resolver->base);
}

void *proxy_resolver_create(void) {
    if (!g_proxy_resolver.proxy_resolver_i)
        return NULL;
    proxy_resolver_s *proxy_resolver = (proxy_resolver_s *)calloc(1, sizeof(proxy_resolver_s));
    if (!proxy_resolver)
        return NULL;
    proxy_resolver->base = g_proxy_resolver.proxy_resolver_i->create();
    if (!proxy_resolver->base) {
        free(proxy_resolver);
        return NULL;
    }
    return proxy_resolver;
}

bool proxy_resolver_delete(void **ctx) {
    if (!g_proxy_resolver.proxy_resolver_i)
        return true;
    if (!ctx || !*ctx)
        return false;
    proxy_resolver_s *proxy_resolver = (proxy_resolver_s *)*ctx;
    free(proxy_resolver->url);
    free(proxy_resolver->list);
    g_proxy_resolver.proxy_resolver_i->delete(&proxy_resolver->base);
    free(proxy_resolver);
    *ctx = NULL;
    return true;
}

bool proxy_resolver_global_init(void) {
    if (g_proxy_resolver.ref_count > 0) {
        g_proxy_resolver.ref_count++;
        return true;
    }
    memset(&g_proxy_resolver, 0, sizeof(g_proxy_resolver_s));
#if defined(_WIN32) && (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
    WSADATA WsaData = {0};
    if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0) {
        log_error("Failed to initialize winsock %d", WSAGetLastError());
        return false;
    }
#endif

    if (!proxy_config_global_init())
        return false;
#if defined(__APPLE__)
#  if defined(PROXYRES_EXECUTE) && defined(HAVE_DUKTAPE)
    if (proxy_execute_global_init())
        g_proxy_resolver.proxy_resolver_i = proxy_resolver_posix_get_interface();
#  else
    if (proxy_resolver_mac_global_init())
        g_proxy_resolver.proxy_resolver_i = proxy_resolver_mac_get_interface();
#  endif
#elif defined(__linux__)
    /* Does not work for manually specified proxy auto-config urls
    if (proxy_resolver_gnome3_global_init())
        g_proxy_resolver.proxy_resolver_i = proxy_resolver_gnome3_get_interface();*/
#  ifdef PROXYRES_EXECUTE
    if (proxy_execute_global_init())
        g_proxy_resolver.proxy_resolver_i = proxy_resolver_posix_get_interface();
#  endif
#elif defined(_WIN32)
#  if defined(PROXYRES_EXECUTE) && defined(HAVE_DUKTAPE)
    if (proxy_execute_global_init())
        g_proxy_resolver.proxy_resolver_i = proxy_resolver_posix_get_interface();
#  elif WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP
    if (proxy_resolver_win8_global_init())
        g_proxy_resolver.proxy_resolver_i = proxy_resolver_win8_get_interface();
    else if (proxy_resolver_winxp_global_init())
        g_proxy_resolver.proxy_resolver_i = proxy_resolver_winxp_get_interface();
#  elif WINAPI_FAMILY == WINAPI_FAMILY_PC_APP
    if (proxy_resolver_winrt_global_init())
        g_proxy_resolver.proxy_resolver_i = proxy_resolver_winrt_get_interface();
#  endif
#endif

    if (!g_proxy_resolver.proxy_resolver_i) {
        log_error("No proxy resolver available");
        return false;
    }

    // No need to create thread pool since underlying implementation is already asynchronous
    if (g_proxy_resolver.proxy_resolver_i->is_async) {
        g_proxy_resolver.ref_count++;
        return true;
    }

    // Create thread pool to handle proxy resolution requests asynchronously
    g_proxy_resolver.threadpool = threadpool_create(THREADPOOL_DEFAULT_MIN_THREADS, THREADPOOL_DEFAULT_MAX_THREADS);
    if (!g_proxy_resolver.threadpool) {
        log_error("Failed to create thread pool");
        proxy_resolver_global_cleanup();
        return false;
    }

#if defined(__linux__) && defined(PROXYRES_EXECUTE)
    // Pass threadpool to posix resolver to immediately start wpad discovery
    if (g_proxy_resolver.proxy_resolver_i == proxy_resolver_posix_get_interface()) {
        if (!proxy_resolver_posix_init_ex(g_proxy_resolver.threadpool)) {
            log_error("Failed to initialize posix proxy resolver");
            proxy_resolver_global_cleanup();
            return false;
        }
    }
#endif

    g_proxy_resolver.ref_count++;
    return true;
}

bool proxy_resolver_global_cleanup(void) {
    if (--g_proxy_resolver.ref_count > 0)
        return true;

    if (g_proxy_resolver.threadpool)
        threadpool_delete(&g_proxy_resolver.threadpool);

    if (g_proxy_resolver.proxy_resolver_i)
        g_proxy_resolver.proxy_resolver_i->global_cleanup();

    memset(&g_proxy_resolver, 0, sizeof(g_proxy_resolver));
#ifdef HAVE_DUKTAPE
    if (!proxy_execute_global_cleanup())
        return false;
#endif
    if (!proxy_config_global_cleanup())
        return false;

#if defined(_WIN32) && (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
    WSACleanup();
#endif
    return true;
}
