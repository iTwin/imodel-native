/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <Bentley/LocalState.h>
#include <BeSecurity/SecureLocalState.h>

using namespace ::testing;
USING_NAMESPACE_BENTLEY_SECURITY

class SecureLocalStateTests : public ::testing::Test
    {};

struct MockCipher : public ICipher
    {
    MOCK_METHOD1(Encrypt, Utf8String(Utf8CP value));
    MOCK_METHOD1(Decrypt, Utf8String(Utf8CP value));
    };

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(SecureLocalStateTests, SaveValue_NonEmptyValue_ValueSaved)
    {
    RuntimeLocalState localState;
    SecureLocalState store(&localState);

    store.SaveValue("Foo", "Boo", "TestValue");
    EXPECT_EQ(1, localState.GetValues().size());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(SecureLocalStateTests, SaveValue_EmptyValue_DoesNotSave)
    {
    RuntimeLocalState localState;
    SecureLocalState store(&localState);

    store.SaveValue("Foo", "Boo", "TestValue");
    EXPECT_EQ(1, localState.GetValues().size());
    store.SaveValue("Foo", "Boo", "");
    EXPECT_EQ(0, localState.GetValues().size());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(SecureLocalStateTests, SaveValue_NonEmptyValueTwoTimes_OnlyLastValueSaved)
    {
    RuntimeLocalState localState;
    SecureLocalState store(&localState);

    store.SaveValue("Foo", "Boo", "TestValue");
    EXPECT_EQ(1, localState.GetValues().size());
    store.SaveValue("Foo", "Boo", "TestValue2");
    EXPECT_EQ(1, localState.GetValues().size());
    EXPECT_STREQ("TestValue2", store.GetValue("Foo", "Boo").c_str());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(SecureLocalStateTests, SaveValue_NonEmptyValue_ValueSavedAndEncrypted)
    {
    RuntimeLocalState localState;
    auto cipher = std::make_shared<MockCipher>();
    SecureLocalState store(&localState, cipher);

    EXPECT_CALL(*cipher, Encrypt(_)).WillOnce(Return("Encripted"));
    store.SaveValue("Foo", "Boo", "TestValue");
    EXPECT_EQ(1, localState.GetValues().size());
    EXPECT_STREQ("Encripted", localState.GetValue("Foo", "Boo").c_str());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(SecureLocalStateTests, GetValue_ValueSavedWithSameKey_ReturnsSameValue)
    {
    RuntimeLocalState localState;
    SecureLocalState store(&localState);

    store.SaveValue("A", "B", "TestValue");
    EXPECT_STREQ("TestValue", store.GetValue("A", "B").c_str());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(SecureLocalStateTests, GetValue_ValuesSavedWithDifferentKeys_ReturnsDifferentValue)
    {
    RuntimeLocalState localState;
    SecureLocalState store(&localState);

    store.SaveValue("A", "B", "TestValue1");
    store.SaveValue("A", "C", "TestValue2");
    EXPECT_STREQ("TestValue1", store.GetValue("A", "B").c_str());
    EXPECT_STREQ("TestValue2", store.GetValue("A", "C").c_str());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(SecureLocalStateTests, GetValue_NonExistingRecord_ReturnsEmpty)
    {
    RuntimeLocalState localState;
    SecureLocalState store(&localState);

    EXPECT_STREQ("", store.GetValue("A", "B").c_str());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(SecureLocalStateTests, GetValue_ExisitingValue_DecryptsValue)
    {
    RuntimeLocalState localState;
    auto cipher = std::make_shared<MockCipher>();
    SecureLocalState store(&localState, cipher);

    localState.SaveValue("A", "B", "value");
    EXPECT_CALL(*cipher, Decrypt(_)).WillOnce(Return("Decripted"));
    EXPECT_STREQ("Decripted", store.GetValue("A", "B").c_str());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Vincas.Razma
//---------------------------------------------------------------------------------------
TEST_F(SecureLocalStateTests, GetValue_10KBValueSaved_ReturnsSameValue)
    {
    RuntimeLocalState localState;
    SecureLocalState store(&localState);

    // Just generate string with characters
    int count = 10 * 1024;
    Utf8String value(count, ' ');
    while (--count >= 0)
        value[count] = ' ' + count % ('z' - ' ' + 1);

    store.SaveValue("A", "B", value.c_str());
    EXPECT_STREQ(value.c_str(), store.GetValue("A", "B").c_str());
    }

