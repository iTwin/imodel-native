/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/ImsClientTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ImsClientTests.h"
#include <WebServices/Connect/ImsClient.h>
#include <Bentley/Base64Utilities.h>
#include <WebServices/Configuration/UrlProvider.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
using namespace ::testing;

void ImsClientTests::SetUp()
    {
    BaseMockHttpHandlerTest::SetUp();
    m_buddiClient = std::make_shared<StubBuddiClient>();
    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &m_localState, m_buddiClient);
    }

TEST_F(ImsClientTests, GetToken_DefaultUrls_SendsRequestToRetrieveToken)
    {
    GetHandler().ForFirstRequest([&] (Http::RequestCR request)
        {
        EXPECT_STREQ("TestUrl", request.GetUrl().c_str());
        EXPECT_EQ("Basic " + Base64Utilities::Encode("Foo:Boo"), request.GetHeaders().GetAuthorization());
        return StubHttpResponse();
        });

    auto client = ImsClient::Create(StubClientInfo(), GetHandlerPtr());
    client->RequestToken(Credentials("Foo", "Boo"))->GetResult();
    }

TEST_F(ImsClientTests, GetToken_ResponseContainsToken_ReturnsToken)
    {
    auto tokenStr = StubSamlTokenXML();
    Json::Value bodyJson;
    bodyJson["RequestedSecurityToken"] = tokenStr;

    GetHandler().ForFirstRequest(StubHttpResponse(HttpStatus::OK, bodyJson.toStyledString()));

    auto client = ImsClient::Create(StubClientInfo(), GetHandlerPtr());
    auto result = client->RequestToken(Credentials("Foo", "Boo"))->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(tokenStr, result.GetValue()->AsString());
    }

TEST_F(ImsClientTests, GetToken_ByCredentials_SendsRequestToRetrieveToken)
    {
    GetHandler().ForFirstRequest([&] (Http::RequestCR request)
        {
        auto bodyJson = Json::Reader::DoParse(request.GetRequestBody()->AsString());

        EXPECT_EQ("http://applies.to.url", bodyJson["AppliesTo"].asString());
        EXPECT_EQ("", bodyJson["ActAs"].asString());
        EXPECT_EQ("123", bodyJson["Lifetime"].asString());

        EXPECT_EQ("Basic " + Base64Utilities::Encode("Foo:Boo"), request.GetHeaders().GetAuthorization());

        return StubHttpResponse();
        });

    auto client = ImsClient::Create(StubClientInfo(), GetHandlerPtr());
    client->RequestToken(Credentials("Foo", "Boo"), "http://applies.to.url", 123)->GetResult();
    }

TEST_F(ImsClientTests, GetToken_ByCredentialsWithZeroLifetime_SendsRequestToRetrieveTokenWithoutLifetimee)
    {
    GetHandler().ForFirstRequest([&] (Http::RequestCR request)
        {
        auto bodyJson = Json::Reader::DoParse(request.GetRequestBody()->AsString());

        EXPECT_EQ("http://applies.to.url", bodyJson["AppliesTo"].asString());
        EXPECT_EQ("", bodyJson["ActAs"].asString());
        EXPECT_FALSE(bodyJson.isMember("Lifetime"));

        EXPECT_EQ("Basic " + Base64Utilities::Encode("Foo:Boo"), request.GetHeaders().GetAuthorization());

        return StubHttpResponse();
        });

    auto client = ImsClient::Create(StubClientInfo(), GetHandlerPtr());
    client->RequestToken(Credentials("Foo", "Boo"), "http://applies.to.url", 0)->GetResult();
    }

TEST_F(ImsClientTests, GetToken_ByParentToken_SendsRequestToRetrieveToken)
    {
    SamlToken parentToken(StubSamlTokenXML(0, "TestCert"));

    GetHandler().ForFirstRequest([&] (Http::RequestCR request)
        {
        auto bodyJson = Json::Reader::DoParse(request.GetRequestBody()->AsString());

        EXPECT_EQ("http://applies.to.url", bodyJson["AppliesTo"].asString());
        EXPECT_EQ("http://applies.to.url", bodyJson["AppliesToBootstrapToken"].asString());
        EXPECT_EQ(parentToken.AsString(), bodyJson["ActAs"].asString());
        EXPECT_EQ("123", bodyJson["Lifetime"].asString());

        EXPECT_STREQ("X509 access_token=TestCert", request.GetHeaders().GetAuthorization());

        return StubHttpResponse();
        });

    auto client = ImsClient::Create(StubClientInfo(), GetHandlerPtr());
    client->RequestToken(parentToken, "http://applies.to.url", 123)->GetResult();
    }

TEST_F(ImsClientTests, GetToken_ByParentTokenWithZeroLifetime_SendsRequestToRetrieveTokenWithoutLifetime)
    {
    SamlToken parentToken(StubSamlTokenXML(0, "TestCert"));

    GetHandler().ForFirstRequest([&] (Http::RequestCR request)
        {
        auto bodyJson = Json::Reader::DoParse(request.GetRequestBody()->AsString());

        EXPECT_EQ("http://applies.to.url", bodyJson["AppliesTo"].asString());
        EXPECT_EQ("http://applies.to.url", bodyJson["AppliesToBootstrapToken"].asString());
        EXPECT_EQ(parentToken.AsString(), bodyJson["ActAs"].asString());
        EXPECT_FALSE(bodyJson.isMember("Lifetime"));

        EXPECT_STREQ("X509 access_token=TestCert", request.GetHeaders().GetAuthorization());

        return StubHttpResponse();
        });

    auto client = ImsClient::Create(StubClientInfo(), GetHandlerPtr());
    client->RequestToken(parentToken, "http://applies.to.url", 0)->GetResult();
    }

TEST_F(ImsClientTests, IsLoginRedirect_NonLoginUrl_False)
    {
    EXPECT_FALSE(ImsClient::IsLoginRedirect(StubHttpResponseWithUrl(HttpStatus::OK, "http://test.com/other")));
    }

TEST_F(ImsClientTests, IsLoginRedirect_StatusNotOK_False)
    {
    EXPECT_FALSE(ImsClient::IsLoginRedirect(StubHttpResponseWithUrl(HttpStatus::Found, "http://test.com//IMS/Account/Login?foo")));
    }

TEST_F(ImsClientTests, IsLoginRedirect_IMSLoginUrlAndStatusOK_True)
    {
    EXPECT_TRUE(ImsClient::IsLoginRedirect(StubHttpResponseWithUrl(HttpStatus::OK, "http://test.com//IMS/Account/Login?foo")));
    }

TEST_F(ImsClientTests, GetFederatedSignInUrl_NoDomainParameter_UrlHasNoOfhParameter)
    {
#if defined (BENTLEY_WIN32)
    const Utf8String platformType("desktop");
#else
    const Utf8String platformType("mobile");
#endif

    Utf8String signInUrl("TestUrl?wa=wsignin1.0&wtrealm=sso%3A%2F%2Fwsfed_");
    signInUrl += platformType + "%2FTestAppProductId";

    auto info = StubClientInfo();
    EXPECT_STREQ(signInUrl.c_str(), ImsClient::GetFederatedSignInUrl(*info).c_str());
    }

TEST_F(ImsClientTests, GetFederatedSignInUrl_DomainParameter_UrlHasOfhParameter)
    {
#if defined (BENTLEY_WIN32)
    const Utf8String platformType("desktop");
#else
    const Utf8String platformType("mobile");
#endif

    const Utf8String domainName("bentley");
    Utf8String signInUrl("TestUrl?wa=wsignin1.0&wtrealm=sso%3A%2F%2Fwsfed_");
    signInUrl += platformType + "%2FTestAppProductId&ofh=" + Base64Utilities::Encode(domainName);

    auto info = StubClientInfo();
    EXPECT_STREQ(signInUrl.c_str(), ImsClient::GetFederatedSignInUrl(*info, domainName).c_str());
    }
