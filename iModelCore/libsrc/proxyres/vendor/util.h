#pragma once

#define HOST_MAX   (260)
#define SCRIPT_MAX (256 * 1024)
#define UNUSED(x)  ((void)x)

#ifdef __cplusplus
extern "C" {
#endif

// Replace one character in the string with another
int32_t str_change_chr(char *str, char from, char to);

// Count number of characters in a string
int32_t str_count_chr(const char *str, char chr);

// Trim a character from the end of the string
int32_t str_trim_end(char *str, char c);

// Find first character in string
const char *str_find_first_char(const char *str, const char *chars);

// Find character in string up to max length
const char *str_find_len_char(const char *str, size_t str_len, char c);

// Find string in string up to max length
const char *str_find_len_str(const char *str, size_t str_len, const char *find);

// Find string case-insensitve in string up to max length
const char *str_find_len_case_str(const char *str, size_t str_len, const char *find);

// Extract and duplicate token from string
char *str_sep_dup(const char **strp, const char *delim);

// Compare a string using wildcard pattern
bool str_wildcard_match(const char *str, const char *pattern, bool ignore_case);

// Find host for a given url
char *get_url_host(const char *url);

// Find path for a given url
const char *get_url_path(const char *url);

// Get the scheme for a given url
char *get_url_scheme(const char *url, const char *default_scheme);

// Create url from host with port
char *get_url_from_host(const char *scheme, const char *host);

// Get port from host
uint16_t get_host_port(const char *host, size_t host_len, uint16_t default_port);

// Strip and parse port from host
uint16_t strip_host_port(char *host, size_t host_len, uint16_t default_port);

// Strip ipv6 brackets from host
bool strip_host_ipv6_brackets(char *host);

// Use scheme based on port specified
const char *get_port_scheme(uint16_t port, const char *default_scheme);

// Get default port for a scheme
uint16_t get_scheme_default_port(const char *scheme);

// Convert proxy list returned by FindProxyForURL to a list of uris separated by commas.
char *convert_proxy_list_to_uri_list(const char *proxy_list, const char *default_scheme);

// Evaluates whether or not the proxy should be bypassed for a given url
bool should_bypass_proxy(const char *url, const char *bypass_list);

#ifdef __cplusplus
}
#endif
