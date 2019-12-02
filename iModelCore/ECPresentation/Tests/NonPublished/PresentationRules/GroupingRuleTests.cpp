/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct GroupingRuleTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyGroup_CopyConstructorCopiesRanges)
    {
    // Create group1
    PropertyGroup group1("menuLabel", "imgId", true, "propertyName", "defaultLabel");
    group1.AddRange(*new PropertyRangeGroupSpecification());
    group1.AddRange(*new PropertyRangeGroupSpecification());

    // Validate group1
    EXPECT_STREQ("menuLabel", group1.GetContextMenuLabel().c_str());
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
    EXPECT_STREQ("menuLabel", group2.GetContextMenuLabel().c_str());
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
* @betest                                   Grigas.Petraitis                    11/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, SameLabelInstanceGroup_LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "specType": "SameLabelInstance",
        "applicationStage": "PostProcess"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    SameLabelInstanceGroup rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_EQ(SameLabelInstanceGroupApplicationStage::PostProcess, rule.GetApplicationStage());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, SameLabelInstanceGroup_LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "specType": "SameLabelInstance"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());
    
    SameLabelInstanceGroup rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("", rule.GetContextMenuLabel().c_str());
    EXPECT_STREQ("", rule.GetDefaultLabel().c_str());
    EXPECT_EQ(SameLabelInstanceGroupApplicationStage::Query, rule.GetApplicationStage());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, SameLabelInstanceGroup_WriteToJson)
    {
    SameLabelInstanceGroup rule(SameLabelInstanceGroupApplicationStage::PostProcess);
    Json::Value json = rule.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "specType": "SameLabelInstance",
        "applicationStage": "PostProcess"
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, SameLabelInstanceGroup_LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <SameLabelInstanceGroup ContextMenuLabel="TestMenuLabel" DefaultGroupLabel="defaultGroupLabel"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    SameLabelInstanceGroup rule;
    EXPECT_TRUE(rule.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("TestMenuLabel", rule.GetContextMenuLabel().c_str());
    EXPECT_STREQ("defaultGroupLabel", rule.GetDefaultLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, SameLabelInstanceGroup_LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = "<SameLabelInstanceGroup/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    SameLabelInstanceGroup rule;
    EXPECT_TRUE(rule.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("", rule.GetContextMenuLabel().c_str());
    EXPECT_STREQ("", rule.GetDefaultLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, ClassGroup_LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "specType": "Class",
        "createGroupForSingleItem": true,
        "baseClass": {"schemaName": "TestSchema", "className": "TestClass"}
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());
    
    ClassGroup rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_TRUE(rule.GetCreateGroupForSingleItem());
    EXPECT_STREQ("TestSchema", rule.GetSchemaName().c_str());
    EXPECT_STREQ("TestClass", rule.GetBaseClassName().c_str());
    EXPECT_STREQ("", rule.GetContextMenuLabel().c_str());
    EXPECT_STREQ("", rule.GetDefaultLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, ClassGroup_LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "specType": "Class"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    ClassGroup rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_FALSE(rule.GetCreateGroupForSingleItem());
    EXPECT_STREQ("", rule.GetContextMenuLabel().c_str());
    EXPECT_STREQ("", rule.GetDefaultLabel().c_str());
    EXPECT_STREQ("", rule.GetContextMenuLabel().c_str());
    EXPECT_STREQ("", rule.GetDefaultLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, ClassGroup_WriteToJson)
    {
    ClassGroup rule("", "true", "s", "c");
    Json::Value json = rule.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "specType": "Class",
        "createGroupForSingleItem": true,
        "baseClass": {"schemaName": "s", "className": "c"}
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, ClassGroup_LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <ClassGroup CreateGroupForSingleItem="true" SchemaName="TestSchema" BaseClassName="TestClass"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    ClassGroup rule;
    EXPECT_TRUE(rule.ReadXml(xml->GetRootElement()));
    EXPECT_TRUE(rule.GetCreateGroupForSingleItem());
    EXPECT_STREQ("TestSchema", rule.GetSchemaName().c_str());
    EXPECT_STREQ("TestClass", rule.GetBaseClassName().c_str());
    EXPECT_STREQ("", rule.GetContextMenuLabel().c_str());
    EXPECT_STREQ("", rule.GetDefaultLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, ClassGroup_LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = "<ClassGroup/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    ClassGroup rule;
    EXPECT_TRUE(rule.ReadXml(xml->GetRootElement()));
    EXPECT_FALSE(rule.GetCreateGroupForSingleItem());
    EXPECT_STREQ("", rule.GetContextMenuLabel().c_str());
    EXPECT_STREQ("", rule.GetDefaultLabel().c_str());
    EXPECT_STREQ("", rule.GetContextMenuLabel().c_str());
    EXPECT_STREQ("", rule.GetDefaultLabel().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyRangeGroupSpecification_LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "fromValue": "fromValue",
        "toValue": "toValue",
        "label": "label",
        "imageId": "imgId"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertyRangeGroupSpecification rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("fromValue", rule.GetFromValue().c_str());
    EXPECT_STREQ("toValue", rule.GetToValue().c_str());
    EXPECT_STREQ("label", rule.GetLabel().c_str());
    EXPECT_STREQ("imgId", rule.GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyRangeGroupSpecification_LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "fromValue": "fromValue",
        "toValue": "toValue"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());
    
    PropertyRangeGroupSpecification rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("fromValue", rule.GetFromValue().c_str());
    EXPECT_STREQ("toValue", rule.GetToValue().c_str());
    EXPECT_STREQ("", rule.GetLabel().c_str());
    EXPECT_STREQ("", rule.GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyRangeGroupSpecification_LoadFromJsonFailsWhenFromValueIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "toValue": "toValue"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertyRangeGroupSpecification rule;
    EXPECT_FALSE(rule.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyRangeGroupSpecification_LoadFromJsonFailsWhenToValueIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "fromValue": "fromValue"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertyRangeGroupSpecification rule;
    EXPECT_FALSE(rule.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyRangeGroupSpecification_WriteToJson)
    {
    PropertyRangeGroupSpecification rule("label", "imageid", "f", "t");
    Json::Value json = rule.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "label": "label",
        "imageId": "imageid",
        "fromValue": "f",
        "toValue": "t"
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyRangeGroupSpecification_LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <Range FromValue="fromValue" ToValue="toValue" Label="label" ImageId="imgId"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    PropertyRangeGroupSpecification rule;
    EXPECT_TRUE(rule.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("fromValue", rule.GetFromValue().c_str());
    EXPECT_STREQ("toValue", rule.GetToValue().c_str());
    EXPECT_STREQ("label", rule.GetLabel().c_str());
    EXPECT_STREQ("imgId", rule.GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyRangeGroupSpecification_LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = R"(
        <Range FromValue="fromValue" ToValue="toValue"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    PropertyRangeGroupSpecification rule;
    EXPECT_TRUE(rule.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("fromValue", rule.GetFromValue().c_str());
    EXPECT_STREQ("toValue", rule.GetToValue().c_str());
    EXPECT_STREQ("", rule.GetLabel().c_str());
    EXPECT_STREQ("", rule.GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyRangeGroupSpecification_LoadFromXmlFailsWhenFromValueIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <Range ToValue="toValue"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    PropertyRangeGroupSpecification rule;
    EXPECT_FALSE(rule.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyRangeGroupSpecification_LoadFromXmlFailsWhenToValueIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <Range FromValue="fromValue"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    PropertyRangeGroupSpecification rule;
    EXPECT_FALSE(rule.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
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
    Json::Value json = Json::Reader::DoParse(jsonString);
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
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyGroup_LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "specType": "Property",
        "propertyName": "property"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());
    IGNORE_BE_ASSERT();

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
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyGroup_LoadFromJsonFailsWhenPropertyNameIsNorSpecified)
    {
    static Utf8CP jsonString = R"({
        "specType": "Property"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertyGroup rule;
    EXPECT_FALSE(rule.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyGroup_LoadFromJsonAssertsWhenPropertyGroupingValueIsInvalidAndReturnsDefault)
    {
    static Utf8CP jsonString = R"({
        "specType": "Property",
        "propertyName": "property",
        "groupingValue": "InvalidValue"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertyGroup rule;
    IGNORE_BE_ASSERT();
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("property", rule.GetPropertyName().c_str());
    EXPECT_EQ(PropertyGroupingValue::DisplayLabel, rule.GetPropertyGroupingValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyGroup_WriteToJson)
    {
    PropertyGroup rule("", "imageid", true, "prop", "label");
    rule.SetCreateGroupForUnspecifiedValues(true);
    rule.SetPropertyGroupingValue(PropertyGroupingValue::PropertyValue);
    rule.AddRange(*new PropertyRangeGroupSpecification("range", "", "f", "t"));
    Json::Value json = rule.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
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
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyGroup_LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <PropertyGroup PropertyName="property" ImageId="imgId" CreateGroupForSingleItem="true" CreateGroupForUnspecifiedValues="false" GroupingValue="DisplayLabel" SortingValue="PropertyValue">
            <Range FromValue="fromValue" ToValue="toValue" Label="label" ImageId="imgId"/>
        </PropertyGroup>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    PropertyGroup rule;
    EXPECT_TRUE(rule.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("property", rule.GetPropertyName().c_str());
    EXPECT_STREQ("imgId", rule.GetImageId().c_str());
    EXPECT_EQ(PropertyGroupingValue::DisplayLabel, rule.GetPropertyGroupingValue());
    EXPECT_EQ(PropertyGroupingValue::PropertyValue, rule.GetSortingValue());
    EXPECT_EQ(1, rule.GetRanges().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyGroup_LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = R"(
        <PropertyGroup PropertyName="property"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    PropertyGroup rule;
    EXPECT_TRUE(rule.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("property", rule.GetPropertyName().c_str());
    EXPECT_STREQ("", rule.GetImageId().c_str());
    EXPECT_EQ(PropertyGroupingValue::DisplayLabel, rule.GetPropertyGroupingValue());
    EXPECT_EQ(PropertyGroupingValue::DisplayLabel, rule.GetSortingValue());
    EXPECT_EQ(0, rule.GetRanges().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyGroup_LoadFromXmlFailsWhenPropertyNameIsNorSpecified)
    {
    static Utf8CP xmlString = "<PropertyGroup/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    PropertyGroup rule;
    EXPECT_FALSE(rule.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyGroup_LoadFromXmlAssertsWhenPropertyGroupingValueIsInvalidAndReturnsDefault)
    {
    static Utf8CP xmlString = R"(
        <PropertyGroup PropertyName="property" GroupingValue="InvalidValue"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    PropertyGroup rule;
    IGNORE_BE_ASSERT();
    EXPECT_TRUE(rule.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("property", rule.GetPropertyName().c_str());
    EXPECT_EQ(PropertyGroupingValue::DisplayLabel, rule.GetPropertyGroupingValue());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
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
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());
    
    GroupingRule rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("TestSchema", rule.GetSchemaName().c_str());
    EXPECT_STREQ("TestClass", rule.GetClassName().c_str());
    EXPECT_EQ(3, rule.GetGroups().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, LoadFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "Grouping",
        "class": {"schemaName": "TestSchema", "className": "TestClass"}
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());
    
    GroupingRule rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("TestSchema", rule.GetSchemaName().c_str());
    EXPECT_STREQ("TestClass", rule.GetClassName().c_str());
    EXPECT_STREQ("", rule.GetContextMenuCondition().c_str());
    EXPECT_STREQ("", rule.GetContextMenuLabel().c_str());
    EXPECT_STREQ("", rule.GetSettingsId().c_str());
    EXPECT_EQ(0, rule.GetGroups().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, LoadFromJsonFailsWhenClassNameIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "Grouping",
        "class": {"schemaName": "TestSchema"}
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    GroupingRule rule;
    EXPECT_FALSE(rule.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, LoadFromJsonFailsWhenSchemaNameIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "Grouping",
        "class": {"className": "TestClass"}
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    GroupingRule rule;
    EXPECT_FALSE(rule.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, WriteToJson)
    {
    GroupingRule rule("cond", 123, true, "s", "c", "", "", "");
    rule.AddGroup(*new SameLabelInstanceGroup());
    rule.AddGroup(*new ClassGroup());
    rule.AddGroup(*new PropertyGroup());
    Json::Value json = rule.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
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
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <GroupingRule SchemaName="TestSchema" ClassName="TestClass" ContextMenuCondition="TestMenuCondition" ContextMenuLabel="TestLabel" SettingsId="TestID">
            <ClassGroup CreateGroupForSingleItem="true" SchemaName="TestSchema" BaseClassName="TestClass"/>
            <SameLabelInstanceGroup CreateGroupForSingleItem="true" SchemaName="TestSchema" BaseClassName="TestClass"/>
            <PropertyGroup PropertyName="property" ImageId="imgId" CreateGroupForSingleItem="true" CreateGroupForUnspecifiedValues="false" GroupingValue="DisplayLabel" SortingValue="PropertyValue"/>
        </GroupingRule>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    GroupingRule rule;
    EXPECT_TRUE(rule.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("TestSchema", rule.GetSchemaName().c_str());
    EXPECT_STREQ("TestClass", rule.GetClassName().c_str());
    EXPECT_STREQ("TestMenuCondition", rule.GetContextMenuCondition().c_str());
    EXPECT_STREQ("TestLabel", rule.GetContextMenuLabel().c_str());
    EXPECT_STREQ("TestID", rule.GetSettingsId().c_str());
    EXPECT_EQ(3, rule.GetGroups().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, LoadFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = R"(
        <GroupingRule SchemaName="TestSchema" ClassName="TestClass"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    GroupingRule rule;
    EXPECT_TRUE(rule.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("TestSchema", rule.GetSchemaName().c_str());
    EXPECT_STREQ("TestClass", rule.GetClassName().c_str());
    EXPECT_STREQ("", rule.GetContextMenuCondition().c_str());
    EXPECT_STREQ("", rule.GetContextMenuLabel().c_str());
    EXPECT_STREQ("", rule.GetSettingsId().c_str());
    EXPECT_EQ(0, rule.GetGroups().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, LoadFromXmlFailsWhenClassNameIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <GroupingRule SchemaName="TestSchema" ContextMenuCondition="TestMenuCondition" ContextMenuLabel="TestLabel" SettingsId="TestID"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    GroupingRule rule;
    EXPECT_FALSE(rule.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, LoadFromXmlFailsWhenSchemaNameIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <GroupingRule ClassName="TestClass" ContextMenuCondition="TestMenuCondition" ContextMenuLabel="TestLabel" SettingsId="TestID"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    GroupingRule rule;
    EXPECT_FALSE(rule.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyRangeGroupSpecification_WriteToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    PropertyRangeGroupSpecification spec("label", "imgId", "fromValue", "toValue");
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<Range FromValue="fromValue" ToValue="toValue" Label="label" ImageId="imgId"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, SameLabelInstanceGroup_WriteToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    SameLabelInstanceGroup spec("label");
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<SameLabelInstanceGroup ContextMenuLabel="label" DefaultGroupLabel=""/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, PropertyGroup_WriteToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    PropertyGroup group("menuLabel", "imgId", true, "testProperty", "defaultLabel");
    group.SetSortingValue(PropertyGroupingValue::PropertyValue);
    group.AddRange(*new PropertyRangeGroupSpecification("label", "imgId", "fromValue", "toValue"));
    group.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<PropertyGroup ContextMenuLabel="menuLabel" DefaultGroupLabel="defaultLabel" ImageId="imgId" CreateGroupForSingleItem="true" CreateGroupForUnspecifiedValues="true" PropertyName="testProperty" GroupingValue="DisplayLabel" SortingValue="PropertyValue">)"
                R"(<Range FromValue="fromValue" ToValue="toValue" Label="label" ImageId="imgId"/>)"
            R"(</PropertyGroup>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, ClassGroup_WriteToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    ClassGroup group("TestMenuLabel", true, "TestSchema", "TestClass");
    group.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<ClassGroup ContextMenuLabel="TestMenuLabel" DefaultGroupLabel="" CreateGroupForSingleItem="true" SchemaName="TestSchema" BaseClassName="TestClass"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GroupingRuleTests, WriteToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    GroupingRule rule("condition", 1000, true, "TestSchema", "TestClass", "ConditionToBeReplaced", "TestLabel", "TestID");
    rule.SetContextMenuCondition("TestMenuCondition");
    rule.AddGroup(*new ClassGroup("TestMenuLabel", true, "TestSchema", "TestClass"));
    rule.AddGroup(*new SameLabelInstanceGroup("label"));
    PropertyGroup* propertyGroup = new PropertyGroup("menuLabel", "imgId", true, "testProperty", "defaultLabel");
    propertyGroup->AddRange(*new PropertyRangeGroupSpecification("label", "imgId", "fromValue", "toValue"));
    rule.AddGroup(*propertyGroup);  
    rule.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<GroupingRule Priority="1000" OnlyIfNotHandled="true" Condition="condition" SchemaName="TestSchema" ClassName="TestClass" ContextMenuCondition="TestMenuCondition" ContextMenuLabel="TestLabel" SettingsId="TestID">)"
                R"(<ClassGroup ContextMenuLabel="TestMenuLabel" DefaultGroupLabel="" CreateGroupForSingleItem="true" SchemaName="TestSchema" BaseClassName="TestClass"/>)"
                R"(<SameLabelInstanceGroup ContextMenuLabel="label" DefaultGroupLabel=""/>)"
                R"(<PropertyGroup ContextMenuLabel="menuLabel" DefaultGroupLabel="defaultLabel" ImageId="imgId" CreateGroupForSingleItem="true" CreateGroupForUnspecifiedValues="true" PropertyName="testProperty" GroupingValue="DisplayLabel" SortingValue="DisplayLabel">)"
                    R"(<Range FromValue="fromValue" ToValue="toValue" Label="label" ImageId="imgId"/>)"
                R"(</PropertyGroup>)"
            R"(</GroupingRule>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }