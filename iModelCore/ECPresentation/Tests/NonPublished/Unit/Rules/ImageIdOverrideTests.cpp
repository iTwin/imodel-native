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
struct ImageIdOverrideTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImageIdOverrideTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "ImageIdOverride",
        "imageIdExpression": "imgId"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    ImageIdOverride override;
    EXPECT_TRUE(override.ReadJson(json));
    EXPECT_STREQ("imgId", override.GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImageIdOverrideTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "ImageIdOverride"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    ImageIdOverride override;
    EXPECT_TRUE(override.ReadJson(json));
    EXPECT_STREQ("", override.GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImageIdOverrideTests, WriteToJson)
    {
    ImageIdOverride rule("cond", 123, "imageid");
    BeJsDocument json = rule.WriteJson();
    BeJsDocument expected(R"({
        "ruleType": "ImageIdOverride",
        "priority": 123,
        "condition": "cond",
        "imageIdExpression": "imageid"
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImageIdOverrideTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "1a35587b46fb965926801bd9c3c9f325";

    // Make sure that introducing additional attributes with default values don't affect the hash
    ImageIdOverride defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    ImageIdOverride copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }
