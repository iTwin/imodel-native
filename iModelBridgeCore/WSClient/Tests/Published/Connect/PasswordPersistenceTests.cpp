/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Connect/PasswordPersistenceTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "PasswordPersistenceTests.h"

#include <WebServices/Connect/PasswordPersistence.h>
#include "MockLocalState.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_MOBILEDGN

#ifdef USE_GTEST
TEST_F (PasswordPersistenceTests, SavePassword_EmptyIdentifier_DoesNothing)
    {
    MockLocalState localState;
    PasswordPersistence persistence (&localState);

    EXPECT_CALL (localState, SaveValue (_, _, _)).Times (0);

    persistence.SavePassword ("", "Foo");
    }

TEST_F (PasswordPersistenceTests, SavePassword_EmptyPassword_SavesNullValueToDeleteIt)
    {
    MockLocalState localState;
    PasswordPersistence persistence (&localState);

    EXPECT_CALL (localState, SaveValue (_, _, _)).WillOnce (Invoke ([=] (Utf8CP, Utf8CP, JsonValueCR value)
        {
        EXPECT_TRUE (value.isNull ());
        }));

    persistence.SavePassword ("Foo", "");
    }

TEST_F (PasswordPersistenceTests, SavePassword_IdentifierAndPasswordPassed_SavesEncodedPassword)
    {
    MockLocalState localState;
    PasswordPersistence persistence (&localState);

    EXPECT_CALL (localState, SaveValue (_, _, _))
        .Times (1)
        .WillOnce (Invoke ([] (Utf8CP nameSpace, Utf8CP key, JsonValueCR value)
        {
        EXPECT_STRNE ("", nameSpace);
        EXPECT_STREQ ("TestIdentifier", key);

        EXPECT_NE ("", value.asString ());
        EXPECT_NE ("TestPassword", value.asString ());
        }));

    persistence.SavePassword ("TestIdentifier", "TestPassword");
    }

TEST_F (PasswordPersistenceTests, LoadPassword_EmptyIdentifier_DoesNothingAndReturnsEmpty)
    {
    MockLocalState localState;
    PasswordPersistence persistence (&localState);

    EXPECT_CALL (localState, GetValue (_, _)).Times (0);

    auto password = persistence.LoadPassword ("");

    EXPECT_EQ ("", password);
    }

TEST_F (PasswordPersistenceTests, LoadPassword_IdentifierPassed_CallsReadValue)
    {
    MockLocalState localState;
    PasswordPersistence persistence (&localState);

    EXPECT_CALL (localState, GetValue (_, _))
        .Times (1)
        .WillOnce (Invoke ([] (Utf8CP nameSpace, Utf8CP key)
        {
        EXPECT_STRNE ("", nameSpace);
        EXPECT_STREQ ("TestIdentifier", key);

        return Json::nullValue;
        }));

    persistence.LoadPassword ("TestIdentifier");
    }
#endif