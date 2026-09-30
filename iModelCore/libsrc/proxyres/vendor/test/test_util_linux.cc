#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <gtest/gtest.h>

#include "util_linux.h"

struct get_config_value_param {
    const char *section;
    const char *key;
    const char *expected;

    friend std::ostream &operator<<(std::ostream &os, const get_config_value_param &param) {
        return os << "section: " << param.section << std::endl << "key: " << param.key;
    }
};

static const char *config = R"(
[Notification Messages]
WarnOnLeaveSSLMode=false

[Proxy Settings]
AuthMode=0
NoProxyFor=
Proxy Config Script=
ProxyType=0
ReversedException=false
ftpProxy=
httpProxy=http://127.0.0.1:8000/
httpsProxy=http://127.0.0.1:8001/)";

constexpr get_config_value_param get_config_value_tests[] = {
    {"Proxy Settings", "ProxyType", "0"},
    {"Proxy Settings", "httpProxy", "http://127.0.0.1:8000/"},
    {"Proxy Settings", "httpsProxy", "http://127.0.0.1:8001/"},
    // No value set should return NULL
    {"Proxy Settings", "ftpProxy", NULL},
    {"Proxy Settings", "AuthMode", "0"},
    {"Proxy Settings", "NoProxyFor", NULL},
    {"Proxy Settings", "Proxy Config Script", NULL},
    {"Proxy Settings", "ReversedException", "false"},
    // No key should return NULL
    {"Proxy Settings", "Does Not Exist", NULL},
    {"Notification Messages", "WarnOnLeaveSSLMode", "false"},
};

class util_config : public ::testing::TestWithParam<get_config_value_param> {};

INSTANTIATE_TEST_SUITE_P(util, util_config, testing::ValuesIn(get_config_value_tests));

TEST_P(util_config, get_value) {
    const auto &param = GetParam();
    char *value = get_config_value(config, param.section, param.key);
    if (value) {
        EXPECT_STREQ(value, param.expected);
        free(value);
    } else {
        EXPECT_EQ(value, param.expected);
    }
}