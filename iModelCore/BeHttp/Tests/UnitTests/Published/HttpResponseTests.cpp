/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/HttpResponseTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "WebTestsHelper.h"
#include <BeHttp/HttpResponse.h>

USING_NAMESPACE_BENTLEY_HTTP

struct HttpResponseTests : public ::testing::Test {};

TEST_F(HttpResponseTests, ToStatusString_AllConnectionStatusValues_NotEmpty)
    {
    for (int i = (int) ConnectionStatus::None; i <= (int) ConnectionStatus::UnknownError; i++)
        {
        EXPECT_NE("", Response::ToStatusString((ConnectionStatus) i, HttpStatus::None));
        }
    }
