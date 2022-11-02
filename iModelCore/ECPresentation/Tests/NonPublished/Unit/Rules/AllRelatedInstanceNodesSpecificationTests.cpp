/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct AllRelatedInstanceNodesSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AllRelatedInstanceNodesSpecificationTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "specType": "AllRelatedInstanceNodes",
        "groupByClass": false,
        "groupByLabel": false,
        "skipRelatedLevel": 3,
        "supportedSchemas": {"schemaNames":["TestSchema"]},
        "requiredDirection": "Forward"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    AllRelatedInstanceNodesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    EXPECT_FALSE(spec.GetGroupByClass());
    EXPECT_FALSE(spec.GetGroupByLabel());
    EXPECT_EQ(3, spec.GetSkipRelatedLevel());
    EXPECT_STREQ("TestSchema", spec.GetSupportedSchemas().c_str());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Forward, spec.GetRequiredRelationDirection());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AllRelatedInstanceNodesSpecificationTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "specType": "AllRelatedInstanceNodes"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    AllRelatedInstanceNodesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_TRUE(spec.GetGroupByClass());
    EXPECT_FALSE(spec.GetGroupByRelationship());
    EXPECT_TRUE(spec.GetGroupByLabel());
    EXPECT_EQ(0, spec.GetSkipRelatedLevel());
    EXPECT_STREQ("", spec.GetSupportedSchemas().c_str());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Both, spec.GetRequiredRelationDirection());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AllRelatedInstanceNodesSpecificationTests, WriteToJson)
    {
    AllRelatedInstanceNodesSpecification spec(123, true, true, true, true, true, false, 5, "schema1, schema2");
    spec.SetRequiredRelationDirection(RequiredRelationDirection::RequiredRelationDirection_Forward);
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "specType": "AllRelatedInstanceNodes",
        "priority": 123,
        "hasChildren": "Always",
        "hideNodesInHierarchy": true,
        "hideIfNoChildren": true,
        "groupByLabel": false,
        "requiredDirection": "Forward",
        "skipRelatedLevel": 5,
        "supportedSchemas": {"schemaNames": ["schema1", "schema2"]}
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AllRelatedInstanceNodesSpecificationTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <AllRelatedInstances GroupByClass="false" GroupByRelationship="true" GroupByLabel="false"
         SkipRelatedLevel="3" SupportedSchemas="TestSchema" RequiredDirection="Forward"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    AllRelatedInstanceNodesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_FALSE(spec.GetGroupByClass());
    EXPECT_TRUE(spec.GetGroupByRelationship());
    EXPECT_FALSE(spec.GetGroupByLabel());
    EXPECT_EQ(3, spec.GetSkipRelatedLevel());
    EXPECT_STREQ("TestSchema", spec.GetSupportedSchemas().c_str());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Forward, spec.GetRequiredRelationDirection());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AllRelatedInstanceNodesSpecificationTests, LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = "<AllRelatedInstances/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    AllRelatedInstanceNodesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_TRUE(spec.GetGroupByClass());
    EXPECT_FALSE(spec.GetGroupByRelationship());
    EXPECT_TRUE(spec.GetGroupByLabel());
    EXPECT_EQ(0, spec.GetSkipRelatedLevel());
    EXPECT_STREQ("", spec.GetSupportedSchemas().c_str());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Both, spec.GetRequiredRelationDirection());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AllRelatedInstanceNodesSpecificationTests, WriteToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    AllRelatedInstanceNodesSpecification spec;
    spec.SetHasChildren(ChildrenHint::Never);
    spec.SetGroupByClass(false);
    spec.SetGroupByLabel(true);
    spec.SetGroupByRelationship(false);
    spec.SetSkipRelatedLevel(3);
    spec.SetSupportedSchemas("TestSchema");
    spec.SetRequiredRelationDirection(RequiredRelationDirection::RequiredRelationDirection_Forward);
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<AllRelatedInstances Priority="1000" HasChildren="Never" HideNodesInHierarchy="false"
                HideIfNoChildren="false" DoNotSort="false" GroupByClass="false"
                GroupByRelationship="false" GroupByLabel="true" SkipRelatedLevel="3"
                SupportedSchemas="TestSchema" RequiredDirection="Forward"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AllRelatedInstanceNodesSpecificationTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "2073dd9d003d50bcb17c352c3e0152f7";

    // Make sure that introducing additional attributes with default values don't affect the hash
    AllRelatedInstanceNodesSpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    AllRelatedInstanceNodesSpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }
