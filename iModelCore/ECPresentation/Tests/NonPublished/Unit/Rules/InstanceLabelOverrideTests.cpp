/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceLabelOverrideTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "InstanceLabelOverride",
        "class": {"schemaName": "TestSchema", "className": "TestClass"},
        "values": [{
            "specType": "Composite",
            "separator": "-",
            "parts": [{
                "spec": {
                    "specType": "Property",
                    "propertyName": "prop1"
                }
            }, {
                "spec": {
                    "specType": "BriefcaseId"
                },
                "isRequired": true
            }]
        }, {
            "specType": "Property",
            "propertyName": "prop2"
        }, {
            "specType": "ClassName",
            "full": true
        }, {
            "specType": "ClassLabel"
        }, {
            "specType": "BriefcaseId"
        }, {
            "specType": "LocalId"
        }, {
            "specType": "String",
            "value": "test"
        }]
    })";
    BeJsDocument json(jsonString);
    ASSERT_FALSE(json.isNull());

    InstanceLabelOverride rule;

    ASSERT_TRUE(rule.ReadJson(json));
    EXPECT_STREQ("TestSchema:TestClass", rule.GetClassName().c_str());
    ASSERT_EQ(7, rule.GetValueSpecifications().size());

    InstanceLabelOverrideCompositeValueSpecification* s1 = dynamic_cast<InstanceLabelOverrideCompositeValueSpecification*>(rule.GetValueSpecifications()[0]);
    ASSERT_TRUE(nullptr != s1);
    EXPECT_STREQ("-", s1->GetSeparator().c_str());
    ASSERT_EQ(2, s1->GetValueParts().size());
    EXPECT_TRUE(nullptr != dynamic_cast<InstanceLabelOverridePropertyValueSpecification*>(s1->GetValueParts()[0]->GetSpecification()));
    EXPECT_FALSE(s1->GetValueParts()[0]->IsRequired());
    EXPECT_TRUE(nullptr != dynamic_cast<InstanceLabelOverrideBriefcaseIdValueSpecification*>(s1->GetValueParts()[1]->GetSpecification()));
    EXPECT_TRUE(s1->GetValueParts()[1]->IsRequired());

    InstanceLabelOverridePropertyValueSpecification* s2 = dynamic_cast<InstanceLabelOverridePropertyValueSpecification*>(rule.GetValueSpecifications()[1]);
    ASSERT_TRUE(nullptr != s2);
    EXPECT_STREQ("prop2", s2->GetPropertyName().c_str());

    InstanceLabelOverrideClassNameValueSpecification* s3 = dynamic_cast<InstanceLabelOverrideClassNameValueSpecification*>(rule.GetValueSpecifications()[2]);
    ASSERT_TRUE(nullptr != s3);
    EXPECT_TRUE(s3->ShouldUseFullName());

    ASSERT_TRUE(nullptr != dynamic_cast<InstanceLabelOverrideClassLabelValueSpecification*>(rule.GetValueSpecifications()[3]));

    ASSERT_TRUE(nullptr != dynamic_cast<InstanceLabelOverrideBriefcaseIdValueSpecification*>(rule.GetValueSpecifications()[4]));

    ASSERT_TRUE(nullptr != dynamic_cast<InstanceLabelOverrideLocalIdValueSpecification*>(rule.GetValueSpecifications()[5]));

    InstanceLabelOverrideStringValueSpecification* s7 = dynamic_cast<InstanceLabelOverrideStringValueSpecification*>(rule.GetValueSpecifications()[6]);
    ASSERT_TRUE(nullptr != s7);
    EXPECT_STREQ("test", s7->GetValue().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideTests, WriteToJson)
    {
    bvector<InstanceLabelOverrideValueSpecification*> specs;
    specs.push_back(new InstanceLabelOverrideCompositeValueSpecification(
        {
        new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverridePropertyValueSpecification("prop1")),
        new InstanceLabelOverrideCompositeValueSpecification::Part(*new InstanceLabelOverrideLocalIdValueSpecification(), true),
        }, ","));
    specs.push_back(new InstanceLabelOverridePropertyValueSpecification(" prop2   "));
    specs.push_back(new InstanceLabelOverrideClassNameValueSpecification(true));
    specs.push_back(new InstanceLabelOverrideClassLabelValueSpecification());
    specs.push_back(new InstanceLabelOverrideBriefcaseIdValueSpecification());
    specs.push_back(new InstanceLabelOverrideLocalIdValueSpecification());
    specs.push_back(new InstanceLabelOverrideStringValueSpecification(" test "));
    InstanceLabelOverride rule(123, true, "s:c", specs);

    BeJsDocument json = rule.WriteJson();
    BeJsDocument expected(R"({
        "ruleType": "InstanceLabelOverride",
        "priority": 123,
        "onlyIfNotHandled": true,
        "class": {"schemaName": "s", "className": "c"},
        "values": [{
            "specType": "Composite",
            "separator": ",",
            "parts": [{
                "spec": {
                    "specType": "Property",
                    "propertyName": "prop1"
                }
            }, {
                "spec": {
                    "specType": "LocalId"
                },
                "isRequired": true
            }]
        }, {
            "specType": "Property",
            "propertyName": "prop2"
        }, {
            "specType": "ClassName",
            "full": true
        }, {
            "specType": "ClassLabel"
        }, {
            "specType": "BriefcaseId"
        }, {
            "specType": "LocalId"
        }, {
            "specType": "String",
            "value": " test "
        }]
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(InstanceLabelOverrideTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "b40dc190a5d2fe9ba58d6824b87d38ab";

    // Make sure that introducing additional attributes with default values don't affect the hash
    InstanceLabelOverride defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    InstanceLabelOverride copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }
