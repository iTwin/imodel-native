#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <net/if.h>
#ifdef HAVE_NET_IF_ARP_H
#  include <net/if_arp.h>
#else
#  define ARPHRD_ETHER   1  // Ethernet hardware format
#  define ARPHRD_IEEE802 6  // IEEE 802.2 Ethernet/TR/TB
#endif
#include <sys/ioctl.h>

#include "log.h"
#include "net_adapter.h"
#include "util.h"
#include "util_win.h"

bool net_adapter_enum(void *user_data, net_adapter_cb callback) {
    net_adapter_s adapter;
    struct ifaddrs *ifp = NULL;
    struct ifaddrs *ifa = NULL;

    if (getifaddrs(&ifp) == -1)
        return false;

    for (ifa = ifp; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr)
            continue;
        // Ignore non-physical adapters
        if (ifa->ifa_flags & IFF_LOOPBACK)
            continue;
        if ((ifa->ifa_flags & ARPHRD_ETHER) == 0 && (ifa->ifa_flags & ARPHRD_IEEE802) == 0)
            continue;

        memset(&adapter, 0, sizeof(adapter));

        int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
        if (fd) {
            struct ifreq ifr = {0};

            ifr.ifr_ifindex = if_nametoindex(ifa->ifa_name);

            // Copy adapter name
            if (ioctl(fd, SIOCGIFNAME, &ifr) == 0)
                strncat(adapter.name, ifr.ifr_name, sizeof(adapter.name) - 1);

            // Check if the connection is online
            if (ioctl(fd, SIOCGIFFLAGS, &ifr) == 0) {
                if (ifr.ifr_flags & IFF_UP)
                    adapter.is_connected = true;
            }

            // Copy hardware address
            if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {
                adapter.mac_length = IFHWADDRLEN;
                if (adapter.mac_length > sizeof(adapter.mac))
                    adapter.mac_length = sizeof(adapter.mac);
                memcpy(adapter.mac, ifr.ifr_hwaddr.sa_data, adapter.mac_length);
            }

            close(fd);
        } else {
            strncat(adapter.name, ifa->ifa_name, sizeof(adapter.name) - 1);
        }

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

    freeifaddrs(ifp);
    return true;
}
