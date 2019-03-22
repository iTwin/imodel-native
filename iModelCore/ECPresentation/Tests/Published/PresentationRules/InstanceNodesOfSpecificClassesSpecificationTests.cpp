/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/InstanceNodesOfSpecificClassesSpecificationTests.cpp $
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
struct InstanceNodesOfSpecificClassesSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceNodesOfSpecificClassesSpecificationTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "specType": "InstanceNodesOfSpecificClasses",
        "classes": {"schemaName": "TestSchema", "classNames": ["TestClass"]},
        "arePolymorphic": true,
        "groupByClass": false,
        "groupByLabel": false,
        "instanceFilter": "filter"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());
    
    InstanceNodesOfSpecificClassesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("TestSchema:TestClass", spec.GetClassNames().c_str());
    EXPECT_TRUE(spec.GetArePolymorphic());
    EXPECT_FALSE(spec.GetGroupByClass());
    EXPECT_FALSE(spec.GetGroupByLabel());
    EXPECT_STREQ("filter", spec.GetInstanceFilter().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceNodesOfSpecificClassesSpecificationTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "specType": "InstanceNodesOfSpecificClasses",
        "classes": {"schemaName": "TestSchema", "classNames": ["TestClass"]}
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    InstanceNodesOfSpecificClassesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("TestSchema:TestClass", spec.GetClassNames().c_str());
    EXPECT_FALSE(spec.GetArePolymorphic());
    EXPECT_TRUE(spec.GetGroupByClass());
    EXPECT_TRUE(spec.GetGroupByLabel());
    EXPECT_STREQ("", spec.GetInstanceFilter().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceNodesOfSpecificClassesSpecificationTests, LoadsFromJsonFailsWhenClassesAttributeIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "specType": "InstanceNodesOfSpecificClasses"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    InstanceNodesOfSpecificClassesSpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceNodesOfSpecificClassesSpecificationTests, WriteToJson)
    {
    InstanceNodesOfSpecificClassesSpecification spec(456, true, true, true, true, true,
        true, "filter", "s1:c1,c2;s2:c3", true);
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "specType": "InstanceNodesOfSpecificClasses",
        "priority": 456,
        "hasChildren": "Always",
        "hideNodesInHierarchy": true,
        "hideIfNoChildren": true,
        "classes": [
            {"schemaName": "s1", "classNames": ["c1", "c2"]},
            {"schemaName": "s2", "classNames": ["c3"]}
        ],
        "arePolymorphic": true,
        "instanceFilter": "filter"
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceNodesOfSpecificClassesSpecificationTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <InstancesOfSpecificClasses ClassNames="TestSchema:TestClass" ArePolymorphic="true" GroupByClass="false"
         GroupByLabel="false" ShowEmptyGroups="true" InstanceFilter="filter"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    InstanceNodesOfSpecificClassesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("TestSchema:TestClass", spec.GetClassNames().c_str());
    EXPECT_TRUE(spec.GetArePolymorphic());
    EXPECT_FALSE(spec.GetGroupByClass());
    EXPECT_FALSE(spec.GetGroupByLabel());
    EXPECT_TRUE(spec.GetShowEmptyGroups());
    EXPECT_STREQ("filter", spec.GetInstanceFilter().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceNodesOfSpecificClassesSpecificationTests, LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = R"(
        <InstancesOfSpecificClasses ClassNames="TestSchema:TestClass"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    InstanceNodesOfSpecificClassesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("TestSchema:TestClass", spec.GetClassNames().c_str());
    EXPECT_FALSE(spec.GetArePolymorphic());
    EXPECT_TRUE(spec.GetGroupByClass());
    EXPECT_TRUE(spec.GetGroupByLabel());
    EXPECT_FALSE(spec.GetShowEmptyGroups());
    EXPECT_STREQ("", spec.GetInstanceFilter().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceNodesOfSpecificClassesSpecificationTests, LoadsFromXmlFailsWhenClassNamesAttributeIsNotSpecified)
    {
    static Utf8CP xmlString = "<InstancesOfSpecificClasses/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    InstanceNodesOfSpecificClassesSpecification spec;
    EXPECT_FALSE(spec.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceNodesOfSpecificClassesSpecificationTests, WritesToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    InstanceNodesOfSpecificClassesSpecification spec;
    spec.SetClassNames("TestSchema:TestClass");
    spec.SetArePolymorphic(true);
    spec.SetGroupByClass(false);
    spec.SetGroupByLabel(false);
    spec.SetShowEmptyGroups(true);
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
           R"(<InstancesOfSpecificClasses Priority="1000" HasChildren="Unknown" HideNodesInHierarchy="false" 
               HideIfNoChildren="false" ExtendedData="" DoNotSort="false" ClassNames="TestSchema:TestClass"
               ArePolymorphic="true" GroupByClass="false" GroupByLabel="false" ShowEmptyGroups="true" InstanceFilter=""/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceNodesOfSpecificClassesSpecificationTests, ComputesCorrectHashes)
    {
    InstanceNodesOfSpecificClassesSpecification spec1(1, true, true, true, true, true, true, "", "TestClass", true);
    InstanceNodesOfSpecificClassesSpecification spec2(1, true, true, true, true, true, true, "", "TestClass", true);
    InstanceNodesOfSpecificClassesSpecification spec3(1, false, true, false, true, true, true, "", "TestClass2", true);

    // Hashes are same for identical specifications
    EXPECT_STREQ(spec1.GetHash().c_str(), spec2.GetHash().c_str());
    // Hashes differs for different specifications
    EXPECT_STRNE(spec1.GetHash().c_str(), spec3.GetHash().c_str());
    }
