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
struct AllRelatedInstanceNodesSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AllRelatedInstanceNodesSpecificationTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "specType": "AllRelatedInstanceNodes",
        "groupByClass": false,
        "groupByLabel": false,
        "skipRelatedLevel": 3,
        "supportedSchemas": {"schemaNames":["TestSchema"]},
        "requiredDirection": "Forward"
    })";
    BeJsDocument json(jsonString);
    ASSERT_FALSE(json.isNull());

    AllRelatedInstanceNodesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    EXPECT_FALSE(spec.GetGroupByClass());
    EXPECT_FALSE(spec.GetGroupByLabel());
    EXPECT_EQ(3, spec.GetSkipRelatedLevel());
    EXPECT_STREQ("TestSchema", spec.GetSupportedSchemas().c_str());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Forward, spec.GetRequiredRelationDirection());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AllRelatedInstanceNodesSpecificationTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "specType": "AllRelatedInstanceNodes"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    AllRelatedInstanceNodesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_TRUE(spec.GetGroupByClass());
    EXPECT_TRUE(spec.GetGroupByLabel());
    EXPECT_EQ(0, spec.GetSkipRelatedLevel());
    EXPECT_STREQ("", spec.GetSupportedSchemas().c_str());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Both, spec.GetRequiredRelationDirection());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AllRelatedInstanceNodesSpecificationTests, WriteToJson)
    {
    AllRelatedInstanceNodesSpecification spec(123, ChildrenHint::Always, true, true, true, false, 5, "schema1, schema2");
    spec.SetRequiredRelationDirection(RequiredRelationDirection::RequiredRelationDirection_Forward);
    BeJsDocument json = spec.WriteJson();
    BeJsDocument expected(R"({
        "specType": "AllRelatedInstanceNodes",
        "priority": 123,
        "hasChildren": "Always",
        "hideNodesInHierarchy": true,
        "hideIfNoChildren": true,
        "groupByLabel": false,
        "requiredDirection": "Forward",
        "skipRelatedLevel": 5,
        "supportedSchemas": {"schemaNames": ["schema1", "schema2"]}
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AllRelatedInstanceNodesSpecificationTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "2073dd9d003d50bcb17c352c3e0152f7";

    // Make sure that introducing additional attributes with default values don't affect the hash
    AllRelatedInstanceNodesSpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    AllRelatedInstanceNodesSpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }
