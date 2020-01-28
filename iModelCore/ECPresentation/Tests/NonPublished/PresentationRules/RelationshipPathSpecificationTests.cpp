/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelationshipPathSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                        Grigas.Petraitis                01/2020
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
* @betest                                        Grigas.Petraitis                01/2020
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
* @betest                                        Grigas.Petraitis                01/2020
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
* @bsimethod                                    Grigas.Petraitis                01/2020
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
* @bsimethod                                    Grigas.Petraitis                01/2020
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
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelationshipPathSpecificationTests, ComputesCorrectHashes)
    {
    RelationshipPathSpecification spec1;
    spec1.AddStep(*new RelationshipStepSpecification("s1:c1", RequiredRelationDirection_Backward, "s2:c2"));
    spec1.AddStep(*new RelationshipStepSpecification("s3:c3", RequiredRelationDirection_Forward));

    RelationshipPathSpecification spec2;
    spec2.AddStep(*new RelationshipStepSpecification("s1:c1", RequiredRelationDirection_Backward, "s2:c2"));
    spec2.AddStep(*new RelationshipStepSpecification("s3:c3", RequiredRelationDirection_Forward));

    RelationshipPathSpecification spec3;
    spec3.AddStep(*new RelationshipStepSpecification("s4:c4", RequiredRelationDirection_Forward, "s5:c5"));
    spec3.AddStep(*new RelationshipStepSpecification("s6:c6", RequiredRelationDirection_Backward));

    // Hashes are same for specifications with same properties
    EXPECT_STREQ(spec1.GetHash().c_str(), spec2.GetHash().c_str());
    // Hashes differs for specifications with different properties
    EXPECT_STRNE(spec1.GetHash().c_str(), spec3.GetHash().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
struct RepeatableRelationshipPathSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                        Grigas.Petraitis                01/2020
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
* @betest                                        Grigas.Petraitis                01/2020
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
* @betest                                        Grigas.Petraitis                01/2020
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
* @betest                                        Grigas.Petraitis                01/2020
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
* @bsimethod                                    Grigas.Petraitis                01/2020
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
* @bsimethod                                    Grigas.Petraitis                01/2020
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
* @bsimethod                                    Grigas.Petraitis                01/2020
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
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RepeatableRelationshipPathSpecificationTests, ComputesCorrectHashes)
    {
    RepeatableRelationshipPathSpecification spec1;
    spec1.AddStep(*new RepeatableRelationshipStepSpecification("s1:c1", RequiredRelationDirection_Backward, "s2:c2", 3));
    spec1.AddStep(*new RepeatableRelationshipStepSpecification("s3:c3", RequiredRelationDirection_Forward));

    RepeatableRelationshipPathSpecification spec2;
    spec2.AddStep(*new RepeatableRelationshipStepSpecification("s1:c1", RequiredRelationDirection_Backward, "s2:c2", 3));
    spec2.AddStep(*new RepeatableRelationshipStepSpecification("s3:c3", RequiredRelationDirection_Forward));

    RepeatableRelationshipPathSpecification spec3;
    spec3.AddStep(*new RepeatableRelationshipStepSpecification("s4:c4", RequiredRelationDirection_Forward, "s5:c5", 4));
    spec3.AddStep(*new RepeatableRelationshipStepSpecification("s6:c6", RequiredRelationDirection_Backward));

    // Hashes are same for specifications with same properties
    EXPECT_STREQ(spec1.GetHash().c_str(), spec2.GetHash().c_str());
    // Hashes differs for specifications with different properties
    EXPECT_STRNE(spec1.GetHash().c_str(), spec3.GetHash().c_str());
    }
