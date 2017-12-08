/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/GroupingRuleTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
TEST_F(GroupingRuleTests, PropertyGroup_AssertsWhenPropertyGroupingValueIsInvalidAndReturnsDefault)
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
            R"(<GroupingRule Priority="1000" SchemaName="TestSchema" ClassName="TestClass" ContextMenuCondition="TestMenuCondition" ContextMenuLabel="TestLabel" SettingsId="TestID" Condition="condition" OnlyIfNotHandled="true">)"
                R"(<ClassGroup ContextMenuLabel="TestMenuLabel" DefaultGroupLabel="" CreateGroupForSingleItem="true" SchemaName="TestSchema" BaseClassName="TestClass"/>)"
                R"(<SameLabelInstanceGroup ContextMenuLabel="label" DefaultGroupLabel=""/>)"
                R"(<PropertyGroup ContextMenuLabel="menuLabel" DefaultGroupLabel="defaultLabel" ImageId="imgId" CreateGroupForSingleItem="true" CreateGroupForUnspecifiedValues="true" PropertyName="testProperty" GroupingValue="DisplayLabel" SortingValue="DisplayLabel">)"
                    R"(<Range FromValue="fromValue" ToValue="toValue" Label="label" ImageId="imgId"/>)"
                R"(</PropertyGroup>)"
            R"(</GroupingRule>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }