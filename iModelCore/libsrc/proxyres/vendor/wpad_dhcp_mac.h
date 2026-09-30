#pragma once

// Request WPAD url using DHCP with a particular network adapter
char *wpad_dhcp_adapter_mac(uint8_t bind_ip[4], net_adapter_s *adapter, int32_t timeout_sec);
