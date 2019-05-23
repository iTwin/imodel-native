/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ConnectTokenProviderTests.h"
#include <WebServices/Connect/ConnectTokenProvider.h>
#include <WebServices/Connect/ImsClient.h>
#include <Bentley/Base64Utilities.h>
#include <WebServices/Configuration/UrlProvider.h>
#include "MockConnectAuthenticationPersistence.h"
#include "MockImsClient.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
using namespace ::testing;

#ifdef USE_GTEST
void ConnectTokenProviderTests::SetUp()
    {
    WSClientBaseTest::SetUp();
    m_buddiClient = std::make_shared<StubBuddiClient>();
    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &m_localState, m_buddiClient);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectTokenProviderTests, GetToken_PersistedToken_ReturnsTokenFromPersistence)
    {
    auto persistence = std::make_shared<MockConnectAuthenticationPersistence>();
    auto client = std::make_shared<MockImsClient>();
    ConnectTokenProvider provider(client, persistence);

    SamlTokenPtr token = StubSamlToken(100);
    EXPECT_CALL(*persistence, GetToken()).WillOnce(Return(token));

    EXPECT_EQ(token, provider.GetToken());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectTokenProviderTests, GetToken_NotPersistedToken_ReturnsNullptr)
    {
    auto persistence = std::make_shared<MockConnectAuthenticationPersistence>();
    auto client = std::make_shared<MockImsClient>();
    ConnectTokenProvider provider(client, persistence);

    EXPECT_CALL(*persistence, GetToken()).WillOnce(Return(nullptr));

    EXPECT_EQ(nullptr, provider.GetToken());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectTokenProviderTests, UpdateToken_CredentialsNotSet_ReturnsNull)
    {
    auto persistence = std::make_shared<MockConnectAuthenticationPersistence>();
    auto client = std::make_shared<MockImsClient>();
    ConnectTokenProvider provider(client, persistence);

    EXPECT_CALL(*persistence, GetCredentials()).WillOnce(Return(Credentials()));

    EXPECT_EQ(nullptr, provider.UpdateToken()->GetResult());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectTokenProviderTests, UpdateToken_CredentialsSet_CallsImsServerToRetrieveToken)
    {
    auto persistence = std::make_shared<MockConnectAuthenticationPersistence>();
    auto client = std::make_shared<MockImsClient>();
    ConnectTokenProvider provider(client, persistence);

    Credentials creds("TestUser", "TestPass");
    EXPECT_CALL(*persistence, GetCredentials()).WillOnce(Return(creds));
    EXPECT_CALL(*client, RequestToken(creds, _, _)).WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Error(HttpError()))));

    provider.UpdateToken()->GetResult();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectTokenProviderTests, UpdateToken_TokenRequestReturnsError_Nullptr)
    {
    auto persistence = std::make_shared<MockConnectAuthenticationPersistence>();
    auto client = std::make_shared<MockImsClient>();
    ConnectTokenProvider provider(client, persistence);

    Credentials creds("TestUser", "TestPass");
    EXPECT_CALL(*persistence, GetCredentials()).WillOnce(Return(creds));
    EXPECT_CALL(*client, RequestToken(creds, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Error(HttpError(StubHttpResponse(ConnectionStatus::CouldNotConnect))))));

    EXPECT_EQ(nullptr, provider.UpdateToken()->GetResult());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectTokenProviderTests, UpdateToken_CredentialsSetAndTokenRecieved_SetsTokenToPersistenceAndReturns)
    {
    auto persistence = std::make_shared<MockConnectAuthenticationPersistence>();
    auto client = std::make_shared<MockImsClient>();
    ConnectTokenProvider provider(client, persistence);

    SamlTokenPtr newToken = StubSamlToken(100);

    Credentials creds("TestUser", "TestPass");
    EXPECT_CALL(*persistence, GetCredentials()).WillOnce(Return(creds));
    EXPECT_CALL(*persistence, SetToken(_)).WillOnce(Invoke([&] (SamlTokenPtr token)
        {
        EXPECT_EQ(newToken->AsString(), token->AsString());
        }));

    EXPECT_CALL(*client, RequestToken(creds, _, _))
        .WillOnce(Return(CreateCompletedAsyncTask(SamlTokenResult::Success(newToken))));

    auto updatedToken = provider.UpdateToken()->GetResult();
    EXPECT_EQ(newToken->AsString(), updatedToken->AsString());
    }
#endif
