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
struct RelationshipPathSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelationshipPathSpecificationTests, LoadsFromJsonObject)
    {
    static Utf8CP jsonString = R"({
        "relationship": {"schemaName": "schema", "className": "rel"},
        "direction": "Forward",
        "targetClass": {"schemaName": "schema", "className": "class"}
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RelationshipPathSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    ASSERT_EQ(1, spec.GetSteps().size());
    EXPECT_STREQ("schema:rel", spec.GetSteps()[0]->GetRelationshipClassName().c_str());
    EXPECT_EQ(RequiredRelationDirection_Forward, spec.GetSteps()[0]->GetRelationDirection());
    EXPECT_STREQ("schema:class", spec.GetSteps()[0]->GetTargetClassName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelationshipPathSpecificationTests, LoadsFromJsonObjectWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "relationship": {"schemaName": "schema", "className": "rel"},
        "direction": "Forward"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RelationshipPathSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    ASSERT_EQ(1, spec.GetSteps().size());
    EXPECT_STREQ("schema:rel", spec.GetSteps()[0]->GetRelationshipClassName().c_str());
    EXPECT_EQ(RequiredRelationDirection_Forward, spec.GetSteps()[0]->GetRelationDirection());
    EXPECT_TRUE(spec.GetSteps()[0]->GetTargetClassName().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelationshipPathSpecificationTests, LoadsFromJsonArray)
    {
    static Utf8CP jsonString = R"([{
        "relationship": {"schemaName": "schema1", "className": "rel1"},
        "direction": "Backward"
    }, {
        "relationship": {"schemaName": "schema2", "className": "rel2"},
        "direction": "Forward"
    }])";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RelationshipPathSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    ASSERT_EQ(2, spec.GetSteps().size());
    EXPECT_STREQ("schema1:rel1", spec.GetSteps()[0]->GetRelationshipClassName().c_str());
    EXPECT_EQ(RequiredRelationDirection_Backward, spec.GetSteps()[0]->GetRelationDirection());
    EXPECT_STREQ("schema2:rel2", spec.GetSteps()[1]->GetRelationshipClassName().c_str());
    EXPECT_EQ(RequiredRelationDirection_Forward, spec.GetSteps()[1]->GetRelationDirection());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelationshipPathSpecificationTests, WriteToJsonAsJsonObjectWhenThereIsOneStep)
    {
    RelationshipPathSpecification spec;
    spec.AddStep(*new RelationshipStepSpecification("s1:c1", RequiredRelationDirection_Backward, "s2:c2"));
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "relationship": {"schemaName": "s1", "className": "c1"},
        "direction": "Backward",
        "targetClass": {"schemaName": "s2", "className": "c2"}
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelationshipPathSpecificationTests, WriteToJsonAsJsonArrayWhenThereAreMultipleSteps)
    {
    RelationshipPathSpecification spec;
    spec.AddStep(*new RelationshipStepSpecification("s1:c1", RequiredRelationDirection_Backward, "s2:c2"));
    spec.AddStep(*new RelationshipStepSpecification("s3:c3", RequiredRelationDirection_Forward));
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"([{
        "relationship": {"schemaName": "s1", "className": "c1"},
        "direction": "Backward",
        "targetClass": {"schemaName": "s2", "className": "c2"}
    }, {
        "relationship": {"schemaName": "s3", "className": "c3"},
        "direction": "Forward"
    }])");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelationshipPathSpecificationTests, ComputesCorrectHashes_Path)
    {
    Utf8CP DEFAULT_HASH = "52642a0cbce13ec964fa84e7b14469e3";

    // first we make sure that introducing additional attributes with default values don't affect the hash
    RelationshipPathSpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // then we make sure that copy has the same hash
    RelationshipPathSpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // then we test that each attribute affects the hash
    RelationshipPathSpecification specWithSteps(defaultSpec);
    specWithSteps.AddStep(*new RelationshipStepSpecification());
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithSteps.GetHash().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelationshipPathSpecificationTests, ComputesCorrectHashes_Step)
    {
    Utf8CP DEFAULT_HASH = "50e00a8ef99719ee9505252526409155";

    // first we make sure that introducing additional attributes with default values don't affect the hash
    RelationshipStepSpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // then we make sure that copy has the same hash
    RelationshipStepSpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // then we test that each attribute affects the hash
    RelationshipStepSpecification specWithRelationDirection(defaultSpec);
    specWithRelationDirection.SetRelationDirection(RequiredRelationDirection_Backward);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithRelationDirection.GetHash().c_str());
    specWithRelationDirection.SetRelationDirection(RequiredRelationDirection_Forward);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithRelationDirection.GetHash().c_str());

    RelationshipStepSpecification specWithRelationshipClassName(defaultSpec);
    specWithRelationshipClassName.SetRelationshipClassName("a");
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithRelationshipClassName.GetHash().c_str());
    specWithRelationshipClassName.SetRelationshipClassName("");
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithRelationshipClassName.GetHash().c_str());

    RelationshipStepSpecification specWithTargetClassName(defaultSpec);
    specWithTargetClassName.SetTargetClassName("a");
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithTargetClassName.GetHash().c_str());
    specWithTargetClassName.SetTargetClassName("");
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithTargetClassName.GetHash().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct RepeatableRelationshipPathSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RepeatableRelationshipPathSpecificationTests, LoadsFromJsonObject)
    {
    static Utf8CP jsonString = R"({
        "relationship": {"schemaName": "schema", "className": "rel"},
        "direction": "Forward",
        "targetClass": {"schemaName": "schema", "className": "class"},
        "count": "2"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RepeatableRelationshipPathSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    ASSERT_EQ(1, spec.GetSteps().size());
    EXPECT_STREQ("schema:rel", spec.GetSteps()[0]->GetRelationshipClassName().c_str());
    EXPECT_EQ(RequiredRelationDirection_Forward, spec.GetSteps()[0]->GetRelationDirection());
    EXPECT_STREQ("schema:class", spec.GetSteps()[0]->GetTargetClassName().c_str());
    EXPECT_EQ(2, spec.GetSteps()[0]->GetCount());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RepeatableRelationshipPathSpecificationTests, LoadsFromJsonObjectWithStarCount)
    {
    static Utf8CP jsonString = R"({
        "relationship": {"schemaName": "schema", "className": "rel"},
        "direction": "Forward",
        "count": "*"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RepeatableRelationshipPathSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    ASSERT_EQ(1, spec.GetSteps().size());
    EXPECT_EQ(0, spec.GetSteps()[0]->GetCount());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RepeatableRelationshipPathSpecificationTests, LoadsFromJsonObjectWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "relationship": {"schemaName": "schema", "className": "rel"},
        "direction": "Forward"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RepeatableRelationshipPathSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    ASSERT_EQ(1, spec.GetSteps().size());
    EXPECT_STREQ("schema:rel", spec.GetSteps()[0]->GetRelationshipClassName().c_str());
    EXPECT_EQ(RequiredRelationDirection_Forward, spec.GetSteps()[0]->GetRelationDirection());
    EXPECT_TRUE(spec.GetSteps()[0]->GetTargetClassName().empty());
    EXPECT_EQ(1, spec.GetSteps()[0]->GetCount());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RepeatableRelationshipPathSpecificationTests, LoadsFromJsonArray)
    {
    static Utf8CP jsonString = R"([{
        "relationship": {"schemaName": "schema1", "className": "rel1"},
        "direction": "Backward"
    }, {
        "relationship": {"schemaName": "schema2", "className": "rel2"},
        "direction": "Forward"
    }])";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RepeatableRelationshipPathSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    ASSERT_EQ(2, spec.GetSteps().size());
    EXPECT_STREQ("schema1:rel1", spec.GetSteps()[0]->GetRelationshipClassName().c_str());
    EXPECT_EQ(RequiredRelationDirection_Backward, spec.GetSteps()[0]->GetRelationDirection());
    EXPECT_STREQ("schema2:rel2", spec.GetSteps()[1]->GetRelationshipClassName().c_str());
    EXPECT_EQ(RequiredRelationDirection_Forward, spec.GetSteps()[1]->GetRelationDirection());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RepeatableRelationshipPathSpecificationTests, WriteToJsonAsJsonObjectWhenThereIsOneStep)
    {
    RepeatableRelationshipPathSpecification spec;
    spec.AddStep(*new RepeatableRelationshipStepSpecification("s1:c1", RequiredRelationDirection_Backward, "s2:c2", 3));
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "relationship": {"schemaName": "s1", "className": "c1"},
        "direction": "Backward",
        "targetClass": {"schemaName": "s2", "className": "c2"},
        "count": 3
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RepeatableRelationshipPathSpecificationTests, WriteToJsonAsJsonArrayWhenThereAreMultipleSteps)
    {
    RepeatableRelationshipPathSpecification spec;
    spec.AddStep(*new RepeatableRelationshipStepSpecification("s1:c1", RequiredRelationDirection_Backward, "s2:c2", 3));
    spec.AddStep(*new RepeatableRelationshipStepSpecification("s3:c3", RequiredRelationDirection_Forward));
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"([{
        "relationship": {"schemaName": "s1", "className": "c1"},
        "direction": "Backward",
        "targetClass": {"schemaName": "s2", "className": "c2"},
        "count": 3
    }, {
        "relationship": {"schemaName": "s3", "className": "c3"},
        "direction": "Forward"
    }])");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RepeatableRelationshipPathSpecificationTests, WriteToJsonWithCountZero)
    {
    RepeatableRelationshipPathSpecification spec;
    spec.AddStep(*new RepeatableRelationshipStepSpecification("s1:c1", RequiredRelationDirection_Backward, "s2:c2", 0));
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "relationship": {"schemaName": "s1", "className": "c1"},
        "direction": "Backward",
        "targetClass": {"schemaName": "s2", "className": "c2"},
        "count": "*"
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RepeatableRelationshipPathSpecificationTests, ComputesCorrectHashes_Path)
    {
    Utf8CP DEFAULT_HASH = "7be56b3fbf959a7a83bfef17432664ea";

    // first we make sure that introducing additional attributes with default values don't affect the hash
    RepeatableRelationshipPathSpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // then we make sure that copy has the same hash
    RepeatableRelationshipPathSpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // then we test that each attribute affects the hash
    RepeatableRelationshipPathSpecification specWithSteps(defaultSpec);
    specWithSteps.AddStep(*new RepeatableRelationshipStepSpecification());
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithSteps.GetHash().c_str());
    specWithSteps.ClearSteps();
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithSteps.GetHash().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RepeatableRelationshipPathSpecificationTests, ComputesCorrectHashes_Step)
    {
    Utf8CP DEFAULT_HASH = "4d16471377acd4a4a72ddfeb0af808c1";

    // first we make sure that introducing additional attributes with default values don't affect the hash
    RepeatableRelationshipStepSpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // then we make sure that copy has the same hash
    RepeatableRelationshipStepSpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // then we test that each attribute affects the hash
    RepeatableRelationshipStepSpecification specWithCount(defaultSpec);
    specWithCount.SetCount(999);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithCount.GetHash().c_str());
    specWithCount.SetCount(1);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithCount.GetHash().c_str());
    }
