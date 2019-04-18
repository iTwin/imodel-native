/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeId.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeTest.h>

//-------------------------------------------------------------------------------------------
// @bsimethod                                          Farhad.Kabir                    11/16
//-------------------------------------------------------------------------------------------
TEST(BeIdTests, ToFromString)
    {
    BentleyStatus stat;

    // Decimal Format
    BeInt64Id received = BeInt64Id::FromString("122321123", &stat);
    ASSERT_EQ(SUCCESS, stat);
    EXPECT_TRUE(BeInt64Id(122321123) == received);

    received = BeInt64Id::FromString("0", &stat);
    ASSERT_EQ(SUCCESS, stat);
    EXPECT_TRUE(!received.IsValid());

    received = BeInt64Id::FromString("0x0", &stat);
    ASSERT_EQ(SUCCESS, stat);
    EXPECT_TRUE(!received.IsValid());

    received = BeInt64Id::FromString("0x0d", &stat);
    ASSERT_EQ(SUCCESS, stat);
    EXPECT_TRUE(received.IsValid());

    // Empty string
    received = BeInt64Id::FromString("", &stat);
    ASSERT_EQ(ERROR, stat);
    EXPECT_TRUE(!received.IsValid());

    // Negative string
    received = BeInt64Id::FromString("-4521", &stat);
    ASSERT_EQ(ERROR, stat);
    EXPECT_TRUE(!received.IsValid());

    // Double format
    received = BeInt64Id::FromString("45.21", &stat);
    ASSERT_EQ(ERROR, stat);
    EXPECT_TRUE(!received.IsValid());

    // Garbage
    received = BeInt64Id::FromString("asdsa89as", &stat);
    ASSERT_EQ(ERROR, stat);
    EXPECT_TRUE(!received.IsValid());
    
    Utf8String idStr1 = received.ToString(BeInt64Id::UseHex::Yes);
    EXPECT_TRUE(idStr1 == "0");

    BeInt64Id hexId(0xabcd12345678);
    Utf8String idStr = hexId.ToString(BeInt64Id::UseHex::Yes);
    EXPECT_TRUE(idStr == "0xabcd12345678");

    // Hexadecimal format
    received = BeInt64Id::FromString(idStr.c_str(), &stat);
    ASSERT_EQ(SUCCESS, stat);
    EXPECT_TRUE(received.IsValid());
    EXPECT_TRUE(received == hexId);
    }
