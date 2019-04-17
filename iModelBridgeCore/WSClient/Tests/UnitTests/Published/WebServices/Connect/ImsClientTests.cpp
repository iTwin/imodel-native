/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImsClientTests, GetToken_DefaultUrls_SendsRequestToRetrieveToken)
    {
    GetHandler().ForFirstRequest([&] (Http::RequestCR request)
        {
        EXPECT_STREQ("https://test/foo", request.GetUrl().c_str());
        EXPECT_EQ("Basic " + Base64Utilities::Encode("Foo:Boo"), request.GetHeaders().GetAuthorization());
        return StubHttpResponse();
        });

    auto client = ImsClient::Create(StubClientInfo(), GetHandlerPtr());
    client->RequestToken(Credentials("Foo", "Boo"))->GetResult();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImsClientTests, GetToken_ResponseContainsToken_ReturnsToken)
    {
    auto tokenStr = StubSamlTokenWithUser("Foo")->AsString();
    Json::Value bodyJson;
    bodyJson["RequestedSecurityToken"] = tokenStr;

    GetHandler().ForFirstRequest(StubHttpResponse(HttpStatus::OK, bodyJson.toStyledString()));

    auto client = ImsClient::Create(StubClientInfo(), GetHandlerPtr());
    auto result = client->RequestToken(Credentials("Foo", "Boo"))->GetResult();

    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(tokenStr, result.GetValue()->AsString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImsClientTests, GetToken_MismatchingUserName_ReturnsError)
    {
    auto tokenStr = StubSamlTokenWithUser("Boo")->AsString();
    Json::Value bodyJson;
    bodyJson["RequestedSecurityToken"] = tokenStr;

    GetHandler().ForFirstRequest(StubHttpResponse(HttpStatus::OK, bodyJson.toStyledString()));

    auto client = ImsClient::Create(StubClientInfo(), GetHandlerPtr());
    auto result = client->RequestToken(Credentials("Foo", "Boo"))->GetResult();

    ASSERT_FALSE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImsClientTests, GetToken_ByCredentialsToAnyEnvironment_ValidatesCertificate)
    {
    GetHandler().ForAnyRequest([&] (Http::RequestCR request)
        {
        EXPECT_TRUE(request.GetValidateCertificate());
        return StubHttpResponse();
        });

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &m_localState, m_buddiClient);
    auto client = ImsClient::Create(StubClientInfo(), GetHandlerPtr());
    client->RequestToken(Credentials("Foo", "Boo"))->Wait();
    EXPECT_EQ(1, GetHandler().GetRequestsPerformed());

    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, &m_localState, m_buddiClient);
    client = ImsClient::Create(StubClientInfo(), GetHandlerPtr());
    client->RequestToken(Credentials("Foo", "Boo"))->Wait();
    EXPECT_EQ(2, GetHandler().GetRequestsPerformed());

    UrlProvider::Initialize(UrlProvider::Environment::Release, UrlProvider::DefaultTimeout, &m_localState, m_buddiClient);
    client = ImsClient::Create(StubClientInfo(), GetHandlerPtr());
    client->RequestToken(Credentials("Foo", "Boo"))->Wait();
    EXPECT_EQ(3, GetHandler().GetRequestsPerformed());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImsClientTests, GetToken_ByTokenToAnyEnvironment_ValidatesCertificate)
    {
    GetHandler().ForAnyRequest([&] (Http::RequestCR request)
        {
        EXPECT_TRUE(request.GetValidateCertificate());
        return StubHttpResponse();
        });

    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &m_localState, m_buddiClient);
    auto client = ImsClient::Create(StubClientInfo(), GetHandlerPtr());
    client->RequestToken(*StubSamlToken())->Wait();
    EXPECT_EQ(1, GetHandler().GetRequestsPerformed());

    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, &m_localState, m_buddiClient);
    client = ImsClient::Create(StubClientInfo(), GetHandlerPtr());
    client->RequestToken(*StubSamlToken())->Wait();
    EXPECT_EQ(2, GetHandler().GetRequestsPerformed());

    UrlProvider::Initialize(UrlProvider::Environment::Release, UrlProvider::DefaultTimeout, &m_localState, m_buddiClient);
    client = ImsClient::Create(StubClientInfo(), GetHandlerPtr());
    client->RequestToken(*StubSamlToken())->Wait();
    EXPECT_EQ(3, GetHandler().GetRequestsPerformed());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImsClientTests, IsLoginRedirect_NonLoginUrl_False)
    {
    EXPECT_FALSE(ImsClient::IsLoginRedirect(StubHttpResponseWithUrl(HttpStatus::OK, "http://test.com/other")));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImsClientTests, IsLoginRedirect_StatusNotOK_False)
    {
    EXPECT_FALSE(ImsClient::IsLoginRedirect(StubHttpResponseWithUrl(HttpStatus::Found, "http://test.com//IMS/Account/Login?foo")));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImsClientTests, IsLoginRedirect_IMSLoginUrlAndStatusOK_True)
    {
    EXPECT_TRUE(ImsClient::IsLoginRedirect(StubHttpResponseWithUrl(HttpStatus::OK, "http://test.com//IMS/Account/Login?foo")));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImsClientTests, GetFederatedSignInUrl_NoDomainParameter_UrlHasNoOfhParameter)
    {
#if defined (BENTLEY_WIN32)
    const Utf8String platformType("desktop");
#else
    const Utf8String platformType("mobile");
#endif

    Utf8String signInUrl("https://test/foo?wa=wsignin1.0&wtrealm=sso%3A%2F%2Fwsfed_");
    signInUrl += platformType + "%2FTestAppProductId";

    auto info = StubClientInfo();
    EXPECT_STREQ(signInUrl.c_str(), ImsClient::GetFederatedSignInUrl(*info).c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImsClientTests, GetFederatedSignInUrl_DomainParameter_UrlHasOfhParameter)
    {
#if defined (BENTLEY_WIN32)
    const Utf8String platformType("desktop");
#else
    const Utf8String platformType("mobile");
#endif

    const Utf8String domainName("bentley");
    Utf8String signInUrl("https://test/foo?wa=wsignin1.0&wtrealm=sso%3A%2F%2Fwsfed_");
    signInUrl += platformType + "%2FTestAppProductId&ofh=" + Base64Utilities::Encode(domainName);

    auto info = StubClientInfo();
    EXPECT_STREQ(signInUrl.c_str(), ImsClient::GetFederatedSignInUrl(*info, domainName).c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Tomas.Tamasauskas    08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImsClientTests, GetA2PUrl)
    {
    GetHandler().ForFirstRequest([&](Http::RequestCR request)
        {
        EXPECT_STREQ((UrlProvider::Urls::ImsActiveSTSHelperService.Get() + "/json/RegisterToken").c_str(), request.GetUrl().c_str());
        Utf8PrintfString credsPair("%s:%s", "BentleyConnectAppServiceUser@bentley.com", "A6u6I09FP70YQWHlbrfS0Ct2fTyIMt6JNnMtbjHSx6smCgSinlRFCXqM6wcuYuj");
        Utf8PrintfString authorization("Basic %s", Base64Utilities::Encode(credsPair).c_str());
        EXPECT_EQ(authorization, request.GetHeaders().GetAuthorization());
        return StubHttpResponse(HttpStatus::OK, Utf8PrintfString("\"%s\"", BeGuid(true).ToString().c_str()));
        });

    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, &m_localState, m_buddiClient);
    auto client = ImsClient::Create(StubClientInfo(), GetHandlerPtr());
    auto url = client->GetA2PUrl("https://something.bentley.com", StubSamlToken(), BeGuid(true).ToString());
    EXPECT_EQ(1, GetHandler().GetRequestsPerformed());
    EXPECT_NE("https://something.bentley.com", url);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                Tomas.Tamasauskas    08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImsClientTests, GetA2PUrl_FailsToRegisterToken)
    {
    GetHandler().ForFirstRequest([&](Http::RequestCR request)
        {
        return StubHttpResponse(HttpStatus::Unauthorized);
        });

    UrlProvider::Initialize(UrlProvider::Environment::Qa, UrlProvider::DefaultTimeout, &m_localState, m_buddiClient);
    auto client = ImsClient::Create(StubClientInfo(), GetHandlerPtr());
    BeTest::SetFailOnAssert(false);
    auto url = client->GetA2PUrl("https://something.bentley.com", StubSamlToken(), BeGuid(true).ToString());
    BeTest::SetFailOnAssert(true);
    EXPECT_EQ(1, GetHandler().GetRequestsPerformed());
    EXPECT_EQ("https://something.bentley.com", url);
    }
