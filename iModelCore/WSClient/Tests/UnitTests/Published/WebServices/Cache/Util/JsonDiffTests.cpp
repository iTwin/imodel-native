/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "JsonDiffTests.h"

#include <WebServices/Cache/Util/JsonDiff.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

#define TEST_GET_CHANGES_RUN(oldJsonString, newJsonString, outJsonString, expectedDiffString, flags) { \
    auto oldJson = ToRapidJson(oldJsonString); \
    auto newJson = ToRapidJson(newJsonString); \
    auto outJson = ToRapidJson(outJsonString); \
    JsonDiff jsonDiff(flags); \
    jsonDiff.GetChanges(*oldJson, *newJson, *outJson); \
    auto expectedOutJson = ToRapidJson(expectedDiffString); \
    EXPECT_EQ(*expectedOutJson, *outJson);}

#define TEST_GET_CHANGES(oldJsonString, newJsonString, expectedDiffString) \
    TEST_GET_CHANGES_RUN(oldJsonString, newJsonString, "{}", expectedDiffString, JsonDiff::Flags::Default)

#define TEST_GET_CHANGES2(oldJsonString, newJsonString, outJsonString, expectedDiffString) \
    TEST_GET_CHANGES_RUN(oldJsonString, newJsonString, outJsonString, expectedDiffString, JsonDiff::Flags::Default)

#define TEST_GET_CHANGES_FIND_DELETIONS(oldJsonString, newJsonString, expectedDiffString) \
    TEST_GET_CHANGES_RUN(oldJsonString, newJsonString, "{}", expectedDiffString, JsonDiff::Flags::FindDeletions)

#define TEST_GET_CHANGES_FIND_DELETIONS2(oldJsonString, newJsonString, outJsonString, expectedDiffString) \
    TEST_GET_CHANGES_RUN(oldJsonString, newJsonString, outJsonString, expectedDiffString, JsonDiff::Flags::FindDeletions)

using namespace ::testing;

/*--------------------------------------------------------------------------------------+
* @bsitest                                  Benediktas.Lipnickas                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(JsonDiffTests, GetChanges_OldAndNewAreNull_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(
        R"(null)",
        R"(null)",
        R"({ })");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                  Benediktas.Lipnickas                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(JsonDiffTests, GetChanges_OldIsNull_ReturnsAllNew)
    {
    TEST_GET_CHANGES(
        R"(null)",
        R"({"A" : "foo"})",
        R"({"A" : "foo"})");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                  Benediktas.Lipnickas                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(JsonDiffTests, GetChanges_NewIsNullAndIgnoreDeletions_ReturnsEmpty)
    {
    TEST_GET_CHANGES(
        R"({"A" : "foo"})",
        R"(null)",
        R"({ })");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                  Benediktas.Lipnickas                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(JsonDiffTests, GetChanges_NewIsNullAndFindDeletions_ReturnsAllOldAsDeleted)
    {
    TEST_GET_CHANGES_FIND_DELETIONS(
        R"({"A" : "foo"})",
        R"(null)",
        R"({"A" : null})");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                  Benediktas.Lipnickas                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(JsonDiffTests, GetChanges_EmptyJsons_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(
        R"({ })",
        R"({ })",
        R"({ })");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                  Benediktas.Lipnickas                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(JsonDiffTests, GetChanges_EqualJson_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(
        R"({ "A" : "foo" })",
        R"({ "A" : "foo" })",
        R"({ })");

    TEST_GET_CHANGES(
        R"({ "A" : 1 })",
        R"({ "A" : 1 })",
        R"({ })");

    TEST_GET_CHANGES(
        R"({ "A" : 3.14 })",
        R"({ "A" : 3.14 })",
        R"({ })");

    TEST_GET_CHANGES(
        R"({ "A" : null })",
        R"({ "A" : null })",
        R"({ })");

    TEST_GET_CHANGES(
        R"({ "A" : false })",
        R"({ "A" : false })",
        R"({ })");

    TEST_GET_CHANGES(
        R"({ "A" : true })",
        R"({ "A" : true })",
        R"({ })");

    TEST_GET_CHANGES(
        R"({ "A" : [] })",
        R"({ "A" : [] })",
        R"({ })");

    TEST_GET_CHANGES(
        R"({ "A" : ["foo","bar"] })",
        R"({ "A" : ["foo","bar"] })",
        R"({ })");

    TEST_GET_CHANGES(
        R"({ "A" : [1,2,3] })",
        R"({ "A" : [1,2,3] })",
        R"({ })");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                  Benediktas.Lipnickas                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(JsonDiffTests, GetChanges_MemberValueDifferent_ReturnsDiff)
    {
    TEST_GET_CHANGES(
        R"({ "A" : "foo" })",
        R"({ "A" : "bar" })",
        R"({ "A" : "bar" })");

    TEST_GET_CHANGES(
        R"({ "A" : 1 })",
        R"({ "A" : 2 })",
        R"({ "A" : 2 })");

    TEST_GET_CHANGES(
        R"({ "A" : 3.14 })",
        R"({ "A" : 6.28 })",
        R"({ "A" : 6.28 })");

    TEST_GET_CHANGES(
        R"({ "A" : [1] })",
        R"({ "A" : [2] })",
        R"({ "A" : [2] })");

    TEST_GET_CHANGES(
        R"({ "A" : ["foo"] })",
        R"({ "A" : ["bar"] })",
        R"({ "A" : ["bar"] })");

    TEST_GET_CHANGES(
        R"({ "A" : "1", "B" : "1", "C" : "1" })",
        R"({ "A" : "2", "B" : "1", "C" : "3" })",
        R"({ "A" : "2", "C" : "3" })");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                  Benediktas.Lipnickas                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(JsonDiffTests, GetChanges_MemberAdded_ReturnsDiff)
    {
    TEST_GET_CHANGES(
        R"({ })",
        R"({ "A" : "foo" })",
        R"({ "A" : "foo" })");

    TEST_GET_CHANGES(
        R"({ })",
        R"({ "A" : 1 })",
        R"({ "A" : 1 })");

    TEST_GET_CHANGES(
        R"({ })",
        R"({ "A" : 3.14 })",
        R"({ "A" : 3.14 })");

    TEST_GET_CHANGES(
        R"({ })",
        R"({ "A" : null })",
        R"({ "A" : null })");

    TEST_GET_CHANGES(
        R"({ })",
        R"({ "A" : false })",
        R"({ "A" : false })");

    TEST_GET_CHANGES(
        R"({ })",
        R"({ "A" : true })",
        R"({ "A" : true })");

    TEST_GET_CHANGES(
        R"({ })",
        R"({ "A" : [] })",
        R"({ "A" : [] })");

    TEST_GET_CHANGES(
        R"({ })",
        R"({ "A" : ["foo","bar"] })",
        R"({ "A" : ["foo","bar"] })");

    TEST_GET_CHANGES(
        R"({ })",
        R"({ "A" : [1,2,3] })",
        R"({ "A" : [1,2,3] })");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                  Benediktas.Lipnickas                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(JsonDiffTests, GetChanges_MemberDeleted_IgnoresDeletedMembers)
    {
    TEST_GET_CHANGES(
        R"({ "A" : "foo" })",
        R"({ })",
        R"({ })");

    TEST_GET_CHANGES(
        R"({ "A" : 1 })",
        R"({ })",
        R"({ })");

    TEST_GET_CHANGES(
        R"({ "A" : 3.14 })",
        R"({ })",
        R"({ })");

    TEST_GET_CHANGES(
        R"({ "A" : null })",
        R"({ })",
        R"({ })");

    TEST_GET_CHANGES(
        R"({ "A" : false })",
        R"({ })",
        R"({ })");

    TEST_GET_CHANGES(
        R"({ "A" : true })",
        R"({ })",
        R"({ })");

    TEST_GET_CHANGES(
        R"({ "A" : [] })",
        R"({ })",
        R"({ })");

    TEST_GET_CHANGES(
        R"({ "A" : ["foo","bar"] })",
        R"({ })",
        R"({ })");

    TEST_GET_CHANGES(
        R"({ "A" : [1,2,3] })",
        R"({ })",
        R"({ })");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                  Benediktas.Lipnickas                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(JsonDiffTests, GetChanges_MemberDeletedAndFindDeletions_ReturnsNullMember)
    {
    TEST_GET_CHANGES_FIND_DELETIONS(
        R"({ "A" : "foo" })",
        R"({ })",
        R"({ "A" : null })");

    TEST_GET_CHANGES_FIND_DELETIONS(
        R"({ "A" : 1 })",
        R"({ })",
        R"({ "A" : null })");

    TEST_GET_CHANGES_FIND_DELETIONS(
        R"({ "A" : 3.14 })",
        R"({ })",
        R"({ "A" : null })");

    TEST_GET_CHANGES_FIND_DELETIONS(
        R"({ "A" : null })",
        R"({ })",
        R"({ "A" : null })");

    TEST_GET_CHANGES_FIND_DELETIONS(
        R"({ "A" : false })",
        R"({ })",
        R"({ "A" : null })");

    TEST_GET_CHANGES_FIND_DELETIONS(
        R"({ "A" : true })",
        R"({ })",
        R"({ "A" : null })");

    TEST_GET_CHANGES_FIND_DELETIONS(
        R"({ "A" : [] })",
        R"({ })",
        R"({ "A" : null })");

    TEST_GET_CHANGES_FIND_DELETIONS(
        R"({ "A" : ["foo","bar"] })",
        R"({ })",
        R"({ "A" : null })");

    TEST_GET_CHANGES_FIND_DELETIONS(
        R"({ "A" : [1,2,3] })",
        R"({ })",
        R"({ "A" : null })");

    TEST_GET_CHANGES_FIND_DELETIONS(
        R"({ "A" : "foo" })",
        R"({ "B" : "boo" })",
        R"({ "A" : null, "B" : "boo" })");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                  Benediktas.Lipnickas                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(JsonDiffTests, GetChanges_MemberDeletedExistsInOutputAndFindDeletions_ReturnsNullMember)
    {
    TEST_GET_CHANGES_FIND_DELETIONS2(
        R"({ "A" : "foo" })",
        R"({ })",
        R"({ "A" : "otherValue" })",
        R"({ "A" : null })");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                  Benediktas.Lipnickas                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(JsonDiffTests, GetChanges_MemberTypeChanged_ReturnsNewMember)
    {
    // From String to X
    TEST_GET_CHANGES(
        R"({ "A" : "foo" })",
        R"({ "A" : 1 })",
        R"({ "A" : 1 })");

    TEST_GET_CHANGES(
        R"({ "A" : "foo" })",
        R"({ "A" : null })",
        R"({ "A" : null })");

    TEST_GET_CHANGES(
        R"({ "A" : "foo" })",
        R"({ "A" : true })",
        R"({ "A" : true })");

    TEST_GET_CHANGES(
        R"({ "A" : "foo" })",
        R"({ "A" : false })",
        R"({ "A" : false })");

    TEST_GET_CHANGES(
        R"({ "A" : "foo" })",
        R"({ "A" : [] })",
        R"({ "A" : [] })");

    // From Integer to X
    TEST_GET_CHANGES(
        R"({ "A" : 1 })",
        R"({ "A" : "foo" })",
        R"({ "A" : "foo" })");

    TEST_GET_CHANGES(
        R"({ "A" : 1 })",
        R"({ "A" : 3.14 })",
        R"({ "A" : 3.14 })");

    TEST_GET_CHANGES(
        R"({ "A" : 1 })",
        R"({ "A" : null })",
        R"({ "A" : null })");

    TEST_GET_CHANGES(
        R"({ "A" : 1 })",
        R"({ "A" : true })",
        R"({ "A" : true })");

    TEST_GET_CHANGES(
        R"({ "A" : 1 })",
        R"({ "A" : false })",
        R"({ "A" : false })");

    TEST_GET_CHANGES(
        R"({ "A" : 1 })",
        R"({ "A" : [] })",
        R"({ "A" : [] })");

    // From Null to X
    TEST_GET_CHANGES(
        R"({ "A" : null })",
        R"({ "A" : "foo" })",
        R"({ "A" : "foo" })");

    TEST_GET_CHANGES(
        R"({ "A" : null })",
        R"({ "A" : 1 })",
        R"({ "A" : 1 })");

    TEST_GET_CHANGES(
        R"({ "A" : null })",
        R"({ "A" : true })",
        R"({ "A" : true })");

    TEST_GET_CHANGES(
        R"({ "A" : null })",
        R"({ "A" : false })",
        R"({ "A" : false })");

    TEST_GET_CHANGES(
        R"({ "A" : null })",
        R"({ "A" : [] })",
        R"({ "A" : [] })");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                  Benediktas.Lipnickas                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(JsonDiffTests, GetChanges_StringPropertyDifferentAndExistsInOutput_ReturnsDiff)
    {
    TEST_GET_CHANGES2(
        R"({ "A" : "foo" })",
        R"({ "A" : "bar" })",
        R"({ })",
        R"({ "A" : "bar" })");

    TEST_GET_CHANGES2(
        R"({ "A" : "foo" })",
        R"({ "A" : "bar" })",
        R"({ "A" : "kuk" })",
        R"({ "A" : "bar" })");

    TEST_GET_CHANGES2(
        R"({ "A" : "foo" })",
        R"({ "A" : "bar" })",
        R"({ "A" : 1 })",
        R"({ "A" : "bar" })");

    TEST_GET_CHANGES2(
        R"({ "A" : "foo" })",
        R"({ "A" : "bar" })",
        R"({ "A" : true })",
        R"({ "A" : "bar" })");

    TEST_GET_CHANGES2(
        R"({ "A" : "foo" })",
        R"({ "A" : "bar" })",
        R"({ "A" : false })",
        R"({ "A" : "bar" })");

    TEST_GET_CHANGES2(
        R"({ "A" : "foo" })",
        R"({ "A" : "bar" })",
        R"({ "A" : null })",
        R"({ "A" : "bar" })");

    TEST_GET_CHANGES2(
        R"({ "A" : "foo" })",
        R"({ "A" : "bar" })",
        R"({ "A" : [] })",
        R"({ "A" : "bar" })");

    TEST_GET_CHANGES2(
        R"({ "A" : "foo" })",
        R"({ "A" : "bar" })",
        R"({ "A" : { "B" : 1 } })",
        R"({ "A" : "bar" })");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                  Benediktas.Lipnickas                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(JsonDiffTests, GetChanges_PropertyDifferentExistsInDeepOutput_ReturnsDiff)
    {
    TEST_GET_CHANGES2(
        R"({ "A" : { "B" : 1 } })",
        R"({ "A" : { "B" : 2 } })",
        R"({ "A" : { "B" : 5, "C" : "bar" } })",
        R"({ "A" : { "B" : 2, "C" : "bar" } })");

    TEST_GET_CHANGES2(
        R"({ "A" : { "B" : 1 } })",
        R"({ "A" : { "B" : 2 } })",
        R"({ "A" : { "B" : "foo", "C" : "bar" } })",
        R"({ "A" : { "B" : 2,     "C" : "bar" } })");

    TEST_GET_CHANGES2(
        R"({ "A" : { "B" : [1,2] } })",
        R"({ "A" : { "B" : [1,3] } })",
        R"({ "A" : { "B" : [4,5], "C" : "bar" } })",
        R"({ "A" : { "B" : [1,3], "C" : "bar" } })");

    TEST_GET_CHANGES2(
        R"({ "A" : true })",
        R"({ "A" : false })",
        R"({ "B" : { "C" : [4,5], "D" : "bar" } })",
        R"({ "A" : false, "B" : { "C" : [4,5], "D" : "bar" } })");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                  Benediktas.Lipnickas                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(JsonDiffTests, GetChanges_PropertyDifferentDeeply_ReturnsDiff)
    {
    TEST_GET_CHANGES(
        R"({ })",
        R"({ "A": { "B" : "Val1", "C" : "Val2" } })",
        R"({ "A": { "B" : "Val1", "C" : "Val2" } })");

    TEST_GET_CHANGES(
        R"({ "A": { "B" : "Val1" } })",
        R"({ "A": { "B" : "Val1" } })",
        R"({ })");

    TEST_GET_CHANGES(
        R"({ "A": { "B" : "Val1" } })",
        R"({ "A": { "B" : "Val2" } })",
        R"({ "A": { "B" : "Val2" } })");

    TEST_GET_CHANGES(
        R"({})",
        R"({ "A": { "B" : "Val1" } })",
        R"({ "A": { "B" : "Val1" } })");

    TEST_GET_CHANGES(
        R"({ "A" : { "B" : [1,2], "C" : "bar" } })",
        R"({ "A" : { "B" : [1,3], "C" : "bar" } })",
        R"({ "A" : { "B" : [1,3] } })");

    TEST_GET_CHANGES(
        R"({ "A" : 3, "B": { "C" : true } })",
        R"({ "A" : 4, "B": { "C" : true } })",
        R"({ "A" : 4 })");

    TEST_GET_CHANGES(
        R"({ "A" : 3, "B": { "C" : true, "D" : 4 } })",
        R"({ "A" : 3, "B": { "C" : false, "D" : 4 } })",
        R"({ "B": { "C" : false } })");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                  Benediktas.Lipnickas                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(JsonDiffTests, GetChanges_ArrayOfIntsElementRemoved_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(
        R"({ "A" : [1] })",
        R"({ "A" : [] })",
        R"({ "A" : [] })");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                  Benediktas.Lipnickas                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(JsonDiffTests, GetChanges_ArrayOfIntsPropertyEqual_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(
        R"({ "A" : [1] })",
        R"({ "A" : [1] })",
        R"({ })");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                  Benediktas.Lipnickas                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(JsonDiffTests, GetChanges_ArrayOfIntsPropertyDifferent_ReturnsNewArray)
    {
    TEST_GET_CHANGES(
        R"({ "A" : [1] })",
        R"({ "A" : [3,4] })",
        R"({ "A" : [3,4] })");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                  Benediktas.Lipnickas                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(JsonDiffTests, GetChanges_ArrayOfObjectsPropertyEqual_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(
        R"({ "A" : [{"A":1}] })",
        R"({ "A" : [{"A":1}] })",
        R"({ })");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                  Benediktas.Lipnickas                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(JsonDiffTests, GetChanges_ArrayOfObjectsPropertyDifferent_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(
        R"({ "A" : [ {"A":1} ] })",
        R"({ "A" : [ {"A":2} ] })",
        R"({ "A" : [ {"A":2} ] })");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                  Benediktas.Lipnickas                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(JsonDiffTests, GetChanges_ArrayOfDeepObjectsPropertyDifferent_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(
        R"({ "A" : [ {"A":[ {"A":"eq", "B":true} ]} ] })",
        R"({ "A" : [ {"A":[ {"A":"eq", "B":false} ]} ] })",
        R"({ "A" : [ {"A":[ {"A":"eq", "B":false} ]} ] })");
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                  Benediktas.Lipnickas                   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(JsonDiffTests, GetChanges_ArrayOfDeepObjectsPropertyEqual_ReturnsEmptyJson)
    {
    TEST_GET_CHANGES(
        R"({ "A" : [ {"A":[ {"A":"eq", "B":false} ]} ] })",
        R"({ "A" : [ {"A":[ {"A":"eq", "B":false} ]} ] })",
        R"({ })");
    }
