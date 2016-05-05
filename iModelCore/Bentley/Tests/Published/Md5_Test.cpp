/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/Md5_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <Bentley/md5.h>

//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                          05/16
//---------------------------------------------------------------------------------------
TEST(Md5Test,String)
    {
    MD5 md5;
    // Utf8String
    EXPECT_STREQ("b10a8db164e0754105b7a99be72e3fe5", md5("Hello World").c_str());
    // empty string
    EXPECT_STREQ("d41d8cd98f00b204e9800998ecf8427e", md5("").c_str());

    Utf8String longStr = "";
    for (int i = 0; i < 8; i++)
        longStr += "12345678";

    EXPECT_STREQ("6456d36652220045192fb2b53da70d63", md5(longStr).c_str()) << longStr.c_str();
    }
//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                          05/16
//---------------------------------------------------------------------------------------
TEST(Md5Test, Bytes)
    {
    MD5 md5;
    // arbitrary data, 11 bytes
    EXPECT_STREQ("b10a8db164e0754105b7a99be72e3fe5", md5("Hello World", 11).c_str());
    // arbitrary data, 11 bytes , sub string
    EXPECT_STREQ("b10a8db164e0754105b7a99be72e3fe5", md5("Hello World!", 11).c_str());
    // Emptry
    EXPECT_STREQ("d41d8cd98f00b204e9800998ecf8427e", md5("", 0).c_str());
    }
//---------------------------------------------------------------------------------------
// @betest                                      Umar.Hayat                          05/16
//---------------------------------------------------------------------------------------
TEST(Md5Test, Stream)
    {
    MD5 md5;
    Utf8String utf8Str1 = "Hello ";
    Utf8String utf8Str2 = "World";
    md5.Add(utf8Str1.data(), 6);
    md5.Add(utf8Str2.data(), 5);

    EXPECT_STREQ("b10a8db164e0754105b7a99be72e3fe5", md5.GetHashString().c_str());
    }