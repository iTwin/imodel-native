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
struct PropertySpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertySpecificationTests, CopyConstructor)
    {
    // build a custom PropertySpecification
    auto source = new PropertySpecification();
    source->SetPropertyName("a");
    source->SetOverridesPriority(999);
    source->SetLabelOverride("a");
    source->SetIsDisplayed(true);
    source->SetDoNotHideOtherPropertiesOnDisplayOverride(true);
    source->SetRendererOverride(new CustomRendererSpecification());
    source->SetEditorOverride(new PropertyEditorSpecification());
    source->SetCategoryId(PropertyCategoryIdentifier::CreateForId("a"));
    source->SetIsReadOnly(true);
    source->SetPriority(10);
    Utf8String sourceHash = source->GetHash();

    // copy
    PropertySpecification target(*source);

    // delete source
    DELETE_AND_CLEAR(source);

    // calculate target hash and confirm it matches source
    EXPECT_EQ(sourceHash, target.GetHash());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertySpecificationTests, MoveConstructor)
    {
    // build a custom PropertySpecification
    auto source = new PropertySpecification();
    source->SetPropertyName("a");
    source->SetOverridesPriority(999);
    source->SetLabelOverride("a");
    source->SetIsDisplayed(true);
    source->SetDoNotHideOtherPropertiesOnDisplayOverride(true);
    source->SetRendererOverride(new CustomRendererSpecification());
    source->SetEditorOverride(new PropertyEditorSpecification());
    source->SetCategoryId(PropertyCategoryIdentifier::CreateForId("a"));
    source->SetIsReadOnly(true);
    source->SetPriority(10);
    Utf8String sourceHash = source->GetHash();

    // move
    PropertySpecification target(std::move(*source));

    // delete source
    DELETE_AND_CLEAR(source);

    // calculate target hash and confirm it matches source
    EXPECT_EQ(sourceHash, target.GetHash());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertySpecificationTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "name": "p1",
        "overridesPriority":1,
        "isDisplayed":false,
        "doNotHideOtherPropertiesOnDisplayOverride": false,
        "renderer": {
            "rendererName": "custom renderer"
        },
        "editor": {
            "editorName": "custom editor"
        },
        "labelOverride": "test",
        "categoryId": "category id",
        "isReadOnly": false,
        "priority": 10
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertySpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("p1", spec.GetPropertyName().c_str());
    EXPECT_TRUE(spec.IsDisplayed().IsValid());
    EXPECT_TRUE(spec.IsDisplayed().IsBoolean());
    EXPECT_FALSE(spec.IsDisplayed().GetBoolean());
    EXPECT_STREQ("test", spec.GetLabelOverride().c_str());
    EXPECT_EQ(1, spec.GetOverridesPriority());
    ASSERT_NE(nullptr, spec.GetRendererOverride());
    ASSERT_NE(nullptr, spec.GetEditorOverride());
    ASSERT_NE(nullptr, spec.GetCategoryId());
    EXPECT_STREQ("category id", spec.GetCategoryId()->AsIdIdentifier()->GetCategoryId().c_str());
    EXPECT_FALSE(spec.DoNotHideOtherPropertiesOnDisplayOverride());
    EXPECT_TRUE(spec.IsReadOnly().IsValid());
    EXPECT_FALSE(spec.IsReadOnly().Value());
    EXPECT_TRUE(spec.GetPriority().IsValid());
    EXPECT_EQ(10, spec.GetPriority().Value());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertySpecificationTests, LoadsFromJsonIsDisplayedStringValue)
    {
    static Utf8CP jsonString = R"({
        "name": "p1",
        "isDisplayed":"ECExpression"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    PropertySpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("p1", spec.GetPropertyName().c_str());
    EXPECT_TRUE(spec.IsDisplayed().IsString());
    EXPECT_STREQ("ECExpression", spec.IsDisplayed().GetString().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    EXPECT_EQ(nullptr, spec.GetRendererOverride());
    EXPECT_EQ(nullptr, spec.GetEditorOverride());
    EXPECT_EQ(1000, spec.GetOverridesPriority());
    EXPECT_EQ(nullptr, spec.GetCategoryId());
    EXPECT_FALSE(spec.DoNotHideOtherPropertiesOnDisplayOverride());
    EXPECT_FALSE(spec.IsReadOnly().IsValid());
    EXPECT_FALSE(spec.GetPriority().IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertySpecificationTests, WriteToJson)
    {
    PropertySpecification spec("p1", 123, "custom label", PropertyCategoryIdentifier::CreateForId("category id"), true,
        new CustomRendererSpecification("custom renderer"), new PropertyEditorSpecification("custom editor"), true, true, 10);
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "name": "p1",
        "overridesPriority": 123,
        "labelOverride": "custom label",
        "isDisplayed": true,
        "doNotHideOtherPropertiesOnDisplayOverride": true,
        "renderer": {
            "rendererName": "custom renderer"
        },
        "editor": {
            "editorName": "custom editor"
        },
        "categoryId": {
            "type": "Id",
            "categoryId": "category id"
        },
        "isReadOnly": true,
        "priority": 10
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertySpecificationTests, WriteToJsonShouldNotWriteDoNotHidePropertiesFlagWhenDisplayFalse)
    {
    PropertySpecification spec("p15", 22, "test", nullptr, false, nullptr, nullptr, true);
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "name": "p15",
        "overridesPriority": 22,
        "labelOverride": "test",
        "isDisplayed": false
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertySpecificationTests, WriteToJsonShouldNotWriteDoNotHidePropertiesFlagWhenItIsFalse)
    {
    PropertySpecification spec("p15", 22, "test", nullptr, true, nullptr, nullptr, false);
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "name": "p15",
        "overridesPriority": 22,
        "labelOverride": "test",
        "isDisplayed": true
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertySpecificationTests, WriteToJsonIsDisplayedStringValue)
    {
    PropertySpecification spec("p15", 1000, "", nullptr, "ECExpression");
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "name": "p15",
        "isDisplayed": "ECExpression"
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertySpecificationTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "979d884fb7ab404a6380840351319a62";

    // first we make sure that introducing additional attributes with default values don't affect the hash
    PropertySpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // then we make sure that copy has the same hash
    PropertySpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // then we test that each attribute affects the hash
    PropertySpecification specWithPropertyName(defaultSpec);
    specWithPropertyName.SetPropertyName("a");
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithPropertyName.GetHash().c_str());
    specWithPropertyName.SetPropertyName("");
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithPropertyName.GetHash().c_str());

    PropertySpecification specWithOverridesPriority(defaultSpec);
    specWithOverridesPriority.SetOverridesPriority(999);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithOverridesPriority.GetHash().c_str());
    specWithOverridesPriority.SetOverridesPriority(1000);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithOverridesPriority.GetHash().c_str());

    PropertySpecification specWithLabelOverride(defaultSpec);
    specWithLabelOverride.SetLabelOverride("a");
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithLabelOverride.GetHash().c_str());
    specWithLabelOverride.SetLabelOverride("");
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithLabelOverride.GetHash().c_str());

    PropertySpecification specWithIsDisplayedOverride(defaultSpec);
    specWithIsDisplayedOverride.SetIsDisplayed(true);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithIsDisplayedOverride.GetHash().c_str());
    specWithIsDisplayedOverride.SetIsDisplayed("value");
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithIsDisplayedOverride.GetHash().c_str());
    specWithIsDisplayedOverride.SetIsDisplayed(nullptr);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithIsDisplayedOverride.GetHash().c_str());

    PropertySpecification specWithDoNotHideOtherPropertiesOnDisplayOverride(defaultSpec);
    specWithDoNotHideOtherPropertiesOnDisplayOverride.SetDoNotHideOtherPropertiesOnDisplayOverride(true);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithDoNotHideOtherPropertiesOnDisplayOverride.GetHash().c_str());
    specWithDoNotHideOtherPropertiesOnDisplayOverride.SetDoNotHideOtherPropertiesOnDisplayOverride(false);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithDoNotHideOtherPropertiesOnDisplayOverride.GetHash().c_str());

    PropertySpecification specWithRendererOverride(defaultSpec);
    specWithRendererOverride.SetRendererOverride(new CustomRendererSpecification());
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithRendererOverride.GetHash().c_str());
    specWithRendererOverride.SetRendererOverride(nullptr);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithRendererOverride.GetHash().c_str());

    PropertySpecification specWithEditorOverride(defaultSpec);
    specWithEditorOverride.SetEditorOverride(new PropertyEditorSpecification());
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithEditorOverride.GetHash().c_str());
    specWithEditorOverride.SetEditorOverride(nullptr);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithEditorOverride.GetHash().c_str());

    PropertySpecification specWithCategoryId(defaultSpec);
    specWithCategoryId.SetCategoryId(PropertyCategoryIdentifier::CreateForId("a"));
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithCategoryId.GetHash().c_str());
    specWithCategoryId.SetCategoryId(nullptr);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithCategoryId.GetHash().c_str());

    PropertySpecification specWithIsReadOnlyOverride(defaultSpec);
    specWithIsReadOnlyOverride.SetIsReadOnly(true);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithIsReadOnlyOverride.GetHash().c_str());
    specWithIsReadOnlyOverride.SetIsReadOnly(nullptr);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithIsReadOnlyOverride.GetHash().c_str());

    PropertySpecification specWithPriorityOverride(defaultSpec);
    specWithPriorityOverride.SetPriority(10);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithPriorityOverride.GetHash().c_str());
    specWithPriorityOverride.SetPriority(nullptr);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithPriorityOverride.GetHash().c_str());
    }
