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
struct ContentRuleTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRuleTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "Content",
        "specifications": [{
            "specType": "ContentInstancesOfSpecificClasses",
            "classes": {"schemaName": "TestSchema", "classNames": ["ClassA"]}
        }, {
            "specType": "ContentRelatedInstances",
            "relationshipPaths": [{
                "relationship": {"schemaName": "A", "className": "B"},
                "direction": "Forward"
            }]
        }, {
            "specType": "SelectedNodeInstances"
        }]
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    ContentRule rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_EQ(3, rule.GetSpecifications().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRuleTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "Content"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    ContentRule rule;
    EXPECT_TRUE(rule.ReadJson(json));
    EXPECT_EQ(0, rule.GetSpecifications().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRuleTests, WriteToJson)
    {
    ContentRule rule("cond", 123, true);
    rule.AddSpecification(*new SelectedNodeInstancesSpecification());
    rule.AddSpecification(*new ContentRelatedInstancesSpecification());
    rule.AddSpecification(*new ContentInstancesOfSpecificClassesSpecification());
    Json::Value json = rule.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "ruleType": "Content",
        "priority": 123,
        "onlyIfNotHandled": true,
        "condition": "cond",
        "specifications": [{
            "specType": "SelectedNodeInstances"
        }, {
            "specType": "ContentRelatedInstances"
        }, {
            "specType": "ContentInstancesOfSpecificClasses",
            "classes": []
        }]
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRuleTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <ContentRule CustomControl="customControl">
            <ContentInstancesOfSpecificClasses ClassNames="TestSchema:ClassA"/>
            <ContentRelatedInstances/>
            <SelectedNodeInstances/>
        </ContentRule>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    ContentRule rule;
    EXPECT_TRUE(rule.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("customControl", rule.GetCustomControl().c_str());
    EXPECT_EQ(3, rule.GetSpecifications().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRuleTests, LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = "<ContentRule/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    ContentRule rule;
    EXPECT_TRUE(rule.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("", rule.GetCustomControl().c_str());
    EXPECT_EQ(0, rule.GetSpecifications().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRuleTests, WriteToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    ContentRule rule;
    rule.SetCustomControl("customControl");
    rule.AddSpecification(*new ContentRelatedInstancesSpecification());
    rule.WriteXml(xml->GetRootElement());
    static Utf8CP expected = ""
        "<Root>"
            R"(<ContentRule Priority="1000" OnlyIfNotHandled="false" Condition="" CustomControl="customControl">)"
                R"(<ContentRelatedInstances Priority="1000" ShowImages="false" OnlyIfNotHandled="false" SkipRelatedLevel="0" IsRecursive="false" InstanceFilter="" RelationshipClassNames="" RelatedClassNames="" RequiredDirection="Both"/>)"
            R"(</ContentRule>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRuleTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "f15c1cae7882448b3fb0404682e17e61";

    // Make sure that introducing additional attributes with default values don't affect the hash
    ContentRule defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    ContentRule copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }
