#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Create a wide char string from a UTF-8 string
wchar_t *utf8_dup_to_wchar(const char *src);

// Create a UTF-8 string from a wide char string
char *wchar_dup_to_utf8(const wchar_t *src);

// Get proxy by scheme from a WinHTTP proxy list.
char *get_winhttp_proxy_by_scheme(const char *scheme, const char *proxy_list);

// Convert WinHTTP proxy list to uri list.
char *convert_winhttp_proxy_list_to_uri_list(const char *proxy_list);

#if _WIN32_WINNT < _WIN32_WINNT_VISTA
int32_t inet_pton(int32_t af, const char *src, void *dst);
#endif

#ifdef __cplusplus
}
#endif
