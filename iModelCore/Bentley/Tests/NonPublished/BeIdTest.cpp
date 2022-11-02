/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeId.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeTest.h>

//-------------------------------------------------------------------------------------------
// @bsimethod
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

/*---------------------------------------------------------------------------------**//**
* From iModel.js Id.test.ts (bentleyjs-core)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BeIdTests, IsWellFormed)
    {
    Utf8String goodIds[] =
        {
        "0",
        "0x12345678ab",
        "0xcdef987654",
        "0x1",
        "0xa",
        "0xa000000000",
        "0x10000000001",
        "0x11000000000",
        "0xfffffe0000000001",
        };

    for (auto const& goodId : goodIds)
        EXPECT_TRUE(BeInt64Id::IsWellFormedString(goodId));

    Utf8String badIds[] =
        {
        "0x0",
        "0x01",
        "0X1",
        "0xg",
        "0x12345678AB",
        "0xCDEF987654",
        "0xffffff0000000000",
        "0xffffffD000000000",
        "0xh0000000001",
        "0xh000000000000001",
        "0x1h00000000000000",
        "0x100000000000000h",
        };

    for (auto const& badId : badIds)
        EXPECT_FALSE(BeInt64Id::IsWellFormedString(badId));
    }
