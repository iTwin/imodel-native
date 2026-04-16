#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <gtest/gtest.h>

#include "util_win.h"

struct get_winhttp_proxy_by_scheme_param {
    const char *proxy_list;
    const char *scheme;
    const char *expected;

    friend std::ostream &operator<<(std::ostream &os, const get_winhttp_proxy_by_scheme_param &param) {
        return os << "proxy_list: " << param.proxy_list << std::endl
                  << "scheme: " << param.scheme;
    }
};

constexpr get_winhttp_proxy_by_scheme_param get_winhttp_proxy_by_scheme_tests[] = {
    // No schemes in list
    {"1proxy.com", "http", "1proxy.com"},
    {"2proxy.com", "https", "2proxy.com"},
    // One http scheme in list
    {"http=3proxy.com", "http", "3proxy.com"},
    {"http=4proxy.com:80", "http", "4proxy.com:80"},
    // One https scheme in list
    {"https=5proxy.com", "https", "5proxy.com"},
    {"https=6proxy.com:443", "https", "6proxy.com:443"},
    // Two schemes in list
    {"http=7proxy.com;https=8proxy.com", "http", "7proxy.com"},
    {"https=9proxy.com;http=10proxy.com", "http", "10proxy.com"},
    // Two schemes in proxy list reversed
    {"http=11proxy.com;https=12proxy.com:443", "https", "12proxy.com:443"},
    {"https=13proxy.com;http=14proxy.com:80;", "https", "13proxy.com"},
};

class util_winhttp_proxy : public ::testing::TestWithParam<get_winhttp_proxy_by_scheme_param> {};

INSTANTIATE_TEST_SUITE_P(util, util_winhttp_proxy, testing::ValuesIn(get_winhttp_proxy_by_scheme_tests));

TEST_P(util_winhttp_proxy, get_by_scheme) {
    const auto &param = GetParam();
    char *proxy = get_winhttp_proxy_by_scheme(param.scheme, param.proxy_list);
    EXPECT_NE(proxy, nullptr);
    if (proxy) {
        EXPECT_STREQ(proxy, param.expected);
        free(proxy);
    }
}

struct convert_winhttp_proxy_list_to_uri_list_param {
    const char *proxy_list;
    const char *expected;

    friend std::ostream &operator<<(std::ostream &os, const convert_winhttp_proxy_list_to_uri_list_param &param) {
        return os << "proxy_list: " << param.proxy_list;
    }
};

constexpr convert_winhttp_proxy_list_to_uri_list_param convert_winhttp_proxy_list_to_uri_list_tests[] = {
    {"http=127.0.0.1;myproxy1.com", "http://127.0.0.1,http://myproxy1.com"},
    {"https=mysecureproxy1.com;http=myproxy2.com", "https://mysecureproxy1.com,http://myproxy2.com"},
    {"myproxy3.com", "http://myproxy3.com"},
    {"myproxy4.com:8080", "http://myproxy4.com:8080"},
    {"socks=socksprox.com http=httpprox.com", "socks://socksprox.com,http://httpprox.com"},
};

class util_winhttp_uri_list : public ::testing::TestWithParam<convert_winhttp_proxy_list_to_uri_list_param> {};

INSTANTIATE_TEST_SUITE_P(util, util_winhttp_uri_list, testing::ValuesIn(convert_winhttp_proxy_list_to_uri_list_tests));

TEST_P(util_winhttp_uri_list, convert_proxy_list) {
    const auto &param = GetParam();
    char *uri_list = convert_winhttp_proxy_list_to_uri_list(param.proxy_list);
    EXPECT_NE(uri_list, nullptr);
    if (uri_list) {
        EXPECT_STREQ(uri_list, param.expected);
        free(uri_list);
    }
}
