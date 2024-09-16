/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyCategoryIdentifierTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyCategoryIdentifierTests, CreatesSchemaBasedIdentifier)
    {
    auto identifier = PropertyCategoryIdentifier::CreateSchemaBasedCategory("x:y");
    EXPECT_EQ(PropertyCategoryIdentifierType::SchemaCategory, identifier->GetType());
    EXPECT_TRUE(nullptr == identifier->AsIdIdentifier());
    EXPECT_TRUE(nullptr != identifier->AsSchemaCategoryIdentifier());
    EXPECT_STREQ("x:y", identifier->AsSchemaCategoryIdentifier()->GetCategoryName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct SchemaPropertyCategoryIdentifierTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaPropertyCategoryIdentifierTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "type": "SchemaCategory",
        "categoryName": "x:y"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());
    
    auto identifier = PropertyCategoryIdentifier::Create(json);
    EXPECT_EQ(PropertyCategoryIdentifierType::SchemaCategory, identifier->GetType());
    EXPECT_STREQ("x:y", identifier->AsSchemaCategoryIdentifier()->GetCategoryName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaPropertyCategoryIdentifierTests, LoadFromJsonFailsIfCategoryNameNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "type": "SchemCategory",
        "categoryName": ""
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    auto identifier = PropertyCategoryIdentifier::Create(json);
    EXPECT_TRUE(nullptr == identifier);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaPropertyCategoryIdentifierTests, WriteToJson)
    {
    auto identifier = PropertyCategoryIdentifier::CreateSchemaBasedCategory("x:y");
    BeJsDocument json = identifier->WriteJson();
    BeJsDocument expected(R"({
        "type": "SchemaCategory",
        "categoryName": "x:y"
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaPropertyCategoryIdentifierTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "7949547da980cc861a9759d0f5d900d3";

    auto identifier = PropertyCategoryIdentifier::CreateSchemaBasedCategory("");
    EXPECT_STREQ(DEFAULT_HASH, identifier->GetHash().c_str());

    // Make sure that copy has the same hash
    auto clone = identifier->Clone();
    EXPECT_STREQ(DEFAULT_HASH, clone->GetHash().c_str());

    // Test that each attribute affects the hash
    EXPECT_STRNE(DEFAULT_HASH, PropertyCategoryIdentifier::CreateSchemaBasedCategory("x")->GetHash().c_str());
    }
