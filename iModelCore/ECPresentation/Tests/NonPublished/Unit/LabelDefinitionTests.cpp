/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../Helpers/TestHelpers.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_ECPRESENTATIONTESTS

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavNodeLabelDefinitionTests : ECPresentationTest
    {
    };

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(NavNodeLabelDefinitionTests, SetsStringValue)
    {
    LabelDefinitionPtr labelDefinition = LabelDefinition::Create();
    rapidjson::Document actual = labelDefinition->SetStringValue("TestValue").ToInternalJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "DisplayValue": "TestValue",
        "RawValue": "TestValue",
        "TypeName": "string"
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(NavNodeLabelDefinitionTests, SetsStringValueWithDifferentDisplayValue)
    {
    LabelDefinitionPtr labelDefinition = LabelDefinition::Create();
    rapidjson::Document actual = labelDefinition->SetStringValue("TestValue", "DisplayValue").ToInternalJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "DisplayValue": "DisplayValue",
        "RawValue": "TestValue",
        "TypeName": "string"
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(NavNodeLabelDefinitionTests, SetsIntECValue)
    {
    LabelDefinitionPtr labelDefinition = LabelDefinition::Create();
    rapidjson::Document actual = labelDefinition->SetECValue(ECValue(10)).ToInternalJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "DisplayValue": "10",
        "RawValue": 10,
        "TypeName": "int"
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(NavNodeLabelDefinitionTests, SetsIntECValueWithDifferentDisplayValue)
    {
    LabelDefinitionPtr labelDefinition = LabelDefinition::Create();
    rapidjson::Document actual = labelDefinition->SetECValue(ECValue(10), "Custom value").ToInternalJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "DisplayValue": "Custom value",
        "RawValue": 10,
        "TypeName": "int"
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(NavNodeLabelDefinitionTests, SetsPoint3dECValue)
    {
    LabelDefinitionPtr labelDefinition = LabelDefinition::Create();
    rapidjson::Document actual = labelDefinition->SetECValue(ECValue(DPoint3d::From(1, 1, 1)), "Point value").ToInternalJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "DisplayValue": "Point value",
        "RawValue": {
            "x": 1,
            "y": 1,
            "z": 1
        },
        "TypeName": "point3d"
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(NavNodeLabelDefinitionTests, SetsDateECValue)
    {
    LabelDefinitionPtr labelDefinition = LabelDefinition::Create();
    rapidjson::Document actual = labelDefinition->SetECValue(ECValue(DateTime(2019, 01, 01)), "Date value").ToInternalJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "DisplayValue": "Date value",
        "RawValue": "2019-01-01",
        "TypeName": "dateTime"
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(NavNodeLabelDefinitionTests, SetsUtcDateTimeECValue)
    {
    LabelDefinitionPtr labelDefinition = LabelDefinition::Create();
    rapidjson::Document actual = labelDefinition->SetECValue(ECValue(DateTime(DateTime::Kind::Utc, 2019, 01, 01, 0, 0)), "DateTime value").ToInternalJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "DisplayValue": "DateTime value",
        "RawValue": "2019-01-01T00:00:00.000Z",
        "TypeName": "dateTime"
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(NavNodeLabelDefinitionTests, SetsJsonValue)
    {
    rapidjson::Document value(rapidjson::kObjectType);
    value.AddMember("testMember", "testValue", value.GetAllocator());
    LabelDefinitionPtr labelDefinition = LabelDefinition::Create();
    rapidjson::Document actual = labelDefinition->SetJsonValue("Json Value", "JsonType", value).ToInternalJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "DisplayValue": "Json Value",
        "RawValue": {
            "testMember": "testValue"
        },
        "TypeName": "JsonType"
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(NavNodeLabelDefinitionTests, SetsCompositeValue)
    {
    std::unique_ptr<LabelDefinition::CompositeRawValue> compositeValue = std::make_unique<LabelDefinition::CompositeRawValue>(" * ");
    compositeValue->AddValue(*LabelDefinition::Create("stringValue"));
    compositeValue->AddValue(*LabelDefinition::Create(ECValue(DateTime(DateTime::Kind::Utc, 2019, 01, 01, 0, 0)), "DateTime value"));

    LabelDefinitionPtr labelDefinition = LabelDefinition::Create();
    rapidjson::Document actual = labelDefinition->SetCompositeValue("CompositeValue", std::move(compositeValue)).ToInternalJson();

    rapidjson::Document expected;
    expected.Parse(R"({
        "DisplayValue": "CompositeValue",
        "RawValue": {
            "Separator": " * ",
            "Values": [
                {
                "DisplayValue": "stringValue",
                "RawValue": "stringValue",
                "TypeName": "string"
                },
                {
                "DisplayValue": "DateTime value",
                "RawValue": "2019-01-01T00:00:00.000Z",
                "TypeName": "dateTime"
                }
            ]
        },
        "TypeName": "composite"
        })");

    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(NavNodeLabelDefinitionTests, ParsesJsonWithCompositeValue)
    {
    rapidjson::Document definitionJson;
    definitionJson.Parse(R"({
        "DisplayValue": "CompositeValue",
        "RawValue": {
            "Separator": " * ",
            "Values": [
                {
                "DisplayValue": "stringValue",
                "RawValue": "stringValue",
                "TypeName": "string"
                },
                {
                "DisplayValue": "DateTime value",
                "RawValue": "2019-01-01T00:00:00.000Z",
                "TypeName": "dateTime"
                }
            ]
        },
        "TypeName": "composite"
        })");

    LabelDefinitionPtr actual = LabelDefinition::FromInternalJson(definitionJson);

    std::unique_ptr<LabelDefinition::CompositeRawValue> compositeValue = std::make_unique<LabelDefinition::CompositeRawValue>(" * ");
    compositeValue->AddValue(*LabelDefinition::Create("stringValue"));
    compositeValue->AddValue(*LabelDefinition::Create(ECValue(DateTime(DateTime::Kind::Utc, 2019, 01, 01, 0, 0)), "DateTime value"));

    LabelDefinitionPtr expected = LabelDefinition::Create("CompositeValue", std::move(compositeValue));

    EXPECT_EQ(*expected, *actual);
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(NavNodeLabelDefinitionTests, ParseStringAsLabelDefinition)
    {
    Utf8CP label = "stringValue";
    LabelDefinitionPtr actual = LabelDefinition::FromString(label);

    LabelDefinitionPtr expected = LabelDefinition::Create("stringValue");

    EXPECT_EQ(*expected, *actual);
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(NavNodeLabelDefinitionTests, EqualIfCompositeValuesHaveSameParts)
    {
    std::unique_ptr<LabelDefinition::CompositeRawValue> compositeValue1 = std::make_unique<LabelDefinition::CompositeRawValue>("-");
    compositeValue1->AddValue(*LabelDefinition::Create("FirstPart"));
    compositeValue1->AddValue(*LabelDefinition::Create("SecondPart"));

    std::unique_ptr<LabelDefinition::CompositeRawValue> compositeValue2 = std::make_unique<LabelDefinition::CompositeRawValue>("-");
    compositeValue2->AddValue(*LabelDefinition::Create("FirstPart"));
    compositeValue2->AddValue(*LabelDefinition::Create("SecondPart"));

    LabelDefinitionPtr definition1 = LabelDefinition::Create("Composite", std::move(compositeValue1));
    LabelDefinitionPtr definition2 = LabelDefinition::Create("Composite", std::move(compositeValue2));

    EXPECT_EQ(*definition1, *definition2);
    }

//---------------------------------------------------------------------------------------
// @betest
//---------------------------------------------------------------------------------------
TEST_F(NavNodeLabelDefinitionTests, NotEqualIfCompositePartsAreInDefferentOrder)
    {
    std::unique_ptr<LabelDefinition::CompositeRawValue> compositeValue1 = std::make_unique<LabelDefinition::CompositeRawValue>("-");
    compositeValue1->AddValue(*LabelDefinition::Create("FirstPart"));
    compositeValue1->AddValue(*LabelDefinition::Create("SecondPart"));

    std::unique_ptr<LabelDefinition::CompositeRawValue> compositeValue2 = std::make_unique<LabelDefinition::CompositeRawValue>("-");
    compositeValue2->AddValue(*LabelDefinition::Create("SecondPart"));
    compositeValue2->AddValue(*LabelDefinition::Create("FirstPart"));

    LabelDefinitionPtr definition1 = LabelDefinition::Create("Composite", std::move(compositeValue1));
    LabelDefinitionPtr definition2 = LabelDefinition::Create("Composite", std::move(compositeValue2));

    EXPECT_NE(*definition1, *definition2);
    }
