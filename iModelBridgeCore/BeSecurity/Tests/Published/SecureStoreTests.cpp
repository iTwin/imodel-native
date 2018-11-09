/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/SecureStoreTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <Bentley/LocalState.h>
#include <BeSecurity/SecureStore.h>

USING_NAMESPACE_BENTLEY_SECURITY

class SecureStoreTests : public ::testing::Test
    {
    public: static void SetUpTestCase();
    };

void SecureStoreTests::SetUpTestCase()
    {
#if defined(ANDROID)
    SecureStore::Initialize(BeTest::GetHost().InvokeP("getEnv"));
#endif
    }

#if defined(ANDROID) || defined(BENTLEY_WIN32) || defined(BENTLEY_WINRT)
//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(SecureStoreTests, SaveValue_ValuePassed_GeneratesKeyBasedOnNamespaceAndKeyAndStoresToLocalState)
    {
    RuntimeLocalState localState;
    SecureStore store(localState);

    store.SaveValue("TestNameSpace", "TestKey", "Foo");
    EXPECT_EQ(1, localState.GetValues().size());
    EXPECT_TRUE(localState.GetValues().find({"fe_shape", "TestNameSpace:TestKey"}) != localState.GetValues().end());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(SecureStoreTests, SaveValue_ValuePassed_SavesEncryptedValueAndStoresToLocalState)
    {
    RuntimeLocalState localState;
    SecureStore store(localState);

    store.SaveValue("Foo", "Boo", "TestPassword");
    EXPECT_EQ(1, localState.GetValues().size());

    auto storedValue = localState.GetValue("fe_shape", "Foo:Boo");
    EXPECT_FALSE(storedValue.empty());
    EXPECT_STRNE("TestPassword", storedValue.c_str());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(SecureStoreTests, SaveValue_EmptyPassword_SavesNullValueToDeleteIt)
    {
    RuntimeLocalState localState;
    SecureStore store(localState);

    store.SaveValue("Foo", "Boo", "TestValue");
    EXPECT_EQ(1, localState.GetValues().size());
    store.SaveValue("Foo", "Boo", "");
    EXPECT_EQ(0, localState.GetValues().size());
    }
#endif

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(SecureStoreTests, LoadValue_ValueSavedWithSameKey_ReturnsSameValue)
    {
    RuntimeLocalState localState;
    SecureStore store(localState);

    store.SaveValue("A", "B", "TestValue");
    EXPECT_STREQ("TestValue", store.LoadValue("A", "B").c_str());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(SecureStoreTests, LoadValue_ValuesSavedWithDifferentKeys_ReturnsDifferentValue)
    {
    RuntimeLocalState localState;
    SecureStore store(localState);

    store.SaveValue("A", "B", "TestValue1");
    store.SaveValue("A", "C", "TestValue2");
    EXPECT_STREQ("TestValue1", store.LoadValue("A", "B").c_str());
    EXPECT_STREQ("TestValue2", store.LoadValue("A", "C").c_str());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(SecureStoreTests, LoadValue_10KBValueSaved_ReturnsSameValue)
    {
    RuntimeLocalState localState;
    SecureStore store(localState);

    // Just generate string with characters
    int count = 10 * 1024;
    Utf8String value(count, ' ');
    while (--count >= 0)
        value[count] = ' ' + count % ('z' - ' ' + 1);

    store.SaveValue("A", "B", value.c_str());
    EXPECT_STREQ(value.c_str(), store.LoadValue("A", "B").c_str());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(SecureStoreTests, SaveValue_EmptyValue_RemovesOldValue)
    {
    RuntimeLocalState localState;
    SecureStore store(localState);

    store.SaveValue("A", "B", "TestValue");
    EXPECT_STREQ("TestValue", store.LoadValue("A", "B").c_str());

    store.SaveValue("A", "B", "");
    EXPECT_STREQ("", store.LoadValue("A", "B").c_str());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(SecureStoreTests, SaveValue_EmptyNamespace_SavesNothing)
    {
    RuntimeLocalState localState;
    SecureStore store(localState);

    store.SaveValue("", "Foo", "Value");
    EXPECT_TRUE(localState.GetValues().empty());
    EXPECT_STREQ("", store.LoadValue("", "Foo").c_str());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(SecureStoreTests, SaveValue_EmptyKey_SavesNothing)
    {
    RuntimeLocalState localState;
    SecureStore store(localState);

    store.SaveValue("Foo", "", "Value");
    EXPECT_TRUE(localState.GetValues().empty());
    EXPECT_STREQ("", store.LoadValue("Foo", "").c_str());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(SecureStoreTests, LoadValue_EmptyNameSpace_DoesNothingAndReturnsEmpty)
    {
    RuntimeLocalState localState;
    SecureStore store(localState);

    EXPECT_EQ("", store.LoadValue("", "Foo"));
    EXPECT_TRUE(localState.GetValues().empty());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(SecureStoreTests, LoadValue_EmptyKey_DoesNothingAndReturnsEmpty)
    {
    RuntimeLocalState localState;
    SecureStore store(localState);

    EXPECT_EQ("", store.LoadValue("Foo", ""));
    EXPECT_TRUE(localState.GetValues().empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                   Vincas.Razma                   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SecureStoreTests, Encrypt_EmptyPassed_ReturnsNonEmpty)
    {
    RuntimeLocalState localState;
    SecureStore store(localState);
    EXPECT_STRNE("", store.Encrypt("").c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                   Vincas.Razma                   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SecureStoreTests, Encrypt_NullPassed_ReturnsNotEmpty)
    {
    RuntimeLocalState localState;
    SecureStore store(localState);
    EXPECT_STRNE("", store.Encrypt(nullptr).c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                   Vincas.Razma                   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SecureStoreTests, Encrypt_ValuePassed_ReturnsNonEmptyDifferentValue)
    {
    RuntimeLocalState localState;
    SecureStore store(localState);
    EXPECT_STRNE("Foo Boo", store.Encrypt("Foo Boo").c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                   Vincas.Razma                   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SecureStoreTests, Encrypt_SameValuePassedTwice_ReturnsNonEmptyDifferentValues)
    {
    RuntimeLocalState localState;
    SecureStore store(localState);

    Utf8String encrypted1 = store.Encrypt("Foo");
    Utf8String encrypted2 = store.Encrypt("Foo");

    EXPECT_STRNE("", encrypted1.c_str());
    EXPECT_STRNE(encrypted1.c_str(), encrypted2.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                   Vincas.Razma                   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SecureStoreTests, Decrypt_InvalidValuesPassed_ReturnsEmpty)
    {
    RuntimeLocalState localState;
    SecureStore store(localState);

    BeTest::SetFailOnAssert(false);
    EXPECT_STREQ("", store.Decrypt(nullptr).c_str());
    EXPECT_STREQ("", store.Decrypt("").c_str());
    EXPECT_STREQ("", store.Decrypt("A").c_str());
    EXPECT_STREQ("", store.Decrypt("Foo Boo").c_str());
    BeTest::SetFailOnAssert(true);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                   Vincas.Razma                   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SecureStoreTests, Decrypt_EncryptedValuePassed_ReturnsOriginal)
    {
    RuntimeLocalState localState;
    SecureStore store(localState);
    EXPECT_STREQ("", store.Decrypt(store.Encrypt(nullptr).c_str()).c_str());
    EXPECT_STREQ("", store.Decrypt(store.Encrypt("").c_str()).c_str());
    EXPECT_STREQ("Foo Boo", store.Decrypt(store.Encrypt("Foo Boo").c_str()).c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                   Vincas.Razma                   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SecureStoreTests, Decrypt_EncryptedLargeValuePassed_ReturnsOriginal)
    {
    RuntimeLocalState localState;
    SecureStore store(localState);

    Utf8String value;
    for (int i = 0; i < 1000; i++)
        {
        value += "0123456789";
        }
    EXPECT_STREQ(value.c_str(), store.Decrypt(store.Encrypt(value.c_str()).c_str()).c_str());
    }
