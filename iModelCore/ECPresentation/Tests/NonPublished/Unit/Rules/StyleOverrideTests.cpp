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
struct StyleOverrideTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(StyleOverrideTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "StyleOverride",
        "foreColor": "fore",
        "backColor": "back",
        "fontStyle": "font"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    StyleOverride override;
    EXPECT_TRUE(override.ReadJson(json));
    EXPECT_STREQ("fore", override.GetForeColor().c_str());
    EXPECT_STREQ("back", override.GetBackColor().c_str());
    EXPECT_STREQ("font", override.GetFontStyle().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(StyleOverrideTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "StyleOverride"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    StyleOverride override;
    EXPECT_TRUE(override.ReadJson(json));
    EXPECT_STREQ("", override.GetForeColor().c_str());
    EXPECT_STREQ("", override.GetBackColor().c_str());
    EXPECT_STREQ("", override.GetFontStyle().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(StyleOverrideTests, WriteToJson)
    {
    StyleOverride rule("cond", 123, "f", "b", "fs");
    BeJsDocument json = rule.WriteJson();
    BeJsDocument expected(R"({
        "ruleType": "StyleOverride",
        "priority": 123,
        "condition": "cond",
        "foreColor": "f",
        "backColor": "b",
        "fontStyle": "fs"
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(StyleOverrideTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "2dfd1232cdca31eecb210f40c8f67bb3";

    // Make sure that introducing additional attributes with default values don't affect the hash
    StyleOverride defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    StyleOverride copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }
