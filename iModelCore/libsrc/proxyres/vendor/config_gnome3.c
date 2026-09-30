#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>

#include <dlfcn.h>
#include <glib.h>
#include <gio/gio.h>

#include "config.h"
#include "config_i.h"
#include "config_gnome3.h"
#include "util.h"

typedef struct g_proxy_config_gnome3_s {
    // GIO module handle
    void *gio_module;
    // GConf settings functions
    void (*g_object_unref)(gpointer object);
    GSettings *(*g_settings_new)(const gchar *schema_id);
    gchar *(*g_settings_get_string)(GSettings *settings, const gchar *key);
    gint (*g_settings_get_int)(GSettings *settings, const gchar *key);
    gchar **(*g_settings_get_strv)(GSettings *settings, const gchar *key);
    gboolean (*g_settings_get_boolean)(GSettings *settings, const gchar *key);
    // Glib module handle
    void *glib_module;
    // Glib memory functions
    void (*g_free)(gpointer mem);
    void (*g_strfreev)(gchar **str_array);
} g_proxy_config_gnome3_s;

g_proxy_config_gnome3_s g_proxy_config_gnome3;

static bool proxy_config_gnome3_is_mode(const char *mode) {
    bool equal = false;

    GSettings *settings = g_proxy_config_gnome3.g_settings_new("org.gnome.system.proxy");
    if (!settings)
        return false;

    char *system_mode = g_proxy_config_gnome3.g_settings_get_string(settings, "mode");
    if (system_mode) {
        equal = strcmp(system_mode, mode) == 0;
        g_proxy_config_gnome3.g_free(system_mode);
    }
    g_proxy_config_gnome3.g_object_unref(settings);
    return equal;
}

bool proxy_config_gnome3_get_auto_discover(void) {
    return proxy_config_gnome3_is_mode("auto");
}

char *proxy_config_gnome3_get_auto_config_url(void) {
    char *auto_config_url = NULL;

    if (!proxy_config_gnome3_is_mode("auto"))
        return NULL;
    GSettings *settings = g_proxy_config_gnome3.g_settings_new("org.gnome.system.proxy");
    if (!settings)
        return NULL;
    char *url = g_proxy_config_gnome3.g_settings_get_string(settings, "autoconfig-url");
    if (url) {
        if (*url)
            auto_config_url = strdup(url);
        g_proxy_config_gnome3.g_free(url);
    }
    g_proxy_config_gnome3.g_object_unref(settings);
    return auto_config_url;
}

static bool proxy_config_gnome3_use_same_proxy(void) {
    GSettings *settings = g_proxy_config_gnome3.g_settings_new("org.gnome.system.proxy");
    if (!settings)
        return false;

    bool use_same_proxy = g_proxy_config_gnome3.g_settings_get_boolean(settings, "use-same-proxy");
    g_proxy_config_gnome3.g_object_unref(settings);
    return use_same_proxy;
}

char *proxy_config_gnome3_get_proxy(const char *scheme) {
    char settings_path[128];
    char *proxy = NULL;

    if (!proxy_config_gnome3_is_mode("manual"))
        return NULL;

    if (proxy_config_gnome3_use_same_proxy())
        strncpy(settings_path, "org.gnome.system.proxy.http", sizeof(settings_path));
    else
        snprintf(settings_path, sizeof(settings_path), "org.gnome.system.proxy.%s", scheme);

    GSettings *settings = g_proxy_config_gnome3.g_settings_new(settings_path);
    if (!settings)
        return NULL;

    char *host = g_proxy_config_gnome3.g_settings_get_string(settings, "host");
    if (host && *host) {
        // Allocate space for host:port
        size_t max_proxy = strlen(host) + 32;
        proxy = (char *)malloc(max_proxy);
        if (proxy) {
            uint32_t port = g_proxy_config_gnome3.g_settings_get_int(settings, "port");
            if (port == 0)
                snprintf(proxy, max_proxy, "%s", host);
            else
                snprintf(proxy, max_proxy, "%s:%" PRIu32 "", host, port);
        }

        g_proxy_config_gnome3.g_free(host);
    }
    g_proxy_config_gnome3.g_object_unref(settings);
    return proxy;
}

char *proxy_config_gnome3_get_bypass_list(void) {
    char *bypass_list = NULL;

    if (!proxy_config_gnome3_is_mode("manual"))
        return NULL;

    GSettings *settings = g_proxy_config_gnome3.g_settings_new("org.gnome.system.proxy");
    if (!settings)
        return NULL;

    char **hosts = g_proxy_config_gnome3.g_settings_get_strv(settings, "ignore-hosts");
    if (hosts) {
        size_t max_value = 0;

        // Enumerate the list to get the size of the bypass list
        for (int32_t i = 0; hosts[i] && *(hosts[i]); i++)
            max_value += strlen(hosts[i]) + 2;

        if (max_value > 0) {
            // Allocate space for the bypass list
            bypass_list = (char *)calloc(max_value, sizeof(char));
            if (bypass_list) {
                // Enumerate hosts and copy them to the bypass list
                for (int32_t i = 0; hosts[i]; i++) {
                    size_t bypass_list_len = strlen(bypass_list);
                    snprintf(bypass_list + bypass_list_len, max_value - bypass_list_len, "%s,", hosts[i]);
                }

                // Remove the last separator
                str_trim_end(bypass_list, ',');
            }
        }

        g_proxy_config_gnome3.g_strfreev(hosts);
    }
    g_proxy_config_gnome3.g_object_unref(settings);
    return bypass_list;
}

bool proxy_config_gnome3_global_init(void) {
    g_proxy_config_gnome3.glib_module = dlopen("libglib-2.0.so.0", RTLD_LAZY | RTLD_LOCAL);
    if (!g_proxy_config_gnome3.glib_module)
        goto gnome3_init_error;
    g_proxy_config_gnome3.gio_module = dlopen("libgio-2.0.so.0", RTLD_LAZY | RTLD_LOCAL);
    if (!g_proxy_config_gnome3.gio_module)
        goto gnome3_init_error;

    // Glib functions
    g_proxy_config_gnome3.g_free = (void (*)(gpointer))dlsym(g_proxy_config_gnome3.glib_module, "g_free");
    if (!g_proxy_config_gnome3.g_free)
        goto gnome3_init_error;
    g_proxy_config_gnome3.g_strfreev = (void (*)(gchar **))dlsym(g_proxy_config_gnome3.glib_module, "g_strfreev");
    if (!g_proxy_config_gnome3.g_strfreev)
        goto gnome3_init_error;

    // GIO functions
    g_proxy_config_gnome3.g_object_unref =
        (void (*)(gpointer))dlsym(g_proxy_config_gnome3.gio_module, "g_object_unref");
    if (!g_proxy_config_gnome3.g_object_unref)
        goto gnome3_init_error;
    g_proxy_config_gnome3.g_settings_new =
        (GSettings * (*)(const gchar *)) dlsym(g_proxy_config_gnome3.gio_module, "g_settings_new");
    if (!g_proxy_config_gnome3.g_settings_new)
        goto gnome3_init_error;
    g_proxy_config_gnome3.g_settings_get_string =
        (gchar * (*)(GSettings *, const gchar *)) dlsym(g_proxy_config_gnome3.gio_module, "g_settings_get_string");
    if (!g_proxy_config_gnome3.g_settings_get_string)
        goto gnome3_init_error;
    g_proxy_config_gnome3.g_settings_get_int =
        (gint(*)(GSettings *, const gchar *))dlsym(g_proxy_config_gnome3.gio_module, "g_settings_get_int");
    if (!g_proxy_config_gnome3.g_settings_get_int)
        goto gnome3_init_error;
    g_proxy_config_gnome3.g_settings_get_strv =
        (gchar * *(*)(GSettings *, const gchar *)) dlsym(g_proxy_config_gnome3.gio_module, "g_settings_get_strv");
    if (!g_proxy_config_gnome3.g_settings_get_strv)
        goto gnome3_init_error;
    g_proxy_config_gnome3.g_settings_get_boolean =
        (gboolean(*)(GSettings *, const gchar *))dlsym(g_proxy_config_gnome3.gio_module, "g_settings_get_boolean");
    if (!g_proxy_config_gnome3.g_settings_get_boolean)
        goto gnome3_init_error;
    return true;

gnome3_init_error:
    proxy_config_gnome3_global_cleanup();
    return false;
}

bool proxy_config_gnome3_global_cleanup(void) {
    if (g_proxy_config_gnome3.gio_module)
        dlclose(g_proxy_config_gnome3.gio_module);
    if (g_proxy_config_gnome3.glib_module)
        dlclose(g_proxy_config_gnome3.glib_module);

    memset(&g_proxy_config_gnome3, 0, sizeof(g_proxy_config_gnome3));
    return true;
}

proxy_config_i_s *proxy_config_gnome3_get_interface(void) {
    static proxy_config_i_s proxy_config_gnome3_i = {
        proxy_config_gnome3_get_auto_discover, proxy_config_gnome3_get_auto_config_url,
        proxy_config_gnome3_get_proxy,         proxy_config_gnome3_get_bypass_list,
        proxy_config_gnome3_global_init,       proxy_config_gnome3_global_cleanup};
    return &proxy_config_gnome3_i;
}
