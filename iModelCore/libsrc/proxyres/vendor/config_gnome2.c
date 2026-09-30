#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include <dlfcn.h>
#include <glib.h>
#include <gconf/gconf.h>

#include "config.h"
#include "config_i.h"
#include "config_gnome2.h"
#include "util.h"

typedef struct g_proxy_config_gnome2_s {
    // GConf module handle
    void *gconf_module;
    // GConf settings functions
    GConfEngine *(*gconf_engine_get_default)(void);
    gchar *(*gconf_engine_get_string)(GConfEngine *conf, const gchar *key, GError **err);
    gint (*gconf_engine_get_int)(GConfEngine *conf, const gchar *key, GError **err);
    gboolean (*gconf_engine_get_bool)(GConfEngine *conf, const gchar *key, GError **err);
    GSList *(*gconf_engine_get_list)(GConfEngine *conf, const gchar *key, GConfValueType list_type, GError **err);
    // Default GConf instance
    GConfEngine *gconf_default;
    // Glib module handle
    void *glib_module;
    // Glib memory functions
    void (*g_free)(gpointer Mem);
    void (*g_slist_foreach)(GSList *list, GFunc func, gpointer user_data);
    void (*g_slist_free_full)(GSList *list, GDestroyNotify free_func);
} g_proxy_config_gnome2_s;

g_proxy_config_gnome2_s g_proxy_config_gnome2;

static bool proxy_config_gnome2_is_mode(const char *mode) {
    bool equal = false;

    char *system_mode =
        g_proxy_config_gnome2.gconf_engine_get_string(g_proxy_config_gnome2.gconf_default, "/system/proxy/mode", NULL);
    if (system_mode) {
        equal = strcmp(system_mode, mode) == 0;
        g_proxy_config_gnome2.g_free(system_mode);
    }
    return equal;
}

bool proxy_config_gnome2_get_auto_discover(void) {
    return proxy_config_gnome2_is_mode("auto");
}

char *proxy_config_gnome2_get_auto_config_url(void) {
    char *auto_config_url = NULL;
    char *url = NULL;

    if (!proxy_config_gnome2_is_mode("auto"))
        return NULL;

    url = g_proxy_config_gnome2.gconf_engine_get_string(g_proxy_config_gnome2.gconf_default,
                                                        "/system/proxy/autoconfig_url", NULL);
    if (url) {
        if (*url)
            auto_config_url = strdup(url);
        g_proxy_config_gnome2.g_free(url);
    }

    return auto_config_url;
}

char *proxy_config_gnome2_get_proxy(const char *scheme) {
    char host_key[128];
    char port_key[128];
    char *host = NULL;
    uint32_t port = 0;
    char *proxy = NULL;

    if (!proxy_config_gnome2_is_mode("manual"))
        return NULL;

    if (strncasecmp(scheme, "https", 5) == 0) {
        strncpy(host_key, "/system/proxy/secure_host", sizeof(host_key));
        strncpy(port_key, "/system/proxy/secure_port", sizeof(port_key));
    } else if (strncasecmp(scheme, "http", 4) == 0) {
        strncpy(host_key, "/system/http_proxy/host", sizeof(host_key));
        strncpy(port_key, "/system/http_proxy/port", sizeof(port_key));
    } else {
        snprintf(host_key, sizeof(host_key), "/system/proxy/%s_host", scheme);
        snprintf(port_key, sizeof(port_key), "/system/proxy/%s_port", scheme);
    }

    host = g_proxy_config_gnome2.gconf_engine_get_string(g_proxy_config_gnome2.gconf_default, host_key, NULL);
    if (host && *host) {
        // Allocate space for host:port
        size_t max_proxy = strlen(host) + 32;
        proxy = (char *)malloc(max_proxy);
        if (proxy) {
            port = g_proxy_config_gnome2.gconf_engine_get_int(g_proxy_config_gnome2.gconf_default, port_key, NULL);
            if (port == 0)
                snprintf(proxy, max_proxy, "%s", host);
            else
                snprintf(proxy, max_proxy, "%s:%" PRIu32 "", host, port);
        }

        g_proxy_config_gnome2.g_free(host);
    }
    return proxy;
}

typedef struct g_slist_for_each_bypass_s {
    char *value;
    size_t max_value;
} g_slist_for_each_bypass_s;

static void gs_list_for_each_func(gpointer data, gpointer user_data) {
    g_slist_for_each_bypass_s *bypass = (g_slist_for_each_bypass_s *)user_data;
    char *rule = (char *)data;

    // Ignore empty rules
    if (!rule || *rule == 0)
        return;

    if (!bypass->value) {
        // Calculate the size of the string
        bypass->max_value += strlen(rule) + 2;
    } else {
        // Append the rule to the string
        size_t bypass_len = strlen(bypass->value);
        snprintf(bypass->value + bypass_len, bypass->max_value - bypass_len, "%s,", rule);
    }
}

