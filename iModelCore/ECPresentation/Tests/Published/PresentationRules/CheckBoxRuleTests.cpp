/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/CheckBoxRuleTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct CheckBoxRuleTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CheckBoxRuleTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "CheckBox",
  	    "propertyName": "checkBoxProperty",
  	    "useInversedPropertyValue": true,
  	    "defaultValue": true,
  	    "isEnabled": "isEnabledExpression"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    CheckBoxRule rule;
    ASSERT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("checkBoxProperty", rule.GetPropertyName().c_str());
    EXPECT_TRUE(rule.GetUseInversedPropertyValue());
    EXPECT_TRUE(rule.GetDefaultValue());
    EXPECT_STREQ("isEnabledExpression", rule.GetIsEnabled().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CheckBoxRuleTests, LoadsFromJsonWhenIsEnabledIsBoolean)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "CheckBox",
  	    "propertyName": "checkBoxProperty",
  	    "isEnabled": true
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    CheckBoxRule rule;
    ASSERT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("checkBoxProperty", rule.GetPropertyName().c_str());
    EXPECT_STREQ("true", rule.GetIsEnabled().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CheckBoxRuleTests, WriteToJson)
    {
    CheckBoxRule rule("cond", 123, true, "prop", true, true, "a = b");
    Json::Value json = rule.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "ruleType": "CheckBox",
        "priority": 123,
        "onlyIfNotHandled": true,
        "condition": "cond",
        "propertyName": "prop",
        "useInversedPropertyValue": true,
        "defaultValue": true,
        "isEnabled": "a = b"
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CheckBoxRuleTests, LoadsFromXmlJsonDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "CheckBox"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());
    
    CheckBoxRule rule;
    ASSERT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("", rule.GetPropertyName().c_str());
    EXPECT_FALSE(rule.GetUseInversedPropertyValue());
    EXPECT_FALSE(rule.GetDefaultValue());
    EXPECT_STREQ("", rule.GetIsEnabled().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Aidas.Vaiksnoras                08/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CheckBoxRuleTests, LoadsFromXml)
    {
   static Utf8CP xmlString = R"(
        <CheckBoxRule PropertyName="checkBoxProperty" UseInversedPropertyValue="false" 
         DefaultValue="true" IsEnabled="isEnabledExpression"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    CheckBoxRule rule;
    EXPECT_TRUE(rule.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("checkBoxProperty", rule.GetPropertyName().c_str());
    EXPECT_FALSE(rule.GetUseInversedPropertyValue());
    EXPECT_TRUE(rule.GetDefaultValue());
    EXPECT_STREQ("isEnabledExpression", rule.GetIsEnabled().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CheckBoxRuleTests, LoadsFromXmlWithDefaultValues)
    {
   static Utf8CP xmlString = "<CheckBoxRule/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    CheckBoxRule rule;
    EXPECT_TRUE(rule.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("", rule.GetPropertyName().c_str());
    EXPECT_FALSE(rule.GetUseInversedPropertyValue());
    EXPECT_FALSE(rule.GetDefaultValue());
    EXPECT_STREQ("", rule.GetIsEnabled().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Aidas.Vaiksnoras                08/2017
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(CheckBoxRuleTests, WriteToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    CheckBoxRule rule("conditionexpresion", 9, true, "checkBoxProperty", false, true, "isEnabledExpression");
    rule.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<CheckBoxRule Priority="9" OnlyIfNotHandled="true" Condition="conditionexpresion" PropertyName="checkBoxProperty" UseInversedPropertyValue="false"
                DefaultValue="true" IsEnabled="isEnabledExpression"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }