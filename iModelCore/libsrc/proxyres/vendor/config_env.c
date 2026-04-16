#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "config.h"
#include "config_i.h"
#include "config_env.h"
#include "util.h"

// Get the environment variable using all lowercase name and then all uppercase name
// https://unix.stackexchange.com/questions/212894

static char *get_proxy_env_var(const char *name, bool check_uppercase) {
    // Check original lowercase environment variable name passed in
    char *proxy = getenv(name);

    if (proxy) {
        if (*proxy)
            return proxy;
        free(proxy);
    }

    if (check_uppercase) {
        // Allocate a copy of the environment variable name
        char *upper_name = strdup(name);
        if (!upper_name)
            return NULL;

        // Convert string to all uppercase
        for (size_t i = 0; i < strlen(upper_name); i++)
            upper_name[i] = toupper(upper_name[i]);

        // Check uppercase environment variable name
        proxy = getenv(upper_name);
        free(upper_name);
        if (proxy && !*proxy) {
            free(proxy);
            proxy = NULL;
        }
        return proxy;
    }
    return NULL;
}

bool proxy_config_env_get_auto_discover(void) {
    return false;
}

char *proxy_config_env_get_auto_config_url(void) {
    return NULL;
}

char *proxy_config_env_get_proxy(const char *scheme) {
    if (!scheme)
        return NULL;

    // Construct name of environment variable based on proxy scheme
    size_t name_len = strlen(scheme) + 8;
    char *name = (char *)malloc(name_len);
    if (!name)
        return NULL;
    snprintf(name, name_len, "%s_proxy", scheme);

    // Don't check HTTP_PROXY due to CGI environment variable creation
    // https://everything.curl.dev/usingcurl/proxies/env
    bool check_uppercase = strcmp(scheme, "http") != 0;

    char *proxy = get_proxy_env_var(name, check_uppercase);
    free(name);
    if (!proxy)
        return NULL;

    return strdup(proxy);
}

char *proxy_config_env_get_bypass_list(void) {
    const char *no_proxy = get_proxy_env_var("no_proxy", true);
    if (!no_proxy)
        return NULL;

    char *bypass_list = strdup(no_proxy);
    if (!bypass_list)
        return NULL;

    // Remove last separator
    str_trim_end(bypass_list, ',');
    return bypass_list;
}

bool proxy_config_env_global_init(void) {
    return true;
}
bool proxy_config_env_global_cleanup(void) {
    return true;
}

proxy_config_i_s *proxy_config_env_get_interface(void) {
    static proxy_config_i_s proxy_config_env_i = {
        proxy_config_env_get_auto_discover, proxy_config_env_get_auto_config_url, proxy_config_env_get_proxy,
        proxy_config_env_get_bypass_list,   proxy_config_env_global_init,         proxy_config_env_global_cleanup};
    return &proxy_config_env_i;
}
