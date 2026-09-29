/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct LabelOverrideTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LabelOverrideTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "LabelOverride",
        "label":"label",
        "description":"description"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    LabelOverride override;
    EXPECT_TRUE(override.ReadJson(json));
    EXPECT_STREQ("label", override.GetLabel().c_str());
    EXPECT_STREQ("description", override.GetDescription().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LabelOverrideTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "LabelOverride"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    LabelOverride override;
    EXPECT_TRUE(override.ReadJson(json));
    EXPECT_STREQ("", override.GetLabel().c_str());
    EXPECT_STREQ("", override.GetDescription().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LabelOverrideTests, WriteToJson)
    {
    LabelOverride rule("cond", 123, "label", "descr");
    BeJsDocument json = rule.WriteJson();
    BeJsDocument expected(R"({
        "ruleType": "LabelOverride",
        "priority": 123,
        "condition": "cond",
        "label": "label",
        "description": "descr"
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LabelOverrideTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "009a12f57ed6c77d8d270f0332717f4e";

    // Make sure that introducing additional attributes with default values don't affect the hash
    LabelOverride defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    LabelOverride copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }
