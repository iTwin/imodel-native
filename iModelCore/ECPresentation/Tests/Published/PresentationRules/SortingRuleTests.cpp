/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/SortingRuleTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct SortingRuleTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SortingRuleTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
	    "schemaName": "TestSchema",
	    "className": "TestClass",
	    "propertyName": "TestProperty",
	    "sortAscending": false,
	    "doNotSort": true,
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
    EXPECT_TRUE(rule.GetDoNotSort());
    EXPECT_TRUE(rule.GetIsPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SortingRuleTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = "{}";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());
    
    SortingRule rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("", rule.GetSchemaName().c_str());
    EXPECT_STREQ("", rule.GetClassName().c_str());
    EXPECT_STREQ("", rule.GetPropertyName().c_str());
    EXPECT_TRUE(rule.GetSortAscending());
    EXPECT_FALSE(rule.GetDoNotSort());
    EXPECT_FALSE(rule.GetIsPolymorphic());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
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
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
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
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
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
            R"(<SortingRule Priority="1000" SchemaName="TestSchema" ClassName="TestClass" PropertyName="TestProperty" SortAscending="false" DoNotSort="true" IsPolymorphic="true" Condition="condition" OnlyIfNotHandled="false"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }