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
    BeJsDocument json(jsonString);
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
    BeJsDocument json(jsonString);
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
    BeJsDocument json(jsonString);
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
    BeJsDocument json(jsonString);
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
    BeJsDocument json(jsonString);
    ASSERT_FALSE(json.isNull());

    StringQuerySpecification spec;
    ASSERT_FALSE(spec.ReadJson(json));
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
    BeJsDocument json(jsonString);
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
    BeJsDocument json(jsonString);
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
    BeJsDocument json(jsonString);
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
    BeJsDocument json(jsonString);
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
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    ECPropertyValueQuerySpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
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
    BeJsDocument json(jsonString);
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
    BeJsDocument json(jsonString);
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
    BeJsDocument json = spec.WriteJson();
    BeJsDocument  expected(R"({
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
    EXPECT_TRUE(expected.isExactEqual(json));
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
