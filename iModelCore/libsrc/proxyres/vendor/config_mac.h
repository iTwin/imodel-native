#pragma once

bool proxy_config_mac_get_auto_discover(void);
char *proxy_config_mac_get_auto_config_url(void);
char *proxy_config_mac_get_proxy(const char *scheme);
char *proxy_config_mac_get_bypass_list(void);

bool proxy_config_mac_global_init(void);
bool proxy_config_mac_global_cleanup(void);

proxy_config_i_s *proxy_config_mac_get_interface(void);
