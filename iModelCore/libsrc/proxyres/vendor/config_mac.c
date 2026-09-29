#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <CoreFoundation/CoreFoundation.h>
#include <CFNetwork/CFNetwork.h>
#include <TargetConditionals.h>

#include "config.h"
#include "config_i.h"
#include "config_mac.h"
#include "resolver.h"
#include "util.h"

static bool get_cf_dictionary_bool(CFDictionaryRef dictionary, CFStringRef key) {
    CFNumberRef item = NULL;
    int value = 0;

    return CFDictionaryGetValueIfPresent(dictionary, key, (const void **)&item) &&
        CFNumberGetValue(item, kCFNumberIntType, &value) && value;
}

bool proxy_config_mac_get_auto_discover(void) {
#if !TARGET_OS_IPHONE
    bool auto_discover = false;
    CFDictionaryRef proxy_settings = CFNetworkCopySystemProxySettings();
    if (!proxy_settings)
        return false;

    // Get whether or not auto-discovery is enabled
    if (get_cf_dictionary_bool(proxy_settings, kCFNetworkProxiesProxyAutoDiscoveryEnable))
        auto_discover = true;

    CFRelease(proxy_settings);
    return auto_discover;
#else
    return false;
#endif
}

char *proxy_config_mac_get_auto_config_url(void) {
    char *url = NULL;

    CFDictionaryRef proxy_settings = CFNetworkCopySystemProxySettings();
    if (!proxy_settings)
        return NULL;

    // Check if auto-config url is enabled
    if (get_cf_dictionary_bool(proxy_settings, kCFNetworkProxiesProxyAutoConfigEnable)) {
        // Get the auto-config url
        const CFStringRef auto_config_url =
            (CFStringRef)CFDictionaryGetValue(proxy_settings, kCFNetworkProxiesProxyAutoConfigURLString);
        if (auto_config_url) {
            const char *auto_config_url_utf8 = CFStringGetCStringPtr(auto_config_url, kCFStringEncodingUTF8);
            if (auto_config_url_utf8) {
                url = strdup(auto_config_url_utf8);
            } else {
                url = (char *)calloc(MAX_PROXY_URL, sizeof(char));
                CFStringGetCString(auto_config_url, url, MAX_PROXY_URL, kCFStringEncodingUTF8);
            }
        }
    }

    // Don't return empty url
    if (url && !*url) {
        free(url);
        url = NULL;
    }

    CFRelease(proxy_settings);
    return url;
}

char *proxy_config_mac_get_proxy(const char *scheme) {
    char *proxy = NULL;
    size_t max_proxy = 0;

    // Determine the indexes to retrieve from the system proxy settings to get the value
    // for the proxy list we want
    CFStringRef enable_index = kCFNetworkProxiesHTTPEnable;
    CFStringRef host_index = kCFNetworkProxiesHTTPProxy;
    CFStringRef port_index = kCFNetworkProxiesHTTPPort;

#if !TARGET_OS_IPHONE
    if (strncasecmp(scheme, "https", 5) == 0) {
        enable_index = kCFNetworkProxiesHTTPSEnable;
        host_index = kCFNetworkProxiesHTTPSProxy;
        port_index = kCFNetworkProxiesHTTPSPort;
    } else if (strncasecmp(scheme, "socks", 5) == 0) {
        enable_index = kCFNetworkProxiesSOCKSEnable;
        host_index = kCFNetworkProxiesSOCKSProxy;
        port_index = kCFNetworkProxiesSOCKSPort;
    } else if (strncasecmp(scheme, "ftp", 3) == 0) {
        enable_index = kCFNetworkProxiesFTPEnable;
        host_index = kCFNetworkProxiesFTPProxy;
        port_index = kCFNetworkProxiesFTPPort;
    } else if (strncasecmp(scheme, "rtsp", 4) == 0) {
        enable_index = kCFNetworkProxiesRTSPEnable;
        host_index = kCFNetworkProxiesRTSPProxy;
        port_index = kCFNetworkProxiesRTSPPort;
    }
#endif

    CFDictionaryRef proxy_settings = CFNetworkCopySystemProxySettings();
    if (!proxy_settings)
        return NULL;

    if (get_cf_dictionary_bool(proxy_settings, enable_index)) {
        // Get the proxy url associated with the scheme
        const CFStringRef host = (CFStringRef)CFDictionaryGetValue(proxy_settings, host_index);
        if (host) {
            const char *host_utf8 = CFStringGetCStringPtr(host, kCFStringEncodingUTF8);
            if (host_utf8) {
                max_proxy = strlen(host_utf8) + 32;  // Allow enough room for port number
                proxy = (char *)calloc(max_proxy, sizeof(char));
                strncat(proxy, host_utf8, max_proxy - 1);
            } else {
                max_proxy = MAX_PROXY_URL;
                proxy = (char *)calloc(max_proxy, sizeof(char));
                CFStringGetCString(host, proxy, max_proxy, kCFStringEncodingUTF8);
            }
        }

        // Get the proxy port associated with the scheme
        const CFNumberRef port = (CFNumberRef)CFDictionaryGetValue(proxy_settings, port_index);
        if (proxy && port) {
            // Append the proxy port to the proxy url
            int64_t port_number = 0;
            CFNumberGetValue(port, kCFNumberSInt64Type, &port_number);
            size_t proxy_len = strlen(proxy);
            snprintf(proxy + proxy_len, max_proxy - proxy_len, ":%" PRId64 "", port_number);
        }
    }

    // Don't return empty proxy
    if (proxy && !*proxy) {
        free(proxy);
        proxy = NULL;
    }

    CFRelease(proxy_settings);
    return proxy;
}

char *proxy_config_mac_get_bypass_list(void) {
    char *bypass_list = NULL;
    size_t bypass_list_len = 0;
    size_t bypass_list_count = 0;
    size_t exception_count = 0;
    size_t max_bypass_list = 0;

    CFDictionaryRef proxy_settings = CFNetworkCopySystemProxySettings();
    if (!proxy_settings)
        return NULL;

    // Get whether to exclude simple hostnames
    bool exclude_simple_hostnames = false;
#if !TARGET_OS_IPHONE
    exclude_simple_hostnames = get_cf_dictionary_bool(proxy_settings, kCFNetworkProxiesExcludeSimpleHostnames);
    if (exclude_simple_hostnames)
        bypass_list_count++;
#endif
    // Get exception list
    CFArrayRef exceptions_list = NULL;
#if !TARGET_OS_IPHONE
    exceptions_list = (CFArrayRef)CFDictionaryGetValue(proxy_settings, kCFNetworkProxiesExceptionsList);
    if (exceptions_list) {
        exception_count = CFArrayGetCount(exceptions_list);
        bypass_list_count += exception_count;
    }
#endif

    if (!bypass_list_count)
        goto bypass_list_error;

    // Allocate memory to copy bypass list
    max_bypass_list = bypass_list_count * MAX_PROXY_URL + 1;
    bypass_list = (char *)calloc(max_bypass_list, sizeof(char));

    if (!bypass_list)
        goto bypass_list_error;

    // Add exclusion for simple hostnames to bypass list
    if (exclude_simple_hostnames) {
        strncat(bypass_list, "<local>,", max_bypass_list - bypass_list_len - 1);
        bypass_list_len += 8;
    }

    // Enumerate exception array and copy to comma delimited string
    for (size_t i = 0; exceptions_list && i < exception_count; ++i) {
        const CFStringRef exception = (CFStringRef)CFArrayGetValueAtIndex(exceptions_list, i);
        if (exception) {
            const char *exception_utf8 = CFStringGetCStringPtr(exception, kCFStringEncodingUTF8);
            if (exception_utf8) {
                snprintf(bypass_list + bypass_list_len, max_bypass_list - bypass_list_len, "%s", exception_utf8);
                bypass_list_len += strlen(exception_utf8);
            } else {
                CFStringGetCString(exception, bypass_list + bypass_list_len, max_bypass_list - bypass_list_len,
                                   kCFStringEncodingUTF8);
                bypass_list_len = strlen(bypass_list);
            }

            // Append comma separation
            strncat(bypass_list, ",", max_bypass_list - bypass_list_len - 1);
            bypass_list_len++;
        }
    }

    // Remove last separator
    if (bypass_list)
        str_trim_end(bypass_list, ',');

bypass_list_error:

    // Don't return empty bypass list
    if (bypass_list && !*bypass_list) {
        free(bypass_list);
        bypass_list = NULL;
    }

    CFRelease(proxy_settings);
    return bypass_list;
}

bool proxy_config_mac_global_init(void) {
    return true;
}

bool proxy_config_mac_global_cleanup(void) {
    return true;
}

proxy_config_i_s *proxy_config_mac_get_interface(void) {
    static proxy_config_i_s proxy_config_mac_i = {
        proxy_config_mac_get_auto_discover, proxy_config_mac_get_auto_config_url, proxy_config_mac_get_proxy,
        proxy_config_mac_get_bypass_list,   proxy_config_mac_global_init,         proxy_config_mac_global_cleanup};
    return &proxy_config_mac_i;
}
