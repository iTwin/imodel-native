#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <windows.h>
#include <wininet.h>

#include "config.h"
#include "config_i.h"
#include "config_win.h"

#include "util.h"
#include "util_win.h"

// Use WinInet to retrieve user's proxy configuration because WinHttpGetIEProxyConfigForCurrentUser
// has a network dependency that causes it to be slow https://stackoverflow.com/questions/2151462/.

bool proxy_config_win_get_auto_discover(void) {
    INTERNET_PER_CONN_OPTIONW options[1] = {{INTERNET_PER_CONN_FLAGS_UI, {0}}};
    INTERNET_PER_CONN_OPTION_LISTW option_list = {sizeof(INTERNET_PER_CONN_OPTION_LISTW), NULL, 1, 0, options};
    DWORD option_list_size = sizeof(option_list);
    bool auto_discover = false;

    // INTERNET_PER_CONN_FLAGS hides auto-detection setting if it believes the network does not have WPAD. To get
    // the actual value we need to query using INTERNET_PER_CONN_FLAGS_UI https://stackoverflow.com/questions/15565997

    if (!InternetQueryOptionW(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &option_list, &option_list_size)) {
        options[0].dwOption = INTERNET_PER_CONN_FLAGS;
        if (!InternetQueryOptionW(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &option_list, &option_list_size))
            return auto_discover;
    }
    auto_discover = options[0].Value.dwValue & PROXY_TYPE_AUTO_DETECT;
    return auto_discover;
}

char *proxy_config_win_get_auto_config_url(void) {
    INTERNET_PER_CONN_OPTIONW options[2] = {{INTERNET_PER_CONN_FLAGS, {0}}, {INTERNET_PER_CONN_AUTOCONFIG_URL, {0}}};
    INTERNET_PER_CONN_OPTION_LISTW option_list = {sizeof(INTERNET_PER_CONN_OPTION_LISTW), NULL, 2, 0, options};
    DWORD option_list_size = sizeof(option_list);
    char *auto_config_url = NULL;

    if (InternetQueryOptionW(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &option_list, &option_list_size)) {
        if (options[1].Value.pszValue) {
            if (options[0].Value.dwValue & PROXY_TYPE_AUTO_PROXY_URL && *options[1].Value.pszValue)
                auto_config_url = wchar_dup_to_utf8(options[1].Value.pszValue);
            GlobalFree(options[1].Value.pszValue);
        }
    }

    return auto_config_url;
}

char *proxy_config_win_get_proxy(const char *scheme) {
    INTERNET_PER_CONN_OPTIONW options[2] = {{INTERNET_PER_CONN_FLAGS, {0}}, {INTERNET_PER_CONN_PROXY_SERVER, {0}}};
    INTERNET_PER_CONN_OPTION_LISTW option_list = {sizeof(INTERNET_PER_CONN_OPTION_LISTW), NULL, 2, 0, options};
    DWORD option_list_size = sizeof(option_list);
    char *proxy_list = NULL;

    if (InternetQueryOptionW(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &option_list, &option_list_size)) {
        if (options[1].Value.pszValue) {
            if (options[0].Value.dwValue & PROXY_TYPE_PROXY && *options[1].Value.pszValue)
                proxy_list = wchar_dup_to_utf8(options[1].Value.pszValue);
            GlobalFree(options[1].Value.pszValue);
        }
    }

    if (!proxy_list)
        return NULL;

    // Proxy may be returned as a list of proxies
    char *proxy = get_winhttp_proxy_by_scheme(scheme, proxy_list);
    free(proxy_list);
    return proxy;
}

char *proxy_config_win_get_bypass_list(void) {
    INTERNET_PER_CONN_OPTIONW options[2] = {{INTERNET_PER_CONN_FLAGS, {0}}, {INTERNET_PER_CONN_PROXY_BYPASS, {0}}};
    INTERNET_PER_CONN_OPTION_LISTW option_list = {sizeof(INTERNET_PER_CONN_OPTION_LISTW), NULL, 2, 0, options};
    DWORD option_list_size = sizeof(option_list);
    char *list = NULL;

    if (InternetQueryOptionW(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &option_list, &option_list_size)) {
        if (options[1].Value.pszValue) {
            if (options[0].Value.dwValue & PROXY_TYPE_PROXY && *options[1].Value.pszValue)
                list = wchar_dup_to_utf8(options[1].Value.pszValue);
            GlobalFree(options[1].Value.pszValue);
        }
    }

    // Normalize separators for all platforms to comma
    if (list)
        str_change_chr(list, ';', ',');

    return list;
}

bool proxy_config_win_global_init(void) {
    return true;
}

bool proxy_config_win_global_cleanup(void) {
    return true;
}

proxy_config_i_s *proxy_config_win_get_interface(void) {
    static proxy_config_i_s proxy_config_win_i = {
        proxy_config_win_get_auto_discover, proxy_config_win_get_auto_config_url, proxy_config_win_get_proxy,
        proxy_config_win_get_bypass_list,   proxy_config_win_global_init,         proxy_config_win_global_cleanup};
    return &proxy_config_win_i;
}
