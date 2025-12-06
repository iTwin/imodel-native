#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>
#else
#  include <arpa/inet.h>
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netdb.h>
#  include <unistd.h>
#endif

#include "net_adapter.h"
#include "net_util.h"
#include "util.h"

typedef struct address_list {
    int32_t family;
    int32_t max_addrs;
    char *string;
    size_t string_len;
    size_t max_string;
} address_list;

// Calculate the max length of the address string
static bool my_ip_address_list_length(void *user_data, net_adapter_s *adapter) {
    address_list *list = (address_list *)user_data;
    // Use different length depending on the address type
    list->max_string += ((list->family == AF_INET) ? INET_ADDRSTRLEN : INET6_ADDRSTRLEN);
    // Add room for semi-colon separators
    list->max_string += 2;
    return true;
}

// Copy each localhost address into a string buffer seperated by semi-colons
static bool my_ip_address_list_populate(void *user_data, net_adapter_s *adapter) {
    address_list *list = (address_list *)user_data;

    if (!list->max_addrs || !adapter->is_connected || !*adapter->ip)
        return true;

#ifdef _WIN32
    if (!adapter->is_dhcp_v4)
        return true;
#endif

    if (list->family == AF_INET || list->family == AF_UNSPEC) {
        char ip_str[INET_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET, adapter->ip, ip_str, sizeof(ip_str));
        strncat(list->string + list->string_len, ip_str, list->max_string - list->string_len - 1);
        list->string_len += strlen(ip_str);
    }

    if (adapter->is_ipv6 && (list->family == AF_INET6 || list->family == AF_UNSPEC)) {
        // Append semi-colon separator
        if (list->max_string - list->string_len > 1) {
            list->string[list->string_len++] = ';';
            list->string[list->string_len] = 0;
        }

        char ipv6_str[INET6_ADDRSTRLEN] = {0};
        inet_ntop(AF_INET6, adapter->ipv6, ipv6_str, sizeof(ipv6_str));
        strncat(list->string + list->string_len, ipv6_str, list->max_string - list->string_len - 1);
        list->string_len += strlen(ipv6_str);
    }

    list->max_addrs--;

    // Append semi-colon separator
    if (list->max_addrs && list->max_string - list->string_len > 1) {
        list->string[list->string_len++] = ';';
        list->string[list->string_len] = 0;
    }
    return true;
}

// Enumerate network adapters and get localhost addresses with a filter
static char *my_ip_address_filter(int32_t family, int32_t max_addrs) {
    address_list list = {family, max_addrs, NULL, 0, 1};

    if (!net_adapter_enum(&list, my_ip_address_list_length))
        return NULL;

    // Allocate buffer for the return string
    list.string = (char *)calloc(1, list.max_string);
    if (list.string) {
        if (!net_adapter_enum(&list, my_ip_address_list_populate)) {
            free(list.string);
            list.string = NULL;
        }
    }

    return list.string;
}

// Get local IPv4 address for localhost
char *my_ip_address(void) {
    return my_ip_address_filter(AF_INET, 1);
}

// Get local IPv6 and IPv6 addresses for localhost
char *my_ip_address_ex(void) {
    return my_ip_address_filter(AF_UNSPEC, UINT8_MAX);
}

// Resolve a host name to its addresses with a filter and custom separator
static char *dns_resolve_filter(const char *host, int32_t family, uint8_t max_addrs, int32_t *error) {
    address_list list = {family, max_addrs, NULL, 0, 1};
    struct addrinfo hints = {0};
    struct addrinfo *address_info = NULL;
    struct addrinfo *address = NULL;
    int32_t err = 0;

    if (!host) {
        if (error)
            *error = EINVAL;
        return NULL;
    }

    hints.ai_family = family;
    hints.ai_socktype = SOCK_STREAM;

    err = getaddrinfo(host, NULL, &hints, &address_info);
    if (err != 0)
        goto dns_resolve_error;

    // Calculate the length of the return string
    address = address_info;
    while (address) {
        // Use different length depending on the address type
        list.max_string += (address->ai_family == AF_INET) ? INET_ADDRSTRLEN : INET6_ADDRSTRLEN;
        // Add room for semi-colon separator
        list.max_string++;
        address = address->ai_next;
    }

    // Allocate buffer for the return string
    list.string = (char *)calloc(1, list.max_string);
    if (!list.string)
        goto dns_resolve_error;

    // Enumerate each address
    address = address_info;
    while (address && list.max_addrs) {
        // Only copy addresses that match the family filter
        if (list.family == AF_UNSPEC || address->ai_family == list.family) {
            // Ensure there is room to copy something into return string buffer
            if (list.string_len >= list.max_string)
                break;

            // Convert address name into a numeric host string
            err = getnameinfo(address->ai_addr, (socklen_t)address->ai_addrlen, list.string + list.string_len,
                              (uint32_t)(list.max_string - list.string_len), NULL, 0, NI_NUMERICHOST);
            if (err != 0) {
                continue;
            }

            list.max_addrs--;

            // Append semi-colon separator
            list.string_len = strlen(list.string);
            if (list.max_addrs && address->ai_next && list.string_len + 1 < list.max_string) {
                list.string[list.string_len++] = ';';
                list.string[list.string_len] = 0;
            }
        }

        address = address->ai_next;
    }

    if (err != 0 && list.string_len == 0)
        goto dns_resolve_error;

    return list.string;

dns_resolve_error:

    free(list.string);

    if (address_info)
        freeaddrinfo(address_info);

    if (error != NULL)
        *error = err;

    return NULL;
}

// Resolve a host name to it an IPv4 address
char *dns_resolve(const char *host, int32_t *error) {
    return dns_resolve_filter(host, AF_INET, 1, error);
}

// Resolve a host name to its addresses
char *dns_resolve_ex(const char *host, int32_t *error) {
    return dns_resolve_filter(host, AF_UNSPEC, UINT8_MAX, error);
}

#if _WIN32_WINNT < _WIN32_WINNT_VISTA
// Backwards compatible inet_pton for Windows XP
int32_t inet_pton(int32_t af, const char *src, void *dst) {
    struct sockaddr_storage sock_storage;
    int32_t size = sizeof(sockaddr_storage);
    char src_copy[INET6_ADDRSTRLEN + 1];

    memset(&sock_storage, 0, sizeof(sock_storage));
    strncpy(src_copy, src, INET6_ADDRSTRLEN + 1);
    src_copy[INET6_ADDRSTRLEN] = 0;

    if (WSAStringToAddress(src_copy, af, NULL, (struct sockaddr *)&sock_storage, &size) == 0) {
        switch (af) {
        case AF_INET:
            *(struct in_addr *)dst = ((struct sockaddr_in *)&ss)->sin_addr;
            return 1;
        case AF_INET6:
            *(struct in6_addr *)dst = ((struct sockaddr_in6 *)&ss)->sin6_addr;
            return 1;
        }
    }
    return 0;
}
#endif

// Check if the ipv4 address matches the cidr notation range
bool is_ipv4_in_cidr_range(const char *ip, const char *cidr) {
    if (!ip || !cidr)
        return false;

    // Convert ip from text to binary
    struct in_addr ip_addr;
    if (!inet_pton(AF_INET, ip, &ip_addr))
        return false;

    // Parse cidr notation
    char *cidr_ip = strdup(cidr);
    char *cidr_prefix = strchr(cidr_ip, '/');
    if (!cidr_prefix) {
        free(cidr_ip);
        return false;
    }
    *cidr_prefix = 0;
    cidr_prefix++;

    // Parse cidr prefix
    int32_t prefix = atoi(cidr_prefix);
    if (prefix < 0 || prefix > 32) {
        free(cidr_ip);
        return false;
    }

    // Convert cidr ip from text to binary
    struct in_addr cidr_addr;
    if (!inet_pton(AF_INET, cidr_ip, &cidr_addr)) {
        free(cidr_ip);
        return false;
    }
    free(cidr_ip);

    // Check if ip address is in cidr range
    uint32_t ip_int = ntohl(ip_addr.s_addr);
    uint32_t cidr_int = ntohl(cidr_addr.s_addr);
    uint32_t mask = prefix >= 32 ? 0xFFFFFFFFu : ~(0xFFFFFFFFu >> prefix);

    return (ip_int & mask) == (cidr_int & mask);
}

// Check if the ipv6 address matches the cidr notation range
bool is_ipv6_in_cidr_range(const char *ip, const char *cidr) {
    if (!ip || !cidr)
        return false;

    // Convert ip from text to binary
    struct in6_addr ip_addr;
    if (!inet_pton(AF_INET6, ip, &ip_addr))
        return false;

    // Parse cidr notation
    char *cidr_ip = strdup(cidr);
    char *cidr_prefix = strchr(cidr_ip, '/');
    if (!cidr_prefix) {
        free(cidr_ip);
        return false;
    }
    *cidr_prefix = 0;
    cidr_prefix++;

    // Parse cidr prefix
    int32_t prefix = atoi(cidr_prefix);
    if (prefix < 0 || prefix > 128) {
        free(cidr_ip);
        return false;
    }

    // Convert cidr ip from text to binary
    struct in6_addr cidr_addr;
    if (!inet_pton(AF_INET6, cidr_ip, &cidr_addr)) {
        free(cidr_ip);
        return false;
    }
    free(cidr_ip);

    // Check if ip address is in cidr range
    uint8_t *ip_data = (uint8_t *)&ip_addr.s6_addr;
    uint8_t *cidr_data = (uint8_t *)&cidr_addr.s6_addr;

    // Compare leading bytes of address
    int32_t check_bytes = prefix / 8;
    if (check_bytes) {
        if (memcmp(ip_data, cidr_data, check_bytes))
            return false;
    }

    // Check remaining bits of address
    int32_t check_bits = prefix & 0x07;
    if (!check_bits)
        return true;

    uint8_t mask = (0xff << (8 - check_bits));
    return ((ip_data[check_bytes] ^ cidr_data[check_bytes]) & mask) == 0;
}
