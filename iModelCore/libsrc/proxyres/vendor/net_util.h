#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Get local IPv4 address for localhost
char *my_ip_address(void);

// Get local IPv6 and IPv6 addresses for localhost
char *my_ip_address_ex(void);

// Resolve a host name to it an IPv4 address
char *dns_resolve(const char *host, int32_t *error);

// Resolve a host name to its IPv6 and IPv6 addresses
char *dns_resolve_ex(const char *host, int32_t *error);

// Check if the ipv4 address matches the cidr notation range
bool is_ipv4_in_cidr_range(const char *ip, const char *cidr);

// Check if the ipv6 address matches the cidr notation range
bool is_ipv6_in_cidr_range(const char *ip, const char *cidr);

#ifdef __cplusplus
}
#endif
