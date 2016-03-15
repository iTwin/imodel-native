/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Base64UtilitiesTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/Base64Utilities.h>

struct Base64UtilitiesTests : public ::testing::Test
    {
    };

USING_NAMESPACE_BENTLEY

TEST_F (Base64UtilitiesTests, Encode_EmptyString_ReturnsEmptyString)
    {
    Utf8String input("");
    EXPECT_STREQ("", Base64Utilities::Encode(input).c_str());
    EXPECT_STREQ("", Base64Utilities::Encode(input.c_str()).c_str());
    }

TEST_F (Base64UtilitiesTests, Encode_String_ReturnsEncoded)
    {
    Utf8String input("Foo123!");
    EXPECT_STREQ("Rm9vMTIzIQ==", Base64Utilities::Encode(input).c_str());
    EXPECT_STREQ("Rm9vMTIzIQ==", Base64Utilities::Encode(input.c_str()).c_str());
    Utf8String out;
    EXPECT_EQ(SUCCESS, Base64Utilities::Encode(out, (Byte*)input.data(), input.size()));
    EXPECT_STREQ("Rm9vMTIzIQ==", out.c_str());
    }

TEST_F (Base64UtilitiesTests, Decode_EmptyString_ReturnsEmptyString)
    {
    Utf8String input("");
    EXPECT_STREQ ("", Base64Utilities::Decode (input).c_str ());
    EXPECT_STREQ ("", Base64Utilities::Decode (input.c_str()).c_str ());
    }

TEST_F (Base64UtilitiesTests, Decode_Base64EncodedString_ReturnsDecoded)
    {
    Utf8String input("Rm9vMTIzIQ==");
    EXPECT_STREQ ("Foo123!", Base64Utilities::Decode (input).c_str ());
    EXPECT_STREQ ("Foo123!", Base64Utilities::Decode (input.c_str()).c_str ());
    }

TEST_F (Base64UtilitiesTests, Decode_Base64EncodedString_ReturnsDecodedBytes)
    {
    Utf8String input("Foo123!");
    Utf8String encoded;
    encoded = Base64Utilities::Encode(input);
    EXPECT_STREQ("Rm9vMTIzIQ==", encoded.c_str());

    bvector<Byte> out;
    EXPECT_EQ(SUCCESS, Base64Utilities::Decode(out, encoded));
    ASSERT_TRUE(0 != out.size());
    //EXPECT_STREQ("Foo123!", (Utf8CP) out.data());

    SUCCEED();
    }

