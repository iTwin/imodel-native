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

struct NumericFormatSpecTest : FormattingTestFixture 
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
TEST_F(NumericFormatSpecTest, LoadFromJson)
    {
    Utf8CP jsonString = R"json({
        "type": "station",
        "signOption": "signAlways",
        "formatTraits": "LeadZeroes|TrailZeroes",
        "precision": 4,
        "decSeparator": "-",
        "thousandSeparator": "+",
        "uomSeparator": "&",
        "stationSeparator": "-",
        "stationOffsetSize": "3"
    })json";

    Json::Value jval(Json::objectValue);
    ASSERT_TRUE(Json::Reader::Parse(jsonString, jval));

    NumericFormatSpec testFormat;

    ASSERT_EQ(BentleyStatus::SUCCESS, testFormat.FromJson(jval));

    EXPECT_EQ(PresentationType::Station, testFormat.GetPresentationType());
    EXPECT_EQ(SignOption::SignAlways, testFormat.GetSignOption());
    EXPECT_EQ(DecimalPrecision::Precision4, testFormat.GetDecimalPrecision());
    EXPECT_STREQ("LeadZeroes|TrailZeroes", testFormat.GetFormatTraitsString().c_str());
    EXPECT_EQ('-', testFormat.GetDecimalSeparator());
    EXPECT_EQ('+', testFormat.GetThousandSeparator());
    EXPECT_STREQ("&", testFormat.GetUomSeparator());
    EXPECT_EQ('-', testFormat.GetStationSeparator());
    EXPECT_EQ(3, testFormat.GetStationOffsetSize());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
void NumericFormatSpecTest::ValidateJson_Type(JsonValueCR jval, PresentationType expectedType)
    {
    EXPECT_EQ(Json::stringValue, jval.type());
    PresentationType presType;
    EXPECT_TRUE(Utils::ParsePresentationType(presType, jval.asCString()));
    EXPECT_EQ(expectedType, presType);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
void NumericFormatSpecTest::ValidateJson_DefaultCommonAttributes(JsonValueCR jval)
    {
    // Sign Option
    JsonValueCR signOptJson = jval[json_signOption()];
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
    EXPECT_EQ(Json::intValue, minWidth.type());
    EXPECT_EQ(FormatConstant::DefaultMinWidth(), minWidth.asInt());

    // Decimal Separator
    JsonValueCR decSeparator = jval[json_decSeparator()];
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
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
TEST_F(NumericFormatSpecTest, SerializeDecimalTypeToJson)
    {
    NumericFormatSpec format;

    {
    Json::Value basicJson = format.ToJson(false);
    EXPECT_FALSE(basicJson.empty());
    EXPECT_EQ(1, (uint32_t)basicJson.size()) << "Incorrect number of default Decimal attributes.";
    JsonValueCR firstValue = basicJson[json_type()];
    ValidateJson_Type(firstValue, PresentationType::Decimal);
    }
    {
    Json::Value verboseJson = format.ToJson(true);
    EXPECT_FALSE(verboseJson.empty());
    EXPECT_EQ(9, (uint32_t)verboseJson.size()) << "Incorrect number of Decimal attributes.";
    ValidateJson_DefaultCommonAttributes(verboseJson);
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
TEST_F(NumericFormatSpecTest, SerializeScientificToJson)
    {
    NumericFormatSpec format;
    format.SetPresentationType(PresentationType::Scientific);

    {
    Json::Value basicJson = format.ToJson(false);
    EXPECT_FALSE(basicJson.empty());
    EXPECT_EQ(2, (uint32_t)basicJson.size()) << "Incorrect number of default Scientific attributes.";
    JsonValueCR firstValue = basicJson[json_type()];
    ValidateJson_Type(firstValue, PresentationType::Scientific);
    }
    {
    Json::Value verboseJson = format.ToJson(true);
    EXPECT_FALSE(verboseJson.empty());
    EXPECT_EQ(10, (uint32_t)verboseJson.size()) << "Incorrect number of Scientific attributes.";
    ValidateJson_DefaultCommonAttributes(verboseJson);
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
TEST_F(NumericFormatSpecTest, SerializeStationToJson)
    {
    NumericFormatSpec format;
    format.SetPresentationType(PresentationType::Station);

    {
    Json::Value basicJson = format.ToJson(false);
    EXPECT_FALSE(basicJson.empty());
    EXPECT_EQ(1, (uint32_t)basicJson.size()) << "Incorrect number of default Station attributes.";
    JsonValueCR firstValue = basicJson[json_type()];
    ValidateJson_Type(firstValue, PresentationType::Station);
    }
    {
    Json::Value verboseJson = format.ToJson(true);
    EXPECT_FALSE(verboseJson.empty());
    EXPECT_EQ(10, (uint32_t)verboseJson.size()) << "Incorrect number of Station attributes.";
    ValidateJson_DefaultCommonAttributes(verboseJson);
    }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    04/2018
//--------------------------------------------------------------------------------------
TEST_F(NumericFormatSpecTest, SerializeFractionalToJson)
    {
    NumericFormatSpec format;
    format.SetPresentationType(PresentationType::Fractional);

    {
    Json::Value basicJson = format.ToJson(false);
    EXPECT_FALSE(basicJson.empty());
    EXPECT_EQ(1, (uint32_t)basicJson.size()) << "Incorrect number of default Fractional attributes.";
    }
    {
    Json::Value verboseJson = format.ToJson(true);
    EXPECT_FALSE(verboseJson.empty());
    EXPECT_EQ(9, (uint32_t)verboseJson.size()) << "Incorrect number of Fractional attributes.";
    ValidateJson_DefaultCommonAttributes(verboseJson);
    }
    }

END_BENTLEY_FORMATTEST_NAMESPACE
