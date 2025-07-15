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
struct RelatedInstanceSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, LoadsFromJsonDeprecated)
    {
    static Utf8CP jsonString = R"({
        "relationship": {"schemaName": "a", "className": "b"},
        "class": {"schemaName": "c", "className": "d"},
        "alias": "TestAlias",
        "isRequired": true,
        "requiredDirection": "Forward"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    RelatedInstanceSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    ASSERT_EQ(1, spec.GetRelationshipPath().GetSteps().size());
    EXPECT_STREQ("a:b", spec.GetRelationshipPath().GetSteps().front()->GetRelationshipClassName().c_str());
    EXPECT_STREQ("c:d", spec.GetRelationshipPath().GetSteps().front()->GetTargetClassName().c_str());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Forward, spec.GetRelationshipPath().GetSteps().front()->GetRelationDirection());
    EXPECT_STREQ("TestAlias", spec.GetAlias().c_str());
    EXPECT_TRUE(spec.IsRequired());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, LoadsFromJsonWithPath)
    {
    static Utf8CP jsonString = R"({
        "relationshipPath": {
            "relationship": {"schemaName": "a", "className": "b"},
            "direction": "Forward",
            "targetClass": {"schemaName": "c", "className": "d"}
        },
        "alias": "TestAlias",
        "isRequired": true
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    RelatedInstanceSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    ASSERT_EQ(1, spec.GetRelationshipPath().GetSteps().size());
    EXPECT_STREQ("a:b", spec.GetRelationshipPath().GetSteps().front()->GetRelationshipClassName().c_str());
    EXPECT_STREQ("c:d", spec.GetRelationshipPath().GetSteps().front()->GetTargetClassName().c_str());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Forward, spec.GetRelationshipPath().GetSteps().front()->GetRelationDirection());
    EXPECT_STREQ("TestAlias", spec.GetAlias().c_str());
    EXPECT_TRUE(spec.IsRequired());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, LoadsFromJsonWithTargetInstances)
    {
    static Utf8CP jsonString = R"({
        "targetInstances": {
            "class": {"schemaName": "a", "className": "b"},
            "instanceIds": ["0x1", "0x3"]
        },
        "alias": "TestAlias",
        "isRequired": true
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    RelatedInstanceSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    ASSERT_EQ(0, spec.GetRelationshipPath().GetSteps().size());
    ASSERT_TRUE(nullptr != spec.GetTargetInstancesSpecification());
    EXPECT_STREQ("a:b", spec.GetTargetInstancesSpecification()->GetClassName().c_str());
    EXPECT_EQ((bvector<ECInstanceId>{ ECInstanceId((uint64_t)1), ECInstanceId((uint64_t)3) }), spec.GetTargetInstancesSpecification()->GetInstanceIds());
    EXPECT_STREQ("TestAlias", spec.GetAlias().c_str());
    EXPECT_TRUE(spec.IsRequired());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, LoadsFromJsonWithDefaultValuesDeprecated)
    {
    static Utf8CP jsonString = R"({
        "relationship": {"schemaName": "a", "className": "b"},
        "class": {"schemaName": "c", "className": "d"},
        "alias": "TestAlias"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    RelatedInstanceSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    ASSERT_EQ(1, spec.GetRelationshipPath().GetSteps().size());
    EXPECT_STREQ("a:b", spec.GetRelationshipPath().GetSteps().front()->GetRelationshipClassName().c_str());
    EXPECT_STREQ("c:d", spec.GetRelationshipPath().GetSteps().front()->GetTargetClassName().c_str());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Both, spec.GetRelationshipPath().GetSteps().front()->GetRelationDirection());
    EXPECT_STREQ("TestAlias", spec.GetAlias().c_str());
    EXPECT_FALSE(spec.IsRequired());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, WriteToJson)
    {
    RelatedInstanceSpecification spec(RequiredRelationDirection_Forward, "s1:c1", "s2:c2", "alias", true);
    BeJsDocument json = spec.WriteJson();
    BeJsDocument expected(R"({
        "relationshipPath": {
            "relationship": {"schemaName": "s1", "className": "c1"},
            "direction": "Forward",
            "targetClass": {"schemaName": "s2", "className": "c2"}
        },
        "alias": "alias",
        "isRequired": true
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, LoadFromJsonFailsWhenRelationshipAttributeIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "class": {"schemaName": "c", "className": "d"},
        "alias": "TestAlias",
        "isRequired": true,
        "direction": "Forward"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    RelatedInstanceSpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, LoadFromJsonFailsWhenClassNameAttributeIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "relationship": {"schemaName": "a", "className": "b"},
        "alias": "TestAlias",
        "isRequired": true,
        "direction": "Forward"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    RelatedInstanceSpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, LoadFromJsonFailsWhenAliasAttributeIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "relationship": {"schemaName": "a", "className": "b"},
        "class": {"schemaName": "c", "className": "d"},
        "isRequired": true,
        "direction": "Forward"
    })";
    BeJsDocument json(jsonString);
    EXPECT_FALSE(json.isNull());

    RelatedInstanceSpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, WriteToJsonWithTargetInstances)
    {
    RelatedInstanceSpecification spec(std::make_unique<RelatedInstanceTargetInstancesSpecification>("a:b", bvector<ECInstanceId>{ ECInstanceId((uint64_t)4), ECInstanceId((uint64_t)5) }), "a", true);
    BeJsDocument json = spec.WriteJson();
    BeJsDocument expected(R"({
        "targetInstances": {
            "class": {"schemaName": "a", "className": "b"},
            "instanceIds": ["0x4", "0x5"]
        },
        "alias": "a",
        "isRequired": true
    })");
    EXPECT_TRUE(expected.isExactEqual(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "611d4d5ffb1d82349193dfb1bc7fed9b";

    // Make sure that introducing additional attributes with default values don't affect the hash
    RelatedInstanceSpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    RelatedInstanceSpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    RelatedInstanceSpecification specWithTargetInstances(std::make_unique<RelatedInstanceTargetInstancesSpecification>("", bvector<ECInstanceId>()), "", false);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithTargetInstances.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }
