#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Request WPAD script using DNS
char *wpad_dns(const char *fqdn);

#ifdef __cplusplus
}
#endif
