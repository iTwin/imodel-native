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
struct ExtendedDataRuleTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ExtendedDataRuleTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "ExtendedData",
        "condition": "test condition",
        "items": {
            "item1": "value1",
            "item2": "value2"
        }
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    ExtendedDataRule rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("test condition", rule.GetCondition().c_str());
    ASSERT_EQ(2, rule.GetItemsMap().size());
    ASSERT_TRUE(rule.GetItemsMap().end() != rule.GetItemsMap().find("item1"));
    ASSERT_TRUE(rule.GetItemsMap().end() != rule.GetItemsMap().find("item2"));
    EXPECT_STREQ("value1", rule.GetItemsMap().find("item1")->second.c_str());
    EXPECT_STREQ("value2", rule.GetItemsMap().find("item2")->second.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ExtendedDataRuleTests, WriteToJson)
    {
    bmap<Utf8String, Utf8String> items;
    items.Insert("key1", "value1");
    items.Insert("key2", "value2");
    ExtendedDataRule rule("cond", items);
    Json::Value json = rule.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "ruleType": "ExtendedData",
        "condition": "cond",
        "items": {
            "key1": "value1",
            "key2": "value2"
        }
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ExtendedDataRuleTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "e56e8e95b40431783661d1f59ed2ad49";

    // Make sure that introducing additional attributes with default values don't affect the hash
    ExtendedDataRule defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    ExtendedDataRule copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }
