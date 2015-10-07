/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Cache/Util/JsonDiffTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "JsonDiffTests.h"

#include <WebServices/Cache/Util/JsonDiff.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

using namespace ::testing;

TEST_F(JsonDiffTests, Diff_EqualJsons_ReturnsEmptyJson)
    {
    auto oldJson = ToRapidJson(R"({ "A" : "1" })");
    auto newJson = ToRapidJson(R"({ "A" : "1" })");

    rapidjson::Value outJson;

    JsonDiff jsonDiff;
    jsonDiff.GetChanges(*oldJson, *newJson, outJson);

    EXPECT_TRUE(outJson.IsNull());
    }

//TEST_F(JsonDiffTests, Diff_StringPropertyDifferent_ReturnsDiff)
//    {
//    auto oldJson = ToRapidJson(R"({ "A" : "1", "B" : "1" })");
//    auto newJson = ToRapidJson(R"({ "A" : "2", "B" : "1" })");
//
//    rapidjson::Value outJson;
//
//    JsonDiff jsonDiff;
//    jsonDiff.GetChanges(*oldJson, *newJson, outJson);
//
//    auto expectedOutJson = ToRapidJson(R"({ "A" : "2" })");
//
//    EXPECT_EQ(*expectedOutJson, outJson);
//    }