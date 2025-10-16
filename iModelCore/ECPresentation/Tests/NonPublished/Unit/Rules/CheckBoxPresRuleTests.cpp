/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct CheckBoxPresRuleTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CheckBoxPresRuleTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "CheckBox",
  	    "propertyName": "checkBoxProperty",
  	    "useInversedPropertyValue": true,
  	    "defaultValue": true,
  	    "isEnabled": "isEnabledExpression"
    })";
    BeJsDocument json(jsonString);
    ASSERT_FALSE(json.isNull());

    CheckBoxRule rule;
    ASSERT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("checkBoxProperty", rule.GetPropertyName().c_str());
    EXPECT_TRUE(rule.GetUseInversedPropertyValue());
    EXPECT_TRUE(rule.GetDefaultValue());
    EXPECT_STREQ("isEnabledExpression", rule.GetIsEnabled().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CheckBoxPresRuleTests, LoadsFromJsonWhenIsEnabledIsBoolean)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "CheckBox",
  	    "propertyName": "checkBoxProperty",
  	    "isEnabled": true
    })";
    BeJsDocument json(jsonString);
    ASSERT_FALSE(json.isNull());

    CheckBoxRule rule;
    ASSERT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("checkBoxProperty", rule.GetPropertyName().c_str());
    EXPECT_STREQ("true", rule.GetIsEnabled().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CheckBoxPresRuleTests, WriteToJson)
    {
    CheckBoxRule rule("cond", 123, true, "prop", true, true, "a = b");
    BeJsDocument json = rule.WriteJson();
    BeJsDocument expected(R"({
        "ruleType": "CheckBox",
        "priority": 123,
        "onlyIfNotHandled": true,
        "condition": "cond",
        "propertyName": "prop",
        "useInversedPropertyValue": true,
        "defaultValue": true,
        "isEnabled": "a = b"
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CheckBoxPresRuleTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "CheckBox"
    })";
    BeJsDocument json(jsonString);
    ASSERT_FALSE(json.isNull());

    CheckBoxRule rule;
    ASSERT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("", rule.GetPropertyName().c_str());
    EXPECT_FALSE(rule.GetUseInversedPropertyValue());
    EXPECT_FALSE(rule.GetDefaultValue());
    EXPECT_STREQ("", rule.GetIsEnabled().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CheckBoxPresRuleTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "7ea0f1332ade5b23b34502a3bfe715a2";

    // Make sure that introducing additional attributes with default values don't affect the hash
    CheckBoxRule defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    CheckBoxRule copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }
