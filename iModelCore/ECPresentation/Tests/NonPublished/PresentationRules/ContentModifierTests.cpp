/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentModifierTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentModifierTests, CopyConstructorCopiesProperties)
    {
    // Create modifier1
    ContentModifier modifier1("TestSchema", "TestClassName");
    modifier1.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, "RelationshipClassName", "RelatedClassNames", "Properties", RelationshipMeaning::RelatedInstance));
    modifier1.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("properties", 1000, true));
    modifier1.AddCalculatedProperty(*new CalculatedPropertiesSpecification("label", 0, "Value"));
    modifier1.AddPropertyEditor(*new PropertyEditorsSpecification("property", "editor"));

    // Validate modifier1
    EXPECT_STREQ("TestSchema", modifier1.GetSchemaName().c_str());
    EXPECT_STREQ("TestClassName", modifier1.GetClassName().c_str());
    EXPECT_EQ(1, modifier1.GetCalculatedProperties().size());
    EXPECT_EQ(1, modifier1.GetRelatedProperties().size());
    EXPECT_EQ(1, modifier1.GetPropertiesDisplaySpecifications().size());
    EXPECT_EQ(1, modifier1.GetPropertyEditors().size());

    // Create modifier2 via copy constructor
    ContentModifier modifier2(modifier1);

    // Validate modifier2
    EXPECT_STREQ("TestSchema", modifier2.GetSchemaName().c_str());
    EXPECT_STREQ("TestClassName", modifier2.GetClassName().c_str());
    EXPECT_EQ(1, modifier2.GetCalculatedProperties().size());
    EXPECT_EQ(1, modifier2.GetRelatedProperties().size());
    EXPECT_EQ(1, modifier2.GetPropertiesDisplaySpecifications().size());
    EXPECT_EQ(1, modifier2.GetPropertyEditors().size());
    // Validate properties
    EXPECT_NE(modifier1.GetCalculatedProperties()[0], modifier2.GetCalculatedProperties()[0]);
    EXPECT_NE(modifier1.GetRelatedProperties()[0], modifier2.GetRelatedProperties()[0]);
    EXPECT_NE(modifier1.GetPropertiesDisplaySpecifications()[0], modifier2.GetPropertiesDisplaySpecifications()[0]);
    EXPECT_NE(modifier1.GetPropertyEditors()[0], modifier2.GetPropertyEditors()[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentModifierTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "ContentModifier",
        "class": {"schemaName": "TestSchema", "className": "TestClassName"},
        "relatedProperties": [
            {}
        ],
        "propertiesDisplay": [
            {"propertyNames": ["property names"]}
        ],
        "calculatedProperties": [
            {
            "value": "value",
            "label": "label"
            }
        ],
        "propertyEditors": [
            {
            "propertyName": "property names",
            "editorName": "editor"
            }
        ]
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    ContentModifier modifier;
    EXPECT_TRUE(modifier.ReadJson(json));

    EXPECT_STREQ("TestSchema", modifier.GetSchemaName().c_str());
    EXPECT_STREQ("TestClassName", modifier.GetClassName().c_str());
    EXPECT_EQ(1, modifier.GetCalculatedProperties().size());
    EXPECT_EQ(1, modifier.GetRelatedProperties().size());
    EXPECT_EQ(1, modifier.GetPropertiesDisplaySpecifications().size());
    EXPECT_EQ(1, modifier.GetPropertyEditors().size());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentModifierTests, LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "ruleType": "ContentModifier"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    ContentModifier modifier;
    EXPECT_TRUE(modifier.ReadJson(json));

    EXPECT_STREQ("", modifier.GetSchemaName().c_str());
    EXPECT_STREQ("", modifier.GetClassName().c_str());
    EXPECT_EQ(0, modifier.GetCalculatedProperties().size());
    EXPECT_EQ(0, modifier.GetRelatedProperties().size());
    EXPECT_EQ(0, modifier.GetPropertiesDisplaySpecifications().size());
    EXPECT_EQ(0, modifier.GetPropertyEditors().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentModifierTests, WriteToJson)
    {
    ContentModifier rule("schema", "class");
    rule.AddCalculatedProperty(*new CalculatedPropertiesSpecification());
    rule.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification());
    rule.AddPropertyEditor(*new PropertyEditorsSpecification());
    rule.AddRelatedProperty(*new RelatedPropertiesSpecification());
    Json::Value json = rule.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "ruleType": "ContentModifier",
        "class": {"schemaName": "schema", "className": "class"},
        "calculatedProperties": [{
            "label": "",
            "value": ""
        }],
        "propertyEditors": [{
            "editorName": "",
            "propertyName": ""
        }],
        "relatedProperties": [{
        }],
        "propertiesDisplay": [{
        }]
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentModifierTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <ContentModifier SchemaName="TestSchema" ClassName="TestClassName"> 
            <RelatedProperties RelationshipClassNames="Schema:OnSameElement" RequiredDirection="Forward" RelationshipMeaning="RelatedInstance"/> 
            <HiddenProperties PropertyNames="property" /> 
            <CalculatedProperties> 
                <Property Label="Label1" Priority="1000">Value1</Property> 
            </CalculatedProperties> 
            <PropertyEditors PropertyName="TestProperty" EditorName="TestEditor">
                <Editor PropertyName="TestProperty" EditorName="TestEditor"/>
            </PropertyEditors>
        </ContentModifier> 
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    ContentModifier modifier;
    EXPECT_TRUE(modifier.ReadXml(xml->GetRootElement()));

    EXPECT_STREQ("TestSchema", modifier.GetSchemaName().c_str());
    EXPECT_STREQ("TestClassName", modifier.GetClassName().c_str());
    EXPECT_EQ(1, modifier.GetCalculatedProperties().size());
    EXPECT_EQ(1, modifier.GetRelatedProperties().size());
    EXPECT_EQ(1, modifier.GetPropertiesDisplaySpecifications().size());
    EXPECT_EQ(1, modifier.GetPropertyEditors().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentModifierTests, LoadsFromXmlWithDefaultValues)
    {
    static Utf8CP xmlString = "<ContentModifier/>";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    ContentModifier modifier;
    EXPECT_TRUE(modifier.ReadXml(xml->GetRootElement()));

    EXPECT_STREQ("", modifier.GetSchemaName().c_str());
    EXPECT_STREQ("", modifier.GetClassName().c_str());
    EXPECT_EQ(0, modifier.GetCalculatedProperties().size());
    EXPECT_EQ(0, modifier.GetRelatedProperties().size());
    EXPECT_EQ(0, modifier.GetPropertiesDisplaySpecifications().size());
    EXPECT_EQ(0, modifier.GetPropertyEditors().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentModifierTests, WriteToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    ContentModifier modifier("SchemaName", "ClassName");
    modifier.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, "RelationshipClassName", "RelatedClassNames", "Properties", RelationshipMeaning::RelatedInstance));
    modifier.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("properties", 1000, true));
    modifier.AddCalculatedProperty(*new CalculatedPropertiesSpecification("label", 0, "Value"));
    modifier.AddPropertyEditor(*new PropertyEditorsSpecification("property", "editor"));
    modifier.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<ContentModifier Priority="1000" ClassName="ClassName" SchemaName="SchemaName">)"
                R"(<RelatedProperties RelationshipClassNames="RelationshipClassName" RelatedClassNames="RelatedClassNames" PropertyNames="Properties" RequiredDirection="Forward" RelationshipMeaning="RelatedInstance" IsPolymorphic="false" AutoExpand="false"/>)"
                R"(<DisplayedProperties PropertyNames="properties" Priority="1000"/>)"
                R"(<CalculatedProperties>)"
                    R"(<Property Label="label" Priority="0">Value</Property>)"
                R"(</CalculatedProperties>)"
                R"(<PropertyEditors>)"
                    R"(<Editor PropertyName="property" EditorName="editor"/>)"
                R"(</PropertyEditors>)"
            R"(</ContentModifier>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras               12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentModifierTests, ComputesCorrectHashes)
    {
    ContentModifier modifier1("SchemaName", "ClassName");
    modifier1.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, "RelationshipClassName", "RelatedClassNames", "Properties", RelationshipMeaning::RelatedInstance));
    modifier1.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("properties", 1000, true));
    modifier1.AddCalculatedProperty(*new CalculatedPropertiesSpecification("label", 0, "Value"));
    modifier1.AddPropertyEditor(*new PropertyEditorsSpecification("property", "editor"));
    ContentModifier modifier2("SchemaName", "ClassName");
    modifier2.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, "RelationshipClassName", "RelatedClassNames", "Properties", RelationshipMeaning::RelatedInstance));
    modifier2.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("properties", 1000, true));
    modifier2.AddCalculatedProperty(*new CalculatedPropertiesSpecification("label", 0, "Value"));
    modifier2.AddPropertyEditor(*new PropertyEditorsSpecification("property", "editor"));
    ContentModifier modifier3("SchemaName", "ClassName");

    // Hashes are same for modifier with same properties
    EXPECT_STREQ(modifier1.GetHash().c_str(), modifier2.GetHash().c_str());
    // Hashes differs for modifier with different properties
    EXPECT_STRNE(modifier1.GetHash().c_str(), modifier3.GetHash().c_str());
    }