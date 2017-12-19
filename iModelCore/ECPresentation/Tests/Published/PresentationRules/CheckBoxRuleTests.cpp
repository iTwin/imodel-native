/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/CheckBoxRuleTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
            R"(<CheckBoxRule Priority="9" PropertyName="checkBoxProperty" UseInversedPropertyValue="false"
                DefaultValue="true" IsEnabled="isEnabledExpression" Condition="conditionexpresion" OnlyIfNotHandled="true"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }