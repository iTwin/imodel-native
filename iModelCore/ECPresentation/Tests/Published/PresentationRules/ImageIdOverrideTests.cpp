/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/ImageIdOverrideTests.cpp $
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
struct ImageIdOverrideTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImageIdOverrideTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <ImageIdOverride ImageId="imgId"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    ImageIdOverride override;
    EXPECT_TRUE(override.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("imgId", override.GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImageIdOverrideTests, LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = "<ImageIdOverride/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    ImageIdOverride override;
    EXPECT_TRUE(override.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("", override.GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImageIdOverrideTests, WriteToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    ImageIdOverride override;
    override.SetImageId("imgId");
    override.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<ImageIdOverride Priority="1000" ImageId="imgId" Condition="" OnlyIfNotHandled="false"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }