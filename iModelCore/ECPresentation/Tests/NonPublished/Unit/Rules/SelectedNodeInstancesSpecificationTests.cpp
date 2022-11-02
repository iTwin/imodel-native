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
struct SelectedNodeInstancesSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SelectedNodeInstancesSpecificationTests, LoadFromJson)
    {
    static Utf8CP jsonString = R"({
        "specType": "SelectedNodeInstances",
        "acceptableSchemaName": "TestSchema",
        "acceptableClassNames": ["ClassA", "B"],
        "acceptablePolymorphically": true
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    SelectedNodeInstancesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_TRUE(spec.GetAcceptablePolymorphically());
    EXPECT_STREQ("TestSchema", spec.GetAcceptableSchemaName().c_str());
    EXPECT_STREQ("ClassA,B", spec.GetAcceptableClassNames().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SelectedNodeInstancesSpecificationTests, LoadFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "specType": "SelectedNodeInstances"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    SelectedNodeInstancesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_FALSE(spec.GetAcceptablePolymorphically());
    EXPECT_STREQ("", spec.GetAcceptableSchemaName().c_str());
    EXPECT_STREQ("", spec.GetAcceptableClassNames().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SelectedNodeInstancesSpecificationTests, WriteToJson)
    {
    SelectedNodeInstancesSpecification spec(123, true, "schema", "a, b", true);
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "specType": "SelectedNodeInstances",
        "priority": 123,
        "onlyIfNotHandled": true,
        "acceptableSchemaName": "schema",
        "acceptableClassNames": ["a", "b"],
        "acceptablePolymorphically": true
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SelectedNodeInstancesSpecificationTests, LoadFromXml)
    {
    static Utf8CP xmlString = R"(
        <SelectedNodeInstances AcceptableSchemaName="TestSchema" AcceptableClassNames="ClassA" AcceptablePolymorphically="true"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    SelectedNodeInstancesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_TRUE(spec.GetAcceptablePolymorphically());
    EXPECT_STREQ("TestSchema", spec.GetAcceptableSchemaName().c_str());
    EXPECT_STREQ("ClassA", spec.GetAcceptableClassNames().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SelectedNodeInstancesSpecificationTests, LoadFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = "<SelectedNodeInstances/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    SelectedNodeInstancesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_FALSE(spec.GetAcceptablePolymorphically());
    EXPECT_STREQ("", spec.GetAcceptableSchemaName().c_str());
    EXPECT_STREQ("", spec.GetAcceptableClassNames().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SelectedNodeInstancesSpecificationTests, WritesToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    SelectedNodeInstancesSpecification spec;
    spec.SetAcceptablePolymorphically(true);
    spec.SetAcceptableSchemaName("TestSchema");
    spec.SetAcceptableClassNames("ClassA");
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<SelectedNodeInstances Priority="1000" ShowImages="false" OnlyIfNotHandled="false" AcceptableSchemaName="TestSchema"
               AcceptableClassNames="ClassA" AcceptablePolymorphically="true"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SelectedNodeInstancesSpecificationTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "5d76802072cbf92405575180cdf256f3";

    // Make sure that introducing additional attributes with default values don't affect the hash
    SelectedNodeInstancesSpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    SelectedNodeInstancesSpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // Test that each attribute affects the hash
    SelectedNodeInstancesSpecification specSchemaName(defaultSpec);
    specSchemaName.SetAcceptableSchemaName("test");
    EXPECT_STRNE(DEFAULT_HASH, specSchemaName.GetHash().c_str());
    specSchemaName.SetAcceptableSchemaName("");
    EXPECT_STREQ(DEFAULT_HASH, specSchemaName.GetHash().c_str());

    SelectedNodeInstancesSpecification specClassNames(defaultSpec);
    specClassNames.SetAcceptableClassNames("test");
    EXPECT_STRNE(DEFAULT_HASH, specClassNames.GetHash().c_str());
    specClassNames.SetAcceptableClassNames("");
    EXPECT_STREQ(DEFAULT_HASH, specClassNames.GetHash().c_str());

    SelectedNodeInstancesSpecification specPolymorphically(defaultSpec);
    specPolymorphically.SetAcceptablePolymorphically(true);
    EXPECT_STRNE(DEFAULT_HASH, specPolymorphically.GetHash().c_str());
    specPolymorphically.SetAcceptablePolymorphically(false);
    EXPECT_STREQ(DEFAULT_HASH, specPolymorphically.GetHash().c_str());
    }
