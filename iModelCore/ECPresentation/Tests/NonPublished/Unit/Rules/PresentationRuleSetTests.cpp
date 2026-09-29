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
struct PresentationRuleSetTests : PresentationRulesTests
    {
    };

Utf8CP RuleSetJsonString = R"({
    "id": "Items",
    "schemaVersion": "1.2.3",
    "version": "4.5.6",
    "supportedSchemas": {"schemaNames": ["supportedSchema"]},
    "supplementationInfo": {
        "supplementationPurpose": "supplementationPurpose"
    },
    "rules": [{
        "ruleType": "RootNodes",
        "specifications": [{
            "specType": "AllInstanceNodes",
            "groupByClass": true,
            "groupByLabel": true
        }]
    }, {
        "ruleType": "RootNodes",
        "condition": "TestCondition",
        "priority": 12,
        "onlyIfNotHandled": true,
        "specifications": [{
            "specType": "AllInstanceNodes",
            "groupByClass": true,
            "groupByLabel": true
        }]
    }, {
        "ruleType": "ChildNodes",
        "specifications": [{
            "specType": "AllInstanceNodes",
            "groupByClass": true,
            "groupByLabel": true
        }]
    }, {
        "ruleType": "ChildNodes",
        "condition": "ParentNode.IsSearchNode",
        "priority": 58,
        "onlyIfNotHandled": true,
        "specifications": [{
            "specType": "CustomQueryInstanceNodes",
            "groupByClass": true,
            "groupByLabel": true
        }]
    }, {
        "ruleType": "Content",
        "specifications": [{
            "specType": "ContentInstancesOfSpecificClasses",
            "classes": {"schemaName": "dgn", "classNames": ["Model"]},
            "arePolymorphic": true,
            "showImages": true,
            "calculatedProperties": [{
                "label": "Label1",
                "priority": 1000,
                "value": "Value1",
                "type": "string"
            }, {
                "label": "Label2",
                "priority": 2000,
                "value": "Value2"
            }]
        }]
    }, {
        "ruleType": "Content",
        "condition": "ParentNode.IsClassNode",
        "priority": 97,
        "onlyIfNotHandled": true,
        "specifications": []
    }, {
        "ruleType": "ContentModifier"
    }, {
        "ruleType": "ContentModifier"
    }, {
        "ruleType": "ImageIdOverride",
        "imageIdExpression": "NewImageId1"
    }, {
        "ruleType": "ImageIdOverride",
        "condition": "ParentNode.IsClassGroupingNode",
        "imageIdExpression": "NewImageId2",
        "priority": 0
    }, {
        "ruleType": "CheckBox"
    }, {
        "ruleType": "Grouping",
        "class": {"schemaName": "schemaName", "className": "className"}
    }, {
        "ruleType": "InstanceLabelOverride",
        "class": {"schemaName": "schemaName", "className": "className"},
        "propertyNames": ["a"]
    }, {
        "ruleType": "LabelOverride"
    }, {
        "ruleType": "DisabledSorting"
    }, {
        "ruleType": "StyleOverride"
    }, {
        "ruleType": "ExtendedData",
        "items": {}
    }, {
        "ruleType": "NodeArtifacts",
        "items": {}
    }],
    "vars": [ {
        "label": "vars group 1",
        "vars": []
    }, {
        "label": "vars group 2",
        "vars": []
    }]
})";

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, LoadsFromJsonString)
    {
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::ReadFromJsonString(RuleSetJsonString);

    ASSERT_FALSE(ruleSet.IsNull());
    EXPECT_STREQ("Items", ruleSet->GetRuleSetId().c_str());
    EXPECT_STREQ("supportedSchema", ruleSet->GetSupportedSchemas().c_str());
    EXPECT_TRUE(ruleSet->GetIsSupplemental());
    EXPECT_STREQ("supplementationPurpose", ruleSet->GetSupplementationPurpose().c_str());
    EXPECT_EQ(Version(1, 2, 3), ruleSet->GetRulesetSchemaVersion());
    EXPECT_EQ(Version(4, 5, 6), ruleSet->GetRulesetVersion().Value());

    EXPECT_EQ(1, ruleSet->GetCheckBoxRules().size());
    EXPECT_EQ(1, ruleSet->GetGroupingRules().size());
    EXPECT_EQ(1, ruleSet->GetInstanceLabelOverrides().size());
    EXPECT_EQ(1, ruleSet->GetLabelOverrides().size());
    EXPECT_EQ(1, ruleSet->GetSortingRules().size());
    EXPECT_EQ(1, ruleSet->GetStyleOverrides().size());
    EXPECT_EQ(1, ruleSet->GetExtendedDataRules().size());
    EXPECT_EQ(1, ruleSet->GetNodeArtifactRules().size());
    EXPECT_EQ(2, ruleSet->GetContentModifierRules().size());
    EXPECT_EQ(2, ruleSet->GetUserSettings().size());

    // Check for root node rules
    EXPECT_EQ(2, ruleSet->GetRootNodesRules().size());
    EXPECT_STREQ("", ruleSet->GetRootNodesRules()[0]->GetCondition().c_str());
    EXPECT_EQ(1000, ruleSet->GetRootNodesRules()[0]->GetPriority());
    EXPECT_FALSE(ruleSet->GetRootNodesRules()[0]->GetOnlyIfNotHandled());
    EXPECT_STREQ("TestCondition", ruleSet->GetRootNodesRules()[1]->GetCondition().c_str());
    EXPECT_EQ(12, ruleSet->GetRootNodesRules()[1]->GetPriority());
    EXPECT_TRUE(ruleSet->GetRootNodesRules()[1]->GetOnlyIfNotHandled());

    // Check for child node rules
    EXPECT_EQ(2, ruleSet->GetChildNodesRules().size());
    EXPECT_STREQ("", ruleSet->GetChildNodesRules()[0]->GetCondition().c_str());
    EXPECT_EQ(1000, ruleSet->GetChildNodesRules()[0]->GetPriority());
    EXPECT_FALSE(ruleSet->GetChildNodesRules()[0]->GetOnlyIfNotHandled());
    EXPECT_STREQ("ParentNode.IsSearchNode", ruleSet->GetChildNodesRules()[1]->GetCondition().c_str());
    EXPECT_EQ(58, ruleSet->GetChildNodesRules()[1]->GetPriority());
    EXPECT_TRUE(ruleSet->GetChildNodesRules()[1]->GetOnlyIfNotHandled());

    // Check for content rules
    ASSERT_EQ(2, ruleSet->GetContentRules().size());
    EXPECT_STREQ("", ruleSet->GetContentRules()[0]->GetCondition().c_str());
    EXPECT_EQ(1000, ruleSet->GetContentRules()[0]->GetPriority());
    EXPECT_FALSE(ruleSet->GetContentRules()[0]->GetOnlyIfNotHandled());
    ASSERT_EQ(1, ruleSet->GetContentRules()[0]->GetSpecifications().size());
    ASSERT_EQ(2, ruleSet->GetContentRules()[0]->GetSpecifications()[0]->GetCalculatedProperties().size());
    CalculatedPropertiesSpecificationP specification1 = ruleSet->GetContentRules()[0]->GetSpecifications()[0]->GetCalculatedProperties()[0];
    EXPECT_EQ("Label1", specification1->GetLabel());
    EXPECT_EQ(1000, specification1->GetPriority());
    EXPECT_EQ("Value1", specification1->GetValue());
    EXPECT_EQ("string", specification1->GetType());
    CalculatedPropertiesSpecificationP specification2 = ruleSet->GetContentRules()[0]->GetSpecifications()[0]->GetCalculatedProperties()[1];
    EXPECT_EQ("Label2", specification2->GetLabel());
    EXPECT_EQ(2000, specification2->GetPriority());
    EXPECT_EQ("Value2", specification2->GetValue());
    EXPECT_EQ("", specification2->GetType());

    ASSERT_EQ(2, ruleSet->GetContentRules().size());
    EXPECT_STREQ("ParentNode.IsClassNode", ruleSet->GetContentRules()[1]->GetCondition().c_str());
    EXPECT_EQ(97, ruleSet->GetContentRules()[1]->GetPriority());
    EXPECT_TRUE(ruleSet->GetContentRules()[1]->GetOnlyIfNotHandled());

    // Check for ImageIdOverride rules
    ASSERT_EQ(2, ruleSet->GetImageIdOverrides().size());
    EXPECT_STREQ("", ruleSet->GetImageIdOverrides()[0]->GetCondition().c_str());
    EXPECT_STREQ("NewImageId1", ruleSet->GetImageIdOverrides()[0]->GetImageId().c_str());
    EXPECT_EQ(1000, ruleSet->GetImageIdOverrides()[0]->GetPriority());

    EXPECT_STREQ("ParentNode.IsClassGroupingNode", ruleSet->GetImageIdOverrides()[1]->GetCondition().c_str());
    EXPECT_STREQ("NewImageId2", ruleSet->GetImageIdOverrides()[1]->GetImageId().c_str());
    EXPECT_EQ(0, ruleSet->GetImageIdOverrides()[1]->GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, LoadsFromJsonValue)
    {
    BeJsDocument json(RuleSetJsonString);
    EXPECT_FALSE(json.isNull());

    PresentationRuleSetPtr ruleSet = PresentationRuleSet::ReadFromJsonValue(json);

    ASSERT_FALSE(ruleSet.IsNull());
    EXPECT_STREQ("Items", ruleSet->GetRuleSetId().c_str());
    EXPECT_STREQ("supportedSchema", ruleSet->GetSupportedSchemas().c_str());
    EXPECT_TRUE(ruleSet->GetIsSupplemental());
    EXPECT_STREQ("supplementationPurpose", ruleSet->GetSupplementationPurpose().c_str());
    EXPECT_EQ(Version(1, 2, 3), ruleSet->GetRulesetSchemaVersion());
    EXPECT_EQ(Version(4, 5, 6), ruleSet->GetRulesetVersion().Value());

    EXPECT_EQ(1, ruleSet->GetCheckBoxRules().size());
    EXPECT_EQ(1, ruleSet->GetGroupingRules().size());
    EXPECT_EQ(1, ruleSet->GetInstanceLabelOverrides().size());
    EXPECT_EQ(1, ruleSet->GetLabelOverrides().size());
    EXPECT_EQ(1, ruleSet->GetSortingRules().size());
    EXPECT_EQ(1, ruleSet->GetStyleOverrides().size());
    EXPECT_EQ(1, ruleSet->GetExtendedDataRules().size());
    EXPECT_EQ(1, ruleSet->GetNodeArtifactRules().size());
    EXPECT_EQ(2, ruleSet->GetContentModifierRules().size());
    EXPECT_EQ(2, ruleSet->GetUserSettings().size());

    // Check for root node rules
    EXPECT_EQ(2, ruleSet->GetRootNodesRules().size());
    EXPECT_STREQ("", ruleSet->GetRootNodesRules()[0]->GetCondition().c_str());
    EXPECT_EQ(1000, ruleSet->GetRootNodesRules()[0]->GetPriority());
    EXPECT_FALSE(ruleSet->GetRootNodesRules()[0]->GetOnlyIfNotHandled());
    EXPECT_STREQ("TestCondition", ruleSet->GetRootNodesRules()[1]->GetCondition().c_str());
    EXPECT_EQ(12, ruleSet->GetRootNodesRules()[1]->GetPriority());
    EXPECT_TRUE(ruleSet->GetRootNodesRules()[1]->GetOnlyIfNotHandled());

    // Check for child node rules
    EXPECT_EQ(2, ruleSet->GetChildNodesRules().size());
    EXPECT_STREQ("", ruleSet->GetChildNodesRules()[0]->GetCondition().c_str());
    EXPECT_EQ(1000, ruleSet->GetChildNodesRules()[0]->GetPriority());
    EXPECT_FALSE(ruleSet->GetChildNodesRules()[0]->GetOnlyIfNotHandled());
    EXPECT_STREQ("ParentNode.IsSearchNode", ruleSet->GetChildNodesRules()[1]->GetCondition().c_str());
    EXPECT_EQ(58, ruleSet->GetChildNodesRules()[1]->GetPriority());
    EXPECT_TRUE(ruleSet->GetChildNodesRules()[1]->GetOnlyIfNotHandled());

    // Check for content rules
    EXPECT_EQ(2, ruleSet->GetContentRules().size());
    EXPECT_STREQ("", ruleSet->GetContentRules()[0]->GetCondition().c_str());
    EXPECT_EQ(1000, ruleSet->GetContentRules()[0]->GetPriority());
    EXPECT_FALSE(ruleSet->GetContentRules()[0]->GetOnlyIfNotHandled());
    EXPECT_EQ(1, ruleSet->GetContentRules()[0]->GetSpecifications().size());
    EXPECT_EQ(2, ruleSet->GetContentRules()[0]->GetSpecifications()[0]->GetCalculatedProperties().size());
    CalculatedPropertiesSpecificationP specification1 = ruleSet->GetContentRules()[0]->GetSpecifications()[0]->GetCalculatedProperties()[0];
    EXPECT_EQ("Label1", specification1->GetLabel());
    EXPECT_EQ(1000, specification1->GetPriority());
    EXPECT_EQ("Value1", specification1->GetValue());
    EXPECT_EQ("string", specification1->GetType());
    CalculatedPropertiesSpecificationP specification2 = ruleSet->GetContentRules()[0]->GetSpecifications()[0]->GetCalculatedProperties()[1];
    EXPECT_EQ("Label2", specification2->GetLabel());
    EXPECT_EQ(2000, specification2->GetPriority());
    EXPECT_EQ("Value2", specification2->GetValue());
    EXPECT_EQ("", specification2->GetType());

    EXPECT_EQ(2, ruleSet->GetContentRules().size());
    EXPECT_STREQ("ParentNode.IsClassNode", ruleSet->GetContentRules()[1]->GetCondition().c_str());
    EXPECT_EQ(97, ruleSet->GetContentRules()[1]->GetPriority());
    EXPECT_TRUE(ruleSet->GetContentRules()[1]->GetOnlyIfNotHandled());

    // Check for ImageIdOverride rules
    EXPECT_EQ(2, ruleSet->GetImageIdOverrides().size());
    EXPECT_STREQ("", ruleSet->GetImageIdOverrides()[0]->GetCondition().c_str());
    EXPECT_STREQ("NewImageId1", ruleSet->GetImageIdOverrides()[0]->GetImageId().c_str());
    EXPECT_EQ(1000, ruleSet->GetImageIdOverrides()[0]->GetPriority());

    EXPECT_STREQ("ParentNode.IsClassGroupingNode", ruleSet->GetImageIdOverrides()[1]->GetCondition().c_str());
    EXPECT_STREQ("NewImageId2", ruleSet->GetImageIdOverrides()[1]->GetImageId().c_str());
    EXPECT_EQ(0, ruleSet->GetImageIdOverrides()[1]->GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, LoadsFromJsonStringWithDefaultValues)
    {
    Utf8CP jsonString = R"({"id":"Items"})";

    PresentationRuleSetPtr ruleSet = PresentationRuleSet::ReadFromJsonString(jsonString);
    EXPECT_FALSE(ruleSet.IsNull());
    EXPECT_STREQ("Items", ruleSet->GetRuleSetId().c_str());
    EXPECT_STREQ("", ruleSet->GetSupportedSchemas().c_str());
    EXPECT_STREQ("", ruleSet->GetSupplementationPurpose().c_str());
    EXPECT_FALSE(ruleSet->GetIsSupplemental());
    EXPECT_EQ(PresentationRuleSet::GetCurrentRulesetSchemaVersion(), ruleSet->GetRulesetSchemaVersion());
    EXPECT_TRUE(ruleSet->GetRulesetVersion().IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, LoadsFromJsonValueWithDefaultValues)
    {
    Utf8CP jsonString = R"({"id":"Items"})";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    PresentationRuleSetPtr ruleSet = PresentationRuleSet::ReadFromJsonValue(json);
    EXPECT_FALSE(ruleSet.IsNull());
    EXPECT_STREQ("Items", ruleSet->GetRuleSetId().c_str());
    EXPECT_STREQ("", ruleSet->GetSupportedSchemas().c_str());
    EXPECT_STREQ("", ruleSet->GetSupplementationPurpose().c_str());
    EXPECT_FALSE(ruleSet->GetIsSupplemental());
    EXPECT_EQ(PresentationRuleSet::GetCurrentRulesetSchemaVersion(), ruleSet->GetRulesetSchemaVersion());
    EXPECT_TRUE(ruleSet->GetRulesetVersion().IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, LoadsFromJsonStringFailsWhenJsonIsInvalid)
    {
    Utf8CP jsonString ="{";

    PresentationRuleSetPtr ruleSet = PresentationRuleSet::ReadFromJsonString(jsonString);
    EXPECT_TRUE(ruleSet.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, LoadsFromJsonFailsWhenRuleSetIdIsNotSpecified)
    {
    Utf8CP jsonString = "{}";

    PresentationRuleSetPtr ruleSet = PresentationRuleSet::ReadFromJsonString(jsonString);
    EXPECT_TRUE(ruleSet.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, ReadWriteToJsonFileRoundtrip)
    {
    BeFileName path;
    BeTest::GetHost().GetOutputRoot(path);
    path.AppendToPath(L"PresentationRuleSetTests.ReadWriteToJsonFileRoundtrip.Ruleset.json");
    path.BeDeleteFile();

    PresentationRuleSetPtr ruleset1 = PresentationRuleSet::CreateInstance("TestRuleSet");
    ASSERT_TRUE(ruleset1->WriteToJsonFile(path));
    ASSERT_TRUE(path.DoesPathExist());

    PresentationRuleSetPtr ruleset2 = PresentationRuleSet::ReadFromJsonFile(path);
    ASSERT_TRUE(ruleset2.IsValid());
    EXPECT_STREQ(ruleset1->GetHash().c_str(), ruleset2->GetHash().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "d41d8cd98f00b204e9800998ecf8427e";

    // Make sure that introducing additional attributes with default values don't affect the hash
    PresentationRuleSetPtr defaultSpec = PresentationRuleSet::CreateInstance("");
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec->GetHash().c_str());

    // Make sure that similarly created instance has the same hash
    PresentationRuleSetPtr copySpec = PresentationRuleSet::CreateInstance("");
    EXPECT_STREQ(DEFAULT_HASH, copySpec->GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, AddPresentationRule_SortsByPriority)
    {
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance("TestRuleSet");
    EXPECT_EQ(0, ruleSet->GetRootNodesRules().size());

    RootNodeRuleP rootNodeRule = new RootNodeRule("TestCondition1", 1, true, false);
    ruleSet->AddPresentationRule(*rootNodeRule);
    EXPECT_EQ(1, ruleSet->GetRootNodesRules().size());

    rootNodeRule = new RootNodeRule("TestCondition2", 1, true, false);
    ruleSet->AddPresentationRule(*rootNodeRule);
    EXPECT_EQ(2, ruleSet->GetRootNodesRules().size());
    EXPECT_STREQ("TestCondition1", ruleSet->GetRootNodesRules()[0]->GetCondition().c_str());
    EXPECT_STREQ("TestCondition2", ruleSet->GetRootNodesRules()[1]->GetCondition().c_str());

    rootNodeRule = new RootNodeRule("TestCondition3", 2, true, false);
    ruleSet->AddPresentationRule(*rootNodeRule);
    EXPECT_EQ(3, ruleSet->GetRootNodesRules().size());
    EXPECT_STREQ("TestCondition3", ruleSet->GetRootNodesRules()[0]->GetCondition().c_str());
    EXPECT_STREQ("TestCondition1", ruleSet->GetRootNodesRules()[1]->GetCondition().c_str());
    EXPECT_STREQ("TestCondition2", ruleSet->GetRootNodesRules()[2]->GetCondition().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, MergeWith_SortsByPriority)
    {
    PresentationRuleSetPtr dest = PresentationRuleSet::CreateInstance("test");
    dest->AddPresentationRule(*new ContentRule("", 0, false));
    EXPECT_EQ(1, dest->GetContentRules().size());

    PresentationRuleSetPtr src = PresentationRuleSet::CreateInstance("test");
    src->AddPresentationRule(*new ContentRule("", 1, false));
    EXPECT_EQ(1, src->GetContentRules().size());

    dest->MergeWith(*src);
    ASSERT_EQ(2, dest->GetContentRules().size());
    EXPECT_EQ(1, dest->GetContentRules()[0]->GetPriority());
    EXPECT_EQ(0, dest->GetContentRules()[1]->GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, RemovePresentationRule)
    {
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance("TestRuleSet");
    EXPECT_EQ(0, ruleSet->GetRootNodesRules().size());

    RootNodeRule rootNodeRule("TestCondition1", 1, true, false);
    ruleSet->AddPresentationRule(rootNodeRule);
    EXPECT_EQ(1, ruleSet->GetRootNodesRules().size());

    ruleSet->RemovePresentationRule(rootNodeRule);
    EXPECT_EQ(0, ruleSet->GetRootNodesRules().size());
    }
