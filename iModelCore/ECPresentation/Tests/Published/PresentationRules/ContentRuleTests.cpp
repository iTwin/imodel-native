/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/ContentRuleTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentRuleTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRuleTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "Content",
        "specifications": [{
            "specType": "ContentInstancesOfSpecificClasses",
            "classes": {"schemaName": "TestSchema", "classNames": ["ClassA"]}
        }, {
            "specType": "ContentRelatedInstances"
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
* @betest                                   Aidas.Kilinskas                		04/2018
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
* @bsimethod                                    Grigas.Petraitis                07/2018
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
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
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
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
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
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
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
            R"(<ContentRule Priority="1000" CustomControl="customControl" Condition="" OnlyIfNotHandled="false">)"
                R"(<ContentRelatedInstances Priority="1000" ShowImages="false" SkipRelatedLevel="0" IsRecursive="false" InstanceFilter="" RelationshipClassNames="" RelatedClassNames="" RequiredDirection="Both"/>)"
            R"(</ContentRule>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentRuleTests, ComputesCorrectHashes)
    {
    ContentRule rule1;
    rule1.SetCustomControl("customControl");
    rule1.AddSpecification(*new ContentRelatedInstancesSpecification());
    ContentRule rule2;
    rule2.SetCustomControl("customControl");
    rule2.AddSpecification(*new ContentRelatedInstancesSpecification());
    ContentRule rule3;
    rule3.SetCustomControl("customControl");

    // Hashes are same for identical rules
    EXPECT_STREQ(rule1.GetHash().c_str(), rule2.GetHash().c_str());
    // Hashes differs for different rules
    EXPECT_STRNE(rule1.GetHash().c_str(), rule3.GetHash().c_str());
    }