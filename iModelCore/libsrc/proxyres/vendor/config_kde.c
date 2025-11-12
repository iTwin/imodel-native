#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include <fcntl.h>
#include <pwd.h>
#include <unistd.h>

#include "config.h"
#include "config_i.h"
#include "config_kde.h"
#include "util.h"
#include "util_linux.h"

typedef struct g_proxy_config_kde_s {
    // User's ini config
    char *config;
} g_proxy_config_kde_s;

g_proxy_config_kde_s g_proxy_config_kde;

enum proxy_type_enum { PROXY_TYPE_NONE, PROXY_TYPE_FIXED, PROXY_TYPE_PAC, PROXY_TYPE_WPAD, PROXY_TYPE_ENV };

static bool check_proxy_type(uint32_t type) {
    char *proxy_type = get_config_value(g_proxy_config_kde.config, "Proxy Settings", "ProxyType");
    if (!proxy_type)
        return false;
    bool is_equal = strtoul(proxy_type, NULL, 0) == type;
    free(proxy_type);
    return is_equal;
}

bool proxy_config_kde_get_auto_discover(void) {
    return check_proxy_type(PROXY_TYPE_WPAD);
}

char *proxy_config_kde_get_auto_config_url(void) {
    if (!check_proxy_type(PROXY_TYPE_PAC))
        return NULL;
    return get_config_value(g_proxy_config_kde.config, "Proxy Settings", "Proxy Config Script");
}

char *proxy_config_kde_get_proxy(const char *scheme) {
    if (!scheme || !check_proxy_type(PROXY_TYPE_FIXED))
        return NULL;

    // Construct key name to search for in config
    size_t scheme_len = strlen(scheme);
    size_t max_key = scheme_len + 8;
    char *key = (char *)calloc(max_key, sizeof(char));
    if (!key)
        return NULL;

    // Check if scheme is actually a url
    const char *host = strchr(scheme, ':');
    if (host) {
        scheme_len = (host - scheme);
        strncat(key, scheme, scheme_len);
    } else {
        strncat(key, scheme, max_key - 1);
    }

    // Scheme should be all lowercase
    for (size_t i = 0; i < scheme_len; i++)
        key[i] = tolower(key[i]);

    // Append "Proxy" to the end of the key
    strncat(key, "Proxy", max_key - scheme_len - 1);

    char *proxy = get_config_value(g_proxy_config_kde.config, "Proxy Settings", key);
    free(key);
    return proxy;
}

char *proxy_config_kde_get_bypass_list(void) {
    return get_config_value(g_proxy_config_kde.config, "Proxy Settings", "NoProxyFor");
}

bool proxy_config_kde_global_init(void) {
    char user_home_path[PATH_MAX];
    char config_path[PATH_MAX];
    int fd = 0;

    // Get the user's home directory
    const char *home_env_var = getenv("KDEHOME");
    if (home_env_var) {
        strncpy(user_home_path, home_env_var, sizeof(user_home_path));
    } else {
        struct passwd *pw = getpwuid(getuid());
        if (!pw)
            return false;

        strncpy(user_home_path, pw->pw_dir, sizeof(user_home_path));
    }

    user_home_path[sizeof(user_home_path) - 1] = 0;

    // Remove trailing slash
    str_trim_end(user_home_path, '/');

    // Get config file path based on desktop environment
    int32_t desktop_env = get_desktop_env();
    switch (desktop_env) {
    case DESKTOP_ENV_KDE3:
        snprintf(config_path, sizeof(config_path), "%s/.kde/share/config/kioslaverc", user_home_path);
        break;
    case DESKTOP_ENV_KDE4:
        snprintf(config_path, sizeof(config_path), "%s/.kde4/share/config/kioslaverc", user_home_path);
        break;
    case DESKTOP_ENV_KDE5:
    default:
        snprintf(config_path, sizeof(config_path), "%s/.config/kioslaverc", user_home_path);
        break;
    }

    // Check if config file exists
    if (access(config_path, F_OK) == -1)
        return false;

    // Open user config file
    fd = open(config_path, O_RDONLY);
    if (fd == -1)
        return false;

    // Create buffer to store config
    int config_size = lseek(fd, 0, SEEK_END);
    g_proxy_config_kde.config = (char *)calloc(config_size + 1, sizeof(char));
    if (!g_proxy_config_kde.config)
        goto kde_init_error;

    // Read config file into buffer
    lseek(fd, 0, SEEK_SET);
    if (read(fd, g_proxy_config_kde.config, config_size) != config_size)
        goto kde_init_error;

    // Use proxy_config_env instead
    if (check_proxy_type(PROXY_TYPE_ENV))
        goto kde_init_error;

    goto kde_ini_cleanup;

kde_init_error:
    proxy_config_kde_global_cleanup();

kde_ini_cleanup:
    if (fd)
        close(fd);

    return g_proxy_config_kde.config;
}

bool proxy_config_kde_global_cleanup(void) {
    free(g_proxy_config_kde.config);

    memset(&g_proxy_config_kde, 0, sizeof(g_proxy_config_kde));
    return true;
}

proxy_config_i_s *proxy_config_kde_get_interface(void) {
    static proxy_config_i_s proxy_config_kde_i = {
        proxy_config_kde_get_auto_discover, proxy_config_kde_get_auto_config_url, proxy_config_kde_get_proxy,
        proxy_config_kde_get_bypass_list,   proxy_config_kde_global_init,         proxy_config_kde_global_cleanup};
    return &proxy_config_kde_i;
}
