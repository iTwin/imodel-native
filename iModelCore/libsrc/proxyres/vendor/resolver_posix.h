#pragma once

bool proxy_resolver_posix_get_proxies_for_url(void *ctx, const char *url);
bool proxy_resolver_posix_get_proxies_for_url(void *ctx, const char *url);
const char *proxy_resolver_posix_get_list(void *ctx);
int32_t proxy_resolver_posix_get_error(void *ctx);
bool proxy_resolver_posix_wait(void *ctx, int32_t timeout_ms);
bool proxy_resolver_posix_cancel(void *ctx);

void *proxy_resolver_posix_create(void);
bool proxy_resolver_posix_delete(void **ctx);

bool proxy_resolver_posix_global_init(void);
bool proxy_resolver_posix_init_ex(void *threadpool);
bool proxy_resolver_posix_global_cleanup(void);

const proxy_resolver_i_s *proxy_resolver_posix_get_interface(void);
