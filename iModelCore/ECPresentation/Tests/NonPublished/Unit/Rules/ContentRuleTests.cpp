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
struct ContentRuleTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRuleTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "Content",
        "specifications": [{
            "specType": "ContentInstancesOfSpecificClasses",
            "classes": {"schemaName": "TestSchema", "classNames": ["ClassA"]}
        }, {
            "specType": "ContentRelatedInstances",
            "relationshipPaths": [{
                "relationship": {"schemaName": "A", "className": "B"},
                "direction": "Forward"
            }]
        }, {
            "specType": "SelectedNodeInstances"
        }]
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    ContentRule rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_EQ(3, rule.GetSpecifications().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRuleTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "Content"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    ContentRule rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_EQ(0, rule.GetSpecifications().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRuleTests, WriteToJson)
    {
    ContentRule rule("cond", 123, true);
    rule.AddSpecification(*new SelectedNodeInstancesSpecification());
    rule.AddSpecification(*new ContentRelatedInstancesSpecification());
    rule.AddSpecification(*new ContentInstancesOfSpecificClassesSpecification());
    BeJsDocument json = rule.WriteJson();
    BeJsDocument expected(R"({
        "ruleType": "Content",
        "priority": 123,
        "onlyIfNotHandled": true,
        "condition": "cond",
        "specifications": [{
            "specType": "SelectedNodeInstances"
        }, {
            "specType": "ContentRelatedInstances"
        }, {
            "specType": "ContentInstancesOfSpecificClasses",
            "classes": []
        }]
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRuleTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "f15c1cae7882448b3fb0404682e17e61";

    // Make sure that introducing additional attributes with default values don't affect the hash
    ContentRule defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    ContentRule copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }
