
#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MAX_PROXY_URL 256

#ifdef __cplusplus
extern "C" {
#endif

// Asynchronously resolves the proxies for a given URL based on the user's proxy configuration.
bool proxy_resolver_get_proxies_for_url(void *ctx, const char *url);

// Gets the list of proxies that have been resolved.
const char *proxy_resolver_get_list(void *ctx);

// Gets the next proxy in the list of proxies that have been resolved.
char *proxy_resolver_get_next_proxy(void *ctx);

// Error code for proxy resolution process.
int32_t proxy_resolver_get_error(void *ctx);

// Waits for the proxy resolution process to complete.
bool proxy_resolver_wait(void *ctx, int32_t timeout_ms);

// Cancel any pending proxy resolution.
bool proxy_resolver_cancel(void *ctx);

// Create a proxy resolver instance.
void *proxy_resolver_create(void);

// Deletes a proxy resolver instance.
bool proxy_resolver_delete(void **ctx);

// Initialization function for proxy resolution.
bool proxy_resolver_global_init(void);

// Uninitialization function for proxy resolution.
bool proxy_resolver_global_cleanup(void);

#ifdef __cplusplus
}
#endif
