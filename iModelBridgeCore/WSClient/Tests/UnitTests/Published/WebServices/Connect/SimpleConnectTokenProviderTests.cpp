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

#ifdef USE_GTEST
void SimpleConnectTokenProviderTests::SetUp()
    {
    WSClientBaseTest::SetUp();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             julius.cepukenas    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SimpleConnectTokenProviderTests, GetToken_CreatedWToken_ReturnsToken)
    {
    auto token = std::make_shared<SecurityToken>("token");
    SimpleConnectTokenProvider provider(token);

    EXPECT_EQ("token", provider.GetToken()->AsString());
    EXPECT_EQ("token", provider.GetToken()->ToAuthorizationString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                               julius.cepukenas    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SimpleConnectTokenProviderTests, UpdateToken_DefaultCallback_ReturnsEmptyToken)
    {
    auto token = std::make_shared<SecurityToken>("token");
    SimpleConnectTokenProvider provider(token);

    EXPECT_EQ("", provider.UpdateToken()->GetResult()->AsString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                              julius.cepukenas    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SimpleConnectTokenProviderTests, UpdateToken_CreatedWToken_Nullptr)
    {
    auto f1([]() {return CreateCompletedAsyncTask(std::make_shared<SecurityToken>("NewToken")); });
    std::function<AsyncTaskPtr<SecurityTokenPtr>()> f(std::cref(f1));

    auto token = std::make_shared<SecurityToken>("token");
    SimpleConnectTokenProvider provider(token, f);

    EXPECT_EQ("NewToken", provider.UpdateToken()->GetResult()->AsString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             julius.cepukenas    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SimpleConnectTokenProviderTests, GetToken_AfterUpdateToken_SameToken)
    {
    auto f1([]() {return CreateCompletedAsyncTask(std::make_shared<SecurityToken>("NewToken")); });
    std::function<AsyncTaskPtr<SecurityTokenPtr>()> f(std::cref(f1));

    auto token = std::make_shared<SecurityToken>("token");
    SimpleConnectTokenProvider provider(token, f);

    EXPECT_EQ("NewToken", provider.UpdateToken()->GetResult()->AsString());
    EXPECT_EQ("NewToken", provider.GetToken()->AsString());
    }
#endif
