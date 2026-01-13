#pragma once

bool proxy_resolver_gnome3_get_proxies_for_url(void *ctx, const char *url);
const char *proxy_resolver_gnome3_get_list(void *ctx);
int32_t proxy_resolver_gnome3_get_error(void *ctx);
bool proxy_resolver_gnome3_wait(void *ctx, int32_t timeout_ms);
bool proxy_resolver_gnome3_cancel(void *ctx);

void *proxy_resolver_gnome3_create(void);
bool proxy_resolver_gnome3_delete(void **ctx);

bool proxy_resolver_gnome3_global_init(void);
bool proxy_resolver_gnome3_global_cleanup(void);

const proxy_resolver_i_s *proxy_resolver_gnome3_get_interface(void);
