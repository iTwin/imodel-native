/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/SearchResultInstanceNodesSpecificationTests.cpp $
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
struct SearchResultInstanceNodesSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, CopyConstructorCopiesSpecifications)
    {
    // Create spec1
    SearchResultInstanceNodesSpecification spec1;
    spec1.AddQuerySpecification(*new StringQuerySpecification("TestQuery", "TestSchema", "TestClass"));
    spec1.AddQuerySpecification(*new ECPropertyValueQuerySpecification("TestSchema", "TestClass", "TestParent"));
    EXPECT_EQ(2, spec1.GetQuerySpecifications().size());
    // Create spec2 via copy constructor
    SearchResultInstanceNodesSpecification spec2(spec1);
    // Validate spec2
    EXPECT_EQ(2, spec2.GetQuerySpecifications().size());
    // Validate deep copy
    for (size_t i = 0; i < spec1.GetQuerySpecifications().size(); i++)
        EXPECT_NE(spec1.GetQuerySpecifications()[i], spec2.GetQuerySpecifications()[i]);  
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, StringQuerySpecification_LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <StringQuery SchemaName="TestSchema" ClassName="TestClass">QueryString</StringQuery>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    StringQuerySpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("TestSchema", spec.GetSchemaName().c_str());
    EXPECT_STREQ("TestClass", spec.GetClassName().c_str());
    EXPECT_STREQ("QueryString", spec.GetQuery().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, StringQuerySpecification_LoadFromXmlFailsWhenSchemaNameIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <StringQuery ClassName="TestClass">QueryString</StringQuery>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    StringQuerySpecification spec;
    EXPECT_FALSE(spec.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, StringQuerySpecification_LoadFromXmlFailsWhenClassNameIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <StringQuery SchemaName="TestSchema">QueryString</StringQuery>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    StringQuerySpecification spec;
    EXPECT_FALSE(spec.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, StringQuerySpecification_LoadFromXmlFailsWhenQueryStringIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <StringQuery SchemaName="TestSchema" ClassName="TestClass"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    StringQuerySpecification spec;
    EXPECT_FALSE(spec.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, ECPropertyValueQuerySpecification_LoadFromXml)
    {
    static Utf8CP xmlString = R"(
        <PropertyValueQuery SchemaName="TestSchema" ClassName="TestClass" ParentPropertyName="parent"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    ECPropertyValueQuerySpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("TestSchema", spec.GetSchemaName().c_str());
    EXPECT_STREQ("TestClass", spec.GetClassName().c_str());
    EXPECT_STREQ("parent", spec.GetParentPropertyName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, ECPropertyValueQuerySpecification_LoadFailsFromXmlWhenSchemaNameIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <PropertyValueQuery ClassName="TestClass" ParentPropertyName="parent"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    ECPropertyValueQuerySpecification spec;
    EXPECT_FALSE(spec.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, ECPropertyValueQuerySpecification_LoadFailsFromXmlWhenClassNameIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <PropertyValueQuery SchemaName="TestSchema" ParentPropertyName="parent"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    ECPropertyValueQuerySpecification spec;
    EXPECT_FALSE(spec.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, ECPropertyValueQuerySpecification_LoadFailsFromXmlWhenParentNameIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <PropertyValueQuery SchemaName="TestSchema" ClassName="TestClass"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    ECPropertyValueQuerySpecification spec;
    EXPECT_FALSE(spec.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <SearchResultInstances GroupByClass="false" GroupByLabel="false"> 
            <StringQuery SchemaName="TestSchema" ClassName="TestClass">StringQuery</StringQuery>
            <PropertyValueQuery SchemaName="TestSchema" ClassName="TestClass" ParentPropertyName="parent"/>
        </SearchResultInstances> 
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    SearchResultInstanceNodesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_FALSE(spec.GetGroupByClass());
    EXPECT_FALSE(spec.GetGroupByLabel());
    EXPECT_EQ(2, spec.GetQuerySpecifications().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = "<SearchResultInstances/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    SearchResultInstanceNodesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_TRUE(spec.GetGroupByClass());
    EXPECT_TRUE(spec.GetGroupByLabel());
    EXPECT_TRUE(spec.GetQuerySpecifications().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, StringQuerySpecification_WriteToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    StringQuerySpecification spec;
    spec.SetClassName("TestClass");
    spec.SetSchemaName("TestSchema");
    spec.SetQuery("TestQuery");
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<StringQuery SchemaName="TestSchema" ClassName="TestClass">)"
                R"(TestQuery)"
            R"(</StringQuery>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, ECPropertyValueQuerySpecification_WriteToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    ECPropertyValueQuerySpecification spec;
    spec.SetClassName("TestClass");
    spec.SetSchemaName("TestSchema");
    spec.SetParentPropertyName("TestParent");
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<PropertyValueQuery SchemaName="TestSchema" ClassName="TestClass" ParentPropertyName="TestParent"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, WriteToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);
    SearchResultInstanceNodesSpecification spec;
    spec.SetGroupByClass(false);
    spec.SetGroupByLabel(false);
    spec.AddQuerySpecification(*new StringQuerySpecification("TestQuery", "TestSchema", "TestClass"));
    spec.AddQuerySpecification(*new ECPropertyValueQuerySpecification("TestSchema", "TestClass", "TestParent"));
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<SearchResultInstances Priority="1000" AlwaysReturnsChildren="false" HideNodesInHierarchy="false" HideIfNoChildren="false" ExtendedData="" DoNotSort="false" GroupByClass="false" GroupByLabel="false">)"
                R"(<StringQuery SchemaName="TestSchema" ClassName="TestClass">TestQuery</StringQuery>)"
                R"(<PropertyValueQuery SchemaName="TestSchema" ClassName="TestClass" ParentPropertyName="TestParent"/>)"
            R"(</SearchResultInstances>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, ComputesCorrectHashes)
    {  
    SearchResultInstanceNodesSpecification spec1;
    spec1.AddQuerySpecification(*new StringQuerySpecification("query", "TestSchema", "TestClass"));
    spec1.AddQuerySpecification(*new ECPropertyValueQuerySpecification("TestSchema", "TestClass", "Property"));
    SearchResultInstanceNodesSpecification spec2;
    spec2.AddQuerySpecification(*new StringQuerySpecification("query", "TestSchema", "TestClass"));
    spec2.AddQuerySpecification(*new ECPropertyValueQuerySpecification("TestSchema", "TestClass", "Property"));
    SearchResultInstanceNodesSpecification spec3;
    spec3.AddQuerySpecification(*new StringQuerySpecification("query", "TestSchema", "TestClass"));

    // Hashes are same for specifications with same properties
    EXPECT_STREQ(spec1.GetHash().c_str(), spec2.GetHash().c_str());
    // Hashes differ for specifications with different properties
    EXPECT_STRNE(spec1.GetHash().c_str(), spec3.GetHash().c_str());
    }