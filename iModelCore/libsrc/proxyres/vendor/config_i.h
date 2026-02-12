#pragma once

typedef struct proxy_config_i_s {
    bool (*auto_discover)(void);
    char *(*get_auto_config_url)(void);
    char *(*get_proxy)(const char *scheme);
    char *(*get_bypass_list)(void);

    bool (*global_init)(void);
    bool (*global_cleanup)(void);
} proxy_config_i_s;
