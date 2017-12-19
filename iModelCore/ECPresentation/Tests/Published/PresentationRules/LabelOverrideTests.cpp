/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/LabelOverrideTests.cpp $
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
struct LabelOverrideTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LabelOverrideTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <LabelOverride Label="label" Description="description"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    LabelOverride override;
    EXPECT_TRUE(override.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("label", override.GetLabel().c_str());
    EXPECT_STREQ("description", override.GetDescription().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LabelOverrideTests, LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = "<LabelOverride/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    LabelOverride override;
    EXPECT_TRUE(override.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("", override.GetLabel().c_str());
    EXPECT_STREQ("", override.GetDescription().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(LabelOverrideTests, WriteToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    LabelOverride override;
    override.SetLabel("label");
    override.SetDescription("description");
    override.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<LabelOverride Priority="1000" Label="label" Description="description" Condition="" OnlyIfNotHandled="false"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }