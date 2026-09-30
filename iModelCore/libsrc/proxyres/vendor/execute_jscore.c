#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <dlfcn.h>

#include <JavaScriptCore/JavaScript.h>

#include "execute.h"
#include "execute_i.h"
#include "execute_jscore.h"
#include "log.h"
#include "mozilla_js.h"
#include "net_util.h"
#include "util.h"

typedef struct g_proxy_execute_jscore_s {
    // JSCoreGTK module
    void *module;
    // Object functions
    JSObjectRef (*JSObjectMakeFunctionWithCallback)(JSContextRef ctx, JSStringRef name,
                                                    JSObjectCallAsFunctionCallback callback);
    void (*JSObjectSetProperty)(JSContextRef context, JSObjectRef object, JSStringRef name, JSValueRef value,
                                JSPropertyAttributes attribs, JSValueRef *exception);
    JSValueRef (*JSObjectGetProperty)(JSContextRef ctx, JSObjectRef object, JSStringRef name, JSValueRef *exception);
    // Context functions
    JSObjectRef (*JSContextGetGlobalObject)(JSContextRef ctx);
    // Value functions
    bool (*JSValueIsString)(JSContextRef ctx, JSValueRef value);
    double (*JSValueIsNumber)(JSContextRef ctx, JSValueRef value);
    JSObjectRef (*JSValueToObject)(JSContextRef ctx, JSValueRef value, JSValueRef *exception);
    JSStringRef (*JSValueToStringCopy)(JSContextRef ctx, JSValueRef value, JSValueRef *exception);
    double (*JSValueToNumber)(JSContextRef ctx, JSValueRef value, JSValueRef *exception);
    JSValueRef (*JSValueMakeString)(JSContextRef ctx, JSStringRef str);
    // String functions
    JSStringRef (*JSStringCreateWithUTF8CString)(const char *str);
    size_t (*JSStringGetUTF8CString)(JSStringRef str, char *buffer, size_t buffer_size);
    size_t (*JSStringGetMaximumUTF8CStringSize)(JSStringRef str);
    void (*JSStringRelease)(JSStringRef str);
    // Global context functions
    JSGlobalContextRef (*JSGlobalContextCreate)(JSClassRef global_object_cls);
    void (*JSGlobalContextRelease)(JSGlobalContextRef global_ctx);
    // Exception functions
    JSValueRef (*JSEvaluateScript)(JSContextRef ctx, JSStringRef script, JSObjectRef object, JSStringRef source_url,
                                   int start_line_num, JSValueRef *exception);
    // Garbage collection functions
    void (*JSGarbageCollect)(JSContextRef ctx);
} g_proxy_execute_jscore_s;

static g_proxy_execute_jscore_s g_proxy_execute_jscore;
static pthread_once_t g_proxy_execute_jscore_init_flag = PTHREAD_ONCE_INIT;

typedef struct proxy_execute_jscore_s {
    // Execute error
    int32_t error;
    // Proxy list
    char *list;
} proxy_execute_jscore_s;

static char *js_string_dup_to_utf8(JSStringRef str) {
    size_t utf8_string_size = 0;
    char *utf8_string = NULL;

    if (!str)
        return NULL;

    utf8_string_size = g_proxy_execute_jscore.JSStringGetMaximumUTF8CStringSize(str) + 1;
    utf8_string = (char *)calloc(utf8_string_size, sizeof(char));
    if (!utf8_string)
        return NULL;
    g_proxy_execute_jscore.JSStringGetUTF8CString(str, utf8_string, utf8_string_size);
    return utf8_string;
}

double js_object_get_double_property(JSContextRef ctx, JSObjectRef object, const char *name, JSValueRef *exception) {
    JSValueRef property = NULL;

    JSStringRef name_string = g_proxy_execute_jscore.JSStringCreateWithUTF8CString(name);
    if (name_string) {
        property = g_proxy_execute_jscore.JSObjectGetProperty(ctx, object, name_string, exception);
        g_proxy_execute_jscore.JSStringRelease(name_string);
    }

    if (property && g_proxy_execute_jscore.JSValueIsNumber(ctx, property))
        return g_proxy_execute_jscore.JSValueToNumber(ctx, property, exception);
    return 0;
}

JSStringRef js_object_get_string_property(JSContextRef ctx, JSObjectRef object, const char *name,
                                          JSValueRef *exception) {
    JSValueRef property = NULL;

    JSStringRef name_string = g_proxy_execute_jscore.JSStringCreateWithUTF8CString(name);
    if (name_string) {
        property = g_proxy_execute_jscore.JSObjectGetProperty(ctx, object, name_string, exception);
        g_proxy_execute_jscore.JSStringRelease(name_string);
    }

    if (property && g_proxy_execute_jscore.JSValueIsString(ctx, property))
        return g_proxy_execute_jscore.JSValueToStringCopy(ctx, property, exception);

    return NULL;
}

static void js_print_exception(JSContextRef ctx, JSValueRef exception) {
    bool printed = false;

    if (!exception)
        return;

    JSObjectRef exception_object = g_proxy_execute_jscore.JSValueToObject(ctx, exception, NULL);
    if (exception_object) {
        int line = js_object_get_double_property(ctx, exception_object, "line", NULL);
        JSStringRef message_string = js_object_get_string_property(ctx, exception_object, "message", NULL);
        if (message_string) {
            char *message = js_string_dup_to_utf8(message_string);
            if (message) {
                printf("EXCEPTION: %s (line %d)\n", message, line);
                free(message);
                printed = true;
            }
            g_proxy_execute_jscore.JSStringRelease(message_string);
        }
    }

    if (!printed) {
        log_error("Unable to print unknown exception object");
        return;
    }
}

static JSValueRef proxy_execute_jscore_dns_resolve(JSContextRef ctx, JSObjectRef function, JSObjectRef object,
                                                   size_t argc, const JSValueRef argv[], JSValueRef *exception) {
    if (argc != 1)
        return NULL;
    if (!g_proxy_execute_jscore.JSValueIsString(ctx, argv[0]))
        return NULL;

    JSStringRef host_string = g_proxy_execute_jscore.JSValueToStringCopy(ctx, argv[0], NULL);
    if (!host_string)
        return NULL;

    char *host = js_string_dup_to_utf8(host_string);
    g_proxy_execute_jscore.JSStringRelease(host_string);
    if (!host)
        return NULL;

    char *address = dns_resolve(host, NULL);
    free(host);
    if (!address)
        return NULL;

    JSStringRef address_string = g_proxy_execute_jscore.JSStringCreateWithUTF8CString(address);
    free(address);
    if (!address_string)
        return NULL;

    JSValueRef address_value = g_proxy_execute_jscore.JSValueMakeString(ctx, address_string);
    g_proxy_execute_jscore.JSStringRelease(address_string);
    return address_value;
}

static JSValueRef proxy_execute_jscore_dns_resolve_ex(JSContextRef ctx, JSObjectRef function, JSObjectRef object,
                                                      size_t argc, const JSValueRef argv[], JSValueRef *exception) {
    if (argc != 1)
        return NULL;
    if (!g_proxy_execute_jscore.JSValueIsString(ctx, argv[0]))
        return NULL;

    JSStringRef host_string = g_proxy_execute_jscore.JSValueToStringCopy(ctx, argv[0], NULL);
    if (!host_string)
        return NULL;

    char *host = js_string_dup_to_utf8(host_string);
    g_proxy_execute_jscore.JSStringRelease(host_string);
    if (!host)
        return NULL;

    char *address = dns_resolve_ex(host, NULL);
    free(host);
    if (!address)
        return NULL;

    JSStringRef address_string = g_proxy_execute_jscore.JSStringCreateWithUTF8CString(address);
    free(address);
    if (!address_string)
        return NULL;

    JSValueRef address_value = g_proxy_execute_jscore.JSValueMakeString(ctx, address_string);
    g_proxy_execute_jscore.JSStringRelease(address_string);
    return address_value;
}

static JSValueRef proxy_execute_jscore_my_ip_address(JSContextRef ctx, JSObjectRef function, JSObjectRef object,
                                                     size_t argc, const JSValueRef argv[], JSValueRef *exception) {
    char *address = my_ip_address();
    if (!address)
        return NULL;

    JSStringRef address_string = g_proxy_execute_jscore.JSStringCreateWithUTF8CString(address);
    free(address);
    if (!address_string)
        return NULL;

    JSValueRef address_value = g_proxy_execute_jscore.JSValueMakeString(ctx, address_string);
    g_proxy_execute_jscore.JSStringRelease(address_string);
    return address_value;
}

static JSValueRef proxy_execute_jscore_my_ip_address_ex(JSContextRef ctx, JSObjectRef function, JSObjectRef object,
                                                        size_t argc, const JSValueRef argv[], JSValueRef *exception) {
    char *addresses = my_ip_address_ex();
    if (!addresses)
        return NULL;

    JSStringRef addresses_string = g_proxy_execute_jscore.JSStringCreateWithUTF8CString(addresses);
    free(addresses);
    if (!addresses_string)
        return NULL;

    JSValueRef addresses_value = g_proxy_execute_jscore.JSValueMakeString(ctx, addresses_string);
    g_proxy_execute_jscore.JSStringRelease(addresses_string);
    return addresses_value;
}

bool proxy_execute_register_function(void *ctx, JSGlobalContextRef global, const char *name,
                                     JSObjectCallAsFunctionCallback callback) {
    // Register native function with JavaScript engine
    JSStringRef name_string = g_proxy_execute_jscore.JSStringCreateWithUTF8CString(name);
    if (!name_string)
        return false;
    JSObjectRef function = g_proxy_execute_jscore.JSObjectMakeFunctionWithCallback(global, name_string, callback);
    if (!function) {
        g_proxy_execute_jscore.JSStringRelease(name_string);
        log_error("Unable to hook native function for %s", name);
        return false;
    }

    g_proxy_execute_jscore.JSObjectSetProperty(global, g_proxy_execute_jscore.JSContextGetGlobalObject(global),
                                               name_string, function, kJSPropertyAttributeNone, NULL);
    g_proxy_execute_jscore.JSStringRelease(name_string);
    return true;
}

bool proxy_execute_jscore_get_proxies_for_url(void *ctx, const char *script, const char *url) {
    proxy_execute_jscore_s *proxy_execute = (proxy_execute_jscore_s *)ctx;
    JSGlobalContextRef global = NULL;
    JSValueRef exception = NULL;
    char find_proxy[4096];
    bool is_ok = false;
    JSStringRef utils_javascript = NULL;
    JSStringRef script_string = NULL;
    char *host = NULL;
    JSStringRef proxy_string = NULL;
    JSValueRef proxy_value = NULL;
    JSStringRef find_proxy_string = NULL;

    if (!proxy_execute)
        goto jscoregtk_execute_cleanup;

    global = g_proxy_execute_jscore.JSGlobalContextCreate(NULL);
    if (!global) {
        log_error("Failed to create global JS context");
        goto jscoregtk_execute_cleanup;
    }

    // Register dnsResolve C function
    if (!proxy_execute_register_function(ctx, global, "dnsResolve", proxy_execute_jscore_dns_resolve))
        goto jscoregtk_execute_cleanup;
    if (!proxy_execute_register_function(ctx, global, "dnsResolveEx", proxy_execute_jscore_dns_resolve_ex))
        goto jscoregtk_execute_cleanup;
    if (!proxy_execute_register_function(ctx, global, "myIpAddress", proxy_execute_jscore_my_ip_address))
        goto jscoregtk_execute_cleanup;
    if (!proxy_execute_register_function(ctx, global, "myIpAddressEx", proxy_execute_jscore_my_ip_address_ex))
        goto jscoregtk_execute_cleanup;

    // Load Mozilla's JavaScript PAC utilities to help process PAC files
    utils_javascript = g_proxy_execute_jscore.JSStringCreateWithUTF8CString(MOZILLA_PAC_JAVASCRIPT);
    if (!utils_javascript) {
        log_error("Unable to load Mozilla's JavaScript PAC utilities");
        goto jscoregtk_execute_cleanup;
    }
    g_proxy_execute_jscore.JSEvaluateScript(global, utils_javascript, NULL, NULL, 1, &exception);
    g_proxy_execute_jscore.JSStringRelease(utils_javascript);
    if (exception) {
        log_error("Unable to execute Mozilla's JavaScript PAC utilities");
        js_print_exception(global, exception);
        goto jscoregtk_execute_cleanup;
    }

    // Load PAC script
    script_string = g_proxy_execute_jscore.JSStringCreateWithUTF8CString(script);
    g_proxy_execute_jscore.JSEvaluateScript(global, script_string, NULL, NULL, 1, &exception);
    g_proxy_execute_jscore.JSStringRelease(script_string);
    if (exception) {
        log_error("Unable to execute PAC script");
        js_print_exception(global, exception);
        goto jscoregtk_execute_cleanup;
    }

    // Construct the call FindProxyForURL
    host = get_url_host(url);
    snprintf(find_proxy, sizeof(find_proxy), "FindProxyForURL(\"%s\", \"%s\");", url, host ? host : url);
    free(host);

    // Execute the call to FindProxyForURL
    find_proxy_string = g_proxy_execute_jscore.JSStringCreateWithUTF8CString(find_proxy);
    if (!find_proxy_string)
        goto jscoregtk_execute_cleanup;
    proxy_value = g_proxy_execute_jscore.JSEvaluateScript(global, find_proxy_string, NULL, NULL, 1, &exception);
    g_proxy_execute_jscore.JSStringRelease(find_proxy_string);
    if (exception) {
        log_error("Unable to execute FindProxyForURL");
        js_print_exception(global, exception);
        goto jscoregtk_execute_cleanup;
    }

    if (!g_proxy_execute_jscore.JSValueIsString(global, proxy_value)) {
        log_error("Incorrect return type from FindProxyForURL");
        goto jscoregtk_execute_cleanup;
    }

    // Get the result of the call to FindProxyForURL
    proxy_string = g_proxy_execute_jscore.JSValueToStringCopy(global, proxy_value, NULL);
    if (proxy_string) {
        proxy_execute->list = js_string_dup_to_utf8(proxy_string);
        g_proxy_execute_jscore.JSStringRelease(proxy_string);
        is_ok = true;
    }

jscoregtk_execute_cleanup:

    if (global) {
        g_proxy_execute_jscore.JSGarbageCollect(global);
        g_proxy_execute_jscore.JSGlobalContextRelease(global);
    }

    return is_ok;
}

const char *proxy_execute_jscore_get_list(void *ctx) {
    proxy_execute_jscore_s *proxy_execute = (proxy_execute_jscore_s *)ctx;
    return proxy_execute->list;
}

int32_t proxy_execute_jscore_get_error(void *ctx) {
    proxy_execute_jscore_s *proxy_execute = (proxy_execute_jscore_s *)ctx;
    return proxy_execute->error;
}

void proxy_execute_jscore_delayed_init(void) {
#ifdef __APPLE__
    g_proxy_execute_jscore.module = dlopen(
        "/System/Library/Frameworks/JavaScriptCore.framework/Versions/Current/JavaScriptCore", RTLD_LAZY | RTLD_LOCAL);
#else
    const char *library_names[] = {"libjavascriptcoregtk-4.1.so.0", "libjavascriptcoregtk-4.0.so.18",
                                   "libjavascriptcoregtk-3.0.so.0", "libjavascriptcoregtk-1.0.so.0"};
    const size_t library_names_size = sizeof(library_names) / sizeof(library_names[0]);

    // Use existing JavaScriptCoreGTK if already loaded
    struct link_map *map = NULL;
    void *current_process = dlopen(0, RTLD_LAZY);
    if (!current_process)
        return;
    if (dlinfo(current_process, RTLD_DI_LINKMAP, &map) == 0) {
        while (map && !g_proxy_execute_jscore.module) {
            for (size_t i = 0; i < library_names_size; i++) {
                if (strstr(map->l_name, library_names[i])) {
                    g_proxy_execute_jscore.module = dlopen(map->l_name, RTLD_NOLOAD | RTLD_LAZY | RTLD_LOCAL);
                    break;
                }
            }
            map = map->l_next;
        }
    }
    dlclose(current_process);

    // Load the first available version of the JavaScriptCoreGTK
    for (size_t i = 0; !g_proxy_execute_jscore.module && i < library_names_size; i++) {
        g_proxy_execute_jscore.module = dlopen(library_names[i], RTLD_LAZY | RTLD_LOCAL);
    }
#endif

    if (!g_proxy_execute_jscore.module)
        return;

    // Object functions
    g_proxy_execute_jscore.JSObjectMakeFunctionWithCallback =
        (JSObjectRef(*)(JSContextRef, JSStringRef, JSObjectCallAsFunctionCallback))dlsym(
            g_proxy_execute_jscore.module, "JSObjectMakeFunctionWithCallback");
    if (!g_proxy_execute_jscore.JSObjectMakeFunctionWithCallback)
        goto jscore_init_error;
    g_proxy_execute_jscore.JSObjectGetProperty =
        (JSValueRef(*)(JSContextRef, JSObjectRef, JSStringRef, JSValueRef *))dlsym(g_proxy_execute_jscore.module,
                                                                                   "JSObjectGetProperty");
    if (!g_proxy_execute_jscore.JSObjectGetProperty)
        goto jscore_init_error;
    g_proxy_execute_jscore.JSObjectSetProperty =
        (void (*)(JSContextRef, JSObjectRef, JSStringRef, JSValueRef, JSPropertyAttributes, JSValueRef *))dlsym(
            g_proxy_execute_jscore.module, "JSObjectSetProperty");
    if (!g_proxy_execute_jscore.JSObjectSetProperty)
        goto jscore_init_error;
    // Context functions
    g_proxy_execute_jscore.JSContextGetGlobalObject =
        (JSObjectRef(*)(JSContextRef))dlsym(g_proxy_execute_jscore.module, "JSContextGetGlobalObject");
    if (!g_proxy_execute_jscore.JSContextGetGlobalObject)
        goto jscore_init_error;
    // Value functions
    g_proxy_execute_jscore.JSValueIsString =
        (bool (*)(JSContextRef, JSValueRef))dlsym(g_proxy_execute_jscore.module, "JSValueIsString");
    if (!g_proxy_execute_jscore.JSValueIsString)
        goto jscore_init_error;
    g_proxy_execute_jscore.JSValueIsNumber =
        (double (*)(JSContextRef, JSValueRef))dlsym(g_proxy_execute_jscore.module, "JSValueIsNumber");
    if (!g_proxy_execute_jscore.JSValueIsNumber)
        goto jscore_init_error;
    g_proxy_execute_jscore.JSValueToObject =
        (JSObjectRef(*)(JSContextRef, JSValueRef, JSValueRef *))dlsym(g_proxy_execute_jscore.module, "JSValueToObject");
    if (!g_proxy_execute_jscore.JSValueToObject)
        goto jscore_init_error;
    g_proxy_execute_jscore.JSValueToStringCopy = (JSStringRef(*)(JSContextRef, JSValueRef, JSValueRef *))dlsym(
        g_proxy_execute_jscore.module, "JSValueToStringCopy");
    if (!g_proxy_execute_jscore.JSValueToStringCopy)
        goto jscore_init_error;
    g_proxy_execute_jscore.JSValueToNumber =
        (double (*)(JSContextRef, JSValueRef, JSValueRef *))dlsym(g_proxy_execute_jscore.module, "JSValueToNumber");
    if (!g_proxy_execute_jscore.JSValueToNumber)
        goto jscore_init_error;
    g_proxy_execute_jscore.JSValueMakeString =
        (JSValueRef(*)(JSContextRef, JSStringRef))dlsym(g_proxy_execute_jscore.module, "JSValueMakeString");
    if (!g_proxy_execute_jscore.JSValueMakeString)
        goto jscore_init_error;
    // String functions
    g_proxy_execute_jscore.JSStringCreateWithUTF8CString =
        (JSStringRef(*)(const char *))dlsym(g_proxy_execute_jscore.module, "JSStringCreateWithUTF8CString");
    if (!g_proxy_execute_jscore.JSStringCreateWithUTF8CString)
        goto jscore_init_error;
    g_proxy_execute_jscore.JSStringGetUTF8CString =
        (size_t (*)(JSStringRef, char *, size_t))dlsym(g_proxy_execute_jscore.module, "JSStringGetUTF8CString");
    if (!g_proxy_execute_jscore.JSStringGetUTF8CString)
        goto jscore_init_error;
    g_proxy_execute_jscore.JSStringGetMaximumUTF8CStringSize =
        (size_t (*)(JSStringRef))dlsym(g_proxy_execute_jscore.module, "JSStringGetMaximumUTF8CStringSize");
    if (!g_proxy_execute_jscore.JSStringGetMaximumUTF8CStringSize)
        goto jscore_init_error;
    g_proxy_execute_jscore.JSStringGetMaximumUTF8CStringSize =
        (size_t (*)(JSStringRef))dlsym(g_proxy_execute_jscore.module, "JSStringGetMaximumUTF8CStringSize");
    if (!g_proxy_execute_jscore.JSStringGetMaximumUTF8CStringSize)
        goto jscore_init_error;
    g_proxy_execute_jscore.JSStringRelease =
        (void (*)(JSStringRef))dlsym(g_proxy_execute_jscore.module, "JSStringRelease");
    if (!g_proxy_execute_jscore.JSStringRelease)
        goto jscore_init_error;
    // Global context functions
    g_proxy_execute_jscore.JSGlobalContextCreate =
        (JSGlobalContextRef(*)(JSClassRef))dlsym(g_proxy_execute_jscore.module, "JSGlobalContextCreate");
    if (!g_proxy_execute_jscore.JSGlobalContextCreate)
        goto jscore_init_error;
    g_proxy_execute_jscore.JSGlobalContextRelease =
        (void (*)(JSGlobalContextRef))dlsym(g_proxy_execute_jscore.module, "JSGlobalContextRelease");
    if (!g_proxy_execute_jscore.JSGlobalContextRelease)
        goto jscore_init_error;
    // Execute functions
    g_proxy_execute_jscore.JSEvaluateScript =
        (JSValueRef(*)(JSContextRef, JSStringRef, JSObjectRef, JSStringRef, int, JSValueRef *))dlsym(
            g_proxy_execute_jscore.module, "JSEvaluateScript");
    if (!g_proxy_execute_jscore.JSEvaluateScript)
        goto jscore_init_error;
    // Garbage collection functions
    g_proxy_execute_jscore.JSGarbageCollect =
        (void (*)(JSContextRef))dlsym(g_proxy_execute_jscore.module, "JSGarbageCollect");
    if (!g_proxy_execute_jscore.JSGarbageCollect)
        goto jscore_init_error;

    return;

jscore_init_error:
    proxy_execute_jscore_global_cleanup();
}

void *proxy_execute_jscore_create(void) {
    pthread_once(&g_proxy_execute_jscore_init_flag, proxy_execute_jscore_delayed_init);
    if (!g_proxy_execute_jscore.module)
        return NULL;
    proxy_execute_jscore_s *proxy_execute = (proxy_execute_jscore_s *)calloc(1, sizeof(proxy_execute_jscore_s));
    return proxy_execute;
}

bool proxy_execute_jscore_delete(void **ctx) {
    if (!ctx)
        return false;
    proxy_execute_jscore_s *proxy_execute = (proxy_execute_jscore_s *)*ctx;
    if (!proxy_execute)
        return false;
    free(proxy_execute->list);
    free(proxy_execute);
    *ctx = NULL;
    return true;
}

/*********************************************************************/

bool proxy_execute_jscore_global_init(void) {
    // JSCoreGTK will be initialized with a delay to avoid conflicts with the
    // loaded JSCoreGTK in a user application after this function.
    return true;
}

bool proxy_execute_jscore_global_cleanup(void) {
    if (g_proxy_execute_jscore.module)
        dlclose(g_proxy_execute_jscore.module);

    memset(&g_proxy_execute_jscore, 0, sizeof(g_proxy_execute_jscore));

    static const pthread_once_t proxy_execute_jscore_init_flag = PTHREAD_ONCE_INIT;
    memcpy(&g_proxy_execute_jscore_init_flag, &proxy_execute_jscore_init_flag, sizeof(proxy_execute_jscore_init_flag));
    return true;
}

proxy_execute_i_s *proxy_execute_jscore_get_interface(void) {
    static proxy_execute_i_s proxy_execute_jscore_i = {proxy_execute_jscore_get_proxies_for_url,
                                                       proxy_execute_jscore_get_list,
                                                       proxy_execute_jscore_get_error,
                                                       proxy_execute_jscore_create,
                                                       proxy_execute_jscore_delete,
                                                       proxy_execute_jscore_global_init,
                                                       proxy_execute_jscore_global_cleanup};
    return &proxy_execute_jscore_i;
}
