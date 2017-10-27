/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/ContentSpecificationTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
    bool _ReadXml(BeXmlNodeP xmlNode) override {return true;}
    void _WriteXml(BeXmlNodeP xmlNode) const override {}
    ContentSpecification* _Clone() const override {return new TestContentSpecification(*this);}
    };

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
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<TestSpecification Priority="123" ShowImages="true">)"
                R"(<RelatedProperties RelationshipClassNames="RelationshipClassName" RelatedClassNames="RelatedClassNames" )"
                    R"(PropertyNames="Properties" RequiredDirection="Forward" RelationshipMeaning="SameInstance" />)"
                R"(<DisplayedProperties PropertyNames="DisplayedProperty" Priority="123"/>)"
                R"(<HiddenProperties PropertyNames="HiddenProperty" Priority="456"/>)"
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