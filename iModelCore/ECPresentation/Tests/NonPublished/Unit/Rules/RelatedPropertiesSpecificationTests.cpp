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
struct RelatedPropertiesSpecificationsTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @betest
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
        "nestedRelatedProperties": [{
            "relationships": {"schemaName": "A", "classNames": ["B"]},
            "relatedClasses": [{"schemaName": "C", "classNames": ["D", "E"]}],
            "requiredDirection": "Backward"
        }],
        "autoExpand": true
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RelatedPropertiesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));

    ASSERT_EQ(2, spec.GetProperties().size());
    EXPECT_STREQ("Property", spec.GetProperties()[0]->GetPropertyName().c_str());
    EXPECT_STREQ("Names", spec.GetProperties()[1]->GetPropertyName().c_str());

    EXPECT_EQ(RelationshipMeaning::SameInstance, spec.GetRelationshipMeaning());
    EXPECT_TRUE(spec.IsPolymorphic());
    EXPECT_TRUE(spec.ShouldAutoExpand());
    EXPECT_FALSE(spec.ShouldSkipIfDuplicate());
    EXPECT_EQ(1, spec.GetNestedRelatedProperties().size());

    EXPECT_STREQ("Related:Class,Names", spec.GetRelatedClassNames().c_str());
    EXPECT_STREQ("Relationship:Names", spec.GetRelationshipClassNames().c_str());
    EXPECT_EQ(RequiredRelationDirection_Backward, spec.GetRequiredRelationDirection());

    EXPECT_TRUE(nullptr == spec.GetPropertiesSource());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "relationships": {"schemaName": "Relationship", "classNames": ["Names"]},
        "relatedClasses": [{"schemaName": "Related", "classNames": ["Class", "Names"]}],
        "instanceFilter": "xxx",
        "requiredDirection": "Backward",
        "propertiesSource": {
            "relationship": {"schemaName": "Relationship", "className": "Name"},
            "direction": "Backward",
            "targetClass": {"schemaName": "Target", "className": "Class"}
        },
        "properties": ["Property", "Names"],
        "relationshipMeaning": "SameInstance",
        "handleTargetClassPolymorphically": true,
        "nestedRelatedProperties": [{
            "propertiesSource": {
                "relationship": {"schemaName": "A", "className": "B"},
                "direction": "Forward"
            }
        }],
        "autoExpand": true,
        "skipIfDuplicate": true,
        "relationshipProperties": ["Relationship", "Properties"],
        "forceCreateRelationshipCategory": true
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RelatedPropertiesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));

    ASSERT_EQ(2, spec.GetProperties().size());
    EXPECT_STREQ("Property", spec.GetProperties()[0]->GetPropertyName().c_str());
    EXPECT_STREQ("Names", spec.GetProperties()[1]->GetPropertyName().c_str());

    EXPECT_EQ(RelationshipMeaning::SameInstance, spec.GetRelationshipMeaning());
    EXPECT_STREQ("xxx", spec.GetInstanceFilter().c_str());
    EXPECT_TRUE(spec.IsPolymorphic());
    EXPECT_TRUE(spec.ShouldAutoExpand());
    EXPECT_TRUE(spec.ShouldSkipIfDuplicate());
    EXPECT_EQ(1, spec.GetNestedRelatedProperties().size());

    EXPECT_TRUE(spec.GetRelatedClassNames().empty());
    EXPECT_TRUE(spec.GetRelationshipClassNames().empty());
    EXPECT_EQ(RequiredRelationDirection_Both, spec.GetRequiredRelationDirection());

    ASSERT_TRUE(nullptr != spec.GetPropertiesSource());
    ASSERT_EQ(1, spec.GetPropertiesSource()->GetSteps().size());
    EXPECT_STREQ("Relationship:Name", spec.GetPropertiesSource()->GetSteps().front()->GetRelationshipClassName().c_str());
    EXPECT_EQ(RequiredRelationDirection_Backward, spec.GetPropertiesSource()->GetSteps().front()->GetRelationDirection());
    EXPECT_STREQ("Target:Class", spec.GetPropertiesSource()->GetSteps().front()->GetTargetClassName().c_str());

    ASSERT_EQ(2, spec.GetRelationshipProperties().size());
    EXPECT_STREQ("Relationship", spec.GetRelationshipProperties()[0]->GetPropertyName().c_str());
    EXPECT_STREQ("Properties", spec.GetRelationshipProperties()[1]->GetPropertyName().c_str());
    EXPECT_TRUE(spec.ShouldForceCreateRelationshipCategory());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromJsonWithSpecifiedPropertiesSpecAsObjects)
    {
    static Utf8CP jsonString = R"({
        "propertiesSource": {
            "relationship": {"schemaName": "A", "className": "B"},
            "direction": "Forward"
        },
        "properties": [{
            "name": "Prop1"
        }, {
            "name": "Prop2"
        }],
        "relationshipProperties": [{
            "name": "RelProp1"
        }, {
            "name": "RelProp2"
        }]
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RelatedPropertiesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    ASSERT_EQ(2, spec.GetProperties().size());
    EXPECT_STREQ("Prop1", spec.GetProperties()[0]->GetPropertyName().c_str());
    EXPECT_STREQ("Prop2", spec.GetProperties()[1]->GetPropertyName().c_str());
    ASSERT_EQ(2, spec.GetRelationshipProperties().size());
    EXPECT_STREQ("RelProp1", spec.GetRelationshipProperties()[0]->GetPropertyName().c_str());
    EXPECT_STREQ("RelProp2", spec.GetRelationshipProperties()[1]->GetPropertyName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromJsonWithPropertiesSortedByOverridesPriority)
    {
    static Utf8CP jsonString = R"({
        "propertiesSource": {
            "relationship": {"schemaName": "A", "className": "B"},
            "direction": "Forward"
        },
        "properties": [{
            "name": "P1"
        }, {
            "name": "P2",
            "overridesPriority": 2000
        }, {
            "name": "P3",
            "overridesPriority": 0
        }, {
            "name": "P4"
        }],
        "relationshipProperties": [{
            "name": "RP1"
        }, {
            "name": "RP2",
            "overridesPriority": 2000
        }, {
            "name": "RP3",
            "overridesPriority": 0
        }, {
            "name": "RP4"
        }]
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RelatedPropertiesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    ASSERT_EQ(4, spec.GetProperties().size());
    EXPECT_STREQ("P2", spec.GetProperties()[0]->GetPropertyName().c_str());
    EXPECT_STREQ("P1", spec.GetProperties()[1]->GetPropertyName().c_str());
    EXPECT_STREQ("P4", spec.GetProperties()[2]->GetPropertyName().c_str());
    EXPECT_STREQ("P3", spec.GetProperties()[3]->GetPropertyName().c_str());
    ASSERT_EQ(4, spec.GetRelationshipProperties().size());
    EXPECT_STREQ("RP2", spec.GetRelationshipProperties()[0]->GetPropertyName().c_str());
    EXPECT_STREQ("RP1", spec.GetRelationshipProperties()[1]->GetPropertyName().c_str());
    EXPECT_STREQ("RP4", spec.GetRelationshipProperties()[2]->GetPropertyName().c_str());
    EXPECT_STREQ("RP3", spec.GetRelationshipProperties()[3]->GetPropertyName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromJsonWithSpecifiedPropertiesSpecAsObjectsWithStar)
    {
    static Utf8CP jsonString = R"({
        "propertiesSource": {
            "relationship": {"schemaName": "A", "className": "B"},
            "direction": "Forward"
        },
        "properties": [
            "*",
            {"name": "Prop"}
        ],
        "relationshipProperties": [
            "*",
            {"name": "RelProp"}
        ]
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RelatedPropertiesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    ASSERT_EQ(2, spec.GetProperties().size());
    EXPECT_STREQ("*", spec.GetProperties()[0]->GetPropertyName().c_str());
    EXPECT_STREQ("Prop", spec.GetProperties()[1]->GetPropertyName().c_str());
    ASSERT_EQ(2, spec.GetRelationshipProperties().size());
    EXPECT_STREQ("*", spec.GetRelationshipProperties()[0]->GetPropertyName().c_str());
    EXPECT_STREQ("RelProp", spec.GetRelationshipProperties()[1]->GetPropertyName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromJsonWithEmptyPropertiesSpec)
    {
    static Utf8CP jsonString = R"({
        "propertiesSource": {
            "relationship": {"schemaName": "A", "className": "B"},
            "direction": "Forward"
        },
        "properties": [],
        "relationshipProperties": []
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RelatedPropertiesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    ASSERT_EQ(0, spec.GetProperties().size());
    ASSERT_EQ(0, spec.GetRelationshipProperties().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromJsonWithNonePropertiesSpec)
    {
    static Utf8CP jsonString = R"({
        "propertiesSource": {
            "relationship": {"schemaName": "A", "className": "B"},
            "direction": "Forward"
        },
        "properties": "_none_",
        "relationshipProperties": "_none_"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RelatedPropertiesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    ASSERT_EQ(0, spec.GetProperties().size());
    ASSERT_EQ(0, spec.GetRelationshipProperties().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromJsonWithAllPropertiesSpec)
    {
    static Utf8CP jsonString = R"({
        "propertiesSource": {
            "relationship": {"schemaName": "A", "className": "B"},
            "direction": "Forward"
        },
        "properties": "*",
        "relationshipProperties": "*"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RelatedPropertiesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    EXPECT_EQ(1, spec.GetProperties().size());
    EXPECT_STREQ("*", spec.GetProperties()[0]->GetPropertyName().c_str());
    EXPECT_STREQ("*", spec.GetRelationshipProperties()[0]->GetPropertyName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromJsonWithSpecifiedDeprecatedPropertyNamesSpec)
    {
    static Utf8CP jsonString = R"({
        "propertiesSource": {
            "relationship": {"schemaName": "A", "className": "B"},
            "direction": "Forward"
        },
        "propertyNames": ["Prop1", "Prop2"]
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RelatedPropertiesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    ASSERT_EQ(2, spec.GetProperties().size());
    EXPECT_STREQ("Prop1", spec.GetProperties()[0]->GetPropertyName().c_str());
    EXPECT_STREQ("Prop2", spec.GetProperties()[1]->GetPropertyName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromJsonWithEmptyDeprecatedPropertyNamesSpec)
    {
    static Utf8CP jsonString = R"({
        "propertiesSource": {
            "relationship": {"schemaName": "A", "className": "B"},
            "direction": "Forward"
        },
        "propertyNames": []
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RelatedPropertiesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    ASSERT_EQ(0, spec.GetProperties().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromJsonWithNoneDeprecatedPropertyNamesSpec)
    {
    static Utf8CP jsonString = R"({
        "propertiesSource": {
            "relationship": {"schemaName": "A", "className": "B"},
            "direction": "Forward"
        },
        "propertyNames": "_none_"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());

    RelatedPropertiesSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    ASSERT_EQ(0, spec.GetProperties().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "propertiesSource": {
            "relationship": {"schemaName": "A", "className": "B"},
            "direction": "Forward"
        }
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    RelatedPropertiesSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("", spec.GetRelatedClassNames().c_str());
    EXPECT_STREQ("", spec.GetRelationshipClassNames().c_str());
    EXPECT_EQ(1, spec.GetProperties().size());
    EXPECT_STREQ("*", spec.GetProperties()[0]->GetPropertyName().c_str());
    EXPECT_EQ(RequiredRelationDirection_Both, spec.GetRequiredRelationDirection());
    EXPECT_EQ(RelationshipMeaning::RelatedInstance, spec.GetRelationshipMeaning());
    EXPECT_FALSE(spec.IsPolymorphic());
    EXPECT_FALSE(spec.ShouldAutoExpand());
    EXPECT_EQ(0, spec.GetNestedRelatedProperties().size());
    EXPECT_EQ(0, spec.GetRelationshipProperties().size());
    EXPECT_FALSE(spec.ShouldForceCreateRelationshipCategory());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, WriteToJsonDeprecated)
    {
    PropertySpecificationsList list =  {new PropertySpecification("p1"), new PropertySpecification(Utf8String("p2"))};
    RelatedPropertiesSpecification spec(RequiredRelationDirection_Backward, "s1:c1", "s2:c2,c3", list,
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
        "handleTargetClassPolymorphically": true,
        "autoExpand": true
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, WriteToJson)
    {
    RelatedPropertiesSpecification spec(*new RelationshipPathSpecification(*new RelationshipStepSpecification("s1:c1", RequiredRelationDirection_Backward, "s2:c2")),
        {new PropertySpecification("p1"), new PropertySpecification("p2")}, RelationshipMeaning::SameInstance, true, true, false,
        {new PropertySpecification("rp1"), new PropertySpecification("rp2")}, true);
    spec.SetInstanceFilter("xxx");
    spec.SetSkipIfDuplicate(true);
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "propertiesSource": {
            "relationship": {"schemaName": "s1", "className": "c1"},
            "direction": "Backward",
            "targetClass": {"schemaName": "s2", "className": "c2"}
        },
        "instanceFilter": "xxx",
        "relationshipMeaning": "SameInstance",
        "properties": [{
            "name": "p1"
        }, {
            "name": "p2"
        }],
        "handleTargetClassPolymorphically": true,
        "autoExpand": true,
        "skipIfDuplicate": true,
        "relationshipProperties": [{
            "name": "rp1"
        }, {
            "name": "rp2"
        }],
        "forceCreateRelationshipCategory": true
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, AddsPropertiesSortedByOverridesPriority)
    {
    RelatedPropertiesSpecification spec;

    PropertySpecificationP p1 = new PropertySpecification("P1");
    PropertySpecificationP p2 = new PropertySpecification("P2", 2000);
    PropertySpecificationP p3 = new PropertySpecification("P3", 0);
    PropertySpecificationP p4 = new PropertySpecification("P4");

    spec.AddProperty(*p1);
    spec.AddProperty(*p2);
    spec.AddProperty(*p3);
    spec.AddProperty(*p4);

    ASSERT_EQ(4, spec.GetProperties().size());
    EXPECT_STREQ("P2", spec.GetProperties()[0]->GetPropertyName().c_str());
    EXPECT_STREQ("P1", spec.GetProperties()[1]->GetPropertyName().c_str());
    EXPECT_STREQ("P4", spec.GetProperties()[2]->GetPropertyName().c_str());
    EXPECT_STREQ("P3", spec.GetProperties()[3]->GetPropertyName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, AddsRelationshipPropertiesSortedByOverridesPriority)
    {
    RelatedPropertiesSpecification spec;

    PropertySpecificationP p1 = new PropertySpecification("P1");
    PropertySpecificationP p2 = new PropertySpecification("P2", 2000);
    PropertySpecificationP p3 = new PropertySpecification("P3", 0);
    PropertySpecificationP p4 = new PropertySpecification("P4");

    spec.AddRelationshipProperty(*p1);
    spec.AddRelationshipProperty(*p2);
    spec.AddRelationshipProperty(*p3);
    spec.AddRelationshipProperty(*p4);

    ASSERT_EQ(4, spec.GetRelationshipProperties().size());
    EXPECT_STREQ("P2", spec.GetRelationshipProperties()[0]->GetPropertyName().c_str());
    EXPECT_STREQ("P1", spec.GetRelationshipProperties()[1]->GetPropertyName().c_str());
    EXPECT_STREQ("P4", spec.GetRelationshipProperties()[2]->GetPropertyName().c_str());
    EXPECT_STREQ("P3", spec.GetRelationshipProperties()[3]->GetPropertyName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, WriteToJsonWithNoPropertiesIncluded)
    {
    RelatedPropertiesSpecification spec;
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "properties": "_none_"
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, WriteToJsonWithAllPropertiesIncluded)
    {
    RelatedPropertiesSpecification spec;
    auto propSpec = new PropertySpecification("*");
    spec.AddProperty(*propSpec);
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, WriteToJsonWithAllIncludedPropertiesWithOverrides)
    {
    RelatedPropertiesSpecification spec;
    spec.AddProperty(*new PropertySpecification("*", 2000, "newLabel"));
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "properties": [{"name": "*", "overridesPriority": 2000, "labelOverride": "newLabel"}]
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, WriteToJsonWithAllRelationshipPropertiesIncluded)
    {
    RelatedPropertiesSpecification spec;
    auto relPropSpec = new PropertySpecification("*");
    spec.AddRelationshipProperty(*relPropSpec);
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "properties": "_none_",
        "relationshipProperties": "*"
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, WriteToJsonWithAllIncludedRelationshipPropertiesWithOverrides)
    {
    RelatedPropertiesSpecification spec;
    spec.AddRelationshipProperty(*new PropertySpecification("*", 2000, "newLabel"));
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "properties": "_none_",
        "relationshipProperties": [{"name": "*", "overridesPriority": 2000, "labelOverride": "newLabel"}]
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
    EXPECT_EQ(RequiredRelationDirection_Backward, spec.GetRequiredRelationDirection());
    EXPECT_EQ(RelationshipMeaning::SameInstance, spec.GetRelationshipMeaning());
    EXPECT_TRUE(spec.IsPolymorphic());
    EXPECT_EQ(1, spec.GetNestedRelatedProperties().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
    EXPECT_EQ(1, spec.GetProperties().size());
    EXPECT_STREQ("*", spec.GetProperties()[0]->GetPropertyName().c_str());
    EXPECT_EQ(RequiredRelationDirection_Both, spec.GetRequiredRelationDirection());
    EXPECT_EQ(RelationshipMeaning::RelatedInstance, spec.GetRelationshipMeaning());
    EXPECT_FALSE(spec.IsPolymorphic());
    EXPECT_EQ(0, spec.GetNestedRelatedProperties().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, WritesToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    RelatedPropertiesSpecification spec;
    spec.SetRequiredRelationDirection(RequiredRelationDirection_Backward);
    spec.SetRelationshipClassNames("Relationship:Names");
    spec.SetRelationshipMeaning(RelationshipMeaning::SameInstance);
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
                    R"(PropertyNames="_none_" )"
                    R"(RequiredDirection="Both" )"
                    R"(RelationshipMeaning="RelatedInstance" )"
                    R"(IsPolymorphic="false" )"
                    R"(AutoExpand="false" />)"
            "</RelatedProperties>"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelatedPropertiesSpecificationsTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "d9c2317e3b3fdd54fe97a3d6b5c346da";

    // Make sure that introducing additional attributes with default values don't affect the hash
    RelatedPropertiesSpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    RelatedPropertiesSpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // then we test that each attribute affects the hash
    RelatedPropertiesSpecification specWithRequiredDirection(defaultSpec);
    specWithRequiredDirection.SetRequiredRelationDirection(RequiredRelationDirection_Backward);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithRequiredDirection.GetHash().c_str());
    specWithRequiredDirection.SetRequiredRelationDirection(RequiredRelationDirection_Both);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithRequiredDirection.GetHash().c_str());

    RelatedPropertiesSpecification specWithRelationshipClassNames(defaultSpec);
    specWithRelationshipClassNames.SetRelationshipClassNames("names");
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithRelationshipClassNames.GetHash().c_str());
    specWithRelationshipClassNames.SetRelationshipClassNames("");
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithRelationshipClassNames.GetHash().c_str());

    RelatedPropertiesSpecification specWithRelatedClassNames(defaultSpec);
    specWithRelatedClassNames.SetRelatedClassNames("names");
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithRelatedClassNames.GetHash().c_str());
    specWithRelatedClassNames.SetRelatedClassNames("");
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithRelatedClassNames.GetHash().c_str());

    RelatedPropertiesSpecification specWithPropertiesSource(defaultSpec);
    specWithPropertiesSource.SetPropertiesSource(new RelationshipPathSpecification());
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithPropertiesSource.GetHash().c_str());
    specWithPropertiesSource.SetPropertiesSource(nullptr);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithPropertiesSource.GetHash().c_str());

    RelatedPropertiesSpecification specWithInstanceFilter(defaultSpec);
    specWithInstanceFilter.SetInstanceFilter("xxx");
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithInstanceFilter.GetHash().c_str());
    specWithInstanceFilter.SetInstanceFilter("");
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithInstanceFilter.GetHash().c_str());

    RelatedPropertiesSpecification specWithProperties(defaultSpec);
    specWithProperties.AddProperty(*new PropertySpecification());
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithProperties.GetHash().c_str());
    specWithProperties.SetProperties({});
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithProperties.GetHash().c_str());

    RelatedPropertiesSpecification specWithNestedRelatedProperties(defaultSpec);
    specWithNestedRelatedProperties.AddNestedRelatedProperty(*new RelatedPropertiesSpecification());
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithNestedRelatedProperties.GetHash().c_str());

    RelatedPropertiesSpecification specWithRelationshipMeaning(defaultSpec);
    specWithRelationshipMeaning.SetRelationshipMeaning(RelationshipMeaning::SameInstance);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithRelationshipMeaning.GetHash().c_str());
    specWithRelationshipMeaning.SetRelationshipMeaning(RelationshipMeaning::RelatedInstance);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithRelationshipMeaning.GetHash().c_str());

    RelatedPropertiesSpecification specWithPolymorphic(defaultSpec);
    specWithPolymorphic.SetIsPolymorphic(true);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithPolymorphic.GetHash().c_str());
    specWithPolymorphic.SetIsPolymorphic(false);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithPolymorphic.GetHash().c_str());

    RelatedPropertiesSpecification specWithAutoExpand(defaultSpec);
    specWithAutoExpand.SetAutoExpand(true);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithAutoExpand.GetHash().c_str());
    specWithAutoExpand.SetAutoExpand(false);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithAutoExpand.GetHash().c_str());

    RelatedPropertiesSpecification specWithSkipIfDuplicateFlag(defaultSpec);
    specWithSkipIfDuplicateFlag.SetSkipIfDuplicate(true);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithSkipIfDuplicateFlag.GetHash().c_str());
    specWithSkipIfDuplicateFlag.SetSkipIfDuplicate(false);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithSkipIfDuplicateFlag.GetHash().c_str());

    RelatedPropertiesSpecification specWithRelationshipProperties(defaultSpec);
    specWithRelationshipProperties.AddRelationshipProperty(*new PropertySpecification());
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithRelationshipProperties.GetHash().c_str());
    specWithRelationshipProperties.SetRelationshipProperties({});
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithRelationshipProperties.GetHash().c_str());

    RelatedPropertiesSpecification specWithForceCreateRelationshipCategoryFlag(defaultSpec);
    specWithForceCreateRelationshipCategoryFlag.SetForceCreateRelationshipCategory(true);
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithForceCreateRelationshipCategoryFlag.GetHash().c_str());
    specWithForceCreateRelationshipCategoryFlag.SetForceCreateRelationshipCategory(false);
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithForceCreateRelationshipCategoryFlag.GetHash().c_str());
    }
