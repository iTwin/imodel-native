
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include <errno.h>
#include <dlfcn.h>
#include <gio/gio.h>

#include "event.h"
#include "log.h"
#include "resolver.h"
#include "resolver_i.h"
#include "resolver_gnome3.h"

typedef struct g_proxy_resolver_gnome3_s {
    // GIO module handle
    void *gio_module;
    // GIO cancellable functions
    void (*g_object_unref)(gpointer object);
    GCancellable *(*g_cancellable_new)(void);
    void (*g_cancellable_cancel)(GCancellable *cancellable);
    // GProxy resolution functions
    GProxyResolver *(*g_proxy_resolver_get_default)(void);
    gchar **(*g_proxy_resolver_lookup)(GProxyResolver *resolver, const gchar *uri, GCancellable *cancellable,
                                       GError **error);
    // Glib module handle
    void *glib_module;
    // Glib functions
    guint (*g_strv_length)(gchar **str_array);
    void (*g_strfreev)(gchar **str_array);
    void (*g_error_free)(GError *error);
} g_proxy_resolver_gnome3_s;

g_proxy_resolver_gnome3_s g_proxy_resolver_gnome3;

typedef struct proxy_resolver_gnome3_s {
    // GProxy resolver
    GProxyResolver *resolver;
    // Last system error
    int32_t error;
    // Complete event
    void *complete;
    // Cancellable object
    GCancellable *cancellable;
    // Proxy list
    char *list;
} proxy_resolver_gnome3_s;

static void proxy_resolver_gnome3_delete_resolver(proxy_resolver_gnome3_s *proxy_resolver) {
    if (proxy_resolver->cancellable) {
        g_proxy_resolver_gnome3.g_object_unref(proxy_resolver->cancellable);
        proxy_resolver->cancellable = NULL;
    }

    /* Don't unref default resolver
    if (proxy_resolver->resolver) {
        g_proxy_resolver_gnome3.g_object_unref(proxy_resolver->resolver);
        proxy_resolver->resolver = NULL;
    }*/
}

static bool proxy_resolver_gnome3_create_resolver(proxy_resolver_gnome3_s *proxy_resolver) {
    // Get reference to the default proxy resolver
    proxy_resolver->resolver = g_proxy_resolver_gnome3.g_proxy_resolver_get_default();
    if (!proxy_resolver->resolver) {
        proxy_resolver->error = ENOMEM;
        log_error("Unable to allocate memory for %s (%" PRId32 ")", "resolver object", proxy_resolver->error);
        return false;
    }

    // Create cancellable object in case we need to cancel operation
    proxy_resolver->cancellable = g_proxy_resolver_gnome3.g_cancellable_new();
    if (!proxy_resolver->cancellable) {
        proxy_resolver_gnome3_delete_resolver(proxy_resolver);
        proxy_resolver->error = ENOMEM;
        log_error("Unable to allocate memory for %s (%" PRId32 ")", "cancellable object", proxy_resolver->error);
        return false;
    }

    return true;
}

static bool proxy_resolver_gnome3_get_proxies(proxy_resolver_gnome3_s *proxy_resolver, char **proxies, GError *error) {
    guint proxy_count = g_proxy_resolver_gnome3.g_strv_length(proxies);
    size_t max_list = (proxy_count + 1) * MAX_PROXY_URL;
    int32_t list_len = 0;

    if (!proxies) {
        proxy_resolver->error = error->code;
        log_error("Unable to get proxies for list (%" PRId32 ":%s)", proxy_resolver->error, error->message);
        return false;
    }

    // Allocate memory for proxy list
    proxy_resolver->list = (char *)calloc(max_list, sizeof(char));
    if (!proxy_resolver->list) {
        proxy_resolver->error = ENOMEM;
        log_error("Unable to allocate memory for %s (%" PRId32 ")", "list", proxy_resolver->error);
        return false;
    }

    for (guint i = 0; proxies[i] && i < proxy_count; i++) {
        // Copy string since it is already in the format "scheme://host:port"
        strncat(proxy_resolver->list, proxies[i], max_list - list_len - 1);
        list_len += strlen(proxies[i]);

        if (i != proxy_count - 1) {
            // Separate each proxy with a comma
            strncat(proxy_resolver->list, ",", max_list - list_len - 1);
            list_len++;
        }
    }

    return true;
}

bool proxy_resolver_gnome3_get_proxies_for_url(void *ctx, const char *url) {
    proxy_resolver_gnome3_s *proxy_resolver = (proxy_resolver_gnome3_s *)ctx;
    GError *error = NULL;
    char **proxies = NULL;
    bool is_ok = false;

    if (proxy_resolver_gnome3_create_resolver(proxy_resolver)) {
        // Get list of proxies from resolver
        proxies = g_proxy_resolver_gnome3.g_proxy_resolver_lookup(proxy_resolver->resolver, url,
                                                                  proxy_resolver->cancellable, &error);
        proxy_resolver_gnome3_get_proxies(proxy_resolver, proxies, error);

        if (error) {
            proxy_resolver->error = error->code;
            log_error("%s (%" PRId32 ")", error->message, error->code);
            g_proxy_resolver_gnome3.g_error_free(error);
        } else {
            is_ok = true;
        }

        if (proxies)
            g_proxy_resolver_gnome3.g_strfreev(proxies);
    }

    proxy_resolver_gnome3_delete_resolver(proxy_resolver);

    event_set(proxy_resolver->complete);

    return is_ok;
}

const char *proxy_resolver_gnome3_get_list(void *ctx) {
    proxy_resolver_gnome3_s *proxy_resolver = (proxy_resolver_gnome3_s *)ctx;
    if (!proxy_resolver)
        return NULL;
    return proxy_resolver->list;
}

int32_t proxy_resolver_gnome3_get_error(void *ctx) {
    proxy_resolver_gnome3_s *proxy_resolver = (proxy_resolver_gnome3_s *)ctx;
    return proxy_resolver->error;
}

bool proxy_resolver_gnome3_wait(void *ctx, int32_t timeout_ms) {
    proxy_resolver_gnome3_s *proxy_resolver = (proxy_resolver_gnome3_s *)ctx;
    if (!proxy_resolver)
        return false;
    return event_wait(proxy_resolver->complete, timeout_ms);
}

bool proxy_resolver_gnome3_cancel(void *ctx) {
    proxy_resolver_gnome3_s *proxy_resolver = (proxy_resolver_gnome3_s *)ctx;
    if (!proxy_resolver)
        return false;

    // Cancel request to the proxy resolver
    if (proxy_resolver->cancellable)
        g_proxy_resolver_gnome3.g_cancellable_cancel(proxy_resolver->cancellable);
    return true;
}

void *proxy_resolver_gnome3_create(void) {
    proxy_resolver_gnome3_s *proxy_resolver = (proxy_resolver_gnome3_s *)calloc(1, sizeof(proxy_resolver_gnome3_s));
    if (!proxy_resolver)
        return NULL;
    proxy_resolver->complete = event_create();
    if (!proxy_resolver->complete) {
        free(proxy_resolver);
        return NULL;
    }
    return proxy_resolver;
}

bool proxy_resolver_gnome3_delete(void **ctx) {
    if (!ctx)
        return false;
    proxy_resolver_gnome3_s *proxy_resolver = (proxy_resolver_gnome3_s *)*ctx;
    if (!proxy_resolver)
        return false;
    proxy_resolver_cancel(ctx);
    event_delete(&proxy_resolver->complete);
    free(proxy_resolver->list);
    free(proxy_resolver);
    return true;
}

