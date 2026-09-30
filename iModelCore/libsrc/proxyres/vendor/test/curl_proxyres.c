#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#  define strncasecmp _strnicmp
#endif

#include "curl/curl.h"

#include "proxyres/proxyres.h"

static int print_help(void) {
    printf("curl_proxyres [--help] url\n");
    return 1;
}

static int fetch_url_with_proxy(const char *url, const char *proxy) {
    // Create curl easy instance
    CURL *curl_handle = curl_easy_init();
    if (!curl_handle) {
        printf("Unable to initialize curl handle\n");
        return 1;
    }

    printf("Fetching url %s\n", url);
    // Set proxy if specified
    if (proxy && strcmp(proxy, "direct://")) {
        printf("Using proxy %s\n", proxy);
        curl_easy_setopt(curl_handle, CURLOPT_PROXY, proxy);
    } else {
        printf("Using direct connection\n");
    }

    // Setup url to fetch
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);

    // Perform fetch
    CURLcode res = curl_easy_perform(curl_handle);
    if (res != CURLE_OK)
        printf("Failed to fetch url with curl (%s)\n", curl_easy_strerror(res));
    else
        printf("Successful\n");

    curl_easy_cleanup(curl_handle);
    return res;
}

int main(int argc, char *argv[]) {
    if (argc <= 1)
        return print_help();

    if (strcmp(argv[1], "--help") == 0) {
        print_help();
        return 0;
    }

    // Initialize curl library
    CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
    if (res != CURLE_OK)
        return res;

    // Initialize proxy resolver library
    if (!proxyres_global_init()) {
        curl_global_cleanup();
        printf("Unable to initialize proxy resolution library\n");
        return 2;
    }

    // Create proxy resolver
    void *proxy_resolver = proxy_resolver_create();
    if (!proxy_resolver) {
        proxyres_global_cleanup();
        curl_global_cleanup();
        printf("Unable to create proxy resolver\n");
        return 3;
    }

    // Resolve proxies for url
    proxy_resolver_get_proxies_for_url(proxy_resolver, argv[1]);
    // Wait until proxy resolution is complete
    proxy_resolver_wait(proxy_resolver, -1);

    // Enumerate each proxy for url and attempt to fetch
    char *proxy;
    do {
        proxy = proxy_resolver_get_next_proxy(proxy_resolver);
        if (!proxy) {
            printf("No more proxies to try\n");
            break;
        }
        // If proxy is HTTPS, then check to see if curl is built with HTTPS proxy support
        if (strncasecmp(proxy, "https:", 6) == 0) {
            struct curl_version_info_data *version_info = curl_version_info(CURLVERSION_NOW);
            if ((version_info->features & CURL_VERSION_HTTPS_PROXY) == 0) {
                // Remove s from https and attempt HTTP proxy
                memmove(proxy + 4, proxy + 5, strlen(proxy) - 4);
            }
        }
        res = fetch_url_with_proxy(argv[1], proxy);
        free(proxy);
    } while (res != CURLE_OK);

    // Delete proxy resolver instance
    proxy_resolver_delete(&proxy_resolver);

    // Uninitialize proxy resolver library
    proxyres_global_cleanup();

    // Uninitialize curl library
    curl_global_cleanup();
    return 0;
}
