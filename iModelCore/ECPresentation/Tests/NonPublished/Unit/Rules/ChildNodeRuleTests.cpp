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
struct SubConditionTests : PresentationRulesTests
    {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SubConditionTests, CopyConstructorCopiesSpecifications)
    {
    // Create condition1
    SubCondition condition1("condition");
    condition1.AddSubCondition(*new SubCondition());
    condition1.AddSpecification(*new AllInstanceNodesSpecification());

    // Validate condition1
    EXPECT_STREQ("condition", condition1.GetCondition().c_str());
    EXPECT_EQ(1, condition1.GetSpecifications().size());
    EXPECT_EQ(1, condition1.GetSubConditions().size());

    // Create condition2 via copy constructor
    SubCondition condition2(condition1);

    // Validate condition2
    EXPECT_STREQ("condition", condition2.GetCondition().c_str());
    EXPECT_EQ(1, condition2.GetSpecifications().size());
    EXPECT_EQ(1, condition2.GetSubConditions().size());
    EXPECT_NE(condition1.GetSpecifications()[0], condition2.GetSpecifications()[0]);
    EXPECT_NE(condition1.GetSubConditions()[0], condition2.GetSubConditions()[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SubConditionTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "condition": "condition",
        "requiredSchemas": [{"name": "TestSchema"}],
        "subConditions": [{
            "condition": "nestedSubCondition"
        }],
        "specifications": [{
            "specType": "AllInstanceNodes"
        }, {
            "specType": "AllRelatedInstanceNodes"
        }, {
            "specType": "CustomNode",
            "type": "test type",
            "label": "test label"
        }, {
            "specType": "InstanceNodesOfSpecificClasses",
            "classes": {"schemaName": "TestSchema", "classNames": ["TestClass"]}
        }, {
            "specType": "RelatedInstanceNodes",
            "relationshipPaths": [{
                "relationship": {"schemaName": "A", "className": "B"},
                "direction": "Forward"
            }]
        }, {
            "specType": "CustomQueryInstanceNodes"
        }]
    })";
    BeJsDocument json(jsonString);
    ASSERT_FALSE(json.isNull());

    SubCondition rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("condition", rule.GetCondition().c_str());
    EXPECT_EQ(1, rule.GetRequiredSchemaSpecifications().size());
    EXPECT_EQ(6, rule.GetSpecifications().size());
    EXPECT_EQ(1, rule.GetSubConditions().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SubConditionTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = "{}";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    SubCondition rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("", rule.GetCondition().c_str());
    EXPECT_EQ(0, rule.GetRequiredSchemaSpecifications().size());
    EXPECT_EQ(0, rule.GetSpecifications().size());
    EXPECT_EQ(0, rule.GetSubConditions().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SubConditionTests, WriteToJson)
    {
    SubCondition subCondition("cond");
    subCondition.AddRequiredSchemaSpecification(*new RequiredSchemaSpecification("TestSchema"));
    subCondition.AddSubCondition(*new SubCondition("cond 2"));
    subCondition.AddSpecification(*new AllInstanceNodesSpecification());
    BeJsDocument json = subCondition.WriteJson();
    BeJsDocument expected(R"({
        "condition": "cond",
        "requiredSchemas": [{"name": "TestSchema"}],
        "subConditions": [{
            "condition": "cond 2"
        }],
        "specifications": [{
            "specType": "AllInstanceNodes"
        }]
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SubConditionTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "15b88e611b65f91be9911e80d981fd1d";

    // Make sure that introducing additional attributes with default values don't affect the hash
    SubCondition defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    SubCondition copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // Test that each attribute affects the hash
    SubCondition specWithRequiredSchemas;
    specWithRequiredSchemas.AddRequiredSchemaSpecification(*new RequiredSchemaSpecification("Test"));
    EXPECT_STRNE(DEFAULT_HASH, specWithRequiredSchemas.GetHash().c_str());
    specWithRequiredSchemas.ClearRequiredSchemaSpecifications();
    EXPECT_STREQ(DEFAULT_HASH, specWithRequiredSchemas.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct ChildNodeRuleTests : PresentationRulesTests
    {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChildNodeRuleTests, CopyConstructorWorksCorrectly)
    {
    ChildNodeRule rule1("condition", 1000, true);
    rule1.AddSubCondition(*new SubCondition("subCondition"));
    rule1.AddSpecification(*new AllInstanceNodesSpecification());
    rule1.AddCustomizationRule(*new CheckBoxRule());

    ChildNodeRule rule2(rule1);

    // Validate subconditions
    EXPECT_EQ(rule1.GetSubConditions().size(), rule2.GetSubConditions().size());
    EXPECT_NE(rule1.GetSubConditions()[0], rule2.GetSubConditions()[0]);

    // Validate specifications
    EXPECT_EQ(rule1.GetSpecifications().size(), rule2.GetSpecifications().size());
    EXPECT_NE(rule1.GetSpecifications()[0], rule2.GetSpecifications()[0]);

    // Validate CustomizationRules
    EXPECT_EQ(rule1.GetCustomizationRules().size(), rule2.GetCustomizationRules().size());
    EXPECT_NE(rule1.GetCustomizationRules()[0], rule2.GetCustomizationRules()[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChildNodeRuleTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "ChildNodes",
        "requiredSchemas": [{"name": "TestSchema"}],
        "stopFurtherProcessing": true,
        "subConditions": [{}, {}],
        "specifications": [{
            "specType": "AllInstanceNodes"
        }, {
            "specType": "AllRelatedInstanceNodes"
        }, {
            "specType": "CustomNode",
            "type": "test type",
            "label": "test label"
        }, {
            "specType": "InstanceNodesOfSpecificClasses",
            "classes": {"schemaName": "TestSchema", "classNames": ["TestClass"]}
        }, {
            "specType": "RelatedInstanceNodes",
            "relationshipPaths": [{
                "relationship": {"schemaName": "A", "className": "B"},
                "direction": "Forward"
            }]
        }, {
            "specType": "CustomQueryInstanceNodes"
        }],
        "customizationRules": [{
            "ruleType": "Grouping",
            "class": {"schemaName": "schema", "className": "ClassName"}
        }, {
            "ruleType": "DisabledSorting"
        }, {
            "ruleType": "CheckBox"
        }, {
            "ruleType": "StyleOverride"
        }, {
            "ruleType": "LabelOverride"
        }, {
            "ruleType": "InstanceLabelOverride",
            "class": {"schemaName": "schema", "className": "ClassName"},
            "propertyNames": ["property"]
        }, {
            "ruleType": "ImageIdOverride"
        }, {
            "ruleType": "ExtendedData",
            "items": {}
        }, {
            "ruleType": "NodeArtifacts",
            "items": {}
        }]
    })";
    BeJsDocument json(jsonString);
    ASSERT_FALSE(json.isNull());

    ChildNodeRule rule;
    ASSERT_TRUE(rule.ReadJson(json));
    EXPECT_TRUE(rule.GetStopFurtherProcessing());
    EXPECT_EQ(1, rule.GetRequiredSchemaSpecifications().size());
    EXPECT_EQ(6, rule.GetSpecifications().size());
    EXPECT_EQ(2, rule.GetSubConditions().size());
    EXPECT_EQ(9, rule.GetCustomizationRules().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChildNodeRuleTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "ChildNodes"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    ChildNodeRule rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_FALSE(rule.GetStopFurtherProcessing());
    EXPECT_EQ(0, rule.GetRequiredSchemaSpecifications().size());
    EXPECT_EQ(0, rule.GetSpecifications().size());
    EXPECT_EQ(0, rule.GetSubConditions().size());
    EXPECT_EQ(0, rule.GetCustomizationRules().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChildNodeRuleTests, WriteToJson)
    {
    ChildNodeRule rule("cond", 123, true);
    rule.AddRequiredSchemaSpecification(*new RequiredSchemaSpecification("TestSchema"));
    BeJsDocument json = rule.WriteJson();
    BeJsDocument expected(R"({
        "ruleType": "ChildNodes",
        "onlyIfNotHandled": true,
        "condition": "cond",
        "requiredSchemas": [{"name": "TestSchema"}],
        "priority": 123
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ChildNodeRuleTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "5e53bdeb9f1ae282ccfd24c28031406e";

    // Make sure that introducing additional attributes with default values don't affect the hash
    ChildNodeRule defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    ChildNodeRule copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // Test that each attribute affects the hash
    ChildNodeRule specWithRequiredSchemas;
    specWithRequiredSchemas.AddRequiredSchemaSpecification(*new RequiredSchemaSpecification("Test"));
    EXPECT_STRNE(DEFAULT_HASH, specWithRequiredSchemas.GetHash().c_str());
    specWithRequiredSchemas.ClearRequiredSchemaSpecifications();
    EXPECT_STREQ(DEFAULT_HASH, specWithRequiredSchemas.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct RootNodeRuleTests : PresentationRulesTests
    {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RootNodeRuleTests, WriteToJson)
    {
    RootNodeRule rule("cond", 123, true, true);
    BeJsDocument json = rule.WriteJson();
    BeJsDocument expected(R"({
        "ruleType": "RootNodes",
        "onlyIfNotHandled": true,
        "condition": "cond",
        "priority": 123,
        "autoExpand": true
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RootNodeRuleTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "021fbb5c7395d6cadaaa3f4daebb3248";

    // Make sure that introducing additional attributes with default values don't affect the hash
    RootNodeRule defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    RootNodeRule copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }
