/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include "../../../Source/RulesDriven/RulesEngine/RulesetVariables.h"
#include "TestHelpers.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                04/2020
+===============+===============+===============+===============+===============+======*/
struct RulesetVariablesTests : ECPresentationTest
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesetVariablesTests, InitsFromJson)
    {
    Utf8CP stringJson = R"(
        [
            {"id": "stringId", "type": "string", "value": "stringValue"},
            {"id": "intId", "type": "int", "value": 10},
            {"id": "boolId", "type": "bool", "value": true},
            {"id": "id64Id", "type": "id64", "value": "1000"},
            {"id": "intsId", "type": "int[]", "value": [123, 456, 789]}
        ]
    )";
    Json::Value json = Json::Value::From(stringJson);

    RulesetVariables variables(json);
    EXPECT_STREQ("stringValue", variables.GetStringValue("stringId"));
    EXPECT_EQ(10, variables.GetIntValue("intId"));
    EXPECT_TRUE(variables.GetBoolValue("boolId"));
    EXPECT_EQ(1000, variables.GetIntValue("id64Id"));
    EXPECT_EQ(bvector<int64_t>({123, 456, 789}), variables.GetIntValues("intsId"));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesetVariablesTests, SetsAndReturnsStringValue)
    {
    RulesetVariables variables;
    // check default value
    EXPECT_STREQ("", variables.GetStringValue("stringId"));

    variables.SetStringValue("stringId", "stringValue");
    EXPECT_STREQ("stringValue", variables.GetStringValue("stringId"));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesetVariablesTests, SetsAndReturnsIntValue)
    {
    RulesetVariables variables;
    // check default value
    EXPECT_EQ(0, variables.GetIntValue("intId"));

    variables.SetIntValue("intId", 5);
    EXPECT_EQ(5, variables.GetIntValue("intId"));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesetVariablesTests, SetsAndReturnsBoolValue)
    {
    RulesetVariables variables;
    // check default value
    EXPECT_FALSE(variables.GetBoolValue("boolId"));

    variables.SetBoolValue("boolId", true);
    EXPECT_TRUE(variables.GetBoolValue("boolId"));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RulesetVariablesTests, SetsAndReturnsIntValues)
    {
    RulesetVariables variables;
    // check default value
    EXPECT_EQ(bvector<int64_t>(), variables.GetIntValues("intsId"));

    variables.SetIntValues("intsId", { 123, 456, 789 });
    EXPECT_EQ(bvector<int64_t>({ 123, 456, 789 }), variables.GetIntValues("intsId"));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Saulius.Skliutas                04/2020
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
* @betest                                       Saulius.Skliutas                04/2020
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
* @betest                                       Saulius.Skliutas                04/2020
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
* @betest                                       Saulius.Skliutas                04/2020
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
* @betest                                       Saulius.Skliutas                04/2020
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
* @betest                                       Saulius.Skliutas                04/2020
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