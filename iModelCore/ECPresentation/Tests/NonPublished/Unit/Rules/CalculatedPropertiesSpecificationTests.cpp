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
struct CalculatedPropertiesSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CalculatedPropertiesSpecificationTests, CopyConstructor)
    {
    CalculatedPropertiesSpecification source("a", 1, "1", "string");
    source.SetEditor(new PropertyEditorSpecification());
    source.SetRenderer(new CustomRendererSpecification());
    source.SetCategoryId(PropertyCategoryIdentifier::CreateForId("a"));
    Utf8String sourceHash = source.GetHash();

    // copy
    CalculatedPropertiesSpecification target(source);
    // calculate target hash and confirm it matches source
    EXPECT_EQ(sourceHash, target.GetHash());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CalculatedPropertiesSpecificationTests, MoveConstructor)
    {
    CalculatedPropertiesSpecification source("a", 1, "1", "string");
    source.SetEditor(new PropertyEditorSpecification());
    source.SetRenderer(new CustomRendererSpecification());
    source.SetCategoryId(PropertyCategoryIdentifier::CreateForId("a"));
    Utf8String sourceHash = source.GetHash();

    // copy
    CalculatedPropertiesSpecification target(std::move(source));
    // calculate target hash and confirm it matches source
    EXPECT_EQ(sourceHash, target.GetHash());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CalculatedPropertiesSpecificationTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "label": "calculated property",
        "value": "calculated value",
        "type": "string",
        "renderer": {
            "rendererName": "custom renderer"
        },
        "editor": {
            "editorName": "custom editor"
        },
        "categoryId": "categoryId",
        "priority": 10,
        "extendedData": {
            "extendedData1": "2+2"
        }
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    CalculatedPropertiesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("calculated property", spec.GetLabel().c_str());
    EXPECT_STREQ("calculated value", spec.GetValue().c_str());
    EXPECT_STREQ("string", spec.GetType().c_str());
    EXPECT_STREQ("2+2", spec.GetExtendedDataMap().find("extendedData1")->second.c_str());
    EXPECT_STREQ("custom renderer", spec.GetRenderer()->GetRendererName().c_str());
    EXPECT_STREQ("custom editor", spec.GetEditor()->GetEditorName().c_str());
    EXPECT_STREQ("categoryId", spec.GetCategoryId()->AsIdIdentifier()->GetCategoryId().c_str());
    EXPECT_EQ(10, spec.GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CalculatedPropertiesSpecificationTests, LoadFromJsonFailsWhenTypeIsNotSupported)
    {
    static Utf8CP jsonString = R"({
        "label": "calculated property",
        "type": "binary"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    CalculatedPropertiesSpecification rule;
    EXPECT_FALSE(rule.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CalculatedPropertiesSpecificationTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "label": "calculated property"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    CalculatedPropertiesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("calculated property", spec.GetLabel().c_str());
    EXPECT_EQ("", spec.GetValue());
    EXPECT_EQ("", spec.GetType());
    EXPECT_EQ(nullptr, spec.GetRenderer());
    EXPECT_EQ(nullptr, spec.GetEditor());
    EXPECT_EQ(nullptr, spec.GetCategoryId());
    EXPECT_FALSE(spec.GetExtendedDataMap().size() > 0);
    EXPECT_EQ(1000, spec.GetPriority());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CalculatedPropertiesSpecificationTests, WriteToJson)
    {
    CalculatedPropertiesSpecification spec("custom label", 123, "custom value", "string");
    spec.SetRenderer(new CustomRendererSpecification("custom renderer"));
    spec.SetEditor(new PropertyEditorSpecification("custom editor"));
    spec.SetCategoryId(PropertyCategoryIdentifier::CreateForId("category id"));
    spec.AddExtendedData("extendedDataVal", "someValue");
    BeJsDocument json = spec.WriteJson();
    BeJsDocument expected(R"({
        "label": "custom label",
        "priority": 123,
        "value": "custom value",
        "type": "string",
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
        "extendedData": {
            "extendedDataVal": "someValue"
        }
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CalculatedPropertiesSpecificationTests, WriteToJsonWithDefaultValues)
    {
    CalculatedPropertiesSpecification spec("custom label", 123, "");
    BeJsDocument json = spec.WriteJson();
    BeJsDocument expected(R"({
        "label": "custom label",
        "priority": 123
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }


/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CalculatedPropertiesSpecificationTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "1557340c4b482efb431717c6ffdd867b";

    // first we make sure that introducing additional attributes with default values don't affect the hash
    CalculatedPropertiesSpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // then we make sure that copy has the same hash
    CalculatedPropertiesSpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // then we test that each attribute affects the hash
    CalculatedPropertiesSpecification specWithPropertyName(defaultSpec);
    specWithPropertyName.SetLabel("a");
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithPropertyName.GetHash().c_str());
    specWithPropertyName.SetLabel("");
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithPropertyName.GetHash().c_str());

    CalculatedPropertiesSpecification specWithLabelOverride(defaultSpec);
    specWithLabelOverride.SetValue("10");
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithLabelOverride.GetHash().c_str());
    specWithLabelOverride.SetValue("");
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithLabelOverride.GetHash().c_str());

    CalculatedPropertiesSpecification specWithTypeOverride(defaultSpec);
    specWithTypeOverride.SetType("string");
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithTypeOverride.GetHash().c_str());
    specWithTypeOverride.SetType("");
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithTypeOverride.GetHash().c_str());

    CalculatedPropertiesSpecification specWithExtendedDataOverride(defaultSpec);
    bmap<Utf8String, Utf8String> extendedData1;
    extendedData1.Insert("extendedDataVal", "val1");
    specWithExtendedDataOverride.SetExtendedDataMap(extendedData1);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithExtendedDataOverride.GetHash().c_str());
    bmap<Utf8String, Utf8String> extendedData2;
    specWithExtendedDataOverride.SetExtendedDataMap(extendedData2);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithExtendedDataOverride.GetHash().c_str());

    CalculatedPropertiesSpecification specWithRendererOverride(defaultSpec);
    specWithRendererOverride.SetRenderer(new CustomRendererSpecification());
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithRendererOverride.GetHash().c_str());
    specWithRendererOverride.SetRenderer(nullptr);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithRendererOverride.GetHash().c_str());

    CalculatedPropertiesSpecification specWithEditorOverride(defaultSpec);
    specWithEditorOverride.SetEditor(new PropertyEditorSpecification());
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithEditorOverride.GetHash().c_str());
    specWithEditorOverride.SetEditor(nullptr);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithEditorOverride.GetHash().c_str());

    CalculatedPropertiesSpecification specWithCategoryId(defaultSpec);
    specWithCategoryId.SetCategoryId(PropertyCategoryIdentifier::CreateForId("a"));
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithCategoryId.GetHash().c_str());
    specWithCategoryId.SetCategoryId(nullptr);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithCategoryId.GetHash().c_str());

    CalculatedPropertiesSpecification specWithPriorityOverride(defaultSpec);
    specWithPriorityOverride.SetPriority(10);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithPriorityOverride.GetHash().c_str());
    specWithPriorityOverride.SetPriority(1000);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithPriorityOverride.GetHash().c_str());
    }