/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/InstanceLabelOverrideTests.cpp $
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
struct InstanceLabelOverrideTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "className": "TestClass",
        "properties": [ "prop1", "prop2" ]
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    InstanceLabelOverride override;

    EXPECT_TRUE(override.ReadJson(json));
    EXPECT_STREQ("TestClass", override.GetClassName().c_str());
    ASSERT_EQ(2, override.GetPropertyNames().size());
    EXPECT_STREQ("prop1", override.GetPropertyNames()[0].c_str());
    EXPECT_STREQ("prop2", override.GetPropertyNames()[1].c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideTests, LoadsFromJson_TrimsEmptySpacesAroundPropertyNames)
    {
    static Utf8CP jsonString = R"({
	    "className": "TestClass",
	    "properties": [" prop1   ", "   prop2  "]
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    InstanceLabelOverride override;

    EXPECT_TRUE(override.ReadJson(json));
    EXPECT_STREQ("TestClass", override.GetClassName().c_str());
    ASSERT_EQ(2, override.GetPropertyNames().size());
    EXPECT_STREQ("prop1", override.GetPropertyNames()[0].c_str());
    EXPECT_STREQ("prop2", override.GetPropertyNames()[1].c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = "{}";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    InstanceLabelOverride override;
    
    EXPECT_TRUE(override.ReadJson(json));
    EXPECT_STREQ("", override.GetClassName().c_str());
    EXPECT_TRUE(override.GetPropertyNames().empty());
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
    static Utf8CP xmlString = "<InstanceLabelOverride/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    InstanceLabelOverride override;
    
    EXPECT_TRUE(override.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("", override.GetClassName().c_str());
    EXPECT_TRUE(override.GetPropertyNames().empty());
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
            R"(<InstanceLabelOverride Priority="100" ClassName="TestClassName" PropertyNames="TestProperties" OnlyIfNotHandled="true"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }