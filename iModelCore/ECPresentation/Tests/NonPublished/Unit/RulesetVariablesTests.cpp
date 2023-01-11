/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentation/RulesetVariables.h>
#include "../Helpers/TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RulesetVariablesTests : ECPresentationTest
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesetVariablesTests, InitsFromSerializedInternalJsonObject)
    {
    Utf8CP stringJson = R"(
        {
        "stringId": "stringValue",
        "intId": 10,
        "boolId": true,
        "id64Id": 1000,
        "intsId": [123, 456, 789]
        }
    )";
    
    RulesetVariables variables = RulesetVariables::FromSerializedInternalJsonObjectString(stringJson);
    EXPECT_STREQ("stringValue", variables.GetStringValue("stringId"));
    EXPECT_EQ(10, variables.GetIntValue("intId"));
    EXPECT_TRUE(variables.GetBoolValue("boolId"));
    EXPECT_EQ(1000, variables.GetIntValue("id64Id"));
    EXPECT_EQ(bvector<int64_t>({123, 456, 789}), variables.GetIntValues("intsId"));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesetVariablesTests, SetsAndReturnsStringValue)
    {
    RulesetVariables variables;
    // check default value
    EXPECT_STREQ("", variables.GetStringValue("stringId"));

    variables.SetStringValue("stringId", "stringValue");
    EXPECT_STREQ("stringValue", variables.GetStringValue("stringId"));

    variables.SetStringValue("stringId", "otherStringValue");
    EXPECT_STREQ("otherStringValue", variables.GetStringValue("stringId"));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesetVariablesTests, SetsAndReturnsIntValue)
    {
    RulesetVariables variables;
    // check default value
    EXPECT_EQ(0, variables.GetIntValue("intId"));

    variables.SetIntValue("intId", 5);
    EXPECT_EQ(5, variables.GetIntValue("intId"));

    variables.SetIntValue("intId", 6);
    EXPECT_EQ(6, variables.GetIntValue("intId"));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesetVariablesTests, SetsAndReturnsBoolValue)
    {
    RulesetVariables variables;
    // check default value
    EXPECT_FALSE(variables.GetBoolValue("boolId"));

    variables.SetBoolValue("boolId", true);
    EXPECT_TRUE(variables.GetBoolValue("boolId"));

    variables.SetBoolValue("boolId", false);
    EXPECT_FALSE(variables.GetBoolValue("boolId"));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesetVariablesTests, SetsAndReturnsIntValues)
    {
    RulesetVariables variables;
    // check default value
    EXPECT_EQ(bvector<int64_t>(), variables.GetIntValues("intsId"));

    variables.SetIntValues("intsId", { 123, 456, 789 });
    EXPECT_EQ(bvector<int64_t>({ 123, 456, 789 }), variables.GetIntValues("intsId"));

    variables.SetIntValues("intsId", { 753 });
    EXPECT_EQ(bvector<int64_t>({ 753}), variables.GetIntValues("intsId"));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesetVariablesTests, ReturnsValuesAsJson)
    {
    RulesetVariables variables;

    variables.SetStringValue("stringId", "stringValue");
    EXPECT_EQ(rapidjson::Value("stringValue"), variables.GetJsonValue("stringId"));

    variables.SetIntValue("intId", 5);
    EXPECT_EQ(rapidjson::Value(5), variables.GetJsonValue("intId"));

    variables.SetBoolValue("boolId", true);
    EXPECT_EQ(rapidjson::Value(true), variables.GetJsonValue("boolId"));

    variables.SetIntValues("intsId", { 123, 456, 789 });
    rapidjson::Document jsonArray(rapidjson::kArrayType);
    jsonArray.PushBack(rapidjson::Value(123), jsonArray.GetAllocator());
    jsonArray.PushBack(rapidjson::Value(456), jsonArray.GetAllocator());
    jsonArray.PushBack(rapidjson::Value(789), jsonArray.GetAllocator());
    EXPECT_EQ(jsonArray, variables.GetJsonValue("intsId"));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesetVariablesTests, MergesWithUserSettings_AddsValuesFromUserSettings)
    {
    TestUserSettings userSettings;
    RulesetVariables variables;

    userSettings.SetSettingValue("stringId", "stringSettingValue");
    userSettings.SetSettingIntValue("intId", 5);
    variables.Merge(userSettings);

    EXPECT_STREQ(variables.GetStringValue("stringId"), userSettings.GetSettingValue("stringId").c_str());
    EXPECT_EQ(variables.GetIntValue("intId"), userSettings.GetSettingIntValue("intId"));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesetVariablesTests, MergesWithUserSettings_AddsOnlyMissingSettingsValues)
    {
    TestUserSettings userSettings;
    RulesetVariables variables;

    userSettings.SetSettingValue("stringId", "settingValue");
    userSettings.SetSettingIntValue("intId", 5);

    variables.SetStringValue("stringId", "variableValue");
    variables.Merge(userSettings);

    EXPECT_STREQ(variables.GetStringValue("stringId"), "variableValue");
    EXPECT_EQ(variables.GetIntValue("intId"), userSettings.GetSettingIntValue("intId"));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesetVariablesTests, ContainsVariablesIfTheyAreEmpty)
    {
    RulesetVariables empty;
    RulesetVariables variables;

    variables.SetStringValue("stringId", "variableValue");
    EXPECT_TRUE(variables.Contains(empty));
    EXPECT_FALSE(empty.Contains(variables));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesetVariablesTests, ContainsVariablesIfMembersHaveSameValues)
    {
    RulesetVariables variables1;
    RulesetVariables variables2;

    variables1.SetStringValue("stringId", "variableValue");
    variables1.SetIntValue("intId", 5);

    variables2.SetStringValue("stringId", "variableValue");
    variables2.SetIntValue("intId", 5);
    variables2.SetBoolValue("boolId", true);
    variables2.SetStringValue("stringId2", "variableValue2");

    EXPECT_TRUE(variables2.Contains(variables1));
    EXPECT_FALSE(variables1.Contains(variables2));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesetVariablesTests, DoesNotContainVariablesIfMembersHaveDifferentValues)
    {
    RulesetVariables variables1;
    RulesetVariables variables2;

    variables1.SetStringValue("stringId", "variableValue");
    variables1.SetIntValue("intId", 5);

    variables2.SetStringValue("stringId", "differentVariableValue");
    variables2.SetIntValue("intId", 5);
    variables2.SetBoolValue("boolId", true);

    EXPECT_FALSE(variables2.Contains(variables1));
    EXPECT_FALSE(variables1.Contains(variables2));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesetVariablesTests, EmptyVariablesMatchVariablesWithoutValues)
    {
    RulesetVariables variables{ RulesetVariableEntry("v", rapidjson::Value()) };
    RulesetVariables empty;

    EXPECT_TRUE(variables.Contains(empty));
    EXPECT_TRUE(empty.Contains(variables));
    }
