/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/DisplayRelatedItemsSpecificationTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct DisplayRelatedItemsSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DisplayRelatedItemsSpecificationTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <DisplayRelatedItems NestingDepth="1" LogicalChildren="false" RelationshipClasses="ClassAHasClassB"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    DisplayRelatedItemsSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_EQ(1, spec.GetNestingDepth());
    EXPECT_FALSE(spec.GetLogicalChildren());
    EXPECT_STREQ("ClassAHasClassB", spec.GetRelationshipClasses().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DisplayRelatedItemsSpecificationTests, LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = "<DisplayRelatedItems/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    DisplayRelatedItemsSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_EQ(0, spec.GetNestingDepth());
    EXPECT_TRUE(spec.GetLogicalChildren());
    EXPECT_STREQ("", spec.GetRelationshipClasses().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DisplayRelatedItemsSpecificationTests, WritesToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    DisplayRelatedItemsSpecification spec(false, 3, "ClassAHasClassB");
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<DisplayRelatedItems NestingDepth="3" LogicalChildren="false" RelationshipClasses="ClassAHasClassB"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DisplayRelatedItemsSpecificationTests, ComputesCorrectHashes)
    {
    DisplayRelatedItemsSpecification spec1(false, 3, "ClassAHasClassB");
    DisplayRelatedItemsSpecification spec2(false, 3, "ClassAHasClassB");
    DisplayRelatedItemsSpecification spec3(false, 3, "ClassBHasClassA");

    // Hashes are same for identical specifications
    EXPECT_STREQ(spec1.GetHash().c_str(), spec2.GetHash().c_str());
    // Hashes differs for different specifications
    EXPECT_STRNE(spec1.GetHash().c_str(), spec3.GetHash().c_str());
    }