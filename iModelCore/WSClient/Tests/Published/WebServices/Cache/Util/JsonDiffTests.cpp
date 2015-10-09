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

#define TEST_GET_CHANGES_IGNORE_DEL(oldJsonString, newJsonString, expectedDiffString, ignoreDeletedProperties) { \
    auto oldJson = ToRapidJson(oldJsonString);              \
    auto newJson = ToRapidJson(newJsonString);              \
                                                            \
    rapidjson::Document outJson;                            \
    JsonDiff jsonDiff(true, ignoreDeletedProperties);       \
    jsonDiff.GetChanges(*oldJson, *newJson, outJson);       \
                                                            \
    auto expectedOutJson = ToRapidJson(expectedDiffString); \
    EXPECT_EQ(*expectedOutJson, outJson); }

#define TEST_GET_CHANGES(oldJsonString, newJsonString, expectedDiffString) \
    TEST_GET_CHANGES_IGNORE_DEL(oldJsonString, newJsonString, expectedDiffString, true)

#define TEST_GET_CHANGES_DO_NOT_IGNORE_DEL(oldJsonString, newJsonString, expectedDiffString) \
    TEST_GET_CHANGES_IGNORE_DEL(oldJsonString, newJsonString, expectedDiffString, false)

using namespace ::testing;

TEST_F(JsonDiffTests, GetChanges_EmptyJsons_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(R"({ })",
                     R"({ })",
                     R"({ })");
    }

TEST_F(JsonDiffTests, GetChanges_StringPropertyEqual_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(R"({ "A" : "1" })",
                     R"({ "A" : "1" })",
                     R"({ })");
    }

TEST_F(JsonDiffTests, GetChanges_StringPropertyDifferent_ReturnsDiff)
    {
    TEST_GET_CHANGES(R"({ "A" : "1" })",
                     R"({ "A" : "2" })",
                     R"({ "A" : "2" })");
    }


TEST_F(JsonDiffTests, GetChanges_StringPropertyDeleted_ReturnsDiff)
    {
    TEST_GET_CHANGES(R"({ "A" : "1" })",
                     R"({ })",
                     R"({ })");
    }

TEST_F(JsonDiffTests, GetChanges_StringPropertyDeletedDoNotIgnoreDeletedProperties_ReturnsDiff)
    {
    TEST_GET_CHANGES_DO_NOT_IGNORE_DEL(
                     R"({ "A" : "1" })",
                     R"({ })",
                     R"({ "A" : null })");
    }

TEST_F(JsonDiffTests, GetChanges_StringPropertyAdded_ReturnsDiff)
    {
    TEST_GET_CHANGES(R"({ })",
                     R"({ "A" : "1" })",
                     R"({ "A" : "1" })");
    }

TEST_F(JsonDiffTests, GetChanges_MultipleStringPropertiesDifferent_ReturnsDiff)
    {
    TEST_GET_CHANGES(R"({ "A" : "1", "B" : "1", "C" : "1" })",
                     R"({ "A" : "2", "B" : "1", "C" : "3" })",
                     R"({ "A" : "2", "C" : "3" })");
    }

TEST_F(JsonDiffTests, GetChanges_IntegerPropertyEqual_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(R"({ "A" : 1 })",
                     R"({ "A" : 1 })",
                     R"({ })");
    }

TEST_F(JsonDiffTests, GetChanges_IntegerPropertyDifferent_ReturnsDiff)
    {
    TEST_GET_CHANGES(R"({ "A" : 1 })",
                     R"({ "A" : 2 })",
                     R"({ "A" : 2 })");
    }

TEST_F(JsonDiffTests, GetChanges_IntegerPropertyDeleted_ReturnsDiff)
    {
    TEST_GET_CHANGES(R"({ "A" : 1 })",
                     R"({ })",
                     R"({ })");
    }

TEST_F(JsonDiffTests, GetChanges_IntegerPropertyDeletedDoNotIgnoreDeletedProperties_ReturnsDiff)
    {
    TEST_GET_CHANGES_DO_NOT_IGNORE_DEL(
                     R"({ "A" : "1" })",
                     R"({ })",
                     R"({ "A" : null })");
    }

TEST_F(JsonDiffTests, GetChanges_IntegerPropertyAdded_ReturnsDiff)
    {
    TEST_GET_CHANGES(R"({ })",
                     R"({ "A" : 1 })",
                     R"({ "A" : 1 })");
    }

TEST_F(JsonDiffTests, GetChanges_NullPropertyDeleted_ReturnsDiff)
    {
    TEST_GET_CHANGES(R"({ "A" : null })",
                     R"({ })",
                     R"({ })");
    }

TEST_F(JsonDiffTests, GetChanges_NullPropertyDeletedDoNotIgnoreDeletedProperties_ReturnsDiff)
    {
    TEST_GET_CHANGES_DO_NOT_IGNORE_DEL(
                     R"({ "A" : null })",
                     R"({ })",
                     R"({ "A" : null })");
    }

TEST_F(JsonDiffTests, GetChanges_TruePropertyDeleted_ReturnsDiff)
    {
    TEST_GET_CHANGES(R"({ "A" : true })",
                     R"({ })",
                     R"({ "a" :4})");
    }

TEST_F(JsonDiffTests, GetChanges_TruePropertyDeletedDoNotIgnoreDeletedProperties_ReturnsDiff)
    {
    TEST_GET_CHANGES_DO_NOT_IGNORE_DEL(
                     R"({ "A" : true })",
                     R"({ })",
                     R"({ "A" : null })");
    }

TEST_F(JsonDiffTests, GetChanges_FalsePropertyDeleted_ReturnsDiff)
    {
    TEST_GET_CHANGES(R"({ "A" : false })",
                     R"({ })",
                     R"({ })");
    }

TEST_F(JsonDiffTests, GetChanges_FalsePropertyDeletedDoNotIgnoreDeletedProperties_ReturnsDiff)
    {
    TEST_GET_CHANGES_DO_NOT_IGNORE_DEL(
                     R"({ "A" : false })",
                     R"({ })",
                     R"({ "A" : null })");
    }

TEST_F(JsonDiffTests, GetChanges_TypeCangedTrueToFalse_ReturnsDiff)
    {
    TEST_GET_CHANGES(R"({ "A" : true})",
                     R"({ "A" : false })",
                     R"({ "A" : false })");
    }

TEST_F(JsonDiffTests, GetChanges_TypeCangedStringToFalse_ReturnsDiff)
    {
    TEST_GET_CHANGES(R"({ "A" : "foo" })",
                     R"({ "A" : false })",
                     R"({ "A" : false })");
    }

TEST_F(JsonDiffTests, GetChanges_TypeCangedTrueToInteger_ReturnsDiff)
    {
    TEST_GET_CHANGES(R"({ "A" : true })",
                     R"({ "A" : 5 })",
                     R"({ "A" : 5 })");
    }

TEST_F(JsonDiffTests, GetChanges_TypeCangedTrueToNull_ReturnsDiff)
    {
    TEST_GET_CHANGES(R"({ "A" : true })",
                     R"({ "A" : null })",
                     R"({ "A" : null })");
    }

TEST_F(JsonDiffTests, GetChanges_TypeCangedNullToString_ReturnsDiff)
    {
    TEST_GET_CHANGES(R"({ "A" : null })",
                     R"({ "A" : "foo" })",
                     R"({ "A" : "foo" })");
    }