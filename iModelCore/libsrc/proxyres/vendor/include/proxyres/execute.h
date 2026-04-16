#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Executes a PAC script for a particular URL.
bool proxy_execute_get_proxies_for_url(void *ctx, const char *script, const char *url);

// Get the list of proxies returned by the call to `FindProxyForURL`.
const char *proxy_execute_get_list(void *ctx);

// Error code for script execution.
int32_t proxy_execute_get_error(void *ctx);

// Create new PAC script execution instance.
void *proxy_execute_create(void);

// Delete a PAC script execution instance.
bool proxy_execute_delete(void **ctx);

// Initialization function for PAC script execution.
bool proxy_execute_global_init(void);

// Uninitialization function for PAC script execution.
bool proxy_execute_global_cleanup(void);

#ifdef __cplusplus
}
#endif