char *proxy_config_gnome2_get_bypass_list(void) {
    GSList *hosts = NULL;
    g_slist_for_each_bypass_s enum_bypass = {0};
    char *bypass_list = NULL;

    if (!proxy_config_gnome2_is_mode("manual"))
        return NULL;

    hosts = g_proxy_config_gnome2.gconf_engine_get_list(g_proxy_config_gnome2.gconf_default,
                                                        "/system/http_proxy/ignore_hosts", GCONF_VALUE_STRING, NULL);
    if (hosts) {
        // Enumerate the list to get the size of the bypass list
        g_proxy_config_gnome2.g_slist_foreach(hosts, gs_list_for_each_func, &enum_bypass);
        if (enum_bypass.max_value > 0) {
            enum_bypass.max_value++;

            // Allocate space for the bypass list
            bypass_list = (char *)calloc(enum_bypass.max_value, sizeof(char));
            if (bypass_list) {
                enum_bypass.value = bypass_list;

                // Enumerate the list to get the bypass list string
                g_proxy_config_gnome2.g_slist_foreach(hosts, gs_list_for_each_func, &enum_bypass);

                // Remove the last separator
                str_trim_end(bypass_list, ',');
            }
        }

        g_proxy_config_gnome2.g_slist_free_full(hosts, g_proxy_config_gnome2.g_free);
    }

    return bypass_list;
}

bool proxy_config_gnome2_global_init(void) {
    g_proxy_config_gnome2.glib_module = dlopen("libglib-2.0.so.0", RTLD_LAZY | RTLD_LOCAL);
    if (!g_proxy_config_gnome2.glib_module)
        goto gnome2_init_error;
    g_proxy_config_gnome2.gconf_module = dlopen("libgconf-2.so.4", RTLD_LAZY | RTLD_LOCAL);
    if (!g_proxy_config_gnome2.gconf_module)
        goto gnome2_init_error;

    // Glib functions
    g_proxy_config_gnome2.g_free = (void (*)(gpointer))dlsym(g_proxy_config_gnome2.glib_module, "g_free");
    if (!g_proxy_config_gnome2.g_free)
        goto gnome2_init_error;
    g_proxy_config_gnome2.g_slist_free_full =
        (void (*)(GSList *, GDestroyNotify))dlsym(g_proxy_config_gnome2.glib_module, "g_slist_free_full");
    if (!g_proxy_config_gnome2.g_slist_free_full)
        goto gnome2_init_error;
    g_proxy_config_gnome2.g_slist_foreach =
        (void (*)(GSList *, GFunc, gpointer))dlsym(g_proxy_config_gnome2.glib_module, "g_slist_foreach");
    if (!g_proxy_config_gnome2.g_slist_foreach)
        goto gnome2_init_error;

    // Gconf functions
    g_proxy_config_gnome2.gconf_engine_get_default =
        (GConfEngine * (*)()) dlsym(g_proxy_config_gnome2.gconf_module, "gconf_engine_get_default");
    if (!g_proxy_config_gnome2.gconf_engine_get_default)
        goto gnome2_init_error;
    g_proxy_config_gnome2.gconf_engine_get_string = (gchar * (*)(GConfEngine *, const gchar *, GError **))
        dlsym(g_proxy_config_gnome2.gconf_module, "gconf_engine_get_string");
    if (!g_proxy_config_gnome2.gconf_engine_get_string)
        goto gnome2_init_error;
    g_proxy_config_gnome2.gconf_engine_get_int = (gint(*)(GConfEngine *, const gchar *, GError **))dlsym(
        g_proxy_config_gnome2.gconf_module, "gconf_engine_get_int");
    if (!g_proxy_config_gnome2.gconf_engine_get_int)
        goto gnome2_init_error;
    g_proxy_config_gnome2.gconf_engine_get_bool = (gboolean(*)(GConfEngine *, const gchar *, GError **))dlsym(
        g_proxy_config_gnome2.gconf_module, "gconf_engine_get_bool");
    if (!g_proxy_config_gnome2.gconf_engine_get_bool)
        goto gnome2_init_error;
    g_proxy_config_gnome2.gconf_engine_get_list =
        (GSList * (*)(GConfEngine *, const gchar *, GConfValueType, GError **))
            dlsym(g_proxy_config_gnome2.gconf_module, "gconf_engine_get_list");
    if (!g_proxy_config_gnome2.gconf_engine_get_list)
        goto gnome2_init_error;

    // Get default config instance
    g_proxy_config_gnome2.gconf_default = g_proxy_config_gnome2.gconf_engine_get_default();
    if (!g_proxy_config_gnome2.gconf_default)
        goto gnome2_init_error;
    return true;

gnome2_init_error:
    proxy_config_gnome2_global_cleanup();
    return false;
}

bool proxy_config_gnome2_global_cleanup(void) {
    if (g_proxy_config_gnome2.glib_module)
        dlclose(g_proxy_config_gnome2.glib_module);
    if (g_proxy_config_gnome2.gconf_module)
        dlclose(g_proxy_config_gnome2.gconf_module);

    memset(&g_proxy_config_gnome2, 0, sizeof(g_proxy_config_gnome2));
    return true;
}

proxy_config_i_s *proxy_config_gnome2_get_interface(void) {
    static proxy_config_i_s proxy_config_gnome2_i = {
        proxy_config_gnome2_get_auto_discover, proxy_config_gnome2_get_auto_config_url,
        proxy_config_gnome2_get_proxy,         proxy_config_gnome2_get_bypass_list,
        proxy_config_gnome2_global_init,       proxy_config_gnome2_global_cleanup};
    return &proxy_config_gnome2_i;
}
