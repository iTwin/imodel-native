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
struct DefaultPropertyCategoryOverrideTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultPropertyCategoryOverrideTests, CopyConstructorCopiesSpecification)
    {
    DefaultPropertyCategoryOverride ovr(*new PropertyCategorySpecification("test", "label"));
    ovr.AddRequiredSchemaSpecification(*new RequiredSchemaSpecification("TestSchema"));

    DefaultPropertyCategoryOverride copy(ovr);

    EXPECT_NE(&ovr.GetSpecification(), &copy.GetSpecification());
    EXPECT_STREQ(ovr.GetSpecification().GetId().c_str(), copy.GetSpecification().GetId().c_str());

    ASSERT_EQ(1, copy.GetRequiredSchemaSpecifications().size());
    EXPECT_NE(ovr.GetRequiredSchemaSpecifications()[0], copy.GetRequiredSchemaSpecifications()[0]);
    EXPECT_STREQ(ovr.GetRequiredSchemaSpecifications()[0]->GetName().c_str(), copy.GetRequiredSchemaSpecifications()[0]->GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultPropertyCategoryOverrideTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "DefaultPropertyCategoryOverride",
        "priority": 999,
        "specification": {
            "id": "test id",
            "label": "test label"
        }
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    DefaultPropertyCategoryOverride rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_EQ(999, rule.GetPriority());
    EXPECT_STREQ("test id", rule.GetSpecification().GetId().c_str());
    EXPECT_STREQ("test label", rule.GetSpecification().GetLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultPropertyCategoryOverrideTests, WriteToJson)
    {
    DefaultPropertyCategoryOverride rule(*new PropertyCategorySpecification("test id", "test label"), 789);
    Json::Value json = rule.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "ruleType": "DefaultPropertyCategoryOverride",
        "priority": 789,
        "specification": {
            "id": "test id",
            "label": "test label"
        }
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DefaultPropertyCategoryOverrideTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "3ca265ca14d91f1a2351d0e53b9cb7d4";

    // Make sure that introducing additional attributes with default values don't affect the hash
    DefaultPropertyCategoryOverride defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    DefaultPropertyCategoryOverride copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }
