/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/PresentationRules/PropertyEditorRulesTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/RulesDriven/Rules/PropertyEditorsSpecification.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyEditorRulesTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <Editor PropertyName="TestProperty" EditorName="TestEditor">
            <JsonParams>{}</JsonParams>
        </Editor>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    PropertyEditorsSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("TestProperty", spec.GetPropertyName().c_str());
    EXPECT_STREQ("TestEditor", spec.GetEditorName().c_str());
    EXPECT_EQ(1, spec.GetParameters().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, WritesToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    PropertyEditorsSpecification spec("Property1", "Editor1");
    spec.GetParametersR().push_back(new PropertyEditorJsonParameters());
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<Editor PropertyName="Property1" EditorName="Editor1">)"
                R"(<JsonParams />)"
            R"(</Editor>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, LoadsJsonParamsFromXml)
    {
    static Utf8CP xmlString = R"(
        <JsonParams>{"Custom": "Json"}</JsonParams>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    PropertyEditorJsonParameters spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));

    static Utf8CP expectedJsonStr = R"({
        "Custom": "Json"
        })";
    Json::Value expectedJson;
    Json::Reader::Parse(expectedJsonStr, expectedJson);

    EXPECT_EQ(expectedJson, spec.GetJson())
        << "Expected:\r\n" << expectedJson.toStyledString() << "\r\n"
        << "Actual: \r\n" << spec.GetJson().toStyledString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, WritesJsonParamsToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    Json::Value json;
    json["Test"] = 123;

    PropertyEditorJsonParameters spec(json);
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<JsonParams>{"Test":123}</JsonParams>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, LoadsMultilineParamsFromXml)
    {
    static Utf8CP xmlString = R"(
        <MultilineParams HeightInRows="999" />
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    PropertyEditorMultilineParameters spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_EQ(999, spec.GetHeightInRows());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, WritesMultilineParamsToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    PropertyEditorMultilineParameters spec(999);
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<MultilineParams HeightInRows="999"/>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, LoadsRangeParamsFromXml)
    {
    static Utf8CP xmlString = R"(
        <RangeParams Minimum="1.23" Maximum="4.56" />
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    PropertyEditorRangeParameters spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    ASSERT_TRUE(nullptr != spec.GetMinimumValue());
    EXPECT_DOUBLE_EQ(1.23, *spec.GetMinimumValue());
    ASSERT_TRUE(nullptr != spec.GetMaximumValue());
    EXPECT_DOUBLE_EQ(4.56, *spec.GetMaximumValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, LoadsDefaultRangeParamsFromXml)
    {
    static Utf8CP xmlString = R"(
        <RangeParams />
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    PropertyEditorRangeParameters spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_TRUE(nullptr == spec.GetMinimumValue());
    EXPECT_TRUE(nullptr == spec.GetMaximumValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, WritesRangeParamsToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    PropertyEditorRangeParameters spec(123, 456);
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<RangeParams Minimum="123" Maximum="456" />)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, WritesDefaultRangeParamsToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    PropertyEditorRangeParameters spec;
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<RangeParams />)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, LoadsSliderParamsFromXml)
    {
    static Utf8CP xmlString = R"(
        <SliderParams Minimum="1.23" Maximum="4.56" Intervals="5" ValueFactor="100" Vertical="True" />
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    PropertyEditorSliderParameters spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_DOUBLE_EQ(1.23, spec.GetMinimumValue());
    EXPECT_DOUBLE_EQ(4.56, spec.GetMaximumValue());
    EXPECT_EQ(5, spec.GetIntervalsCount());
    EXPECT_EQ(100, spec.GetValueFactor());
    EXPECT_TRUE(spec.IsVertical());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, LoadsDefaultSliderParamsFromXml)
    {
    static Utf8CP xmlString = R"(
        <SliderParams Minimum="1.23" Maximum="4.56" />
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    PropertyEditorSliderParameters spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_DOUBLE_EQ(1.23, spec.GetMinimumValue());
    EXPECT_DOUBLE_EQ(4.56, spec.GetMaximumValue());
    EXPECT_EQ(1, spec.GetIntervalsCount());
    EXPECT_EQ(1, spec.GetValueFactor());
    EXPECT_FALSE(spec.IsVertical());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, WritesSliderParamsToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    PropertyEditorSliderParameters spec(123, 456, 6, 1000, true);
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<SliderParams Minimum="123" Maximum="456" Intervals="6" ValueFactor="1000" Vertical="true" />)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, WritesDefaultSliderParamsToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    PropertyEditorSliderParameters spec(123, 456);
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<SliderParams Minimum="123" Maximum="456" Intervals="1" ValueFactor="1" Vertical="false" />)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }