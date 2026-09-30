/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

// The USE_JSONCPP bridge in BeJsValue.h is header-order sensitive: it must work whether json/json.h
// has already been included or is pulled in by BeJsValue.h itself. This translation unit pins the
// "json/json.h first" order; BridgeIncludeOrderBeJsValueFirstTests.cpp pins the other one.
#include <json/json.h>
#include <BeRapidJson/BeJsValue.h>

#include <Bentley/BeTest.h>

USING_NAMESPACE_BENTLEY

#ifndef USE_JSONCPP
    #error "The BeJsonCpp unit tests must build with USE_JSONCPP defined; see tests/BuildTests.mke."
#endif

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(BridgeIncludeOrderJsonFirst, RoundTripsThroughBeJsValue)
    {
    Json::Value val(Json::objectValue);
    BeJsValue json(val);
    json["answer"] = 42;
    json["name"] = "bridge";

    EXPECT_EQ(42, val["answer"].asInt());
    EXPECT_STREQ("bridge", val["name"].asCString());

    BeJsConst readBack(val);
    EXPECT_EQ(42, readBack["answer"].asInt());
    EXPECT_STREQ("bridge", readBack["name"].asCString());
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST(BridgeIncludeOrderJsonFirst, CopiesLargeIntegersExactly)
    {
    Json::Value val(Json::objectValue);
    BeJsValue json(val);
    json["big"] = UINT64_C(18446744073709551615);

    BeJsDocument doc;
    doc.From(BeJsConst(val));
    EXPECT_EQ(UINT64_C(18446744073709551615), doc["big"].GetUInt64());
    }
