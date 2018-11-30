/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/WebServices/Connect/IdentityAuthenticationPersistenceTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "IdentityAuthenticationPersistenceTests.h"

#include "../../../../../Connect/IdentityAuthenticationPersistence.h"
#include "ConnectTestsHelper.h"
#include "StubLocalState.h"
#include "StubSecureStore.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
using namespace ::testing;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IdentityAuthenticationPersistenceTests, GetToken_SetTokenNotCalled_ReturnsNull)
    {
    StubLocalState localState;
    IdentityAuthenticationPersistence persistence(&localState, std::make_shared<StubSecureStore>());

    ASSERT_EQ(nullptr, persistence.GetToken());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IdentityAuthenticationPersistenceTests, GetToken_SetTokenWithNullCalled_ReturnsNull)
    {
    StubLocalState localState;
    IdentityAuthenticationPersistence persistence(&localState, std::make_shared<StubSecureStore>());

    persistence.SetToken(StubSamlToken());
    persistence.SetToken(nullptr);
    ASSERT_EQ(nullptr, persistence.GetToken());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IdentityAuthenticationPersistenceTests, GetToken_SetTokenCalled_ReturnsSameToken)
    {
    StubLocalState localState;
    IdentityAuthenticationPersistence persistence(&localState, std::make_shared<StubSecureStore>());

    auto token = StubSamlToken();
    persistence.SetToken(token);
    ASSERT_NE(nullptr, persistence.GetToken());
    EXPECT_EQ(*token, *persistence.GetToken());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IdentityAuthenticationPersistenceTests, GetTokenSetTime_SetTokenNotCalled_ReturnsInvalid)
    {
    StubLocalState localState;
    IdentityAuthenticationPersistence persistence(&localState, std::make_shared<StubSecureStore>());

    EXPECT_EQ(DateTime(), persistence.GetTokenSetTime());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IdentityAuthenticationPersistenceTests, GetTokenSetTime_SetTokenCalledWithNull_ReturnsInvalid)
    {
    StubLocalState localState;
    IdentityAuthenticationPersistence persistence(&localState, std::make_shared<StubSecureStore>());

    persistence.SetToken(StubSamlToken());
    persistence.SetToken(nullptr);
    EXPECT_EQ(DateTime(), persistence.GetTokenSetTime());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IdentityAuthenticationPersistenceTests, GetTokenSetTime_SetTokenCalled_ReturnsSetTime)
    {
    StubLocalState localState;
    IdentityAuthenticationPersistence persistence(&localState, std::make_shared<StubSecureStore>());

    auto before = DateTime::GetCurrentTimeUtc();
    persistence.SetToken(StubSamlToken());
    auto after = DateTime::GetCurrentTimeUtc();

    auto setTime = persistence.GetTokenSetTime();
    EXPECT_TRUE(setTime.IsValid());
    EXPECT_BETWEEN(before, setTime, after);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IdentityAuthenticationPersistenceTests, GetCredentials_Always_ReturnsEmpty)
    {
    StubLocalState localState;
    IdentityAuthenticationPersistence persistence(&localState, std::make_shared<StubSecureStore>());

    EXPECT_TRUE(persistence.GetCredentials().IsEmpty());
    persistence.SetCredentials(Credentials("Foo", "Boo"));
    EXPECT_TRUE(persistence.GetCredentials().IsEmpty());
    }
