/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentSpecificationsTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct TestContentSpecification : ContentSpecification
    {
    CharCP _GetXmlElementName() const override {return "TestSpecification";}
    bool _ReadXml(BeXmlNodeP xmlNode) override {return ContentSpecification::_ReadXml(xmlNode);}
    void _WriteXml(BeXmlNodeP xmlNode) const override {ContentSpecification::_WriteXml(xmlNode);}
    Utf8CP _GetJsonElementType() const override {return "testSpecification";}
    bool _ReadJson(JsonValueCR json) override {return ContentSpecification::_ReadJson(json);}
    void _WriteJson(JsonValueR json) const override {ContentSpecification::_WriteJson(json);}
    ContentSpecification* _Clone() const override {return new TestContentSpecification(*this);}
    };

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentSpecificationsTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "specType": "testSpecification",
        "priority": 123,
        "showImages": true,
        "relatedProperties": [{}],
        "propertiesDisplay": [{
            "propertyNames": ["property"]
        }, {
            "propertyNames": ["names"]
        }],
        "calculatedProperties": [{
            "value": "value",
            "label": "label"
        }, {
            "value": "value",
            "label": "label"
        }],
        "propertyEditors": [{
            "propertyName": "property names",
            "editorName": "editor"
        }],
        "relatedInstances": [{ 
            "relationship": {"schemaName": "TestSchema", "className": "TestRelName"},
            "class": {"schemaName": "TestSchema", "className": "TestClassName"},
            "alias":"TestAlias"
        }]
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    ASSERT_FALSE(json.isNull());
    
    TestContentSpecification spec;
    ASSERT_TRUE(spec.ReadJson(json));
    EXPECT_EQ(123, spec.GetPriority());
    EXPECT_TRUE(spec.GetShowImages());
    EXPECT_EQ(2, spec.GetCalculatedProperties().size());
    EXPECT_EQ(2, spec.GetPropertiesDisplaySpecifications().size());
    EXPECT_EQ(1, spec.GetRelatedProperties().size());
    EXPECT_EQ(1, spec.GetPropertyEditors().size());   
    EXPECT_EQ(1, spec.GetRelatedInstances().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentSpecificationsTests, WriteToJson)
    {
    TestContentSpecification spec;
    spec.AddCalculatedProperty(*new CalculatedPropertiesSpecification("label", 456, "value"));
    spec.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("prop1,prop2", 456, false));
    spec.AddPropertyEditor(*new PropertyEditorsSpecification("prop", "editor"));
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Both, "s1:c1", "s2:c2", "alias", true));
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Both, "s3:c3", "s4:c4", "p1,p2", RelationshipMeaning::SameInstance, true));
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "specType": "testSpecification",
        "calculatedProperties": [{
            "priority": 456,
            "value": "value",
            "label": "label"
        }],
        "propertiesDisplay": [{
            "priority": 456,
            "propertyNames": ["prop1", "prop2"],
            "isDisplayed": false
        }],
        "propertyEditors": [{
            "propertyName": "prop",
            "editorName": "editor"
        }],
        "relatedInstances": [{
            "relationship": {"schemaName": "s1", "className": "c1"},
            "class": {"schemaName": "s2", "className": "c2"},
            "alias": "alias",
            "isRequired": true
        }],
        "relatedProperties": [{ 
            "relationships": {"schemaName": "s3", "classNames": ["c3"]},
            "relatedClasses": {"schemaName": "s4", "classNames": ["c4"]},
            "propertyNames": ["p1", "p2"],
            "relationshipMeaning": "SameInstance",
            "isPolymorphic": true
        }]
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentSpecificationsTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <TestSpecification Priority="123" ShowImages="true">
            <DisplayedProperties PropertyNames="Properties" />
            <HiddenProperties PropertyNames="Properties" />
            <RelatedProperties />
            <CalculatedProperties>
                <Property Label="Label1">Expression1</Property>
                <Property Label="Label2">Expression2</Property>
            </CalculatedProperties>
            <PropertyEditors>
               <Editor PropertyName="TestProperty" EditorName="TestEditor" />
            </PropertyEditors>
            <RelatedInstance RelationshipName="TestRelName" ClassName="TestClassName" Alias="TestAlias" />
        </TestSpecification>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    TestContentSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_EQ(123, spec.GetPriority());
    EXPECT_TRUE(spec.GetShowImages());
    EXPECT_EQ(2, spec.GetCalculatedProperties().size());
    EXPECT_EQ(2, spec.GetPropertiesDisplaySpecifications().size());
    EXPECT_EQ(1, spec.GetRelatedProperties().size());
    EXPECT_EQ(1, spec.GetPropertyEditors().size());
    EXPECT_EQ(1, spec.GetRelatedInstances().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentSpecificationsTests, LoadFromXml_NoCalculatedPropertiesLoadedWhenLabelForCalculatedPropertyIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <TestSpecification Priority="123" ShowImages="true">
            <CalculatedProperties>
                <Property>Expression1</Property>
            </CalculatedProperties>
        </TestSpecification>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    TestContentSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_EQ(0, spec.GetCalculatedProperties().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentSpecificationsTests, LoadFromXml_NoCalculatedPropertiesLoadedWhenValueForCalculatedPropertyIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <TestSpecification Priority="123" ShowImages="true">
            <CalculatedProperties>
                <Property Label="Label1"/>
            </CalculatedProperties>
        </TestSpecification>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    TestContentSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_EQ(0, spec.GetCalculatedProperties().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentSpecificationsTests, WritesToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    TestContentSpecification spec;
    spec.SetPriority(123);
    spec.SetShowImages(true);
    spec.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("DisplayedProperty", 123, true));
    spec.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("HiddenProperty", 456, false));
    spec.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, "RelationshipClassName", 
        "RelatedClassNames", "Properties", RelationshipMeaning::SameInstance));
    spec.AddPropertyEditor(*new PropertyEditorsSpecification("Property1", "Editor1"));
    spec.AddCalculatedProperty(*new CalculatedPropertiesSpecification("Label1", 123, "Expression1"));
    spec.AddCalculatedProperty(*new CalculatedPropertiesSpecification("Label2", 456, "Expression2"));
    spec.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Both, "TestRelName", "TestClassName", "TestAlias"));
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<TestSpecification Priority="123" ShowImages="true">)"
                R"(<RelatedProperties RelationshipClassNames="RelationshipClassName" RelatedClassNames="RelatedClassNames" )"
                    R"(PropertyNames="Properties" RequiredDirection="Forward" RelationshipMeaning="SameInstance" IsPolymorphic="false" AutoExpand="false"/>)"
                R"(<DisplayedProperties PropertyNames="DisplayedProperty" Priority="123"/>)"
                R"(<HiddenProperties PropertyNames="HiddenProperty" Priority="456"/>)"
                R"(<RelatedInstance ClassName="TestClassName" RelationshipName="TestRelName" RelationshipDirection="Both" Alias="TestAlias" IsRequired="false" />)"
                R"(<CalculatedProperties>)"
                    R"(<Property Label="Label1" Priority="123">Expression1</Property>)"
                    R"(<Property Label="Label2" Priority="456">Expression2</Property>)"
                R"(</CalculatedProperties>)"
                R"(<PropertyEditors>)"
                    R"(<Editor PropertyName="Property1" EditorName="Editor1"/>)"
                R"(</PropertyEditors>)"
            R"(</TestSpecification>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentSpecificationsTests, CopiedSpecificationHasSameNestedSpecifications)
    {
    // Create spec1
    TestContentSpecification spec1;
    spec1.SetShowImages(true);
    spec1.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("HiddenProperty", 456, false));
    spec1.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, "RelationshipClassName", 
        "RelatedClassNames", "Properties", RelationshipMeaning::SameInstance));
    spec1.AddPropertyEditor(*new PropertyEditorsSpecification("Property1", "Editor1"));
    spec1.AddCalculatedProperty(*new CalculatedPropertiesSpecification("Label1", 123, "Expression1"));

    // Validate spec1
    EXPECT_TRUE(spec1.GetShowImages());
    EXPECT_EQ(1, spec1.GetRelatedProperties().size());
    EXPECT_EQ(1, spec1.GetCalculatedProperties().size());
    EXPECT_EQ(1, spec1.GetPropertiesDisplaySpecifications().size());
    EXPECT_EQ(1, spec1.GetPropertyEditors().size()); 

    // Create spec2 via copy consstructor
    TestContentSpecification spec2(spec1);

    // Validate spec2
    EXPECT_TRUE(spec2.GetShowImages());
    EXPECT_EQ(1, spec2.GetRelatedProperties().size());
    EXPECT_EQ(1, spec2.GetCalculatedProperties().size());
    EXPECT_EQ(1, spec2.GetPropertiesDisplaySpecifications().size());
    EXPECT_EQ(1, spec2.GetPropertyEditors().size());   

    // Validate specifications pointers
    EXPECT_NE(spec1.GetRelatedProperties()[0], spec2.GetRelatedProperties()[0]);
    EXPECT_NE(spec1.GetCalculatedProperties()[0], spec2.GetCalculatedProperties()[0]);
    EXPECT_NE(spec1.GetPropertiesDisplaySpecifications()[0], spec2.GetPropertiesDisplaySpecifications()[0]);
    EXPECT_NE(spec1.GetPropertyEditors()[0], spec2.GetPropertyEditors()[0]);   
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentSpecificationsTests, ComputesCorrectHashes)
    {
    TestContentSpecification spec1;
    spec1.SetShowImages(true);
    spec1.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("HiddenProperty", 456, false));
    spec1.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, "RelationshipClassName", 
        "RelatedClassNames", "Properties", RelationshipMeaning::SameInstance));
    spec1.AddPropertyEditor(*new PropertyEditorsSpecification("Property1", "Editor1"));
    spec1.AddCalculatedProperty(*new CalculatedPropertiesSpecification("Label1", 123, "Expression1"));
    spec1.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Both, "TestRelName", "TestClassName", "TestAlias"));
    TestContentSpecification spec2;
    spec2.SetShowImages(true);
    spec2.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("HiddenProperty", 456, false));
    spec2.AddRelatedProperty(*new RelatedPropertiesSpecification(RequiredRelationDirection_Forward, "RelationshipClassName", 
        "RelatedClassNames", "Properties", RelationshipMeaning::SameInstance));
    spec2.AddPropertyEditor(*new PropertyEditorsSpecification("Property1", "Editor1"));
    spec2.AddCalculatedProperty(*new CalculatedPropertiesSpecification("Label1", 123, "Expression1"));
    spec2.AddRelatedInstance(*new RelatedInstanceSpecification(RequiredRelationDirection_Both, "TestRelName", "TestClassName", "TestAlias"));
    TestContentSpecification spec3;
    spec3.AddPropertiesDisplaySpecification(*new PropertiesDisplaySpecification("HiddenProperty", 456, false));

    // Hashes are same for identical specifications
    EXPECT_STREQ(spec1.GetHash().c_str(), spec2.GetHash().c_str());
    // Hashes differs for different specifications
    EXPECT_STRNE(spec1.GetHash().c_str(), spec3.GetHash().c_str());
    }
