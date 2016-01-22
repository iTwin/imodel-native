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

void ConnectTests::SetUp ()
    {
    Connect::Initialize (StubClientInfo (), GetHandlerPtr ());
    m_client = std::make_shared<StubBuddiClient>();
    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &m_localState, m_client);
    }

void ConnectTests::TearDown ()
    {
    Connect::Uninintialize ();
    }

TEST_F (ConnectTests, Login_DefaultUrls_SendsRequestToRetrieveToken)
    {
    GetHandler ().ForFirstRequest ([&] (HttpRequestCR request)
        {
        EXPECT_STREQ ("TestUrl", request.GetUrl ().c_str ());
        EXPECT_EQ ("Basic " + Base64Utilities::Encode ("Foo:Boo"), request.GetHeaders ().GetAuthorization ());
        return StubHttpResponse ();
        });

    SamlToken token;
    Connect::Login (Credentials ("Foo", "Boo"), token);
    }

TEST_F (ConnectTests, Login_ResponseContainsToken_ReturnsToken)
    {
    auto tokenStr = StubSamlTokenXML ();
    Json::Value bodyJson;
    bodyJson["RequestedSecurityToken"] = tokenStr;

    GetHandler ().ForFirstRequest (StubHttpResponse (HttpStatus::OK, bodyJson.toStyledString ()));

    SamlToken token;
    auto status = Connect::Login (Credentials ("Foo", "Boo"), token);

    EXPECT_EQ (SUCCESS, status);
    EXPECT_EQ (tokenStr, token.AsString ());
    }

TEST_F (ConnectTests, GetStsToken_ByCredentials_SendsRequestToRetrieveToken)
    {
    GetHandler ().ForFirstRequest ([&] (HttpRequestCR request)
        {
        auto bodyJson = request.GetRequestBody ()->AsJson ();

        EXPECT_EQ ("http://applies.to.url", bodyJson["AppliesTo"].asString ());
        EXPECT_EQ ("", bodyJson["ActAs"].asString ());
        EXPECT_FALSE (bodyJson["Lifetime"].asString ().empty ());

        EXPECT_EQ ("Basic " + Base64Utilities::Encode ("Foo:Boo"), request.GetHeaders ().GetAuthorization ());
        EXPECT_EQ ("http://sts.url", request.GetUrl ());

        return StubHttpResponse ();
        });

    SamlToken token;
    Connect::GetStsToken (Credentials ("Foo", "Boo"), token, "http://applies.to.url", "http://sts.url");
    }

TEST_F (ConnectTests, GetStsToken_ByParentToken_SendsRequestToRetrieveToken)
    {
    SamlToken parentToken (StubSamlTokenXML (0, "TestCert"));

    GetHandler ().ForFirstRequest ([&] (HttpRequestCR request)
        {
        auto bodyJson = request.GetRequestBody ()->AsJson ();

        EXPECT_EQ ("http://applies.to.url", bodyJson["AppliesTo"].asString ());
        EXPECT_EQ ("http://applies.to.url", bodyJson["AppliesToBootstrapToken"].asString ());
        EXPECT_EQ (parentToken.AsString (), bodyJson["ActAs"].asString ());

        EXPECT_FALSE (bodyJson["Lifetime"].asString ().empty ());

        EXPECT_STREQ ("X509 access_token=TestCert", request.GetHeaders ().GetAuthorization ());
        EXPECT_EQ ("http://sts.url", request.GetUrl ());

        return StubHttpResponse ();
        });

    SamlToken token;
    Connect::GetStsToken (parentToken, token, "http://applies.to.url", "http://sts.url");
    }

TEST_F (ConnectTests, IsImsLoginRedirect_NonLoginUrl_False)
    {
    EXPECT_FALSE (Connect::IsImsLoginRedirect (StubHttpResponseWithUrl (HttpStatus::OK, "http://test.com/other")));
    }

TEST_F (ConnectTests, IsImsLoginRedirect_StatusNotOK_False)
    {
    EXPECT_FALSE (Connect::IsImsLoginRedirect (StubHttpResponseWithUrl (HttpStatus::Found, "http://test.com//IMS/Account/Login?foo")));
    }

TEST_F (ConnectTests, IsImsLoginRedirect_IMSLoginUrlAndStatusOK_True)
    {
    EXPECT_TRUE (Connect::IsImsLoginRedirect (StubHttpResponseWithUrl (HttpStatus::OK, "http://test.com//IMS/Account/Login?foo")));
    }

TEST_F (ConnectTests, GetFederatedSignInUrl)
    {
    // App's relying party URI is URL-encoded (aside from the '_') and passed as the wtrealm parameter.
#if defined (BENTLEY_WIN32)
    EXPECT_STREQ ("TestUrl?wa=wsignin1.0&wtrealm=sso%3A%2F%2Fwsfed_desktop%2FTestAppProductId", Connect::GetFederatedSignInUrl().c_str());
#else
    EXPECT_STREQ ("TestUrl?wa=wsignin1.0&wtrealm=sso%3A%2F%2Fwsfed_mobile%2FTestAppProductId", Connect::GetFederatedSignInUrl().c_str());
#endif
    }

TEST_F (ConnectTests, GetFederatedSignInUrlWithDomain)
    {
    // App's relying party URI is URL-encoded (aside from the '_') and passed as the wtrealm parameter.
    // "YmVudGxleS5jb20=" (ofh parameter) is "bentley.com" in Base 64.
#if defined (BENTLEY_WIN32)
    EXPECT_STREQ ("TestUrl?wa=wsignin1.0&wtrealm=sso%3A%2F%2Fwsfed_desktop%2FTestAppProductId&ofh=YmVudGxleS5jb20=", Connect::GetFederatedSignInUrl("bentley.com").c_str());
#else
    EXPECT_STREQ ("TestUrl?wa=wsignin1.0&wtrealm=sso%3A%2F%2Fwsfed_mobile%2FTestAppProductId&ofh=YmVudGxleS5jb20=", Connect::GetFederatedSignInUrl("bentley.com").c_str());
#endif
    }
