/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../FormattingTestsPch.h"
#include "../TestFixture/FormattingTestFixture.h"

USING_BENTLEY_FORMATTING
BEGIN_BENTLEY_FORMATTEST_NAMESPACE

struct NumericFormatSpecTest : FormattingTestFixture {};

struct NumericFormatSpecJsonTest : NumericFormatSpecTest
{
    static void ValidateJson_Type(BeJsConst jval, PresentationType expectedType);
    //! Searches for and validates all common attributes between all presentation types against their expected default values.
    //! Expects all common attributes to exist
    static void ValidateJson_DefaultCommonAttributes(BeJsConst jval);
};

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
//--------------------------------------------------------------------------------------
void NumericFormatSpecJsonTest::ValidateJson_Type(BeJsConst jval, PresentationType expectedType)
    {
    EXPECT_TRUE(jval.isString());
    PresentationType presType;
    EXPECT_TRUE(Utils::ParsePresentationType(presType, jval.asCString()));
    EXPECT_EQ(expectedType, presType);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
void NumericFormatSpecJsonTest::ValidateJson_DefaultCommonAttributes(BeJsConst jval)
    {
    // Sign Option
    BeJsConst signOptJson = jval[json_showSignOption()];
    EXPECT_TRUE(signOptJson.isString());
    SignOption signOpt;
    EXPECT_TRUE(Utils::ParseSignOption(signOpt, signOptJson.asCString()));
    EXPECT_EQ(FormatConstant::DefaultSignOption(), signOpt);

    // Rounding Factor
    BeJsConst factor = jval[json_roundFactor()];
    EXPECT_TRUE(factor.isNumeric());
    EXPECT_EQ(FormatConstant::DefaultRoundingFactor(), factor.asDouble());

    // MinWidth
    BeJsConst minWidth = jval[json_minWidth()];
    EXPECT_TRUE(minWidth.isNumeric());
    EXPECT_EQ(FormatConstant::DefaultMinWidth(), minWidth.asInt());

    // Decimal Separator
    BeJsConst decSeparator = jval[json_decimalSeparator()];
    EXPECT_TRUE(decSeparator.isString());
    Utf8String decSep = decSeparator.asString();
    EXPECT_EQ(1, decSep.length());
    EXPECT_EQ(FormatConstant::DefaultDecimalSeparator(), decSep.at(0));

    // Thousand Separator
    BeJsConst thousandSeparator = jval[json_thousandSeparator()];
    EXPECT_TRUE(thousandSeparator.isString());
    Utf8String thoSep = thousandSeparator.asString();
    EXPECT_EQ(1, thoSep.length());
    EXPECT_EQ(FormatConstant::DefaultThousandSeparator(), thoSep.at(0));

    // UOM Separator
    BeJsConst uomSeparator = jval[json_uomSeparator()];
    EXPECT_TRUE(uomSeparator.isString());
    EXPECT_STREQ(FormatConstant::DefaultUomSeparator().c_str(), uomSeparator.asCString());

    // Format Traits
    BeJsConst formatTraits = jval[json_formatTraits()];
    EXPECT_TRUE(formatTraits.isArray());
    // EXPECT_EQ(FormatConstant::DefaultFormatTraits())
    }

//--------------------------------------------------------------------------------------
// @bsimethod
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

    BeJsDocument jval(jsonString);
    ASSERT_FALSE(jval.hasParseError());

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
// @bsimethod
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

    BeJsDocument jval(jsonString);
    ASSERT_FALSE(jval.hasParseError());

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
// @bsimethod
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

    BeJsDocument jval(jsonString);
    ASSERT_FALSE(jval.hasParseError());

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
// @bsimethod
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

    BeJsDocument jval(jsonString);
    ASSERT_FALSE(jval.hasParseError());

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
// @bsimethod
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

    BeJsDocument jval(jsonString);
    ASSERT_FALSE(jval.hasParseError());

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
// @bsimethod
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

    BeJsDocument jval(jsonString);
    ASSERT_FALSE(jval.hasParseError());

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
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(NumericFormatSpecJsonTest, SerializeDecimalType)
    {
    NumericFormatSpec format;

    {
    BeJsDocument basicJson;
    format.ToJson(basicJson, false);
    EXPECT_FALSE(basicJson.empty());
    EXPECT_EQ(1, (uint32_t)basicJson.size()) << "Incorrect number of default Decimal attributes.";
    BeJsConst firstValue = basicJson[json_type()];
    ValidateJson_Type(firstValue, PresentationType::Decimal);
    }
    {
    BeJsDocument verboseJson;
    format.ToJson(verboseJson, true);
    EXPECT_FALSE(verboseJson.empty());
    EXPECT_EQ(9, (uint32_t)verboseJson.size()) << "Incorrect number of Decimal attributes.";
    ValidateJson_DefaultCommonAttributes(verboseJson);
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(NumericFormatSpecJsonTest, SerializeScientific)
    {
    NumericFormatSpec format;
    format.SetPresentationType(PresentationType::Scientific);

    {
    BeJsDocument basicJson;
    format.ToJson(basicJson, false);
    EXPECT_FALSE(basicJson.empty());
    EXPECT_EQ(2, (uint32_t)basicJson.size()) << "Incorrect number of default Scientific attributes.";
    BeJsConst firstValue = basicJson[json_type()];
    ValidateJson_Type(firstValue, PresentationType::Scientific);
    }
    {
    BeJsDocument verboseJson;
    format.ToJson(verboseJson, true);
    EXPECT_FALSE(verboseJson.empty());
    EXPECT_EQ(10, (uint32_t)verboseJson.size()) << "Incorrect number of Scientific attributes.";
    ValidateJson_DefaultCommonAttributes(verboseJson);
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(NumericFormatSpecJsonTest, SerializeStation)
    {
    NumericFormatSpec format;
    format.SetPresentationType(PresentationType::Station);

    {
    BeJsDocument basicJson;
    format.ToJson(basicJson, false);
    EXPECT_FALSE(basicJson.empty());
    EXPECT_EQ(2, (uint32_t)basicJson.size()) << "Incorrect number of default Station attributes.";
    BeJsConst firstValue = basicJson[json_type()];
    ValidateJson_Type(firstValue, PresentationType::Station);

    BeJsConst offsetSize = basicJson[json_stationOffsetSize()];
    EXPECT_TRUE(offsetSize.isNumeric());
    EXPECT_EQ(0u, offsetSize.asUInt());
    }
    {
    BeJsDocument verboseJson;
    format.ToJson(verboseJson, true);
    EXPECT_FALSE(verboseJson.empty());
    EXPECT_EQ(11, (uint32_t)verboseJson.size()) << "Incorrect number of Station attributes.";
    ValidateJson_DefaultCommonAttributes(verboseJson);
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(NumericFormatSpecJsonTest, SerializeFractional)
    {
    NumericFormatSpec format;
    format.SetPresentationType(PresentationType::Fractional);

    {
    BeJsDocument basicJson;
    format.ToJson(basicJson, false);
    EXPECT_FALSE(basicJson.empty());
    EXPECT_EQ(1, (uint32_t)basicJson.size()) << "Incorrect number of default Fractional attributes.";
    }
    {
    BeJsDocument verboseJson;
    format.ToJson(verboseJson, true);
    EXPECT_FALSE(verboseJson.empty());
    EXPECT_EQ(9, (uint32_t)verboseJson.size()) << "Incorrect number of Fractional attributes.";
    ValidateJson_DefaultCommonAttributes(verboseJson);
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod
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

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(NumericFormatSpecJsonTest, DeserializeRatio)
    {
    Utf8CP jsonString = R"json({
        "type": "Ratio",
        "ratioType": "OneToN",
        "composite": {
            "includeZero": true,
            "units": [
                {"name": "UNITS.VERTICAL_PER_HORIZONTAL"}
            ]
        }
    })json";


    BeJsDocument jval(jsonString);
    ASSERT_FALSE(jval.hasParseError());

    NumericFormatSpec testFormat;

    ASSERT_TRUE(NumericFormatSpec::FromJson(testFormat, jval));
    ASSERT_TRUE(testFormat.GetPresentationType() == PresentationType::Ratio);
    ASSERT_TRUE(testFormat.GetRatioType() == RatioType::OneToN);
    }

TEST_F(NumericFormatSpecJsonTest, SerializeRatio)
    {
    NumericFormatSpec format;
    format.SetPresentationType(PresentationType::Ratio);

    {
    BeJsDocument basicJson;
    format.ToJson(basicJson, false);
    EXPECT_FALSE(basicJson.empty());
    EXPECT_EQ(2, (uint32_t)basicJson.size()) << "Incorrect number of default Ratio attributes.";
    BeJsConst firstValue = basicJson[json_type()];
    ValidateJson_Type(firstValue, PresentationType::Ratio);
    }
    {
    BeJsDocument verboseJson;
    format.ToJson(verboseJson, true);
    EXPECT_FALSE(verboseJson.empty());
    EXPECT_EQ(10, (uint32_t)verboseJson.size()) << "Incorrect number of Ratio attributes.";
    ValidateJson_DefaultCommonAttributes(verboseJson);
    }
    }

//---------------------------------------------------------------------------------------
// The string-coercion helpers exist so that string-encoded numbers ("stationOffsetSize": "3")
// keep deserializing the way the Bentley JsonCpp fork accepted them. strtod also accepts
// "nan"/"inf"/"-infinity", and converting a non-finite double to an integer type is undefined
// behavior, so the integer helpers must fall back to the caller's default instead.
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(NumericFormatSpecJsonTest, JsonStringCoercionRejectsNonFinite)
    {
    BeJsDocument json(R"json({
        "nan": "nan",
        "negativeNan": "-NaN",
        "infinity": "inf",
        "negativeInfinity": "-Infinity",
        "numeric": "3"
    })json");
    ASSERT_FALSE(json.hasParseError());

    for (Utf8CP key : {"nan", "negativeNan", "infinity", "negativeInfinity"})
        {
        EXPECT_EQ(7u, JsonToUInt(json[key], 7u)) << "JsonToUInt should return the default for '" << key << "'";
        EXPECT_EQ(INT64_C(7), JsonToInt64(json[key], INT64_C(7))) << "JsonToInt64 should return the default for '" << key << "'";
        }

    // Ordinary string-encoded numbers must still coerce, otherwise the compatibility shim is pointless.
    EXPECT_EQ(3u, JsonToUInt(json["numeric"], 7u));
    EXPECT_EQ(INT64_C(3), JsonToInt64(json["numeric"], INT64_C(7)));
    }

//---------------------------------------------------------------------------------------
// Out-of-range (but finite) string-encoded numbers are clamped rather than converted, since
// that conversion would also be undefined behavior.
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(NumericFormatSpecJsonTest, JsonStringCoercionClampsOutOfRange)
    {
    BeJsDocument json(R"json({
        "aboveUInt32": "1e30",
        "belowZero": "-5",
        "aboveInt64": "1e30",
        "belowInt64": "-1e30"
    })json");
    ASSERT_FALSE(json.hasParseError());

    EXPECT_EQ(UINT32_MAX, JsonToUInt(json["aboveUInt32"], 7u));
    EXPECT_EQ(0u, JsonToUInt(json["belowZero"], 7u));
    EXPECT_EQ(INT64_MAX, JsonToInt64(json["aboveInt64"], INT64_C(7)));
    EXPECT_EQ(INT64_MIN, JsonToInt64(json["belowInt64"], INT64_C(7)));
    }

END_BENTLEY_FORMATTEST_NAMESPACE
