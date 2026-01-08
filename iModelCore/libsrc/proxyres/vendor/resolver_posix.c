#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <errno.h>
#include <time.h>

#include "config.h"
#include "event.h"
#include "fetch.h"
#include "log.h"
#include "execute.h"
#include "mutex.h"
#include "net_adapter.h"
#include "resolver.h"
#include "resolver_i.h"
#include "resolver_posix.h"
#include "threadpool.h"
#include "util.h"
#include "wpad_dhcp.h"
#include "wpad_dns.h"

#define WPAD_DHCP_TIMEOUT   (3)
#define WPAD_EXPIRE_SECONDS (300)

typedef struct g_proxy_resolver_posix_s {
    // WPAD discovered url
    char *auto_config_url;
    // WPAD discovery lock
    void *mutex;
    // PAC script
    char *script;
    time_t last_wpad_time;
    time_t last_fetch_time;
} g_proxy_resolver_posix_s;

g_proxy_resolver_posix_s g_proxy_resolver_posix;

typedef struct proxy_resolver_posix_s {
    // Last system error
    int32_t error;
    // Complete event
    void *complete;
    // Proxy list
    char *list;
} proxy_resolver_posix_s;

static char *proxy_resolver_posix_wpad_discover(void) {
    char *auto_config_url = NULL;
    char *script = NULL;

    // Check if we need to re-discover the WPAD auto config url
    if (g_proxy_resolver_posix.last_wpad_time > 0 &&
        g_proxy_resolver_posix.last_wpad_time + WPAD_EXPIRE_SECONDS >= time(NULL)) {
        // Use cached version of WPAD auto config url
        auto_config_url = g_proxy_resolver_posix.auto_config_url;
    } else {
        free(g_proxy_resolver_posix.auto_config_url);
        g_proxy_resolver_posix.auto_config_url = NULL;

        // Detect proxy auto configuration using DHCP
        log_info("Discovering proxy auto config using WPAD (%s)", "DHCP");
        auto_config_url = wpad_dhcp(WPAD_DHCP_TIMEOUT);

        // Detect proxy auto configuration using DNS
        if (!auto_config_url) {
            log_info("Discovering proxy auto config using WPAD (%s)", "DNS");
            script = wpad_dns(NULL);
            if (script) {
                g_proxy_resolver_posix.script = script;
                g_proxy_resolver_posix.last_fetch_time = time(NULL);
            }
        }

        g_proxy_resolver_posix.auto_config_url = auto_config_url;
        g_proxy_resolver_posix.last_wpad_time = time(NULL);
    }

    // Duplicate so it can be freed the same if proxy_config_get_auto_config_url() returns a string
    return auto_config_url ? strdup(auto_config_url) : NULL;
}

static char *proxy_resolver_posix_fetch_pac(const char *auto_config_url, int32_t *error) {
    char *script = NULL;

    // Check if the auto config url has changed
    bool url_changed = false;
    if (g_proxy_resolver_posix.auto_config_url)
        url_changed = strcmp(g_proxy_resolver_posix.auto_config_url, auto_config_url) != 0;
    else
        url_changed = true;

    // Check if we need to re-fetch the PAC script
    if (g_proxy_resolver_posix.last_fetch_time > 0 &&
        g_proxy_resolver_posix.last_fetch_time + WPAD_EXPIRE_SECONDS >= time(NULL) && !url_changed) {
        // Use cached version of the PAC script
        script = g_proxy_resolver_posix.script;
    } else {
        log_info("Fetching proxy auto config script from %s", auto_config_url);

        script = fetch_get(auto_config_url, error);
        if (!script)
            log_error("Unable to fetch proxy auto config script %s (%" PRId32 ")", auto_config_url, *error);

        free(g_proxy_resolver_posix.script);
        g_proxy_resolver_posix.script = script;
        g_proxy_resolver_posix.last_fetch_time = time(NULL);
    }

    return script;
}

bool proxy_resolver_posix_get_proxies_for_url(void *ctx, const char *url) {
    proxy_resolver_posix_s *proxy_resolver = (proxy_resolver_posix_s *)ctx;
    void *proxy_execute = NULL;
    char *auto_config_url = NULL;
    char *proxy = NULL;
    char *script = NULL;
    bool locked = false;
    bool is_ok = false;

    if (proxy_config_get_auto_discover()) {
        locked = mutex_lock(g_proxy_resolver_posix.mutex);
        // Discover the proxy auto config url
        auto_config_url = proxy_resolver_posix_wpad_discover();
    }

    // Use manually specified proxy auto configuration
    if (!auto_config_url)
        auto_config_url = proxy_config_get_auto_config_url();

    if (auto_config_url) {
        // Download proxy auto config script if available
        script = proxy_resolver_posix_fetch_pac(auto_config_url, &proxy_resolver->error);
        locked = locked && !mutex_unlock(g_proxy_resolver_posix.mutex);

        if (!script)
            goto posix_done;

        // Execute blocking proxy auto config script for url
        proxy_execute = proxy_execute_create();
        if (!proxy_execute) {
            proxy_resolver->error = ENOMEM;
            log_error("Unable to allocate memory for %s (%" PRId32 ")", "execute object", proxy_resolver->error);
            goto posix_done;
        }

        if (!proxy_execute_get_proxies_for_url(proxy_execute, g_proxy_resolver_posix.script, url)) {
            proxy_resolver->error = proxy_execute_get_error(proxy_execute);
            log_error("Unable to get proxies for url (%" PRId32 ")", proxy_resolver->error);
            goto posix_done;
        }

        // Get return value from FindProxyForURL and convert to uri list. We use default http
        // scheme if PROXY is returned.
        const char *list = proxy_execute_get_list(proxy_execute);
        proxy_resolver->list = convert_proxy_list_to_uri_list(list, "http");
    } else {
        // Use DIRECT connection since WPAD didn't result in a proxy auto-configuration url
        proxy_resolver->list = strdup("direct://");
    }

posix_done:

    if (proxy_execute)
        proxy_execute_delete(&proxy_execute);
    if (locked)
        mutex_unlock(g_proxy_resolver_posix.mutex);

    is_ok = proxy_resolver->list != NULL;
    event_set(proxy_resolver->complete);

    free(proxy);
    free(auto_config_url);

    return is_ok;
}

const char *proxy_resolver_posix_get_list(void *ctx) {
    proxy_resolver_posix_s *proxy_resolver = (proxy_resolver_posix_s *)ctx;
    if (!proxy_resolver)
        return NULL;
    return proxy_resolver->list;
}

int32_t proxy_resolver_posix_get_error(void *ctx) {
    proxy_resolver_posix_s *proxy_resolver = (proxy_resolver_posix_s *)ctx;
    return proxy_resolver->error;
}

bool proxy_resolver_posix_wait(void *ctx, int32_t timeout_ms) {
    proxy_resolver_posix_s *proxy_resolver = (proxy_resolver_posix_s *)ctx;
    if (!proxy_resolver)
        return false;
    return event_wait(proxy_resolver->complete, timeout_ms);
}

bool proxy_resolver_posix_cancel(void *ctx) {
    UNUSED(ctx);
    return false;
}

void *proxy_resolver_posix_create(void) {
    proxy_resolver_posix_s *proxy_resolver = (proxy_resolver_posix_s *)calloc(1, sizeof(proxy_resolver_posix_s));
    if (!proxy_resolver)
        return NULL;
    proxy_resolver->complete = event_create();
    if (!proxy_resolver->complete) {
        free(proxy_resolver);
        return NULL;
    }
    return proxy_resolver;
}

bool proxy_resolver_posix_delete(void **ctx) {
    if (!ctx)
        return false;
    proxy_resolver_posix_s *proxy_resolver = (proxy_resolver_posix_s *)*ctx;
    if (!proxy_resolver)
        return false;
    proxy_resolver_cancel(ctx);
    event_delete(&proxy_resolver->complete);
    free(proxy_resolver->list);
    free(proxy_resolver);
    return true;
}

static void proxy_resolver_posix_wpad_startup(void *arg) {
    UNUSED(arg);

    mutex_lock(g_proxy_resolver_posix.mutex);

    // Discover the proxy auto config url
    char *auto_config_url = proxy_resolver_posix_wpad_discover();
    if (auto_config_url) {
        int32_t error = 0;

        // Download proxy auto config script if available
        proxy_resolver_posix_fetch_pac(auto_config_url, &error);
        free(auto_config_url);
    }

    mutex_unlock(g_proxy_resolver_posix.mutex);
}

bool proxy_resolver_posix_global_init(void) {
    return proxy_resolver_posix_init_ex(NULL);
}

bool proxy_resolver_posix_init_ex(void *threadpool) {
    g_proxy_resolver_posix.mutex = mutex_create();
    if (!g_proxy_resolver_posix.mutex)
        return false;

    if (!fetch_global_init())
        return proxy_resolver_posix_global_cleanup();

    // Start WPAD discovery process immediately
    if (threadpool && proxy_config_get_auto_discover())
        threadpool_enqueue(threadpool, NULL, proxy_resolver_posix_wpad_startup);

    return true;
}

bool proxy_resolver_posix_global_cleanup(void) {
    free(g_proxy_resolver_posix.script);
    free(g_proxy_resolver_posix.auto_config_url);
    mutex_delete(&g_proxy_resolver_posix.mutex);

    fetch_global_cleanup();

    memset(&g_proxy_resolver_posix, 0, sizeof(g_proxy_resolver_posix));
    return true;
}

const proxy_resolver_i_s *proxy_resolver_posix_get_interface(void) {
    static const proxy_resolver_i_s proxy_resolver_posix_i = {
        proxy_resolver_posix_get_proxies_for_url,
        proxy_resolver_posix_get_list,
        proxy_resolver_posix_get_error,
        proxy_resolver_posix_wait,
        proxy_resolver_posix_cancel,
        proxy_resolver_posix_create,
        proxy_resolver_posix_delete,
        false,  // get_proxies_for_url should be spooled to another thread
        false,  // get_proxies_for_url does not take into account system config
        proxy_resolver_posix_global_init,
        proxy_resolver_posix_global_cleanup};
    return &proxy_resolver_posix_i;
}
