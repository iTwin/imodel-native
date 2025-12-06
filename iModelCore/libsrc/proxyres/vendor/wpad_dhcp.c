#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include "net_adapter.h"
#include "util.h"
#include "wpad_dhcp.h"
#include "wpad_dhcp_posix.h"
#if defined(_WIN32)
#  include "wpad_dhcp_win.h"
#elif defined(__APPLE__)
#  include "wpad_dhcp_mac.h"
#endif

char *wpad_dhcp_adapter(uint8_t bind_ip[4], net_adapter_s *adapter, int32_t timeout_sec) {
    char *wpad = NULL;
#if defined(_WIN32)
    wpad = wpad_dhcp_adapter_win(bind_ip, adapter, timeout_sec);
#elif defined(__APPLE__)
    wpad = wpad_dhcp_adapter_mac(bind_ip, adapter, timeout_sec);
#endif
    if (!wpad)
        return wpad_dhcp_adapter_posix(bind_ip, adapter, timeout_sec);
    return NULL;
}

typedef struct wpad_dhcp_adapter_enum_s {
    char *url;
    int32_t timeout_sec;
} wpad_dhcp_adapter_enum_s;

static bool wpad_dhcp_enum_adapter(void *user_data, net_adapter_s *adapter) {
    wpad_dhcp_adapter_enum_s *adapter_enum = (wpad_dhcp_adapter_enum_s *)user_data;

    // Check adapter is connected
    if (!adapter->is_connected)
        return true;
#ifdef _WIN32
    // Only Windows supports DHCPv4 detection
    if (!adapter->is_dhcp_v4)
        return true;
#endif
    // Check adapter has valid ip
    if (!*adapter->ip)
        return true;

    // Get WPAD from adapter's DHCP server
    adapter_enum->url = wpad_dhcp_adapter(adapter->ip, adapter, adapter_enum->timeout_sec);
    if (adapter_enum->url)
        return false;

    return true;
}

char *wpad_dhcp(int32_t timeout_sec) {
    wpad_dhcp_adapter_enum_s adapter_enum = {NULL, timeout_sec};

    // Enumerate each network adapter and send DHCP request for WPAD
    net_adapter_enum(&adapter_enum, wpad_dhcp_enum_adapter);

    return adapter_enum.url;
}