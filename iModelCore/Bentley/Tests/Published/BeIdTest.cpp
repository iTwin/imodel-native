/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/BeIdTest.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeId.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/BeTest.h>

//-------------------------------------------------------------------------------------------
// @bsimethod                                          Farhad.Kabir                    11/16
//-------------------------------------------------------------------------------------------
TEST(BeIdTests, FromString)
    {
    //Decimal Format
    Utf8CP idString = "122321123";
    BeInt64Id received;
    BeInt64Id expected(122321123);
    ASSERT_EQ(BentleyStatus::SUCCESS, BeInt64Id::FromString(received, idString));
    EXPECT_TRUE(expected == received);
    //Empty string
    idString = "";
    ASSERT_EQ(BentleyStatus::ERROR, BeInt64Id::FromString(received, idString));
    //Negative string
    idString = "-4521";
    ASSERT_EQ(BentleyStatus::ERROR, BeInt64Id::FromString(received, idString));
    //Double format
    idString = "45.21";
    ASSERT_EQ(BentleyStatus::ERROR, BeInt64Id::FromString(received, idString));
    //Hexadecimal format
    idString = "0x00FD";
    ASSERT_EQ(BentleyStatus::ERROR, BeInt64Id::FromString(received, idString));
    //Random
    idString = "asdsa89as";
    ASSERT_EQ(BentleyStatus::ERROR, BeInt64Id::FromString(received, idString));
    }