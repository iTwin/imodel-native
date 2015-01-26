/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Connect/SecureStoreTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "SecureStoreTests.h"

#include <WebServices/Connect/SecureStore.h>
#include "MockLocalState.h"
#include "StubLocalState.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

#ifdef USE_GTEST
TEST_F (SecureStoreTests, SaveValue_EmptyNameSpace_DoesNothing)
    {
    MockLocalState localState;
    SecureStore store (&localState);

    EXPECT_CALL (localState, SaveValue (_, _, _)).Times (0);
    store.SaveValue ("", "Foo", "Value");
    }

TEST_F (SecureStoreTests, SaveValue_EmptyKey_DoesNothing)
    {
    MockLocalState localState;
    SecureStore store (&localState);

    EXPECT_CALL (localState, SaveValue (_, _, _)).Times (0);
    store.SaveValue ("Foo", "", "Value");
    }

TEST_F (SecureStoreTests, SaveValue_EmptyPassword_SavesNullValueToDeleteIt)
    {
    MockLocalState localState;
    SecureStore store (&localState);

    EXPECT_CALL (localState, SaveValue (_, _, _)).WillOnce (Invoke ([=] (Utf8CP, Utf8CP, JsonValueCR value)
        {
        EXPECT_TRUE (value.isNull ());
        }));

    store.SaveValue ("Foo", "Boo", "");
    }

TEST_F (SecureStoreTests, SaveValue_ValuePassed_SavesEncodedValue)
    {
    MockLocalState localState;
    SecureStore store (&localState);

    EXPECT_CALL (localState, SaveValue (_, _, _)).Times (1).WillOnce (Invoke ([] (Utf8CP nameSpace, Utf8CP key, JsonValueCR value)
        {
        EXPECT_NE ("", value.asString ());
        EXPECT_NE ("TestPassword", value.asString ());
        }));

    store.SaveValue ("Foo", "Boo", "TestPassword");
    }

TEST_F (SecureStoreTests, LoadValue_EmptyNameSpace_DoesNothingAndReturnsEmpty)
    {
    MockLocalState localState;
    SecureStore store (&localState);

    EXPECT_CALL (localState, GetValue (_, _)).Times (0);
    EXPECT_EQ ("", store.LoadValue ("", "Foo"));
    }

TEST_F (SecureStoreTests, LoadValue_EmptyKey_DoesNothingAndReturnsEmpty)
    {
    MockLocalState localState;
    SecureStore store (&localState);

    EXPECT_CALL (localState, GetValue (_, _)).Times (0);
    EXPECT_EQ ("", store.LoadValue ("Foo", ""));
    }

TEST_F (SecureStoreTests, SaveValue_ValuePassed_GeneratesKeyBasedOnNamespaceAndKey)
    {
    MockLocalState localState;
    SecureStore store (&localState);

    EXPECT_CALL (localState, SaveValue (_, _, _)).Times (1).WillOnce (Invoke ([] (Utf8CP nameSpace, Utf8CP key, JsonValueCR value)
        {
        EXPECT_STRNE ("", nameSpace);
        EXPECT_STREQ ("TestNameSpace:TestKey", key);
        }));

    store.SaveValue ("TestNameSpace", "TestKey", "Foo");
    }

TEST_F (SecureStoreTests, LoadValue_IdentifierPassed_CallsReadValueWithGeneratedKey)
    {
    MockLocalState localState;
    SecureStore store (&localState);

    EXPECT_CALL (localState, GetValue (_, _)).Times (1).WillOnce (Invoke ([] (Utf8CP nameSpace, Utf8CP key)
        {
        EXPECT_STRNE ("", nameSpace);
        EXPECT_STREQ ("TestNameSpace:TestKey", key);
        return Json::nullValue;
        }));

    store.LoadValue ("TestNameSpace", "TestKey");
    }

TEST_F (SecureStoreTests, LoadValue_ValueSavedWithSameKey_ReturnsSameValue)
    {
    MockLocalState localState;
    SecureStore store (&localState);

    store.SaveValue ("A", "B", "TestValue");
    EXPECT_EQ ("TestValue", store.LoadValue ("A", "B"));
    }
#endif