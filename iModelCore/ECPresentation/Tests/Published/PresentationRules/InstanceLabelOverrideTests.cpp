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
    EXPECT_STREQ("prop1,prop2", override.GetPropertyNames().c_str());
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
    EXPECT_STREQ("", override.GetPropertyNames().c_str());
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