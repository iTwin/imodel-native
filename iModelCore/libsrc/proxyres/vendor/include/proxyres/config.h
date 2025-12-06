#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Read whether WPAD is enabled on the user's system.
bool proxy_config_get_auto_discover(void);

// Read the proxy auto config (PAC) url configured on the user's system.
char *proxy_config_get_auto_config_url(void);

// Read the proxy configured on the user's system for a given URL scheme.
char *proxy_config_get_proxy(const char *scheme);

// Read the proxy bypass list configured on the user's system.
char *proxy_config_get_bypass_list(void);

// Override the user's configured proxy auto configuration (PAC) url.
void proxy_config_set_auto_config_url_override(const char *auto_config_url);

// Override the user's configured proxy.
void proxy_config_set_proxy_override(const char *proxy);

// Override the user's configured proxy bypass list.
void proxy_config_set_bypass_list_override(const char *bypass_list);

// Initialization function for reading user's proxy configuration.
bool proxy_config_global_init(void);

// Uninitialization function for reading user's proxy configuration.
bool proxy_config_global_cleanup(void);

#ifdef __cplusplus
}
#endif
