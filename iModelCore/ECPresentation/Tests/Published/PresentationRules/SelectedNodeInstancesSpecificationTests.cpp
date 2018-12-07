/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/SelectedNodeInstancesSpecificationTests.cpp $
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
struct SelectedNodeInstancesSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SelectedNodeInstancesSpecificationTests, LoadFromJson)
    {
    static Utf8CP jsonString = R"({
        "specType": "SelectedNodeInstances",
        "onlyIfNotHandled": true,
        "acceptableSchemaName": "TestSchema",
        "acceptableClassNames": ["ClassA", "B"],
        "acceptablePolymorphically": true
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    SelectedNodeInstancesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_TRUE(spec.GetOnlyIfNotHandled());
    EXPECT_TRUE(spec.GetAcceptablePolymorphically());
    EXPECT_STREQ("TestSchema", spec.GetAcceptableSchemaName().c_str());
    EXPECT_STREQ("ClassA,B", spec.GetAcceptableClassNames().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
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
    EXPECT_FALSE(spec.GetOnlyIfNotHandled());
    EXPECT_FALSE(spec.GetAcceptablePolymorphically());
    EXPECT_STREQ("", spec.GetAcceptableSchemaName().c_str());
    EXPECT_STREQ("", spec.GetAcceptableClassNames().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
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
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SelectedNodeInstancesSpecificationTests, LoadFromXml)
    {
    static Utf8CP xmlString = R"(
        <SelectedNodeInstances OnlyIfNotHandled="true" AcceptableSchemaName="TestSchema" AcceptableClassNames="ClassA" AcceptablePolymorphically="true"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    SelectedNodeInstancesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_TRUE(spec.GetOnlyIfNotHandled());
    EXPECT_TRUE(spec.GetAcceptablePolymorphically());
    EXPECT_STREQ("TestSchema", spec.GetAcceptableSchemaName().c_str());
    EXPECT_STREQ("ClassA", spec.GetAcceptableClassNames().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SelectedNodeInstancesSpecificationTests, LoadFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = "<SelectedNodeInstances/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    SelectedNodeInstancesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_FALSE(spec.GetOnlyIfNotHandled());
    EXPECT_FALSE(spec.GetAcceptablePolymorphically());
    EXPECT_STREQ("", spec.GetAcceptableSchemaName().c_str());
    EXPECT_STREQ("", spec.GetAcceptableClassNames().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SelectedNodeInstancesSpecificationTests, WritesToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    SelectedNodeInstancesSpecification spec;
    spec.SetOnlyIfNotHandled(true);
    spec.SetAcceptablePolymorphically(true);
    spec.SetAcceptableSchemaName("TestSchema");
    spec.SetAcceptableClassNames("ClassA");
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<SelectedNodeInstances Priority="1000" ShowImages="false" OnlyIfNotHandled="true" AcceptableSchemaName="TestSchema"
               AcceptableClassNames="ClassA" AcceptablePolymorphically="true"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SelectedNodeInstancesSpecificationTests, ComputesCorrectHashes)
    {
    SelectedNodeInstancesSpecification spec1(1000, false, "TestSchema", "ClassA", true);
    SelectedNodeInstancesSpecification spec2(1000, false, "TestSchema", "ClassA", true);
    SelectedNodeInstancesSpecification spec3(1000, false, "TestSchema2", "ClassB", true);

    // Hashes are same for identical specifications
    EXPECT_STREQ(spec1.GetHash().c_str(), spec2.GetHash().c_str());
    // Hashes differs for different specifications
    EXPECT_STRNE(spec1.GetHash().c_str(), spec3.GetHash().c_str());
    }