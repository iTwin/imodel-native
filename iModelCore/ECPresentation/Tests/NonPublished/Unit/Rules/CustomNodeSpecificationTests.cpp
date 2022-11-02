/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct CustomNodeSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodeSpecificationTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "specType": "CustomNode",
        "type": "type",
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodeSpecificationTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "specType": "CustomNode",
        "type": "type",
        "label": "label"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    CustomNodeSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("type", spec.GetNodeType().c_str());
    EXPECT_STREQ("label", spec.GetLabel().c_str());
    EXPECT_STREQ("", spec.GetDescription().c_str());
    EXPECT_STREQ("", spec.GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodeSpecificationTests, WriteToJson)
    {
    CustomNodeSpecification spec(1000, true, "type", "label", "descr", "imageid");
    spec.SetHasChildren(ChildrenHint::Never);
    spec.SetDoNotSort(true);
    spec.SetHideNodesInHierarchy(true);
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "specType": "CustomNode",
        "hasChildren": "Never",
        "hideNodesInHierarchy": true,
        "hideIfNoChildren": true,
        "doNotSort": true,
        "type": "type",
        "label": "label",
        "description": "descr",
        "imageId": "imageid"
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
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
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodeSpecificationTests, LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = "<CustomNode Type=\"type\" Label=\"label\" />";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    CustomNodeSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("type", spec.GetNodeType().c_str());
    EXPECT_STREQ("label", spec.GetLabel().c_str());
    EXPECT_STREQ("", spec.GetDescription().c_str());
    EXPECT_STREQ("", spec.GetImageId().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
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
            R"(<CustomNode Priority="1000" HasChildren="Unknown"
                HideNodesInHierarchy="false" HideIfNoChildren="false" DoNotSort="false"
                Type="type" Label="label" Description="description" ImageId="imgID"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CustomNodeSpecificationTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "e054bcea05d3a00ea1df3807191dd53a";

    // first we make sure that introducing additional attributes with default values don't affect the hash
    CustomNodeSpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // then we make sure that copy has the same hash
    CustomNodeSpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // then we test that each attribute affects the hash
    CustomNodeSpecification specWithNodeType(defaultSpec);
    specWithNodeType.SetNodeType("a");
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithNodeType.GetHash().c_str());
    specWithNodeType.SetNodeType("");
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithNodeType.GetHash().c_str());

    CustomNodeSpecification specWithLabel(defaultSpec);
    specWithLabel.SetLabel("a");
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithLabel.GetHash().c_str());
    specWithLabel.SetLabel("");
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithLabel.GetHash().c_str());

    CustomNodeSpecification specWithDescription(defaultSpec);
    specWithDescription.SetDescription("a");
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithDescription.GetHash().c_str());
    specWithDescription.SetDescription("");
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithDescription.GetHash().c_str());

    CustomNodeSpecification specWithImageId(defaultSpec);
    specWithImageId.SetImageId("a");
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithImageId.GetHash().c_str());
    specWithImageId.SetImageId("");
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithImageId.GetHash().c_str());
    }
