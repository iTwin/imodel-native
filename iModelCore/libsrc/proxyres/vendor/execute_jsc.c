#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <dlfcn.h>
#include <glib-object.h>
#include <link.h>

#include <jsc/jsc.h>

#include "execute.h"
#include "execute_i.h"
#include "execute_jsc.h"
#include "log.h"
#include "mozilla_js.h"
#include "net_util.h"
#include "util.h"

typedef struct g_proxy_execute_jsc_s {
    // JSCoreGTK module
    void *module;
    // GObject functions
    void (*g_object_unref)(gpointer object);
    // Context functions
    JSCContext *(*jsc_context_new)(void);
    JSCValue *(*jsc_context_get_global_object)(JSCContext *context);
    JSCValue *(*jsc_context_evaluate)(JSCContext *context, const char *code, gssize length);
    JSCException *(*jsc_context_get_exception)(JSCContext *context);
    void (*jsc_context_set_value)(JSCContext *context, const char *name, JSCValue *value);
    void (*jsc_context_garbage_collect)(JSCContext *, bool sanitize_stack);
    // Value functions
    gboolean (*jsc_value_is_string)(JSCValue *value);
    gboolean (*jsc_value_is_number)(JSCValue *value);
    gboolean (*jsc_value_is_object)(JSCValue *value);
    double (*jsc_value_to_double)(JSCValue *value);
    JSCValue *(*jsc_value_new_string)(JSCContext *context, const char *string);
    char *(*jsc_value_to_string)(JSCValue *value);
    JSCValue *(*jsc_value_new_function)(JSCContext *context, const char *name, GCallback callback, gpointer user_data,
                                        GDestroyNotify destroy_notify, GType return_type, guint n_params, ...);
    JSCValue *(*jsc_value_object_get_property)(JSCValue *value, const char *name);
    // Exception functions
    char *(*jsc_exception_report)(JSCException *exception);
} g_proxy_execute_jsc_s;

g_proxy_execute_jsc_s g_proxy_execute_jsc;
static pthread_once_t g_proxy_execute_jsc_init_flag = PTHREAD_ONCE_INIT;

typedef struct proxy_execute_jsc_s {
    // Execute error
    int32_t error;
    // Proxy list
    char *list;
} proxy_execute_jsc_s;

static void js_print_exception(JSCContext *context, JSCException *exception) {
    if (!exception)
        return;

    char *report = g_proxy_execute_jsc.jsc_exception_report(exception);
    if (report) {
        printf("EXCEPTION: %s\n", report);
        free(report);
        return;
    }

    log_error("Unable to print exception object");
}

static char *proxy_execute_jsc_dns_resolve(const char *host) {
    if (!host)
        return NULL;

    return dns_resolve(host, NULL);
}

static char *proxy_execute_jsc_dns_resolve_ex(const char *host) {
    if (!host)
        return NULL;

    return dns_resolve_ex(host, NULL);
}

static char *proxy_execute_jsc_my_ip_address(void) {
    return my_ip_address();
}

static char *proxy_execute_jsc_my_ip_address_ex(void) {
    return my_ip_address_ex();
}

bool proxy_execute_jsc_get_proxies_for_url(void *ctx, const char *script, const char *url) {
    proxy_execute_jsc_s *proxy_execute = (proxy_execute_jsc_s *)ctx;
    JSCContext *global = NULL;
    JSCException *exception = NULL;
    JSCValue *result = NULL;
    char find_proxy[4096];
    bool is_ok = false;
    char *host = NULL;

    if (!proxy_execute)
        goto jscgtk_execute_cleanup;

    global = g_proxy_execute_jsc.jsc_context_new();
    if (!global) {
        log_error("Failed to create global JS context");
        goto jscgtk_execute_cleanup;
    }

    // Array of JavaScript function names and corresponding callbacks
    static struct {
        const char *name;
        GCallback callback;
        GType return_type;
        gint param_count;
        JSCValue *value;
    } functions[] = {{"dnsResolve", G_CALLBACK(proxy_execute_jsc_dns_resolve), G_TYPE_STRING, 1},
                     {"dnsResolveEx", G_CALLBACK(proxy_execute_jsc_dns_resolve_ex), G_TYPE_STRING, 1},
                     {"myIpAddress", G_CALLBACK(proxy_execute_jsc_my_ip_address), G_TYPE_STRING, 0},
                     {"myIpAddressEx", G_CALLBACK(proxy_execute_jsc_my_ip_address_ex), G_TYPE_STRING, 0}};

    // Register native functions with JavaScript engine
    for (uint32_t i = 0; i < sizeof(functions) / sizeof(functions[0]); i++) {
        functions[i].value = g_proxy_execute_jsc.jsc_value_new_function(
            global, functions[i].name, functions[i].callback, NULL, NULL, functions[i].return_type,
            functions[i].param_count, G_TYPE_STRING);

        if (!functions[i].value) {
            log_error("Unable to hook native function for %s", functions[i].name);
            goto jscgtk_execute_cleanup;
        }

        g_proxy_execute_jsc.jsc_context_set_value(global, functions[i].name, functions[i].value);
    }

    // Load Mozilla's JavaScript PAC utilities to help process PAC files
    result = g_proxy_execute_jsc.jsc_context_evaluate(global, MOZILLA_PAC_JAVASCRIPT, -1);
    exception = g_proxy_execute_jsc.jsc_context_get_exception(global);
    if (exception) {
        log_error("Unable to execute Mozilla's JavaScript PAC utilities");
        js_print_exception(global, exception);
        goto jscgtk_execute_cleanup;
    }

    // Load PAC script
    if (result)
        g_proxy_execute_jsc.g_object_unref(result);
    result = g_proxy_execute_jsc.jsc_context_evaluate(global, script, -1);
    exception = g_proxy_execute_jsc.jsc_context_get_exception(global);
    if (exception) {
        log_error("Unable to execute PAC script");
        js_print_exception(global, exception);
        goto jscgtk_execute_cleanup;
    }

    // Construct the call FindProxyForURL
    host = get_url_host(url);
    snprintf(find_proxy, sizeof(find_proxy), "FindProxyForURL(\"%s\", \"%s\");", url, host ? host : url);
    free(host);

    // Execute the call to FindProxyForURL
    if (result)
        g_proxy_execute_jsc.g_object_unref(result);
    result = g_proxy_execute_jsc.jsc_context_evaluate(global, find_proxy, -1);
    exception = g_proxy_execute_jsc.jsc_context_get_exception(global);
    if (exception) {
        log_error("Unable to execute FindProxyForURL");
        js_print_exception(global, exception);
        goto jscgtk_execute_cleanup;
    }

    if (!g_proxy_execute_jsc.jsc_value_is_string(result)) {
        log_error("Incorrect return type from FindProxyForURL");
        goto jscgtk_execute_cleanup;
    }

    // Get the result of the call to FindProxyForURL
    if (result && g_proxy_execute_jsc.jsc_value_is_string(result)) {
        proxy_execute->list = g_proxy_execute_jsc.jsc_value_to_string(result);
        is_ok = true;
    }

jscgtk_execute_cleanup:

    if (result)
        g_proxy_execute_jsc.g_object_unref(result);

    for (uint32_t i = 0; i < sizeof(functions) / sizeof(functions[0]); i++) {
        if (functions[i].value)
            g_object_unref(functions[i].value);
    }

    if (global) {
        if (g_proxy_execute_jsc.jsc_context_garbage_collect)
            g_proxy_execute_jsc.jsc_context_garbage_collect(global, false);
        g_object_unref(global);
    }

    return is_ok;
}

const char *proxy_execute_jsc_get_list(void *ctx) {
    proxy_execute_jsc_s *proxy_execute = (proxy_execute_jsc_s *)ctx;
    return proxy_execute->list;
}

int32_t proxy_execute_jsc_get_error(void *ctx) {
    proxy_execute_jsc_s *proxy_execute = (proxy_execute_jsc_s *)ctx;
    return proxy_execute->error;
}

void proxy_execute_jsc_delayed_init(void) {
    if (g_proxy_execute_jsc.module)
        return;

    const char *library_names[] = {"libjavascriptcoregtk-6.0.so.1", "libjavascriptcoregtk-4.1.so.0",
                                   "libjavascriptcoregtk-4.0.so.18"};
    const size_t library_names_size = sizeof(library_names) / sizeof(library_names[0]);

    // Use existing JavaScriptCoreGTK if already loaded
    struct link_map *map = NULL;
    void *current_process = dlopen(0, RTLD_LAZY);
    if (!current_process)
        return;
    if (dlinfo(current_process, RTLD_DI_LINKMAP, &map) == 0) {
        while (map && !g_proxy_execute_jsc.module) {
            for (size_t i = 0; i < library_names_size; i++) {
                if (strstr(map->l_name, library_names[i])) {
                    g_proxy_execute_jsc.module = dlopen(map->l_name, RTLD_NOLOAD | RTLD_LAZY | RTLD_LOCAL);
                    break;
                }
            }
            map = map->l_next;
        }
    }
    dlclose(current_process);

    // Load the first available version of the JavaScriptCoreGTK
    for (size_t i = 0; !g_proxy_execute_jsc.module && i < library_names_size; i++) {
        g_proxy_execute_jsc.module = dlopen(library_names[i], RTLD_LAZY | RTLD_LOCAL);
    }

    if (!g_proxy_execute_jsc.module)
        return;

    // GObject functions (loaded as a dependency of the module)
    g_proxy_execute_jsc.g_object_unref = (void (*)(gpointer))dlsym(g_proxy_execute_jsc.module, "g_object_unref");
    if (!g_proxy_execute_jsc.g_object_unref)
        goto jsc_init_error;

    // Context functions
    g_proxy_execute_jsc.jsc_context_new = (JSCContext * (*)()) dlsym(g_proxy_execute_jsc.module, "jsc_context_new");
    if (!g_proxy_execute_jsc.jsc_context_new)
        goto jsc_init_error;
    g_proxy_execute_jsc.jsc_context_get_global_object =
        (JSCValue * (*)(JSCContext *)) dlsym(g_proxy_execute_jsc.module, "jsc_context_get_global_object");
    if (!g_proxy_execute_jsc.jsc_context_get_global_object)
        goto jsc_init_error;
    g_proxy_execute_jsc.jsc_context_evaluate =
        (JSCValue * (*)(JSCContext *, const char *, gssize)) dlsym(g_proxy_execute_jsc.module, "jsc_context_evaluate");
    if (!g_proxy_execute_jsc.jsc_context_evaluate)
        goto jsc_init_error;
    g_proxy_execute_jsc.jsc_context_get_exception =
        (JSCException * (*)(JSCContext *)) dlsym(g_proxy_execute_jsc.module, "jsc_context_get_exception");
    if (!g_proxy_execute_jsc.jsc_context_get_exception)
        goto jsc_init_error;
    g_proxy_execute_jsc.jsc_context_set_value =
        (void (*)(JSCContext *, const char *, JSCValue *))dlsym(g_proxy_execute_jsc.module, "jsc_context_set_value");
    if (!g_proxy_execute_jsc.jsc_context_set_value)
        goto jsc_init_error;
    // JS_EXPORT_PRIVATE void jscContextGarbageCollect(JSCContext*, bool sanitizeStack = false) is undocumented, may be
    // unavailable, and is not declared with C language linkage.
    g_proxy_execute_jsc.jsc_context_garbage_collect =
        (void (*)(JSCContext *, bool))dlsym(g_proxy_execute_jsc.module, "_Z24jscContextGarbageCollectP11_JSCContextb");
    // Value functions
    g_proxy_execute_jsc.jsc_value_is_string =
        (gboolean(*)(JSCValue *))dlsym(g_proxy_execute_jsc.module, "jsc_value_is_string");
    if (!g_proxy_execute_jsc.jsc_value_is_string)
        goto jsc_init_error;
    g_proxy_execute_jsc.jsc_value_is_number =
        (gboolean(*)(JSCValue *))dlsym(g_proxy_execute_jsc.module, "jsc_value_is_number");
    if (!g_proxy_execute_jsc.jsc_value_is_number)
        goto jsc_init_error;
    g_proxy_execute_jsc.jsc_value_is_object =
        (gboolean(*)(JSCValue *))dlsym(g_proxy_execute_jsc.module, "jsc_value_is_object");
    if (!g_proxy_execute_jsc.jsc_value_is_object)
        goto jsc_init_error;
    g_proxy_execute_jsc.jsc_value_to_double =
        (double (*)(JSCValue *))dlsym(g_proxy_execute_jsc.module, "jsc_value_to_double");
    if (!g_proxy_execute_jsc.jsc_value_to_double)
        goto jsc_init_error;
    g_proxy_execute_jsc.jsc_value_new_string =
        (JSCValue * (*)(JSCContext *, const char *)) dlsym(g_proxy_execute_jsc.module, "jsc_value_new_string");
    if (!g_proxy_execute_jsc.jsc_value_new_string)
        goto jsc_init_error;
    g_proxy_execute_jsc.jsc_value_to_string =
        (char *(*)(JSCValue *))dlsym(g_proxy_execute_jsc.module, "jsc_value_to_string");
    if (!g_proxy_execute_jsc.jsc_value_to_string)
        goto jsc_init_error;
    g_proxy_execute_jsc.jsc_value_new_function =
        (JSCValue * (*)(JSCContext *, const char *, GCallback, gpointer, GDestroyNotify, GType, guint, ...))
            dlsym(g_proxy_execute_jsc.module, "jsc_value_new_function");
    if (!g_proxy_execute_jsc.jsc_value_new_function)
        goto jsc_init_error;
    g_proxy_execute_jsc.jsc_value_object_get_property =
        (JSCValue * (*)(JSCValue *, const char *)) dlsym(g_proxy_execute_jsc.module, "jsc_value_object_get_property");
    if (!g_proxy_execute_jsc.jsc_value_object_get_property)
        goto jsc_init_error;
    // Exception functions
    g_proxy_execute_jsc.jsc_exception_report =
        (char *(*)(JSCException *))dlsym(g_proxy_execute_jsc.module, "jsc_exception_report");
    if (!g_proxy_execute_jsc.jsc_exception_report)
        goto jsc_init_error;

    return;

jsc_init_error:
    proxy_execute_jsc_global_cleanup();
}

void *proxy_execute_jsc_create(void) {
    pthread_once(&g_proxy_execute_jsc_init_flag, proxy_execute_jsc_delayed_init);
    if (!g_proxy_execute_jsc.module)
        return NULL;
    proxy_execute_jsc_s *proxy_execute = (proxy_execute_jsc_s *)calloc(1, sizeof(proxy_execute_jsc_s));
    return proxy_execute;
}

bool proxy_execute_jsc_delete(void **ctx) {
    if (!ctx)
        return false;
    proxy_execute_jsc_s *proxy_execute = (proxy_execute_jsc_s *)*ctx;
    if (!proxy_execute)
        return false;
    free(proxy_execute->list);
    free(proxy_execute);
    *ctx = NULL;
    return true;
}

/*********************************************************************/

bool proxy_execute_jsc_global_init(void) {
    // JSCoreGTK will be initialized with a delay to avoid conflicts with the
    // loaded JSCoreGTK in a user application after this function.
    return true;
}

bool proxy_execute_jsc_global_cleanup(void) {
#if 0  // Do not unload the library, as a thread in its address space may run
    if (g_proxy_execute_jsc.module)
        dlclose(g_proxy_execute_jsc.module);

    memset(&g_proxy_execute_jsc, 0, sizeof(g_proxy_execute_jsc));
    g_proxy_execute_jsc_init_flag = PTHREAD_ONCE_INIT;
#endif
    return true;
}

proxy_execute_i_s *proxy_execute_jsc_get_interface(void) {
    static proxy_execute_i_s proxy_execute_jsc_i = {proxy_execute_jsc_get_proxies_for_url,
                                                    proxy_execute_jsc_get_list,
                                                    proxy_execute_jsc_get_error,
                                                    proxy_execute_jsc_create,
                                                    proxy_execute_jsc_delete,
                                                    proxy_execute_jsc_global_init,
                                                    proxy_execute_jsc_global_cleanup};
    return &proxy_execute_jsc_i;
}
