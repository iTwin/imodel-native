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

TEST_F(JsonDiffTests, GetChanges_NullPropertyDidNotChange_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(R"({ "A" : null })",
                     R"({ "A" : null })",
                     R"({ })");
    }

TEST_F(JsonDiffTests, GetChanges_FalsePropertyDidNotChange_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(R"({ "A" : false })",
                     R"({ "A" : false })",
                     R"({ })");
    }

TEST_F(JsonDiffTests, GetChanges_TruePropertyDidNotChange_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(R"({ "A" : true })",
                     R"({ "A" : true })",
                     R"({ })");
    }

TEST_F(JsonDiffTests, GetChanges_StringPropertyDifferentAndItExistsInOutput_ReturnsDiff)
    {
    auto oldJson = ToRapidJson(R"({ "A" : "Old" })");
    auto newJson = ToRapidJson(R"({ "A" : "New" })");
    auto outJson = ToRapidJson(R"({ "A" : "Foo" })");

    JsonDiff jsonDiff;
    jsonDiff.GetChanges(*oldJson, *newJson, *outJson);

    auto expectedOutJson = ToRapidJson(R"({ "A" : "New" })");
    EXPECT_EQ(*expectedOutJson, *outJson);
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
                     R"({ })");
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

TEST_F(JsonDiffTests, GetChanges_EmptyArrays_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(R"({ "A" : [] })",
                     R"({ "A" : [] })",
                     R"({ })");
    }

TEST_F(JsonDiffTests, GetChanges_ArrayOfIntsElementRemoved_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(R"({ "A" : [1] })",
                     R"({ "A" : [] })",
                     R"({ "A" : [] })");
    }

TEST_F(JsonDiffTests, GetChanges_ArrayOfIntsPropertyEqual_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(R"({ "A" : [1] })",
                     R"({ "A" : [1] })",
                     R"({ })");
    }

TEST_F(JsonDiffTests, GetChanges_ArrayOfIntsPropertyDifferent_ReturnsNewArray)
    {
    TEST_GET_CHANGES(R"({ "A" : [1] })",
                     R"({ "A" : [3,4] })",
                     R"({ "A" : [3,4] })");
    }

TEST_F(JsonDiffTests, GetChanges_ArrayOfIntsPropertyDeleted_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(R"({ "A" : [1] })",
                     R"({ })",
                     R"({ })");
    }

TEST_F(JsonDiffTests, GetChanges_ArrayOfIntsPropertyDeletedDoNotIgnoreDeletedProperties_ReturnsDiff)
    {
    TEST_GET_CHANGES_DO_NOT_IGNORE_DEL(
                      R"({ "A" : [1] })",
                      R"({ })",
                      R"({ "A" : null })");
    }

TEST_F(JsonDiffTests, GetChanges_ArrayOfIntsPropertyAdded_ReturnsNewArray)
    {
    TEST_GET_CHANGES(R"({ })",
                     R"({ "A" : [1] })",
                     R"({ "A" : [1] })");
    }

TEST_F(JsonDiffTests, GetChanges_ArrayOfObjectsPropertyEqual_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(R"({ "A" : [{"A":1}] })",
                     R"({ "A" : [{"A":1}] })",
                     R"({ })");
    }

TEST_F(JsonDiffTests, GetChanges_ArrayOfObjectsPropertyDifferent_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(R"({ "A" : [ {"A":1} ] })",
                     R"({ "A" : [ {"A":2} ] })",
                     R"({ "A" : [ {"A":2} ] })");
    }

TEST_F(JsonDiffTests, GetChanges_ArrayOfDeepObjectsPropertyDifferent_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(R"({ "A" : [ {"A":[ {"A":"eq", "B":true} ]} ] })",
                     R"({ "A" : [ {"A":[ {"A":"eq", "B":false} ]} ] })",
                     R"({ "A" : [ {"A":[ {"A":"eq", "B":false} ]} ] })");
    }

TEST_F(JsonDiffTests, GetChanges_ArrayOfDeepObjectsPropertyEqual_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(R"({ "A" : [ {"A":[ {"A":"eq", "B":false} ]} ] })",
                     R"({ "A" : [ {"A":[ {"A":"eq", "B":false} ]} ] })",
                     R"({  })");
    }
