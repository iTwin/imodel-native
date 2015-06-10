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

TEST_F (ConnectAuthenticationPersistenceTests, GetShared_CalledTwice_ReturnsSameCredentials)
    {
    StubLocalState localState;
    auto secureStore = std::make_shared<StubSecureStore> ();
    ConnectAuthenticationPersistence::CustomInitialize (&localState, secureStore);

    auto p1 = ConnectAuthenticationPersistence::GetShared ();
    auto p2 = ConnectAuthenticationPersistence::GetShared ();

    EXPECT_EQ (p1, p2);
    }

TEST_F (ConnectAuthenticationPersistenceTests, GetToken_SetTokenCalled_ReturnsSameToken)
    {
    StubLocalState localState;
    auto secureStore = std::make_shared<StubSecureStore> ();
    ConnectAuthenticationPersistence::CustomInitialize (&localState, secureStore);
    auto persistence = ConnectAuthenticationPersistence::GetShared ();

    auto token = StubSamlToken ();
    persistence->SetToken (token);
    ASSERT_NE (nullptr, persistence->GetToken ());
    EXPECT_EQ (token->AsString (), persistence->GetToken ()->AsString ());
    }

TEST_F (ConnectAuthenticationPersistenceTests, GetCredentials_CredentialsStoredInOldLocation_SavesToSecureStoreAndDeletesThemFromOldLocation)
    {
    StubLocalState localState;
    auto secureStore = std::make_shared<StubSecureStore> ();
    ConnectAuthenticationPersistence::CustomInitialize (&localState, secureStore);
    auto persistence = ConnectAuthenticationPersistence::GetShared ();

    localState.SaveValue ("Connect", "Username", "TestUsername");
    secureStore->legacyValues["ConnectLogin"]["TestUsername"] = "TestPassword";
    secureStore->legacyValues["ConnectToken"]["Token"] = "TestToken";

    auto credentials = persistence->GetCredentials ();
    EXPECT_STREQ ("TestUsername", credentials.GetUsername ().c_str ());
    EXPECT_STREQ ("TestPassword", credentials.GetPassword ().c_str ());

    EXPECT_TRUE (localState.GetValue ("Connect", "Username").isNull ());
    EXPECT_STREQ ("", secureStore->legacyValues["ConnectLogin"]["TestUsername"].asCString ());
    EXPECT_STREQ ("", secureStore->legacyValues["ConnectToken"]["Token"].asCString ());

    EXPECT_STREQ ("TestUsername", secureStore->values["Connect"]["Username"].asString ().c_str ());
    EXPECT_STREQ ("TestPassword", secureStore->values["Connect"]["Password"].asString ().c_str ());
    EXPECT_STREQ ("TestToken", secureStore->values["Connect"]["Token"].asString ().c_str ());
    }
