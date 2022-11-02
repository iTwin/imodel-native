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
struct PropertyCategorySpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyCategorySpecificationTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "id": "test id",
        "parentId": "test parent id",
        "label": "test label",
        "description": "test description",
        "priority": 999,
        "autoExpand": true,
        "renderer": {
            "rendererName": "test renderer"
        }
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertyCategorySpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("test id", spec.GetId().c_str());
    ASSERT_NE(nullptr, spec.GetParentId());
    EXPECT_STREQ("test parent id", spec.GetParentId()->AsIdIdentifier()->GetCategoryId().c_str());
    EXPECT_STREQ("test label", spec.GetLabel().c_str());
    EXPECT_STREQ("test description", spec.GetDescription().c_str());
    EXPECT_EQ(999, spec.GetPriority());
    EXPECT_EQ(true, spec.ShouldAutoExpand());
    EXPECT_STREQ("test renderer", spec.GetRendererOverride()->GetRendererName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyCategorySpecificationTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "id": "test id",
        "label": "test label"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    PropertyCategorySpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("test id", spec.GetId().c_str());
    EXPECT_EQ(PropertyCategoryIdentifierType::DefaultParent, spec.GetParentId()->GetType());
    EXPECT_STREQ("test label", spec.GetLabel().c_str());
    EXPECT_STREQ("", spec.GetDescription().c_str());
    EXPECT_EQ(1000, spec.GetPriority());
    EXPECT_EQ(false, spec.ShouldAutoExpand());
    EXPECT_EQ(nullptr, spec.GetRendererOverride());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyCategorySpecificationTests, LoadFromJsonFailsIfIdNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "label": "test label"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertyCategorySpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyCategorySpecificationTests, LoadFromJsonFailsLabelIdNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "id": "test id"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertyCategorySpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyCategorySpecificationTests, WriteToJson)
    {
    PropertyCategorySpecification spec("test id", "test label", "test description", 123, true, std::make_unique<CustomRendererSpecification>("test renderer"),
        PropertyCategoryIdentifier::CreateForId("test parent id"));
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "id": "test id",
        "parentId": {
            "type": "Id",
            "categoryId": "test parent id"
        },
        "label": "test label",
        "description": "test description",
        "priority": 123,
        "autoExpand": true,
        "renderer": {
            "rendererName": "test renderer"
        }
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyCategorySpecificationTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "6fffc84a20c7e0679d2d146a29b666d1";

    // Make sure that introducing additional attributes with default values don't affect the hash
    PropertyCategorySpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    PropertyCategorySpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // Test that each attribute affects the hash
    PropertyCategorySpecification specWithParentId(defaultSpec);
    specWithParentId.SetParentId(PropertyCategoryIdentifier::CreateForId("test"));
    EXPECT_STRNE(DEFAULT_HASH, specWithParentId.GetHash().c_str());
    specWithParentId.SetParentId(nullptr);
    EXPECT_STREQ(DEFAULT_HASH, specWithParentId.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }
