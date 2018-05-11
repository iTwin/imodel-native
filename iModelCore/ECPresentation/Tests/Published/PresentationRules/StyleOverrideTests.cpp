/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/StyleOverrideTests.cpp $
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
struct StyleOverrideTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(StyleOverrideTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "foreColor": "fore",
        "backColor": "back",
        "fontStyle": "font"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());
    
    StyleOverride override;
    EXPECT_TRUE(override.ReadJson(json));
    EXPECT_STREQ("fore", override.GetForeColor().c_str());
    EXPECT_STREQ("back", override.GetBackColor().c_str());
    EXPECT_STREQ("font", override.GetFontStyle().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(StyleOverrideTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = "{}";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    StyleOverride override;
    EXPECT_TRUE(override.ReadJson(json));
    EXPECT_STREQ("", override.GetForeColor().c_str());
    EXPECT_STREQ("", override.GetBackColor().c_str());
    EXPECT_STREQ("", override.GetFontStyle().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(StyleOverrideTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <StyleOverride ForeColor="fore" BackColor="back" FontStyle="font"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    StyleOverride override;
    EXPECT_TRUE(override.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("fore", override.GetForeColor().c_str());
    EXPECT_STREQ("back", override.GetBackColor().c_str());
    EXPECT_STREQ("font", override.GetFontStyle().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(StyleOverrideTests, LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = "<StyleOverride />";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    StyleOverride override;
    EXPECT_TRUE(override.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("", override.GetForeColor().c_str());
    EXPECT_STREQ("", override.GetBackColor().c_str());
    EXPECT_STREQ("", override.GetFontStyle().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(StyleOverrideTests, WriteToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    StyleOverride override("condition", 1000, "foreToBetReplaced", "back", "font");
    override.SetForeColor("fore");
    override.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<StyleOverride Priority="1000" ForeColor="fore" BackColor="back" FontStyle="font" Condition="condition" OnlyIfNotHandled="false"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }