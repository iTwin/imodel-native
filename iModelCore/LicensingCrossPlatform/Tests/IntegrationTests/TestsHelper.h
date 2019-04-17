/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <Bentley/BeTest.h>
#include <Licensing/Licensing.h>
#include <BeHttp/HttpClient.h>

#define BEGIN_BENTLEY_LICENSING_INTEGRATION_TESTS_NAMESPACE BEGIN_BENTLEY_LICENSING_NAMESPACE namespace IntegrationTests {
#define END_BENTLEY_LICENSING_INTEGRATION_TESTS_NAMESPACE   } END_BENTLEY_LICENSING_NAMESPACE
#define USING_NAMESPACE_BENTLEY_LICENSING_INTEGRATION_TESTS using namespace BentleyApi::Licensing::IntegrationTests;

#define EXPECT_SUCCESS(EXPR) EXPECT_EQ(SUCCESS, (EXPR))
#define EXPECT_ERROR(EXPR) EXPECT_EQ(ERROR, (EXPR))

BEGIN_BENTLEY_LICENSING_INTEGRATION_TESTS_NAMESPACE

END_BENTLEY_LICENSING_INTEGRATION_TESTS_NAMESPACE

USING_NAMESPACE_BENTLEY_HTTP
