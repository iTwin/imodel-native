#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef _WIN32
#  include <ws2tcpip.h>
#else
#  include <arpa/inet.h>
#endif

#include "net_adapter.h"

static inline void print_ip(const char *name, uint8_t ip[4]) {
    char ip_str[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, ip, ip_str, sizeof(ip_str));
    printf("  %s: %s\n", name, ip_str);
}

static inline void print_ipv6(const char *name, uint8_t ipv6[16]) {
    char ipv6_str[INET6_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET6, ipv6, ipv6_str, sizeof(ipv6_str));
    printf("  %s: %s\n", name, ipv6_str);
}

void net_adapter_print(net_adapter_s *adapter) {
    printf("adapter: %s\n", adapter->name);
    printf("  description: %s\n", adapter->description);
    if (adapter->mac_length) {
        printf("  mac: ");
        for (int32_t i = 0; i < adapter->mac_length; i++)
            printf("%02x", adapter->mac[i]);
        printf("\n");
    }
    print_ip("ip", adapter->ip);
    print_ip("netmask", adapter->netmask);
    print_ip("gateway", adapter->gateway);
    if (adapter->is_ipv6) {
        print_ipv6("ipv6", adapter->ipv6);
        print_ipv6("netmaskv6", adapter->netmaskv6);
    }
    print_ip("primary dns", adapter->primary_dns);
    print_ip("secondary dns", adapter->secondary_dns);

    if (adapter->is_dhcp_v4)
        print_ip("dhcp", adapter->dhcp);
    if (adapter->is_connected)
        printf("  connected\n");
}
