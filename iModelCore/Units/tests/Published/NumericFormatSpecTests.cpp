/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/Published/NumericFormatSpecTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../FormattingTestsPch.h"
#include "../TestFixture/FormattingTestFixture.h"

USING_BENTLEY_FORMATTING
BEGIN_BENTLEY_FORMATTEST_NAMESPACE

struct NumericFormatSpecTest : FormattingTestFixture {};

struct NumericFormatSpecJsonTest : NumericFormatSpecTest
{
    static void ValidateJson_Type(JsonValueCR jval, PresentationType expectedType);
    //! Searches for and validates all common attributes between all presentation types against their expected default values.
    //! Expects all common attributes to exist
    static void ValidateJson_DefaultCommonAttributes(JsonValueCR jval);
};

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(NumericFormatSpecTest, DefaultConstructorValues)
    {
    // Default constructed.
    NumericFormatSpec nfs;
    EXPECT_DOUBLE_EQ(FormatConstant::DefaultRoundingFactor(), nfs.GetRoundingFactor());
    EXPECT_EQ(FormatConstant::DefaultPresentaitonType(), nfs.GetPresentationType());
    EXPECT_EQ(FormatConstant::DefaultSignOption(), nfs.GetSignOption());
    EXPECT_EQ(FormatConstant::DefaultFormatTraits(), nfs.GetFormatTraits());
    EXPECT_EQ(FormatConstant::DefaultDecimalPrecision(), nfs.GetDecimalPrecision());
    EXPECT_EQ(FormatConstant::DefaultFractionalPrecision(), nfs.GetFractionalPrecision());
    EXPECT_EQ(FormatConstant::DefaultDecimalSeparator(), nfs.GetDecimalSeparator());
    EXPECT_EQ(FormatConstant::DefaultThousandSeparator(), nfs.GetThousandSeparator());
    EXPECT_EQ(FormatConstant::DefaultUomSeparator(), nfs.GetUomSeparator());
    EXPECT_EQ(FormatConstant::DefaultStationSeparator(), nfs.GetStationSeparator());
    EXPECT_EQ(FormatConstant::DefaultMinWidth(), nfs.GetMinWidth());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Victor.Cushman                  03/18
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(NumericFormatSpecTest, IsIdentical)
    {
    NumericFormatSpec nfsA;
    NumericFormatSpec nfsB;

    EXPECT_TRUE(nfsA.IsIdentical(nfsA)) << "NumericFormatSpec is not identical to itself.";

    nfsB.SetPrecision(DecimalPrecision::Max);
    EXPECT_FALSE(nfsA.IsIdentical(nfsB)) << "nfsA should not be identical to nfsB, which has different decimal precision.";

    nfsA.SetPrecision(DecimalPrecision::Max);
    EXPECT_TRUE(nfsA.IsIdentical(nfsB)) << "nfsA should be identical to nfsB.";
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
TEST_F(NumericFormatSpecTest, FormatTraitsSet)
    {
    FormatTraits traits = NumericFormatSpec::SetTraitsBit(FormatTraits::None, FormatTraits::ApplyRounding, true);
    traits = NumericFormatSpec::SetTraitsBit(traits, FormatTraits::ExponenentOnlyNegative, true);
    EXPECT_TRUE(NumericFormatSpec::AreTraitsSet(traits, FormatTraits::ApplyRounding));
    EXPECT_TRUE(NumericFormatSpec::AreTraitsSet(traits, FormatTraits::ExponenentOnlyNegative));
    EXPECT_TRUE(NumericFormatSpec::AreTraitsSet(traits, traits));

    FormatTraits partialTraits = NumericFormatSpec::SetTraitsBit(traits, FormatTraits::PrependUnitLabel, true);
    EXPECT_TRUE(NumericFormatSpec::AreTraitsSet(partialTraits, FormatTraits::ApplyRounding));
    EXPECT_TRUE(NumericFormatSpec::AreTraitsSet(partialTraits, FormatTraits::ExponenentOnlyNegative));
    EXPECT_TRUE(NumericFormatSpec::AreTraitsSet(partialTraits, FormatTraits::PrependUnitLabel));
    EXPECT_TRUE(NumericFormatSpec::AreTraitsSet(partialTraits, traits));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
void NumericFormatSpecJsonTest::ValidateJson_Type(JsonValueCR jval, PresentationType expectedType)
    {
    EXPECT_EQ(Json::stringValue, jval.type());
    PresentationType presType;
    EXPECT_TRUE(Utils::ParsePresentationType(presType, jval.asCString()));
    EXPECT_EQ(expectedType, presType);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
void NumericFormatSpecJsonTest::ValidateJson_DefaultCommonAttributes(JsonValueCR jval)
    {
    // Sign Option
    JsonValueCR signOptJson = jval[json_showSignOption()];
    EXPECT_EQ(Json::stringValue, signOptJson.type());
    SignOption signOpt;
    EXPECT_TRUE(Utils::ParseSignOption(signOpt, signOptJson.asCString()));
    EXPECT_EQ(FormatConstant::DefaultSignOption(), signOpt);

    // Rounding Factor
    JsonValueCR factor = jval[json_roundFactor()];
    EXPECT_EQ(Json::realValue, factor.type());
    EXPECT_EQ(FormatConstant::DefaultRoundingFactor(), factor.asDouble());

    // MinWidth
    JsonValueCR minWidth = jval[json_minWidth()];
    EXPECT_EQ(Json::uintValue, minWidth.type());
    EXPECT_EQ(FormatConstant::DefaultMinWidth(), minWidth.asInt());

    // Decimal Separator
    JsonValueCR decSeparator = jval[json_decimalSeparator()];
    EXPECT_EQ(Json::stringValue, decSeparator.type());
    Utf8String decSep = decSeparator.asString();
    EXPECT_EQ(1, decSep.length());
    EXPECT_EQ(FormatConstant::DefaultDecimalSeparator(), decSep.at(0));

    // Thousand Separator
    JsonValueCR thousandSeparator = jval[json_thousandSeparator()];
    EXPECT_EQ(Json::stringValue, thousandSeparator.type());
    Utf8String thoSep = thousandSeparator.asString();
    EXPECT_EQ(1, thoSep.length());
    EXPECT_EQ(FormatConstant::DefaultThousandSeparator(), thoSep.at(0));

    // UOM Separator
    JsonValueCR uomSeparator = jval[json_uomSeparator()];
    EXPECT_EQ(Json::stringValue, uomSeparator.type());
    EXPECT_STREQ(FormatConstant::DefaultUomSeparator().c_str(), uomSeparator.asCString());

    // Format Traits
    JsonValueCR formatTraits = jval[json_formatTraits()];
    EXPECT_EQ(Json::arrayValue, formatTraits.type());
    // EXPECT_EQ(FormatConstant::DefaultFormatTraits())
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
TEST_F(NumericFormatSpecJsonTest, DeserializeStation)
    {
    Utf8CP jsonString = R"json({
        "type": "station",
        "showSignOption": "signAlways",
        "formatTraits": ["trailZeroes", "keepSingleZero"],
        "precision": 4,
        "decimalSeparator": "-",
        "thousandSeparator": "+",
        "uomSeparator": "&",
        "stationSeparator": "-",
        "stationOffsetSize": "3",
        "minWidth": 50
    })json";

    Json::Value jval(Json::objectValue);
    ASSERT_TRUE(Json::Reader::Parse(jsonString, jval));

    NumericFormatSpec testFormat;

    ASSERT_TRUE(NumericFormatSpec::FromJson(testFormat, jval));

    EXPECT_EQ(PresentationType::Station, testFormat.GetPresentationType());
    EXPECT_EQ(SignOption::SignAlways, testFormat.GetSignOption());
    EXPECT_EQ(DecimalPrecision::Precision4, testFormat.GetDecimalPrecision());
    EXPECT_STREQ("TrailZeroes|KeepSingleZero", testFormat.GetFormatTraitsString().c_str());
    EXPECT_EQ('-', testFormat.GetDecimalSeparator());
    EXPECT_EQ('+', testFormat.GetThousandSeparator());
    EXPECT_STREQ("&", testFormat.GetUomSeparator());
    EXPECT_EQ('-', testFormat.GetStationSeparator());
    EXPECT_EQ(3, testFormat.GetStationOffsetSize());
    EXPECT_EQ(50, testFormat.GetMinWidth());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  06/2018
//--------------------------------------------------------------------------------------
TEST_F(NumericFormatSpecJsonTest, DeserializeAllFormatTraitsString)
    {
    Utf8CP jsonString = R"json({
        "type": "station",
        "showSignOption": "signAlways",
        "formatTraits": "TrailZeroes|KeepSingleZero|ZeroEmpty|KeepDecimalPoint|ApplyRounding|FractionDash|ShowUnitLabel|PrependUnitLabel|Use1000Separator|ExponentOnlyNegative",
        "precision": 4,
        "decimalSeparator": "-",
        "thousandSeparator": "+",
        "uomSeparator": "&",
        "stationSeparator": "-",
        "stationOffsetSize": "3",
        "minWidth": 50
    })json";

    Json::Value jval(Json::objectValue);
    ASSERT_TRUE(Json::Reader::Parse(jsonString, jval));

    NumericFormatSpec testFormat;

    ASSERT_TRUE(NumericFormatSpec::FromJson(testFormat, jval));

    EXPECT_EQ(PresentationType::Station, testFormat.GetPresentationType());
    EXPECT_EQ(SignOption::SignAlways, testFormat.GetSignOption());
    EXPECT_EQ(DecimalPrecision::Precision4, testFormat.GetDecimalPrecision());
    EXPECT_STREQ("TrailZeroes|KeepSingleZero|ZeroEmpty|KeepDecimalPoint|ApplyRounding|FractionDash|ShowUnitLabel|PrependUnitLabel|Use1000Separator|ExponentOnlyNegative", testFormat.GetFormatTraitsString().c_str());
    EXPECT_EQ('-', testFormat.GetDecimalSeparator());
    EXPECT_EQ('+', testFormat.GetThousandSeparator());
    EXPECT_STREQ("&", testFormat.GetUomSeparator());
    EXPECT_EQ('-', testFormat.GetStationSeparator());
    EXPECT_EQ(3, testFormat.GetStationOffsetSize());
    EXPECT_EQ(50, testFormat.GetMinWidth());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                  Kyle.Abramowitz                  06/2018
//--------------------------------------------------------------------------------------
TEST_F(NumericFormatSpecJsonTest, DeserializeAllFormatTraitsArray)
    {
    Utf8CP jsonString = R"json({
        "type": "station",
        "showSignOption": "signAlways",
        "formatTraits": ["trailZeroes", "keepSingleZero", "zeroEmpty", "keepDecimalPoint", "applyRounding", "fractionDash", "showUnitLabel", "prependUnitLabel", "use1000Separator", "exponentOnlyNegative"],
        "precision": 4,
        "decimalSeparator": "-",
        "thousandSeparator": "+",
        "uomSeparator": "&",
        "stationSeparator": "-",
        "stationOffsetSize": "3",
        "minWidth": 50
    })json";

    Json::Value jval(Json::objectValue);
    ASSERT_TRUE(Json::Reader::Parse(jsonString, jval));

    NumericFormatSpec testFormat;

    ASSERT_TRUE(NumericFormatSpec::FromJson(testFormat, jval));

    EXPECT_EQ(PresentationType::Station, testFormat.GetPresentationType());
    EXPECT_EQ(SignOption::SignAlways, testFormat.GetSignOption());
    EXPECT_EQ(DecimalPrecision::Precision4, testFormat.GetDecimalPrecision());
    EXPECT_STREQ("TrailZeroes|KeepSingleZero|ZeroEmpty|KeepDecimalPoint|ApplyRounding|FractionDash|ShowUnitLabel|PrependUnitLabel|Use1000Separator|ExponentOnlyNegative", testFormat.GetFormatTraitsString().c_str());
    EXPECT_EQ('-', testFormat.GetDecimalSeparator());
    EXPECT_EQ('+', testFormat.GetThousandSeparator());
    EXPECT_STREQ("&", testFormat.GetUomSeparator());
    EXPECT_EQ('-', testFormat.GetStationSeparator());
    EXPECT_EQ(3, testFormat.GetStationOffsetSize());
    EXPECT_EQ(50, testFormat.GetMinWidth());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
TEST_F(NumericFormatSpecJsonTest, DeserializeDecimal)
    {
    Utf8CP jsonString = R"json({
        "type": "decimal",
        "showSignOption": "signAlways",
        "formatTraits": ["trailZeroes", "keepSingleZero"],
        "precision": 4,
        "decimalSeparator": "-",
        "thousandSeparator": "+",
        "uomSeparator": "&",
        "minWidth": 50
    })json";

    Json::Value jval(Json::objectValue);
    ASSERT_TRUE(Json::Reader::Parse(jsonString, jval));

    NumericFormatSpec testFormat;

    ASSERT_TRUE(NumericFormatSpec::FromJson(testFormat, jval));

    EXPECT_EQ(PresentationType::Decimal, testFormat.GetPresentationType());
    EXPECT_EQ(SignOption::SignAlways, testFormat.GetSignOption());
    EXPECT_EQ(DecimalPrecision::Precision4, testFormat.GetDecimalPrecision());
    EXPECT_STREQ("TrailZeroes|KeepSingleZero", testFormat.GetFormatTraitsString().c_str());
    EXPECT_EQ('-', testFormat.GetDecimalSeparator());
    EXPECT_EQ('+', testFormat.GetThousandSeparator());
    EXPECT_STREQ("&", testFormat.GetUomSeparator());
    EXPECT_EQ(50, testFormat.GetMinWidth());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
TEST_F(NumericFormatSpecJsonTest, DeserializeFractional)
    {
    Utf8CP jsonString = R"json({
        "type": "fractional",
        "showSignOption": "signAlways",
        "formatTraits": ["trailZeroes", "keepSingleZero"],
        "precision": 4,
        "decimalSeparator": "-",
        "thousandSeparator": "+",
        "uomSeparator": "&",
        "minWidth": 50
    })json";

    Json::Value jval(Json::objectValue);
    ASSERT_TRUE(Json::Reader::Parse(jsonString, jval));

    NumericFormatSpec testFormat;

    ASSERT_TRUE(NumericFormatSpec::FromJson(testFormat, jval));

    EXPECT_EQ(PresentationType::Fractional, testFormat.GetPresentationType());
    EXPECT_EQ(SignOption::SignAlways, testFormat.GetSignOption());
    EXPECT_EQ(FractionalPrecision::Quarter, testFormat.GetFractionalPrecision());
    EXPECT_STREQ("TrailZeroes|KeepSingleZero", testFormat.GetFormatTraitsString().c_str());
    EXPECT_EQ('-', testFormat.GetDecimalSeparator());
    EXPECT_EQ('+', testFormat.GetThousandSeparator());
    EXPECT_STREQ("&", testFormat.GetUomSeparator());
    EXPECT_EQ(50, testFormat.GetMinWidth());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
TEST_F(NumericFormatSpecJsonTest, DeserializeScientific)
    {
    Utf8CP jsonString = R"json({
        "type": "scientific",
        "scientificType": "normalized",
        "showSignOption": "signAlways",
        "formatTraits": ["trailZeroes", "keepSingleZero"],
        "precision": 4,
        "decimalSeparator": "-",
        "thousandSeparator": "+",
        "uomSeparator": "&",
        "minWidth": 50
    })json";

    Json::Value jval(Json::objectValue);
    ASSERT_TRUE(Json::Reader::Parse(jsonString, jval));

    NumericFormatSpec testFormat;

    ASSERT_TRUE(NumericFormatSpec::FromJson(testFormat, jval));

    EXPECT_EQ(PresentationType::Scientific, testFormat.GetPresentationType());
    EXPECT_EQ(ScientificType::Normalized, testFormat.GetScientificType());
    EXPECT_EQ(SignOption::SignAlways, testFormat.GetSignOption());
    EXPECT_EQ(DecimalPrecision::Precision4, testFormat.GetDecimalPrecision());
    EXPECT_STREQ("TrailZeroes|KeepSingleZero", testFormat.GetFormatTraitsString().c_str());
    EXPECT_EQ('-', testFormat.GetDecimalSeparator());
    EXPECT_EQ('+', testFormat.GetThousandSeparator());
    EXPECT_STREQ("&", testFormat.GetUomSeparator());
    EXPECT_EQ(50, testFormat.GetMinWidth());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
TEST_F(NumericFormatSpecJsonTest, SerializeDecimalType)
    {
    NumericFormatSpec format;

    {
    Json::Value basicJson;
    format.ToJson(basicJson, false);
    EXPECT_FALSE(basicJson.empty());
    EXPECT_EQ(1, (uint32_t)basicJson.size()) << "Incorrect number of default Decimal attributes.";
    JsonValueCR firstValue = basicJson[json_type()];
    ValidateJson_Type(firstValue, PresentationType::Decimal);
    }
    {
    Json::Value verboseJson;
    format.ToJson(verboseJson, true);
    EXPECT_FALSE(verboseJson.empty());
    EXPECT_EQ(9, (uint32_t)verboseJson.size()) << "Incorrect number of Decimal attributes.";
    ValidateJson_DefaultCommonAttributes(verboseJson);
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
TEST_F(NumericFormatSpecJsonTest, SerializeScientific)
    {
    NumericFormatSpec format;
    format.SetPresentationType(PresentationType::Scientific);

    {
    Json::Value basicJson;
    format.ToJson(basicJson, false);
    EXPECT_FALSE(basicJson.empty());
    EXPECT_EQ(2, (uint32_t)basicJson.size()) << "Incorrect number of default Scientific attributes.";
    JsonValueCR firstValue = basicJson[json_type()];
    ValidateJson_Type(firstValue, PresentationType::Scientific);
    }
    {
    Json::Value verboseJson;
    format.ToJson(verboseJson, true);
    EXPECT_FALSE(verboseJson.empty());
    EXPECT_EQ(10, (uint32_t)verboseJson.size()) << "Incorrect number of Scientific attributes.";
    ValidateJson_DefaultCommonAttributes(verboseJson);
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
TEST_F(NumericFormatSpecJsonTest, SerializeStation)
    {
    NumericFormatSpec format;
    format.SetPresentationType(PresentationType::Station);

    {
    Json::Value basicJson;
    format.ToJson(basicJson, false);
    EXPECT_FALSE(basicJson.empty());
    EXPECT_EQ(2, (uint32_t)basicJson.size()) << "Incorrect number of default Station attributes.";
    JsonValueCR firstValue = basicJson[json_type()];
    ValidateJson_Type(firstValue, PresentationType::Station);

    JsonValueCR offsetSize = basicJson[json_stationOffsetSize()];
    EXPECT_EQ(Json::uintValue, offsetSize.type());
    EXPECT_EQ(0, offsetSize.asUInt());
    }
    {
    Json::Value verboseJson;
    format.ToJson(verboseJson, true);
    EXPECT_FALSE(verboseJson.empty());
    EXPECT_EQ(11, (uint32_t)verboseJson.size()) << "Incorrect number of Station attributes.";
    ValidateJson_DefaultCommonAttributes(verboseJson);
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
TEST_F(NumericFormatSpecJsonTest, SerializeFractional)
    {
    NumericFormatSpec format;
    format.SetPresentationType(PresentationType::Fractional);

    {
    Json::Value basicJson;
    format.ToJson(basicJson, false);
    EXPECT_FALSE(basicJson.empty());
    EXPECT_EQ(1, (uint32_t)basicJson.size()) << "Incorrect number of default Fractional attributes.";
    }
    {
    Json::Value verboseJson;
    format.ToJson(verboseJson, true);
    EXPECT_FALSE(verboseJson.empty());
    EXPECT_EQ(9, (uint32_t)verboseJson.size()) << "Incorrect number of Fractional attributes.";
    ValidateJson_DefaultCommonAttributes(verboseJson);
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                 Kyle.Abramowitz                   04/2018
//--------------------------------------------------------------------------------------
TEST_F(NumericFormatSpecTest, FormatTraitsStringTest)
    {
    NumericFormatSpec format;
    EXPECT_TRUE(format.SetFormatTraits(""));
    EXPECT_STREQ("", format.GetFormatTraitsString().c_str());

    EXPECT_FALSE(format.SetFormatTraits("Banana"));
    EXPECT_STREQ("", format.GetFormatTraitsString().c_str());

    EXPECT_TRUE(format.SetFormatTraits("trailZeroes|keepSingleZero|zeroEmpty|keepDecimalPoint|applyRounding|fractionDash|showUnitLabel|prependUnitLabel|use1000Separator|exponentOnlyNegative"));
    EXPECT_STRCASEEQ("trailZeroes|keepSingleZero|zeroEmpty|keepDecimalPoint|applyRounding|fractionDash|showUnitLabel|prependUnitLabel|use1000Separator|exponentOnlyNegative", format.GetFormatTraitsString().c_str());
    }

END_BENTLEY_FORMATTEST_NAMESPACE
