/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelOverrideTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "InstanceLabelOverride",
        "class": {"schemaName": "TestSchema", "className": "TestClass"},
        "propertyNames": [ "prop1", "prop2" ]
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    InstanceLabelOverride rule;

    ASSERT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("TestSchema:TestClass", rule.GetClassName().c_str());
    ASSERT_EQ(2, rule.GetPropertyNames().size());
    EXPECT_STREQ("prop1", rule.GetPropertyNames()[0].c_str());
    EXPECT_STREQ("prop2", rule.GetPropertyNames()[1].c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideTests, LoadsFromJson_TrimsEmptySpacesAroundPropertyNames)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "InstanceLabelOverride",
        "class": {"schemaName": "TestSchema", "className": "TestClass"},
        "propertyNames": [" prop1   ", "   prop2  "]
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    InstanceLabelOverride rule;

    ASSERT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("TestSchema:TestClass", rule.GetClassName().c_str());
    ASSERT_EQ(2, rule.GetPropertyNames().size());
    EXPECT_STREQ("prop1", rule.GetPropertyNames()[0].c_str());
    EXPECT_STREQ("prop2", rule.GetPropertyNames()[1].c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "InstanceLabelOverride",
        "class": {"schemaName": "TestSchema", "className": "TestClass"},
        "propertyNames": ["test"]
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    InstanceLabelOverride rule;    
    ASSERT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("TestSchema:TestClass", rule.GetClassName().c_str());
    ASSERT_EQ(1, rule.GetPropertyNames().size());
    ASSERT_STREQ("test", rule.GetPropertyNames()[0].c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideTests, WriteToJson)
    {
    InstanceLabelOverride rule(123, true, "s:c", "p1,p2");
    Json::Value json = rule.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "ruleType": "InstanceLabelOverride",
        "priority": 123,
        "onlyIfNotHandled": true,
        "class": {"schemaName": "s", "className": "c"},
        "propertyNames": ["p1", "p2"]
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <InstanceLabelOverride ClassName="TestClass" PropertyNames="prop1,prop2"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    InstanceLabelOverride override;
    EXPECT_TRUE(override.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("TestClass", override.GetClassName().c_str());
    ASSERT_EQ(2, override.GetPropertyNames().size());
    EXPECT_STREQ("prop1", override.GetPropertyNames()[0].c_str());
    EXPECT_STREQ("prop2", override.GetPropertyNames()[1].c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideTests, LoadsFromXml_TrimsEmptySpacesAroundPropertyNames)
    {
    static Utf8CP xmlString = R"(
        <InstanceLabelOverride ClassName="TestClass" PropertyNames="  prop1 ,   prop2  "/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    InstanceLabelOverride override;
    EXPECT_TRUE(override.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("TestClass", override.GetClassName().c_str());
    ASSERT_EQ(2, override.GetPropertyNames().size());
    EXPECT_STREQ("prop1", override.GetPropertyNames()[0].c_str());
    EXPECT_STREQ("prop2", override.GetPropertyNames()[1].c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideTests, LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = "<InstanceLabelOverride ClassName=\"schema:class\" PropertyNames=\"Test\" />";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    InstanceLabelOverride rule;
    
    ASSERT_TRUE(rule.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("schema:class", rule.GetClassName().c_str());
    ASSERT_EQ(1, rule.GetPropertyNames().size());
    EXPECT_STREQ("Test", rule.GetPropertyNames()[0].c_str());
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Aidas.Vaiksnoras               12/2017
//+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideTests, WriteToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    InstanceLabelOverride override(100, true, "TestClassName", "TestProperties");
    override.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<InstanceLabelOverride Priority="100" OnlyIfNotHandled="true" ClassName="TestClassName" PropertyNames="TestProperties"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }