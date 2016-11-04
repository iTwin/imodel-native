/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Tasks/CancellationTokenTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CancellationTokenTests.h"

#ifdef USE_GTEST
#include <gmock/gmock.h>
#endif

#include "MockCancellationListener.h"

#ifdef USE_GTEST
TEST_F (CancellationTokenTests, SimpleCancellationToken_Register_ListenerPassed_TakesByWeakPointer)
    {
    auto listener = std::make_shared<MockCancellationListener> ();
    auto token = SimpleCancellationToken::Create ();

    token->Register (listener);
    EXPECT_TRUE (listener.unique ());
    }

TEST_F (CancellationTokenTests, SimpleCancellationToken_Register_NotCanceledToken_DoesNotCallOnCanceled)
    {
    auto listener = std::make_shared<MockCancellationListener> ();
    auto token = SimpleCancellationToken::Create ();

    EXPECT_CALL (*listener, OnCanceled ()).Times (0);

    token->Register (listener);
    }

TEST_F (CancellationTokenTests, SimpleCancellationToken_Register_CanceledToken_CallsOnCanceledWhenRegistering)
    {
    auto listener = std::make_shared<MockCancellationListener> ();
    auto token = SimpleCancellationToken::Create ();
    token->SetCanceled ();

    EXPECT_CALL (*listener, OnCanceled ()).Times (1);

    token->Register (listener);
    }

TEST_F (CancellationTokenTests, SimpleCancellationToken_Register_CanceledTokenAndNullListener_DoesNothing)
    {
    auto listener = std::make_shared<MockCancellationListener> ();
    auto token = SimpleCancellationToken::Create ();
    token->SetCanceled ();

    token->Register (std::shared_ptr<ICancellationListener> ());
    }

TEST_F (CancellationTokenTests, SimpleCancellationToken_Register_TokenCanceledAfterRegistration_CallsOnCanceled)
    {
    auto listener = std::make_shared<MockCancellationListener> ();
    auto token = SimpleCancellationToken::Create ();

    token->Register (listener);

    EXPECT_CALL (*listener, OnCanceled ()).Times (1);
    token->SetCanceled ();
    }

TEST_F (CancellationTokenTests, SimpleCancellationToken_Register_TokenCanceledAfterRegistrationButListenerDestroyed_DoesNothing)
    {
    auto listener = std::make_shared<MockCancellationListener> ();
    auto token = SimpleCancellationToken::Create ();

    token->Register (listener);

    listener = nullptr;
    token->SetCanceled ();
    }

TEST_F (CancellationTokenTests, MergeCancellationToken_Register_ListenerPassed_TakesByWeakPointer)
    {
    auto listener = std::make_shared<MockCancellationListener> ();
    auto a = SimpleCancellationToken::Create ();
    auto b = SimpleCancellationToken::Create ();
    auto token = MergeCancellationToken::Create (a, b);

    token->Register (listener);
    EXPECT_TRUE (listener.unique ());
    }

TEST_F (CancellationTokenTests, MergeCancellationToken_Register_NotCanceledToken_DoesNotCallOnCanceled)
    {
    auto listener = std::make_shared<MockCancellationListener> ();
    auto a = SimpleCancellationToken::Create ();
    auto b = SimpleCancellationToken::Create ();
    auto token = MergeCancellationToken::Create (a, b);

    EXPECT_CALL (*listener, OnCanceled ()).Times (0);

    token->Register (listener);
    }

TEST_F (CancellationTokenTests, MergeCancellationToken_Register_CanceledToken_CallsOnCanceledWhenRegistering)
    {
    auto listener = std::make_shared<MockCancellationListener> ();
    auto a = SimpleCancellationToken::Create ();
    auto b = SimpleCancellationToken::Create ();
    auto token = MergeCancellationToken::Create (a, b);
    a->SetCanceled ();

    EXPECT_CALL (*listener, OnCanceled ()).Times (1);

    token->Register (listener);
    }

TEST_F (CancellationTokenTests, MergeCancellationToken_Register_CanceledTokenAndNullListener_DoesNothing)
    {
    auto listener = std::make_shared<MockCancellationListener> ();
    auto a = SimpleCancellationToken::Create ();
    auto b = SimpleCancellationToken::Create ();
    auto token = MergeCancellationToken::Create (a, b);
    a->SetCanceled ();

    token->Register (std::shared_ptr<ICancellationListener> ());
    }

TEST_F (CancellationTokenTests, MergeCancellationToken_Register_TokenCanceledAfterRegistration_CallsOnCanceled)
    {
    auto listener = std::make_shared<MockCancellationListener> ();
    auto a = SimpleCancellationToken::Create ();
    auto b = SimpleCancellationToken::Create ();
    auto token = MergeCancellationToken::Create (a, b);

    token->Register (listener);

    EXPECT_CALL (*listener, OnCanceled ()).Times (1);
    a->SetCanceled ();
    }

TEST_F (CancellationTokenTests, MergeCancellationToken_Register_TokenCanceledAfterRegistrationButListenerDestroyed_DoesNothing)
    {
    auto listener = std::make_shared<MockCancellationListener> ();
    auto a = SimpleCancellationToken::Create ();
    auto b = SimpleCancellationToken::Create ();
    auto token = MergeCancellationToken::Create (a, b);

    token->Register (listener);

    listener = nullptr;
    a->SetCanceled ();
    }
#endif