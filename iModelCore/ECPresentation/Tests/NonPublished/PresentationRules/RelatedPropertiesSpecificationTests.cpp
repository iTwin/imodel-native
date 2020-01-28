/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct RelatedPropertiesSpecificationsTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromJsonDeprecated)
    {
    static Utf8CP jsonString = R"({
        "relationships": {"schemaName": "Relationship", "classNames": ["Names"]},
        "relatedClasses": [{"schemaName": "Related", "classNames": ["Class", "Names"]}],
        "requiredDirection": "Backward",
        "properties": ["Property", "Names"],
        "relationshipMeaning": "SameInstance",
        "isPolymorphic": true,
        "nestedRelatedProperties": [{}],
        "autoExpand": true
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RelatedPropertiesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));

    ASSERT_EQ(2, spec.GetProperties().size());
    EXPECT_STREQ("Property", spec.GetProperties()[0]->GetPropertyName().c_str());
    EXPECT_STREQ("Names", spec.GetProperties()[1]->GetPropertyName().c_str());
    EXPECT_FALSE(spec.GetIncludeAllProperties());
    EXPECT_FALSE(spec.GetIncludeNoProperties());

    EXPECT_EQ(RelationshipMeaning::SameInstance, spec.GetRelationshipMeaning());
    EXPECT_TRUE(spec.IsPolymorphic());
    EXPECT_TRUE(spec.ShouldAutoExpand());
    EXPECT_EQ(1, spec.GetNestedRelatedProperties().size());

    EXPECT_STREQ("Related:Class,Names", spec.GetRelatedClassNames().c_str());
    EXPECT_STREQ("Relationship:Names", spec.GetRelationshipClassNames().c_str());
    EXPECT_EQ(RequiredRelationDirection_Backward, spec.GetRequiredRelationDirection());

    EXPECT_TRUE(nullptr == spec.GetPropertiesSource());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "relationships": {"schemaName": "Relationship", "classNames": ["Names"]},
        "relatedClasses": [{"schemaName": "Related", "classNames": ["Class", "Names"]}],
        "requiredDirection": "Backward",
        "propertiesSource": {
            "relationship": {"schemaName": "Relationship", "className": "Name"},
            "direction": "Backward",
            "targetClass": {"schemaName": "Target", "className": "Class"}
        },
        "properties": ["Property", "Names"],
        "relationshipMeaning": "SameInstance",
        "isPolymorphic": true,
        "nestedRelatedProperties": [{}],
        "autoExpand": true
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RelatedPropertiesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    
    ASSERT_EQ(2, spec.GetProperties().size());
    EXPECT_STREQ("Property", spec.GetProperties()[0]->GetPropertyName().c_str());
    EXPECT_STREQ("Names", spec.GetProperties()[1]->GetPropertyName().c_str());
    EXPECT_FALSE(spec.GetIncludeAllProperties());
    EXPECT_FALSE(spec.GetIncludeNoProperties());

    EXPECT_EQ(RelationshipMeaning::SameInstance, spec.GetRelationshipMeaning());
    EXPECT_TRUE(spec.IsPolymorphic());
    EXPECT_TRUE(spec.ShouldAutoExpand());
    EXPECT_EQ(1, spec.GetNestedRelatedProperties().size());

    EXPECT_TRUE(spec.GetRelatedClassNames().empty());
    EXPECT_TRUE(spec.GetRelationshipClassNames().empty());
    EXPECT_EQ(RequiredRelationDirection_Both, spec.GetRequiredRelationDirection());

    ASSERT_TRUE(nullptr != spec.GetPropertiesSource());
    ASSERT_EQ(1, spec.GetPropertiesSource()->GetSteps().size());
    EXPECT_STREQ("Relationship:Name", spec.GetPropertiesSource()->GetSteps().front()->GetRelationshipClassName().c_str());
    EXPECT_EQ(RequiredRelationDirection_Backward, spec.GetPropertiesSource()->GetSteps().front()->GetRelationDirection());
    EXPECT_STREQ("Target:Class", spec.GetPropertiesSource()->GetSteps().front()->GetTargetClassName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromJsonWithSpecifiedPropertiesSpecAsObjects)
    {
    static Utf8CP jsonString = R"({
        "properties": [{
            "name": "Prop1"
        }, {
            "name": "Prop2"
        }]
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RelatedPropertiesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    ASSERT_EQ(2, spec.GetProperties().size());
    EXPECT_STREQ("Prop1", spec.GetProperties()[0]->GetPropertyName().c_str());
    EXPECT_STREQ("Prop2", spec.GetProperties()[1]->GetPropertyName().c_str());
    EXPECT_FALSE(spec.GetIncludeAllProperties());
    EXPECT_FALSE(spec.GetIncludeNoProperties());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromJsonWithEmptyPropertiesSpec)
    {
    static Utf8CP jsonString = R"({
        "properties": []
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RelatedPropertiesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    EXPECT_TRUE(spec.GetProperties().empty());
    EXPECT_TRUE(spec.GetIncludeAllProperties());
    EXPECT_FALSE(spec.GetIncludeNoProperties());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromJsonWithNonePropertiesSpec)
    {
    static Utf8CP jsonString = R"({
        "properties": "_none_"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RelatedPropertiesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    EXPECT_TRUE(spec.GetProperties().empty());
    EXPECT_FALSE(spec.GetIncludeAllProperties());
    EXPECT_TRUE(spec.GetIncludeNoProperties());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromJsonWithSpecifiedDeprecatedPropertyNamesSpec)
    {
    static Utf8CP jsonString = R"({
        "propertyNames": ["Prop1", "Prop2"]
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RelatedPropertiesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    ASSERT_EQ(2, spec.GetProperties().size());
    EXPECT_STREQ("Prop1", spec.GetProperties()[0]->GetPropertyName().c_str());
    EXPECT_STREQ("Prop2", spec.GetProperties()[1]->GetPropertyName().c_str());
    EXPECT_FALSE(spec.GetIncludeAllProperties());
    EXPECT_FALSE(spec.GetIncludeNoProperties());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromJsonWithEmptyDeprecatedPropertyNamesSpec)
    {
    static Utf8CP jsonString = R"({
        "propertyNames": []
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());
    
    RelatedPropertiesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    EXPECT_TRUE(spec.GetProperties().empty());
    EXPECT_TRUE(spec.GetIncludeAllProperties());
    EXPECT_FALSE(spec.GetIncludeNoProperties());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                       Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromJsonWithNoneDeprecatedPropertyNamesSpec)
    {
    static Utf8CP jsonString = R"({
        "propertyNames": "_none_"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RelatedPropertiesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    EXPECT_TRUE(spec.GetProperties().empty());
    EXPECT_FALSE(spec.GetIncludeAllProperties());
    EXPECT_TRUE(spec.GetIncludeNoProperties());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = "{}";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    RelatedPropertiesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("", spec.GetRelatedClassNames().c_str());
    EXPECT_STREQ("", spec.GetRelationshipClassNames().c_str());
    EXPECT_TRUE(spec.GetProperties().empty());
    EXPECT_TRUE(spec.GetIncludeAllProperties());
    EXPECT_FALSE(spec.GetIncludeNoProperties());
    EXPECT_EQ(RequiredRelationDirection_Both, spec.GetRequiredRelationDirection());
    EXPECT_EQ(RelationshipMeaning::RelatedInstance, spec.GetRelationshipMeaning());
    EXPECT_FALSE(spec.IsPolymorphic());
    EXPECT_FALSE(spec.ShouldAutoExpand());
    EXPECT_EQ(0, spec.GetNestedRelatedProperties().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, WriteToJsonDeprecated)
    {
    RelatedPropertiesSpecification spec(RequiredRelationDirection_Backward, "s1:c1", "s2:c2,c3", {new PropertySpecification("p1"), new PropertySpecification("p2")},
        RelationshipMeaning::SameInstance, true, true);
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "requiredDirection": "Backward",
        "relationships": {"schemaName": "s1", "classNames": ["c1"]},
        "relatedClasses": {"schemaName": "s2", "classNames": ["c2", "c3"]},
        "relationshipMeaning": "SameInstance",
        "properties": [{
            "name": "p1"
        }, {
            "name": "p2"
        }],
        "isPolymorphic": true,
        "autoExpand": true
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, WriteToJson)
    {
    RelatedPropertiesSpecification spec(*new RelationshipPathSpecification(*new RelationshipStepSpecification("s1:c1", RequiredRelationDirection_Backward, "s2:c2")), 
        {new PropertySpecification("p1"), new PropertySpecification("p2")}, RelationshipMeaning::SameInstance, true, true);
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "propertiesSource": {
            "relationship": {"schemaName": "s1", "className": "c1"},
            "direction": "Backward",
            "targetClass": {"schemaName": "s2", "className": "c2"}
        },
        "relationshipMeaning": "SameInstance",
        "properties": [{
            "name": "p1"
        }, {
            "name": "p2"
        }],
        "isPolymorphic": true,
        "autoExpand": true
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, WriteToJsonWithPropertyNamesSetToNone)
    {
    RelatedPropertiesSpecification spec;
    spec.SetIncludeNoProperties();
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "properties": "_none_"
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <RelatedProperties RelationshipClassNames="Relationship:Names" 
            RelatedClassNames="Related:Class,Names"
            RequiredDirection="Backward" 
            PropertyNames="Property,Names"
            RelationshipMeaning="SameInstance"
            IsPolymorphic="True">
            <RelatedProperties  />
        </RelatedProperties>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    RelatedPropertiesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("Related:Class,Names", spec.GetRelatedClassNames().c_str());
    EXPECT_STREQ("Relationship:Names", spec.GetRelationshipClassNames().c_str());
    ASSERT_EQ(2, spec.GetProperties().size());
    EXPECT_STREQ("Property", spec.GetProperties()[0]->GetPropertyName().c_str());
    EXPECT_STREQ("Names", spec.GetProperties()[1]->GetPropertyName().c_str());
    EXPECT_FALSE(spec.GetIncludeAllProperties());
    EXPECT_FALSE(spec.GetIncludeNoProperties());
    EXPECT_EQ(RequiredRelationDirection_Backward, spec.GetRequiredRelationDirection());
    EXPECT_EQ(RelationshipMeaning::SameInstance, spec.GetRelationshipMeaning());
    EXPECT_TRUE(spec.IsPolymorphic());
    EXPECT_EQ(1, spec.GetNestedRelatedProperties().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = "<RelatedProperties />";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    RelatedPropertiesSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("", spec.GetRelatedClassNames().c_str());
    EXPECT_STREQ("", spec.GetRelationshipClassNames().c_str());
    EXPECT_TRUE(spec.GetProperties().empty());
    EXPECT_TRUE(spec.GetIncludeAllProperties());
    EXPECT_FALSE(spec.GetIncludeNoProperties());
    EXPECT_EQ(RequiredRelationDirection_Both, spec.GetRequiredRelationDirection());
    EXPECT_EQ(RelationshipMeaning::RelatedInstance, spec.GetRelationshipMeaning());
    EXPECT_FALSE(spec.IsPolymorphic());
    EXPECT_EQ(0, spec.GetNestedRelatedProperties().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, WritesToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    RelatedPropertiesSpecification spec(RequiredRelationDirection_Backward, "Relationship:Names",
        "", "", RelationshipMeaning::SameInstance);
    spec.SetRelatedClassNames("Related:Class,Names");
    spec.AddProperty(*new PropertySpecification("Property"));
    spec.AddProperty(*new PropertySpecification("Names"));
    spec.SetRelationshipMeaning(RelationshipMeaning::RelatedInstance);
    spec.SetIsPolymorphic(true);
    spec.SetAutoExpand(true);
    spec.AddNestedRelatedProperty(*new RelatedPropertiesSpecification());
    spec.WriteXml(xml->GetRootElement());
    
    static Utf8CP expected = ""
        "<Root>"
            "<RelatedProperties "
                R"(RelationshipClassNames="Relationship:Names" )"
                R"(RelatedClassNames="Related:Class,Names" )"
                R"(PropertyNames="Property,Names" )"
                R"(RequiredDirection="Backward" )"
                R"(RelationshipMeaning="RelatedInstance" )"
                R"(IsPolymorphic="true" )"
                R"(AutoExpand="true">)"
                "<RelatedProperties "
                    R"(RelationshipClassNames="" )"
                    R"(RelatedClassNames="" )"
                    R"(PropertyNames="" )"
                    R"(RequiredDirection="Both" )"
                    R"(RelationshipMeaning="RelatedInstance" )"
                    R"(IsPolymorphic="false" )"
                    R"(AutoExpand="false" />)"
            "</RelatedProperties>"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, ComputesCorrectHashesDeprecated)
    {
    RelatedPropertiesSpecification spec1(RequiredRelationDirection_Backward, "Relationship:Names",
        "Related:Class,Names", { new PropertySpecification("p1"), new PropertySpecification("p2") }, RelationshipMeaning::RelatedInstance, true);
    spec1.AddNestedRelatedProperty(*new RelatedPropertiesSpecification());

    RelatedPropertiesSpecification spec2(RequiredRelationDirection_Backward, "Relationship:Names",
        "Related:Class,Names", { new PropertySpecification("p1"), new PropertySpecification("p2") }, RelationshipMeaning::RelatedInstance, true);
    spec2.AddNestedRelatedProperty(*new RelatedPropertiesSpecification());

    RelatedPropertiesSpecification spec3(RequiredRelationDirection_Backward, "Relationship:Names",
        "Related:Class,Names", { new PropertySpecification("p1"), new PropertySpecification("p2") }, RelationshipMeaning::RelatedInstance);
    spec3.AddNestedRelatedProperty(*new RelatedPropertiesSpecification());
    spec3.AddNestedRelatedProperty(*new RelatedPropertiesSpecification());    

    // Hashes are same for specifications with same properties
    EXPECT_STREQ(spec1.GetHash().c_str(), spec2.GetHash().c_str());
    // Hashes differs for specifications with different properties
    EXPECT_STRNE(spec1.GetHash().c_str(), spec3.GetHash().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2020
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, ComputesCorrectHashes)
    {
    RelatedPropertiesSpecification spec1(*new RelationshipPathSpecification(*new RelationshipStepSpecification("Relationship:Name", RequiredRelationDirection_Backward, "Related:ClassName")),
        {new PropertySpecification("p1"), new PropertySpecification("p2")}, RelationshipMeaning::RelatedInstance, true);
    spec1.AddNestedRelatedProperty(*new RelatedPropertiesSpecification());

    RelatedPropertiesSpecification spec2(*new RelationshipPathSpecification(*new RelationshipStepSpecification("Relationship:Name", RequiredRelationDirection_Backward, "Related:ClassName")),
        {new PropertySpecification("p1"), new PropertySpecification("p2")}, RelationshipMeaning::RelatedInstance, true);
    spec2.AddNestedRelatedProperty(*new RelatedPropertiesSpecification());

    RelatedPropertiesSpecification spec3(*new RelationshipPathSpecification(*new RelationshipStepSpecification("Relationship:Name2", RequiredRelationDirection_Backward, "Related:ClassName2")),
        {new PropertySpecification("p1"), new PropertySpecification("p2")}, RelationshipMeaning::RelatedInstance);
    spec3.AddNestedRelatedProperty(*new RelatedPropertiesSpecification());
    spec3.AddNestedRelatedProperty(*new RelatedPropertiesSpecification());

    // Hashes are same for specifications with same properties
    EXPECT_STREQ(spec1.GetHash().c_str(), spec2.GetHash().c_str());
    // Hashes differs for specifications with different properties
    EXPECT_STRNE(spec1.GetHash().c_str(), spec3.GetHash().c_str());
    }