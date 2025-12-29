#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <windows.h>
#include <dhcpcsdk.h>

#include "log.h"
#include "net_adapter.h"
#include "util.h"
#include "util_win.h"

char *wpad_dhcp_adapter_win(uint8_t bind_ip[4], net_adapter_s *adapter, int32_t timeout_sec) {
    DHCPCAPI_PARAMS wpad_params = {0, 252, false, NULL, 0};
    DHCPCAPI_PARAMS_ARRAY request_params = {1, &wpad_params};
    DHCPCAPI_PARAMS_ARRAY send_params = {0, NULL};
    uint8_t buffer[4096];
    DWORD buffer_len = sizeof(buffer);

    UNUSED(timeout_sec);
    UNUSED(bind_ip);

    wchar_t *adapter_guid_wide = utf8_dup_to_wchar(adapter->guid);
    if (!adapter_guid_wide)
        return NULL;

    DWORD err = DhcpRequestParams(DHCPCAPI_REQUEST_SYNCHRONOUS, NULL, adapter_guid_wide, NULL, send_params,
                                  request_params, buffer, &buffer_len, NULL);
    free(adapter_guid_wide);

    if (err != NO_ERROR || wpad_params.nBytesData) {
        log_debug("Error requesting WPAD from DHCP server (%d)", err);
        return NULL;
    }

    // Copy wpad url from buffer
    char *wpad = (char *)calloc(wpad_params.nBytesData + 1, sizeof(char));
    if (!wpad)
        return NULL;
    strncat(wpad, (char *)wpad_params.Data, wpad_params.nBytesData);

    // Remove any trailing new line character that some DHCP servers send
    str_trim_end(wpad, '\n');

    if (!*wpad) {
        free(wpad);
        return NULL;
    }

    return wpad;
}
