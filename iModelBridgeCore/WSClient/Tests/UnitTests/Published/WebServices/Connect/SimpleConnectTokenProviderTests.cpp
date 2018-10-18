/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/SimpleConnectTokenProviderTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "SimpleConnectTokenProviderTests.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
using namespace ::testing;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             julius.cepukenas    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SimpleConnectTokenProviderTests, GetToken_CreatedWithToken_ReturnsToken)
    {
    auto token = std::make_shared<SecurityToken>("token");
    SimpleConnectTokenProvider provider(token);

    EXPECT_EQ("token", provider.GetToken()->AsString());
    EXPECT_EQ("token", provider.GetToken()->ToAuthorizationString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SimpleConnectTokenProviderTests, UpdateToken_DefaultCallback_ReturnsNullToken)
    {
    auto token = std::make_shared<SecurityToken>("token");
    SimpleConnectTokenProvider provider(token);

    EXPECT_EQ(nullptr, provider.UpdateToken()->GetResult());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              julius.cepukenas    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SimpleConnectTokenProviderTests, UpdateToken_CreatedWithTokenAndUpdateCallback_CallsAndReturnsUpdateCallbackResult)
    {
    auto onUpdate = [] { return CreateCompletedAsyncTask<ISecurityTokenPtr>(std::make_shared<SecurityToken>("NewToken")); };

    auto token = std::make_shared<SecurityToken>("token");
    SimpleConnectTokenProvider provider(token, onUpdate);

    EXPECT_EQ("NewToken", provider.UpdateToken()->GetResult()->AsString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             julius.cepukenas    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SimpleConnectTokenProviderTests, GetToken_AfterUpdateToken_SameToken)
    {
    auto onUpdate = [] { return CreateCompletedAsyncTask<ISecurityTokenPtr>(std::make_shared<SecurityToken>("NewToken")); };

    auto token = std::make_shared<SecurityToken>("token");
    SimpleConnectTokenProvider provider(token, onUpdate);

    EXPECT_EQ("NewToken", provider.UpdateToken()->GetResult()->AsString());
    EXPECT_EQ("NewToken", provider.GetToken()->AsString());
    }
