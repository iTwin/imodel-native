/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/RelatedInstanceSpecificationTests.cpp $
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
struct RelatedInstanceSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <RelatedInstance RelationshipName="ClassAHasClassB" ClassName="ClassA" Alias="TestAlias"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    RelatedInstanceSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("ClassAHasClassB", spec.GetRelationshipName().c_str());
    EXPECT_STREQ("ClassA", spec.GetClassName().c_str());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Both, spec.GetRelationshipDirection());
    EXPECT_STREQ("TestAlias", spec.GetAlias().c_str());
    EXPECT_EQ(false, spec.IsRequired());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, LoadsFromXmlWithIsRequiredAttributeSet)
    {
    static Utf8CP xmlString = R"(
        <RelatedInstance RelationshipName="ClassAHasClassB" ClassName="ClassA" Alias="TestAlias" IsRequired="true"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    RelatedInstanceSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("ClassAHasClassB", spec.GetRelationshipName().c_str());
    EXPECT_STREQ("ClassA", spec.GetClassName().c_str());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Both, spec.GetRelationshipDirection());
    EXPECT_STREQ("TestAlias", spec.GetAlias().c_str());
    EXPECT_EQ(true, spec.IsRequired());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, LoadFromXmlFailsWhenRelationshipAttributeIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <RelatedInstance ClassName="ClassA" Alias="TestAlias"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    RelatedInstanceSpecification spec;
    EXPECT_FALSE(spec.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, LoadFromXmlFailsWhenClassNameAttributeIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <RelatedInstance RelationshipName="ClassAHasClassB" Alias="TestAlias"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    RelatedInstanceSpecification spec;
    EXPECT_FALSE(spec.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, LoadFromXmlFailsWhenAliasAttributeIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <RelatedInstance RelationshipName="ClassAHasClassB" ClassName="ClassA"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    RelatedInstanceSpecification spec;
    EXPECT_FALSE(spec.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, WritesToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    RelatedInstanceSpecification spec(RequiredRelationDirection::RequiredRelationDirection_Backward, "ClassAHasClassB", "ClassA", "TestAlias");
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<RelatedInstance ClassName="ClassA" RelationshipName="ClassAHasClassB" RelationshipDirection="Backward" Alias="TestAlias" IsRequired="false"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, WritesToXmlWithIsRequiredAttribute)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    RelatedInstanceSpecification spec(RequiredRelationDirection::RequiredRelationDirection_Backward, "ClassAHasClassB", "ClassA", "TestAlias", true);
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<RelatedInstance ClassName="ClassA" RelationshipName="ClassAHasClassB" RelationshipDirection="Backward" Alias="TestAlias" IsRequired="true"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, ComputesCorrectHashes)
    {
    RelatedInstanceSpecification spec1(RequiredRelationDirection::RequiredRelationDirection_Backward, "Relation", "ClassName", "class1");
    RelatedInstanceSpecification spec2(RequiredRelationDirection::RequiredRelationDirection_Backward, "Relation", "ClassName", "class1");
    RelatedInstanceSpecification spec3(RequiredRelationDirection::RequiredRelationDirection_Backward, "Relation", "ClassName", "class2");

    // Hashes are same for identical specifications
    EXPECT_STREQ(spec1.GetHash().c_str(), spec2.GetHash().c_str());
    // Hashes differs for different specifications
    EXPECT_STRNE(spec1.GetHash().c_str(), spec3.GetHash().c_str());
    }