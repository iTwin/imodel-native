/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/AuthenticationHandlerTests.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "TestsHelper.h"
#include "WebTestsHelper.h"
#include "MockAuthenticationHandler.h"
#include <BeHttp/AuthenticationHandler.h>
#include <Bentley/Base64Utilities.h>
USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS
class AuthenticationHandlerTests : public BaseMockHttpHandlerTest, public ::testing::Test
    {
    };
