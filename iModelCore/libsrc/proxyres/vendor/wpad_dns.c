#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  include <windows.h>
#else
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netdb.h>
#  include <unistd.h>
#endif

#include "fetch.h"
#include "log.h"
#include "net_util.h"
#include "util.h"
#include "wpad_dns.h"

#ifdef _WIN32
#  define socketerr WSAGetLastError()
#else
#  define socketerr errno
#endif

// Request WPAD script using DNS
char *wpad_dns(const char *fqdn) {
    char hostname[HOST_MAX] = {0};
    char wpad_host[HOST_MAX] = {0};
    int32_t error = 0;

    if (!fqdn) {
        // Get local hostname
        if (gethostname(hostname, sizeof(hostname)) == -1) {
            log_error("Unable to get hostname (%d)", socketerr);
            return NULL;
        }
        hostname[sizeof(hostname) - 1] = 0;

        // Get hostent for local hostname
        struct hostent *localent = gethostbyname(hostname);
        if (!localent) {
            log_error("Unable to get hostent for %s (%d)", hostname, socketerr);
            return NULL;
        }

        // Canonical name found in h_name
        fqdn = localent->h_name;

        // Validate canonical name
        if (!fqdn || !strlen(fqdn) || !strcmp(fqdn, "localhost") || isdigit(*fqdn)) {
            log_debug("Unable to get canonical name for %s (%d)", hostname, socketerr);
            return NULL;
        }
    }

    // Enumerate through each part of the FQDN
    const char *name = fqdn;
    const char *next_part;
    do {
        next_part = strchr(name, '.');
        if (!next_part)
            break;
        next_part++;

        // Construct WPAD url with next part of FQDN
        snprintf(wpad_host, sizeof(wpad_host), "wpad.%s", name);
        log_info("Checking next WPAD hostname: %s", wpad_host);

        char wpad_url[HOST_MAX + 18];
        snprintf(wpad_url, sizeof(wpad_url), "http://%s/wpad.dat", wpad_host);
        char *script = fetch_get(wpad_url, &error);
        if (script)
            return script;

        log_info("No server found at %s (%d)", wpad_host, error);
        name = next_part;
    } while (true);

    return NULL;
}