#pragma once

bool proxy_resolver_win8_get_proxies_for_url(void *ctx, const char *url);
const char *proxy_resolver_win8_get_list(void *ctx);
int32_t proxy_resolver_win8_get_error(void *ctx);
bool proxy_resolver_win8_wait(void *ctx, int32_t timeout_ms);
bool proxy_resolver_win8_cancel(void *ctx);

void *proxy_resolver_win8_create(void);
bool proxy_resolver_win8_delete(void **ctx);

bool proxy_resolver_win8_global_init(void);
bool proxy_resolver_win8_global_cleanup(void);

const proxy_resolver_i_s *proxy_resolver_win8_get_interface(void);
