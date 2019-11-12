/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ConnectTestsHelper.h"
#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include "ConnectTestsHelper.h"
#include "StubSecureStore.h"
#include "MockLocalState.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
using namespace ::testing;

struct ConnectAuthenticationPersistenceTests : WSClientBaseTest {};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationPersistenceTests, GetShared_CalledTwice_ReturnsSameCredentials)
    {
    RuntimeJsonLocalState localState;
    auto secureStore = std::make_shared<StubSecureStore>();
    ConnectAuthenticationPersistence::CustomInitialize(&localState, secureStore);

    auto p1 = ConnectAuthenticationPersistence::GetShared();
    auto p2 = ConnectAuthenticationPersistence::GetShared();

    EXPECT_EQ(p1, p2);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationPersistenceTests, GetToken_SetTokenCalled_ReturnsSameToken)
    {
    RuntimeJsonLocalState localState;
    auto secureStore = std::make_shared<StubSecureStore>();
    ConnectAuthenticationPersistence::CustomInitialize(&localState, secureStore);
    auto persistence = ConnectAuthenticationPersistence::GetShared();

    auto token = StubSamlToken();
    persistence->SetToken(token);
    ASSERT_NE(nullptr, persistence->GetToken());
    EXPECT_EQ(token->AsString(), persistence->GetToken()->AsString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationPersistenceTests, SetCredentials_ListenerRegisteredAndCredentialsStored_CallsListenerBeforeSettingNewCredentials)
    {
    RuntimeJsonLocalState localState;
    auto secureStore = std::make_shared<StubSecureStore>();
    ConnectAuthenticationPersistence::CustomInitialize(&localState, secureStore);
    auto persistence = ConnectAuthenticationPersistence::GetShared();

    persistence->SetCredentials({"User", "Pass"});

    int called = 0;
    persistence->RegisterUserChangedListener([&]
        {
        EXPECT_STREQ("User", secureStore->LoadValue("Connect", "Username").c_str());
        EXPECT_STREQ("Pass", secureStore->LoadValue("Connect", "Password").c_str());
        called++;
        });

    persistence->SetCredentials({"OtherUser", "OtherPass"});

    EXPECT_STREQ("OtherUser", secureStore->LoadValue("Connect", "Username").c_str());
    EXPECT_STREQ("OtherPass", secureStore->LoadValue("Connect", "Password").c_str());
    EXPECT_EQ(1, called);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationPersistenceTests, SetCredentials_ListenerRegisteredAndNoCredentialsStored_DoesNotCallListener)
    {
    RuntimeJsonLocalState localState;
    auto secureStore = std::make_shared<StubSecureStore>();
    ConnectAuthenticationPersistence::CustomInitialize(&localState, secureStore);
    auto persistence = ConnectAuthenticationPersistence::GetShared();

    int called = 0;
    persistence->RegisterUserChangedListener([&]
        {
        called++;
        });

    persistence->SetCredentials({"User", "Pass"});

    EXPECT_STREQ("User", secureStore->LoadValue("Connect", "Username").c_str());
    EXPECT_STREQ("Pass", secureStore->LoadValue("Connect", "Password").c_str());
    EXPECT_EQ(0, called);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationPersistenceTests, SetCredentials_ListenerRegisteredAndAndPasswordChanges_DoesNotCallListener)
    {
    RuntimeJsonLocalState localState;
    auto secureStore = std::make_shared<StubSecureStore>();
    ConnectAuthenticationPersistence::CustomInitialize(&localState, secureStore);
    auto persistence = ConnectAuthenticationPersistence::GetShared();

    persistence->SetCredentials({"User", "Pass"});

    int called = 0;
    persistence->RegisterUserChangedListener([&]
        {
        called++;
        });

    persistence->SetCredentials({"User", "OtherPass"});

    EXPECT_STREQ("User", secureStore->LoadValue("Connect", "Username").c_str());
    EXPECT_STREQ("OtherPass", secureStore->LoadValue("Connect", "Password").c_str());
    EXPECT_EQ(0, called);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ConnectAuthenticationPersistenceTests, SetCredentials_ListenerUnregisteredAndCredentialsStored_DoesNotCallUnregisteredListener)
    {
    RuntimeJsonLocalState localState;
    auto secureStore = std::make_shared<StubSecureStore>();
    ConnectAuthenticationPersistence::CustomInitialize(&localState, secureStore);
    auto persistence = ConnectAuthenticationPersistence::GetShared();

    persistence->SetCredentials({"User", "Pass"});

    int calledA = 0;
    auto keyA = persistence->RegisterUserChangedListener([&]
        {
        calledA++;
        });

    int calledB = 0;
    persistence->RegisterUserChangedListener([&]
        {
        calledB++;
        });

    persistence->UnregisterUserChangedListener(keyA);
    persistence->SetCredentials({"OtherUser", "OtherPass"});

    EXPECT_EQ(0, calledA);
    EXPECT_EQ(1, calledB);
    }
