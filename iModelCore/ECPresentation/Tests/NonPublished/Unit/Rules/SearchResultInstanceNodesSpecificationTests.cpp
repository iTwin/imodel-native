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
struct SearchResultInstanceNodesSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, StringQuerySpecification_LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "specType": "String",
        "class": {"schemaName": "TestSchema", "className": "TestClass"},
        "query": "QueryString"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    StringQuerySpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("TestSchema", spec.GetSchemaName().c_str());
    EXPECT_STREQ("TestClass", spec.GetClassName().c_str());
    EXPECT_STREQ("QueryString", spec.GetQuery().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, StringQuerySpecification_LoadFromJsonFailsWhenSpecTypeIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "class": {"schemaName": "TestSchema", "className": "TestClass"},
        "query": "QueryString"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    StringQuerySpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, StringQuerySpecification_LoadFromJsonFailsWhenSchemaNameIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "specType": "String",
        "class": {"className": "TestClass"},
        "query": "QueryString"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    StringQuerySpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, StringQuerySpecification_LoadFromJsonFailsWhenClassNameIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "specType": "String",
        "class": {"schemaName": "TestSchema"},
        "query": "QueryString"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    StringQuerySpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, StringQuerySpecification_LoadFromJsonFailsWhenQueryStringIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "specType": "String",
        "class": {"schemaName": "TestSchema", "className": "TestClass"}
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    StringQuerySpecification spec;
    ASSERT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, StringQuerySpecification_ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "27118326006d3829667a400ad23d5d98";

    // Make sure that introducing additional attributes with default values don't affect the hash
    StringQuerySpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    StringQuerySpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, ECPropertyValueQuerySpecification_LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "specType": "ECPropertyValue",
        "class": {"schemaName": "TestSchema", "className": "TestClass"},
        "parentPropertyName": "parent"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    ECPropertyValueQuerySpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("TestSchema", spec.GetSchemaName().c_str());
    EXPECT_STREQ("TestClass", spec.GetClassName().c_str());
    EXPECT_STREQ("parent", spec.GetParentPropertyName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, ECPropertyValueQuerySpecification_LoadFromJsonFailsWhenSpecTypeIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "class": {"className": "TestClass"},
        "parentPropertyName": "parent"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    ECPropertyValueQuerySpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, ECPropertyValueQuerySpecification_LoadFromJsonFailsWhenSchemaNameIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "specType": "ECPropertyValue",
        "class": {"className": "TestClass"},
        "parentPropertyName": "parent"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    ECPropertyValueQuerySpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, ECPropertyValueQuerySpecification_LoadFromJsonFailsWhenClassNameIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "specType": "ECPropertyValue",
        "class": {"schemaName": "TestSchema"},
        "parentPropertyName": "parent"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    ECPropertyValueQuerySpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, ECPropertyValueQuerySpecification_LoadFromJsonFailsWhenParentNameIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "specType": "ECPropertyValue",
        "class": {"schemaName": "TestSchema", "className": "TestClass"}
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    ECPropertyValueQuerySpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, ECPropertyValueQuerySpecification_ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "73436b24d7cd1c9f7c33d9e61a682664";

    // Make sure that introducing additional attributes with default values don't affect the hash
    ECPropertyValueQuerySpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    ECPropertyValueQuerySpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "specType": "CustomQueryInstanceNodes",
        "groupByClass": false,
        "groupByLabel": false,
        "queries": [{
            "specType": "String",
            "class": {"schemaName": "TestSchema", "className": "TestClass"},
            "query": "QueryString"
        }, {
            "specType": "ECPropertyValue",
            "class": {"schemaName": "TestSchema", "className": "TestClass"},
            "parentPropertyName": "parent"
        }]
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    SearchResultInstanceNodesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    EXPECT_FALSE(spec.GetGroupByClass());
    EXPECT_FALSE(spec.GetGroupByLabel());
    EXPECT_EQ(2, spec.GetQuerySpecifications().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "specType": "CustomQueryInstanceNodes"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    SearchResultInstanceNodesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_TRUE(spec.GetGroupByClass());
    EXPECT_TRUE(spec.GetGroupByLabel());
    EXPECT_TRUE(spec.GetQuerySpecifications().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, WriteToJson)
    {
    SearchResultInstanceNodesSpecification spec(123, true, true, true, false, false);
    spec.AddQuerySpecification(*new StringQuerySpecification("query", "schema", "class"));
    spec.AddQuerySpecification(*new ECPropertyValueQuerySpecification("schema", "class", "prop"));
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "specType": "CustomQueryInstanceNodes",
        "priority": 123,
        "hasChildren": "Always",
        "hideNodesInHierarchy": true,
        "hideIfNoChildren": true,
        "groupByClass": false,
        "groupByLabel": false,
        "queries": [{
            "specType": "String",
            "class": {"schemaName": "schema", "className": "class"},
            "query": "query"
        }, {
            "specType": "ECPropertyValue",
            "class": {"schemaName": "schema", "className": "class"},
            "parentPropertyName": "prop"
        }]
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
            R"(<SearchResultInstances Priority="1000" HasChildren="Unknown" HideNodesInHierarchy="false" HideIfNoChildren="false" DoNotSort="false" GroupByClass="false" GroupByLabel="false">)"
                R"(<StringQuery SchemaName="TestSchema" ClassName="TestClass">TestQuery</StringQuery>)"
                R"(<PropertyValueQuery SchemaName="TestSchema" ClassName="TestClass" ParentPropertyName="TestParent"/>)"
            R"(</SearchResultInstances>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SearchResultInstanceNodesSpecificationTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "0969fc875bdcd5e6d32c5f7e78ba6643";

    // Make sure that introducing additional attributes with default values don't affect the hash
    SearchResultInstanceNodesSpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    SearchResultInstanceNodesSpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }
