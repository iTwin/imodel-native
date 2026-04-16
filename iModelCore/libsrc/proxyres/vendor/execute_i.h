#pragma once

typedef struct proxy_execute_i_s {
    bool (*get_proxies_for_url)(void *ctx, const char *script, const char *url);

    const char *(*get_list)(void *ctx);
    int32_t (*get_error)(void *ctx);

    void *(*create)(void);
#ifdef __cplusplus
    bool (*f_delete)(void **ctx);
#else
    bool (*delete)(void **ctx);
#endif

    bool (*global_init)(void);
    bool (*global_cleanup)(void);
} proxy_execute_i_s;
