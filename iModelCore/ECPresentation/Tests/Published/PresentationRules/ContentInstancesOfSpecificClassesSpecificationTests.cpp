/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/ContentInstancesOfSpecificClassesSpecificationTests.cpp $
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
struct ContentInstancesOfSpecificClassesSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentInstancesOfSpecificClassesSpecificationTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
	    "classNames": "TestSchema:TestClass",
	    "arePolymorphic": true,
	    "instanceFilter": "filter"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());
    
    ContentInstancesOfSpecificClassesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("TestSchema:TestClass", spec.GetClassNames().c_str());
    EXPECT_TRUE(spec.GetArePolymorphic());
    EXPECT_STREQ("filter", spec.GetInstanceFilter().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentInstancesOfSpecificClassesSpecificationTests, LoadFromJsonFailsWhenClassNamesAreNotSpecified)
    {
    static Utf8CP jsonString = "{}";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    ContentInstancesOfSpecificClassesSpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentInstancesOfSpecificClassesSpecificationTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
	    "classNames": "TestSchema:TestClass"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());
    
    ContentInstancesOfSpecificClassesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));

    EXPECT_STREQ("TestSchema:TestClass", spec.GetClassNames().c_str());
    EXPECT_FALSE(spec.GetArePolymorphic());
    EXPECT_STREQ("", spec.GetInstanceFilter().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentInstancesOfSpecificClassesSpecificationTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <ContentInstancesOfSpecificClasses ClassNames="TestSchema:TestClass" ArePolymorphic="true" InstanceFilter="filter"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    ContentInstancesOfSpecificClassesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("TestSchema:TestClass", spec.GetClassNames().c_str());
    EXPECT_TRUE(spec.GetArePolymorphic());
    EXPECT_STREQ("filter", spec.GetInstanceFilter().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentInstancesOfSpecificClassesSpecificationTests, LoadFromXmlFailsWhenClassNamesAreNotSpecified)
    {
    static Utf8CP xmlString = "<ContentInstancesOfSpecificClasses/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    ContentInstancesOfSpecificClassesSpecification spec;
    EXPECT_FALSE(spec.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentInstancesOfSpecificClassesSpecificationTests, LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = R"(
        <ContentInstancesOfSpecificClasses ClassNames="TestSchema:ClassA"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    ContentInstancesOfSpecificClassesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));

    EXPECT_STREQ("TestSchema:ClassA", spec.GetClassNames().c_str());
    EXPECT_FALSE(spec.GetArePolymorphic());
    EXPECT_STREQ("", spec.GetInstanceFilter().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentInstancesOfSpecificClassesSpecificationTests, WritesToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    ContentInstancesOfSpecificClassesSpecification spec;
    spec.SetClassNames("TestSchema:TestClass");
    spec.SetArePolymorphic(true);
    spec.SetInstanceFilter("filter");
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<ContentInstancesOfSpecificClasses Priority="1000" ShowImages="false" 
                ClassNames="TestSchema:TestClass" ArePolymorphic="true" InstanceFilter="filter"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentInstancesOfSpecificClassesSpecificationTests, ComputesCorrectHashes)
    {
    ContentInstancesOfSpecificClassesSpecification spec1(1000, "filter", "TestSchema:ClassA", true);
    ContentInstancesOfSpecificClassesSpecification spec2(1000, "filter", "TestSchema:ClassA", true);
    ContentInstancesOfSpecificClassesSpecification spec3(1000, "instancefilter", "TestSchema:ClassB", true);

    // Hashes are same for identical specifications
    EXPECT_STREQ(spec1.GetHash().c_str(), spec2.GetHash().c_str());
    // Hashes differs for different specifications
    EXPECT_STRNE(spec1.GetHash().c_str(), spec3.GetHash().c_str());
    }