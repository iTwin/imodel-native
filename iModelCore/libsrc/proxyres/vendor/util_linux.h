#pragma once

enum DESKTOP_ENV {
    DESKTOP_ENV_UNKNOWN,
    DESKTOP_ENV_KDE3,
    DESKTOP_ENV_KDE4,
    DESKTOP_ENV_KDE5,
    DESKTOP_ENV_GNOME2,
    DESKTOP_ENV_GNOME3
};

#ifdef __cplusplus
extern "C" {
#endif

// Get the desktop environment defined by the process' environment variables
int32_t get_desktop_env(void);

// Retrieve the value for a setting stored in an INI configuration file
char *get_config_value(const char *config, const char *section, const char *key);

#ifdef __cplusplus
}
#endif
