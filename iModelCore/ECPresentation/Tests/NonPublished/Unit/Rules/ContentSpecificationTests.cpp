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
struct ContentSpecificationsTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestContentSpecification : ContentSpecification
    {
    Utf8CP _GetJsonElementType() const override {return "testSpecification";}
    bool _ReadJson(BeJsConst json) override {return ContentSpecification::_ReadJson(json);}
    void _WriteJson(BeJsValue json) const override {ContentSpecification::_WriteJson(json);}
    ContentSpecification* _Clone() const override {return new TestContentSpecification(*this);}
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentSpecificationsTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "specType": "testSpecification",
        "priority": 123,
        "showImages": true,
        "onlyIfNotHandled": true,
        "relatedProperties": [{
            "propertiesSource": {
                "relationship": {"schemaName": "A", "className": "B"},
                "direction": "Forward"
            }
        }],
        "calculatedProperties": [{
            "value": "value",
            "label": "label"
        }, {
            "value": "value",
            "label": "label"
        }],
        "propertyCategories": [{
            "id": "category id",
            "label": "category label"
        }],
        "propertyOverrides": [{
            "name": "property name"
        }],
        "relatedInstances": [{
            "relationship": {"schemaName": "TestSchema", "className": "TestRelName"},
            "class": {"schemaName": "TestSchema", "className": "TestClassName"},
            "alias":"TestAlias"
        }]
    })";
    BeJsDocument json(jsonString);
    ASSERT_FALSE(json.isNull());

    TestContentSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    EXPECT_EQ(123, spec.GetPriority());
    EXPECT_TRUE(spec.GetShowImages());
    EXPECT_EQ(2, spec.GetCalculatedProperties().size());
    EXPECT_EQ(1, spec.GetPropertyOverrides().size());
    EXPECT_EQ(1, spec.GetRelatedProperties().size());
    EXPECT_EQ(1, spec.GetRelatedInstances().size());
    EXPECT_EQ(1, spec.GetPropertyCategories().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentSpecificationsTests, LoadFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "specType": "testSpecification"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    TestContentSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));

    EXPECT_FALSE(spec.GetShowImages());
    EXPECT_TRUE(spec.GetCalculatedProperties().empty());
    EXPECT_TRUE(spec.GetPropertyOverrides().empty());
    EXPECT_TRUE(spec.GetRelatedProperties().empty());
    EXPECT_TRUE(spec.GetRelatedInstances().empty());
    EXPECT_TRUE(spec.GetPropertyCategories().empty());
    EXPECT_FALSE(spec.GetOnlyIfNotHandled());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentSpecificationsTests, WriteToJson)
    {
    TestContentSpecification spec;
    spec.SetShowImages(true);
    spec.SetOnlyIfNotHandled(true);
    spec.AddCalculatedProperty(*new CalculatedPropertiesSpecification("label", 456, "value"));
    spec.AddPropertyCategory(*new PropertyCategorySpecification("category id", "category label"));
    spec.AddPropertyOverride(*new PropertySpecification("prop1", 456, "", nullptr, nullptr));
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Both, "s1:c1", "s2:c2", "alias", true));
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification());
    BeJsDocument json = spec.WriteJson();
    BeJsDocument expected(R"({
        "specType": "testSpecification",
        "showImages": true,
        "onlyIfNotHandled": true,
        "calculatedProperties": [{
            "priority": 456,
            "value": "value",
            "label": "label"
        }],
        "propertyCategories": [{
            "id": "category id",
            "label": "category label"
        }],
        "propertyOverrides": [{
            "name": "prop1",
            "overridesPriority": 456
        }],
        "relatedInstances": [{
            "relationshipPath": {
                "relationship": {"schemaName": "s1", "className": "c1"},
                "direction" : "Both",
                "targetClass": {"schemaName": "s2", "className": "c2"}
            },
            "alias": "alias",
            "isRequired": true
        }],
        "relatedProperties": [{
            "properties": "_none_"
        }]
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentSpecificationsTests, CopiedSpecificationHasSameNestedSpecifications)
    {
    // Create spec1
    TestContentSpecification spec1;
    spec1.SetShowImages(true);
    spec1.SetOnlyIfNotHandled(true);
    spec1.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, "RelationshipClassName",
        "RelatedClassNames", "Properties", RelationshipMeaning::SameInstance));
    spec1.AddPropertyCategory(*new PropertyCategorySpecification());
    spec1.AddPropertyOverride(*new PropertySpecification("Property1", 132, "", nullptr, nullptr));
    spec1.AddCalculatedProperty(*new CalculatedPropertiesSpecification("Label1", 123, "Expression1"));

    // Validate spec1
    EXPECT_TRUE(spec1.GetShowImages());
    EXPECT_TRUE(spec1.GetOnlyIfNotHandled());
    EXPECT_EQ(1, spec1.GetRelatedProperties().size());
    EXPECT_EQ(1, spec1.GetCalculatedProperties().size());
    EXPECT_EQ(1, spec1.GetPropertyOverrides().size());
    EXPECT_EQ(1, spec1.GetPropertyCategories().size());

    // Create spec2 via copy constructor
    TestContentSpecification spec2(spec1);

    // Validate spec2
    EXPECT_TRUE(spec2.GetShowImages());
    EXPECT_TRUE(spec2.GetOnlyIfNotHandled());
    EXPECT_EQ(1, spec2.GetRelatedProperties().size());
    EXPECT_EQ(1, spec2.GetCalculatedProperties().size());
    EXPECT_EQ(1, spec2.GetPropertyOverrides().size());
    EXPECT_EQ(1, spec2.GetPropertyCategories().size());

    // Validate specifications pointers
    EXPECT_NE(spec1.GetRelatedProperties()[0], spec2.GetRelatedProperties()[0]);
    EXPECT_NE(spec1.GetCalculatedProperties()[0], spec2.GetCalculatedProperties()[0]);
    EXPECT_NE(spec1.GetPropertyOverrides()[0], spec2.GetPropertyOverrides()[0]);
    EXPECT_NE(spec1.GetPropertyCategories()[0], spec2.GetPropertyCategories()[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentSpecificationsTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "147735ebe84421fa849da398c3423bed";

    // Make sure that introducing additional attributes with default values don't affect the hash
    TestContentSpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    TestContentSpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // Test that each attribute affects the hash
    TestContentSpecification specWithShowImages(defaultSpec);
    specWithShowImages.SetShowImages(true);
    EXPECT_STRNE(DEFAULT_HASH, specWithShowImages.GetHash().c_str());
    specWithShowImages.SetShowImages(false);
    EXPECT_STREQ(DEFAULT_HASH, specWithShowImages.GetHash().c_str());

    TestContentSpecification specWithRelatedInstances(defaultSpec);
    specWithRelatedInstances.AddRelatedInstance(*new RelatedInstanceSpecification());
    EXPECT_STRNE(DEFAULT_HASH, specWithRelatedInstances.GetHash().c_str());
    specWithRelatedInstances.ClearRelatedInstances();
    EXPECT_STREQ(DEFAULT_HASH, specWithRelatedInstances.GetHash().c_str());

    TestContentSpecification specWithRelatedProperty(defaultSpec);
    specWithRelatedProperty.AddRelatedProperty(*new RelatedPropertiesSpecification());
    EXPECT_STRNE(DEFAULT_HASH, specWithRelatedProperty.GetHash().c_str());
    specWithRelatedProperty.ClearRelatedProperties();
    EXPECT_STREQ(DEFAULT_HASH, specWithRelatedProperty.GetHash().c_str());

    TestContentSpecification specWithCalculatedProperty(defaultSpec);
    specWithCalculatedProperty.AddCalculatedProperty(*new CalculatedPropertiesSpecification());
    EXPECT_STRNE(DEFAULT_HASH, specWithCalculatedProperty.GetHash().c_str());
    specWithCalculatedProperty.ClearCalculatedProperties();
    EXPECT_STREQ(DEFAULT_HASH, specWithCalculatedProperty.GetHash().c_str());

    TestContentSpecification specWithPropertyCategories(defaultSpec);
    specWithPropertyCategories.AddPropertyCategory(*new PropertyCategorySpecification());
    EXPECT_STRNE(DEFAULT_HASH, specWithPropertyCategories.GetHash().c_str());
    specWithPropertyCategories.ClearPropertyCategories();
    EXPECT_STREQ(DEFAULT_HASH, specWithPropertyCategories.GetHash().c_str());

    TestContentSpecification specWithPropertyOverrides(defaultSpec);
    specWithPropertyOverrides.AddPropertyOverride(*new PropertySpecification());
    EXPECT_STRNE(DEFAULT_HASH, specWithPropertyOverrides.GetHash().c_str());
    specWithPropertyOverrides.ClearPropertyOverrides();
    EXPECT_STREQ(DEFAULT_HASH, specWithPropertyOverrides.GetHash().c_str());

    TestContentSpecification specWithOnlyIfNotHandled(defaultSpec);
    specWithOnlyIfNotHandled.SetOnlyIfNotHandled(true);
    EXPECT_STRNE(DEFAULT_HASH, specWithOnlyIfNotHandled.GetHash().c_str());
    specWithOnlyIfNotHandled.SetOnlyIfNotHandled(false);
    EXPECT_STREQ(DEFAULT_HASH, specWithOnlyIfNotHandled.GetHash().c_str());
    }
