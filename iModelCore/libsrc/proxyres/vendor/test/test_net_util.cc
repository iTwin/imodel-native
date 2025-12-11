#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <gtest/gtest.h>

#include "net_util.h"

TEST(net_util, my_ip_address) {
    char *address = my_ip_address();
    EXPECT_NE(address, nullptr);
    if (address) {
        EXPECT_NE(strchr(address, '.'), nullptr);
        EXPECT_EQ(strpbrk(address, ";:"), nullptr);
        free(address);
    }
}

TEST(net_util, my_ip_address_ex) {
    char *addresses = my_ip_address_ex();
    EXPECT_NE(addresses, nullptr);
    if (addresses) {
        EXPECT_NE(strpbrk(addresses, ".:"), nullptr);
        free(addresses);
    }
}

TEST(net_util, dns_resolve_google) {
    int32_t error = 0;
    char *ip = dns_resolve("google.com", &error);
    EXPECT_EQ(error, 0);
    EXPECT_NE(ip, nullptr);
    if (ip) {
        EXPECT_NE(strstr(ip, "."), nullptr);
        EXPECT_EQ(strpbrk(ip, ";:"), nullptr);
        free(ip);
    }
}

TEST(net_util, dns_resolve_bad) {
    int32_t error = 0;
    char *ip = dns_resolve("hopefully-doesnt-exist.com", &error);
    EXPECT_EQ(ip, nullptr);
    EXPECT_NE(error, 0);
}

TEST(net_util, dns_ex_resolve_google) {
    int32_t error = 0;
    char *ips = dns_resolve_ex("google.com", &error);
    EXPECT_EQ(error, 0);
    EXPECT_NE(ips, nullptr);
    if (ips) {
        EXPECT_NE(strpbrk(ips, ".:"), nullptr);
        free(ips);
    }
}
