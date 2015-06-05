/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Connect/ConnectAuthenticationPersistenceTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ConnectAuthenticationPersistenceTests.h"

#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include "ConnectTestsHelper.h"
#include "StubLocalState.h"
#include "StubSecureStore.h"
#include "MockLocalState.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
using namespace ::testing;

TEST_F (ConnectAuthenticationPersistenceTests, GetCredentials_SetCredentialsCalledOnOtherPersistenceWithSameLocalState_ReturnsSameCredentials)
    {
    StubLocalState localState;
    auto secureStore = std::make_shared<StubSecureStore> ();
    ConnectAuthenticationPersistence p1 (&localState, secureStore);
    ConnectAuthenticationPersistence p2 (&localState, secureStore);

    p1.SetCredentials ({"A", "B"});
    EXPECT_EQ (Credentials ("A", "B"), p2.GetCredentials ());
    }

TEST_F (ConnectAuthenticationPersistenceTests, GetToken_SetTokenCalledOnOtherPersistenceOnOtherPersistenceWithSameLocalState_ReturnsSameToken)
    {
    StubLocalState localState;
    auto secureStore = std::make_shared<StubSecureStore> ();
    ConnectAuthenticationPersistence p1 (&localState, secureStore);
    ConnectAuthenticationPersistence p2 (&localState, secureStore);

    auto token = StubSamlToken ();
    EXPECT_FALSE (token->AsString ().empty ());

    p1.SetToken (token);
    ASSERT_NE (nullptr, p2.GetToken ());
    EXPECT_EQ (token->AsString (), p2.GetToken ()->AsString ());
    }

TEST_F (ConnectAuthenticationPersistenceTests, GetCredentials_CredentialsStoredInOldLocation_SavesToSecureStoreAndDeletesThemFromOldLocation)
    {
    StubLocalState localState;
    auto secureStore = std::make_shared<StubSecureStore> ();
    ConnectAuthenticationPersistence p1 (&localState, secureStore);
    ConnectAuthenticationPersistence p2 (&localState, secureStore);

    localState.SaveValue ("Connect", "Username", "TestUsername");
    secureStore->legacyValues["ConnectLogin"]["TestUsername"] = "TestPassword";
    secureStore->legacyValues["ConnectToken"]["Token"] = "TestToken";

    auto credentials = p1.GetCredentials ();
    EXPECT_STREQ ("TestUsername", credentials.GetUsername ().c_str ());
    EXPECT_STREQ ("TestPassword", credentials.GetPassword ().c_str ());

    EXPECT_TRUE (localState.GetValue ("Connect", "Username").isNull ());
    EXPECT_STREQ ("", secureStore->legacyValues["ConnectLogin"]["TestUsername"].asCString ());
    EXPECT_STREQ ("", secureStore->legacyValues["ConnectToken"]["Token"].asCString ());

    EXPECT_STREQ ("TestUsername", secureStore->values["Connect"]["Username"].asString ().c_str ());
    EXPECT_STREQ ("TestPassword", secureStore->values["Connect"]["Password"].asString ().c_str ());
    EXPECT_STREQ ("TestToken", secureStore->values["Connect"]["Token"].asString ().c_str ());

    // Load from new storage
    credentials = p2.GetCredentials ();
    EXPECT_STREQ ("TestUsername", credentials.GetUsername ().c_str ());
    EXPECT_STREQ ("TestPassword", credentials.GetPassword ().c_str ());
    }
