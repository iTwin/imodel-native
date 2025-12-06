#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#ifdef HAVE_NET_IF_ARP_H
#  include <net/if_arp.h>
#else
#  define ARPHRD_ETHER   1  // Ethernet hardware format
#  define ARPHRD_IEEE802 6  // Token-ring hardware format
#endif
#include <net/if_dl.h>
#include <sys/sysctl.h>

#include "log.h"
#include "net_adapter.h"
#include "util.h"
#include "util_win.h"

bool net_adapter_enum(void *user_data, net_adapter_cb callback) {
    net_adapter_s adapter;
    struct ifaddrs *ifp = NULL;
    struct ifaddrs *ifa = NULL;
    char *buffer = NULL;
    size_t buffer_len = 0;
    size_t required_len = 0;

    if (getifaddrs(&ifp) == -1)
        return false;

    for (ifa = ifp; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr)
            continue;

        memset(&adapter, 0, sizeof(adapter));

        int mib[6] = {0};

        mib[0] = CTL_NET;
        mib[1] = AF_ROUTE;
        mib[2] = 0;
        mib[3] = AF_LINK;
        mib[4] = NET_RT_IFLIST;
        mib[5] = if_nametoindex(ifa->ifa_name);

        required_len = buffer_len;
        if (sysctl(mib, 6, NULL, &required_len, NULL, 0) < 0)
            continue;

        if (required_len > buffer_len) {
            free(buffer);
            buffer_len = required_len;
            buffer = (char *)malloc(buffer_len + 1);
        }

        if (sysctl(mib, 6, buffer, &buffer_len, NULL, 0) < 0)
            continue;

        struct if_msghdr *ifm = (struct if_msghdr *)buffer;

        // Ignore non-physical adapters
        if (ifm->ifm_flags & IFF_LOOPBACK)
            continue;
        if ((ifm->ifm_data.ifi_type & ARPHRD_ETHER) == 0 && (ifm->ifm_data.ifi_type & ARPHRD_IEEE802) == 0)
            continue;

        if (ifm->ifm_flags & IFF_UP)
            adapter.is_connected = true;

        strncat(adapter.name, ifa->ifa_name, sizeof(adapter.name) - 1);
        adapter.mac_length = 6;
        memcpy(adapter.mac, LLADDR((struct sockaddr_dl *)(ifm + 1)), adapter.mac_length);

        if (ifa->ifa_addr->sa_family == AF_INET) {
            memcpy(adapter.ip, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, sizeof(adapter.ip));
            memcpy(adapter.gateway, &((struct sockaddr_in *)ifa->ifa_broadaddr)->sin_addr, sizeof(adapter.gateway));
            memcpy(adapter.netmask, &((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr, sizeof(adapter.netmask));
        } else if (ifa->ifa_addr->sa_family == AF_INET6) {
            memcpy(adapter.ipv6, &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr, sizeof(adapter.ipv6));
            memcpy(adapter.netmaskv6, &((struct sockaddr_in6 *)ifa->ifa_netmask)->sin6_addr, sizeof(adapter.netmaskv6));
            adapter.is_ipv6 = true;
        }

        if (!callback(user_data, &adapter))
            break;
    }

    free(buffer);

    freeifaddrs(ifp);
    return true;
}
