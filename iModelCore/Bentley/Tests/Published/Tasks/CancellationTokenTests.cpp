/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Tasks/CancellationTokenTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CancellationTokenTests.h"

#ifdef USE_GTEST
#include <gmock/gmock.h>
#endif

#include "MockCancellationListener.h"

#ifdef USE_GTEST

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas                03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CancellationTokenTests, SimpleCancellationToken_Register_ListenerPassed_TakesByWeakPointer)
    {
    auto listener = std::make_shared<MockCancellationListener> ();
    auto token = SimpleCancellationToken::Create ();

    token->Register (listener);
    EXPECT_TRUE (listener.unique ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas               03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CancellationTokenTests, SimpleCancellationToken_Register_NotCanceledToken_DoesNotCallOnCanceled)
    {
    auto listener = std::make_shared<MockCancellationListener> ();
    auto token = SimpleCancellationToken::Create ();

    EXPECT_CALL (*listener, OnCanceled ()).Times (0);

    token->Register (listener);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas                03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CancellationTokenTests, SimpleCancellationToken_Register_CanceledToken_CallsOnCanceledWhenRegistering)
    {
    auto listener = std::make_shared<MockCancellationListener> ();
    auto token = SimpleCancellationToken::Create ();
    token->SetCanceled ();

    EXPECT_CALL (*listener, OnCanceled ()).Times (1);

    token->Register (listener);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas                03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CancellationTokenTests, SimpleCancellationToken_Register_CanceledTokenAndNullListener_DoesNothing)
    {
    auto listener = std::make_shared<MockCancellationListener> ();
    auto token = SimpleCancellationToken::Create ();
    token->SetCanceled ();

    token->Register (std::shared_ptr<ICancellationListener> ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas                03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CancellationTokenTests, SimpleCancellationToken_Register_TokenCanceledAfterRegistration_CallsOnCanceled)
    {
    auto listener = std::make_shared<MockCancellationListener> ();
    auto token = SimpleCancellationToken::Create ();

    token->Register (listener);

    EXPECT_CALL (*listener, OnCanceled ()).Times (1);
    token->SetCanceled ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas                03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CancellationTokenTests, SimpleCancellationToken_Register_TokenCanceledAfterRegistrationButListenerDestroyed_DoesNothing)
    {
    auto listener = std::make_shared<MockCancellationListener> ();
    auto token = SimpleCancellationToken::Create ();

    token->Register (listener);

    listener = nullptr;
    token->SetCanceled ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas               03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CancellationTokenTests, MergeCancellationToken_Register_ListenerPassed_TakesByWeakPointer)
    {
    auto listener = std::make_shared<MockCancellationListener> ();
    auto a = SimpleCancellationToken::Create ();
    auto b = SimpleCancellationToken::Create ();
    auto token = MergeCancellationToken::Create (a, b);

    token->Register (listener);
    EXPECT_TRUE (listener.unique ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas               03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CancellationTokenTests, MergeCancellationToken_Register_NotCanceledToken_DoesNotCallOnCanceled)
    {
    auto listener = std::make_shared<MockCancellationListener> ();
    auto a = SimpleCancellationToken::Create ();
    auto b = SimpleCancellationToken::Create ();
    auto token = MergeCancellationToken::Create (a, b);

    EXPECT_CALL (*listener, OnCanceled ()).Times (0);

    token->Register (listener);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas                03/16
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas                 03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CancellationTokenTests, MergeCancellationToken_Register_CanceledTokenAndNullListener_DoesNothing)
    {
    auto listener = std::make_shared<MockCancellationListener> ();
    auto a = SimpleCancellationToken::Create ();
    auto b = SimpleCancellationToken::Create ();
    auto token = MergeCancellationToken::Create (a, b);
    a->SetCanceled ();

    token->Register (std::shared_ptr<ICancellationListener> ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas               03/16
+---------------+---------------+---------------+---------------+---------------+------*/
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Benediktas.Lipnickas               03/16
+---------------+---------------+---------------+---------------+---------------+------*/
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

//-----------------------------------------------------------------------------------------
// @bsimethod                                          Farhad.Kabir                  11/16
//-----------------------------------------------------------------------------------------
TEST_F(CancellationTokenTests, SimpleCancellationToken_Register_CanceledToken_IsCancelled)
    {
    auto listener = std::make_shared<MockCancellationListener>();
    auto token = SimpleCancellationToken::Create();
    EXPECT_FALSE(1 == token->IsCanceled());
    token->SetCanceled();
    EXPECT_TRUE(1 == token->IsCanceled());
    EXPECT_CALL(*listener, OnCanceled()).Times(1);

    token->Register(listener);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                          Farhad.Kabir                  11/16
//-----------------------------------------------------------------------------------------
TEST_F(CancellationTokenTests, MergeCancellationToken_Register_CanceledTokens_IsCanceled)
    {
    auto listener = std::make_shared<MockCancellationListener>();
    auto a = SimpleCancellationToken::Create();
    auto b = SimpleCancellationToken::Create();
    auto token = MergeCancellationToken::Create(a, b);
    EXPECT_FALSE(1 == token->IsCanceled());
    token->Register(listener);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                          Farhad.Kabir                  11/16
//-----------------------------------------------------------------------------------------
TEST_F(CancellationTokenTests, MergeCancellationToken_Register_CanceledTokens_IsCanceled_One)
    {
    auto listener = std::make_shared<MockCancellationListener>();
    auto a = SimpleCancellationToken::Create();
    auto b = SimpleCancellationToken::Create();
    a->SetCanceled();
    EXPECT_CALL(*listener, OnCanceled()).Times(1);
    auto token = MergeCancellationToken::Create(a, b);
    EXPECT_TRUE(1 == token->IsCanceled());
    token->Register(listener);
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                          Farhad.Kabir                  11/16
//-----------------------------------------------------------------------------------------
TEST_F(CancellationTokenTests, MergeCancellationToken_Register_CanceledTokens_IsCanceled_All)
    {
    auto listener = std::make_shared<MockCancellationListener>();
    auto a = SimpleCancellationToken::Create();
    auto b = SimpleCancellationToken::Create();
    a->SetCanceled();
    EXPECT_TRUE(1 == a->IsCanceled());
    b->SetCanceled();
    EXPECT_TRUE(1 == b->IsCanceled());
    EXPECT_CALL(*listener, OnCanceled()).Times(2);
    bvector<ICancellationTokenPtr> tokens;
    tokens.push_back(a);
    tokens.push_back(b);
    auto token = MergeCancellationToken::Create(tokens);
    EXPECT_TRUE(1 == token->IsCanceled());
    token->Register(listener);
    }
#endif