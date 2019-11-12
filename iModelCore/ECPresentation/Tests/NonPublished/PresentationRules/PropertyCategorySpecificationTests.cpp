/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyCategorySpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyCategorySpecificationTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "id": "test id",
        "label": "test label",
        "description": "test description",
        "priority": 999,
        "autoExpand": true
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertyCategorySpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("test id", spec.GetId().c_str());
    EXPECT_STREQ("test label", spec.GetLabel().c_str());
    EXPECT_STREQ("test description", spec.GetDescription().c_str());
    EXPECT_EQ(999, spec.GetPriority());
    EXPECT_EQ(true, spec.ShouldAutoExpand());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2019
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
    EXPECT_STREQ("test label", spec.GetLabel().c_str());
    EXPECT_STREQ("", spec.GetDescription().c_str());
    EXPECT_EQ(1000, spec.GetPriority());
    EXPECT_EQ(false, spec.ShouldAutoExpand());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2019
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
* @bsiclass                                     Grigas.Petraitis                10/2019
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
* @bsiclass                                     Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyCategorySpecificationTests, WriteToJson)
    {
    PropertyCategorySpecification spec("test id", "test label", "test description", 123, true);
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "id": "test id",
        "label": "test label",
        "description": "test description",
        "priority": 123,
        "autoExpand": true
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }
