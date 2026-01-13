#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

#include <gtest/gtest.h>

#include "net_adapter.h"
#include "wpad_dhcp.h"
#include "wpad_dhcp_posix.h"
#include "wpad_dns.h"
#include "util.h"

TEST(wpad, dhcp) {
    const int32_t timeout_sec = 5;
    uint8_t bind_ip[4] = {0};
    char hostname[HOST_MAX] = {0};

    // Get local hostname
    EXPECT_NE(gethostname(hostname, sizeof(hostname)), -1);
    hostname[sizeof(hostname) - 1] = 0;

    // Get hostent for local hostname
    struct hostent *localent = nullptr;
#ifdef _WIN32
    do {
        localent = gethostbyname(hostname);
    } while (!localent && WSAGetLastError() == WSATRY_AGAIN);
#else
    do {
        localent = gethostbyname(hostname);
    } while (!localent && h_errno == TRY_AGAIN);
#endif
    // On CI runners, hostname might not resolve, so try common fallbacks
    if (!localent) {
        printf("Trying localhost fallback...\n");
        localent = gethostbyname("localhost");
        if (!localent) {
            printf("Trying 127.0.0.1 fallback...\n");
            localent = gethostbyname("127.0.0.1");
        }
    }

    // If we still can't resolve anything, skip this test on CI environments
    if (!localent) {
        GTEST_SKIP() << "Cannot resolve hostname '" << hostname << "' or localhost on this system. Skipping DHCP test.";
    }

    // Create network adapter
    net_adapter_s adapter{};
    strncat(adapter.name, "Ethernet", sizeof(adapter.name) - 1);
    adapter.is_dhcp_v4 = true;
    memset(adapter.mac, 'a', sizeof(adapter.mac));
    adapter.mac_length = 6;
    if (localent && localent->h_addr_list)
        memcpy(adapter.ip, *localent->h_addr_list, sizeof(adapter.ip));

    // Lookup WPAD using network adapter
    char *wpad = wpad_dhcp_adapter_posix(bind_ip, &adapter, timeout_sec);
    EXPECT_NE(wpad, nullptr);
    if (wpad)
        EXPECT_STREQ(wpad, "http://wpad.com/wpad.dat");
    free(wpad);
}

TEST(wpad, dns) {
    char *wpad = wpad_dns(NULL);
    (void)wpad;
    /*EXPECT_NE(wpad, nullptr);
    if (wpad)
        EXPECT_STREQ(wpad, "http://wpad.com/wpad.dat");
    free(wpad);*/
}
