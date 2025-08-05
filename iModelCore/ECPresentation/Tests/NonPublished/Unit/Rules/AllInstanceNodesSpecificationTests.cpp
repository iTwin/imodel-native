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
struct AllInstanceNodesSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AllInstanceNodesSpecificationTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "specType": "AllInstanceNodes",
        "groupByClass": false,
        "groupByLabel": false,
        "hideExpression": "hide expr",
        "supportedSchemas": {"schemaNames": ["TestSchema"]},
        "suppressSimilarAncestorsCheck": true
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    AllInstanceNodesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_FALSE(spec.GetGroupByClass());
    EXPECT_FALSE(spec.GetGroupByLabel());
    EXPECT_STREQ("TestSchema", spec.GetSupportedSchemas().c_str());
    EXPECT_STREQ("hide expr", spec.GetHideExpression().c_str());
    EXPECT_TRUE(spec.ShouldSuppressSimilarAncestorsCheck());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AllInstanceNodesSpecificationTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "specType": "AllInstanceNodes"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    AllInstanceNodesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_TRUE(spec.GetGroupByClass());
    EXPECT_TRUE(spec.GetGroupByLabel());
    EXPECT_STREQ("", spec.GetSupportedSchemas().c_str());
    EXPECT_STREQ("", spec.GetHideExpression().c_str());
    EXPECT_FALSE(spec.ShouldSuppressSimilarAncestorsCheck());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AllInstanceNodesSpecificationTests, WriteToJson)
    {
    AllInstanceNodesSpecification spec(1000, true, true, true, true, true, "SupportedSchema");
    spec.SetSuppressSimilarAncestorsCheck(true);
    spec.SetHideExpression("hide expr");
    BeJsDocument json = spec.WriteJson();
    BeJsDocument expected(R"({
        "specType": "AllInstanceNodes",
        "hasChildren": "Always",
        "hideNodesInHierarchy": true,
        "hideIfNoChildren": true,
        "hideExpression": "hide expr",
        "suppressSimilarAncestorsCheck": true,
        "supportedSchemas": {"schemaNames": ["SupportedSchema"]}
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AllInstanceNodesSpecificationTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "e40b17cb8c6b5ad3c26e7de8e2a38a7d";

    // Make sure that introducing additional attributes with default values don't affect the hash
    AllInstanceNodesSpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    AllInstanceNodesSpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }
