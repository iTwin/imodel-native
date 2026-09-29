#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <CoreFoundation/CoreFoundation.h>
#include <CFNetwork/CFNetwork.h>

#include "config.h"
#include "event.h"
#include "log.h"
#include "resolver.h"
#include "resolver_i.h"
#include "resolver_mac.h"
#include "util.h"

#ifdef LEAK_SANITIZER
#  include <sanitizer/lsan_interface.h>
#endif

#define PROXY_RESOLVER_RUN_LOOP    CFSTR("proxy_resolver_mac.run_loop")
#define PROXY_RESOLVER_TIMEOUT_SEC 10

typedef struct g_proxy_resolver_mac_s {
    bool reserved;
} g_proxy_resolver_mac_s;

g_proxy_resolver_mac_s g_proxy_resolver_mac;

typedef struct proxy_resolver_mac_s {
    // Last system error
    int64_t error;
    // Complete event
    void *complete;
    // Proxy list
    char *list;
} proxy_resolver_mac_s;

static const char *proxy_resolver_mac_get_proxy_scheme(CFStringRef proxy_type) {
    const char *scheme = NULL;
    if (CFEqual(proxy_type, kCFProxyTypeNone))
        scheme = "direct://";
    else if (CFEqual(proxy_type, kCFProxyTypeHTTP))
        scheme = "http://";
    else if (CFEqual(proxy_type, kCFProxyTypeHTTPS))
        // "HTTPS Proxy" on macOS means "use CONNENCT verb for a https:// URL", the proxy still should be an HTTP proxy.
        scheme = "http://";
    else if (CFEqual(proxy_type, kCFProxyTypeSOCKS))
        scheme = "socks://";
    else if (CFEqual(proxy_type, kCFProxyTypeFTP))
        scheme = "ftp://";
    return scheme;
}

static void proxy_resolver_mac_auto_config_result_callback(void *client, CFArrayRef proxy_array, CFErrorRef error) {
    proxy_resolver_mac_s *proxy_resolver = (proxy_resolver_mac_s *)client;
    if (error) {
        // Get error code
        proxy_resolver->error = CFErrorGetCode(error);
    } else {
        // Convert proxy array into PAC file return format
        const size_t proxy_count = CFArrayGetCount(proxy_array);
        const size_t max_list = proxy_count * MAX_PROXY_URL + 1;
        size_t list_len = 0;

        proxy_resolver->list = (char *)calloc(max_list, sizeof(char));

        // Enumerate through each proxy in the array
        for (size_t i = 0; proxy_resolver->list && i < proxy_count && list_len < max_list; i++) {
            const CFDictionaryRef proxy = (CFDictionaryRef)CFArrayGetValueAtIndex(proxy_array, i);
            CFStringRef proxy_type = (CFStringRef)CFDictionaryGetValue(proxy, kCFProxyTypeKey);

            // Copy type of connection
            const char *scheme = proxy_resolver_mac_get_proxy_scheme(proxy_type);
            if (scheme) {
                strncpy(proxy_resolver->list + list_len, scheme, max_list - list_len);
                list_len += strlen(scheme);
            } else {
                log_warn("Unknown proxy type encountered");
                continue;
            }

            if (!CFEqual(proxy_type, kCFProxyTypeNone) && list_len < max_list) {
                // Copy proxy host
                CFStringRef host = (CFStringRef)CFDictionaryGetValue(proxy, kCFProxyHostNameKey);
                if (host && list_len < max_list) {
                    const char *host_utf8 = CFStringGetCStringPtr(host, kCFStringEncodingUTF8);
                    if (host_utf8) {
                        strncpy(proxy_resolver->list + list_len, host_utf8, max_list - list_len);
                        list_len += strlen(host_utf8);
                    } else if (CFStringGetCString(host, proxy_resolver->list + list_len, max_list - list_len,
                                                  kCFStringEncodingUTF8)) {
                        list_len = strlen(proxy_resolver->list);
                    } else {
                        list_len = max_list;
                    }
                }
                // Copy proxy port
                CFNumberRef port = (CFNumberRef)CFDictionaryGetValue(proxy, kCFProxyPortNumberKey);
                if (port && list_len < max_list) {
                    int32_t port_number = 0;
                    CFNumberGetValue(port, kCFNumberSInt32Type, &port_number);
                    list_len +=
                        snprintf(proxy_resolver->list + list_len, max_list - list_len, ":%" PRId32, port_number);
                }
            }

            if (i != proxy_count - 1 && list_len < max_list) {
                // Separate each proxy with a comma
                strncat(proxy_resolver->list, ",", max_list - list_len);
                list_len++;
            }
        }
        if (list_len >= max_list) {
            proxy_resolver->error = ERANGE;
            log_warn("Proxy list limit exceeded");
            free(proxy_resolver->list);
            proxy_resolver->list = NULL;
        } else if (!proxy_resolver->list) {
            proxy_resolver->error = ENOMEM;
        }
    }

    CFRunLoopStop(CFRunLoopGetCurrent());
    return;
}

bool proxy_resolver_mac_get_proxies_for_url(void *ctx, const char *url) {
    proxy_resolver_mac_s *proxy_resolver = (proxy_resolver_mac_s *)ctx;
    CFURLRef target_url_ref = NULL;
    CFURLRef url_ref = NULL;
    char *auto_config_url = NULL;
    bool is_ok = false;
    CFStreamClientContext context = {0};
    CFRunLoopSourceRef run_loop = NULL;

    if (!proxy_resolver || !url)
        return false;

    auto_config_url = proxy_config_get_auto_config_url();
    if (!auto_config_url) {
        proxy_resolver->error = EINVAL;
        log_error("Auto configuration url not specified");
        goto mac_done;
    }

    url_ref = CFURLCreateWithBytes(NULL, (const UInt8 *)auto_config_url, strlen(auto_config_url), kCFStringEncodingUTF8,
                                   NULL);

    if (!url_ref) {
        proxy_resolver->error = ENOMEM;
        log_error("Unable to allocate memory for %s (%" PRId64 ")", "auto config url reference", proxy_resolver->error);
        goto mac_done;
    }

    target_url_ref = CFURLCreateWithBytes(NULL, (const UInt8 *)url, strlen(url), kCFStringEncodingUTF8, NULL);
    if (!target_url_ref) {
        proxy_resolver->error = ENOMEM;
        log_error("Unable to allocate memory for %s (%" PRId64 ")", "target url reference", proxy_resolver->error);
        goto mac_done;
    }

    context.info = proxy_resolver;

    run_loop = CFNetworkExecuteProxyAutoConfigurationURL(url_ref, target_url_ref,
                                                         proxy_resolver_mac_auto_config_result_callback, &context);
    if (!run_loop) {
        proxy_resolver->error = ELOOP;
        log_error("Failed to execute pac url (%" PRId64 ")", proxy_resolver->error);
        goto mac_done;
    }

#ifdef LEAK_SANITIZER
    // There is a known issue mentioned in Chromium source, that the run loop instance
    // returned by CFNetworkExecuteProxyAutoConfigurationURL leaks.

    // Additionally it is discussed on these forums:
    //  http://www.openradar.appspot.com/20974299
    //  https://forums.developer.apple.com/forums/thread/724883
    //  https://stackoverflow.com/questions/53290871

    __lsan_ignore_object(run_loop);
#endif

    CFRunLoopAddSource(CFRunLoopGetCurrent(), run_loop, PROXY_RESOLVER_RUN_LOOP);
    CFRunLoopRunInMode(PROXY_RESOLVER_RUN_LOOP, PROXY_RESOLVER_TIMEOUT_SEC, false);
    CFRunLoopRemoveSource(CFRunLoopGetCurrent(), run_loop, PROXY_RESOLVER_RUN_LOOP);
    CFRunLoopSourceInvalidate(run_loop);
    CFRelease(run_loop);

mac_done:

    is_ok = proxy_resolver->error == 0;
    event_set(proxy_resolver->complete);

    free(auto_config_url);

    if (url_ref)
        CFRelease(url_ref);
    if (target_url_ref)
        CFRelease(target_url_ref);

    return is_ok;
}
const char *proxy_resolver_mac_get_list(void *ctx) {
    proxy_resolver_mac_s *proxy_resolver = (proxy_resolver_mac_s *)ctx;
    if (!proxy_resolver)
        return NULL;
    return proxy_resolver->list;
}

int32_t proxy_resolver_mac_get_error(void *ctx) {
    proxy_resolver_mac_s *proxy_resolver = (proxy_resolver_mac_s *)ctx;
    return (int32_t)proxy_resolver->error;
}

bool proxy_resolver_mac_wait(void *ctx, int32_t timeout_ms) {
    proxy_resolver_mac_s *proxy_resolver = (proxy_resolver_mac_s *)ctx;
    if (!proxy_resolver)
        return false;
    return event_wait(proxy_resolver->complete, timeout_ms);
}

bool proxy_resolver_mac_cancel(void *ctx) {
    return false;
}

void *proxy_resolver_mac_create(void) {
    proxy_resolver_mac_s *proxy_resolver = (proxy_resolver_mac_s *)calloc(1, sizeof(proxy_resolver_mac_s));
    if (!proxy_resolver)
        return NULL;
    proxy_resolver->complete = event_create();
    if (!proxy_resolver->complete) {
        free(proxy_resolver);
        return NULL;
    }
    return proxy_resolver;
}

bool proxy_resolver_mac_delete(void **ctx) {
    if (!ctx)
        return false;
    proxy_resolver_mac_s *proxy_resolver = (proxy_resolver_mac_s *)*ctx;
    if (!proxy_resolver)
        return false;
    proxy_resolver_mac_cancel(ctx);
    event_delete(&proxy_resolver->complete);
    free(proxy_resolver->list);
    free(proxy_resolver);
    return true;
}

bool proxy_resolver_mac_global_init(void) {
    return true;
}

bool proxy_resolver_mac_global_cleanup(void) {
    memset(&g_proxy_resolver_mac, 0, sizeof(g_proxy_resolver_mac));
    return true;
}

const proxy_resolver_i_s *proxy_resolver_mac_get_interface(void) {
    static const proxy_resolver_i_s proxy_resolver_mac_i = {
        proxy_resolver_mac_get_proxies_for_url,
        proxy_resolver_mac_get_list,
        proxy_resolver_mac_get_error,
        proxy_resolver_mac_wait,
        proxy_resolver_mac_cancel,
        proxy_resolver_mac_create,
        proxy_resolver_mac_delete,
        false,  // get_proxies_for_url should be spooled to another thread
        false,  // get_proxies_for_url does not take into account system config
        proxy_resolver_mac_global_init,
        proxy_resolver_mac_global_cleanup};
    return &proxy_resolver_mac_i;
}
