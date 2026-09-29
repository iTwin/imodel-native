#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <gtest/gtest.h>

#include "net_adapter.h"

static inline bool print_adapter(void *user_data, net_adapter_s *adapter) {
    net_adapter_print(adapter);
    return true;
}

TEST(net_adapter, enum) {
    net_adapter_enum(NULL, print_adapter);
}
