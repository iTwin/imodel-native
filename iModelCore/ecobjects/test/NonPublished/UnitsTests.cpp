/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
#include <fstream>
#include <sstream>
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct UnitConversionTests : ECTestFixture {};
struct UnitsTests : ECTestFixture {};
struct InvertedUnitsTests : ECTestFixture {};
struct ConstantTests : ECTestFixture {};
struct UnitsDeserializationTests : ECTestFixture {};
struct InvertedUnitsDeserializationTests: ECTestFixture {};
struct ConstantDeserializationTests: ECTestFixture {};

template<class T> typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
static almost_equal(const T x, const T y, const T ulp, T& acceptableDiff)
    {
    T fabsX = fabs(x);
    T fabsY = fabs(y);
    acceptableDiff = std::numeric_limits<T>::epsilon() * fabs(fabsX + fabsY) * ulp;
    // the machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return fabs(fabsX - fabsY) < acceptableDiff
        // unless the result is subnormal
        || fabs(fabsX - fabsY) < std::numeric_limits<T>::min();
    }

static void CompareValues(double expected, double actual, double ulp, Utf8CP message)
    {
    double acceptableDiff;
    if (!almost_equal<double>(expected, actual, ulp, acceptableDiff))
        {
        Utf8PrintfString formattedText("%s\nExpected: %.17g \nActual:   %.17g \nDiff:           %.17g   \nAcceptable Diff: %.17g   ULP: %.17g\n",
                                       message, expected, actual, fabs(actual) - fabs(expected), acceptableDiff, ulp);
        EXPECT_FALSE(true) << formattedText;
        }
    }
static Utf8CP unitsProblemCodeToString(Units::UnitsProblemCode code)
    {
    return Units::UnitsProblemCode::NoProblem == code ? "NoProblem" :
        Units::UnitsProblemCode::InvalidUnitName == code ? "InvalidUnitName" : Units::UnitsProblemCode::UncomparableUnits == code ? "UncomparableUnits" : "InvertingZero";
    }

void testUnitConversion (double sourceValue, Utf8CP sourceUnitName, double expectedValue, Utf8CP targetUnitName, double ulp, Units::UnitsProblemCode expectedProblemCode = Units::UnitsProblemCode::NoProblem)
    {
    ECUnitCP sourceUnit = ECTestFixture::GetUnitsSchema()->GetUnitCP(sourceUnitName);
    ASSERT_NE(nullptr, sourceUnit) << "Could not find source unit: " << sourceUnitName;
    ECUnitCP targetUnit = ECTestFixture::GetUnitsSchema()->GetUnitCP(targetUnitName);
    ASSERT_NE(nullptr, targetUnit) << "Could not target unit: " << targetUnitName;

    double actualValue = 0;
    Utf8String errorMessage = Utf8PrintfString("Conversion from %s to %s of value %.17g not as expected.", sourceUnitName, targetUnitName, sourceValue);
    auto problemCode = sourceUnit->Convert(actualValue, sourceValue, targetUnit);
    EXPECT_EQ(expectedProblemCode, problemCode) << "UnitsProblemCode: " << unitsProblemCodeToString(problemCode) << "\r\n" << errorMessage.c_str();
    CompareValues(expectedValue, actualValue, ulp, errorMessage.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
// Compares conversion from schema defined units to those that used to be hard coded in the units assembly
TEST_F(UnitConversionTests, SchemaUnitConversionsMatchOldConversions)
    {
    auto path = ECTestFixture::GetTestDataPath(L"ec31UnitConversions.csv");
    std::ifstream ifs = std::ifstream(Utf8String(path.c_str()).c_str(), std::ifstream::in);
    std::string line;
    auto s = GetUnitsSchema();
    auto toDouble = [](Utf8StringCR d)
        {
        std::istringstream iss(d.c_str());
        double val = 0.0;
        iss >> val;
        return val;
        };
    int numConversions = 0;
    while (std::getline(ifs, line))
        {
        ++numConversions;
        bvector<Utf8String> split;
        BeStringUtilities::Split(line.c_str(), ",", split);
        ASSERT_EQ(4, split.size());
        auto fromName = split[0];
        auto toName = split[2];
        auto orig = toDouble(split[1]);
        auto conv = toDouble(split[3]);
        auto from = Units::UnitNameMappings::TryGetECNameFromNewName(fromName.c_str());
        auto to = Units::UnitNameMappings::TryGetECNameFromNewName(toName.c_str());
        ASSERT_NE(nullptr, from);
        ASSERT_NE(nullptr, to);

        // From Schema
        Utf8String alias;
        Utf8String fromUnitName, toUnitName;
        ECClass::ParseClassName(alias, fromUnitName, from);
        ECClass::ParseClassName(alias, toUnitName, to);
        testUnitConversion(orig, fromUnitName.c_str(), conv, toUnitName.c_str(), 3);
        }
    ASSERT_EQ(6222, numConversions) << "Did not run the number of conversions expected";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
// Compares conversions from schema defined units to conversions from V8i/EC2 units
TEST_F(UnitConversionTests, SchemaUnitConversionsMatchEC2Conversions)
    {
    auto path = ECTestFixture::GetTestDataPath(L"unitcomparisondata.csv");
    std::ifstream ifs = std::ifstream(Utf8String(path.c_str()).c_str(), std::ifstream::in);
    std::string line;
    auto s = GetUnitsSchema();
    auto toDouble = [](Utf8StringCR d)
        {
        std::istringstream iss(d.c_str());
        double val = 0.0;
        iss >> val;
        return val;
        };
    int numConversions = 0;
    bvector<Utf8String> unconvertedLines;
    while (std::getline(ifs, line))
        {
        bvector<Utf8String> split;
        BeStringUtilities::Split(line.c_str(), ",", split);
        ASSERT_TRUE(split.size() >= 5);
        auto fromEC2Name = split[1];
        auto toEC2Name = split[3];
        auto orig = toDouble(split[0]);
        auto conv = toDouble(split[2]);
        auto ulp = toDouble(split[4]);
        if (ulp > 200)
            EXPECT_EQ(split.size(), 6) << "Expected ulps greater than 200 to have a note.  Line: " << line.c_str();
        auto from = Units::UnitNameMappings::TryGetECNameFromOldName(fromEC2Name.c_str());
        auto to = Units::UnitNameMappings::TryGetECNameFromOldName(toEC2Name.c_str());
        if (nullptr == from || nullptr == to)
            {
            LOG.infov("Skipping conversion from %s (new: %s) to %s (new: %s) because the unit does not exist in the new system.",
                fromEC2Name.c_str(), toEC2Name.c_str(), nullptr == from ? "not found" : from, nullptr == to ? "not found" : to);
            unconvertedLines.push_back(line);
            continue;
            }
        // From Schema
        Utf8String alias;
        Utf8String fromUnitName, toUnitName;
        ECClass::ParseClassName(alias, fromUnitName, from);
        ECClass::ParseClassName(alias, toUnitName, to);
        testUnitConversion(orig, fromUnitName.c_str(), conv, toUnitName.c_str(), ulp);
        ++numConversions;
        }
    ASSERT_EQ(1020, numConversions) << "Did not run the number of conversions expected.  Unconverted Lines: " << BeStringUtilities::Join(unconvertedLines, "\n");
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
TEST_F(UnitsTests, AllNewNamesMapToECNames)
    {
    auto path = ECTestFixture::GetTestDataPath(L"All3_1Names.csv");
    std::ifstream ifs(Utf8String(path).c_str(), std::ifstream::in);
    std::string line;
    bvector<Utf8String> ignoredNames = {"CM/REVOLUTION", "FT/REVOLUTION", "IN/DEGREE", "IN/RAD", "IN/REVOLUTION", "M/DEGREE", "MM/RAD", "MM/REVOLUTION"};

    bool isIgnoredName;
    Utf8String parsedName;
    while (std::getline(ifs, line))
        {
        parsedName = line.c_str();
        parsedName.Trim();
        isIgnoredName = false;

        for (auto const& ignoredName : ignoredNames)
            {
            if(ignoredName.EqualsI(parsedName.c_str()))
                isIgnoredName = true;
            }
        if (!isIgnoredName)
            {
            auto mapped = Units::UnitNameMappings::TryGetECNameFromNewName(parsedName.c_str());
            if (nullptr == mapped)
                {
                ASSERT_TRUE(false) << "Unit with new Name " << parsedName << " not mapped to an ec Name";
                continue;
                }
            if (0 == BeStringUtilities::StricmpAscii(mapped, "UNITS:DECA"))
                parsedName = "DECA";
            if (0 == BeStringUtilities::StricmpAscii(mapped, "UNITS:MEGAPASCAL"))
                parsedName = "MEGAPASCAL";
            if (0 == BeStringUtilities::StricmpAscii(mapped, "UNITS:KPF"))
                parsedName = "KPF";
            if (0 == BeStringUtilities::StricmpAscii(mapped, "UNITS:PERSON"))
                parsedName = "PERSON";
            if (0 == BeStringUtilities::StricmpAscii(mapped, "UNITS:HUNDRED_PERSON"))
                parsedName = "HUNDRED_PERSON";
            if (0 == BeStringUtilities::StricmpAscii(mapped, "UNITS:THOUSAND_PERSON"))
                parsedName = "THOUSAND_PERSON";

            auto roundTrippedName = Units::UnitNameMappings::TryGetNewNameFromECName(mapped);
            if (nullptr == roundTrippedName)
                {
                ASSERT_TRUE(false) <<  "Can't get name from ecname for unit " << mapped;
                continue;
                }

            EXPECT_STREQ(parsedName.c_str(), roundTrippedName) << "Round Tripped name different than input name";
            Utf8String alias, mappedUnitName;
            ECClass::ParseClassName(alias, mappedUnitName, mapped);
            ECUnitCP ecUnit = ECTestFixture::GetUnitsSchema()->GetUnitCP(mappedUnitName.c_str());
            EXPECT_NE(nullptr, ecUnit) << "Failed to find unit with ec name " << mapped << " and 3.1 name " << parsedName;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, VolumetricFlowUnitConversions)
    {
    ECUnitCP gallonPerHour = ECTestFixture::GetUnitsSchema()->GetUnitCP("GALLON_PER_HR");
    ECUnitCP imperialGallonPerHour = ECTestFixture::GetUnitsSchema()->GetUnitCP("GALLON_IMPERIAL_PER_HR");
    ECUnitCP cubicMeterPerSecond = ECTestFixture::GetUnitsSchema()->GetUnitCP("CUB_M_PER_SEC");
    double expected = 1.0515032733e-6; // Value from: http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    double actual;
    gallonPerHour->Convert(actual, 1.0, cubicMeterPerSecond);
    CompareValues(expected, actual, 80000, "Conversion from Gallon per Hour to cubic meter per second not as expected");

    expected = 1.2628027778e-6; // Value from: http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    imperialGallonPerHour->Convert(actual, 1.0, cubicMeterPerSecond);
    CompareValues(expected, actual, 40000, "Conversion from Imperial Gallon per Hour to cubic meter per second not as expected");

    expected = 60;
    ECTestFixture::GetUnitsSchema()->GetUnitCP("GALLON_PER_MIN")->Convert(actual, 1.0, gallonPerHour);
    EXPECT_DOUBLE_EQ(expected, actual) << "Conversion from Gallon per Hour to gallon per minute not as expected";
    ECTestFixture::GetUnitsSchema()->GetUnitCP("GALLON_IMPERIAL_PER_MIN")->Convert(actual, 1.0, imperialGallonPerHour);
    EXPECT_DOUBLE_EQ(expected, actual) << "Conversion from imperial gallon per Hour to imperial gallon per minute not as expected";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, RotationalSpringConstantUnitConversions)
    {
    ECUnitCP newtonMeterPerRadian = ECTestFixture::GetUnitsSchema()->GetUnitCP("N_M_PER_RAD");
    ECUnitCP kiloNewtonMeterPetRadian = ECTestFixture::GetUnitsSchema()->GetUnitCP("KN_M_PER_RAD");
    ECUnitCP newtonMeterPerDegree = ECTestFixture::GetUnitsSchema()->GetUnitCP("N_M_PER_DEG");
    ECUnitCP kiloPoundForceFootPerRadian = ECTestFixture::GetUnitsSchema()->GetUnitCP("KPF_FT_PER_RAD");

    double expected = 1e3;
    double actual;
    kiloNewtonMeterPetRadian->Convert(actual, 1.0, newtonMeterPerRadian);
    CompareValues(expected, actual, 1, "Conversion from kilo Newton-meter per radian to Newton-meter per radian");

    double pi = 3.1415926535897932;
    expected = 180.0 / pi;
    newtonMeterPerDegree->Convert(actual, 1.0, newtonMeterPerRadian);
    CompareValues(expected, actual, 1, "Conversion from Newton-meter per degree to Newton-meter per radian");

    expected = 1.3558179483e3; // Value from: http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    kiloPoundForceFootPerRadian->Convert(actual, 1.0, newtonMeterPerRadian);
    CompareValues(expected, actual, 60000, "Conversion from kilo Pound-foot per radian to Newton-meter per radian");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, LinearRotationalSpringConstantUnitConversions)
    {
    ECUnitCP newtonPerRadian = ECTestFixture::GetUnitsSchema()->GetUnitCP("N_PER_RAD");
    ECUnitCP kiloNewtonPetRadian = ECTestFixture::GetUnitsSchema()->GetUnitCP("KN_PER_RAD");
    ECUnitCP kiloPoundForcePerRadian = ECTestFixture::GetUnitsSchema()->GetUnitCP("KPF_PER_RAD");

    double expected = 1e3;
    double actual;
    kiloNewtonPetRadian->Convert(actual, 1.0, newtonPerRadian);
    CompareValues(expected, actual, 1, "Conversion from kilo Newton per radian to Newton per radian");

    expected = 4.4482216153e3;
    kiloPoundForcePerRadian->Convert(actual, 1.0, newtonPerRadian);
    CompareValues(expected, actual, 50000, "Conversion from kilo kilo Pound per radian to Newton per radian");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, SpringConstantUnitConversions)
    {
    ECUnitCP newtonPerMeter = ECTestFixture::GetUnitsSchema()->GetUnitCP("SPRING_CONSTANT_N_PER_M");
    ECUnitCP kiloNewtonPerMeter = ECTestFixture::GetUnitsSchema()->GetUnitCP("SPRING_CONSTANT_KN_PER_M");
    ECUnitCP kiloPoundForcePerFoot = ECTestFixture::GetUnitsSchema()->GetUnitCP("SPRING_CONSTANT_KPF_PER_FT");

    double expected = 1e3;
    double actual;
    kiloNewtonPerMeter->Convert(actual, 1.0, newtonPerMeter);
    CompareValues(expected, actual, 1, "Conversion from kilo Newton per Meter to Newton per Meter");

    expected = 1.4593902937e4; // Value from: http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    kiloPoundForcePerFoot->Convert(actual, 1.0, newtonPerMeter);
    CompareValues(expected, actual, 50000, "Conversion from kilo Pound force per Foot to Newton per Foot");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, LinearSpringConstantUnitConversions)
    {
    ECUnitCP newtonPerSquareMeter = ECTestFixture::GetUnitsSchema()->GetUnitCP("LINEAR_SPRING_CONSTANT_N_PER_SQ_M");
    ECUnitCP kiloNewtonPerSquareMeter = ECTestFixture::GetUnitsSchema()->GetUnitCP("LINEAR_SPRING_CONSTANT_KN_PER_SQ_M");
    ECUnitCP kiloPoundForcePerSquareFoot = ECTestFixture::GetUnitsSchema()->GetUnitCP("LINEAR_SPRING_CONSTANT_KPF_PER_SQ_FT");

    double expected = 1e3;
    double actual;
    kiloNewtonPerSquareMeter->Convert(actual, 1.0, newtonPerSquareMeter);
    CompareValues(expected, actual, 1, "Conversion from kilo Newton per square Meter to Newton per square Meter");

    expected = 4.788025898e4; // Value from: http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    kiloPoundForcePerSquareFoot->Convert(actual, 1.0, newtonPerSquareMeter);
    CompareValues(expected, actual, 50000, "Conversion from kilo Pound force per square Foot to Newton per square Foot");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, AreaSpringConstantUnitConversions)
    {
    ECUnitCP newtonPerCubeMeter = ECTestFixture::GetUnitsSchema()->GetUnitCP("AREA_SPRING_CONSTANT_N_PER_CUB_M");
    ECUnitCP kiloNewtonPetCubeMeter = ECTestFixture::GetUnitsSchema()->GetUnitCP("AREA_SPRING_CONSTANT_KN_PER_CUB_M");
    ECUnitCP kiloPoundForcePerCubeFoot = ECTestFixture::GetUnitsSchema()->GetUnitCP("AREA_SPRING_CONSTANT_KPF_PER_CUB_FT");

    double expected = 1e3;
    double actual;
    kiloNewtonPetCubeMeter->Convert(actual, 1.0, newtonPerCubeMeter);
    CompareValues(expected, actual, 1, "Conversion from kilo Newton per cube Meter to Newton per cube Meter");

    expected = 1.5708746385e5; // Value from: http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    kiloPoundForcePerCubeFoot->Convert(actual, 1.0, newtonPerCubeMeter);
    CompareValues(expected, actual, 80000, "Conversion from kilo Pound force per cube Foot to Newton per cube Foot");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, LinearLoadUnitConversions)
    {
    ECUnitCP newtonPerMeter = ECTestFixture::GetUnitsSchema()->GetUnitCP("N_PER_M");
    ECUnitCP kiloNewtonPerMeter = ECTestFixture::GetUnitsSchema()->GetUnitCP("KN_PER_M");
    ECUnitCP megaNewtonPerMeter = ECTestFixture::GetUnitsSchema()->GetUnitCP("MEGAN_PER_M");
    ECUnitCP poundForcePerFoot = ECTestFixture::GetUnitsSchema()->GetUnitCP("LBF_PER_FT");
    ECUnitCP kiloPoundForcePerFeet = ECTestFixture::GetUnitsSchema()->GetUnitCP("KPF_PER_FT");
    ECUnitCP kilogramForcePerMeter = ECTestFixture::GetUnitsSchema()->GetUnitCP("KGF_PER_M");

    double actual;

    double expected = 1e3;
    kiloNewtonPerMeter->Convert(actual, 1.0, newtonPerMeter);
    CompareValues(expected, actual, 1, "Conversion from kilo Newton per Meter to Newton per Meter not as expected");

    expected = 1e6; // Value from: http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    megaNewtonPerMeter->Convert(actual, 1.0, newtonPerMeter);
    CompareValues(expected, actual, 1, "Conversion from kilo Pound per Feet to Newton per Meter not as expected");

    expected = 14.593902937; // Value from: http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    poundForcePerFoot->Convert(actual, 1.0, newtonPerMeter);
    CompareValues(expected, actual, 50000, "Conversion from kilo Pound per Feet to Newton per Meter not as expected");

    expected = 1.4593902937e4; // Value from: http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    kiloPoundForcePerFeet->Convert(actual, 1.0, newtonPerMeter);
    CompareValues(expected, actual, 50000, "Conversion from kilo Pound per Feet to Newton per Meter not as expected");

    expected = 9.80665; // Value from: http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    kilogramForcePerMeter->Convert(actual, 1.0, newtonPerMeter);
    CompareValues(expected, actual, 1, "Conversion from kilogram force per meter to Newton per Meter not as expected");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, TorqueUnitConversions)
    {
    ECUnitCP newtonMeter = ECTestFixture::GetUnitsSchema()->GetUnitCP("N_M");
    ECUnitCP kiloNewtonMeter = ECTestFixture::GetUnitsSchema()->GetUnitCP("KN_M");
    ECUnitCP megaNewtonMeter = ECTestFixture::GetUnitsSchema()->GetUnitCP("MEGAN_M");
    ECUnitCP poundForceInch = ECTestFixture::GetUnitsSchema()->GetUnitCP("LBF_IN");
    ECUnitCP kiloPoundForceFeet = ECTestFixture::GetUnitsSchema()->GetUnitCP("KPF_FT");

    double expected = 1e3;
    double actual;
    kiloNewtonMeter->Convert(actual, 1.0, newtonMeter);
    CompareValues(expected, actual, 1, "Conversion from kilo Newton Meter to Newton Meter not as expected");

    expected = 1e6;
    megaNewtonMeter->Convert(actual, 1.0, newtonMeter);
    CompareValues(expected, actual, 1, "Conversion from mega Newton Meter to Newton Meter not as expected");

    expected = 0.11298482903 ; // Value from: http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    poundForceInch->Convert(actual, 1.0, newtonMeter);
    CompareValues(expected, actual, 50000, "Conversion from Pound inch to Newton Meter not as expected");

    expected = 1.3558179483e3 ; // Value from: http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    kiloPoundForceFeet->Convert(actual, 1.0, newtonMeter);
    CompareValues(expected, actual, 80000, "Conversion from kilo Pound Feet to Newton Meter not as expected");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, LinearTorqueUnitsConversions)
    {
    ECUnitCP newtonMeterPerMeter = ECTestFixture::GetUnitsSchema()->GetUnitCP("N_M_PER_M");
    ECUnitCP kiloNewtonMeterPerMeter = ECTestFixture::GetUnitsSchema()->GetUnitCP("KN_M_PER_M");
    ECUnitCP megaNewtonMeterPerMeter = ECTestFixture::GetUnitsSchema()->GetUnitCP("MEGAN_M_PER_M");
    ECUnitCP newtonCentimeterPerCentieter = ECTestFixture::GetUnitsSchema()->GetUnitCP("N_CM_PER_CM");
    ECUnitCP poundForceInchPerInch = ECTestFixture::GetUnitsSchema()->GetUnitCP("LBF_IN_PER_IN");
    ECUnitCP poundForceFeetPerFeet = ECTestFixture::GetUnitsSchema()->GetUnitCP("LBF_FT_PER_FT");
    ECUnitCP kiloPoundForceFootPerFoot = ECTestFixture::GetUnitsSchema()->GetUnitCP("KPF_FT_PER_FT");

    double expected = 1e3;
    double actual;
    kiloNewtonMeterPerMeter->Convert(actual, 1.0, newtonMeterPerMeter);
    CompareValues(expected, actual, 1, "Conversion from kilo Newton Meter per Meter to Newton Meter per Meter not as expected");

    expected = 1e6;
    megaNewtonMeterPerMeter->Convert(actual, 1.0, newtonMeterPerMeter);
    CompareValues(expected, actual, 1, "Conversion from mega Newton Meter per Meter to Newton Meter per Meter not as expected");

    expected = 1.0;
    newtonCentimeterPerCentieter->Convert(actual, 1.0, newtonMeterPerMeter);
    CompareValues(expected, actual, 1, "Conversion from Newton Centimeter per Centimeter to Newton Meter per Meter not as expected");

    expected = 4.4482216153; // Value from: http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    poundForceInchPerInch->Convert(actual, 1.0, newtonMeterPerMeter);
    CompareValues(expected, actual, 50000, "Conversion from Pound-force Inch per Inch to Newton Meter per Meter not as expected");

    expected = 4.4482216153; // Value from: http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    poundForceFeetPerFeet->Convert(actual, 1.0, newtonMeterPerMeter);
    CompareValues(expected, actual, 50000, "Conversion from Pound-force Foot per Foot to Newton Meter per Meter not as expected");

    expected = 4.4482216153e3; // Value from: http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    kiloPoundForceFootPerFoot->Convert(actual, 1.0, newtonMeterPerMeter);
    CompareValues(expected, actual, 50000, "Conversion from kilo Pound-force Foot per Foot to Newton Meter per Meter not as expected");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, AreaTorqueUnitsConversions)
    {
    ECUnitCP newtonMeterPerSquareMeter = ECTestFixture::GetUnitsSchema()->GetUnitCP("N_M_PER_SQ_M");
    ECUnitCP kiloNewtonMeterPerSquareMeter = ECTestFixture::GetUnitsSchema()->GetUnitCP("KN_M_PER_SQ_M");
    ECUnitCP megaNewtonMeterPerSquareMeter = ECTestFixture::GetUnitsSchema()->GetUnitCP("MEGAN_M_PER_SQ_M");
    ECUnitCP newtonCentimeterPerSquareCentieter = ECTestFixture::GetUnitsSchema()->GetUnitCP("N_CM_PER_SQ_CM");
    ECUnitCP poundForceInchPerSquareInch = ECTestFixture::GetUnitsSchema()->GetUnitCP("LBF_IN_PER_SQ_IN");
    ECUnitCP poundForceFeetPerSquareFeet = ECTestFixture::GetUnitsSchema()->GetUnitCP("LBF_FT_PER_SQ_FT");
    ECUnitCP kiloPoundForceFootPerSquareFoot = ECTestFixture::GetUnitsSchema()->GetUnitCP("KPF_FT_PER_SQ_FT");

    double expected = 1e3;
    double actual;
    kiloNewtonMeterPerSquareMeter->Convert(actual, 1.0, newtonMeterPerSquareMeter);
    CompareValues(expected, actual, 1, "Conversion from kilo Newton Meter per square Meter to Newton Meter per square Meter not as expected");

    expected = 1e6;
    megaNewtonMeterPerSquareMeter->Convert(actual, 1.0, newtonMeterPerSquareMeter);
    CompareValues(expected, actual, 1, "Conversion from mega Newton Meter per square Meter to Newton Meter per square Meter not as expected");

    expected = 1e2;
    newtonCentimeterPerSquareCentieter->Convert(actual, 1.0, newtonMeterPerSquareMeter);
    CompareValues(expected, actual, 1, "Conversion from Newton Centimeter per square Centimeter to Newton Meter per square Meter not as expected");

    expected = 175.12683525; // Value from: http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    poundForceInchPerSquareInch->Convert(actual, 1.0, newtonMeterPerSquareMeter);
    CompareValues(expected, actual, 50000, "Conversion from Pound-force Inch per Inch to Newton square Meter per square Meter not as expected");

    expected = 14.593902937; // Value from: http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    poundForceFeetPerSquareFeet->Convert(actual, 1.0, newtonMeterPerSquareMeter);
    CompareValues(expected, actual, 50000, "Conversion from Pound-force Foot per Foot to Newton square Meter per square Meter not as expected");

    expected = 14.593902937e3; // Value from: http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    kiloPoundForceFootPerSquareFoot->Convert(actual, 1.0, newtonMeterPerSquareMeter);
    CompareValues(expected, actual, 50000, "Conversion from kilo Pound-force Foot per Foot to Newton square Meter per square Meter not as expected");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, ForceDensityUnitsConversions)
    {
    ECUnitCP newtonPerCubeMeter = ECTestFixture::GetUnitsSchema()->GetUnitCP("N_PER_CUB_M");
    ECUnitCP kiloPoundForcePerCubeFoot = ECTestFixture::GetUnitsSchema()->GetUnitCP("KPF_PER_CUB_FT");

    double expected = 1.5708746385e5;
    double actual;
    kiloPoundForcePerCubeFoot->Convert(actual, 1.0, newtonPerCubeMeter);
    CompareValues(expected, actual, 80000, "Conversion from kilo Pound-force per cube Foot to Newton per cube Meter not as expected");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, ForceUnitsConversions)
    {
    ECUnitCP newton = ECTestFixture::GetUnitsSchema()->GetUnitCP("N");
    ECUnitCP megaNewton = ECTestFixture::GetUnitsSchema()->GetUnitCP("MEGAN");

    double expected = 1e6;
    double actual;
    megaNewton->Convert(actual, 1.0, newton);
    CompareValues(expected, actual, 1, "Conversion from mega Newton to Newton not as expected");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, ProbabilityUnitConversions)
    {
    ECUnitCP probFraction = ECTestFixture::GetUnitsSchema()->GetUnitCP("PROBABILITY_FRACTION");
    ECUnitCP propPercent = ECTestFixture::GetUnitsSchema()->GetUnitCP("PROBABILITY_PERCENT");
    double expected = 100.0; // 1
    double actual;
    probFraction->Convert(actual, 1.0, propPercent);
    CompareValues(expected, actual, 1, "Conversion from Probability Fraction to Probability Percent not as expected");

    expected = 0.42; // 42%
    propPercent->Convert(actual, 42.0, probFraction);
    CompareValues(expected, actual, 1, "Conversion from Probability Percent to Probability Fraction not as expected");

    expected = 1.0;
    propPercent->Convert(actual, 1.0, propPercent);
    CompareValues(expected, actual, 1, "Conversion from Probability Percent to itself not as expected");

    probFraction->Convert(actual, 1.0, probFraction);
    CompareValues(expected, actual, 1, "Conversion from Probability Fraction to itself not as expected");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, PressureUnitConversions)
    {
    testUnitConversion(42.42, "PA_GAUGE", 42.42 + 101325.0, "PA", 1);
    testUnitConversion(42.42, "PA", 42.42 - 101325.0, "PA_GAUGE", 1);
    testUnitConversion(42.42, "KILOPASCAL_GAUGE", 42.42e3, "PA_GAUGE", 1);
    testUnitConversion(42.42, "KILOPASCAL_GAUGE", 42.42 + 101.325, "KILOPASCAL", 1);
    testUnitConversion(42.42, "KILOPASCAL", 42.42 - 101.325, "KILOPASCAL_GAUGE", 1);
    testUnitConversion(42.42, "KSI", 42.42e3, "PSI", 1);
    testUnitConversion(42.42, "PSI", 42.42e-3, "KSI", 1);
    testUnitConversion(42.42, "TORR", 42.42 * 101325.0 / 760.0, "PA", 1);
    testUnitConversion(42.43, "TORR", 5656.8680921, "PA", 2.2e3);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(42.43, "TORR", 5.6568680921e9, "MICROPASCAL", 2.1e3);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(42.43, "PSI", 2194.264589, "TORR", 1e4);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(42.43, "TORR", 0.82045935072, "PSI", 1e4);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(42.42, "ATM", 42.42 * 760.0, "TORR", 1);
    testUnitConversion(42.43, "ATM", 32246.8, "TORR", 1);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(42.42, "TORR", 42.42 / 760.0, "ATM", 1e4);
    testUnitConversion(42.43, "TORR", 0.055828947368, "ATM", 2e4);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(42.43, "ATM", 623.54910655, "PSI", 1.9e4);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(42.43, "MICROPASCAL", 42.43e-6, "PA", 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitConversionTests, TestTemperatureConversions)
    {
    bvector<Utf8String> loadErrors;
    bvector<Utf8String> conversionErrors;
    bvector<bpair<Utf8String, Utf8String>> handledUnits;

    // Expected values generated using the following equations and windows calculator
    // Celsius = (Fahrenheit-32)*5/9
    // Kelvin = ((Fahrenheit-32)*5/9)+273.15
    // Rankine = Fahrenheit+459.67
    testUnitConversion(32, "FAHRENHEIT", 0, "CELSIUS", 1);
    testUnitConversion(20, "FAHRENHEIT", -6.666666666666666666666666666, "CELSIUS", 1);
    testUnitConversion(122, "FAHRENHEIT", 50, "CELSIUS", 1);
    testUnitConversion(60, "FAHRENHEIT", 288.70555555555555555555555555556, "K", 1);
    testUnitConversion(60, "FAHRENHEIT", 519.67, "RANKINE", 1);
    testUnitConversion(0, "FAHRENHEIT", -17.777777777777777777777777777778, "CELSIUS", 1);
    testUnitConversion(0, "FAHRENHEIT", 255.37222222222222222222222222222, "K", 1);
    testUnitConversion(0, "FAHRENHEIT", 459.67, "RANKINE", 1);

    // Expected values generated using the following equations and windows calculator
    // Fahrenheit = Celsius*9/5+32
    // Kelvin = Celsius+273.15
    // Rankine = Celsius*9/5+32+459.67
    testUnitConversion(1, "CELSIUS", 33.8, "FAHRENHEIT", 1);
    testUnitConversion(-15, "CELSIUS", 5, "FAHRENHEIT", 3);
    testUnitConversion(-25, "CELSIUS", -13, "FAHRENHEIT", 3);
    testUnitConversion(60, "CELSIUS", 140, "FAHRENHEIT", 1);
    testUnitConversion(60, "CELSIUS", 333.15, "K", 1);
    testUnitConversion(60, "CELSIUS", 599.67, "RANKINE", 1);
    testUnitConversion(0, "CELSIUS", 32, "FAHRENHEIT", 1);
    testUnitConversion(0, "CELSIUS", 273.15, "K", 1);
    testUnitConversion(0, "CELSIUS", 491.67, "RANKINE", 1);

    testUnitConversion(42, "K", -231.15, "CELSIUS", 1);
    testUnitConversion(42, "K", -384.07, "FAHRENHEIT", 1);
    testUnitConversion(42, "K", 75.6, "RANKINE", 1);
    testUnitConversion(0, "K", -273.15, "CELSIUS", 1);
    testUnitConversion(0, "K", -459.67, "FAHRENHEIT", 1);
    testUnitConversion(0, "K", 0, "RANKINE", 1);


    // Expected values generated using the following equations and windows calculator
    // Celsius = (Rankine-459.67-32)*5/9
    // Fahrenheit = Rankine-459.67
    // Kelvin = (Rankine-459.67-32)*5/9+273.15
    testUnitConversion(42, "RANKINE", -249.81666666666666666666666666667, "CELSIUS", 1);
    testUnitConversion(42, "RANKINE", -417.67, "FAHRENHEIT", 1);
    testUnitConversion(42, "RANKINE", 23.333333333333333, "K", 1);
    testUnitConversion(0, "RANKINE", -273.15, "CELSIUS", 1);
    testUnitConversion(0, "RANKINE", -459.67, "FAHRENHEIT", 1);
    testUnitConversion(0, "RANKINE", 0, "K", 1);
    }

//-------------------------------------------------------------------------------------
// * @bsimethod
//+---------------+---------------+---------------+---------------+---------------+----
TEST_F(UnitConversionTests, TestInvertedSlopeUnits)
    {
    testUnitConversion(42.42, "HORIZONTAL_PER_VERTICAL", 1.0 / 42.42, "VERTICAL_PER_HORIZONTAL", 1);
    testUnitConversion(0.0, "HORIZONTAL_PER_VERTICAL", 0.0, "VERTICAL_PER_HORIZONTAL", 1, Units::UnitsProblemCode::InvertingZero);
    }

//-------------------------------------------------------------------------------------
// * @bsimethod
//+---------------+---------------+---------------+---------------+---------------+----
TEST_F(UnitConversionTests, TestBasicConversion)
    {
    testUnitConversion(10, "FT", 3048, "MM", 1);
    testUnitConversion(1, "GALLON", 3.785411784, "LITRE", 1);
    testUnitConversion(100000, "SQ_FT", 100, "THOUSAND_SQ_FT", 1);
    testUnitConversion(836127.36, "SQ_MM", 1.0, "SQ_YRD", 1);
    testUnitConversion(3.17097919837647e-7, "YR", 10000.0, "MS", 10);
    testUnitConversion(1000000.0, "LBM", 1000000.0, "LBM", 1);
    testUnitConversion(2204622621.84878, "LBM", 1000000.0, "MEGAGRAM", 10);
    testUnitConversion(1.66666666666667e-02, "ARC_DEG", 1.0, "ARC_MINUTE", 10);
    testUnitConversion(1.852e11, "CM_PER_HR", 1.0e6, "KNOT_INTERNATIONAL", 1);
    testUnitConversion(1.65409011373578e-3, "CUB_FT_PER_ACRE_SEC", 1.0e3, "CUB_M_PER_SQ_KM_DAY", 10);
    testUnitConversion(2.816539021e13, "GALLON_PER_DAY", 1234e6, "LITRE_PER_SEC", 1e5);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(4.4482216152605e5, "DYNE", 1.0, "LBF", 1);
    testUnitConversion(2.8316846592e3, "KN_PER_CUB_FT", 1.0e8, "N_PER_CUB_M", 1);
    testUnitConversion(1.0e9, "N_PER_M", 1.0e6, "N_PER_MM", 1);
    testUnitConversion(3.43774677078493e9, "DEG_PER_HR", 1.0e6, "RAD_PER_MIN", 10);
    testUnitConversion(2.65258238486492e3, "RPS", 1.0e6, "RAD_PER_MIN", 3);
    testUnitConversion(8.92179121619709e5, "LBM_PER_ACRE", 1.0e6, "KG_PER_HECTARE", 15);
    testUnitConversion(8.92179121619701e6, "LBM_PER_ACRE", 1.0e6, "G_PER_SQ_M", 10);
    testUnitConversion(8.54292974552351e7, "FT_PDL", 1.0, "KWH", 10);
    testUnitConversion(2.37303604042319e7, "FT_PDL", 1.0, "MEGAJ", 10);
    testUnitConversion(42, "KG_PER_SEC", 42000.0, "G_PER_SEC", 1);
    testUnitConversion(42, "CUB_M_PER_SEC", 2.562997252e6, "CUB_IN_PER_SEC", 1e5); // Expected value has 10 digits of precision generated using http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(2.326e6, "KJ_PER_KG", 1.0e6, "BTU_PER_LBM", 1);
    testUnitConversion(60, "G_PER_MIN", 1.0, "G_PER_SEC", 1);
    testUnitConversion(3.53146667214886e1, "KN_PER_CUB_M", 1.0, "KN_PER_CUB_FT", 1);
    testUnitConversion(42.42, "KILOPASCAL_GAUGE", 6.15250086300203, "PSIG", 2e7); // Expected value from old system, difference is due to imprecise offset in old system.
    testUnitConversion(42.42, "PERCENT_SLOPE", 0.4242, "M_PER_M", 1);
    testUnitConversion(0.42, "M_PER_M", 42.0, "PERCENT_SLOPE", 1);
    }
//-------------------------------------------------------------------------------------
// * @bsimethod
//+---------------+---------------+---------------+---------------+---------------+----
//This test is an open pot to add conversion tests which don't deserve their own method
TEST_F(UnitConversionTests, TestMiscConversions)
    {
    //Frequency
    testUnitConversion(4.2, "HZ", 4.2e-3, "KHZ", 1);
    testUnitConversion(4.2, "HZ", 4.2e-6, "MHZ", 1);
    testUnitConversion(9, "MHZ", 9000, "KHZ", 1);
    testUnitConversion(9, "KHZ", 9000, "HZ", 1);

    //MASS
    testUnitConversion(5, "TONNE", 5000, "KG", 1);
    testUnitConversion(5000, "KG", 5, "TONNE", 1);
    testUnitConversion(42.42, "TONNE", 42.42e15, "NG", 1);
    testUnitConversion(42.42, "TONNE", 42.42e3 / 0.45359237, "LBM", 1);
    testUnitConversion(42.42, "NG", 42.42e-12 / 0.45359237, "LBM", 1);
    testUnitConversion(42.42, "MKG", 42.42e-12 / 0.45359237, "KIPM", 1);
    testUnitConversion(42.42, "SLUG", (42.42 * 1000 * 0.45359237 * 9.80665) / 0.3048, "G", 1); // 0.45359237 is conversion between LBM and KG, 9.80665 is std g
    testUnitConversion(42.43, "SLUG", 6.1921930163e5, "G", 100000); // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(42.42, "KIPM", (42.42 * 1000 * 0.3048) / 9.80665, "SLUG", 1);

    //DYNAMIC_VISCOSITY
    testUnitConversion(4200.0, "POISE", 420.0, "PA_S", 1);
    testUnitConversion(42.42, "CENTIPOISE", 42.42e-2, "POISE", 1);
    testUnitConversion(42.42, "PA_S", 42.42e3, "CENTIPOISE", 1);
    testUnitConversion(42.42, "LBM_PER_FT_S", (42.42 * 0.45359237) / 0.3048, "PA_S", 1); // 0.45359237 is conversion between LBM and KG
    testUnitConversion(42.43, "LBM_PER_FT_S", 63.142796126, "PA_S", 2e4); // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(63.142796126, "PA_S", 42.43, "LBM_PER_FT_S", 3e4); // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html

    //FORCE
    testUnitConversion(1000.0, "PDL", 138.254954376, "N", 10);
    testUnitConversion(42.42, "PDL", 6.5922695314e-4, "SHORT_TON_FORCE", 1e4); // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(42.42, "PDL", 5.8859549387e-4,"LONG_TON_FORCE", 2e4);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(42.42, "PDL", 42.42 / 32.174048556430442 / 2000, "SHORT_TON_FORCE", 1); // 32.174048556430442 is 9.80665 converted to ft/s^2 using out system, 2000 is the number of pounds in a short ton
    testUnitConversion(42.42, "PDL", 42.42 / 32.174048556430442 / 2240, "LONG_TON_FORCE", 1); // 32.174048556430442 is 9.80665 converted to ft/s^2 using out system, 2240 is the number of pounds in a long ton
    testUnitConversion(42.42, "LBF", 42.42 * 32.174048556430442, "PDL", 1); // 32.174048556430442 is 9.80665 converted to ft/s^2 using out system
    testUnitConversion(42.43, "LBF", 42.43 * 32.174048556, "PDL", 1e5); //32.174048556 is 9.80665 converted to ft/s^2 using http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html

    //LENGTH
    testUnitConversion(42.42, "NAUT_MILE", 42.42 * 1852.0, "M", 1);
    testUnitConversion(42.42, "KM", 42.42 * 1000.0 / 1852.0, "NAUT_MILE", 1);

    //MOLE
    testUnitConversion(0.3, "MOL", 0.0003, "KMOL", 1);
    testUnitConversion(42.42, "KMOL", 42420.0, "MOL", 1);
    testUnitConversion(42.42, "LB_MOL", 42.42 * 453.59237, "MOL", 1);
    testUnitConversion(42.42, "KMOL", 42.42e3 / 453.59237, "LB_MOL", 1);

    //ACCELERATION
    testUnitConversion(42.42, "M_PER_SEC_SQ", 4242.0, "CM_PER_SEC_SQ", 1);
    testUnitConversion(42.42, "M_PER_SEC_SQ", 42.42 / 0.3048, "FT_PER_SEC_SQ", 1);
    testUnitConversion(42.42, "M_PER_SEC_SQ", 42.42, "M_PER_SEC_SQ", 1);
    testUnitConversion(42.42, "FT_PER_SEC_SQ", 42.42 * 0.3048, "M_PER_SEC_SQ", 1);
    testUnitConversion(42.42, "FT_PER_SEC_SQ", 42.42 * 30.48, "CM_PER_SEC_SQ", 1);
    testUnitConversion(42.42, "FT_PER_SEC_SQ", 42.42 , "FT_PER_SEC_SQ", 1);
    testUnitConversion(42.42, "CM_PER_SEC_SQ", 0.4242, "M_PER_SEC_SQ", 1);
    testUnitConversion(42.42, "CM_PER_SEC_SQ", 42.42 / 30.48, "FT_PER_SEC_SQ", 1);
    testUnitConversion(42.42, "CM_PER_SEC_SQ", 42.42, "CM_PER_SEC_SQ", 1);

    testUnitConversion(1.0, "STD_G", 9.80665, "M_PER_SEC_SQ", 1); // Exact constant value for standard gravity
    testUnitConversion(1.0, "STD_G", 32.174048556, "FT_PER_SEC_SQ", 3.1e4); // Documented value for standard gravity in m/s^2 converted to ft/s^2 using http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(1.0, "STD_G", 32.174048556430446194225721784777, "FT_PER_SEC_SQ", 1); // Expected value is the result of 9.80665 / 0.3048 done on windows calculator

    //AREA
    testUnitConversion(42.42, "SQ_UM", 42.42e-6, "SQ_MM", 1);
    testUnitConversion(42.42, "SQ_UM", 42.42e-10, "SQ_DM", 1);
    testUnitConversion(42.42, "SQ_UM", 42.42e-12 / 0.3048 / 0.3048, "SQ_FT", 1);
    testUnitConversion(42.42, "SQ_DM", 42.42e4, "SQ_MM", 1);
    testUnitConversion(42.42, "SQ_DM", 42.42e10, "SQ_UM", 1);
    testUnitConversion(42.42, "SQ_DM", 42.42e-2 / 0.3048 / 0.3048, "SQ_FT", 1);
    testUnitConversion(42.43, "SQ_DM", 4.5671271898, "SQ_FT", 1e4);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(42.42, "ARE", 42.42e-2, "HECTARE", 1);
    testUnitConversion(42.42, "HECTARE", 4242.0, "ARE", 1);
    testUnitConversion(42.42, "ARE", (42.42 * 10) / (66 * 66 * 0.3048 * 0.3048), "ACRE", 1);
    testUnitConversion(42.43, "ARE", 1.0484681336, "ACRE", 1e5);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html

    //ELECTRIC CURRENT
    testUnitConversion(42.42, "A", 42.42e-3, "KILOAMPERE", 1);
    testUnitConversion(42.42, "A", 42.42e3, "MILLIAMPERE", 1);
    testUnitConversion(42.42, "A", 42.42e6, "MICROAMPERE", 1);
    testUnitConversion(42.42, "KILOAMPERE", 42.42e9, "MICROAMPERE", 1);
    testUnitConversion(42.42, "MILLIAMPERE", 42.42e3, "MICROAMPERE", 1);
    testUnitConversion(42.42e3, "MICROAMPERE", 42.42, "MILLIAMPERE", 1);

    //Volume Flow Rate
    testUnitConversion(42.42, "CUB_IN_PER_MIN", 42.42 / 60.0, "CUB_IN_PER_SEC", 1);
    testUnitConversion(42.42, "CUB_IN_PER_SEC", 42.42 * 60.0, "CUB_IN_PER_MIN", 1);
    testUnitConversion(42.42, "CUB_IN_PER_MIN", 42.42 / pow(12.0, 3) , "CUB_FT_PER_MIN", 1);
    testUnitConversion(42.42, "ACRE_IN_PER_DAY", 42.42 / 12.0, "ACRE_FT_PER_DAY", 1);
    testUnitConversion(42.42, "ACRE_IN_PER_DAY", (42.42 * 43560.0 * 144.0) / (24.0 * 60.0), "CUB_IN_PER_MIN", 1); // 43560 is number of sq ft in an acre
    testUnitConversion(42.43, "ACRE_IN_PER_DAY", 1.8482508e5, "CUB_IN_PER_MIN", 1);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(42.42, "CUB_M_PER_SEC", (42.42 * 60.0 * pow(12, 3)) / pow(0.3048, 3) , "CUB_IN_PER_MIN", 1);
    testUnitConversion(42.43, "CUB_M_PER_SEC", 1.5535424772e8, "CUB_IN_PER_MIN", 100000);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(42.42, "CUB_M_PER_MIN", (42.42 * 24.0 * 60.0 * 12.0) / (pow(0.3048, 3) * 43560.0), "ACRE_IN_PER_DAY", 1); // 43560 is number of sq ft in an acre
    testUnitConversion(42.43, "CUB_M_PER_MIN", 594.40713084, "ACRE_IN_PER_DAY", 10000);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html

    //Force Density
    testUnitConversion(42.42, "N_PER_CUB_FT", 42.42 / pow(0.3048, 3), "N_PER_CUB_M", 1);
    testUnitConversion(42.42, "KN_PER_CUB_FT", 42.42e3, "N_PER_CUB_FT", 1);
    testUnitConversion(42.42, "N_PER_CUB_M", 42.42 * pow(0.3048, 3), "N_PER_CUB_FT", 1);

    // Heating Value
    testUnitConversion(42.42, "J_PER_KG", 42.42e-3, "KJ_PER_KG", 1);
    testUnitConversion(42.42, "MEGAJ_PER_KG", 42.42e6, "J_PER_KG", 1);
    testUnitConversion(42.42, "J_PER_KG", 42.42 * 0.45359237 / 1.05505585262e3, "BTU_PER_LBM", 1); // 1.05505585262e3 is J/BTU conversion, 0.45359237 is KG/LBM conversion

    //Molar Concentration
    testUnitConversion(42.42, "MOL_PER_CUB_DM", 42.42e6, "MICROMOL_PER_CUB_DM", 1);
    testUnitConversion(42.42, "MOL_PER_CUB_DM", 42.42e9, "NMOL_PER_CUB_DM", 1);
    testUnitConversion(42.42, "MICROMOL_PER_CUB_DM", 42.42e6, "PICOMOL_PER_CUB_DM", 1);
    testUnitConversion(42.42, "PICOMOL_PER_CUB_DM", 42.42e-12, "MOL_PER_CUB_DM", 1);
    testUnitConversion(42.42, "MOL_PER_CUB_FT", 42.42 / (pow(0.3048, 3) * 1000), "MOL_PER_CUB_DM", 1);
    testUnitConversion(42.42, "NMOL_PER_CUB_DM", 42.42e-6 * pow(0.3048, 3), "MOL_PER_CUB_FT", 2);

    //Pressure
    testUnitConversion(42.42, "PA_GAUGE", 42.42 + 101325.0, "PA", 1);
    testUnitConversion(42.42, "PA", 42.42 - 101325.0, "PA_GAUGE", 1);
    testUnitConversion(42.42, "KILOPASCAL_GAUGE", 42.42e3, "PA_GAUGE", 1);
    testUnitConversion(42.42, "KILOPASCAL_GAUGE", 42.42 + 101.325, "KILOPASCAL", 1);
    testUnitConversion(42.42, "KILOPASCAL", 42.42 - 101.325, "KILOPASCAL_GAUGE", 1);
    testUnitConversion(42.42, "KSI", 42.42e3, "PSI", 1);
    testUnitConversion(42.42, "PSI", 42.42e-3, "KSI", 1);
    testUnitConversion(42.42, "TORR", 42.42 * 101325.0 / 760.0, "PA", 1);
    testUnitConversion(42.43, "TORR", 5656.8680921, "PA", 1e4);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(42.43, "PSI", 2194.264589, "TORR", 1e4);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(42.43, "TORR", 0.82045935072, "PSI", 1e4);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(42.42, "ATM", 42.42 * 760.0, "TORR", 1);
    testUnitConversion(42.43, "ATM", 32246.8, "TORR", 1);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(42.42, "TORR", 42.42 / 760.0, "ATM", 1);
    testUnitConversion(42.43, "TORR", 0.055828947368, "ATM", 1e5);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(42.43, "ATM", 623.54910655, "PSI", 1e5);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html

    // Surface Flow Rate
    testUnitConversion(42.42, "CUB_M_PER_SQ_M_SEC", 42.42 * 60.0 * 60.0 * 24.0, "CUB_M_PER_SQ_M_DAY", 1);
    testUnitConversion(42.42, "CUB_FT_PER_SQ_FT_SEC", 42.42 * 0.3048, "CUB_M_PER_SQ_M_SEC", 1);

    //Time
    testUnitConversion(1.0, "WEEK", 7.0, "DAY", 1);
    testUnitConversion(14.0, "DAY", 2.0, "WEEK", 1);
    testUnitConversion(1.0, "YR", 3.1536e7, "S", 1);
    testUnitConversion(1.0, "YEAR_SIDEREAL", 3.155815e7, "S", 1);
    testUnitConversion(1.0, "YEAR_TROPICAL", 3.155693e7, "S", 1);
    testUnitConversion(3.155693e13, "MKS", 1.0, "YEAR_TROPICAL", 1);
    testUnitConversion(3.155815e10, "MS", 1.0, "YEAR_SIDEREAL", 1);
    testUnitConversion(1.0, "YEAR_TROPICAL",3.155693e13 , "MKS", 10);

    // Torque
    testUnitConversion(42.42, "N_CM", 0.4242, "N_M", 1);
    testUnitConversion(42.42, "LBF_FT", 42.42 * 9.80665 * 0.45359237 * 0.3048 * 100, "N_CM", 1);
    testUnitConversion(42.43, "LBF_FT", 5752.7355548, "N_CM", 2e4);

    // Velocity
    testUnitConversion(42.42, "MM_PER_SEC", 42.42e-3, "M_PER_SEC", 1);
    testUnitConversion(42.42, "KM_PER_SEC", 42.42e6, "MM_PER_SEC", 1);
    testUnitConversion(42.42, "YRD_PER_SEC", 42.42 * 0.3048 * 3.0, "M_PER_SEC", 1);
    testUnitConversion(42.42e6, "MM_PER_SEC", 42.42, "KM_PER_SEC", 1);
    testUnitConversion(42.42, "FT_PER_MIN", 42.42 / (60.0 * 3.0), "YRD_PER_SEC", 1);
    testUnitConversion(42.43, "FT_PER_MIN", 0.23572222222, "YRD_PER_SEC", 2.2e4);

    //Volume
    testUnitConversion(42.42, "CUB_UM", 42.42e-9, "CUB_MM", 1);
    testUnitConversion(42.42, "CUB_DM", 42.42e15, "CUB_UM", 1);
    testUnitConversion(42.42, "CUB_MM", 42.42e-18, "CUB_KM", 1);
    testUnitConversion(42.42, "CUB_KM", 42.42e18, "MICROLITRE", 1);
    testUnitConversion(42.42, "MICROLITRE", 42.42e-6, "CUB_DM", 1);
    testUnitConversion(42.42, "CUB_MILE", 42.42 * pow(5280.0 * 0.3048, 3) / 1.0e9, "CUB_KM", 1);
    testUnitConversion(42.43, "CUB_MILE", 176.85595485, "CUB_KM", 1e5);
    testUnitConversion(176.85595485, "CUB_KM", 42.43, "CUB_MILE", 1e5);

    //The following are not real conversions. These units are all alone in their phenomena, so we just test their 1:1 conversion to touch them once
    testUnitConversion(1.0, "COULOMB", 1.0, "COULOMB", 1);
    testUnitConversion(1.0, "W_PER_SQ_M", 1.0, "W_PER_SQ_M", 1);
    testUnitConversion(1.0, "CD", 1.0, "CD", 1);
    testUnitConversion(1.0, "STERAD", 1.0, "STERAD", 1);

    // Specific heat capacity molar
    testUnitConversion(1.0, "J_PER_MOL_K", 1000.0, "J_PER_KMOL_K", 1);
    }

//-------------------------------------------------------------------------------------
// * @bsimethod
//+---------------+---------------+---------------+---------------+---------------+----
TEST_F(UnitConversionTests, USCustomaryLengths)
    {
    // Conversion tests where expected value is taken directly out of  http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-8
    // Directly from exact values in tables
    testUnitConversion(1.0, "MILE", 63360, "IN", 1);
    testUnitConversion(1.0, "MILE", 5280, "FT", 1);
    testUnitConversion(1.0, "MILE", 1760, "YRD", 1);
    testUnitConversion(1.0, "MILE", 80, "CHAIN", 1);

    testUnitConversion(1.0, "IN", 2.54, "CM", 1);
    testUnitConversion(1.0, "FT", 30.48, "CM", 1);
    testUnitConversion(1.0, "YRD", 91.44, "CM", 1);
    testUnitConversion(1.0, "CHAIN", 66.0 * 30.48, "CM", 1); // Expected value derived from two table entries
    testUnitConversion(1.0, "MILE", 160934.4, "CM", 1);
    }


//-------------------------------------------------------------------------------------
// * @bsimethod
//+---------------+---------------+---------------+---------------+---------------+----
TEST_F(UnitConversionTests, UsSurveyLengths)
    {
    // Conversion tests where expected value is taken directly out of  http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-8
    // Exact values from document used for these conversions
    testUnitConversion(1.0, "FT", 0.999998, "US_SURVEY_FT", 1);
    testUnitConversion(1.0, "FT", 0.0254 * 39.37, "US_SURVEY_FT", 1);
    testUnitConversion(1.0, "US_SURVEY_FT", 1.0 / 0.999998, "FT", 1);
    testUnitConversion(1.0, "US_SURVEY_FT", 1200.0 / 3937.0, "M", 1);
    testUnitConversion(1.0, "M", 3937.0 / 1200.0, "US_SURVEY_FT", 1);
    testUnitConversion(1.0, "US_SURVEY_MILE", 5280.0 * 1200.0 / 3937.0, "M", 1);
    testUnitConversion(1.0, "US_SURVEY_MILE", 1.0 / 0.999998, "MILE", 1);
    testUnitConversion(1.0, "MILE", 0.999998, "US_SURVEY_MILE", 1);
    testUnitConversion(1.0, "M", 3937.0 / 1200.0 / 5280.0, "US_SURVEY_MILE", 1);
    testUnitConversion(1.0, "US_SURVEY_CHAIN", 66, "US_SURVEY_FT", 1);
    testUnitConversion(1.0, "US_SURVEY_FT", 1.0 / 66.0, "US_SURVEY_CHAIN", 1);
    testUnitConversion(1.0, "M", 39.37, "US_SURVEY_IN", 1);
    testUnitConversion(12.0, "US_SURVEY_IN", 1200.0 / 3937.0, "M", 1);

    // Directly from exact values in tables
    testUnitConversion(1.0, "US_SURVEY_MILE", 63360, "US_SURVEY_IN", 1);
    testUnitConversion(1.0, "US_SURVEY_MILE", 5280, "US_SURVEY_FT", 1);
    testUnitConversion(1.0, "US_SURVEY_MILE", 1760, "US_SURVEY_YRD", 1);
    testUnitConversion(1.0, "US_SURVEY_MILE", 80, "US_SURVEY_CHAIN", 1);

    // Exact values do not exist in document
    testUnitConversion(1.0, "US_SURVEY_FT", 0.3048006, "M", 1e8);
    testUnitConversion(1.0, "US_SURVEY_CHAIN", 20.11684, "M", 1e8);
    testUnitConversion(1.0, "US_SURVEY_YRD", 3.0 * 0.3048006, "M", 1e8);
    testUnitConversion(1.0, "US_SURVEY_MILE", 1609.347, "M", 1e9);
    }

//-------------------------------------------------------------------------------------
// * @bsimethod
//+---------------+---------------+---------------+---------------+---------------+----
TEST_F(UnitConversionTests, USCustomaryAreas)
    {
    // Conversion tests where expected value is taken directly out of  http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-8
    // Directly from exact values in tables
    testUnitConversion(1.0, "SQ_MILE", 4014489600, "SQ_IN", 1);
    testUnitConversion(1.0, "SQ_MILE", 27878400, "SQ_FT", 1);
    testUnitConversion(1.0, "SQ_MILE", 3097600, "SQ_YRD", 1);
    testUnitConversion(1.0, "SQ_MILE", 6400, "SQ_CHAIN", 1);
    testUnitConversion(1.0, "SQ_MILE", 640, "ACRE", 1);
    testUnitConversion(1.0, "SQ_CHAIN", 0.1, "ACRE", 1);
    testUnitConversion(1.0, "ACRE", 43560, "SQ_FT", 1);

    testUnitConversion(1.0, "SQ_IN", 0.00064516, "SQ_M", 1);
    testUnitConversion(1.0, "SQ_FT", 0.09290304, "SQ_M", 1);
    testUnitConversion(1.0, "SQ_YRD", 0.83612736, "SQ_M", 1);
    testUnitConversion(1.0, "SQ_CHAIN", 0.09290304 * 4356, "SQ_M", 1);
    testUnitConversion(1.0, "ACRE", 0.09290304 * 43560, "SQ_M", 1);
    testUnitConversion(1.0, "SQ_MILE", 2589988.110336, "SQ_M", 1);
    }

//-------------------------------------------------------------------------------------
// * @bsimethod
//+---------------+---------------+---------------+---------------+---------------+----
TEST_F(UnitConversionTests, USSurveyAreas)
    {
    // Conversion tests where expected value is taken directly out of  http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-8
    // Directly from exact values in tables
    testUnitConversion(1.0, "SQ_US_SURVEY_MILE", 4014489600, "SQ_US_SURVEY_IN", 1);
    testUnitConversion(1.0, "SQ_US_SURVEY_MILE", 27878400, "SQ_US_SURVEY_FT", 1);
    testUnitConversion(1.0, "SQ_US_SURVEY_MILE", 3097600, "SQ_US_SURVEY_YRD", 1);
    testUnitConversion(1.0, "SQ_US_SURVEY_MILE", 6400, "SQ_US_SURVEY_CHAIN", 1);
    testUnitConversion(1.0, "SQ_US_SURVEY_MILE", 640, "US_SURVEY_ACRE", 1);
    testUnitConversion(1.0, "SQ_US_SURVEY_CHAIN", 0.1, "US_SURVEY_ACRE", 1);
    testUnitConversion(1.0, "US_SURVEY_ACRE", 43560, "SQ_US_SURVEY_FT", 1);

    // Derived from exact values
    testUnitConversion(1.0, "SQ_IN", pow(0.999998, 2), "SQ_US_SURVEY_IN", 1);
    testUnitConversion(1.0, "SQ_FT", pow(0.999998, 2), "SQ_US_SURVEY_FT", 1);
    testUnitConversion(1.0, "SQ_YRD", pow(0.999998, 2), "SQ_US_SURVEY_YRD", 1);
    testUnitConversion(1.0, "SQ_CHAIN", pow(0.999998, 2), "SQ_US_SURVEY_CHAIN", 1);
    testUnitConversion(1.0, "ACRE", pow(0.999998, 2), "US_SURVEY_ACRE", 1);
    testUnitConversion(1.0, "SQ_MILE", pow(0.999998, 2), "SQ_US_SURVEY_MILE", 1);

    // Exact values do not exist in document
    testUnitConversion(1.0, "SQ_US_SURVEY_IN", 0.09290341 / 144.0, "SQ_M", 1e8);
    testUnitConversion(1.0, "SQ_US_SURVEY_FT", 0.09290341, "SQ_M", 1e8);
    testUnitConversion(1.0, "SQ_US_SURVEY_YRD", 9.0 * 0.09290341, "SQ_M", 1e8);
    testUnitConversion(1.0, "SQ_US_SURVEY_CHAIN", 404.6873, "SQ_M", 4e8);
    testUnitConversion(1.0, "US_SURVEY_ACRE", 4046.873, "SQ_M", 3e8);
    testUnitConversion(1.0, "SQ_US_SURVEY_MILE", 2589998, "SQ_M", 4.4e8);
    testUnitConversion(1.0, "SQ_US_SURVEY_FT", 1.000004, "SQ_FT", 1e5);
    testUnitConversion(1.0, "SQ_US_SURVEY_MILE", 1.000004, "SQ_MILE", 1e5);
    }

//-------------------------------------------------------------------------------------
// * @bsimethod
//+---------------+---------------+---------------+---------------+---------------+----
TEST_F(UnitConversionTests, UnitsConversions_Complex)
    {
    testUnitConversion(30.48 * 60, "CM_PER_HR", 1.0, "FT_PER_MIN", 1);
    testUnitConversion(30.48 * 3600, "CM_PER_HR", 1.0, "FT_PER_SEC", 1);
    testUnitConversion(2.54 * 60, "CM_PER_HR", 1.0, "IN_PER_MIN", 1);
    testUnitConversion(2.54 * 3600, "CM_PER_HR", 1.0, "IN_PER_SEC", 1);
    testUnitConversion(30.48 * 5280, "CM_PER_HR", 1.0, "MPH", 1);
    testUnitConversion(1.0 / 24.0, "CM_PER_HR", 1.0, "CM_PER_DAY", 1);
    testUnitConversion(30.48 / 24.0, "CM_PER_HR", 1.0, "FT_PER_DAY", 1);
    testUnitConversion(2.54 / 24.0, "CM_PER_HR", 1.0, "IN_PER_DAY", 1);
    testUnitConversion(100.0 / 24.0, "CM_PER_HR", 1.0, "M_PER_DAY", 1);
    testUnitConversion(0.1 / 24.0, "CM_PER_HR", 1.0, "MM_PER_DAY", 1);
    testUnitConversion(30.48 * 60e6, "CM_PER_HR", 1.0e6, "FT_PER_MIN", 1);
    testUnitConversion(30.48 * 3600e6, "CM_PER_HR", 1.0e6, "FT_PER_SEC", 1);
    testUnitConversion(2.54 * 60e6, "CM_PER_HR", 1.0e6, "IN_PER_MIN", 1);
    testUnitConversion(2.54 * 3600e6, "CM_PER_HR", 1.0e6, "IN_PER_SEC", 1);
    testUnitConversion(1.0e6 * 30.48 * 5280, "CM_PER_HR", 1.0e6, "MPH", 1);
    testUnitConversion(1.0e6 / 24.0, "CM_PER_HR", 1.0e6, "CM_PER_DAY", 1);
    testUnitConversion(30.48e6 / 24.0, "CM_PER_HR", 1.0e6, "FT_PER_DAY", 1);
    testUnitConversion(2.54e6 / 24.0, "CM_PER_HR", 1.0e6, "IN_PER_DAY", 1);
    testUnitConversion(1.0e8 / 24.0, "CM_PER_HR", 1.0e6, "M_PER_DAY", 1);
    testUnitConversion(1.0e5 / 24.0, "CM_PER_HR", 1.0e6, "MM_PER_DAY", 1);
    testUnitConversion(9.80665 / 1.0e-5, "DYNE", 1.0, "KGF", 1);
    testUnitConversion(1000 / 1.0e-5, "DYNE", 1.0, "KN", 1);
    testUnitConversion(0.001 / 1.0e-5, "DYNE", 1.0, "MN", 1);
    testUnitConversion(1.0 / 1.0e-5, "DYNE", 1.0, "N", 1);
    testUnitConversion(9.80665e6 / 1.0e-5, "DYNE", 1.0e6, "KGF", 1);
    testUnitConversion(1.0e5 / 1.01325e5, "ATM", 1.0, "BAR", 1);
    testUnitConversion(((1 + 1.01325) * 1.0e5) / 1.01325e5, "ATM", 1.0, "BAR_GAUGE", 1); //Gauge Offset was inverse in Units 1.0.  Switched - to + to correct expected value
    testUnitConversion(0.1 / 1.01325e5, "ATM", 1.0, "BARYE", 1);
    testUnitConversion(2.989067e3 / 1.01325e5, "ATM", 1.0, "FT_H2O", 1);
    testUnitConversion(249.1083 / 1.01325e5, "ATM", 1.0, "IN_H2O_AT_32F", 1);
    testUnitConversion(2.49082e2 / 1.01325e5, "ATM", 1.0, "IN_H2O_AT_39_2F", 1);
    testUnitConversion(2.4884e2 / 1.01325e5, "ATM", 1.0, "IN_H2O_AT_60F", 1);
    testUnitConversion(3.38638e3 / 1.01325e5, "ATM", 1.0, "IN_HG_AT_32F", 1);
    testUnitConversion(3.386389e3 / 1.01325e5, "ATM", 1.0, "IN_HG", 1);
    testUnitConversion(3.37685e3 / 1.01325e5, "ATM", 1.0, "IN_HG_AT_60F", 1);
    testUnitConversion(9.80665e4 / 1.01325e5, "ATM", 1.0, "AT", 1);
    testUnitConversion(((1 + 101.325 / 98.0665) * 9.80665e4) / 1.01325e5, "ATM", 1.0, "AT_GAUGE", 1); //Gauge Offset was inverse in Units 1.0.  Switched - to + to correct expected value
    testUnitConversion(9.80665 / 1.01325e5, "ATM", 1.0, "KGF_PER_SQ_M", 1);
    testUnitConversion(1000 / 1.01325e5, "ATM", 1.0, "KILOPASCAL", 1);
    testUnitConversion(((1 + 101.325) * 1000) / 1.01325e5, "ATM", 1.0, "KILOPASCAL_GAUGE", 1); //Gauge Offset was inverse in Units 1.0.  Switched - to + to correct expected value
    testUnitConversion(1000000 / 1.01325e5, "ATM", 1.0, "MEGAPASCAL", 1);
    testUnitConversion(((1 + 101.325 / 1000) * 1000000) / 1.01325e5, "ATM", 1.0, "MEGAPASCAL_GAUGE", 1); //Gauge Offset was inverse in Units 1.0.  Switched - to + to correct expected value
    testUnitConversion(9806.65 / 1.01325e5, "ATM", 1.0, "M_H2O", 1);
    testUnitConversion(9.80665 / 1.01325e5, "ATM", 1.0, "MM_H2O", 1);
    testUnitConversion(1.0, "MM_HG_AT_32F", 1.33322e2 / 101325.0, "ATM", 1);  // KnowledgeDoor and hand calculation agree with actual value more than value from old system
    testUnitConversion(1 / 1.01325e5, "ATM", 1.0, "PA", 1000);
    testUnitConversion(1.0, "LBF_PER_SQ_FT", 47.88026 / 1.01325e5, "ATM", 1e8);  // Uses NIST table conversion value for LBF/FT^2 to Pascal.  This has 7 significant digits so comparison must use reduced precision.
    testUnitConversion(1.0, "PSI", 6.894757e3 / 1.01325e5, "ATM", 1e8);  // Uses NIST table conversion value for LBF/IN^2 to Pascal.  This has 7 significant digits so comparison must use reduced precision.
    testUnitConversion(1.0, "PSIG", ((1 + 101.325 / 6.894757) * 6.894757e3) / 1.01325e5, "ATM", 1e7);  // Uses NIST table conversion value for LBF/IN^2 to Pascal.  This has 7 significant digits so comparison must use reduced precision.  Gauge Offset was inverse in Units 1.0.  Switched - to + to correct expected value
    testUnitConversion(1.0e2 / 1.01325e5, "ATM", 1.0, "MBAR", 1);
    testUnitConversion(100.0 / 1.01325e5, "ATM", 1.0, "HECTOPASCAL", 1);
    testUnitConversion(1000000.0 * 1.0e5 / 1.01325e5, "ATM", 1000000.0, "BAR", 1);
    testUnitConversion(((1000000.0 + 1.01325) * 1.0e5) / 1.01325e5, "ATM", 1000000.0, "BAR_GAUGE", 1); //Gauge Offset was inverse in Units 1.0.  Switched - to + to correct expected value
    testUnitConversion(100000 / 1.01325e5, "ATM", 1000000.0, "BARYE", 1000);
    testUnitConversion(1000000.0 * 2.989067e3 / 1.01325e5, "ATM", 1000000.0, "FT_H2O", 1);
    testUnitConversion(1000000.0 * 249.1083 / 1.01325e5, "ATM", 1000000.0, "IN_H2O_AT_32F", 1);
    testUnitConversion(1000000.0 * 2.49082e2 / 1.01325e5, "ATM", 1000000.0, "IN_H2O_AT_39_2F", 1);
    testUnitConversion(1000000.0 * 2.4884e2 / 1.01325e5, "ATM", 1000000.0, "IN_H2O_AT_60F", 1);
    testUnitConversion(1000000.0 * 3.38638e3 / 1.01325e5, "ATM", 1000000.0, "IN_HG_AT_32F", 1);
    testUnitConversion(1000000.0 * 3.386389e3 / 1.01325e5, "ATM", 1000000.0, "IN_HG", 1);
    testUnitConversion(1000000.0 * 3.37685e3 / 1.01325e5, "ATM", 1000000.0, "IN_HG_AT_60F", 1);
    testUnitConversion(1000000.0 * 9.80665e4 / 1.01325e5, "ATM", 1000000.0, "AT", 1);
    testUnitConversion(((1000000 + 101.325 / 98.0665) * 9.80665e4) / 1.01325e5, "ATM", 1000000.0, "AT_GAUGE", 1); //Gauge Offset was inverse in Units 1.0.  Switched - to + to correct expected value
    testUnitConversion(1000000.0 * 9.80665 / 1.01325e5, "ATM", 1000000.0, "KGF_PER_SQ_M", 1);
    testUnitConversion(1000000000 / 1.01325e5, "ATM", 1000000.0, "KILOPASCAL", 1);
    testUnitConversion(((1000000 + 101.325) * 1000) / 1.01325e5, "ATM", 1000000.0, "KILOPASCAL_GAUGE", 1); //Gauge Offset was inverse in Units 1.0.  Switched - to + to correct expected value
    testUnitConversion(1000000000000 / 1.01325e5, "ATM", 1000000.0, "MEGAPASCAL", 10100);
    testUnitConversion(((1000000 + 101.325 / 1000) * 1000000) / 1.01325e5, "ATM", 1000000.0, "MEGAPASCAL_GAUGE", 1); //Gauge Offset was inverse in Units 1.0.  Switched - to + to correct expected value
    testUnitConversion(1000000.0 * 9806.65 / 1.01325e5, "ATM", 1000000.0, "M_H2O", 1);
    testUnitConversion(1000000.0 * 9.80665 / 1.01325e5, "ATM", 1000000.0, "MM_H2O", 1);
    testUnitConversion((1000000.0 * 133.322) / 1.01325e5, "ATM", 1000000.0, "MM_HG_AT_32F", 1);
    testUnitConversion(1000000 / 1.01325e5, "ATM", 1000000.0, "PA", 1);
    testUnitConversion((101325.0 / 760.0) / 1.01325e5, "ATM", 1.0, "TORR", 1);// Changed approx. conversion factor for TORR from old system to exact definition, so changed expected value to match
    testUnitConversion(1000000.0 * (101325.0 / 760.0) / 1.01325e5, "ATM", 1000000.0, "TORR", 1); // Changed approx. conversion factor for TORR from old system to exact definition, so changed expected value to match
    testUnitConversion(1.0e8 / 1.01325e5, "ATM", 1.0e6, "MBAR", 1);
    testUnitConversion(100.0e6 / 1.01325e5, "ATM", 1.0e6, "HECTOPASCAL", 1);
    testUnitConversion(1.0 / (1000.0 * 3600.0), "KWH_PER_CUB_M", 1.0, "J_PER_CUB_M", 1);
    testUnitConversion(1.0 / 3600, "KWH_PER_CUB_M", 1.0, "KJ_PER_CUB_M", 1);
    testUnitConversion(1.0e6 / (1000 * 3600), "KWH_PER_CUB_M", 1.0e6, "J_PER_CUB_M", 1);
    testUnitConversion(1.0e6 / 3600, "KWH_PER_CUB_M", 1.0e6, "KJ_PER_CUB_M", 1);
    testUnitConversion(3.6 * 24, "KG_PER_DAY", 1.0, "G_PER_SEC", 1);
    testUnitConversion(1.0e6 * 3600 * 24, "MKG_PER_DAY", 1.0, "G_PER_SEC", 1);
    testUnitConversion(1.0e6 * 3600, "MKG_PER_HR", 1.0, "G_PER_SEC", 1);
    testUnitConversion(1.0e6 * 60, "MKG_PER_MIN", 1.0, "G_PER_SEC", 1);
    testUnitConversion(1.0e3 * 3600 * 24, "MG_PER_DAY", 1.0, "G_PER_SEC", 1);
    testUnitConversion(1.0e3 * 3600, "MG_PER_HR", 1.0, "G_PER_SEC", 1);
    testUnitConversion(1.0e3 * 60, "MG_PER_MIN", 1.0, "G_PER_SEC", 1);
    testUnitConversion(3600 * 1.0e6, "G_PER_HR", 1.0e6, "G_PER_SEC", 1);
    testUnitConversion(60 * 1.0e6, "G_PER_MIN", 1.0e6, "G_PER_SEC", 1);
    testUnitConversion(3.6e6 * 24, "KG_PER_DAY", 1.0e6, "G_PER_SEC", 1);
    testUnitConversion(1.0e12 * 3600 * 24, "MKG_PER_DAY", 1.0e6, "G_PER_SEC", 1);
    testUnitConversion(1.0e12 * 3600, "MKG_PER_HR", 1.0e6, "G_PER_SEC", 1);
    testUnitConversion(1.0e12 * 60, "MKG_PER_MIN", 1.0e6, "G_PER_SEC", 1);
    testUnitConversion(1.0e9 * 3600 * 24, "MG_PER_DAY", 1.0e6, "G_PER_SEC", 1);
    testUnitConversion(1.0e9 * 3600, "MG_PER_HR", 1.0e6, "G_PER_SEC", 1);
    testUnitConversion(1.0e9 * 60, "MG_PER_MIN", 1.0e6, "G_PER_SEC", 1);
    testUnitConversion(5280.0 / 2, "FT_PER_MILE", 50.0, "PERCENT_SLOPE", 1);
    testUnitConversion(1.0 / 7000.0, "KG_PER_KG", 1.0, "GRM_PER_LBM", 1);
    testUnitConversion(5.0 / 9.0, "STRAIN_PER_FAHRENHEIT", 1.0, "STRAIN_PER_CELSIUS", 1);
    testUnitConversion(5.0 / 9.0, "STRAIN_PER_FAHRENHEIT", 1.0, "STRAIN_PER_KELVIN", 1);
    testUnitConversion(9.0 / 5.0, "STRAIN_PER_CELSIUS", 1.0, "STRAIN_PER_FAHRENHEIT", 1);
    testUnitConversion(9.0 / 5.0, "STRAIN_PER_CELSIUS", 1.0, "STRAIN_PER_RANKINE", 1);
    testUnitConversion(1.0 / 0.3048, "PER_M", 1.0, "PER_FT", 1);
    testUnitConversion(1.0 / 1000.0, "PER_FT", 1.0, "PER_THOUSAND_FT", 1);
    testUnitConversion(1.0 / 5280.0, "PER_FT", 1.0, "PER_MILE", 1);
    testUnitConversion(1.0e6 / 0.3048, "PER_M", 1.0e6, "PER_FT", 1);
    testUnitConversion(1.0e6 / 1000.0, "PER_FT", 1.0e6, "PER_THOUSAND_FT", 1);
    testUnitConversion(1.0e6 / 5280.0, "PER_FT", 1.0e6, "PER_MILE", 1);
    testUnitConversion(1.0 / 0.00023884589662749597, "J_PER_KG_K", 1.0, "BTU_PER_LBM_RANKINE", 1);
    testUnitConversion(1.0e6 / 0.00023884589662749597, "J_PER_KG_K", 1.0e6, "BTU_PER_LBM_RANKINE", 1);
    }

//---------------------------------------------------------------------------------------//
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(UnitConversionTests, ApparentPower_Conversions)
    {
    testUnitConversion(1.0, "VA", 1.0e-3, "KVA", 1);
    testUnitConversion(1.0, "VA", 1.0e-6, "MVA", 1);
    testUnitConversion(1.0, "MVA", 1.0e6, "VA", 1);
    testUnitConversion(0.0, "VA", 0.0, "MVA", 1);
    }

//---------------------------------------------------------------------------------------//
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(UnitConversionTests, VolumeRatio_Conversions)
    {
    testUnitConversion(1.0, "CUB_M_PER_CUB_M", 1.0, "LITRE_PER_LITRE", 1);
    testUnitConversion(1.0, "CUB_M_PER_CUB_M", 1.0, "CUB_M_PER_CUB_M", 1);
    testUnitConversion(1.0, "LITRE_PER_LITRE", 1.0, "CUB_M_PER_CUB_M", 1);
    testUnitConversion(1.0, "LITRE_PER_LITRE", 1.0, "LITRE_PER_LITRE", 1);
    }

double pi = 3.1415926535897932;
//---------------------------------------------------------------------------------------//
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(UnitConversionTests, AngularVelocity_Conversions)
    {
    testUnitConversion(1.0, "RAD_PER_SEC", 60.0, "RAD_PER_MIN", 1);
    testUnitConversion(1.0, "RAD_PER_MIN", 60.0, "RAD_PER_HR", 1);
    testUnitConversion(1.0, "RAD_PER_HR", 1.0 / (2.0 * pi * 60.0 * 60.0), "RPS", 1);
    testUnitConversion(1.0, "RPS", 60.0, "RPM", 1);
    testUnitConversion(1.0, "RPM", 60.0, "RPH", 1);
    testUnitConversion(1.0, "RPH", 360.0 / (60.0 * 60.0), "DEG_PER_SEC", 1);
    testUnitConversion(1.0, "DEG_PER_SEC", 60.0, "DEG_PER_MIN", 1);
    testUnitConversion(1.0, "DEG_PER_MIN", 60.0, "DEG_PER_HR", 1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, SpecificHeatCapacityConversions)
    {
    // used http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(50.0, "J_PER_G_CELSIUS", 11.942294831, "BTU_PER_LBM_RANKINE", 1e5);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, ElectricalResistanceConversions)
    {
    // used http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(50.0, "OHM", 0.05, "KOHM", 1);
    testUnitConversion(1723932.235792003, "KOHM", 1723932235.792003, "OHM", 1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, ElectricalResistivityConversions)
    {
    // empty, no units besides OHM_M for this phenomena to compare with
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, EntropyConversions)
    {
    // used http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(50.0, "J_PER_K", 0.05, "KJ_PER_K", 1);
    testUnitConversion(50.0, "KJ_PER_K", 26.328253342, "BTU_PER_FAHRENHEIT", 1e5);
    testUnitConversion(50.0, "BTU_PER_FAHRENHEIT", 94955.026736, "J_PER_K", 1e5);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, ForceScaleConversions)
    {
    // used http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(50.0, "M_PER_N", 50000, "M_PER_KN", 1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, InverseForceConversions)
    {
    // used http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    testUnitConversion(50.0, "INVERSE_N", 50000.0, "INVERSE_KN", 1);
    testUnitConversion(50.0, "INVERSE_KN", 222.41108076, "INVERSE_KPF", 1e5);
    testUnitConversion(50.0, "INVERSE_KPF", 0.011240447155, "INVERSE_N", 1e5);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, LuminousEfficacyConversions)
    {
    // empty, no units besides LUMEN_PER_W for this phenomena to compare with
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitConversionTests, VolumeConversions)
    {
    // Expected values generated using the following equations and windows calculator
    // 1 CUB_M = (3937/100)^3 CUB_US_SURVEY_IN
    // 1 CUB_M = (3937/1200)^3 CUB_US_SURVEY_FT
    // 1 CUB_M = (3937/3600)^3 CUB_US_SURVEY_YRD
    // 1 CUB_M = (3937/(3600*22*80))^3 CUB_US_SURVEY_MILE

    // 1 CUB_(IN/FT/YRD/MILE) = (0.999998)^3 CUB_US_SURVEY_(IN/FT/CHAIN/MILE)

    testUnitConversion(0.0, "CUB_M", 0.0, "CUB_US_SURVEY_IN", 1);
    testUnitConversion(42.42, "CUB_M", 2.58861169276626e6, "CUB_US_SURVEY_IN", 1);
    testUnitConversion(42.42, "CUB_M", 1.49803917405454861e3, "CUB_US_SURVEY_FT", 1);
    testUnitConversion(42.42, "CUB_M", 5.5482932372390689300411522633745e1, "CUB_US_SURVEY_YRD", 1);
    testUnitConversion(42.42, "CUB_M", 1.0177038156444925341835673848984e-8, "CUB_US_SURVEY_MILE", 1);

    testUnitConversion(42.42, "CUB_IN", 42.41974548050903966064, "CUB_US_SURVEY_IN", 1);
    testUnitConversion(42.42, "CUB_FT", 42.41974548050903966064, "CUB_US_SURVEY_FT", 1);
    testUnitConversion(42.42, "CUB_YRD", 42.41974548050903966064, "CUB_US_SURVEY_YRD", 1);
    testUnitConversion(42.42, "CUB_MILE", 42.41974548050903966064, "CUB_US_SURVEY_MILE", 1);

    testUnitConversion(42.42, "CUB_US_SURVEY_IN", 2.454861111111111111111111111111e-2, "CUB_US_SURVEY_FT", 1);
    testUnitConversion(42.42, "CUB_US_SURVEY_FT", 1.5711111111111111111111111111111, "CUB_US_SURVEY_YRD", 1);
    testUnitConversion(42.42, "CUB_US_SURVEY_YRD", 7.780950648009015777610818933133e-9, "CUB_US_SURVEY_MILE", 1);
    }

//-------------------------------------------------------------------------------------
// * @bsimethod
//+---------------+---------------+---------------+---------------+---------------+----
TEST_F(UnitsTests, TestEveryDefaultUnitIsAddedToItsPhenomenon)
    {
    auto const& allUnits = ECTestFixture::GetUnitsSchema()->GetUnits();
    for (auto const& unit : allUnits)
        {
        auto const unitPhenomenon = unit->GetPhenomenon();
        ASSERT_NE(nullptr, unitPhenomenon) << "Unit " << unit->GetName().c_str() << " does not have phenomenon";
        auto it = find_if(unitPhenomenon->GetUnits().begin(), unitPhenomenon->GetUnits().end(),
                       [&unit] (Units::UnitCP unitInPhenomenon) { return unitInPhenomenon->GetName().Equals(unit->GetName().c_str()); });

        T_Utf8StringVector unitNames;
        if (unitPhenomenon->GetUnits().end() == it)
            {
            for (auto const& phenUnit : unitPhenomenon->GetUnits())
                unitNames.push_back(phenUnit->GetName());
            }
        ASSERT_NE(unitPhenomenon->GetUnits().end(), it) << "Unit " << unit->GetName() << " is not registered with it's phenomenon: " << unitPhenomenon->GetName() << "Registered units are: " << BeStringUtilities::Join(unitNames, ", ");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsTests, BasicECUnitCreation)
    {
    ECSchemaPtr schema;
    ECUnitP unit;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    PhenomenonP phenom;
    UnitSystemP system;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    schema->CreateUnitSystem(system, "ExampleUnitSystem");
    EC_EXPECT_SUCCESS(schema->CreateUnit(unit, "ExampleUnit", "M", *phenom, *system, 10.0, 1.0, 1.0, "ExampleUnitLabel", "ExampleUnitDescription"));

    EXPECT_STREQ("ExampleUnitDescription", unit->GetInvariantDescription().c_str());
    EXPECT_TRUE(unit->GetIsDescriptionDefined());
    EXPECT_STREQ("ExampleUnitLabel", unit->GetInvariantDisplayLabel().c_str());
    EXPECT_TRUE(unit->GetIsDisplayLabelDefined());
    EXPECT_STREQ("M", unit->GetDefinition().c_str());
    EXPECT_STREQ("ExampleUnit", unit->GetName().c_str());
    EXPECT_EQ(phenom, unit->GetPhenomenon());
    EXPECT_EQ(system, unit->GetUnitSystem());
    EXPECT_EQ(10.0, unit->GetNumerator());
    EXPECT_TRUE(unit->HasNumerator());
    EXPECT_EQ(1.0, unit->GetDenominator());
    EXPECT_TRUE(unit->HasDenominator());
    EXPECT_EQ(1.0, unit->GetOffset());
    EXPECT_TRUE(unit->HasOffset());
    EXPECT_TRUE(unit->HasUnitSystem());
    EXPECT_TRUE(unit->HasDefinition());
    auto testECUnit = schema->GetUnitCP("ExampleUnit");
    EXPECT_EQ(unit, testECUnit);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsTests, CanCreateUnitWithDescriptionButNoDisplayLabel)
    {
    ECSchemaPtr schema;
    ECUnitP unit;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    PhenomenonP phenom;
    UnitSystemP system;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    schema->CreateUnitSystem(system, "ExampleUnitSystem");
    EC_EXPECT_SUCCESS(schema->CreateUnit(unit, "ExampleUnit", "M", *phenom, *system, 10.0, 1.0, 1.0, "", "ExampleUnitDescription"));

    EXPECT_STREQ("ExampleUnitDescription", unit->GetInvariantDescription().c_str());
    EXPECT_TRUE(unit->GetIsDescriptionDefined());
    EXPECT_STREQ("ExampleUnit", unit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("ExampleUnit", unit->GetDisplayLabel().c_str());
    EXPECT_FALSE(unit->GetIsDisplayLabelDefined());
    EXPECT_STREQ("M", unit->GetDefinition().c_str());
    EXPECT_STREQ("ExampleUnit", unit->GetName().c_str());
    EXPECT_EQ(phenom, unit->GetPhenomenon());
    EXPECT_EQ(system, unit->GetUnitSystem());
    EXPECT_EQ(10.0, unit->GetNumerator());
    EXPECT_TRUE(unit->HasNumerator());
    EXPECT_EQ(1.0, unit->GetDenominator());
    EXPECT_TRUE(unit->HasDenominator());
    EXPECT_EQ(1.0, unit->GetOffset());
    EXPECT_TRUE(unit->HasOffset());
    EXPECT_TRUE(unit->HasUnitSystem());
    EXPECT_TRUE(unit->HasDefinition());
    auto testECUnit = schema->GetUnitCP("ExampleUnit");
    EXPECT_EQ(unit, testECUnit);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsTests, LookupUnitTest)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="01.00.00" alias="u"/>
            <Unit typeName="TestUnit" phenomenon="u:LENGTH" unitSystem="u:SI" definition="u:M"/>
        </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    auto shouldBeNull = schema->LookupUnit("");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = schema->LookupUnit("banana:M");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = schema->LookupUnit("Units:M");
    EXPECT_EQ(nullptr, shouldBeNull);
    shouldBeNull = schema->LookupUnit("u:M", true);
    EXPECT_EQ(nullptr, shouldBeNull);
    auto shouldNotBeNull = schema->LookupUnit("TestUnit");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("TestUnit", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnit("ts:TestUnit");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("TestUnit", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnit("TestSchema:TestUnit", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("TestUnit", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnit("u:M");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("M", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnit("Units:M", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("M", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnit("TS:TestUnit");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("TestUnit", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnit("TESTSCHEMA:TestUnit", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("TestUnit", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnit("U:M");
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("M", shouldNotBeNull->GetName().c_str());
    shouldNotBeNull = schema->LookupUnit("UNITS:M", true);
    ASSERT_NE(nullptr, shouldNotBeNull);
    EXPECT_STRCASEEQ("M", shouldNotBeNull->GetName().c_str());
    bvector<Units::UnitCP> units;
    schema->GetUnitsContext().AllUnits(units);
    ASSERT_EQ(schema->GetUnitCount(), units.size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsTests, ECUnitContainerTest)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    PhenomenonP phenom;
    UnitSystemP system;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    schema->CreateUnitSystem(system, "ExampleUnitSystem");

    {
    ECUnitP unit;
    EXPECT_EQ(ECObjectsStatus::Success,schema->CreateUnit(unit, "ExampleUnit1", "M", *phenom, *system, 10.0, "ExampleUnitLabel1", "ExampleUnitDescription1"));
    EXPECT_TRUE(nullptr != unit);
    }
    {
    ECUnitP unit;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateUnit(unit, "ExampleUnit2", "MM", *phenom, *system, 1.0, "ExampleUnitLabel2", "ExampleUnitDescription2"));
    EXPECT_TRUE(nullptr != unit);
    }
    {
    ECUnitP unit;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateUnit(unit, "ExampleUnit3", "MMM", *phenom, *system));
    EXPECT_TRUE(nullptr != unit);
    }
    ECUnitP unitToBeInverted;
    {
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateUnit(unitToBeInverted, "ExampleUnit4", "MMMM", *phenom, *system, nullptr, "ExampleUnitLabel4", "ExampleUnitDescription4"));
    EXPECT_TRUE(nullptr != unitToBeInverted);
    }
    {
    ECUnitP invUnit;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateInvertedUnit(invUnit, *unitToBeInverted, "InvertedUnit", *system, "Inverted Unit", "Inverted Unit"));
    EXPECT_TRUE(nullptr != invUnit);
    }
    {
    ECUnitP constant;
    EXPECT_EQ(ECObjectsStatus::Success, schema->CreateConstant(constant, "Constant", "M", *phenom, 10.0, 1.0, "Constant", "Constant"));
    EXPECT_TRUE(nullptr != constant);
    }

    EXPECT_EQ(6, schema->GetUnitCount());
    int curCount = 0;
    for (ECUnitCP unit : schema->GetUnits())
        {
        EXPECT_TRUE(nullptr != unit);
        switch (curCount)
            {
            case 0:
                ASSERT_TRUE(unit->IsConstant());
                EXPECT_STREQ("Constant", unit->GetInvariantDescription().c_str());
                EXPECT_STREQ("Constant", unit->GetInvariantDisplayLabel().c_str());
                EXPECT_TRUE(unit->GetIsDescriptionDefined());
                EXPECT_TRUE(unit->GetIsDisplayLabelDefined());
                EXPECT_STREQ("Constant", unit->GetName().c_str());
                EXPECT_STREQ("M", unit->GetDefinition().c_str());
                EXPECT_EQ(phenom, unit->GetPhenomenon());
                EXPECT_EQ(nullptr, unit->GetUnitSystem());
                EXPECT_DOUBLE_EQ(10.0, unit->GetNumerator());
                EXPECT_TRUE(unit->HasNumerator());
                EXPECT_DOUBLE_EQ(1.0, unit->GetDenominator());
                EXPECT_TRUE(unit->HasDenominator());
                EXPECT_FALSE(unit->HasUnitSystem());
                break;
            case 1:
                EXPECT_STREQ("ExampleUnitDescription1", unit->GetInvariantDescription().c_str());
                EXPECT_STREQ("ExampleUnitLabel1", unit->GetInvariantDisplayLabel().c_str());
                EXPECT_TRUE(unit->GetIsDescriptionDefined());
                EXPECT_TRUE(unit->GetIsDisplayLabelDefined());
                EXPECT_STREQ("M", unit->GetDefinition().c_str());
                EXPECT_STREQ("ExampleUnit1", unit->GetName().c_str());
                EXPECT_EQ(phenom, unit->GetPhenomenon());
                EXPECT_EQ(system, unit->GetUnitSystem());
                EXPECT_TRUE(unit->HasNumerator());
                EXPECT_EQ(10.0, unit->GetNumerator());
                EXPECT_FALSE(unit->HasDenominator());
                EXPECT_EQ(1.0, unit->GetDenominator());
                EXPECT_FALSE(unit->HasOffset());
                EXPECT_EQ(0.0, unit->GetOffset());
                EXPECT_TRUE(unit->HasUnitSystem());
                break;
            case 2:
                EXPECT_STREQ("ExampleUnitDescription2", unit->GetInvariantDescription().c_str());
                EXPECT_STREQ("ExampleUnitLabel2", unit->GetInvariantDisplayLabel().c_str());
                EXPECT_STREQ("MM", unit->GetDefinition().c_str());
                EXPECT_STREQ("ExampleUnit2", unit->GetName().c_str());
                EXPECT_TRUE(unit->GetIsDescriptionDefined());
                EXPECT_TRUE(unit->GetIsDisplayLabelDefined());
                EXPECT_EQ(phenom, unit->GetPhenomenon());
                EXPECT_EQ(system, unit->GetUnitSystem());
                EXPECT_TRUE(unit->HasNumerator());
                EXPECT_EQ(1.0, unit->GetNumerator());
                EXPECT_FALSE(unit->HasDenominator());
                EXPECT_EQ(1.0, unit->GetDenominator());
                EXPECT_FALSE(unit->HasOffset());
                EXPECT_EQ(0.0, unit->GetOffset());
                EXPECT_TRUE(unit->HasUnitSystem());
                break;
            case 3:
                EXPECT_STREQ("ExampleUnit3", unit->GetInvariantDisplayLabel().c_str());
                EXPECT_STREQ("MMM", unit->GetDefinition().c_str());
                EXPECT_STREQ("ExampleUnit3", unit->GetName().c_str());
                EXPECT_FALSE(unit->GetIsDescriptionDefined());
                EXPECT_FALSE(unit->GetIsDisplayLabelDefined());
                EXPECT_EQ(phenom, unit->GetPhenomenon());
                EXPECT_EQ(system, unit->GetUnitSystem());
                EXPECT_FALSE(unit->HasNumerator());
                EXPECT_EQ(1.0, unit->GetNumerator());
                EXPECT_FALSE(unit->HasDenominator());
                EXPECT_EQ(1.0, unit->GetDenominator());
                EXPECT_FALSE(unit->HasOffset());
                EXPECT_EQ(0.0, unit->GetOffset());
                EXPECT_TRUE(unit->HasUnitSystem());
                break;
            case 4:
                EXPECT_STREQ("ExampleUnitDescription4", unit->GetInvariantDescription().c_str());
                EXPECT_STREQ("ExampleUnitLabel4", unit->GetInvariantDisplayLabel().c_str());
                EXPECT_STREQ("MMMM", unit->GetDefinition().c_str());
                EXPECT_STREQ("ExampleUnit4", unit->GetName().c_str());
                EXPECT_TRUE(unit->GetIsDescriptionDefined());
                EXPECT_TRUE(unit->GetIsDisplayLabelDefined());
                EXPECT_EQ(phenom, unit->GetPhenomenon());
                EXPECT_EQ(system, unit->GetUnitSystem());
                EXPECT_FALSE(unit->HasNumerator());
                EXPECT_EQ(1.0, unit->GetNumerator());
                EXPECT_FALSE(unit->HasDenominator());
                EXPECT_EQ(1.0, unit->GetDenominator());
                EXPECT_FALSE(unit->HasOffset());
                EXPECT_EQ(0.0, unit->GetOffset());
                EXPECT_TRUE(unit->HasUnitSystem());
                break;
            case 5:
                ASSERT_TRUE(unit->IsInvertedUnit());
                EXPECT_FALSE(unit->HasDefinition());
                EXPECT_STREQ("Inverted Unit", unit->GetInvariantDescription().c_str());
                EXPECT_STREQ("Inverted Unit", unit->GetInvariantDisplayLabel().c_str());
                EXPECT_TRUE(unit->GetIsDescriptionDefined());
                EXPECT_TRUE(unit->GetIsDisplayLabelDefined());
                EXPECT_STREQ("InvertedUnit", unit->GetName().c_str());
                EXPECT_EQ(phenom, unit->GetPhenomenon());
                EXPECT_EQ(system, unit->GetUnitSystem());
                EXPECT_TRUE(unit->HasUnitSystem());
                EXPECT_NE(nullptr, unit->GetInvertingUnit());
                EXPECT_FALSE(unit->HasNumerator());
                EXPECT_FALSE(unit->HasDenominator());
                EXPECT_FALSE(unit->HasOffset());
                break;
            }
        curCount++;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitsTests, StandaloneSchemaChildECUnit)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);
    PhenomenonP phenom;
    UnitSystemP system;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    schema->CreateUnitSystem(system, "ExampleUnitSystem");
    ECUnitP unit;
    schema->CreateUnit(unit, "ExampleUnit", "M", *phenom, *system, 10.0, 1.0, 1.0, "ExampleUnitLabel", "ExampleUnitDescription");

    BeJsDocument schemaJson;
    EXPECT_TRUE(unit->ToJson(schemaJson, true));

    BeJsDocument testDataJson;

    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneUnit.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitsTests, WritingToPre32VersionShouldNotWriteUnit)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);
    PhenomenonP phenom;
    UnitSystemP system;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    schema->CreateUnitSystem(system, "ExampleUnitSystem");
    ECUnitP unit;
    schema->CreateUnit(unit, "ExampleUnit", "M", *phenom, *system, 10.0, 1.0, 1.0, "ExampleUnitLabel", "ExampleUnitDescription");

    Utf8String out;
    ASSERT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(out, ECVersion::V3_1));
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, out.c_str(), *context);
    ASSERT_EQ(nullptr, schema->GetUnitCP("ExampleUnit"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsTests, AllUnitsInStandardUnitsSchemaHaveValidDefinitions)
    {
    ECSchemaPtr schema = ECTestFixture::GetUnitsSchema();

    bvector<Units::UnitCP> allUnits;
    schema->GetUnitsContext().AllUnits(allUnits);

    for(auto const& unit: allUnits)
        {
        Utf8StringCR expression = ((ECUnitCP)unit)->GetParsedUnitExpression();
        ASSERT_FALSE(expression.empty());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, BasicRoundTripTest)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit" offset="10.1234567890" numerator="10.1234567890" denominator="10.1234567890"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ASSERT_EQ(1, schema->GetUnitCount());
    ECUnitCP unit = schema->GetUnitCP("TestUnit");
    ASSERT_TRUE(nullptr != unit);
    ASSERT_EQ(&unit->GetSchema(), schema.get());

    EXPECT_STREQ("Unit", unit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Unit", unit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", unit->GetDefinition().c_str());
    EXPECT_DOUBLE_EQ(10.1234567890, unit->GetOffset());
    EXPECT_TRUE(unit->HasOffset());
    EXPECT_DOUBLE_EQ(10.1234567890, unit->GetNumerator());
    EXPECT_TRUE(unit->HasNumerator());
    EXPECT_DOUBLE_EQ(10.1234567890, unit->GetDenominator());
    EXPECT_TRUE(unit->HasDenominator());

    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchemaXml));
    }
    {
    ECSchemaPtr serializedSchema;
    ECSchemaReadContextPtr serializedContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedSchema, serializedSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(1, serializedSchema->GetUnitCount());
    ECUnitCP serializedECUnit = serializedSchema->GetUnitCP("TestUnit");
    ASSERT_TRUE(nullptr != serializedECUnit);

    EXPECT_STREQ("Unit", serializedECUnit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Unit", serializedECUnit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", serializedECUnit->GetDefinition().c_str());
    EXPECT_DOUBLE_EQ(10.1234567890, serializedECUnit->GetOffset());
    EXPECT_TRUE(serializedECUnit->HasOffset());
    EXPECT_DOUBLE_EQ(10.1234567890, serializedECUnit->GetNumerator());
    EXPECT_TRUE(serializedECUnit->HasNumerator());
    EXPECT_DOUBLE_EQ(10.1234567890, serializedECUnit->GetDenominator());
    EXPECT_TRUE(serializedECUnit->HasDenominator());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, EmptyDisplayLabelRoundTripTest)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="B" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="A" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="" definition="M" description="This is an awesome new Unit" offset="10.1234567890" numerator="10.1234567890" denominator="10.1234567890"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ASSERT_EQ(1, schema->GetUnitCount());
    ECUnitCP unit = schema->GetUnitCP("TestUnit");
    ASSERT_TRUE(nullptr != unit);

    EXPECT_STREQ("TestUnit", unit->GetInvariantDisplayLabel().c_str());

    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchemaXml));
	ASSERT_FALSE(serializedSchemaXml.Contains("displayLabel=\"TestUnit\""));
    ASSERT_FALSE(serializedSchemaXml.Contains("displayLabel=\"\""));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, RoundTripWithReferencedSchemaForPhenomenonAndUnitSystem)
    {
    Utf8CP refXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="refSchema" version="01.00.00" alias="rs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        </ECSchema>)xml";
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="refSchema" version="01.00.00" alias="rs"/>
            <Unit typeName="TestUnit" phenomenon="rs:TestPhenomenon" unitSystem="rs:TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    Utf8String serializedRefSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refXml, *context));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ASSERT_EQ(1, schema->GetUnitCount());
    ECUnitCP unit = schema->GetUnitCP("TestUnit");
    ASSERT_TRUE(nullptr != unit);
    ASSERT_EQ(&unit->GetSchema(), schema.get());

    EXPECT_STREQ("Unit", unit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Unit", unit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", unit->GetDefinition().c_str());
    EXPECT_STREQ("LENGTH*LENGTH", unit->GetPhenomenon()->GetDefinition().c_str());
    EXPECT_STREQ("Unit System", ((ECN::UnitSystemCP)unit->GetUnitSystem())->GetInvariantDisplayLabel().c_str());
    EXPECT_FALSE(unit->HasOffset());
    EXPECT_FALSE(unit->HasNumerator());
    EXPECT_FALSE(unit->HasDenominator());
    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchemaXml));
    EXPECT_EQ(SchemaWriteStatus::Success, refSchema->WriteToXmlString(serializedRefSchemaXml));
    }
    {
    ECSchemaPtr serializedSchema;
    ECSchemaPtr serializedRefSchema;
    ECSchemaReadContextPtr serializedContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedRefSchema, serializedRefSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedSchema, serializedSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(1, serializedSchema->GetUnitCount());
    ECUnitCP serializedUnit = serializedSchema->GetUnitCP("TestUnit");
    ASSERT_TRUE(nullptr != serializedUnit);

    EXPECT_STREQ("Unit", serializedUnit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Unit", serializedUnit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", serializedUnit->GetDefinition().c_str());
    EXPECT_STREQ("LENGTH*LENGTH", serializedUnit->GetPhenomenon()->GetDefinition().c_str());
    EXPECT_STREQ("Unit System", ((ECN::UnitSystemCP)serializedUnit->GetUnitSystem())->GetInvariantDisplayLabel().c_str());
    EXPECT_FALSE(serializedUnit->HasOffset());
    EXPECT_FALSE(serializedUnit->HasNumerator());
    EXPECT_FALSE(serializedUnit->HasDenominator());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, ShouldFailWithoutAliases)
    {
    Utf8CP refXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="refSchema" version="01.00.00" alias="rs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        </ECSchema>)xml";
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="refSchema" version="01.00.00" alias="rs"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    Utf8String serializedRefSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refXml, *context));
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    }

Utf8CP baseSchemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="LENGTH" displayLabel="Length" definition="LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="SI" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="M" phenomenon="LENGTH" unitSystem="SI" displayLabel="meter" definition="M"/>
            %s
        </ECSchema>)xml";

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, BaseSchemaLoads)
    {
    auto xmlString = Utf8PrintfString(baseSchemaXml, "");
    ExpectSchemaDeserializationSuccess(xmlString.c_str(), "Should deserialize base schema xml used for invalid xml tests.");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, ShouldFailWithoutPhenomenonAndUnitSystemBeingDefined)
    {
    auto xmlString = Utf8PrintfString(baseSchemaXml, R"xml(<Unit typeName="TestUnit" phenomenon="LENGTH" unitSystem="SI" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>)xml");
    ExpectSchemaDeserializationSuccess(xmlString.c_str(), "Should deserialize unit with properly defined unit system and phenomenon");

    xmlString = Utf8PrintfString(baseSchemaXml, R"xml(<Unit typeName="TestUnit" phenomenon="LENGTH" unitSystem="bad" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>)xml");
    ExpectSchemaDeserializationFailure(xmlString.c_str(), SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit without properly defined unit system");
    xmlString = Utf8PrintfString(baseSchemaXml, R"xml(<Unit typeName="TestUnit" phenomenon="bad" unitSystem="SI" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>)xml");
    ExpectSchemaDeserializationFailure(xmlString.c_str(), SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit without properly defined phenomenon");
    xmlString = Utf8PrintfString(baseSchemaXml, R"xml(<Unit typeName="TestUnit" phenomenon="LENGTH" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>)xml");
    ExpectSchemaDeserializationFailure(xmlString.c_str(), SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit without unit system attribute");
    xmlString = Utf8PrintfString(baseSchemaXml, R"xml(<Unit typeName="TestUnit" unitSystem="SI" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>)xml");
    ExpectSchemaDeserializationFailure(xmlString.c_str(), SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit without phenomenon attribute");
    xmlString = Utf8PrintfString(baseSchemaXml, R"xml(<Unit typeName="TestUnit" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>)xml");
    ExpectSchemaDeserializationFailure(xmlString.c_str(), SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit without unit system and phenomenon attributes");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, DuplicateUnitNames)
    {
    auto xmlString = Utf8PrintfString(baseSchemaXml, 
        R"xml(<Unit typeName="TestUnit" phenomenon="LENGTH" unitSystem="SI" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <Unit typeName="TestUnit" phenomenon="LENGTH" unitSystem="SI" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>)xml");
    ExpectSchemaDeserializationFailure(xmlString.c_str(), SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize schema with two units with the same name");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, DuplicateSchemaChildNames)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <PropertyCategory typeName="TestUnit"/>
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="TestPhenomenon" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="TestUnit" description="This is an awesome new Unit" numerator="10.0"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize schema with duplicate schema child names, one being a unit");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, ShouldFailWithBadUnitSystemName)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="TestPhenomenon" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem:" displayLabel="Unit" definition="M" description="This is an awesome new Unit" numerator="10.0"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with bad unit system name (fails parseClassName because of colon at end");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="TestPhenomenon" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="badalias:TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit" numerator="10.0"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with bad unit system alias");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="TestPhenomenon" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="" displayLabel="Unit" definition="M" description="This is an awesome new Unit" numerator="10.0"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with empty unit system name");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, ShouldFailWithBadPhenomName)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="TestPhenomenon" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="BadPhenom:" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit" numerator="10.0"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with a bad phenom name (fails parseclassname because of colon at end");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="TestPhenomenon" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="badAlias:TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit" numerator="10.0"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with a phenom with a bad alias");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="TestPhenomenon" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit" numerator="10.0"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with an empty phenom name");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, MissingOrInvalidName)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="TestPhenomenon" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with missing name");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="TestPhenomenon" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with empty name");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, MissingOrInvalidNumerator)
    {
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="TestPhenomenon" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="Smoot" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ECUnitCP unit = schema->GetUnitCP("Smoot");
    ASSERT_FALSE(unit->HasNumerator());
    ASSERT_DOUBLE_EQ(1.0, unit->GetNumerator());
    }

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="Smoot" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="Smoots" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with non numeric numerator");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="Smoot" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with empty numerator");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="Smoot" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="0.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with 0.0 numerator");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, MissingOrInvalidDenominator)
    {
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="Smoot" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="1.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ECUnitCP unit = schema->GetUnitCP("Smoot");
    ASSERT_FALSE(unit->HasDenominator());
    ASSERT_DOUBLE_EQ(1.0, unit->GetDenominator());
    }

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="Smoot" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="1.0" denominator="Smoots" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with an invalid (non-numeric) denominator");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="Smoot" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="1.0" denominator="0.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with a 0 denominator");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="Smoot" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="1.0" denominator="" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with an empty denominator");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, MissingOrInvalidOffset)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit" numerator="10.0"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    EXPECT_DOUBLE_EQ(schema->GetUnitCP("TestUnit")->GetOffset(), 0.0);

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="Smoot" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="1.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit" offset="bananas"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with an invalid (non-numeric) offset");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="Smoot" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="1.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit" offset=""/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with an empty offset");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, MissingDisplayLabel)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" definition="M" description="This is an awesome new Unit"/>
    </ECSchema>)xml";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    ECUnitCP unit = schema->GetUnitCP("TestUnit");
    EXPECT_STREQ("TestUnit", unit->GetInvariantDisplayLabel().c_str());
    EXPECT_FALSE(unit->GetIsDisplayLabelDefined());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(UnitsDeserializationTests, MissingOrEmptyDefinition)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" description="This is an awesome new Unit"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with missing definition");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" definition="" displayLabel="Unit" description="This is an awesome new Unit"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize unit with empty definintion");
    }

//=======================================================================================
//! InvertedUnitsTests
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(InvertedUnitsTests, BasicCreation)
    {
    ECSchemaPtr schema;
    ECUnitP unit;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    PhenomenonP phenom;
    UnitSystemP system;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    schema->CreateUnitSystem(system, "ExampleUnitSystem");
    EC_EXPECT_SUCCESS(schema->CreateUnit(unit, "ExampleUnit", "M", *phenom, *system, 10.0, 1.0, 1.0, "ExampleUnitLabel", "ExampleUnitDescription"));
    ECUnitP invertedUnit;
    EC_EXPECT_SUCCESS(schema->CreateInvertedUnit(invertedUnit, *unit, "ExampleInvertedUnit", *system, "ExampleInvertedUnitLabel", "ExampleInvertedUnitDescription"));
    auto testECUnit = schema->GetUnitCP("ExampleUnit");
    EXPECT_EQ(unit, testECUnit);
    auto schemaInvertedUnit = schema->GetInvertedUnitCP("ExampleInvertedUnit");
    EXPECT_EQ(invertedUnit, schemaInvertedUnit);
    EXPECT_FALSE(invertedUnit->HasOffset());
    EXPECT_FALSE(invertedUnit->HasNumerator());
    EXPECT_FALSE(invertedUnit->HasDenominator());
    EXPECT_STRCASEEQ("ExampleInvertedUnit", invertedUnit->GetName().c_str());
    EXPECT_EQ(system, invertedUnit->GetUnitSystem());
    EXPECT_STRCASEEQ("ExampleInvertedUnitLabel", invertedUnit->GetInvariantDisplayLabel().c_str());
    EXPECT_STRCASEEQ("ExampleInvertedUnitDescription", invertedUnit->GetInvariantDescription().c_str());
    EXPECT_TRUE(invertedUnit->GetIsDisplayLabelDefined());
    EXPECT_TRUE(invertedUnit->GetIsDescriptionDefined());
    EXPECT_EQ(unit, invertedUnit->GetInvertingUnit());
    EXPECT_FALSE(invertedUnit->HasDefinition());
    EXPECT_STREQ(unit->GetDefinition().c_str(), invertedUnit->GetDefinition().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(InvertedUnitsTests, StandaloneSchemaChild)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);
    PhenomenonP phenom;
    UnitSystemP system;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    schema->CreateUnitSystem(system, "ExampleUnitSystem");
    ECUnitP unit;
    schema->CreateUnit(unit, "ExampleUnit", "M", *phenom, *system, 10.0, 1.0, 1.0, "ExampleUnitLabel", "ExampleUnitDescription");
    ECUnitP invUnit;
    schema->CreateInvertedUnit(invUnit, *unit, "ExampleInvertedUnit", *system, "ExampleUnitLabel", "ExampleUnitDescription");

    BeJsDocument schemaJson;
    EXPECT_TRUE(invUnit->ToJson(schemaJson, true));
    BeJsDocument testDataJson;

    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneInvertedUnit.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitsTests, WritingToPre32VersionShouldNotWriteInvertedUnit)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);
    PhenomenonP phenom;
    UnitSystemP system;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    schema->CreateUnitSystem(system, "ExampleUnitSystem");
    ECUnitP unit;
    ECUnitP inv;
    schema->CreateUnit(unit, "ExampleUnit", "M", *phenom, *system, 10.0, 1.0, 1.0, "ExampleUnitLabel", "ExampleUnitDescription");
    schema->CreateInvertedUnit(inv, *unit, "Inv", *system);

    Utf8String out;
    ASSERT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(out, ECVersion::V3_1));
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, out.c_str(), *context);
    ASSERT_EQ(nullptr, schema->GetInvertedUnitCP("Inv"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(InvertedUnitsDeserializationTests, RoundTripWithReferencedSchema)
    {
    Utf8CP refXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="refSchema" version="01.00.00" alias="rs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="1.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
        </ECSchema>)xml";
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="refSchema" version="01.00.00" alias="rs"/>
            <InvertedUnit typeName="TestInvertedUnit" invertsUnit="rs:TestUnit" unitSystem="rs:TestUnitSystem" displayLabel="InvertedUnitLabel" description="InvertedUnitDescription"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    Utf8String serializedRefSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refXml, *context));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ASSERT_EQ(1, schema->GetUnitCount());
    ECUnitCP unit = schema->GetInvertedUnitCP("TestInvertedUnit");
    ASSERT_TRUE(nullptr != unit);
    ASSERT_EQ(&unit->GetSchema(), schema.get());

    EXPECT_STREQ("InvertedUnitLabel", unit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("InvertedUnitDescription", unit->GetInvariantDescription().c_str());
    EXPECT_TRUE(unit->GetIsDescriptionDefined());
    EXPECT_TRUE(unit->GetIsDisplayLabelDefined());
    EXPECT_FALSE(unit->HasOffset());
    EXPECT_FALSE(unit->HasNumerator());
    EXPECT_FALSE(unit->HasDenominator());
    EXPECT_STREQ("LENGTH*LENGTH", unit->GetPhenomenon()->GetDefinition().c_str());
    EXPECT_STREQ("Unit System", ((ECN::UnitSystemCP)unit->GetUnitSystem())->GetInvariantDisplayLabel().c_str());
    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchemaXml));
    EXPECT_EQ(SchemaWriteStatus::Success, refSchema->WriteToXmlString(serializedRefSchemaXml));
    }
    {
    ECSchemaPtr serializedSchema;
    ECSchemaPtr serializedRefSchema;
    ECSchemaReadContextPtr serializedContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedRefSchema, serializedRefSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedSchema, serializedSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(1, serializedSchema->GetUnitCount());
    ECUnitCP serializedUnit = serializedSchema->GetInvertedUnitCP("TestInvertedUnit");
    ASSERT_TRUE(nullptr != serializedUnit);

    EXPECT_STREQ("InvertedUnitLabel", serializedUnit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("InvertedUnitDescription", serializedUnit->GetInvariantDescription().c_str());
    EXPECT_STREQ("LENGTH*LENGTH", serializedUnit->GetPhenomenon()->GetDefinition().c_str());
    EXPECT_STREQ("Unit System", ((ECN::UnitSystemCP)serializedUnit->GetUnitSystem())->GetInvariantDisplayLabel().c_str());
    EXPECT_TRUE(serializedUnit->GetIsDescriptionDefined());
    EXPECT_TRUE(serializedUnit->GetIsDisplayLabelDefined());
    EXPECT_FALSE(serializedUnit->HasOffset());
    EXPECT_FALSE(serializedUnit->HasNumerator());
    EXPECT_FALSE(serializedUnit->HasDenominator());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(InvertedUnitsDeserializationTests, BasicRoundTripTest)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" numerator="1.0" definition="M" description="This is an awesome new Unit"/>
            <InvertedUnit typeName="TestInvertedUnit" invertsUnit="TestUnit" unitSystem="TestUnitSystem" displayLabel="InvertedUnitLabel" description="InvertedUnitDescription"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ASSERT_EQ(2, schema->GetUnitCount());
    ECUnitCP unit = schema->GetUnitCP("TestUnit");
    ASSERT_TRUE(nullptr != unit);
    ASSERT_EQ(&unit->GetSchema(), schema.get());

    EXPECT_STREQ("Unit", unit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Unit", unit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", unit->GetDefinition().c_str());

    ECUnitCP invUnit = schema->GetInvertedUnitCP("TestInvertedUnit");
    ASSERT_TRUE(nullptr != invUnit);
    ASSERT_EQ(&unit->GetSchema(), schema.get());

    EXPECT_STREQ("TestUnitSystem",((ECN::UnitSystemCP)invUnit->GetUnitSystem())->GetName().c_str());
    EXPECT_STREQ("InvertedUnitDescription", invUnit->GetDescription().c_str());
    EXPECT_STREQ("InvertedUnitLabel", invUnit->GetInvariantDisplayLabel().c_str());
    ASSERT_TRUE(invUnit->IsInvertedUnit());
    EXPECT_TRUE(unit->HasUnitSystem());
    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchemaXml));
    }
    {
    ECSchemaPtr serializedSchema;
    ECSchemaReadContextPtr serializedContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedSchema, serializedSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(2, serializedSchema->GetUnitCount());
    ECUnitCP serializedECUnit = serializedSchema->GetUnitCP("TestUnit");
    ASSERT_TRUE(nullptr != serializedECUnit);
    ECUnitCP serializedInvertedUnit = serializedSchema->GetInvertedUnitCP("TestInvertedUnit");
    ASSERT_TRUE(nullptr != serializedInvertedUnit);

    EXPECT_STREQ("Unit", serializedECUnit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Unit", serializedECUnit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", serializedECUnit->GetDefinition().c_str());

    EXPECT_STREQ("InvertedUnitLabel", serializedInvertedUnit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("InvertedUnitDescription", serializedInvertedUnit->GetDescription().c_str());
    ASSERT_TRUE(serializedInvertedUnit->IsInvertedUnit());
    EXPECT_TRUE(serializedInvertedUnit->HasUnitSystem());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(InvertedUnitsDeserializationTests, ShouldFailWithoutAliases)
    {
    Utf8CP refXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="refSchema" version="01.00.00" alias="rs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" numerator="1.0" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
        </ECSchema>)xml";
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="refSchema" version="01.00.00" alias="rs"/>
            <InvertedUnit typeName="TestInvertedUnit" invertsUnit="TestUnit" unitSystem="TestUnitSystem" displayLabel="InvertedUnitLabel" description="InvertedUnitDescription"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    Utf8String serializedRefSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refXml, *context));
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(InvertedUnitsDeserializationTests, ShouldFailWithoutUnitSystemDefined)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Unit typeName="TestUnit" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Unit" definition="M" description="This is an awesome new Unit"/>
            <InvertedUnit typeName="TestInvertedUnit" invertsUnit="TestUnit" unitSystem="bananas" displayLabel="InvertedUnitLabel" description="InvertedUnitDescription"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize inverted unit with a unit system that doesn't exist");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(InvertedUnitsDeserializationTests, ShouldFailWithoutInvertsUnitDefinedOrEmptyOrMissingInvertsUnit)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <InvertedUnit typeName="TestInvertedUnit" invertsUnit="TestUnit" unitSystem="TestUnitSystem" displayLabel="InvertedUnitLabel" description="InvertedUnitDescription"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize an InvertedUnit with unit that doesn't exist");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <InvertedUnit typeName="TestInvertedUnit" invertsUnit="" unitSystem="TestUnitSystem" displayLabel="InvertedUnitLabel" description="InvertedUnitDescription"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize an Inverted unit with empty invertsUnit");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <InvertedUnit typeName="TestInvertedUnit" unitSystem="TestUnitSystem" displayLabel="InvertedUnitLabel" description="InvertedUnitDescription"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize an inverted unit with missing invertsUnit");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(InvertedUnitsDeserializationTests, ShouldFailWithBadSchemaAliases)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="Units" version="1.0.0" alias="u"/>
            <InvertedUnit typeName="TestInvertedUnit" invertsUnit="units:M" unitSystem="u:SI" displayLabel="InvertedUnitLabel" description="InvertedUnitDescription"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize with bad schema alias on invertsUnit");
    }

