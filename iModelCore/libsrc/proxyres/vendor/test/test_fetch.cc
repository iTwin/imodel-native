#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <gtest/gtest.h>

#include "fetch.h"

TEST(fetch, get) {
    int32_t error = 0;
#ifdef HAVE_CURL
    const char *url = "https://google.com/";
    const char *expected_string = "world's information";
#else
    const char *url = "http://google.com/";
    const char *expected_string = "document has moved";
#endif
    char *body = fetch_get(url, &error);
    EXPECT_EQ(error, 0);
    EXPECT_NE(body, nullptr);
    if (body) {
        EXPECT_TRUE(strstr(body, expected_string) != nullptr);
        free(body);
    }
}
