#pragma once

bool proxy_config_gnome3_get_auto_discover(void);
char *proxy_config_gnome3_get_auto_config_url(void);
char *proxy_config_gnome3_get_proxy(const char *scheme);
char *proxy_config_gnome3_get_bypass_list(void);

bool proxy_config_gnome3_global_init(void);
bool proxy_config_gnome3_global_cleanup(void);

proxy_config_i_s *proxy_config_gnome3_get_interface(void);
