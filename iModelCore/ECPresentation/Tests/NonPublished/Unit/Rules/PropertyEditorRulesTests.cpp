/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PresentationRulesTests.h"
#include <ECPresentation/Rules/PropertyEditorSpecification.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyEditorRulesTests : PresentationRulesTests
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, CopyConstructor)
    {
    // build a custom PropertyEditorSpecification
    auto source = new PropertyEditorSpecification();
    source->SetEditorName("a");
    source->AddParameter(*new PropertyEditorJsonParameters());
    Utf8String sourceHash = source->GetHash();

    // copy
    PropertyEditorSpecification target(*source);

    // delete source
    DELETE_AND_CLEAR(source);

    // calculate target hash and confirm it matches source
    EXPECT_EQ(sourceHash, target.GetHash());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, MoveConstructor)
    {
    // build a custom PropertyEditorSpecification
    auto source = new PropertyEditorSpecification();
    source->SetEditorName("a");
    source->AddParameter(*new PropertyEditorJsonParameters());
    Utf8String sourceHash = source->GetHash();

    // copy
    PropertyEditorSpecification target(std::move(*source));

    // delete source
    DELETE_AND_CLEAR(source);

    // calculate target hash and confirm it matches source
    EXPECT_EQ(sourceHash, target.GetHash());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, LoadsFromJson)
    {
    static Utf8CP jsonString = R"({
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

    PropertyEditorSpecification spec;
    EXPECT_TRUE(spec.ReadJson(json));
    EXPECT_STREQ("TestEditor", spec.GetEditorName().c_str());
    EXPECT_EQ(4, spec.GetParameters().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, LoadFromJsonFailsWhenEditorNameIsNotSpecified)
    {
    static Utf8CP jsonString = R"({
    })";
    Json::Value json = Json::Reader::DoParse(jsonString);
    EXPECT_FALSE(json.isNull());

    PropertyEditorSpecification spec;
    EXPECT_FALSE(spec.ReadJson(json));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, WriteToJson)
    {
    PropertyEditorSpecification spec("editor");
    Json::Value json = spec.WriteJson();
    Json::Value expected = Json::Reader::DoParse(R"({
        "editorName": "editor"
    })");
    EXPECT_STREQ(ToPrettyString(expected).c_str(), ToPrettyString(json).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, LoadsFromXml)
    {
    static Utf8CP xmlString = R"(
        <Editor EditorName="TestEditor">
            <JsonParams>{}</JsonParams>
            <MultilineParams>{}</MultilineParams>
            <RangeParams>{}</RangeParams>
            <SliderParams Minimum="1.23" Maximum="4.56">{}</SliderParams>
        </Editor>
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    PropertyEditorSpecification spec;
    EXPECT_TRUE(spec.ReadXml(xml->GetRootElement()));
    EXPECT_STREQ("TestEditor", spec.GetEditorName().c_str());
    EXPECT_EQ(4, spec.GetParameters().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, LoadFromXmlFailsWhenEditorNameIsNotSpecified)
    {
    static Utf8CP xmlString = R"(
        <Editor />
        )";
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateAndReadFromString(xmlStatus, xmlString);
    ASSERT_EQ(BEXML_Success, xmlStatus);

    PropertyEditorSpecification spec;
    EXPECT_FALSE(spec.ReadXml(xml->GetRootElement()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, WritesToXml)
    {
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xml = BeXmlDom::CreateEmpty();
    xml->AddNewElement("Root", nullptr, nullptr);

    PropertyEditorSpecification spec("Editor1");
    spec.AddParameter(*new PropertyEditorJsonParameters());
    spec.WriteXml(xml->GetRootElement());

    static Utf8CP expected = ""
        "<Root>"
            R"(<Editor EditorName="Editor1">)"
                R"(<JsonParams />)"
            R"(</Editor>)"
        "</Root>";
    EXPECT_STREQ(ToPrettyString(*BeXmlDom::CreateAndReadFromString(xmlStatus, expected)).c_str(), ToPrettyString(*xml).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "08b9d3fb5051f5009cfaa38043308082";

    // first we make sure that introducing additional attributes with default values don't affect the hash
    PropertyEditorSpecification defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // then we make sure that copy has the same hash
    PropertyEditorSpecification copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // then we test that each attribute affects the hash
    PropertyEditorSpecification specWithEditorName(defaultSpec);
    specWithEditorName.SetEditorName("a");
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithEditorName.GetHash().c_str());
    specWithEditorName.SetEditorName("");
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithEditorName.GetHash().c_str());

    PropertyEditorSpecification specWithParams(defaultSpec);
    specWithParams.AddParameter(*new PropertyEditorJsonParameters());
    EXPECT_STRNE(defaultSpec.GetHash().c_str(), specWithParams.GetHash().c_str());
    specWithParams.ClearParameters();
    EXPECT_STREQ(defaultSpec.GetHash().c_str(), specWithParams.GetHash().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsiclass
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
* @bsiclass
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
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, JsonParams_ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "5cfda8f8b63d6ed9ae4587332b85343f";

    // Make sure that introducing additional attributes with default values don't affect the hash
    PropertyEditorJsonParameters defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    PropertyEditorJsonParameters copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
* @bsiclass
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
* @bsiclass
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
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, MultilineParams_ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "9ef28508538c9b6936b69e7ea8e1a7b7";

    // Make sure that introducing additional attributes with default values don't affect the hash
    PropertyEditorMultilineParameters defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    PropertyEditorMultilineParameters copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    ASSERT_TRUE(spec.GetMinimumValue().IsValid());
    EXPECT_DOUBLE_EQ(1.23, spec.GetMinimumValue().Value());
    ASSERT_TRUE(spec.GetMaximumValue().IsValid());
    EXPECT_DOUBLE_EQ(4.56, spec.GetMaximumValue().Value());
    }
/*---------------------------------------------------------------------------------**//**
* @bsiclass
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
    ASSERT_TRUE(spec.GetMinimumValue().IsValid());
    EXPECT_DOUBLE_EQ(1.23, spec.GetMinimumValue().Value());
    ASSERT_TRUE(spec.GetMaximumValue().IsValid());
    EXPECT_DOUBLE_EQ(4.56, spec.GetMaximumValue().Value());
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
    EXPECT_TRUE(spec.GetMinimumValue().IsNull());
    EXPECT_TRUE(spec.GetMaximumValue().IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
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
    EXPECT_TRUE(spec.GetMinimumValue().IsNull());
    EXPECT_TRUE(spec.GetMaximumValue().IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
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
* @bsiclass
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
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, RangeParams_ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "87ba2ecc8b6915e8bd6f5089918229fd";

    // Make sure that introducing additional attributes with default values don't affect the hash
    PropertyEditorRangeParameters defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    PropertyEditorRangeParameters copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }

/*---------------------------------------------------------------------------------**//**
* @betest
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
* @betest
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
* @bsiclass
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
* @bsiclass
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
* @betest
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
* @betest
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
* @bsiclass
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
* @bsiclass
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
* @bsiclass
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
* @bsiclass
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
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(PropertyEditorRulesTests, SliderParams_ComputesCorrectHashes)
    {
    Utf8CP DEFAULT_HASH = "2d9b9a764fb0be4be10e1b2fce63f561";

    // Make sure that introducing additional attributes with default values don't affect the hash
    PropertyEditorSliderParameters defaultSpec;
    EXPECT_STREQ(DEFAULT_HASH, defaultSpec.GetHash().c_str());

    // Make sure that copy has the same hash
    PropertyEditorSliderParameters copySpec(defaultSpec);
    EXPECT_STREQ(DEFAULT_HASH, copySpec.GetHash().c_str());

    // TODO: test that each attribute affects the hash
    }