//=======================================================================================
//! ConstantTests
//=======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantTests, BasicCreation)
    {
    ECSchemaPtr schema;
    ECUnitP unit;

    ECSchema::CreateSchema(schema, "TestSchema", "ts", 1, 0, 0);
    PhenomenonP phenom;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    EC_EXPECT_SUCCESS(schema->CreateConstant(unit, "CONSTANT", "M", *phenom, 10, 10.0, "ConstantLabel", "ConstantDescription"));

    EXPECT_STREQ("ConstantDescription", unit->GetInvariantDescription().c_str());
    EXPECT_TRUE(unit->GetIsDescriptionDefined());
    EXPECT_STREQ("ConstantLabel", unit->GetInvariantDisplayLabel().c_str());
    EXPECT_TRUE(unit->GetIsDisplayLabelDefined());
    EXPECT_STREQ("M", unit->GetDefinition().c_str());
    EXPECT_STREQ("CONSTANT", unit->GetName().c_str());
    EXPECT_EQ(phenom, unit->GetPhenomenon());
    EXPECT_EQ(nullptr, unit->GetUnitSystem());
    EXPECT_EQ(10.0, unit->GetNumerator());
    EXPECT_TRUE(unit->HasNumerator());
    EXPECT_EQ(10.0, unit->GetDenominator());
    EXPECT_TRUE(unit->HasDenominator());
    EXPECT_FALSE(unit->HasOffset());
    EXPECT_FALSE(unit->HasUnitSystem());
    EXPECT_TRUE(unit->HasDefinition());

    auto testECUnit = schema->GetUnitCP("CONSTANT");
    EXPECT_EQ(unit, testECUnit);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConstantTests, StandaloneSchemaChild)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);
    PhenomenonP phenom;
    UnitSystemP system;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    schema->CreateUnitSystem(system, "ExampleUnitSystem");
    ECUnitP unit;
    schema->CreateConstant(unit, "ExampleConstant", "M", *phenom, 10.0, 1.0, "ExampleConstantLabel", "ExampleConstantDescription");

    BeJsDocument schemaJson;
    EXPECT_TRUE(unit->ToJson(schemaJson, true));

    BeJsDocument testDataJson;

    BeFileName testDataFile(ECTestFixture::GetTestDataPath(L"ECJson/StandaloneConstant.ecschema.json"));
    auto readJsonStatus = ECTestUtility::ReadJsonInputFromFile(testDataJson, testDataFile);
    ASSERT_EQ(BentleyStatus::SUCCESS, readJsonStatus);

    EXPECT_TRUE(ECTestUtility::JsonDeepEqual(schemaJson, testDataJson)) << ECTestUtility::JsonSchemasComparisonString(schemaJson, testDataJson);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ConstantTests, WritingToPre32VersionShouldNotWriteConstant)
    {
    ECSchemaPtr schema;
    ECSchema::CreateSchema(schema, "ExampleSchema", "ex", 3, 1, 0, ECVersion::Latest);
    PhenomenonP phenom;
    schema->CreatePhenomenon(phenom, "ExamplePhenomenon", "LENGTH");
    ECUnitP unit;
    schema->CreateConstant(unit, "Constant", "M", *phenom, 10.0);

    Utf8String out;
    ASSERT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(out, ECVersion::V3_1));
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchema::ReadFromXmlString(schema, out.c_str(), *context);
    ASSERT_EQ(nullptr, schema->GetConstantCP("Constant"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantDeserializationTests, BasicRoundTripTest)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Constant typeName="TestConstant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="10.0" denominator="1.0" />
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ASSERT_EQ(1, schema->GetUnitCount());
    ECUnitCP unit = schema->GetConstantCP("TestConstant");
    ASSERT_TRUE(nullptr != unit);
    ASSERT_EQ(&unit->GetSchema(), schema.get());

    EXPECT_STREQ("Constant", unit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Constant", unit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", unit->GetDefinition().c_str());
    EXPECT_DOUBLE_EQ(10.0, unit->GetNumerator());
    EXPECT_DOUBLE_EQ(1.0, unit->GetDenominator());
    EXPECT_FALSE(unit->HasUnitSystem());

    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchemaXml));
    }
    {
    ECSchemaPtr serializedSchema;
    ECSchemaReadContextPtr serializedContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedSchema, serializedSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(1, serializedSchema->GetUnitCount());
    ECUnitCP serializedECUnit = serializedSchema->GetConstantCP("TestConstant");
    ASSERT_TRUE(nullptr != serializedECUnit);

    ASSERT_TRUE(serializedECUnit->IsConstant());
    EXPECT_STREQ("Constant", serializedECUnit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Constant", serializedECUnit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", serializedECUnit->GetDefinition().c_str());
    EXPECT_DOUBLE_EQ(10.0, serializedECUnit->GetNumerator());
    EXPECT_DOUBLE_EQ(1.0, serializedECUnit->GetDenominator());
    EXPECT_FALSE(serializedECUnit->HasUnitSystem());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantDeserializationTests, RoundTripWithReferencedSchemaForPhenomenonAndUnitSystem)
    {
    Utf8CP refXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="refSchema" version="01.00.00" alias="rs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        </ECSchema>)xml";
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="refSchema" version="01.00.00" alias="rs"/>
            <Constant typeName="TestConstant" phenomenon="rs:TestPhenomenon" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="10.0"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    Utf8String serializedRefSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refXml, *context));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));

    ASSERT_EQ(1, schema->GetUnitCount());
    ECUnitCP unit = schema->GetUnitCP("TestConstant");
    ASSERT_TRUE(nullptr != unit);
    ASSERT_EQ(&unit->GetSchema(), schema.get());

    EXPECT_STREQ("Constant", unit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Constant", unit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", unit->GetDefinition().c_str());
    EXPECT_STREQ("LENGTH*LENGTH", unit->GetPhenomenon()->GetDefinition().c_str());
    EXPECT_EQ(SchemaWriteStatus::Success, schema->WriteToXmlString(serializedSchemaXml));
    EXPECT_EQ(SchemaWriteStatus::Success, refSchema->WriteToXmlString(serializedRefSchemaXml));
    }
    {
    ECSchemaPtr serializedSchema;
    ECSchemaPtr serializedRefSchema;
    ECSchemaReadContextPtr serializedContext = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedRefSchema, serializedRefSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(serializedSchema, serializedSchemaXml.c_str(), *serializedContext));
    ASSERT_EQ(1, serializedSchema->GetUnitCount());
    ECUnitCP serializedUnit = serializedSchema->GetUnitCP("TestConstant");
    ASSERT_TRUE(nullptr != serializedUnit);

    EXPECT_STREQ("Constant", serializedUnit->GetInvariantDisplayLabel().c_str());
    EXPECT_STREQ("This is an awesome new Constant", serializedUnit->GetInvariantDescription().c_str());
    EXPECT_STREQ("M", serializedUnit->GetDefinition().c_str());
    EXPECT_STREQ("LENGTH*LENGTH", serializedUnit->GetPhenomenon()->GetDefinition().c_str());
    EXPECT_DOUBLE_EQ(10.0, serializedUnit->GetNumerator());
    EXPECT_DOUBLE_EQ(1.0, serializedUnit->GetDenominator());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantDeserializationTests, ShouldFailWithoutAliasesOrBadAliases)
    {
    Utf8CP refXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="refSchema" version="01.00.00" alias="rs" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        </ECSchema>)xml";
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="refSchema" version="01.00.00" alias="rs"/>
            <Constant typeName="TestConstant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="10.0"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    Utf8String serializedRefSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaPtr refSchema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(refSchema, refXml, *context));
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantDeserializationTests, ShouldFailWithoutPhenomenonAndUnitSystemBeingDefined)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Constant typeName="TestConstant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant"  numerator="10.0"/>
        </ECSchema>)xml";

    Utf8String serializedSchemaXml;
    {
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::InvalidECSchemaXml, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantDeserializationTests, DuplicateConstantNames)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Constant typeName="TestConstant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="10.0"/>
            <Constant typeName="TestConstant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="10.0"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize schema with two constants with same name");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantDeserializationTests, ShouldFailWithInvalidDefinition)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
            <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
            <Constant typeName="TestConstant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="" description="This is an awesome new Constant" numerator="10.0"/>
        </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should Fail to derserialize constatnt with empty defintion");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantDeserializationTests, MissingOrInvalidNumerator)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="Constant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize Constant with missing numerator");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="Constant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="bananas"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize Constant with non numeric numerator");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="Constant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="bananas"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize Constant with empty numerator");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="Constant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="0.0"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize Constant with 0 numerator");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantDeserializationTests, MissingOrInvalidDenominator)
    {
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="Constant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="10.0"/>
    </ECSchema>)xml";

    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_DOUBLE_EQ(1.0, schema->GetConstantCP("Constant")->GetDenominator());
    }

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="Constant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="10.0" denominator="bananas"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialized with invalid denominator");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="Constant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="10.0" denominator="0.0"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialized with 0 denominator");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="Constant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="10.0" denominator=""/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialized with invalid denominator");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantDeserializationTests, MissingOrInvalidName)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="10.0"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize constant with missing name");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="M" description="This is an awesome new Constant" numerator="10.0"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize constant with empty name");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantDeserializationTests, MissingDisplayLabel)
    {
    Utf8CP schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="TestConstant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" definition="M" description="This is an awesome new Constant" numerator="10.0"/>
    </ECSchema>)xml";
    ECSchemaPtr schema;
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, schemaXml, *context));
    ASSERT_TRUE(schema.IsValid());

    ECUnitCP unit = schema->GetUnitCP("TestConstant");
    EXPECT_STREQ("TestConstant", unit->GetInvariantDisplayLabel().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
TEST_F(ConstantDeserializationTests, MissingOrEmptyDefinition)
    {
    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="TestConstant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" description="This is an awesome new Constant" numerator="10.0"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize constant with missing definition");

    ExpectSchemaDeserializationFailure(R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="testSchema" version="01.00.00" alias="ts" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <Phenomenon typeName="TestPhenomenon" displayLabel="Phenomenon" definition="LENGTH*LENGTH" description="This is an awesome new Phenomenon"/>
        <UnitSystem typeName="TestUnitSystem" displayLabel="Unit System" description="This is an awesome new Unit System"/>
        <Constant typeName="TestConstant" phenomenon="TestPhenomenon" unitSystem="TestUnitSystem" displayLabel="Constant" definition="" description="This is an awesome new Constant" numerator="10.0"/>
    </ECSchema>)xml", SchemaReadStatus::InvalidECSchemaXml, "Should fail to deserialize constant with empty defintion");
    }

END_BENTLEY_ECN_TEST_NAMESPACE
