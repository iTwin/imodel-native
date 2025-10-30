#pragma once

typedef struct proxy_resolver_i_s {
    bool (*get_proxies_for_url)(void *ctx, const char *url);

    const char *(*get_list)(void *ctx);
    int32_t (*get_error)(void *ctx);
    bool (*wait)(void *ctx, int32_t timeout_ms);
    bool (*cancel)(void *ctx);

    void *(*create)(void);
#ifdef __cplusplus
    bool (*f_delete)(void **ctx);
#else
    bool (*delete)(void **ctx);
#endif

    bool is_async;
    bool uses_system_config;

    bool (*global_init)(void);
    bool (*global_cleanup)(void);
} proxy_resolver_i_s;
