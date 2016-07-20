/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/SecureStoreTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "SecureStoreTests.h"
#include <BeSecurity/BeSecurity.h>
#include "StubLocalState.h"

USING_NAMESPACE_BENTLEY_SECURITY

void SecureStoreTests::SetUpTestCase()
    {
#if defined(__APPLE__)
    // TODO: sign test project to use predefined keychain access group
    SecureStore::Initialize((void*)"Test");
#endif

#if defined(ANDROID)
    SecureStore::Initialize(BeTest::GetHost().InvokeP("getEnv"));
#endif
    }

#if defined(ANDROID) || defined(BENTLEY_WIN32) || defined(BENTLEY_WINRT)
//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F (SecureStoreTests, SaveValue_ValuePassed_GeneratesKeyBasedOnNamespaceAndKeyAndStoresToLocalState)
    {
    StubLocalState localState;
    SecureStore store (localState);

    store.SaveValue ("TestNameSpace", "TestKey", "Foo");
    EXPECT_EQ (1, localState.GetStubMap ().size ());
    EXPECT_TRUE (localState.GetStubMap ().find("fe_shape/TestNameSpace:TestKey")!=localState.GetStubMap().end());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F (SecureStoreTests, SaveValue_ValuePassed_SavesEncryptedValueAndStoresToLocalState)
    {
    StubLocalState localState;
    SecureStore store (localState);

    store.SaveValue ("Foo", "Boo", "TestPassword");
    EXPECT_EQ (1, localState.GetStubMap ().size ());

    auto storedValue = localState.GetValue ("fe_shape", "Foo:Boo");
    EXPECT_FALSE (storedValue.empty ());
    EXPECT_STRNE ("TestPassword", storedValue.c_str());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F (SecureStoreTests, SaveValue_EmptyPassword_SavesNullValueToDeleteIt)
    {
    StubLocalState localState;
    SecureStore store (localState);

    store.SaveValue ("Foo", "Boo", "TestValue");
    EXPECT_EQ (1, localState.GetStubMap ().size ());
    store.SaveValue ("Foo", "Boo", "");
    EXPECT_EQ (0, localState.GetStubMap ().size ());
    }
#endif

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F (SecureStoreTests, LoadValue_ValueSavedWithSameKey_ReturnsSameValue)
    {
    StubLocalState localState;
    SecureStore store (localState);

    store.SaveValue ("A", "B", "TestValue");
    EXPECT_STREQ ("TestValue", store.LoadValue ("A", "B").c_str ());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F (SecureStoreTests, LoadValue_ValuesSavedWithDifferentKeys_ReturnsDifferentValue)
    {
    StubLocalState localState;
    SecureStore store (localState);

    store.SaveValue ("A", "B", "TestValue1");
    store.SaveValue ("A", "C", "TestValue2");
    EXPECT_STREQ ("TestValue1", store.LoadValue ("A", "B").c_str ());
    EXPECT_STREQ ("TestValue2", store.LoadValue ("A", "C").c_str ());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F (SecureStoreTests, LoadValue_10KBValueSaved_ReturnsSameValue)
    {
    StubLocalState localState;
    SecureStore store (localState);

    // Just generate string with characters
    int count = 10 * 1024;
    Utf8String value (count, ' ');
    while (--count >= 0)
        value[count] = ' ' + count % ('z' - ' ' + 1);

    store.SaveValue ("A", "B", value.c_str ());
    EXPECT_STREQ (value.c_str (), store.LoadValue ("A", "B").c_str ());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F (SecureStoreTests, SaveValue_EmptyValue_RemovesOldValue)
    {
    StubLocalState localState;
    SecureStore store (localState);

    store.SaveValue ("A", "B", "TestValue");
    EXPECT_STREQ ("TestValue", store.LoadValue ("A", "B").c_str ());

    store.SaveValue ("A", "B", "");
    EXPECT_STREQ ("", store.LoadValue ("A", "B").c_str ());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F (SecureStoreTests, SaveValue_EmptyNamespace_SavesNothing)
    {
    StubLocalState localState;
    SecureStore store (localState);

    store.SaveValue ("", "Foo", "Value");
    EXPECT_TRUE (localState.GetStubMap ().empty ());
    EXPECT_STREQ ("", store.LoadValue ("", "Foo").c_str ());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F (SecureStoreTests, SaveValue_EmptyKey_SavesNothing)
    {
    StubLocalState localState;
    SecureStore store (localState);

    store.SaveValue ("Foo", "", "Value");
    EXPECT_TRUE (localState.GetStubMap ().empty ());
    EXPECT_STREQ ("", store.LoadValue ("Foo", "").c_str ());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F (SecureStoreTests, LoadValue_EmptyNameSpace_DoesNothingAndReturnsEmpty)
    {
    StubLocalState localState;
    SecureStore store (localState);

    EXPECT_EQ ("", store.LoadValue ("", "Foo"));
    EXPECT_TRUE (localState.GetStubMap ().empty ());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F (SecureStoreTests, LoadValue_EmptyKey_DoesNothingAndReturnsEmpty)
    {
    StubLocalState localState;
    SecureStore store (localState);

    EXPECT_EQ ("", store.LoadValue ("Foo", ""));
    EXPECT_TRUE (localState.GetStubMap ().empty ());
    }

#if defined(ANDROID) || defined(BENTLEY_WIN32) || defined(BENTLEY_WINRT)
//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F (SecureStoreTests, LegacyLoadValue_NoValue_Empty)
    {
    StubLocalState localState;
    SecureStore store (localState);

    EXPECT_EQ ("", store.LegacyLoadValue ("Test", "Key"));
    EXPECT_TRUE (localState.GetStubMap ().empty ());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F (SecureStoreTests, LegacyLoadValue_NonLegacyValueExists_ReturnsSameValue)
    {
    StubLocalState localState;
    SecureStore store (localState);
    store.SaveValue ("Test", "Key", "TestValue");

    EXPECT_STREQ ("TestValue", store.LegacyLoadValue ("Test", "Key").c_str());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F (SecureStoreTests, LegacyClearValue_NonLegacyValueExists_RemovesNonLegacyValue)
    {
    StubLocalState localState;
    SecureStore store (localState);
    store.SaveValue ("Test", "Key", "TestValue");

    store.LegacyClearValue ("Test", "Key");
    EXPECT_TRUE (localState.GetStubMap ().empty ());

    EXPECT_EQ ("", store.LegacyLoadValue ("Test", "Key"));
    EXPECT_EQ ("", store.LoadValue ("Test", "Key"));
    }

#elif defined(__APPLE__)
//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F (SecureStoreTests, LegacyLoadValue_NoValue_Empty)
    {
    SecureStore store;

    EXPECT_EQ ("", store.LegacyLoadValue ("Test", "Key"));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F (SecureStoreTests, LegacyLoadValue_NonLegacyValueExists_DoesNotReturnSameValue)
    {
    SecureStore store;
    store.SaveValue ("Test", "Key", "TestValue");

    EXPECT_EQ ("", store.LegacyLoadValue ("Test", "Key"));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F (SecureStoreTests, LegacyLoadValue_LegacyValueExists_ReturnsLegacyValue)
    {
    SecureStore store;
    store.SaveValue ("WSB", "Test:Key", "LegacyValue");
    store.SaveValue ("Test", "Key", "NonLegacyValue");

    EXPECT_EQ ("LegacyValue", store.LegacyLoadValue ("Test", "Key"));
    EXPECT_EQ ("NonLegacyValue", store.LoadValue ("Test", "Key"));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F (SecureStoreTests, LegacyClearValue_NonLegacyValueExists_DoesNotRemoveNonLegacyValue)
    {
    SecureStore store;
    store.SaveValue ("Test", "Key", "TestValue");

    store.LegacyClearValue ("Test", "Key");
    EXPECT_EQ ("", store.LoadValue ("Test", "Key"));
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F (SecureStoreTests, LegacyClearValue_LegacyValueExists_RemovesNonLegacyValue)
    {
    SecureStore store;
    store.SaveValue ("WSB", "Test:Key", "LegacyValue");
    store.SaveValue ("Test", "Key", "NonLegacyValue");

    store.LegacyClearValue ("Test", "Key");
    EXPECT_EQ ("", store.LegacyLoadValue ("Test", "Key"));
    EXPECT_EQ ("NonLegacyValue", store.LoadValue ("Test", "Key"));
    }
#endif

TEST_F(SecureStoreTests, Encrypt_EmptyPassed_ReturnsNonEmpty)
    {
    StubLocalState localState;
    SecureStore store(localState);
    EXPECT_STRNE("", store.Encrypt("").c_str());
    }

TEST_F(SecureStoreTests, Encrypt_NullPassed_ReturnsNotEmpty)
    {
    StubLocalState localState;
    SecureStore store(localState);
    EXPECT_STRNE("", store.Encrypt(nullptr).c_str());
    }

TEST_F(SecureStoreTests, Encrypt_ValuePassed_ReturnsNonEmptyDifferentValue)
    {
    StubLocalState localState;
    SecureStore store(localState);
    EXPECT_STRNE("Foo Boo", store.Encrypt("Foo Boo").c_str());
    }

TEST_F(SecureStoreTests, Encrypt_SameValuePassedTwice_ReturnsNonEmptyDifferentValues)
    {
    StubLocalState localState;
    SecureStore store(localState);

    Utf8String encrypted1 = store.Encrypt("Foo");
    Utf8String encrypted2 = store.Encrypt("Foo");

    EXPECT_STRNE("", encrypted1.c_str());
    EXPECT_STRNE(encrypted1.c_str(), encrypted2.c_str());
    }

TEST_F(SecureStoreTests, Decrypt_InvalidValuesPassed_ReturnsEmpty)
    {
    StubLocalState localState;
    SecureStore store(localState);

    BeTest::SetFailOnAssert(false);
    EXPECT_STREQ("", store.Decrypt(nullptr).c_str());
    EXPECT_STREQ("", store.Decrypt("").c_str());
    EXPECT_STREQ("", store.Decrypt("A").c_str());
    EXPECT_STREQ("", store.Decrypt("Foo Boo").c_str());
    BeTest::SetFailOnAssert(true);
    }

TEST_F(SecureStoreTests, Decrypt_EncryptedValuePassed_ReturnsOriginal)
    {
    StubLocalState localState;
    SecureStore store(localState);
    EXPECT_STREQ("", store.Decrypt(store.Encrypt(nullptr).c_str()).c_str());
    EXPECT_STREQ("", store.Decrypt(store.Encrypt("").c_str()).c_str());
    EXPECT_STREQ("Foo Boo", store.Decrypt(store.Encrypt("Foo Boo").c_str()).c_str());
    }

TEST_F(SecureStoreTests, Decrypt_EncryptedLargeValuePassed_ReturnsOriginal)
    {
    StubLocalState localState;
    SecureStore store(localState);

    Utf8String value;
    for (int i = 0; i < 1000; i++)
        {
        value += "0123456789";
        }
    EXPECT_STREQ(value.c_str(), store.Decrypt(store.Encrypt(value.c_str()).c_str()).c_str());
    }
