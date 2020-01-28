/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelatedInstanceNodesSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceNodesSpecificationTests, LoadsFromJsonDeprecated)
    {
    static Utf8CP jsonString = R"({
        "specType": "RelatedInstanceNodes",
        "groupByClass": false,
        "groupByLabel": false,
        "skipRelatedLevel": 3,
        "instanceFilter": "filter",
        "supportedSchemas": {"schemaNames": ["TestSchema"]},
        "relationships": [
            {"schemaName": "a", "classNames": ["b", "c", "E:d", "E:e"]},
            {"schemaName": "f", "classNames": ["g", "E:h"]}
        ],
        "relatedClasses": {"schemaName": "q", "classNames": ["w"]},
        "requiredDirection": "Backward"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());
    
    RelatedInstanceNodesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_FALSE(spec.GetGroupByClass());
    EXPECT_FALSE(spec.GetGroupByLabel());
    EXPECT_EQ(3, spec.GetSkipRelatedLevel());
    EXPECT_STREQ("filter", spec.GetInstanceFilter().c_str());
    EXPECT_STREQ("TestSchema", spec.GetSupportedSchemas().c_str());
    EXPECT_STREQ("a:b,c;f:g;E:a:d,e;f:h", spec.GetRelationshipClassNames().c_str());
    EXPECT_STREQ("q:w", spec.GetRelatedClassNames().c_str());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Backward, spec.GetRequiredRelationDirection());
    EXPECT_TRUE(spec.GetRelationshipPaths().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceNodesSpecificationTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "specType": "RelatedInstanceNodes",
        "groupByClass": false,
        "groupByLabel": false,
        "skipRelatedLevel": 3,
        "instanceFilter": "filter",
        "supportedSchemas": {"schemaNames": ["TestSchema"]},
        "relationships": [
            {"schemaName": "a", "classNames": ["b", "c", "E:d", "E:e"]},
            {"schemaName": "f", "classNames": ["g", "E:h"]}
        ],
        "relatedClasses": {"schemaName": "q", "classNames": ["w"]},
        "requiredDirection": "Backward",
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
    EXPECT_FALSE(json.isNull());

    RelatedInstanceNodesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_FALSE(spec.GetGroupByClass());
    EXPECT_FALSE(spec.GetGroupByLabel());
    EXPECT_STREQ("filter", spec.GetInstanceFilter().c_str());

    EXPECT_EQ(0, spec.GetSkipRelatedLevel());
    EXPECT_TRUE(spec.GetSupportedSchemas().empty());
    EXPECT_TRUE(spec.GetRelationshipClassNames().empty());
    EXPECT_TRUE(spec.GetRelatedClassNames().empty());
    EXPECT_EQ(RequiredRelationDirection_Both, spec.GetRequiredRelationDirection());

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
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceNodesSpecificationTests, LoadsFromJsonWithDefaultvalues)
    {
    static Utf8CP jsonString = R"({
        "specType": "RelatedInstanceNodes"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());
    
    RelatedInstanceNodesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_TRUE(spec.GetGroupByClass());
    EXPECT_TRUE(spec.GetGroupByLabel());
    EXPECT_EQ(0, spec.GetSkipRelatedLevel());
    EXPECT_STREQ("", spec.GetInstanceFilter().c_str());
    EXPECT_STREQ("", spec.GetSupportedSchemas().c_str());
    EXPECT_STREQ("", spec.GetRelationshipClassNames().c_str());
    EXPECT_STREQ("", spec.GetRelatedClassNames().c_str());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Both, spec.GetRequiredRelationDirection());
    EXPECT_TRUE(spec.GetRelationshipPaths().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceNodesSpecificationTests, WriteToJsonDeprecated)
    {
    RelatedInstanceNodesSpecification spec(1000, true, true, true, true, true, true, true,
        3, "filter", RequiredRelationDirection_Both, "s1,s2", "s3:c1", "s4:c2");
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "specType": "RelatedInstanceNodes",
        "hasChildren": "Always",
        "hideNodesInHierarchy": true,
        "hideIfNoChildren": true,
        "supportedSchemas": {"schemaNames": ["s1", "s2"]},
        "relationships": {"schemaName": "s3", "classNames": ["c1"]},
        "relatedClasses": {"schemaName": "s4", "classNames": ["c2"]},
        "skipRelatedLevel": 3,
        "instanceFilter": "filter"
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceNodesSpecificationTests, WriteToJson)
    {
    RelatedInstanceNodesSpecification spec(1000, ChildrenHint::Never, true, true, true, true, "filter",
        {new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification("a:b", RequiredRelationDirection_Forward, "c:d", 9))});
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "specType": "RelatedInstanceNodes",
        "hasChildren": "Never",
        "hideNodesInHierarchy": true,
        "hideIfNoChildren": true,
        "relationshipPaths": [{
            "relationship": {"schemaName": "a", "className": "b"},
            "direction": "Forward",
            "targetClass": {"schemaName": "c", "className": "d"},
            "count": 9
        }],
        "instanceFilter": "filter"
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceNodesSpecificationTests, LoadFromXml)
    {
    static Utf8CP xmlString = R"(
        <RelatedInstances GroupByRelationship="true" GroupByClass="false" GroupByLabel="false"
         ShowEmptyGroups="true" SkipRelatedLevel="3" InstanceFilter="filter" SupportedSchemas="TestSchema"
         RelationshipClassNames="ClassAHasClassB" RelatedClassNames="ClassB" RequiredDirection="Backward"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    RelatedInstanceNodesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_TRUE(spec.GetGroupByRelationship());
    EXPECT_FALSE(spec.GetGroupByClass());
    EXPECT_FALSE(spec.GetGroupByLabel());
    EXPECT_TRUE(spec.GetShowEmptyGroups());
    EXPECT_EQ(3, spec.GetSkipRelatedLevel());
    EXPECT_STREQ("filter", spec.GetInstanceFilter().c_str());
    EXPECT_STREQ("TestSchema", spec.GetSupportedSchemas().c_str());
    EXPECT_STREQ("ClassAHasClassB", spec.GetRelationshipClassNames().c_str());
    EXPECT_STREQ("ClassB", spec.GetRelatedClassNames().c_str());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Backward, spec.GetRequiredRelationDirection());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceNodesSpecificationTests, LoadFromXmlWithDefaultvalues)
    {
    static Utf8CP xmlString = "<RelatedInstances/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    RelatedInstanceNodesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_FALSE(spec.GetGroupByRelationship());
    EXPECT_TRUE(spec.GetGroupByClass());
    EXPECT_TRUE(spec.GetGroupByLabel());
    EXPECT_FALSE(spec.GetShowEmptyGroups());
    EXPECT_EQ(0, spec.GetSkipRelatedLevel());
    EXPECT_STREQ("", spec.GetInstanceFilter().c_str());
    EXPECT_STREQ("", spec.GetSupportedSchemas().c_str());
    EXPECT_STREQ("", spec.GetRelationshipClassNames().c_str());
    EXPECT_STREQ("", spec.GetRelatedClassNames().c_str());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Both, spec.GetRequiredRelationDirection());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceNodesSpecificationTests, WritesToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    RelatedInstanceNodesSpecification spec;
    spec.SetGroupByClass(true);
    spec.SetGroupByLabel(true);
    spec.SetGroupByRelationship(true);
    spec.SetShowEmptyGroups(true);
    spec.SetSkipRelatedLevel(3);
    spec.SetInstanceFilter("filter");
    spec.SetSupportedSchemas("TestSchema");
    spec.SetRelationshipClassNames("ClassAHasClassB");
    spec.SetRelatedClassNames("ClassB");
    spec.SetRequiredRelationDirection(RequiredRelationDirection::RequiredRelationDirection_Forward);
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<RelatedInstances Priority="1000" HasChildren="Unknown" HideNodesInHierarchy="false"
                HideIfNoChildren="false" ExtendedData="" DoNotSort="false" GroupByClass="true" GroupByRelationship="true" )"
            R"(GroupByLabel="true" ShowEmptyGroups="true" SkipRelatedLevel="3" InstanceFilter="filter" SupportedSchemas="TestSchema" )"
            R"(RelationshipClassNames="ClassAHasClassB" RelatedClassNames="ClassB" RequiredDirection="Forward"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceNodesSpecificationTests, ComputesCorrectHashesDeprecated)
    {
    RelatedInstanceNodesSpecification spec1(1000, true, true, true, true, true, true, true, 3, "filter", 
        RequiredRelationDirection::RequiredRelationDirection_Forward, "TestSchema", "ClassNames", "RelatedClassNames");
    RelatedInstanceNodesSpecification spec2(1000, true, true, true, true, true, true, true, 3, "filter", 
        RequiredRelationDirection::RequiredRelationDirection_Forward, "TestSchema", "ClassNames", "RelatedClassNames");
    RelatedInstanceNodesSpecification spec3(1000, false, true, true, true, true, true, true, 3, "filter", 
        RequiredRelationDirection::RequiredRelationDirection_Forward, "TestSchema", "ClassNames", "RelatedClassNames");

    // Hashes are same for identical specifications
    EXPECT_STREQ(spec1.GetHash().c_str(), spec2.GetHash().c_str());
    // Hashes differs for different specifications
    EXPECT_STRNE(spec1.GetHash().c_str(), spec3.GetHash().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceNodesSpecificationTests, ComputesCorrectHashes)
    {
    RelatedInstanceNodesSpecification spec1(1000, ChildrenHint::Never, true, true, true, true, "filter",
        {new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification("a:b", RequiredRelationDirection_Forward, "c:d", 9))});
    RelatedInstanceNodesSpecification spec2(1000, ChildrenHint::Never, true, true, true, true, "filter",
        {new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification("a:b", RequiredRelationDirection_Forward, "c:d", 9))});
    RelatedInstanceNodesSpecification spec3(1000, ChildrenHint::Never, true, true, true, true, "filter",
        {new RepeatableRelationshipPathSpecification(*new RepeatableRelationshipStepSpecification("e:f", RequiredRelationDirection_Backward, "g:h", 10))});

    // Hashes are same for identical specifications
    EXPECT_STREQ(spec1.GetHash().c_str(), spec2.GetHash().c_str());
    // Hashes differs for different specifications
    EXPECT_STRNE(spec1.GetHash().c_str(), spec3.GetHash().c_str());
    }