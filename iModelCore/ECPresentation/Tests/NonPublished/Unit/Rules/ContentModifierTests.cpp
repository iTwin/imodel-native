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
struct ContentModifierTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentModifierTests, CopyConstructorCopiesProperties)
    {
    // Create modifier1
    ContentModifier modifier1("TestSchema", "TestClassName");
    modifier1.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, "RelationshipClassName", "RelatedClassNames", "Properties", RelationshipMeaning::RelatedInstance));
    modifier1.AddCalculatedProperty(*new CalculatedPropertiesSpecification("label", 0, "Value"));
    modifier1.AddPropertyOverride(*new PropertySpecification("property", 123, "", nullptr, nullptr));
    modifier1.AddPropertyCategory(*new PropertyCategorySpecification());

    // Validate modifier1
    EXPECT_STREQ("TestSchema", modifier1.GetSchemaName().c_str());
    EXPECT_STREQ("TestClassName", modifier1.GetClassName().c_str());
    EXPECT_EQ(1, modifier1.GetCalculatedProperties().size());
    EXPECT_EQ(1, modifier1.GetRelatedProperties().size());
    EXPECT_EQ(1, modifier1.GetPropertyOverrides().size());
    EXPECT_EQ(1, modifier1.GetPropertyCategories().size());

    // Create modifier2 via copy constructor
    ContentModifier modifier2(modifier1);

    // Validate modifier2
    EXPECT_STREQ("TestSchema", modifier2.GetSchemaName().c_str());
    EXPECT_STREQ("TestClassName", modifier2.GetClassName().c_str());
    EXPECT_EQ(1, modifier2.GetCalculatedProperties().size());
    EXPECT_EQ(1, modifier2.GetRelatedProperties().size());
    EXPECT_EQ(1, modifier2.GetPropertyOverrides().size());
    EXPECT_EQ(1, modifier2.GetPropertyCategories().size());

    // Validate properties
    EXPECT_NE(modifier1.GetCalculatedProperties()[0], modifier2.GetCalculatedProperties()[0]);
    EXPECT_NE(modifier1.GetRelatedProperties()[0], modifier2.GetRelatedProperties()[0]);
    EXPECT_NE(modifier1.GetPropertyOverrides()[0], modifier2.GetPropertyOverrides()[0]);
    EXPECT_NE(modifier1.GetPropertyCategories()[0], modifier2.GetPropertyCategories()[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentModifierTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "ContentModifier",
        "class": {"schemaName": "TestSchema", "className": "TestClassName"},
        "requiredSchemas": [{"name": "TestSchema"}],
        "relatedProperties": [{
            "propertiesSource": {
                "relationship": {"schemaName": "A", "className": "B"},
                "direction": "Forward"
            }
        }],
        "propertyCategories": [{
            "id": "my category",
            "label": "My Category"
        }],
        "propertyOverrides": [{
            "name": "property name"
        }],
        "calculatedProperties": [
            {
            "value": "value",
            "label": "label"
            }
        ],
        "applyOnNestedContent": true
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    ContentModifier modifier;
    EXPECT_TRUE(modifier.ReadJson(json));

    EXPECT_STREQ("TestSchema", modifier.GetSchemaName().c_str());
    EXPECT_STREQ("TestClassName", modifier.GetClassName().c_str());
    EXPECT_EQ(1, modifier.GetRequiredSchemaSpecifications().size());
    EXPECT_EQ(1, modifier.GetCalculatedProperties().size());
    EXPECT_EQ(1, modifier.GetRelatedProperties().size());
    EXPECT_EQ(1, modifier.GetPropertyOverrides().size());
    EXPECT_EQ(1, modifier.GetPropertyCategories().size());
    EXPECT_TRUE(modifier.ShouldApplyOnNestedContent());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentModifierTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "ContentModifier"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    ContentModifier modifier;
    EXPECT_TRUE(modifier.ReadJson(json));

    EXPECT_STREQ("", modifier.GetSchemaName().c_str());
    EXPECT_STREQ("", modifier.GetClassName().c_str());
    EXPECT_EQ(0, modifier.GetRequiredSchemaSpecifications().size());
    EXPECT_EQ(0, modifier.GetCalculatedProperties().size());
    EXPECT_EQ(0, modifier.GetRelatedProperties().size());
    EXPECT_EQ(0, modifier.GetPropertyOverrides().size());
    EXPECT_EQ(0, modifier.GetPropertyCategories().size());
    EXPECT_FALSE(modifier.ShouldApplyOnNestedContent());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentModifierTests, WriteToJson)
    {
    ContentModifier rule("schema", "class");
    rule.AddCalculatedProperty(*new CalculatedPropertiesSpecification());
    rule.AddPropertyOverride(*new PropertySpecification());
    rule.AddRelatedProperty(*new RelatedPropertiesSpecification());
    rule.AddPropertyCategory(*new PropertyCategorySpecification());
    rule.AddRequiredSchemaSpecification(*new RequiredSchemaSpecification("TestSchema"));
    rule.SetApplyOnNestedContent(true);
    BeJsDocument json = rule.WriteJson();
    BeJsDocument expected (R"({
        "ruleType": "ContentModifier",
        "class": {"schemaName": "schema", "className": "class"},
        "requiredSchemas": [{"name": "TestSchema"}],
        "calculatedProperties": [{
            "label": ""
        }],
        "relatedProperties": [{
            "properties": "_none_"
        }],
        "propertyOverrides": [{
            "name": ""
        }],
        "propertyCategories": [{
            "id": "",
            "label": ""
        }],
        "applyOnNestedContent": true
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentModifierTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "af50c6fedf0e9a740b287cc1b6a0ace8";

    // Make sure that introducing additional attributes with default values don't affect the hash
    ContentModifier defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    ContentModifier copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // Test that each attribute affects the hash
    ContentModifier specWithRequiredSchemas;
    specWithRequiredSchemas.AddRequiredSchemaSpecification(*new RequiredSchemaSpecification("Test"));
    EXPECT_STRNE(DEFAULT_HASH, specWithRequiredSchemas.GetHash().c_str());
    specWithRequiredSchemas.ClearRequiredSchemaSpecifications();
    EXPECT_STREQ(DEFAULT_HASH, specWithRequiredSchemas.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }
