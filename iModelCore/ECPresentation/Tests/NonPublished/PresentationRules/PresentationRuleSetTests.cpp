/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct PresentationRuleSetTests : PresentationRulesTests
    {
    };
    
Utf8CP RuleSetJsonString = R"({
    "id": "Items",
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
                "value": "Value1"
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
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, LoadsFromJsonString)
    {
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::ReadFromJsonString(RuleSetJsonString);

    ASSERT_FALSE(ruleSet.IsNull());
    EXPECT_STREQ("Items", ruleSet->GetRuleSetId().c_str());
    EXPECT_STREQ("supportedSchema", ruleSet->GetSupportedSchemas().c_str());
    EXPECT_TRUE(ruleSet->GetIsSupplemental());
    EXPECT_STREQ("supplementationPurpose", ruleSet->GetSupplementationPurpose().c_str());
    EXPECT_EQ(1, ruleSet->GetVersionMajor());
    EXPECT_EQ(0, ruleSet->GetVersionMinor());

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
    CalculatedPropertiesSpecificationP specification2 = ruleSet->GetContentRules()[0]->GetSpecifications()[0]->GetCalculatedProperties()[1];
    EXPECT_EQ("Label2", specification2->GetLabel());
    EXPECT_EQ(2000, specification2->GetPriority());
    EXPECT_EQ("Value2", specification2->GetValue());

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
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, LoadsFromJsonValue)
    {
    Json::Value json = Json::Reader::DoParse(RuleSetJsonString);
    EXPECT_FALSE(json.isNull());

    PresentationRuleSetPtr ruleSet = PresentationRuleSet::ReadFromJsonValue(json);

    ASSERT_FALSE(ruleSet.IsNull());
    EXPECT_STREQ("Items", ruleSet->GetRuleSetId().c_str());
    EXPECT_STREQ("supportedSchema", ruleSet->GetSupportedSchemas().c_str());
    EXPECT_TRUE(ruleSet->GetIsSupplemental());
    EXPECT_STREQ("supplementationPurpose", ruleSet->GetSupplementationPurpose().c_str());
    EXPECT_EQ(1, ruleSet->GetVersionMajor());
    EXPECT_EQ(0, ruleSet->GetVersionMinor());

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
    CalculatedPropertiesSpecificationP specification2 = ruleSet->GetContentRules()[0]->GetSpecifications()[0]->GetCalculatedProperties()[1];
    EXPECT_EQ("Label2", specification2->GetLabel());
    EXPECT_EQ(2000, specification2->GetPriority());
    EXPECT_EQ("Value2", specification2->GetValue());

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
* @betest                                    Aidas.Kilinskas                   04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, LoadsFromJsonStringWithDefaultValues)
    {
    Utf8CP jsonString = R"({"id":"Items"})";

    PresentationRuleSetPtr ruleSet = PresentationRuleSet::ReadFromJsonString(jsonString);
    EXPECT_FALSE(ruleSet.IsNull());
    EXPECT_STREQ("Items", ruleSet->GetRuleSetId().c_str());
    EXPECT_STREQ("", ruleSet->GetSupportedSchemas().c_str());
    EXPECT_STREQ("", ruleSet->GetSupplementationPurpose().c_str());
    EXPECT_STREQ("", ruleSet->GetPreferredImage().c_str());
    EXPECT_STREQ("", ruleSet->GetSearchClasses().c_str());
    EXPECT_STREQ("", ruleSet->GetExtendedData().c_str());
    EXPECT_FALSE(ruleSet->GetIsSupplemental());
    EXPECT_TRUE(ruleSet->GetIsSearchEnabled());
    EXPECT_EQ(1, ruleSet->GetVersionMajor());
    EXPECT_EQ(0, ruleSet->GetVersionMinor());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                    Aidas.Kilinskas                   04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, LoadsFromJsonValueWithDefaultValues)
    {
    Utf8CP jsonString = R"({"id":"Items"})";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    PresentationRuleSetPtr ruleSet = PresentationRuleSet::ReadFromJsonValue(json);
    EXPECT_FALSE(ruleSet.IsNull());
    EXPECT_STREQ("Items", ruleSet->GetRuleSetId().c_str());
    EXPECT_STREQ("", ruleSet->GetSupportedSchemas().c_str());
    EXPECT_STREQ("", ruleSet->GetSupplementationPurpose().c_str());
    EXPECT_STREQ("", ruleSet->GetPreferredImage().c_str());
    EXPECT_STREQ("", ruleSet->GetSearchClasses().c_str());
    EXPECT_STREQ("", ruleSet->GetExtendedData().c_str());
    EXPECT_FALSE(ruleSet->GetIsSupplemental());
    EXPECT_TRUE(ruleSet->GetIsSearchEnabled());
    EXPECT_EQ(1, ruleSet->GetVersionMajor());
    EXPECT_EQ(0, ruleSet->GetVersionMinor());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                    Aidas.Kilinskas                   04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, LoadsFromJsonStringFailsWhenJsonIsInvalid)
    {
    Utf8CP jsonString ="{";

    PresentationRuleSetPtr ruleSet = PresentationRuleSet::ReadFromJsonString(jsonString);
    EXPECT_TRUE(ruleSet.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, LoadsFromJsonFailsWhenRuleSetIdIsNotSpecified)
    {
    Utf8CP jsonString = "{}";

    PresentationRuleSetPtr ruleSet = PresentationRuleSet::ReadFromJsonString(jsonString);
    EXPECT_TRUE(ruleSet.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, ReadWriteToJsonFileRoundtrip)
    {
    BeFileName path;
    BeTest::GetHost().GetOutputRoot(path);
    path.AppendToPath(L"PresentationRuleSetTests.ReadWriteToJsonFileRoundtrip.Ruleset.json");
    path.BeDeleteFile();

    PresentationRuleSetPtr ruleset1 = PresentationRuleSet::CreateInstance("TestRuleSet", 1, 0, true, "Supplemental", "schema1,schema2", "", true);
    ASSERT_TRUE(ruleset1->WriteToJsonFile(path));
    ASSERT_TRUE(path.DoesPathExist());

    PresentationRuleSetPtr ruleset2 = PresentationRuleSet::ReadFromJsonFile(path);
    ASSERT_TRUE(ruleset2.IsValid());
    EXPECT_STREQ(ruleset1->GetHash().c_str(), ruleset2->GetHash().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, LoadsFromXml)
    {
    Utf8CP ruleSetXmlString = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "  <PresentationRuleSet"
        "    RuleSetId=\"Items\""
        "    VersionMajor=\"5\""
        "    VersionMinor=\"3\""
        "    SupportedSchemas=\"E:DgnFileSchema,DgnModelSchema,BaseElementSchema\""
        "    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
        "    xsi:noNamespaceSchemaLocation=\"PresentationRuleSetSchema.xsd\">"
        "    <RootNodeRule>"
        "      <AllInstances GroupByClass='true' GroupByLabel='true' />"
        "    </RootNodeRule>"
        "    <RootNodeRule Condition='TestCondition' Priority='12' OnlyIfNotHandled='true'>"
        "      <AllInstances GroupByClass='true' GroupByLabel='true' />"
        "    </RootNodeRule>"
        "    <ChildNodeRule>"
        "      <AllRelatedInstances GroupByClass='true' GroupByLabel='true' />"
        "    </ChildNodeRule>"
        "    <ChildNodeRule Condition='ParentNode.IsSearchNode' Priority='58' OnlyIfNotHandled='true'>"
        "      <SearchResultInstances GroupByClass='true' GroupByLabel='true' />"
        "    </ChildNodeRule>"
        "    <ContentRule>"
        "       <ContentInstancesOfSpecificClasses ClassNames=\"dgn:Model\" ArePolymorphic=\"true\" ShowImages=\"true\">"
        "           <CalculatedProperties>"
        "               <Property Label = \"Label1\" Priority = \"1000\">Value1</Property>"
        "               <Property Label = \"Label2\" Priority = \"2000\">Value2</Property>"
        "           </CalculatedProperties>" 
        "       </ContentInstancesOfSpecificClasses>"
        "    </ContentRule>"
        "    <ContentRule Condition='ParentNode.IsClassNode' Priority='97' OnlyIfNotHandled='true'>"
        "    </ContentRule>"
        "    <ImageIdOverride ImageId='NewImageId1'/>"
        "    <ImageIdOverride Condition='ParentNode.IsClassGroupingNode' ImageId='NewImageId2' Priority='0'/>"
        "  </PresentationRuleSet>";

    PresentationRuleSetPtr ruleSet = PresentationRuleSet::ReadFromXmlString(ruleSetXmlString);

    EXPECT_FALSE(ruleSet.IsNull());
    EXPECT_STREQ("Items", ruleSet->GetRuleSetId().c_str());
    EXPECT_STREQ("E:DgnFileSchema,DgnModelSchema,BaseElementSchema", ruleSet->GetSupportedSchemas().c_str());
    EXPECT_FALSE(ruleSet->GetIsSupplemental());
    EXPECT_EQ(5, ruleSet->GetVersionMajor());
    EXPECT_EQ(3, ruleSet->GetVersionMinor());

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
    CalculatedPropertiesSpecificationP specification2 = ruleSet->GetContentRules()[0]->GetSpecifications()[0]->GetCalculatedProperties()[1];
    EXPECT_EQ("Label2", specification2->GetLabel());
    EXPECT_EQ(2000, specification2->GetPriority());
    EXPECT_EQ("Value2", specification2->GetValue());

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
* @bsimethod                                    Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, LoadsFromXmlWithDefaultValues)
    {
    Utf8CP ruleSetXmlString = ""
        R"(<?xml version="1.0" encoding="utf-8"?>
               <PresentationRuleSet RuleSetId="Items" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance/" xsi:noNamespaceSchemaLocation="PresentationRuleSetSchema.xsd"/>
        )";

    PresentationRuleSetPtr ruleSet = PresentationRuleSet::ReadFromXmlString(ruleSetXmlString);
    EXPECT_FALSE(ruleSet.IsNull());
    EXPECT_STREQ("Items", ruleSet->GetRuleSetId().c_str());
    EXPECT_STREQ("", ruleSet->GetSupportedSchemas().c_str());
    EXPECT_STREQ("", ruleSet->GetSupplementationPurpose().c_str());
    EXPECT_STREQ("", ruleSet->GetPreferredImage().c_str());
    EXPECT_STREQ("", ruleSet->GetSearchClasses().c_str());
    EXPECT_STREQ("", ruleSet->GetExtendedData().c_str());
    EXPECT_FALSE(ruleSet->GetIsSupplemental());
    EXPECT_TRUE(ruleSet->GetIsSearchEnabled());
    EXPECT_EQ(1, ruleSet->GetVersionMajor());
    EXPECT_EQ(0, ruleSet->GetVersionMinor());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, LoadsFromXmlFailsWhenXmlIsInvalid)
    {
    Utf8CP ruleSetXmlString = R"(<?xml version="1.0" encoding="utf-8"?>
        <RuleSetId="Items" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance/" xsi:noNamespaceSchemaLocation="PresentationRuleSetSchema.xsd">
        )";

    PresentationRuleSetPtr ruleSet = PresentationRuleSet::ReadFromXmlString(ruleSetXmlString);
    EXPECT_TRUE(ruleSet.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, LoadsFromXmlFailsWhenRuleSetIdIsNotSpecified)
    {
    Utf8CP ruleSetXmlString = R"(<?xml version="1.0" encoding="utf-8"?>
        <PresentationRuleSet/>
        )";

    PresentationRuleSetPtr ruleSet = PresentationRuleSet::ReadFromXmlString(ruleSetXmlString);
    EXPECT_TRUE(ruleSet.IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eligijus.Mauragas               06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, WritesToXml)
    {
    //Create PresentationRuleSet and rules usin non-default values, to make sure it saves XML correctly.
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance("TestRuleSet", 2, 1, true, "Supplemental", "DummySchemaName", "MyImage", true);
    ASSERT_TRUE(nullptr != ruleSet.get());

    RootNodeRuleP rootNodeRule = new RootNodeRule("TestCondition1", 1, true, TargetTree_Both, false);
    ASSERT_TRUE(nullptr != rootNodeRule);
    ruleSet->AddPresentationRule(*rootNodeRule);

    ChildNodeRuleP childNodeRule = new ChildNodeRule("TestCondition2", 2, true, TargetTree_SelectionTree);
    ASSERT_TRUE(nullptr != childNodeRule);
    ruleSet->AddPresentationRule(*childNodeRule);

    ContentRuleP contentRule = new ContentRule("TestCondition3", 3, true);
    ASSERT_TRUE(nullptr != contentRule);
    ruleSet->AddPresentationRule(*contentRule);

    ruleSet->AddPresentationRule(*new ImageIdOverride("TestCondition4", 4, "ImageIdOverrideTestValue"));
    ruleSet->AddPresentationRule(*new LabelOverride("TestCondition5", 5, "LabelOverrideLabelValue", "LabelOverrideDescriptionValue"));
    ruleSet->AddPresentationRule(*new StyleOverride("TestCondition6", 6, "Blue", "Red", "Bold"));
    ruleSet->AddPresentationRule(*new LocalizationResourceKeyDefinition(7, "UniqueId", "LocalizedStringAccessKey", "ThisIsTheValueIfItFails"));

    GroupingRuleP groupingRule = new GroupingRule("TestCondition7", 8, true, "DummySchemaName", "DummyClassName", "ContextMenuCondition", "ContextMenuLabel", "SettingsId");
    ClassGroupP classGroup = new ClassGroup("ContextMenuLabel", true, "SchemaName", "BaseClassName");
    groupingRule->AddGroup(*classGroup);
    PropertyGroupP propertyGroup = new PropertyGroup("ImageId", "ContextMenuLabel", true, "PropertyName");
    propertyGroup->AddRange(*new PropertyRangeGroupSpecification("Label", "ImageId", "FromValue", "ToValue"));
    groupingRule->AddGroup(*propertyGroup);
    ruleSet->AddPresentationRule(*groupingRule);

    CheckBoxRuleP checkBoxRule = new CheckBoxRule("checkBoxCondition", 9, true, "checkBoxProperty", false, false, "");
    ASSERT_TRUE(nullptr != checkBoxRule);
    
    SortingRuleP sortingRule = new SortingRule("SortingCondition", 11, "DummySchemaName", "DummyClassName", "DummyPropertyName", false, true, false);
    ASSERT_TRUE(nullptr != sortingRule);

    UserSettingsGroupP userSettingsGroup = new UserSettingsGroup("UserCategoryLabel");
    ASSERT_TRUE(nullptr != userSettingsGroup);

    InstanceLabelOverrideP instanceLabelOverride = new InstanceLabelOverride(100, true, "TestClassName", "TestProperties");
    ASSERT_TRUE(nullptr != sortingRule);

    ruleSet->AddPresentationRule(*checkBoxRule);
    ruleSet->AddPresentationRule(*sortingRule);
    ruleSet->AddPresentationRule(*userSettingsGroup);
    ruleSet->AddPresentationRule(*instanceLabelOverride);

    Utf8String serializedRuleSet = ruleSet->WriteToXmlString();

    static Utf8CP expected = ""
        R"(<?xml version="1.0" encoding="UTF-8"?>)""\n"
        R"(<PresentationRuleSet )"
         R"(RuleSetId="TestRuleSet" )"
         R"(SupportedSchemas="DummySchemaName" )"
         R"(IsSupplemental="true" )"
         R"(SupplementationPurpose="Supplemental" )"
         R"(VersionMajor="2" VersionMinor="1" )"
         R"(PreferredImage="MyImage" )"
         R"(IsSearchEnabled="true" )"
         R"(SearchClasses="" )"
         R"(ExtendedData="">)"
            R"(<RootNodeRule Priority="1" Condition="TestCondition1" OnlyIfNotHandled="true" TargetTree="Both" StopFurtherProcessing="false" AutoExpand="false"/>)"
            R"(<ChildNodeRule Priority="2" Condition="TestCondition2" OnlyIfNotHandled="true" TargetTree="SelectionTree" StopFurtherProcessing="false"/>)"
            R"(<ContentRule Priority="3" CustomControl="" Condition="TestCondition3" OnlyIfNotHandled="true"/>)"
            R"(<ImageIdOverride Priority="4" OnlyIfNotHandled="false" Condition="TestCondition4" ImageId="ImageIdOverrideTestValue"/>)"
            R"(<LabelOverride Priority="5" OnlyIfNotHandled="false" Condition="TestCondition5" Label="LabelOverrideLabelValue" Description="LabelOverrideDescriptionValue"/>)"
            R"(<StyleOverride Priority="6" OnlyIfNotHandled="false" Condition="TestCondition6" ForeColor="Blue" BackColor="Red" FontStyle="Bold"/>)"
            R"(<GroupingRule Priority="8" OnlyIfNotHandled="true" Condition="TestCondition7" SchemaName="DummySchemaName" ClassName="DummyClassName" )"
             R"(ContextMenuCondition="ContextMenuCondition" ContextMenuLabel="ContextMenuLabel" SettingsId="SettingsId">)"
                R"(<ClassGroup ContextMenuLabel="ContextMenuLabel" DefaultGroupLabel="" CreateGroupForSingleItem="true" SchemaName="SchemaName" BaseClassName="BaseClassName"/>)"
                R"(<PropertyGroup ContextMenuLabel="ImageId" DefaultGroupLabel="" ImageId="ContextMenuLabel" CreateGroupForSingleItem="true" )"
                 R"(CreateGroupForUnspecifiedValues="true" PropertyName="PropertyName" GroupingValue="DisplayLabel" SortingValue="DisplayLabel">)"
                    R"(<Range FromValue="FromValue" ToValue="ToValue" Label="Label" ImageId="ImageId"/>)"
                R"(</PropertyGroup>)"
            R"(</GroupingRule>)"
            R"(<LocalizationResourceKeyDefinition Priority="7" Id="UniqueId" Key="LocalizedStringAccessKey" DefaultValue="ThisIsTheValueIfItFails"/>)"
            R"(<UserSettings Priority="1000" CategoryLabel="UserCategoryLabel"/>)"
            R"(<CheckBoxRule Priority="9" OnlyIfNotHandled="true" Condition="checkBoxCondition" PropertyName="checkBoxProperty" UseInversedPropertyValue="false" DefaultValue="false" IsEnabled=""/>)"
            R"(<SortingRule Priority="11" SchemaName="DummySchemaName" ClassName="DummyClassName" PropertyName="DummyPropertyName" SortAscending="false" DoNotSort="true" )"
             R"(IsPolymorphic="false" OnlyIfNotHandled="false" Condition="SortingCondition"/>)"
            R"(<InstanceLabelOverride Priority="100" OnlyIfNotHandled="true" ClassName="TestClassName" PropertyNames="TestProperties"/>)"
        R"(</PresentationRuleSet>)";
    EXPECT_STREQ(expected, serializedRuleSet.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, ComputesCorrectHashes)
    {
    PresentationRuleSetPtr ruleset1 = PresentationRuleSet::CreateInstance("rulesetId", 1, 0, true, "", "", "", true);
    ruleset1->AddPresentationRule(*new RootNodeRule());
    ruleset1->AddPresentationRule(*new ChildNodeRule());
    ruleset1->AddPresentationRule(*new ContentRule());
    ruleset1->AddPresentationRule(*new ImageIdOverride());
    ruleset1->AddPresentationRule(*new LabelOverride());
    ruleset1->AddPresentationRule(*new StyleOverride());
    ruleset1->AddPresentationRule(*new GroupingRule());
    ruleset1->AddPresentationRule(*new LocalizationResourceKeyDefinition());
    ruleset1->AddPresentationRule(*new CheckBoxRule());
    ruleset1->AddPresentationRule(*new SortingRule());
    ruleset1->AddPresentationRule(*new ExtendedDataRule());
    ruleset1->AddPresentationRule(*new NodeArtifactsRule());
    ruleset1->AddPresentationRule(*new UserSettingsGroup());
    ruleset1->AddPresentationRule(*new ContentModifier());
    ruleset1->AddPresentationRule(*new InstanceLabelOverride());
    PresentationRuleSetPtr ruleset2 = PresentationRuleSet::CreateInstance("rulesetId", 1, 0, true, "", "", "", true);
    ruleset2->AddPresentationRule(*new RootNodeRule());
    ruleset2->AddPresentationRule(*new ChildNodeRule());
    ruleset2->AddPresentationRule(*new ContentRule());
    ruleset2->AddPresentationRule(*new ImageIdOverride());
    ruleset2->AddPresentationRule(*new LabelOverride());
    ruleset2->AddPresentationRule(*new ExtendedDataRule());
    ruleset2->AddPresentationRule(*new NodeArtifactsRule());
    ruleset2->AddPresentationRule(*new GroupingRule());
    ruleset2->AddPresentationRule(*new LocalizationResourceKeyDefinition());
    ruleset2->AddPresentationRule(*new CheckBoxRule());
    ruleset2->AddPresentationRule(*new SortingRule());
    ruleset2->AddPresentationRule(*new StyleOverride());
    ruleset2->AddPresentationRule(*new UserSettingsGroup());
    ruleset2->AddPresentationRule(*new ContentModifier());
    ruleset2->AddPresentationRule(*new InstanceLabelOverride());
    PresentationRuleSetPtr ruleset3 = PresentationRuleSet::CreateInstance("rulesetId", 1, 0, true, "", "", "", true);

    // Hashes are same for rulesets with same rules
    EXPECT_STREQ(ruleset1->GetHash().c_str(), ruleset2->GetHash().c_str());
    // Hashes differs for rulesets
    EXPECT_STRNE(ruleset1->GetHash().c_str(), ruleset3->GetHash().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, AddPresentationRule_SortsByPriority)
    {
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance("TestRuleSet", 1, 0, false, "", "", "", true);
    EXPECT_EQ(0, ruleSet->GetRootNodesRules().size());

    RootNodeRuleP rootNodeRule = new RootNodeRule("TestCondition1", 1, true, TargetTree_Both, false);
    ruleSet->AddPresentationRule(*rootNodeRule);
    EXPECT_EQ(1, ruleSet->GetRootNodesRules().size());

    rootNodeRule = new RootNodeRule("TestCondition2", 1, true, TargetTree_Both, false);
    ruleSet->AddPresentationRule(*rootNodeRule);
    EXPECT_EQ(2, ruleSet->GetRootNodesRules().size());
    EXPECT_STREQ("TestCondition1", ruleSet->GetRootNodesRules()[0]->GetCondition().c_str());
    EXPECT_STREQ("TestCondition2", ruleSet->GetRootNodesRules()[1]->GetCondition().c_str());

    rootNodeRule = new RootNodeRule("TestCondition3", 2, true, TargetTree_Both, false);
    ruleSet->AddPresentationRule(*rootNodeRule);
    EXPECT_EQ(3, ruleSet->GetRootNodesRules().size());
    EXPECT_STREQ("TestCondition3", ruleSet->GetRootNodesRules()[0]->GetCondition().c_str());
    EXPECT_STREQ("TestCondition1", ruleSet->GetRootNodesRules()[1]->GetCondition().c_str());
    EXPECT_STREQ("TestCondition2", ruleSet->GetRootNodesRules()[2]->GetCondition().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, RemovePresentationRule)
    {
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance("TestRuleSet", 1, 0, false, "", "", "", true);
    EXPECT_EQ(0, ruleSet->GetRootNodesRules().size());

    RootNodeRule rootNodeRule("TestCondition1", 1, true, TargetTree_Both, false);
    ruleSet->AddPresentationRule(rootNodeRule);
    EXPECT_EQ(1, ruleSet->GetRootNodesRules().size());

    ruleSet->RemovePresentationRule(rootNodeRule);
    EXPECT_EQ(0, ruleSet->GetRootNodesRules().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, TestCustomizationRuleLoadingFromXml)
    {
    Utf8CP ruleSetXmlString = "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "  <PresentationRuleSet"
        "    RuleSetId=\"Items\""
        "    VersionMajor=\"5\""
        "    VersionMinor=\"3\""
        "    xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\""
        "    xsi:noNamespaceSchemaLocation=\"PresentationRuleSetSchema.xsd\">"
        "    <InstanceLabelOverride ClassName='testClass' PropertyNames='testProperties'/>"
        "    <RootNodeRule>"
        "      <LabelOverride Label = 'newLabel1' />"
        "    </RootNodeRule>"
        "    <RootNodeRule>"
        "        <AllInstances>"
        "            <ChildNodeRule>"
        "                <StyleOverride ForeColor = 'blue' />"
        "            </ChildNodeRule>"
        "        </AllInstances>"
        "    </RootNodeRule>"
        "    <ImageIdOverride ImageId='NewImageId100'/>"
        "  </PresentationRuleSet>";

    PresentationRuleSetPtr ruleSet = PresentationRuleSet::ReadFromXmlString(ruleSetXmlString);
    EXPECT_FALSE(ruleSet.IsNull());

    ASSERT_EQ(2, ruleSet->GetRootNodesRules().size());

    ASSERT_EQ(1, ruleSet->GetRootNodesRules()[0]->GetCustomizationRules().size());
    LabelOverride* labelOverride = dynamic_cast<LabelOverride*>(ruleSet->GetRootNodesRules()[0]->GetCustomizationRules()[0]);
    EXPECT_STREQ("", labelOverride->GetCondition().c_str());
    EXPECT_EQ(1000, labelOverride->GetPriority());
    EXPECT_STREQ("newLabel1", labelOverride->GetLabel().c_str());
    EXPECT_STREQ("", labelOverride->GetDescription().c_str());

    ASSERT_EQ(1, ruleSet->GetRootNodesRules()[1]->GetSpecifications().size());
    ASSERT_EQ(1, ruleSet->GetRootNodesRules()[1]->GetSpecifications()[0]->GetNestedRules().size());
    ASSERT_EQ(1, ruleSet->GetRootNodesRules()[1]->GetSpecifications()[0]->GetNestedRules()[0]->GetCustomizationRules().size());
    StyleOverride* styleOverride = dynamic_cast<StyleOverride*>(ruleSet->GetRootNodesRules()[1]->GetSpecifications()[0]->GetNestedRules()[0]->GetCustomizationRules()[0]);
    EXPECT_STREQ("", styleOverride->GetCondition().c_str());
    EXPECT_EQ(1000, styleOverride->GetPriority());
    EXPECT_STREQ("blue", styleOverride->GetForeColor().c_str());
    EXPECT_STREQ("", styleOverride->GetBackColor().c_str());
    EXPECT_STREQ("", styleOverride->GetFontStyle().c_str());

    ASSERT_EQ(1, ruleSet->GetImageIdOverrides().size());
    ImageIdOverride* imageIdOverride = dynamic_cast<ImageIdOverride*>(ruleSet->GetImageIdOverrides()[0]);
    EXPECT_STREQ("", imageIdOverride->GetCondition().c_str());
    EXPECT_STREQ("NewImageId100", imageIdOverride->GetImageId().c_str());
    EXPECT_EQ(1000, imageIdOverride->GetPriority());

    ASSERT_EQ(1, ruleSet->GetInstanceLabelOverrides().size());
    InstanceLabelOverride* instanceLabelOverride = dynamic_cast<InstanceLabelOverride*>(ruleSet->GetInstanceLabelOverrides()[0]);
    EXPECT_STREQ("testClass", instanceLabelOverride->GetClassName().c_str());
    ASSERT_EQ(1, instanceLabelOverride->GetValueSpeficications().size());
    EXPECT_EQ(1000, instanceLabelOverride->GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                    Aidas.Vaiksnoras               03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PresentationRuleSetTests, TestNestedCustomizationRulesWriteToXml)
    {
    PresentationRuleSetPtr ruleSet = PresentationRuleSet::CreateInstance("TestRuleSet", 2, 1, true, "", "", "", true);
    ASSERT_TRUE(nullptr != ruleSet.get());

    RootNodeRuleP rootNodeRule = new RootNodeRule("TestCondition1", 1, true, TargetTree_Both, false);
    ASSERT_TRUE(nullptr != rootNodeRule);
    ruleSet->AddPresentationRule(*rootNodeRule);
    rootNodeRule->AddCustomizationRule(*new ImageIdOverride("TestCondition3", 4, "ImageIdOverrideTestValue"));

    ChildNodeRuleP childNodeRule = new ChildNodeRule("TestCondition2", 2, true, TargetTree_Both);
    ruleSet->AddPresentationRule(*childNodeRule);
    childNodeRule->AddCustomizationRule(*new ImageIdOverride("TestCondition4", 4, "ImageIdOverrideTestValue"));
    childNodeRule->AddCustomizationRule(*new LabelOverride("TestCondition5", 5, "LabelOverrideLabelValue", "LabelOverrideDescriptionValue"));
    childNodeRule->AddCustomizationRule(*new StyleOverride("TestCondition6", 6, "Blue", "Red", "Bold"));

    AllInstanceNodesSpecification* spec = new  AllInstanceNodesSpecification(1, false, false, false, false, false, "one");
    ChildNodeRule* nestedChildNodeRule = new ChildNodeRule("", 1, false, TargetTree_MainTree);
    nestedChildNodeRule->AddCustomizationRule(*new GroupingRule("", 2, false, "TestSchemaName2", "", "", "", ""));
    spec->AddNestedRule(*nestedChildNodeRule);
    ruleSet->GetChildNodesRules()[0]->AddSpecification(*spec);

    StyleOverrideP styleOverride = new StyleOverride("TestCondition7", 7, "Blue", "Red", "Bold");
    ASSERT_TRUE(nullptr != styleOverride);
    ruleSet->AddPresentationRule(*styleOverride);

    Utf8String serializedRuleSet = ruleSet->WriteToXmlString();
    Utf8String expectedRuleSet = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<PresentationRuleSet RuleSetId=\"TestRuleSet\" SupportedSchemas=\"\" IsSupplemental=\"true\" SupplementationPurpose=\"\" "
        "VersionMajor=\"2\" VersionMinor=\"1\" PreferredImage=\"\" IsSearchEnabled=\"true\" SearchClasses=\"\" ExtendedData=\"\">"
            "<RootNodeRule Priority=\"1\" Condition=\"TestCondition1\" OnlyIfNotHandled=\"true\" TargetTree=\"Both\" StopFurtherProcessing=\"false\" AutoExpand=\"false\">"
                "<ImageIdOverride Priority=\"4\" OnlyIfNotHandled=\"false\" Condition=\"TestCondition3\" ImageId=\"ImageIdOverrideTestValue\"/>"
            "</RootNodeRule>"
            "<ChildNodeRule Priority=\"2\" Condition=\"TestCondition2\" OnlyIfNotHandled=\"true\" TargetTree=\"Both\" StopFurtherProcessing=\"false\">"
                "<AllInstances Priority=\"1\" HasChildren=\"Unknown\" HideNodesInHierarchy=\"false\" HideIfNoChildren=\"false\" ExtendedData=\"\" DoNotSort=\"false\" GroupByClass=\"false\" GroupByLabel=\"false\" SupportedSchemas=\"one\">"
                    "<ChildNodeRule Priority=\"1\" Condition=\"\" OnlyIfNotHandled=\"false\" TargetTree=\"MainTree\" StopFurtherProcessing=\"false\">"
                        "<GroupingRule Priority=\"2\" OnlyIfNotHandled=\"false\" Condition=\"\" SchemaName=\"TestSchemaName2\" ClassName=\"\" ContextMenuCondition=\"\" ContextMenuLabel=\"\" SettingsId=\"\"/>"
                    "</ChildNodeRule>"
                "</AllInstances>"
                "<ImageIdOverride Priority=\"4\" OnlyIfNotHandled=\"false\" Condition=\"TestCondition4\" ImageId=\"ImageIdOverrideTestValue\"/>"
                "<LabelOverride Priority=\"5\" OnlyIfNotHandled=\"false\" Condition=\"TestCondition5\" Label=\"LabelOverrideLabelValue\" Description=\"LabelOverrideDescriptionValue\"/>"
                "<StyleOverride Priority=\"6\" OnlyIfNotHandled=\"false\" Condition=\"TestCondition6\" ForeColor=\"Blue\" BackColor=\"Red\" FontStyle=\"Bold\"/>"
            "</ChildNodeRule>"
            "<StyleOverride Priority=\"7\" OnlyIfNotHandled=\"false\" Condition=\"TestCondition7\" ForeColor=\"Blue\" BackColor=\"Red\" FontStyle=\"Bold\"/>"
        "</PresentationRuleSet>";
    EXPECT_STREQ(expectedRuleSet.c_str(), serializedRuleSet.c_str());
    }