/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "ConnectTestsHelper.h"
#include <WebServices/Connect/SecurityToken.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

using namespace ::testing;

struct SecurityTokenTests : WSClientBaseTest {};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SecurityTokenTests, AsString_NoStringIsProvided_EmptyString)
    {
    SecurityToken token;
    EXPECT_STREQ("", token.AsString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SecurityTokenTests, AsString_StringIsProvided_ReturnsString)
    {
    SecurityToken token("token");
    EXPECT_STREQ("token", token.AsString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SecurityTokenTests, ToAuthorizationString_NoStringIsProvided_EmptyString)
    {
    SecurityToken token;
    EXPECT_STREQ("", token.ToAuthorizationString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SecurityTokenTests, ToAuthorizationString_StringIsProvided_ReturnsString)
    {
    SecurityToken token("token");
    EXPECT_STREQ("token", token.ToAuthorizationString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SecurityTokenTests, IsSupported_NoStringIsProvided_False)
    {
    SecurityToken token;
    EXPECT_FALSE(token.IsSupported());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SecurityTokenTests, IsSupported_EmptyStringProvided_False)
    {
    SecurityToken token("");
    EXPECT_FALSE(token.IsSupported());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SecurityTokenTests, IsSupported_StringIsProvided_True)
    {
    SecurityToken token("token");
    EXPECT_TRUE(token.IsSupported());
    }