bool proxy_resolver_gnome3_global_init(void) {
    g_proxy_resolver_gnome3.gio_module = dlopen("libgio-2.0.so.0", RTLD_LAZY | RTLD_LOCAL);
    if (!g_proxy_resolver_gnome3.gio_module)
        goto gnome3_init_error;
    g_proxy_resolver_gnome3.glib_module = dlopen("libglib-2.0.so.0", RTLD_LAZY | RTLD_LOCAL);
    if (!g_proxy_resolver_gnome3.glib_module)
        goto gnome3_init_error;

    // Glib functions
    g_proxy_resolver_gnome3.g_error_free =
        (void (*)(GError *))dlsym(g_proxy_resolver_gnome3.glib_module, "g_error_free");
    if (!g_proxy_resolver_gnome3.g_error_free)
        goto gnome3_init_error;
    g_proxy_resolver_gnome3.g_strv_length =
        (guint(*)(gchar **))dlsym(g_proxy_resolver_gnome3.glib_module, "g_strv_length");
    if (!g_proxy_resolver_gnome3.g_strv_length)
        goto gnome3_init_error;
    g_proxy_resolver_gnome3.g_strfreev = (void (*)(gchar **))dlsym(g_proxy_resolver_gnome3.glib_module, "g_strfreev");
    if (!g_proxy_resolver_gnome3.g_strfreev)
        goto gnome3_init_error;

    // GIO cancellable functions
    g_proxy_resolver_gnome3.g_object_unref =
        (void (*)(gpointer))dlsym(g_proxy_resolver_gnome3.gio_module, "g_object_unref");
    if (!g_proxy_resolver_gnome3.g_object_unref)
        goto gnome3_init_error;
    g_proxy_resolver_gnome3.g_cancellable_new =
        (GCancellable * (*)()) dlsym(g_proxy_resolver_gnome3.gio_module, "g_cancellable_new");
    if (!g_proxy_resolver_gnome3.g_cancellable_new)
        goto gnome3_init_error;
    g_proxy_resolver_gnome3.g_cancellable_cancel =
        (void (*)(GCancellable *))dlsym(g_proxy_resolver_gnome3.gio_module, "g_cancellable_cancel");
    if (!g_proxy_resolver_gnome3.g_cancellable_cancel)
        goto gnome3_init_error;

    // GProxyResolver functions
    g_proxy_resolver_gnome3.g_proxy_resolver_get_default =
        (GProxyResolver * (*)()) dlsym(g_proxy_resolver_gnome3.gio_module, "g_proxy_resolver_get_default");
    if (!g_proxy_resolver_gnome3.g_proxy_resolver_get_default)
        goto gnome3_init_error;
    g_proxy_resolver_gnome3.g_proxy_resolver_lookup =
        (gchar * *(*)(GProxyResolver *, const gchar *, GCancellable *, GError **))
            dlsym(g_proxy_resolver_gnome3.gio_module, "g_proxy_resolver_lookup");
    if (!g_proxy_resolver_gnome3.g_proxy_resolver_lookup)
        goto gnome3_init_error;

    return true;

gnome3_init_error:
    proxy_resolver_gnome3_global_cleanup();
    return false;
}

bool proxy_resolver_gnome3_global_cleanup(void) {
    if (g_proxy_resolver_gnome3.gio_module)
        dlclose(g_proxy_resolver_gnome3.gio_module);
    if (g_proxy_resolver_gnome3.glib_module)
        dlclose(g_proxy_resolver_gnome3.glib_module);

    memset(&g_proxy_resolver_gnome3, 0, sizeof(g_proxy_resolver_gnome3));
    return true;
}

const proxy_resolver_i_s *proxy_resolver_gnome3_get_interface(void) {
    static const proxy_resolver_i_s proxy_resolver_gnome3_i = {
        proxy_resolver_gnome3_get_proxies_for_url,
        proxy_resolver_gnome3_get_list,
        proxy_resolver_gnome3_get_error,
        proxy_resolver_gnome3_wait,
        proxy_resolver_gnome3_cancel,
        proxy_resolver_gnome3_create,
        proxy_resolver_gnome3_delete,
        false,  // get_proxies_for_url should be spooled to another thread
        true,   // get_proxies_for_url takes into account system config
        proxy_resolver_gnome3_global_init,
        proxy_resolver_gnome3_global_cleanup};
    return &proxy_resolver_gnome3_i;
}
