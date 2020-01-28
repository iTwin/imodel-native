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
struct RelatedInstanceSpecificationTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
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
    Json::Value json = Json::Reader::DoParse(jsonString);
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
* @betest                                       Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, LoadsFromJson)
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
    Json::Value json = Json::Reader::DoParse(jsonString);
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
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, LoadsFromJsonWithDefaultValuesDeprecated)
    {
    static Utf8CP jsonString = R"({
        "relationship": {"schemaName": "a", "className": "b"},
        "class": {"schemaName": "c", "className": "d"},
        "alias": "TestAlias"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
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
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, WriteToJson)
    {
    RelatedInstanceSpecification spec(RequiredRelationDirection_Forward, "s1:c1", "s2:c2", "alias", true);
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "relationshipPath": {
            "relationship": {"schemaName": "s1", "className": "c1"},
            "direction": "Forward",
            "targetClass": {"schemaName": "s2", "className": "c2"}
        },
        "alias": "alias",
        "isRequired": true
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <RelatedInstance RelationshipName="ClassAHasClassB" ClassName="ClassA" Alias="TestAlias"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    RelatedInstanceSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    ASSERT_EQ(1, spec.GetRelationshipPath().GetSteps().size());
    EXPECT_STREQ("ClassAHasClassB", spec.GetRelationshipPath().GetSteps().front()->GetRelationshipClassName().c_str());
    EXPECT_STREQ("ClassA", spec.GetRelationshipPath().GetSteps().front()->GetTargetClassName().c_str());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Both, spec.GetRelationshipPath().GetSteps().front()->GetRelationDirection());
    EXPECT_STREQ("TestAlias", spec.GetAlias().c_str());
    EXPECT_EQ(false, spec.IsRequired());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, LoadsFromXmlWithIsRequiredAttributeSet)
    {
    static Utf8CP xmlString = R"(
        <RelatedInstance RelationshipName="ClassAHasClassB" ClassName="ClassA" Alias="TestAlias" IsRequired="true"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    RelatedInstanceSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    ASSERT_EQ(1, spec.GetRelationshipPath().GetSteps().size());
    EXPECT_STREQ("ClassAHasClassB", spec.GetRelationshipPath().GetSteps().front()->GetRelationshipClassName().c_str());
    EXPECT_STREQ("ClassA", spec.GetRelationshipPath().GetSteps().front()->GetTargetClassName().c_str());
    EXPECT_EQ(RequiredRelationDirection::RequiredRelationDirection_Both, spec.GetRelationshipPath().GetSteps().front()->GetRelationDirection());
    EXPECT_STREQ("TestAlias", spec.GetAlias().c_str());
    EXPECT_EQ(true, spec.IsRequired());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, LoadFromJsonFailsWhenRelationshipAttributeIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "class": {"schemaName": "c", "className": "d"},
        "alias": "TestAlias",
        "isRequired": true,
        "direction": "Forward"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    RelatedInstanceSpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, LoadFromJsonFailsWhenClassNameAttributeIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "relationship": {"schemaName": "a", "className": "b"},
        "alias": "TestAlias",
        "isRequired": true,
        "direction": "Forward"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    RelatedInstanceSpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, LoadFromJsonFailsWhenAliasAttributeIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "relationship": {"schemaName": "a", "className": "b"},
        "class": {"schemaName": "c", "className": "d"},
        "isRequired": true,
        "direction": "Forward"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    RelatedInstanceSpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, LoadFromXmlFailsWhenRelationshipAttributeIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <RelatedInstance ClassName="ClassA" Alias="TestAlias"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    RelatedInstanceSpecification spec;
    EXPECT_FALSE(spec.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, LoadFromXmlFailsWhenClassNameAttributeIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <RelatedInstance RelationshipName="ClassAHasClassB" Alias="TestAlias"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    RelatedInstanceSpecification spec;
    EXPECT_FALSE(spec.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, LoadFromXmlFailsWhenAliasAttributeIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <RelatedInstance RelationshipName="ClassAHasClassB" ClassName="ClassA"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    RelatedInstanceSpecification spec;
    EXPECT_FALSE(spec.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, WritesToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    RelatedInstanceSpecification spec(RequiredRelationDirection::RequiredRelationDirection_Backward, "ClassAHasClassB", "ClassA", "TestAlias");
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<RelatedInstance ClassName="ClassA" RelationshipName="ClassAHasClassB" RelationshipDirection="Backward" Alias="TestAlias" IsRequired="false"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Mantas.Kontrimas                03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, WritesToXmlWithIsRequiredAttribute)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    RelatedInstanceSpecification spec(RequiredRelationDirection::RequiredRelationDirection_Backward, "ClassAHasClassB", "ClassA", "TestAlias", true);
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<RelatedInstance ClassName="ClassA" RelationshipName="ClassAHasClassB" RelationshipDirection="Backward" Alias="TestAlias" IsRequired="true"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedInstanceSpecificationTests, ComputesCorrectHashes)
    {
    RelatedInstanceSpecification spec1(RequiredRelationDirection::RequiredRelationDirection_Backward, "Relation", "ClassName", "class1");
    RelatedInstanceSpecification spec2(RequiredRelationDirection::RequiredRelationDirection_Backward, "Relation", "ClassName", "class1");
    RelatedInstanceSpecification spec3(RequiredRelationDirection::RequiredRelationDirection_Backward, "Relation", "ClassName", "class2");

    // Hashes are same for identical specifications
    EXPECT_STREQ(spec1.GetHash().c_str(), spec2.GetHash().c_str());
    // Hashes differs for different specifications
    EXPECT_STRNE(spec1.GetHash().c_str(), spec3.GetHash().c_str());
    }