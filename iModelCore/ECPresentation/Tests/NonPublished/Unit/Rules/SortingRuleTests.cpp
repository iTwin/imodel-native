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
struct SortingRuleTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SortingRuleTests, PropertySorting_LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "PropertySorting",
        "class": {"schemaName": "TestSchema", "className": "TestClass"},
        "propertyName": "TestProperty",
        "sortAscending": false,
        "isPolymorphic": true
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    SortingRule rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("TestSchema", rule.GetSchemaName().c_str());
    EXPECT_STREQ("TestClass", rule.GetClassName().c_str());
    EXPECT_STREQ("TestProperty", rule.GetPropertyName().c_str());
    EXPECT_FALSE(rule.GetSortAscending());
    EXPECT_TRUE(rule.GetIsPolymorphic());
    EXPECT_FALSE(rule.GetDoNotSort());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SortingRuleTests, PropertySorting_LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "PropertySorting"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    SortingRule rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("", rule.GetSchemaName().c_str());
    EXPECT_STREQ("", rule.GetClassName().c_str());
    EXPECT_STREQ("", rule.GetPropertyName().c_str());
    EXPECT_TRUE(rule.GetSortAscending());
    EXPECT_FALSE(rule.GetIsPolymorphic());
    EXPECT_FALSE(rule.GetDoNotSort());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SortingRuleTests, PropertySorting_WriteToJson)
    {
    SortingRule rule("cond", 123, "schema", "class", "prop", false, false, true);
    Json::Value json = rule.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "ruleType": "PropertySorting",
        "priority": 123,
        "condition": "cond",
        "class": {"schemaName": "schema", "className": "class"},
        "propertyName": "prop",
        "sortAscending": false,
        "isPolymorphic": true
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SortingRuleTests, DisabledSorting_LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "DisabledSorting",
        "class": {"schemaName": "TestSchema", "className": "TestClass"},
        "isPolymorphic": true
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    SortingRule rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("TestSchema", rule.GetSchemaName().c_str());
    EXPECT_STREQ("TestClass", rule.GetClassName().c_str());
    EXPECT_STREQ("", rule.GetPropertyName().c_str());
    EXPECT_TRUE(rule.GetSortAscending());
    EXPECT_TRUE(rule.GetDoNotSort());
    EXPECT_TRUE(rule.GetIsPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SortingRuleTests, DisabledSorting_LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "DisabledSorting"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    SortingRule rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("", rule.GetSchemaName().c_str());
    EXPECT_STREQ("", rule.GetClassName().c_str());
    EXPECT_STREQ("", rule.GetPropertyName().c_str());
    EXPECT_TRUE(rule.GetSortAscending());
    EXPECT_TRUE(rule.GetDoNotSort());
    EXPECT_FALSE(rule.GetIsPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SortingRuleTests, DisabledSorting_WriteToJson)
    {
    SortingRule rule("cond", 123, "schema", "class", "", false, true, true);
    Json::Value json = rule.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "ruleType": "DisabledSorting",
        "priority": 123,
        "condition": "cond",
        "class": {"schemaName": "schema", "className": "class"},
        "isPolymorphic": true
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SortingRuleTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <SortingRule SchemaName="TestSchema" ClassName="TestClass" PropertyName="TestProperty" SortAscending="false" DoNotSort="true" IsPolymorphic="true"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    SortingRule rule;
    EXPECT_TRUE(rule.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("TestSchema", rule.GetSchemaName().c_str());
    EXPECT_STREQ("TestClass", rule.GetClassName().c_str());
    EXPECT_STREQ("TestProperty", rule.GetPropertyName().c_str());
    EXPECT_FALSE(rule.GetSortAscending());
    EXPECT_TRUE(rule.GetDoNotSort());
    EXPECT_TRUE(rule.GetIsPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SortingRuleTests, LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = "<SortingRule/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    SortingRule rule;
    EXPECT_TRUE(rule.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("", rule.GetSchemaName().c_str());
    EXPECT_STREQ("", rule.GetClassName().c_str());
    EXPECT_STREQ("", rule.GetPropertyName().c_str());
    EXPECT_TRUE(rule.GetSortAscending());
    EXPECT_FALSE(rule.GetDoNotSort());
    EXPECT_FALSE(rule.GetIsPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SortingRuleTests, WriteToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    SortingRule rule("condition", 1000, "TestSchema", "TestClass", "TestProperty", false, true, true);
    rule.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<SortingRule Priority="1000" OnlyIfNotHandled="false" Condition="condition" SchemaName="TestSchema" ClassName="TestClass" PropertyName="TestProperty" SortAscending="false" DoNotSort="true" IsPolymorphic="true"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SortingRuleTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "d41d8cd98f00b204e9800998ecf8427e";

    // Make sure that introducing additional attributes with default values don't affect the hash
    SortingRule defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    SortingRule copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }
