/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Base64UtilitiesTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/Base64Utilities.h>

USING_NAMESPACE_BENTLEY

//---------------------------------------------------------------------------------------
// @bsiclass                                     Umar.Hayat                  03/16
//+---------------+---------------+---------------+---------------+---------------+------
struct Base64UtilitiesTests : public ::testing::Test
    {};

//---------------------------------------------------------------------------------------
// @bsiclass                                     Umar.Hayat                  03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (Base64UtilitiesTests, EncodeEmptyString)
    {
    Utf8String input("");
    ASSERT_TRUE(Base64Utilities::Encode(input).empty());
    ASSERT_TRUE(Base64Utilities::Encode(input.c_str()).empty());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(Base64UtilitiesTests, EncodeEmptyBlob)
    {
    Byte const* blob = nullptr;
    Utf8String encoded;
    ASSERT_EQ(SUCCESS, Base64Utilities::Encode(encoded, blob, 0));
    ASSERT_TRUE(encoded.empty());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Umar.Hayat                  03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (Base64UtilitiesTests, EncodeString)
    {
    Utf8String input("Foo123!");
    Utf8CP expected = "Rm9vMTIzIQ==";
    ASSERT_STREQ(expected, Base64Utilities::Encode(input).c_str());
    ASSERT_STREQ(expected, Base64Utilities::Encode(input.c_str()).c_str());
    
    Utf8String encoded;
    ASSERT_EQ(SUCCESS, Base64Utilities::Encode(encoded, (Byte*)input.data(), input.size()));
    ASSERT_STREQ(expected, encoded.c_str());

    // encode empty
    Utf8String inputEmptry("");
    ASSERT_EQ(0, Base64Utilities::Encode(inputEmptry).size());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Umar.Hayat                  03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (Base64UtilitiesTests, DecodeEmptyString)
    {
    Utf8String encoded("");
    ASSERT_TRUE(Base64Utilities::Decode(encoded).empty());
    ASSERT_TRUE(Base64Utilities::Decode(nullptr, 0).empty());

    bvector<Byte> blob;
    ASSERT_EQ(SUCCESS, Base64Utilities::Decode(blob, encoded));
    ASSERT_TRUE(blob.empty());
    ASSERT_EQ(SUCCESS, Base64Utilities::Decode(blob, nullptr, 0));
    ASSERT_TRUE(blob.empty());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Umar.Hayat                  03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F (Base64UtilitiesTests, Decode)
    {
    Utf8String expectedStr("Foo123!");
    Utf8String encoded("Rm9vMTIzIQ==");
    ASSERT_STREQ(expectedStr.c_str(), Base64Utilities::Decode(encoded).c_str());
    ASSERT_STREQ(expectedStr.c_str(), Base64Utilities::Decode(encoded.c_str(), encoded.size()).c_str());

    bvector<Byte> byteArray;
    ASSERT_EQ(SUCCESS, Base64Utilities::Decode(byteArray, encoded));
    ASSERT_EQ(expectedStr.size(), byteArray.size());
    for (size_t i = 0; i < expectedStr.size(); i++)
        {
        ASSERT_EQ(expectedStr[i], (Utf8Char) byteArray[i]);
        }
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  03/16
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
// @bsiclass                                     Krischan.Eberle                  03/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(Base64UtilitiesTests, EncodeDecodeBlob)
    {
    const int64_t expectedNumber = INT64_C(1234567890);
    Byte const* expectedBlob = (Byte const*) (&expectedNumber);
    const size_t expectedBlobSize = sizeof(expectedNumber);

    Utf8String encoded;
    ASSERT_EQ(SUCCESS, Base64Utilities::Encode(encoded, expectedBlob, expectedBlobSize));

    ASSERT_STREQ("0gKWSQAAAAA=", encoded.c_str());

    bvector<Byte> decodedBlob;
    ASSERT_EQ(SUCCESS, Base64Utilities::Decode(decodedBlob, encoded));
    ASSERT_EQ(expectedBlobSize, decodedBlob.size());
    
    int64_t actualNumber = INT64_C(-1);
    memcpy(&actualNumber, decodedBlob.data(), sizeof(actualNumber));
    ASSERT_EQ(expectedNumber, actualNumber);
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Umar.Hayat                  06/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(Base64UtilitiesTests, EncodeDecodeByteStream)
    {
    Utf8String expectedString("Foo123!");

    Utf8String encoded = Base64Utilities::Encode(expectedString);
    ASSERT_STREQ("Rm9vMTIzIQ==", encoded.c_str());

    ByteStream decodedStream;
    EXPECT_TRUE(BentleyStatus::SUCCESS == Base64Utilities::Decode(decodedStream, encoded.c_str(), encoded.size()));
    ASSERT_STREQ(expectedString.c_str(), (CharCP)decodedStream.GetData());

    // Decode empty
    ByteStream decodedEmpty;
    EXPECT_TRUE(BentleyStatus::SUCCESS == Base64Utilities::Decode(decodedStream, nullptr, 0));
    EXPECT_TRUE(0 == decodedEmpty.GetSize());
    
    // Decode partial
    ByteStream decodedPartial;
    EXPECT_TRUE(BentleyStatus::SUCCESS == Base64Utilities::Decode(decodedPartial, "Rm9vMTIzIQ==Garbage", 12));
    ASSERT_STREQ(expectedString.c_str(), (CharCP)decodedPartial.GetData());

    }

