/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/ConnectTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConnectTests.h"
#include <WebServices/Connect/Connect.h>
#include <Bentley/Base64Utilities.h>
#include <WebServices/Configuration/UrlProvider.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
using namespace ::testing;

void ConnectTests::SetUp()
    {
    Connect::Initialize(StubClientInfo(), GetHandlerPtr());
    m_client = std::make_shared<StubBuddiClient>();
    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &m_localState, m_client);
    }

void ConnectTests::TearDown()
    {
    Connect::Uninintialize();
    }

TEST_F(ConnectTests, Login_DefaultUrls_SendsRequestToRetrieveToken)
    {
    GetHandler().ForFirstRequest([&] (HttpRequestCR request)
        {
        EXPECT_STREQ("TestUrl", request.GetUrl().c_str());
        EXPECT_EQ("Basic " + Base64Utilities::Encode("Foo:Boo"), request.GetHeaders().GetAuthorization());
        return StubHttpResponse();
        });

    Connect::Login(Credentials("Foo", "Boo"))->GetResult();
    }

TEST_F(ConnectTests, Login_ResponseContainsToken_ReturnsToken)
    {
    auto tokenStr = StubSamlTokenXML();
    Json::Value bodyJson;
    bodyJson["RequestedSecurityToken"] = tokenStr;

    GetHandler().ForFirstRequest(StubHttpResponse(HttpStatus::OK, bodyJson.toStyledString()));

    auto result = Connect::Login(Credentials("Foo", "Boo"))->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(tokenStr, result.GetValue()->AsString());
    }

TEST_F(ConnectTests, GetStsToken_ByCredentials_SendsRequestToRetrieveToken)
    {
    GetHandler().ForFirstRequest([&] (HttpRequestCR request)
        {
        auto bodyJson = request.GetRequestBody()->AsJson();

        EXPECT_EQ("http://applies.to.url", bodyJson["AppliesTo"].asString());
        EXPECT_EQ("", bodyJson["ActAs"].asString());
        EXPECT_EQ("123", bodyJson["Lifetime"].asString());

        EXPECT_EQ("Basic " + Base64Utilities::Encode("Foo:Boo"), request.GetHeaders().GetAuthorization());
        EXPECT_EQ("http://sts.url", request.GetUrl());

        return StubHttpResponse();
        });

    Connect::GetStsToken(Credentials("Foo", "Boo"), "http://applies.to.url", "http://sts.url", 123)->GetResult();
    }

TEST_F(ConnectTests, GetStsToken_ByCredentialsWithZeroLifetime_SendsRequestToRetrieveTokenWithoutLifetimee)
    {
    GetHandler().ForFirstRequest([&] (HttpRequestCR request)
        {
        auto bodyJson = request.GetRequestBody()->AsJson();

        EXPECT_EQ("http://applies.to.url", bodyJson["AppliesTo"].asString());
        EXPECT_EQ("", bodyJson["ActAs"].asString());
        EXPECT_FALSE(bodyJson.isMember("Lifetime"));

        EXPECT_EQ("Basic " + Base64Utilities::Encode("Foo:Boo"), request.GetHeaders().GetAuthorization());
        EXPECT_EQ("http://sts.url", request.GetUrl());

        return StubHttpResponse();
        });

    Connect::GetStsToken(Credentials("Foo", "Boo"), "http://applies.to.url", "http://sts.url", 0)->GetResult();
    }

TEST_F(ConnectTests, GetStsToken_ByParentToken_SendsRequestToRetrieveToken)
    {
    SamlToken parentToken(StubSamlTokenXML(0, "TestCert"));

    GetHandler().ForFirstRequest([&] (HttpRequestCR request)
        {
        auto bodyJson = request.GetRequestBody()->AsJson();

        EXPECT_EQ("http://applies.to.url", bodyJson["AppliesTo"].asString());
        EXPECT_EQ("http://applies.to.url", bodyJson["AppliesToBootstrapToken"].asString());
        EXPECT_EQ(parentToken.AsString(), bodyJson["ActAs"].asString());
        EXPECT_EQ("123", bodyJson["Lifetime"].asString());

        EXPECT_STREQ("X509 access_token=TestCert", request.GetHeaders().GetAuthorization());
        EXPECT_EQ("http://sts.url", request.GetUrl());

        return StubHttpResponse();
        });

    Connect::GetStsToken(parentToken, "http://applies.to.url", "http://sts.url", 123)->GetResult();
    }

TEST_F(ConnectTests, GetStsToken_ByParentTokenWithZeroLifetime_SendsRequestToRetrieveTokenWithoutLifetime)
    {
    SamlToken parentToken(StubSamlTokenXML(0, "TestCert"));

    GetHandler().ForFirstRequest([&] (HttpRequestCR request)
        {
        auto bodyJson = request.GetRequestBody()->AsJson();

        EXPECT_EQ("http://applies.to.url", bodyJson["AppliesTo"].asString());
        EXPECT_EQ("http://applies.to.url", bodyJson["AppliesToBootstrapToken"].asString());
        EXPECT_EQ(parentToken.AsString(), bodyJson["ActAs"].asString());
        EXPECT_FALSE(bodyJson.isMember("Lifetime"));

        EXPECT_STREQ("X509 access_token=TestCert", request.GetHeaders().GetAuthorization());
        EXPECT_EQ("http://sts.url", request.GetUrl());

        return StubHttpResponse();
        });

    Connect::GetStsToken(parentToken, "http://applies.to.url", "http://sts.url", 0)->GetResult();
    }

TEST_F(ConnectTests, IsImsLoginRedirect_NonLoginUrl_False)
    {
    EXPECT_FALSE(Connect::IsImsLoginRedirect(StubHttpResponseWithUrl(HttpStatus::OK, "http://test.com/other")));
    }

TEST_F(ConnectTests, IsImsLoginRedirect_StatusNotOK_False)
    {
    EXPECT_FALSE(Connect::IsImsLoginRedirect(StubHttpResponseWithUrl(HttpStatus::Found, "http://test.com//IMS/Account/Login?foo")));
    }

TEST_F(ConnectTests, IsImsLoginRedirect_IMSLoginUrlAndStatusOK_True)
    {
    EXPECT_TRUE(Connect::IsImsLoginRedirect(StubHttpResponseWithUrl(HttpStatus::OK, "http://test.com//IMS/Account/Login?foo")));
    }

TEST_F(ConnectTests, GetFederatedSignInUrl_NoDomainParameter_UrlHasNoOfhParameter)
    {
#if defined (BENTLEY_WIN32)
    const Utf8String platformType("desktop");
#else
    const Utf8String platformType("mobile");
#endif

    Utf8String signInUrl("TestUrl?wa=wsignin1.0&wtrealm=sso%3A%2F%2Fwsfed_");
    signInUrl += platformType + "%2FTestAppProductId";

    EXPECT_STREQ(signInUrl.c_str(), Connect::GetFederatedSignInUrl().c_str());
    }

TEST_F(ConnectTests, GetFederatedSignInUrl_DomainParameter_UrlHasOfhParameter)
    {
#if defined (BENTLEY_WIN32)
    const Utf8String platformType("desktop");
#else
    const Utf8String platformType("mobile");
#endif

    const Utf8String domainName("bentley");
    Utf8String signInUrl("TestUrl?wa=wsignin1.0&wtrealm=sso%3A%2F%2Fwsfed_");
    signInUrl += platformType + "%2FTestAppProductId&ofh=" + Base64Utilities::Encode(domainName);

    EXPECT_STREQ(signInUrl.c_str(), Connect::GetFederatedSignInUrl(domainName).c_str());
    }
