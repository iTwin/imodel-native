/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "WebTestsHelper.h"

#include <BeHttp/HttpProxy.h>

USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS

struct HttpRequestTests : public ::testing::Test {};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Ctor_InvalidUrl_UrlSetToEmpty)
    {
    BeTest::SetFailOnAssert(false);
    Request request("test url");
    BeTest::SetFailOnAssert(true);

    EXPECT_STREQ("", request.GetUrl().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Robert.Lukasonok                      08/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Ctor_ValidUrl_SameUrlRetained)
    {
    Request request("foo://bar");
    EXPECT_STREQ("foo://bar", request.GetUrl().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Ctor_UrlWithUnsafeCharacters_CharactersAreEscaped)
    {
    Request request(TEST_URL_UNSAFE_CHARS);
    EXPECT_STREQ(TEST_URL_UNSAFE_CHARS_ESCAPED, request.GetUrl().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, Ctor_Default_CertificateVerificationEnabled)
    {
    Request request("foo://bar");
    EXPECT_TRUE(request.GetValidateCertificate());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                           12/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpRequestTests, SetProxy_ProxyUrlWithUnsafeCharacters_CharactersAreEscaped)
    {
    Request request("foo://bar");
    request.SetProxy(TEST_URL_UNSAFE_CHARS);
    EXPECT_STREQ(TEST_URL_UNSAFE_CHARS_ESCAPED, request.GetProxy().c_str());
    }
