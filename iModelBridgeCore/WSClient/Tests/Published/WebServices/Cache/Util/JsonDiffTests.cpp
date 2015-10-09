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

#define TEST_GET_CHANGES(oldJsonString, newJsonString, expectedDiffString) { \
    TestGetChanges(oldJsonString, newJsonString, expectedDiffString); \
    EXPECT_FALSE(HasFailure()); }

using namespace ::testing;

void TestGetChanges(Utf8StringCR oldJsonString, Utf8StringCR newJsonString, Utf8StringCR expectedDiffString)
    {
    auto oldJson = ToRapidJson(oldJsonString);
    auto newJson = ToRapidJson(newJsonString);

    rapidjson::Document outJson;
    JsonDiff jsonDiff;
    jsonDiff.GetChanges(*oldJson, *newJson, outJson);

    auto expectedOutJson = ToRapidJson(expectedDiffString);
    EXPECT_EQ(*expectedOutJson, outJson);
    }

TEST_F(JsonDiffTests, CompareTest_EmptyJsons_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(R"({ })",
                     R"({ })",
                     R"({ })");
    }

TEST_F(JsonDiffTests, CompareTest_StringPropertyEqual_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(R"({ "A" : "1" })",
                     R"({ "A" : "1" })",
                     R"({ })");
    }

TEST_F(JsonDiffTests, CompareTest_StringPropertyDifferent_ReturnsDiff)
    {
    TEST_GET_CHANGES(R"({ "A" : "1" })",
                     R"({ "A" : "2" })",
                     R"({ "A" : "2" })");
    }


TEST_F(JsonDiffTests, CompareTest_StringPropertyDeleted_ReturnsDiff)
    {
    TEST_GET_CHANGES(R"({ "A" : "1" })",
                     R"({ })",
                     R"({ "A" : null })");
    }

TEST_F(JsonDiffTests, CompareTest_StringPropertyAdded_ReturnsDiff)
    {
    TEST_GET_CHANGES(R"({ })",
                     R"({ "A" : "1" })",
                     R"({ "A" : "1" })");
    }

TEST_F(JsonDiffTests, CompareTest_MultipleStringPropertiesDifferent_ReturnsDiff)
    {
    TEST_GET_CHANGES(R"({ "A" : "1", "B" : "1", "C" : "1" })",
                     R"({ "A" : "2", "B" : "1", "C" : "3" })",
                     R"({ "A" : "2", "C" : "3" })");
    }

TEST_F(JsonDiffTests, CompareTest_IntegerPropertyEqual_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(R"({ "A" : 1 })",
                     R"({ "A" : 1 })",
                     R"({ })");
    }

TEST_F(JsonDiffTests, CompareTest_IntegerPropertyDifferent_ReturnsDiff)
    {
    TEST_GET_CHANGES(R"({ "A" : 1 })",
                     R"({ "A" : 2 })",
                     R"({ "A" : 2 })");
    }

TEST_F(JsonDiffTests, CompareTest_IntegerPropertyDeleted_ReturnsDiff)
    {
    TEST_GET_CHANGES(R"({ "A" : 1 })",
                     R"({ })",
                     R"({ "A" : null })");
    }

TEST_F(JsonDiffTests, CompareTest_IntegerPropertyAdded_ReturnsDiff)
    {
    TEST_GET_CHANGES(R"({ })",
                     R"({ "A" : 1 })",
                     R"({ "A" : 1 })");
    }