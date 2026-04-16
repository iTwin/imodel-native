#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Request WPAD url using DHCP with a particular network adapter
char *wpad_dhcp_adapter_posix(uint8_t bind_ip[4], net_adapter_s *adapter, int32_t timeout_sec);

#ifdef __cplusplus
}
#endif
