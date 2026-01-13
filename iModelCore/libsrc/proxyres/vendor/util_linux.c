#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>

#include "util_linux.h"

int32_t get_desktop_env(void) {
    const char *current_desktop = getenv("XDG_CURRENT_DESKTOP");  // Since 2012
    if (current_desktop) {
        if (strcasestr(current_desktop, "gnome") || strcasestr(current_desktop, "unity") ||
            strcasestr(current_desktop, "xfce")) {
            return DESKTOP_ENV_GNOME3;
        } else if (strcasestr(current_desktop, "kde")) {
            const char *session_version = getenv("KDE_SESSION_VERSION");
            if (session_version) {
                if (*session_version > '0' && *session_version <= '4')
                    return DESKTOP_ENV_KDE4;
                return DESKTOP_ENV_KDE5;
            }
        }
    }

    const char *desktop_session = getenv("DESKTOP_SESSION");  // Since 2010
    if (desktop_session) {
        if (strcasestr(desktop_session, "gnome") || strcasestr(desktop_session, "mate")) {
            if (access("/usr/bin/gsettings", F_OK) != -1)
                return DESKTOP_ENV_GNOME3;
            return DESKTOP_ENV_GNOME2;
        } else if (strcasestr(desktop_session, "kde4") || strcasestr(desktop_session, "kde-plasma")) {
            return DESKTOP_ENV_KDE4;
        } else if (strcasestr(desktop_session, "kde")) {
            if (getenv("KDE_SESSION_VERSION"))
                return DESKTOP_ENV_KDE4;
            return DESKTOP_ENV_KDE3;
        }
    }

    if (getenv("GNOME_DESKTOP_SESSION_ID")) {
        if (access("/usr/bin/gsettings", F_OK) != -1)
            return DESKTOP_ENV_GNOME3;
        return DESKTOP_ENV_GNOME2;
    } else if (getenv("KDE_FULL_SESSION")) {
        if (getenv("KDE_SESSION_VERSION"))
            return DESKTOP_ENV_KDE4;
        return DESKTOP_ENV_KDE3;
    }

    return DESKTOP_ENV_UNKNOWN;
}

// Read a value from an ini config file given the section and key
char *get_config_value(const char *config, const char *section, const char *key) {
    size_t max_config = strlen(config);
    int32_t line_len = 0;
    const char *line_start = config;
    bool in_section = true;

    // Read ini file until we find the section and key
    do {
        // Find end of line
        const char *line_end = strchr(line_start, '\n');
        if (!line_end)
            line_end = line_start + strlen(line_start);
        line_len = (int32_t)(line_end - line_start);

        // Check for the key if we are already in the section
        if (in_section) {
            const char *key_start = line_start;
            const char *key_end = strchr(key_start, '=');
            if (key_end) {
                int32_t key_len = (int32_t)(key_end - key_start);
                if (strncmp(key_start, key, key_len) == 0) {
                    // Found key, now make a copy of the value
                    int32_t value_len = line_len - key_len - 1;
                    if (value_len > 0) {
                        char *value = (char *)calloc(value_len + 1, sizeof(char));
                        if (value) {
                            strncpy(value, key_end + 1, value_len);
                            value[value_len] = 0;
                        }
                        return value;
                    }
                }
            }
        }

        // Check if we are in the right section
        if (line_len > 2 && line_start[0] == '[' && line_end[-1] == ']')
            in_section = strncmp(line_start + 1, section, line_len - 2) == 0;

        // Continue to the next line
        line_start = line_end + 1;
    } while (line_start < config + max_config);

    return NULL;
}
