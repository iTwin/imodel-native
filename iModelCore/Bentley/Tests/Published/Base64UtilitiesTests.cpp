/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Base64UtilitiesTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "Base64UtilitiesTests.h"

USING_NAMESPACE_BENTLEY

TEST_F (Base64UtilitiesTests, Encode_EmptyString_ReturnsEmptyString)
    {
    EXPECT_STREQ ("", Base64Utilities::Encode ("").c_str ());
    }

TEST_F (Base64UtilitiesTests, Encode_String_ReturnsEncoded)
    {
    EXPECT_STREQ ("Rm9vMTIzIQ==", Base64Utilities::Encode ("Foo123!").c_str());
    }

TEST_F (Base64UtilitiesTests, Decode_EmptyString_ReturnsEmptyString)
    {
    EXPECT_STREQ ("", Base64Utilities::Decode ("").c_str ());
    }

TEST_F (Base64UtilitiesTests, Decode_Base64EncodedString_ReturnsDecoded)
    {
    EXPECT_STREQ ("Foo123!", Base64Utilities::Decode ("Rm9vMTIzIQ==").c_str ());
    }