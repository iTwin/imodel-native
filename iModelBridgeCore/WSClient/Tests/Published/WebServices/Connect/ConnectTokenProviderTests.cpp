/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Connect/ConnectTokenProviderTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConnectTokenProviderTests.h"
#include <WebServices/Connect/ConnectTokenProvider.h>
#include <WebServices/Connect/Connect.h>
#include <Bentley/Base64Utilities.h>
#include "MockConnectAuthenticationPersistence.h"
#include <WebServices/Configuration/UrlProvider.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
using namespace ::testing;

#ifdef USE_GTEST
void ConnectTokenProviderTests::SetUp ()
    {
    Connect::Initialize (StubClientInfo (), GetHandlerPtr ());
    m_client = std::make_shared<StubBuddiClient>();
    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &m_localState, m_client);
    }

void ConnectTokenProviderTests::TearDown ()
    {
    Connect::Uninintialize ();
    }

TEST_F (ConnectTokenProviderTests, GetToken_PersistedToken_ReturnsTokenFromPersistence)
    {
    auto persistence = std::make_shared<MockConnectAuthenticationPersistence> ();
    ConnectTokenProvider provider (persistence);

    SamlTokenPtr token = StubSamlToken (100);
    EXPECT_CALL (*persistence, GetToken ()).WillOnce (Return (token));

    EXPECT_EQ (token, provider.GetToken ());
    }

TEST_F (ConnectTokenProviderTests, GetToken_NotPersistedToken_ReturnsNullptr)
    {
    auto persistence = std::make_shared<MockConnectAuthenticationPersistence> ();
    ConnectTokenProvider provider (persistence);

    EXPECT_CALL (*persistence, GetToken ()).WillOnce (Return (nullptr));

    EXPECT_EQ (nullptr, provider.GetToken ());
    }

TEST_F (ConnectTokenProviderTests, UpdateToken_CredentialsNotSet_ReturnsNull)
    {
    auto persistence = std::make_shared<MockConnectAuthenticationPersistence> ();
    ConnectTokenProvider provider (persistence);

    EXPECT_CALL (*persistence, GetCredentials ()).WillOnce (Return (Credentials ()));

    EXPECT_EQ (nullptr, provider.UpdateToken ());
    }

TEST_F (ConnectTokenProviderTests, UpdateToken_CredentialsSet_CallsImsServerToRetrieveToken)
    {
    auto persistence = std::make_shared<MockConnectAuthenticationPersistence> ();
    ConnectTokenProvider provider (persistence);

    EXPECT_CALL (*persistence, GetCredentials ()).WillOnce (Return (Credentials ("TestUser", "TestPass")));

    GetHandler ().ForRequest (1, [&] (HttpRequestCR request)
        {
        EXPECT_STREQ("TestUrl", request.GetUrl().c_str());
        EXPECT_EQ ("Basic " + Base64Utilities::Encode ("TestUser:TestPass"), request.GetHeaders ().GetAuthorization ());
        return StubHttpResponse ();
        });

    provider.UpdateToken ();
    EXPECT_EQ (1, GetHandler ().GetRequestsPerformed ());
    }

TEST_F (ConnectTokenProviderTests, UpdateToken_TokenRequestReturnsError_Nullptr)
    {
    auto persistence = std::make_shared<MockConnectAuthenticationPersistence> ();
    ConnectTokenProvider provider (persistence);

    EXPECT_CALL (*persistence, GetCredentials ()).WillOnce (Return (Credentials ("TestUser", "TestPass")));

    GetHandler ().ForRequest (1, StubHttpResponse (ConnectionStatus::CouldNotConnect));

    EXPECT_EQ (nullptr, provider.UpdateToken ());
    EXPECT_EQ (1, GetHandler ().GetRequestsPerformed ());
    }

TEST_F (ConnectTokenProviderTests, UpdateToken_CredentialsSetAndTokenRecieved_SetsTokenToPersistenceAndReturns)
    {
    auto persistence = std::make_shared<MockConnectAuthenticationPersistence> ();
    ConnectTokenProvider provider (persistence);

    SamlTokenPtr newToken = StubSamlToken (100);

    EXPECT_CALL (*persistence, GetCredentials ()).WillOnce (Return (Credentials ("TestUser", "TestPass")));
    EXPECT_CALL (*persistence, SetToken (_)).WillOnce (Invoke ([&] (SamlTokenPtr token)
        {
        EXPECT_EQ (newToken->AsString (), token->AsString ());
        }));

    GetHandler ().ForRequest (1, StubImsTokenHttpResponse (*newToken));

    SamlTokenPtr updatedToken = provider.UpdateToken ();
    EXPECT_EQ (newToken->AsString (), updatedToken->AsString ());
    }
#endif
