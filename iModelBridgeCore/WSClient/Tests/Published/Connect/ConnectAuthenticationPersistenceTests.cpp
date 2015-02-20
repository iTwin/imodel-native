/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Connect/ConnectAuthenticationPersistenceTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ConnectAuthenticationPersistenceTests.h"

#include <WebServices/Client/Connect/ConnectAuthenticationPersistence.h>
#include "ConnectTestsHelper.h"
#include "StubLocalState.h"
#include "MockLocalState.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
using namespace ::testing;

TEST_F (ConnectAuthenticationPersistenceTests, GetCredentials_SetCredentialsCalledOnOtherPersistenceWithSameLocalState_ReturnsSameCredentials)
    {
    StubLocalState localState;
    ConnectAuthenticationPersistence p1 (&localState);
    ConnectAuthenticationPersistence p2 (&localState);

    p1.SetCredentials ({"A", "B"});
    EXPECT_EQ (Credentials ("A", "B"), p2.GetCredentials ());
    }

TEST_F (ConnectAuthenticationPersistenceTests, GetToken_SetTokenCalledOnOtherPersistenceOnOtherPersistenceWithSameLocalState_ReturnsSameToken)
    {
    StubLocalState localState;
    ConnectAuthenticationPersistence p1 (&localState);
    ConnectAuthenticationPersistence p2 (&localState);

    auto token = StubSamlToken ();
    EXPECT_FALSE (token->AsString ().empty ());

    p1.SetToken (token);
    EXPECT_EQ (token->AsString (), p2.GetToken ()->AsString ());
    }

#ifdef USE_GTEST
TEST_F (ConnectAuthenticationPersistenceTests, SetCredentials_CredentialsPassed_EncodedCredentialsSavedToLocalState)
    {
    MockLocalState localState;
    ConnectAuthenticationPersistence persistence (&localState);

    auto token = StubSamlToken ();
    EXPECT_FALSE (token->AsString ().empty ());

    EXPECT_CALL (localState, SaveValue (_, _, _))
        .WillOnce (Invoke ([&] (Utf8CP, Utf8CP, JsonValueCR value)
        {
        EXPECT_TRUE (value.isString ());
        EXPECT_EQ ("TestUser", value.asString ());
        }))
        .WillOnce (Invoke ([&] (Utf8CP, Utf8CP propertyName, JsonValueCR value)
        {
        EXPECT_TRUE (value.isString ());
        EXPECT_STRNE ("TestUser", propertyName);
        EXPECT_THAT (propertyName, EndsWith ("TestUser"));
        EXPECT_THAT (value.asString ().c_str (), Not (HasSubstr ("TestPassword")));
        }));

    persistence.SetCredentials ({"TestUser", "TestPassword"});
    }

TEST_F (ConnectAuthenticationPersistenceTests, SetToken_TokenPassed_EncodedTokenSavedToLocalState)
    {
    MockLocalState localState;
    ConnectAuthenticationPersistence persistence (&localState);

    auto token = StubSamlToken ();
    EXPECT_FALSE (token->AsString ().empty ());

    EXPECT_CALL (localState, SaveValue (_, _, _)).WillOnce (Invoke ([&] (Utf8CP, Utf8CP, JsonValueCR value)
        {
        EXPECT_TRUE (value.isString ());
        EXPECT_THAT (value.asString ().c_str (), Not (HasSubstr (token->AsString ().c_str ())));
        }));

    persistence.SetToken (token);
    }
#endif