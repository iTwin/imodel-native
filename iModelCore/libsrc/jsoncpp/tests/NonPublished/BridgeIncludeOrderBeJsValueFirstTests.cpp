/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

// Companion to BridgeIncludeOrderJsonFirstTests.cpp: here BeJsValue.h is included first and is
// responsible for pulling in json/json.h itself when USE_JSONCPP is defined.
#include <BeRapidJson/BeJsValue.h>
#include <json/json.h>

#include <Bentley/BeTest.h>

USING_NAMESPACE_BENTLEY

#ifndef USE_JSONCPP
    #error "The BeJsonCpp unit tests must build with USE_JSONCPP defined; see tests/BuildTests.mke."
#endif

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(BridgeIncludeOrderBeJsValueFirst, RoundTripsThroughBeJsValue)
    {
    Json::Value val(Json::arrayValue);
    BeJsValue json(val);
    json[0] = 1.5;
    json[1] = true;

    EXPECT_EQ(2, (int)val.size());
    EXPECT_DOUBLE_EQ(1.5, val[0u].asDouble());
    EXPECT_TRUE(val[1u].asBool());

    BeJsConst readBack(val);
    EXPECT_EQ(2, (int)readBack.size());
    EXPECT_DOUBLE_EQ(1.5, readBack[0].asDouble());
    EXPECT_TRUE(readBack[1].asBool());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(BridgeIncludeOrderBeJsValueFirst, CopiesFromRapidJsonDocument)
    {
    BeJsDocument doc;
    doc.Parse(R"({"big":9007199254740993,"s":"x"})");
    ASSERT_FALSE(doc.hasParseError());

    Json::Value val;
    BeJsValue json(val);
    json.From(doc);

    EXPECT_EQ(INT64_C(9007199254740993), val["big"].asInt64());
    EXPECT_STREQ("x", val["s"].asCString());
    }
