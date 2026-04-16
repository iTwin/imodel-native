#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct net_adapter_s {
    char name[256];
    char guid[256];
    char description[256];
    uint8_t ip[4];
    uint8_t ipv6[16];
    uint8_t netmask[4];
    uint8_t netmaskv6[16];
    uint8_t gateway[4];
    uint8_t primary_dns[4];
    uint8_t secondary_dns[4];
    uint8_t dhcp[4];
    uint8_t mac[8];
    uint8_t mac_length;
    bool is_connected;
    bool is_dhcp_v4;
    bool is_ipv6;
} net_adapter_s;

// Callback to enumerate network adapters
typedef bool (*net_adapter_cb)(void *user_data, net_adapter_s *adapter);

// Enumerate IPv4-enabled ethernet adapters
bool net_adapter_enum(void *user_data, net_adapter_cb callback);
// Print network adapter information
void net_adapter_print(net_adapter_s *adapter);

#ifdef __cplusplus
}
#endif
