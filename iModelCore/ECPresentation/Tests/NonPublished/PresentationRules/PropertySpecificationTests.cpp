/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertySpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertySpecificationTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "name": "p1",
        "overridesPriority":1,
        "isDisplayed":false,
        "doNotHideOtherPropertiesOnDisplayOverride": false,
        "editor": {
            "editorName": "custom editor"
        },
        "labelOverride": "test",
        "categoryId": "category id"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertySpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("p1", spec.GetPropertyName().c_str());
    EXPECT_TRUE(spec.IsDisplayed().IsValid());
    EXPECT_FALSE(spec.IsDisplayed().Value());
    EXPECT_STREQ("test", spec.GetLabelOverride().c_str());
    EXPECT_EQ(1, spec.GetOverridesPriority());
    ASSERT_TRUE(nullptr != spec.GetEditorOverride());
    EXPECT_STREQ("category id", spec.GetCategoryId().c_str());
    EXPECT_FALSE(spec.DoNotHideOtherPropertiesOnDisplayOverride());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertySpecificationTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "name": "p1"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    PropertySpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("p1", spec.GetPropertyName().c_str());
    EXPECT_FALSE(spec.IsDisplayed().IsValid());
    EXPECT_STREQ("", spec.GetLabelOverride().c_str());
    EXPECT_TRUE(nullptr == spec.GetEditorOverride());
    EXPECT_EQ(1000, spec.GetOverridesPriority());
    EXPECT_STREQ("", spec.GetCategoryId().c_str());
    EXPECT_FALSE(spec.DoNotHideOtherPropertiesOnDisplayOverride());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertySpecificationTests, LoadFromJsonFailsIfPropertyNameNotSpecified)
    {
    static Utf8CP jsonString = "{}";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertySpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertySpecificationTests, WriteToJson)
    {
    PropertySpecification spec("p1", 123, "custom label", "category id", true, new PropertyEditorSpecification("custom editor"), true);
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "name": "p1",
        "overridesPriority": 123,
        "labelOverride": "custom label",
        "isDisplayed": true,
        "doNotHideOtherPropertiesOnDisplayOverride": true,
        "editor": {
            "editorName": "custom editor"
        },
        "categoryId": "category id"
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ainoras.Zukauskas              03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertySpecificationTests, WriteToJsonShouldNotWriteDoNotHidePropertiesFlagWhenDisplayFalse)
{
    PropertySpecification spec("p15", 22, "test", "test category", false, new PropertyEditorSpecification("test editor"), true);
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "name": "p15",
        "overridesPriority": 22,
        "labelOverride": "test",
        "isDisplayed": false,
        "editor": {
            "editorName": "test editor"
        },
        "categoryId": "test category"
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ainoras.Zukauskas              03/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertySpecificationTests, WriteToJsonShouldNotWriteDoNotHidePropertiesFlagWhenItIsFalse)
{
    PropertySpecification spec("p15", 22, "test", "test category", true, new PropertyEditorSpecification("test editor"), false);
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "name": "p15",
        "overridesPriority": 22,
        "labelOverride": "test",
        "isDisplayed": true,
        "editor": {
            "editorName": "test editor"
        },
        "categoryId": "test category"
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
}
