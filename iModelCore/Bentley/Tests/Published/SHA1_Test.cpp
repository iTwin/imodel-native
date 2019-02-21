/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/SHA1_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/SHA1.h>

//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                          05/16
//---------------------------------------------------------------------------------------
TEST(SHA1Test,String)
    {
    SHA1 sha1;
    // Utf8String
    EXPECT_STREQ("0a4d55a8d778e5022fab701977c5d840bbc486d0", sha1("Hello World").c_str());
    // empty string
    EXPECT_STREQ("da39a3ee5e6b4b0d3255bfef95601890afd80709", sha1("").c_str());

    Utf8String longStr = "";
    for (int i = 0; i < 8; i++) 
        longStr += "12345678";

    EXPECT_STREQ("8c3697a6c16f22ee9a7871ff6cd06ca1e8868216", sha1(longStr).c_str())<<longStr.c_str();
    }
//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                          05/16
//---------------------------------------------------------------------------------------
TEST(SHA1Test, Bytes)
    {
    SHA1 sha1;
    // arbitrary data, 11 bytes
    EXPECT_STREQ("0a4d55a8d778e5022fab701977c5d840bbc486d0", sha1("Hello World", 11).c_str());
    // arbitrary data, 11 bytes , sub string
    EXPECT_STREQ("0a4d55a8d778e5022fab701977c5d840bbc486d0", sha1("Hello World!", 11).c_str());
    // Emptry
    EXPECT_STREQ("da39a3ee5e6b4b0d3255bfef95601890afd80709", sha1("", 0).c_str());
    }
//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                          05/16
//---------------------------------------------------------------------------------------
TEST(SHA1Test, Stream)
    {
    SHA1 sha1;
    Utf8String utf8Str1 = "Hello ";
    Utf8String utf8Str2 = "World";
    sha1.Add(utf8Str1.data(),6);
    sha1.Add(utf8Str2.data(),5);

    EXPECT_STREQ("0a4d55a8d778e5022fab701977c5d840bbc486d0", sha1.GetHashString().c_str());
    }
