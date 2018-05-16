/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/CustomNodeSpecificationTests.cpp $
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
struct CustomNodeSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodeSpecificationTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
	    "nodeType": "type",
	    "label": "label",
	    "description": "description",
	    "imageId": "imgID"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    CustomNodeSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("type", spec.GetNodeType().c_str());
    EXPECT_STREQ("label", spec.GetLabel().c_str());
    EXPECT_STREQ("description", spec.GetDescription().c_str());
    EXPECT_STREQ("imgID", spec.GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodeSpecificationTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = "{}";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    CustomNodeSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("", spec.GetNodeType().c_str());
    EXPECT_STREQ("", spec.GetLabel().c_str());
    EXPECT_STREQ("", spec.GetDescription().c_str());
    EXPECT_STREQ("", spec.GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodeSpecificationTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <CustomNode Type="type" Label="label" Description="description" ImageId="imgID"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    CustomNodeSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("type", spec.GetNodeType().c_str());
    EXPECT_STREQ("label", spec.GetLabel().c_str());
    EXPECT_STREQ("description", spec.GetDescription().c_str());
    EXPECT_STREQ("imgID", spec.GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodeSpecificationTests, LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = "<CustomNode/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    CustomNodeSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("", spec.GetNodeType().c_str());
    EXPECT_STREQ("", spec.GetLabel().c_str());
    EXPECT_STREQ("", spec.GetDescription().c_str());
    EXPECT_STREQ("", spec.GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodeSpecificationTests, WritesToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    CustomNodeSpecification spec;
    spec.SetNodeType("type");
    spec.SetLabel("label");
    spec.SetDescription("description");
    spec.SetImageId("imgID");
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<CustomNode Priority="1000" AlwaysReturnsChildren="false"
                HideNodesInHierarchy="false" HideIfNoChildren="false" ExtendedData="" DoNotSort="false"
                Type="type" Label="label" Description="description" ImageId="imgID"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }