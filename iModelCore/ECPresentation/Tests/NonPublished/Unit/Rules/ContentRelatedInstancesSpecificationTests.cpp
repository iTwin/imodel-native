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
struct ContentRelatedInstancesSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRelatedInstancesSpecificationTests, LoadFromJsonDeprecated)
    {
    static Utf8CP jsonString = R"({
        "specType": "ContentRelatedInstances",
        "relationships": {"schemaName":"MySchemaName", "classNames": ["ClassA","ClassB"]},
        "relatedClasses": [{"schemaName": "MySchemaName", "classNames": ["ClassA","ClassB"]}],
        "requiredDirection": "Both",
        "skipRelatedLevel": 321,
        "instanceFilter": "this.PropertyName = 10",
        "isRecursive": true
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    ContentRelatedInstancesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_EQ(321, spec.GetSkipRelatedLevel());
    EXPECT_TRUE(spec.IsRecursive());
    EXPECT_STREQ("MySchemaName:ClassA,ClassB", spec.GetRelationshipClassNames().c_str());
    EXPECT_STREQ("MySchemaName:ClassA,ClassB", spec.GetRelatedClassNames().c_str());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Both, spec.GetRequiredRelationDirection());
    EXPECT_STREQ("this.PropertyName = 10", spec.GetInstanceFilter().c_str());
    EXPECT_TRUE(spec.GetRelationshipPaths().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRelatedInstancesSpecificationTests, LoadFromJson)
    {
    static Utf8CP jsonString = R"({
        "specType": "ContentRelatedInstances",
        "relationships": {"schemaName":"MySchemaName", "classNames": ["ClassA","ClassB"]},
        "relatedClasses": [{"schemaName": "MySchemaName", "classNames": ["ClassA","ClassB"]}],
        "requiredDirection": "Both",
        "skipRelatedLevel": 321,
        "isRecursive": true,
        "instanceFilter": "this.PropertyName = 10",
        "relationshipPaths": [{
            "relationship": {"schemaName": "m", "className": "n" },
            "direction": "Forward",
            "targetClass": {"schemaName": "o", "className": "p"}
        }, {
            "relationship": {"schemaName": "q", "className": "r" },
            "direction": "Backward",
            "targetClass": {"schemaName": "s", "className": "t"}
        }]
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    ContentRelatedInstancesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("this.PropertyName = 10", spec.GetInstanceFilter().c_str());

    EXPECT_EQ(0, spec.GetSkipRelatedLevel());
    EXPECT_FALSE(spec.IsRecursive());
    EXPECT_TRUE(spec.GetRelationshipClassNames().empty());
    EXPECT_TRUE(spec.GetRelatedClassNames().empty());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Both, spec.GetRequiredRelationDirection());

    ASSERT_EQ(2, spec.GetRelationshipPaths().size());
    ASSERT_EQ(1, spec.GetRelationshipPaths()[0]->GetSteps().size());
    EXPECT_STREQ("m:n", spec.GetRelationshipPaths()[0]->GetSteps().front()->GetRelationshipClassName().c_str());
    EXPECT_EQ(RequiredRelationDirection_Forward, spec.GetRelationshipPaths()[0]->GetSteps().front()->GetRelationDirection());
    EXPECT_STREQ("o:p", spec.GetRelationshipPaths()[0]->GetSteps().front()->GetTargetClassName().c_str());
    ASSERT_EQ(1, spec.GetRelationshipPaths()[1]->GetSteps().size());
    EXPECT_STREQ("q:r", spec.GetRelationshipPaths()[1]->GetSteps().front()->GetRelationshipClassName().c_str());
    EXPECT_EQ(RequiredRelationDirection_Backward, spec.GetRelationshipPaths()[1]->GetSteps().front()->GetRelationDirection());
    EXPECT_STREQ("s:t", spec.GetRelationshipPaths()[1]->GetSteps().front()->GetTargetClassName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRelatedInstancesSpecificationTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "specType": "ContentRelatedInstances",
        "relationshipPaths": [{
            "relationship": {"schemaName": "A", "className": "B"},
            "direction": "Forward"
        }]
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    ContentRelatedInstancesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_EQ(0, spec.GetSkipRelatedLevel());
    EXPECT_FALSE(spec.IsRecursive());
    EXPECT_TRUE(spec.GetRelationshipClassNames().empty());
    EXPECT_TRUE(spec.GetRelatedClassNames().empty());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Both, spec.GetRequiredRelationDirection());
    EXPECT_TRUE(spec.GetInstanceFilter().empty());
    EXPECT_EQ(1, spec.GetRelationshipPaths().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRelatedInstancesSpecificationTests, WriteToJsonDeprecated)
    {
    ContentRelatedInstancesSpecification spec(123, 3, true, "filter", RequiredRelationDirection_Backward, "s1:c1", "E:s2:c2");
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "specType": "ContentRelatedInstances",
        "priority": 123,
        "skipRelatedLevel": 3,
        "isRecursive": true,
        "instanceFilter": "filter",
        "requiredDirection": "Backward",
        "relationships": {"schemaName": "s1", "classNames": ["c1"]},
        "relatedClasses": {"schemaName": "s2", "classNames": ["E:c2"]}
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRelatedInstancesSpecificationTests, WriteToJson)
    {
    ContentRelatedInstancesSpecification spec(123, true, "filter", {new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification("s1:c1", RequiredRelationDirection_Backward, "s2:c2", 5))});
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "specType": "ContentRelatedInstances",
        "priority": 123,
        "onlyIfNotHandled": true,
        "instanceFilter": "filter",
        "relationshipPaths": [{
            "relationship": {"schemaName": "s1", "className": "c1"},
            "direction": "Backward",
            "targetClass": {"schemaName": "s2", "className": "c2"},
            "count": 5
        }]
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRelatedInstancesSpecificationTests, LoadFromXml)
    {
    static Utf8CP xmlString = R"(
        <ContentRelatedInstances RelationshipClassNames="MySchemaName:ClassA,ClassB"
         RelatedClassNames="MySchemaName:ClassA,ClassB" RequiredDirection="Both"
         SkipRelatedLevel="321" InstanceFilter="this.PropertyName = 10" IsRecursive = "true" />
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    ContentRelatedInstancesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_EQ(321, spec.GetSkipRelatedLevel());
    EXPECT_TRUE(spec.IsRecursive());
    EXPECT_STREQ("MySchemaName:ClassA,ClassB", spec.GetRelationshipClassNames().c_str());
    EXPECT_STREQ("MySchemaName:ClassA,ClassB", spec.GetRelatedClassNames().c_str());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Both, spec.GetRequiredRelationDirection());
    EXPECT_STREQ("this.PropertyName = 10", spec.GetInstanceFilter().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRelatedInstancesSpecificationTests, LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = "<ContentRelatedInstances/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    ContentRelatedInstancesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_EQ(0, spec.GetSkipRelatedLevel());
    EXPECT_FALSE(spec.IsRecursive());
    EXPECT_TRUE(spec.GetRelationshipClassNames().empty());
    EXPECT_TRUE(spec.GetRelatedClassNames().empty());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Both, spec.GetRequiredRelationDirection());
    EXPECT_TRUE(spec.GetInstanceFilter().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRelatedInstancesSpecificationTests, WritesToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    ContentRelatedInstancesSpecification spec;
    spec.SetPriority(123);
    spec.SetRelationshipClassNames("MySchemaName:ClassA,ClassB");
    spec.SetRelatedClassNames("MySchemaName:ClassA,ClassB");
    spec.SetInstanceFilter("this.PropertyName = 10");
    spec.SetIsRecursive(true);
    spec.SetSkipRelatedLevel(456);
    spec.SetRequiredRelationDirection(RequiredRelationDirection::RequiredRelationDirection_Backward);
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<ContentRelatedInstances Priority="123" ShowImages="false" OnlyIfNotHandled="false" SkipRelatedLevel="456" IsRecursive="true"
                InstanceFilter="this.PropertyName = 10" RelationshipClassNames="MySchemaName:ClassA,ClassB"
                RelatedClassNames="MySchemaName:ClassA,ClassB" RequiredDirection="Backward"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRelatedInstancesSpecificationTests, ComputesCorrectHashesDeprecated)
    {
    ContentRelatedInstancesSpecification spec1(1000, 3, true, "filter", RequiredRelationDirection::RequiredRelationDirection_Backward, "ClassAHasClassB", "ClassB");
    ContentRelatedInstancesSpecification spec2(1000, 3, true, "filter", RequiredRelationDirection::RequiredRelationDirection_Backward, "ClassAHasClassB", "ClassB");
    ContentRelatedInstancesSpecification spec3(1000, 3, true, "filter", RequiredRelationDirection::RequiredRelationDirection_Backward, "ClassBHasClassA", "ClassA");

    // Hashes are same for identical specifications
    EXPECT_STREQ(spec1.GetHash().c_str(), spec2.GetHash().c_str());
    // Hashes differs for different specifications
    EXPECT_STRNE(spec1.GetHash().c_str(), spec3.GetHash().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRelatedInstancesSpecificationTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "7697665c155704e5bd0159eba51862f8";

    // Make sure that introducing additional attributes with default values don't affect the hash
    ContentRelatedInstancesSpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    ContentRelatedInstancesSpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // Test that each attribute affects the hash
    ContentRelatedInstancesSpecification specSkipRelatedLevel(defaultSpec);
    specSkipRelatedLevel.SetSkipRelatedLevel(123);
    EXPECT_STRNE(DEFAULT_HASH, specSkipRelatedLevel.GetHash().c_str());
    specSkipRelatedLevel.SetSkipRelatedLevel(0);
    EXPECT_STREQ(DEFAULT_HASH, specSkipRelatedLevel.GetHash().c_str());

    ContentRelatedInstancesSpecification specIsRecursive(defaultSpec);
    specIsRecursive.SetIsRecursive(true);
    EXPECT_STRNE(DEFAULT_HASH, specIsRecursive.GetHash().c_str());
    specIsRecursive.SetIsRecursive(false);
    EXPECT_STREQ(DEFAULT_HASH, specIsRecursive.GetHash().c_str());

    ContentRelatedInstancesSpecification specInstanceFilter(defaultSpec);
    specInstanceFilter.SetInstanceFilter("instanceFilter");
    EXPECT_STRNE(DEFAULT_HASH, specInstanceFilter.GetHash().c_str());
    specInstanceFilter.SetInstanceFilter("");
    EXPECT_STREQ(DEFAULT_HASH, specInstanceFilter.GetHash().c_str());

    ContentRelatedInstancesSpecification specRequiredDirection(defaultSpec);
    specRequiredDirection.SetRequiredRelationDirection(RequiredRelationDirection_Backward);
    EXPECT_STRNE(DEFAULT_HASH, specRequiredDirection.GetHash().c_str());
    specRequiredDirection.SetRequiredRelationDirection(RequiredRelationDirection_Both);
    EXPECT_STREQ(DEFAULT_HASH, specRequiredDirection.GetHash().c_str());

    ContentRelatedInstancesSpecification specRelationshipClassNames(defaultSpec);
    specRelationshipClassNames.SetRelationshipClassNames("Classnames");
    EXPECT_STRNE(DEFAULT_HASH, specRelationshipClassNames.GetHash().c_str());
    specRelationshipClassNames.SetRelationshipClassNames("");
    EXPECT_STREQ(DEFAULT_HASH, specRelationshipClassNames.GetHash().c_str());

    ContentRelatedInstancesSpecification specRelatedClassNames(defaultSpec);
    specRelatedClassNames.SetRelatedClassNames("Classnames");
    EXPECT_STRNE(DEFAULT_HASH, specRelatedClassNames.GetHash().c_str());
    specRelatedClassNames.SetRelatedClassNames("");
    EXPECT_STREQ(DEFAULT_HASH, specRelatedClassNames.GetHash().c_str());

    ContentRelatedInstancesSpecification specRelationshipPaths(defaultSpec);
    specRelationshipPaths.AddRelationshipPath(*new RepeatableRelationshipPathSpecification());
    EXPECT_STRNE(DEFAULT_HASH, specRelationshipPaths.GetHash().c_str());
    specRelationshipPaths.ClearRelationshipPaths();
    EXPECT_STREQ(DEFAULT_HASH, specRelationshipPaths.GetHash().c_str());
    }
