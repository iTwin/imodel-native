/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
* @bsimethod                                     Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "propertyName": "TestProperty",
        "editorName": "TestEditor",
        "parameters": [{
            "paramsType": "Multiline"
        }, {
            "paramsType": "Json",
            "json": { }
        }, {
            "paramsType": "Range"
        }, {
            "paramsType": "Slider",
            "min": 1,
            "max": 4.56
        }]
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());
    
    PropertyEditorsSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("TestProperty", spec.GetPropertyName().c_str());
    EXPECT_STREQ("TestEditor", spec.GetEditorName().c_str());
    EXPECT_EQ(4, spec.GetParameters().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, LoadFromJsonFailsWhenPropertyNameIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
       "editorName":"TestEditor"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertyEditorsSpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Aidas.Kilinskas                 04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, LoadFromJsonFailsWhenEditorNameIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "propertyName":"TestProperty"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertyEditorsSpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, WriteToJson)
    {
    PropertyEditorsSpecification spec("prop", "editor");
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "propertyName": "prop",
        "editorName": "editor"
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <Editor PropertyName="TestProperty" EditorName="TestEditor">
            <JsonParams>{}</JsonParams>
            <MultilineParams>{}</MultilineParams>
            <RangeParams>{}</RangeParams>
            <SliderParams Minimum="1.23" Maximum="4.56">{}</SliderParams>
        </Editor>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    PropertyEditorsSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("TestProperty", spec.GetPropertyName().c_str());
    EXPECT_STREQ("TestEditor", spec.GetEditorName().c_str());
    EXPECT_EQ(4, spec.GetParameters().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, LoadFromXmlFailsWhenPropertyNameIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <Editor EditorName="TestEditor"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    PropertyEditorsSpecification spec;
    EXPECT_FALSE(spec.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, LoadFromXmlFailsWhenEditorNameIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <Editor PropertyName="TestProperty"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    PropertyEditorsSpecification spec;
    EXPECT_FALSE(spec.ReadXml(xml->GetRootElement()));
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
    spec.AddParameter(*new PropertyEditorJsonParameters());
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
* @bsimethod                                     Aidas.Kilinskas                04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, JsonParams_LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "paramsType": "Json",
        "json": {"Custom": "Json"}
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertyEditorJsonParameters spec;
    EXPECT_TRUE(spec.ReadJson(json));

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
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, MultilineParams_LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "paramsType": "Multiline",
        "height":999
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());
    
    PropertyEditorMultilineParameters spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_EQ(999, spec.GetHeightInRows());
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
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, RangeParams_LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "paramsType": "Range",
        "min": 1.23,
        "max": 4.56
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());
    
    PropertyEditorRangeParameters spec;
    EXPECT_TRUE(spec.ReadJson(json));
    ASSERT_TRUE(nullptr != spec.GetMinimumValue());
    EXPECT_DOUBLE_EQ(1.23, *spec.GetMinimumValue());
    ASSERT_TRUE(nullptr != spec.GetMaximumValue());
    EXPECT_DOUBLE_EQ(4.56, *spec.GetMaximumValue());
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
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, RangeParams_LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "paramsType": "Range"
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());
    
    PropertyEditorRangeParameters spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_TRUE(nullptr == spec.GetMinimumValue());
    EXPECT_TRUE(nullptr == spec.GetMaximumValue());
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
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, SliderParams_LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
        "paramsType": "Slider",
        "min": 1.23,
        "max": 4.56,
        "intervalsCount": 5,
        "isVertical": true
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());
    
    PropertyEditorSliderParameters spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_DOUBLE_EQ(1.23, spec.GetMinimumValue());
    EXPECT_DOUBLE_EQ(4.56, spec.GetMaximumValue());
    EXPECT_EQ(5, spec.GetIntervalsCount());
    EXPECT_EQ(1, spec.GetValueFactor());
    EXPECT_TRUE(spec.IsVertical());
    }

/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, SliderParams_LoadsFromJsonWithDefaultValues)
    {
    static Utf8CP jsonString = R"({
        "paramsType": "Slider",
        "min": 1.23,
        "max": 4.56
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertyEditorSliderParameters spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_DOUBLE_EQ(1.23, spec.GetMinimumValue());
    EXPECT_DOUBLE_EQ(4.56, spec.GetMaximumValue());
    EXPECT_EQ(1, spec.GetIntervalsCount());
    EXPECT_EQ(1, spec.GetValueFactor());
    EXPECT_FALSE(spec.IsVertical());
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
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, SliderParams_LoadFromJsonFailsWhenMinimumAtributeIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "paramsType": "Slider",
        "max":4.56
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());
    
    PropertyEditorSliderParameters spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }
/*---------------------------------------------------------------------------------**//**
* @betest                                   Aidas.Kilinskas                		04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, SliderParams_LoadFromJsonFailsWhenMaximumAtributeIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
        "paramsType": "Slider",
        "min":4.56
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());
    
    PropertyEditorSliderParameters spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, LoadSliderParamsFromXmlFailsWhenMinimumAtributeIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <SliderParams Maximum="4.56"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    PropertyEditorSliderParameters spec;
    EXPECT_FALSE(spec.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, LoadSliderParamsFromXmlFailsWhenMaximumAtributeIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <SliderParams Minimum="4.56"/>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);
    
    PropertyEditorSliderParameters spec;
    EXPECT_FALSE(spec.ReadXml(xml->GetRootElement()));
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

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                     Aidas.Vaiksnoras                12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, ComputesCorrectHashes)
    {
    PropertyEditorsSpecification spec1("Property", "Editor");
    spec1.AddParameter(*new PropertyEditorJsonParameters());
    spec1.AddParameter(*new PropertyEditorMultilineParameters());
    spec1.AddParameter(*new PropertyEditorRangeParameters());
    PropertyEditorsSpecification spec2("Property", "Editor");
    spec2.AddParameter(*new PropertyEditorJsonParameters());
    spec2.AddParameter(*new PropertyEditorMultilineParameters());
    spec2.AddParameter(*new PropertyEditorRangeParameters());
    PropertyEditorsSpecification spec3("Property", "Editor");
    spec3.AddParameter(*new PropertyEditorSliderParameters());

    // Hashes are same for editor with same parameters
    EXPECT_STREQ(spec1.GetHash().c_str(), spec2.GetHash().c_str());
    // Hashes differs for editor with different parameters
    EXPECT_STRNE(spec1.GetHash().c_str(), spec3.GetHash().c_str());
    }
