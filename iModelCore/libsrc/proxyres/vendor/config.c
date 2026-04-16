#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "config_i.h"
#include "config_env.h"
#if defined(__APPLE__)
#  include "config_mac.h"
#elif defined(__linux__)
#  ifdef HAVE_GCONF
#    include "config_gnome2.h"
#  endif
#  include "config_gnome3.h"
#  include "config_kde.h"
#elif defined(_WIN32)
#  include "config_win.h"
#endif
#include "log.h"
#include "util_linux.h"

typedef struct g_proxy_config_s {
    // Library reference count
    int32_t ref_count;
    // Proxy config interface
    proxy_config_i_s *proxy_config_i;
    // Overrides
    bool auto_discover_disable;
    char *auto_config_url;
    char *proxy;
    char *bypass_list;
} g_proxy_config_s;

g_proxy_config_s g_proxy_config;

bool proxy_config_get_auto_discover(void) {
    if (g_proxy_config.auto_discover_disable)
        return false;
    if (g_proxy_config.proxy_config_i)
        return g_proxy_config.proxy_config_i->auto_discover();
    return false;
}

char *proxy_config_get_auto_config_url(void) {
    if (g_proxy_config.auto_config_url)
        return strdup(g_proxy_config.auto_config_url);
    if (!g_proxy_config.proxy_config_i)
        return NULL;
    return g_proxy_config.proxy_config_i->get_auto_config_url();
}

char *proxy_config_get_proxy(const char *scheme) {
    if (g_proxy_config.proxy)
        return strdup(g_proxy_config.proxy);
    if (!g_proxy_config.proxy_config_i)
        return NULL;
    return g_proxy_config.proxy_config_i->get_proxy(scheme);
}

char *proxy_config_get_bypass_list(void) {
    if (g_proxy_config.bypass_list)
        return strdup(g_proxy_config.bypass_list);
    if (!g_proxy_config.proxy_config_i)
        return NULL;
    return g_proxy_config.proxy_config_i->get_bypass_list();
}

void proxy_config_set_auto_config_url_override(const char *auto_config_url) {
    if (g_proxy_config.auto_config_url)
        free(g_proxy_config.auto_config_url);
    g_proxy_config.auto_config_url = auto_config_url ? strdup(auto_config_url) : NULL;
    g_proxy_config.auto_discover_disable = auto_config_url != NULL;
}

void proxy_config_set_proxy_override(const char *proxy) {
    if (g_proxy_config.proxy)
        free(g_proxy_config.proxy);
    g_proxy_config.proxy = proxy ? strdup(proxy) : NULL;
    g_proxy_config.auto_discover_disable = proxy != NULL;
}

void proxy_config_set_bypass_list_override(const char *bypass_list) {
    free(g_proxy_config.bypass_list);
    g_proxy_config.bypass_list = bypass_list ? strdup(bypass_list) : NULL;
}

bool proxy_config_global_init(void) {
    if (g_proxy_config.ref_count > 0) {
        g_proxy_config.ref_count++;
        return true;
    }
    memset(&g_proxy_config, 0, sizeof(g_proxy_config));
#if defined(__APPLE__)
    if (proxy_config_mac_global_init())
        g_proxy_config.proxy_config_i = proxy_config_mac_get_interface();
#elif defined(__linux__)
    int32_t desktop_env = get_desktop_env();

    switch (desktop_env) {
    case DESKTOP_ENV_GNOME3:
        if (proxy_config_gnome3_global_init())
            g_proxy_config.proxy_config_i = proxy_config_gnome3_get_interface();
        break;
#  ifdef HAVE_GCONF
    case DESKTOP_ENV_GNOME2:
        if (proxy_config_gnome2_global_init())
            g_proxy_config.proxy_config_i = proxy_config_gnome2_get_interface();
        break;
#  endif
    case DESKTOP_ENV_KDE5:
    case DESKTOP_ENV_KDE4:
    case DESKTOP_ENV_KDE3:
        if (proxy_config_kde_global_init())
            g_proxy_config.proxy_config_i = proxy_config_kde_get_interface();
        break;
    }

    if (!g_proxy_config.proxy_config_i)
        g_proxy_config.proxy_config_i = proxy_config_env_get_interface();
#elif defined(_WIN32)
    if (proxy_config_win_global_init())
        g_proxy_config.proxy_config_i = proxy_config_win_get_interface();
#endif
    if (!g_proxy_config.proxy_config_i) {
        log_error("No config interface found");
        return false;
    }
    g_proxy_config.ref_count++;
    return true;
}

bool proxy_config_global_cleanup(void) {
    if (--g_proxy_config.ref_count > 0)
        return true;

    free(g_proxy_config.auto_config_url);
    free(g_proxy_config.proxy);
    free(g_proxy_config.bypass_list);

    if (g_proxy_config.proxy_config_i)
        g_proxy_config.proxy_config_i->global_cleanup();

    memset(&g_proxy_config, 0, sizeof(g_proxy_config));
    return false;
}
