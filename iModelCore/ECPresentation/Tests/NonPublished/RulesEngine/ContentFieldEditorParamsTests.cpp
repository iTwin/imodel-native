/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/NonPublished/RulesEngine/ContentFieldEditorParamsTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/BeTest.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>
#include "../../../Source/RulesDriven/RulesEngine/ContentFieldEditors.h"

USING_NAMESPACE_BENTLEY_ECPRESENTATION

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                10/2017
+===============+===============+===============+===============+===============+======*/
struct ContentFieldEditorParamsTests : ::testing::Test
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentFieldEditorParamsTests, JsonParams_Serialization)
    {
    Json::Value json;
    json["Test1"] = 1;
    json["Test2"][0] = 2;
    json["Test2"][1] = 3;
    PropertyEditorJsonParameters spec(json);
    FieldEditorJsonParams params(spec);
    rapidjson::Document actual = params.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"*({
        "Test1": 1,
        "Test2": [2, 3]
        })*");
    
    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentFieldEditorParamsTests, JsonParams_Equalty)
    {
    Json::Value json;
    json["Test"] = 1;
    PropertyEditorJsonParameters spec(json);
    FieldEditorJsonParams params(spec);

    FieldEditorJsonParams params2(spec);
    EXPECT_TRUE(params.Equals(params2));
    
    Json::Value json2;
    json2["Test"] = 2;
    PropertyEditorJsonParameters spec2(json2);
    FieldEditorJsonParams params3(spec2);
    EXPECT_FALSE(params.Equals(params3));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentFieldEditorParamsTests, MultilineParams_Serialization)
    {
    PropertyEditorMultilineParameters spec(999);
    FieldEditorMultilineParams params(spec);
    rapidjson::Document actual = params.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"*({
        "HeightInRows": 999
        })*");
    
    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentFieldEditorParamsTests, MultilineParams_Equalty)
    {
    PropertyEditorMultilineParameters spec(111);
    FieldEditorMultilineParams params(spec);

    PropertyEditorMultilineParameters spec2(111);
    FieldEditorMultilineParams params2(spec2);
    EXPECT_TRUE(params.Equals(params2));
    
    PropertyEditorMultilineParameters spec3(333);
    FieldEditorMultilineParams params3(spec3);
    EXPECT_FALSE(params.Equals(params3));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentFieldEditorParamsTests, RangeParams_Serialization)
    {
    PropertyEditorRangeParameters spec(123.33, 456.66);
    FieldEditorRangeParams params(spec);
    rapidjson::Document actual = params.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"*({
        "Minimum": 123.33,
        "Maximum": 456.66
        })*");
    
    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentFieldEditorParamsTests, RangeParams_Equalty)
    {
    PropertyEditorRangeParameters spec(123.33, 456.66);
    FieldEditorRangeParams params(spec);
    
    PropertyEditorRangeParameters spec2(123.33, 456.66);
    FieldEditorRangeParams params2(spec2);
    EXPECT_TRUE(params.Equals(params2));
    
    PropertyEditorRangeParameters spec3(456.66, 789.99);
    FieldEditorRangeParams params3(spec3);
    EXPECT_FALSE(params.Equals(params3));
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentFieldEditorParamsTests, SliderParams_Serialization)
    {
    PropertyEditorSliderParameters spec(123.33, 456.66, 5, 100, true);
    FieldEditorSliderParams params(spec);
    rapidjson::Document actual = params.AsJson();

    rapidjson::Document expected;
    expected.Parse(R"*({
        "Minimum": 123.33,
        "Maximum": 456.66,
        "IntervalsCount": 5,
        "ValueFactor": 100,
        "IsVertical": true
        })*");
    
    EXPECT_EQ(expected, actual)
        << "Expected: \r\n" << BeRapidJsonUtilities::ToPrettyString(expected) << "\r\n"
        << "Actual: \r\n" << BeRapidJsonUtilities::ToPrettyString(actual);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ContentFieldEditorParamsTests, SliderParams_Equalty)
    {
    PropertyEditorSliderParameters spec(123.33, 456.66, 5, 100, true);
    FieldEditorSliderParams params(spec);
    
    PropertyEditorSliderParameters spec2(123.33, 456.66, 5, 100, true);
    FieldEditorSliderParams params2(spec2);
    EXPECT_TRUE(params.Equals(params2));
    
    PropertyEditorSliderParameters spec3(456.66, 789.99, 6, 1000, false);
    FieldEditorSliderParams params3(spec3);
    EXPECT_FALSE(params.Equals(params3));
    }