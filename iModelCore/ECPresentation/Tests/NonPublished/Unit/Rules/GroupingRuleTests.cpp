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
struct GroupingRuleTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyGroup_CopyConstructorCopiesRanges)
    {
    // Create group1
    PropertyGroup group1("imgId", true, "propertyName", "defaultLabel");
    group1.AddRange(*new PropertyRangeGroupSpecification());
    group1.AddRange(*new PropertyRangeGroupSpecification());

    // Validate group1
    EXPECT_STREQ("imgId", group1.GetImageId().c_str());
    EXPECT_STREQ("propertyName", group1.GetPropertyName().c_str());
    EXPECT_STREQ("defaultLabel", group1.GetDefaultLabel().c_str());
    EXPECT_TRUE(group1.GetCreateGroupForSingleItem());
    EXPECT_EQ(PropertyGroupingValue::DisplayLabel, group1.GetPropertyGroupingValue());
    EXPECT_EQ(PropertyGroupingValue::DisplayLabel, group1.GetSortingValue());
    EXPECT_EQ(2, group1.GetRanges().size());

    // Create group2 via copy Constructor
    PropertyGroup group2(group1);

    // Validate group2
    EXPECT_STREQ("imgId", group2.GetImageId().c_str());
    EXPECT_STREQ("propertyName", group2.GetPropertyName().c_str());
    EXPECT_STREQ("defaultLabel", group2.GetDefaultLabel().c_str());
    EXPECT_TRUE(group1.GetCreateGroupForSingleItem());
    EXPECT_EQ(PropertyGroupingValue::DisplayLabel, group2.GetPropertyGroupingValue());
    EXPECT_EQ(PropertyGroupingValue::DisplayLabel, group2.GetSortingValue());
    EXPECT_EQ(2, group2.GetRanges().size());
    for (size_t i = 0; i < group1.GetRanges().size(); i++)
        EXPECT_NE(group1.GetRanges()[i], group2.GetRanges()[i]);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, SameLabelInstanceGroup_LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "specType": "SameLabelInstance",
        "applicationStage": "PostProcess"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    SameLabelInstanceGroup rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_EQ(SameLabelInstanceGroupApplicationStage::PostProcess, rule.GetApplicationStage());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, SameLabelInstanceGroup_LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "specType": "SameLabelInstance"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    SameLabelInstanceGroup rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("", rule.GetDefaultLabel().c_str());
    EXPECT_EQ(SameLabelInstanceGroupApplicationStage::Query, rule.GetApplicationStage());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, SameLabelInstanceGroup_WriteToJson)
    {
    SameLabelInstanceGroup rule(SameLabelInstanceGroupApplicationStage::PostProcess);
    BeJsDocument json = rule.WriteJson();
    BeJsDocument expected(R"({
        "specType": "SameLabelInstance",
        "applicationStage": "PostProcess"
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, SameLabelInstanceGroup_ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "a3b02b3fba3fd257720465dafbe0c617";

    // Make sure that introducing additional attributes with default values don't affect the hash
    SameLabelInstanceGroup defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    SameLabelInstanceGroup copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, ClassGroup_LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "specType": "Class",
        "createGroupForSingleItem": true,
        "baseClass": {"schemaName": "TestSchema", "className": "TestClass"}
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    ClassGroup rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_TRUE(rule.GetCreateGroupForSingleItem());
    EXPECT_STREQ("TestSchema", rule.GetSchemaName().c_str());
    EXPECT_STREQ("TestClass", rule.GetBaseClassName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, ClassGroup_LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "specType": "Class"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    ClassGroup rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_FALSE(rule.GetCreateGroupForSingleItem());
    EXPECT_STREQ("", rule.GetDefaultLabel().c_str());
    EXPECT_STREQ("", rule.GetDefaultLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, ClassGroup_WriteToJson)
    {
    ClassGroup rule(true, "s", "c");
    BeJsDocument json = rule.WriteJson();
    BeJsDocument expected(R"({
        "specType": "Class",
        "createGroupForSingleItem": true,
        "baseClass": {"schemaName": "s", "className": "c"}
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, ClassGroup_ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "9bd81329febf6efe22788e03ddeaf0af";

    // Make sure that introducing additional attributes with default values don't affect the hash
    ClassGroup defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    ClassGroup copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyRangeGroupSpecification_LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "fromValue": "fromValue",
        "toValue": "toValue",
        "label": "label",
        "imageId": "imgId"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertyRangeGroupSpecification rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("fromValue", rule.GetFromValue().c_str());
    EXPECT_STREQ("toValue", rule.GetToValue().c_str());
    EXPECT_STREQ("label", rule.GetLabel().c_str());
    EXPECT_STREQ("imgId", rule.GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyRangeGroupSpecification_LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "fromValue": "fromValue",
        "toValue": "toValue"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertyRangeGroupSpecification rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("fromValue", rule.GetFromValue().c_str());
    EXPECT_STREQ("toValue", rule.GetToValue().c_str());
    EXPECT_STREQ("", rule.GetLabel().c_str());
    EXPECT_STREQ("", rule.GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyRangeGroupSpecification_LoadFromJsonFailsWhenFromValueIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "toValue": "toValue"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertyRangeGroupSpecification rule;
    EXPECT_FALSE(rule.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyRangeGroupSpecification_LoadFromJsonFailsWhenToValueIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "fromValue": "fromValue"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertyRangeGroupSpecification rule;
    EXPECT_FALSE(rule.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyRangeGroupSpecification_WriteToJson)
    {
    PropertyRangeGroupSpecification rule("label", "imageid", "f", "t");
    BeJsDocument json = rule.WriteJson();
    BeJsDocument expected(R"({
        "label": "label",
        "imageId": "imageid",
        "fromValue": "f",
        "toValue": "t"
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyRangeGroupSpecification_ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "cb263eb376b3b097540ad2fcc79865d8";

    // Make sure that introducing additional attributes with default values don't affect the hash
    PropertyRangeGroupSpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    PropertyRangeGroupSpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyGroup_LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "specType": "Property",
        "propertyName": "property",
        "imageId": "imgId",
        "createGroupForSingleItem": true,
        "createGroupForUnspecifiedValues": false,
        "groupingValue": "DisplayLabel",
        "sortingValue": "PropertyValue",
        "ranges": [{
            "fromValue": "fromValue",
            "toValue": "toValue",
            "label": "label",
            "imageId": "imgId"
        }]
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertyGroup rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("property", rule.GetPropertyName().c_str());
    EXPECT_STREQ("imgId", rule.GetImageId().c_str());
    EXPECT_TRUE(rule.GetCreateGroupForSingleItem());
    EXPECT_FALSE(rule.GetCreateGroupForUnspecifiedValues());
    EXPECT_EQ(PropertyGroupingValue::DisplayLabel, rule.GetPropertyGroupingValue());
    EXPECT_EQ(PropertyGroupingValue::PropertyValue, rule.GetSortingValue());
    EXPECT_EQ(1, rule.GetRanges().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyGroup_LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "specType": "Property",
        "propertyName": "property"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertyGroup rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("property", rule.GetPropertyName().c_str());
    EXPECT_STREQ("", rule.GetImageId().c_str());
    EXPECT_FALSE(rule.GetCreateGroupForSingleItem());
    EXPECT_TRUE(rule.GetCreateGroupForUnspecifiedValues());
    EXPECT_EQ(PropertyGroupingValue::DisplayLabel, rule.GetPropertyGroupingValue());
    EXPECT_EQ(PropertyGroupingValue::DisplayLabel, rule.GetSortingValue());
    EXPECT_EQ(0, rule.GetRanges().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyGroup_LoadFromJsonFailsWhenPropertyNameIsNorSpecified)
    {
    static Utf8CP jsonString = R"({
        "specType": "Property"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertyGroup rule;
    EXPECT_FALSE(rule.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyGroup_LoadFromJsonAssertsWhenPropertyGroupingValueIsInvalidAndReturnsDefault)
    {
    static Utf8CP jsonString = R"({
        "specType": "Property",
        "propertyName": "property",
        "groupingValue": "InvalidValue"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertyGroup rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("property", rule.GetPropertyName().c_str());
    EXPECT_EQ(PropertyGroupingValue::DisplayLabel, rule.GetPropertyGroupingValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyGroup_WriteToJson)
    {
    PropertyGroup rule("imageid", true, "prop", "label");
    rule.SetCreateGroupForUnspecifiedValues(true);
    rule.SetPropertyGroupingValue(PropertyGroupingValue::PropertyValue);
    rule.AddRange(*new PropertyRangeGroupSpecification("range", "", "f", "t"));
    BeJsDocument json = rule.WriteJson();
    BeJsDocument expected(R"({
        "specType": "Property",
        "createGroupForSingleItem": true,
        "propertyName": "prop",
        "imageId": "imageid",
        "groupingValue": "PropertyValue",
        "defaultLabel": "label",
        "ranges": [{
            "label": "range",
            "fromValue": "f",
            "toValue": "t"
        }]
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyGroup_ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "5ad234cb2cde4266195252a23ca7d84e";

    // Make sure that introducing additional attributes with default values don't affect the hash
    PropertyGroup defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    PropertyGroup copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "Grouping",
        "class": {"schemaName": "TestSchema", "className": "TestClass"},
        "groups": [{
            "specType": "Property",
            "propertyName": "property",
            "imageId": "imgId",
            "createGroupForSingleItem": "true",
            "createGroupForUnspecifiedValues": "false",
            "groupingValue": "DisplayLabel",
            "sortingValue": "PropertyValue",
            "ranges": [{
                "fromValue": "fromValue",
                "toValue": "toValue",
                "label": "label",
                "imageId": "imgId"
            }]
        }, {
            "specType": "SameLabelInstance"
        }, {
            "specType": "Class",
            "createGroupForSingleItem": true,
            "baseClass": {"schemaName": "TestSchema", "className": "TestClass"}
        }]
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    GroupingRule rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("TestSchema", rule.GetSchemaName().c_str());
    EXPECT_STREQ("TestClass", rule.GetClassName().c_str());
    EXPECT_EQ(3, rule.GetGroups().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, LoadFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "Grouping",
        "class": {"schemaName": "TestSchema", "className": "TestClass"}
    })";
    BeJsDocument json(jsonString);
    ASSERT_FALSE(json.isNull());

    GroupingRule rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("TestSchema", rule.GetSchemaName().c_str());
    EXPECT_STREQ("TestClass", rule.GetClassName().c_str());
    EXPECT_STREQ("", rule.GetSettingsId().c_str());
    EXPECT_EQ(0, rule.GetGroups().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, LoadFromJsonFailsWhenClassNameIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "Grouping",
        "class": {"schemaName": "TestSchema"}
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    GroupingRule rule;
    EXPECT_FALSE(rule.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, LoadFromJsonFailsWhenSchemaNameIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "Grouping",
        "class": {"className": "TestClass"}
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    GroupingRule rule;
    EXPECT_FALSE(rule.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, WriteToJson)
    {
    GroupingRule rule("cond", 123, true, "s", "c", "");
    rule.AddGroup(*new SameLabelInstanceGroup());
    rule.AddGroup(*new ClassGroup());
    rule.AddGroup(*new PropertyGroup());
    BeJsDocument json = rule.WriteJson();
    BeJsDocument expected(R"({
        "ruleType": "Grouping",
        "priority": 123,
        "onlyIfNotHandled": true,
        "condition": "cond",
        "class": {"schemaName":"s", "className":"c"},
        "groups": [{
            "specType": "SameLabelInstance"
        }, {
            "specType": "Class"
        }, {
            "specType": "Property",
            "propertyName": ""
        }]
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "0940b48511da784b3c7213afa0c0eade";

    // Make sure that introducing additional attributes with default values don't affect the hash
    GroupingRule defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    GroupingRule copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }
