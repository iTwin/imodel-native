/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/Base64Utilities.h>

USING_NAMESPACE_BENTLEY

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct Base64UtilitiesTests : public ::testing::Test
    {};

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (Base64UtilitiesTests, EncodeEmptyString)
    {
    Utf8String input("");
    ASSERT_TRUE(Base64Utilities::Encode(input).empty());
    ASSERT_TRUE(Base64Utilities::Encode(input.c_str()).empty());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(Base64UtilitiesTests, EncodeEmptyBlob)
    {
    Byte const* blob = nullptr;
    Utf8String encoded;
    Base64Utilities::Encode(encoded, blob, 0);
    ASSERT_TRUE(encoded.empty());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (Base64UtilitiesTests, EncodeString)
    {
    Utf8String input("Foo123!");
    Utf8CP expected = "Rm9vMTIzIQ==";
    ASSERT_STREQ(expected, Base64Utilities::Encode(input).c_str());
    ASSERT_STREQ(expected, Base64Utilities::Encode(input.c_str()).c_str());
    
    Utf8String encoded;
    Base64Utilities::Encode(encoded, (Byte*)input.data(), input.size());
    ASSERT_STREQ(expected, encoded.c_str());

    // encode empty
    Utf8String inputEmptry("");
    ASSERT_EQ(0, Base64Utilities::Encode(inputEmptry).size());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (Base64UtilitiesTests, DecodeEmptyString)
    {
    Utf8String encoded("");
    ASSERT_TRUE(Base64Utilities::Decode(encoded).empty());
    ASSERT_TRUE(Base64Utilities::Decode(nullptr, 0).empty());

    bvector<Byte> blob;
    Base64Utilities::Decode(blob, encoded);
    ASSERT_TRUE(blob.empty());
    Base64Utilities::Decode(blob, nullptr, 0);
    ASSERT_TRUE(blob.empty());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (Base64UtilitiesTests, Decode)
    {
    Utf8String expectedStr("Foo123!");
    Utf8String encoded("Rm9vMTIzIQ==");
    ASSERT_STREQ(expectedStr.c_str(), Base64Utilities::Decode(encoded).c_str());
    ASSERT_STREQ(expectedStr.c_str(), Base64Utilities::Decode(encoded.c_str(), encoded.size()).c_str());

    bvector<Byte> byteArray;
    Base64Utilities::Decode(byteArray, encoded);
    ASSERT_EQ(expectedStr.size(), byteArray.size());
    for (size_t i = 0; i < expectedStr.size(); i++)
        {
        ASSERT_EQ(expectedStr[i], (Utf8Char) byteArray[i]);
        }
    }


//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(Base64UtilitiesTests, EncodeDecodeString)
    {
    Utf8String expectedString("Foo123!");

    Utf8String encoded = Base64Utilities::Encode(expectedString);
    ASSERT_STREQ("Rm9vMTIzIQ==", encoded.c_str());

    Utf8String actualString = Base64Utilities::Decode(encoded);

    ASSERT_STREQ(expectedString.c_str(), actualString.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(Base64UtilitiesTests, EncodeDecodeBlob)
    {
    const int64_t expectedNumber = INT64_C(1234567890);
    Byte const* expectedBlob = (Byte const*) (&expectedNumber);
    const size_t expectedBlobSize = sizeof(expectedNumber);

    Utf8String encoded;
    Base64Utilities::Encode(encoded, expectedBlob, expectedBlobSize);

    ASSERT_STREQ("0gKWSQAAAAA=", encoded.c_str());

    bvector<Byte> decodedBlob;
    Base64Utilities::Decode(decodedBlob, encoded);
    ASSERT_EQ(expectedBlobSize, decodedBlob.size());
    
    int64_t actualNumber = INT64_C(-1);
    memcpy(&actualNumber, decodedBlob.data(), sizeof(actualNumber));
    ASSERT_EQ(expectedNumber, actualNumber);
    }

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(Base64UtilitiesTests, EncodeDecodeByteStream)
    {
    Utf8String expectedString("Foo123!");

    Utf8String encoded = Base64Utilities::Encode(expectedString);
    ASSERT_STREQ("Rm9vMTIzIQ==", encoded.c_str());

    ByteStream decodedStream;
    Base64Utilities::Decode(decodedStream, encoded.c_str(), encoded.size());
    Utf8String decodedString((CharCP)decodedStream.GetData(), expectedString.size());
    ASSERT_STREQ(expectedString.c_str(), decodedString.c_str());

    // Decode empty
    ByteStream decodedEmpty;
    Base64Utilities::Decode(decodedStream, nullptr, 0);
    EXPECT_TRUE(0 == decodedEmpty.GetSize());
    
    // Decode partial
    ByteStream decodedPartial;
    Base64Utilities::Decode(decodedPartial, "Rm9vMTIzIQ==Garbage", 12);
    Utf8String decodedPartialString((CharCP)decodedPartial.GetData(), expectedString.size());
    ASSERT_STREQ(expectedString.c_str(), decodedPartialString.c_str());

    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(Base64UtilitiesTests, AlphabetsMatching)
    {
    Utf8CP charStr = "";
    ASSERT_TRUE(1 == Base64Utilities::MatchesAlphabet(charStr));
    charStr = "FardhaBakKir";
    ASSERT_TRUE(1 == Base64Utilities::MatchesAlphabet(charStr));
    charStr = "945/56*a";
    ASSERT_TRUE(0 == Base64Utilities::MatchesAlphabet(charStr));
    charStr = "x+y=2a/z";
    ASSERT_TRUE(1 == Base64Utilities::MatchesAlphabet(charStr));
    Utf8String baseString = Base64Utilities::Alphabet();
    ASSERT_TRUE(1 == Base64Utilities::MatchesAlphabet(baseString.c_str()));
    }
