/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/Published/UnitConversionTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../TestFixture/UnitsTestFixture.h"

#include <sstream>
#include <Bentley/BeNumerical.h>

USING_NAMESPACE_BENTLEY_UNITS

BEGIN_UNITS_UNITTESTS_NAMESPACE

struct UnitConversionTests : UnitsTestFixture
{
    static bool TestUnitConversion(double fromVal, Utf8CP fromUnitName, double expectedVal, Utf8CP targetUnitName, int ulp,
                                    bvector<Utf8String>& loadErrors, bvector<Utf8String>& conversionErrors, bvector<bpair<Utf8String, Utf8String>>& handledUnits,
                                    bool useLegacyNames = false, bool showDetailLogs = false, UnitsProblemCode expectCode = UnitsProblemCode::NoProblem);
    static void TestConversionsLoadedFromCvsFile(Utf8CP fileName, WCharCP outputFileName, int expectedMissingUnits);

    static double GetDouble(Utf8StringR doubleVal)
        {
        std::istringstream iss(doubleVal.c_str());
        double convertedVal = 0;
        iss >> convertedVal;
        return convertedVal;
        }

    static int GetInt(Utf8CP intValue)
        {
        int value = 0;
        BE_STRING_UTILITIES_UTF8_SSCANF(intValue, "%d", &value);
        return value;
        }
};

static Utf8CP UnitsProblemCodeToString(UnitsProblemCode code)
    {
    return UnitsProblemCode::NoProblem == code ? "NoProblem" :
        UnitsProblemCode::InvalidUnitName == code ? "InvalidUnitName" : UnitsProblemCode::UncomparableUnits == code ? "UncomparableUnits" : "InvertingZero";
    }

/*---------------------------------------------------------------------------------**//**
// @bsiclass                                     Basanta.Kharel                 12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitConversionTests::TestUnitConversion (double fromVal, Utf8CP fromUnitName, double expectedVal, Utf8CP targetUnitName, int ulp,
                                     bvector<Utf8String>& missingUnits, bvector<Utf8String>& conversionErrors, bvector<bpair<Utf8String,Utf8String>>& handledUnits,
                                     bool useLegacyNames, bool showDetailLogs, UnitsProblemCode expectedCode)
    {
    //if either units are not in the library conversion is not possible
    //UnitsMapping test checks if all units are there and fails when a unit is not found
    UnitCP fromUnit = LocateUOM(fromUnitName, useLegacyNames);
    UnitCP targetUnit = LocateUOM(targetUnitName, useLegacyNames);
    if (nullptr == fromUnit || nullptr == targetUnit)
        {
        if (nullptr == fromUnit)
            {
            if (missingUnits.end() == find(missingUnits.begin(), missingUnits.end(), fromUnitName))
                missingUnits.push_back(fromUnitName);
            }
        if (nullptr == targetUnit)
            {
            if (missingUnits.end() == find(missingUnits.begin(), missingUnits.end(), targetUnitName))
                missingUnits.push_back(targetUnitName);
            }
        return false;
        }

    // Bad
    if (showDetailLogs)
        {
        NativeLogging::LoggingConfig::SetSeverity("UnitsNative", NativeLogging::SEVERITY::LOG_TRACE);
        NativeLogging::LoggingConfig::SetSeverity("Performance", NativeLogging::SEVERITY::LOG_TRACE);
        }

    PERFORMANCELOG.debugv("About to try to convert %.16g %s to %s", fromVal, fromUnit->GetName().c_str(), targetUnit->GetName().c_str());

    double convertedVal;
    UnitsProblemCode actualCode = fromUnit->Convert(convertedVal, fromVal, targetUnit);
    if (UnitsProblemCode::NoProblem != actualCode)
        {
        PERFORMANCELOG.debugv("Failed to convert from %s to %s because %s", fromUnit->GetName().c_str(), targetUnit->GetName().c_str(), UnitsProblemCodeToString(actualCode));
        }

    if (expectedCode != actualCode)
        {
        Utf8PrintfString formattedText("Conversion from %s (%s) to %s (%s) returned unexpected code.  Expected: %s  Actual:  %s.", 
                                       fromUnitName, fromUnit->GetName().c_str(), targetUnitName, targetUnit->GetName().c_str(),
                                       UnitsProblemCodeToString(expectedCode), UnitsProblemCodeToString(actualCode));
        EXPECT_EQ(expectedCode, actualCode) << formattedText;
        conversionErrors.push_back(formattedText);
        }

    PERFORMANCELOG.debugv("Converted %s to %s.  Expected: %.17g  Actual: %.17g", fromUnit->GetName().c_str(), targetUnit->GetName().c_str(), expectedVal, convertedVal);

    if (!almost_equal<double>(expectedVal, convertedVal, ulp))
        {
        Utf8PrintfString formattedText("Conversion from %s (%s) to %s (%s). Input: %.17g \nOutput:   %.17g \nExpected: %.17g \nDiff:     %.17g   Diff/Exp: %.17g   ULP: %d\n",
                                        fromUnitName, fromUnit->GetName().c_str(), targetUnitName, targetUnit->GetName().c_str(), fromVal, convertedVal, expectedVal, convertedVal - expectedVal, (convertedVal - expectedVal) / expectedVal, ulp);
        conversionErrors.push_back(formattedText);
        EXPECT_FALSE(true) << formattedText;
        }
    EXPECT_FALSE(BeNumerical::BeIsnan(convertedVal) || !BeNumerical::BeFinite(convertedVal)) << "Conversion from " << fromUnitName << " to " << targetUnitName << " resulted in an invalid number";

    handledUnits.push_back(make_bpair(fromUnit->GetName(), targetUnit->GetName()));
    if (showDetailLogs)
        {
        NativeLogging::LoggingConfig::SetSeverity("UnitsNative", NativeLogging::SEVERITY::LOG_ERROR);
        NativeLogging::LoggingConfig::SetSeverity("Performance", NativeLogging::SEVERITY::LOG_ERROR);
        }
    return true;
    }

void UnitConversionTests::TestConversionsLoadedFromCvsFile(Utf8CP fileName, WCharCP outputFileName, int expectedMissingUnits)
    {
    bvector<Utf8String> loadErrors;
    bvector<Utf8String> conversionErrors;
    bvector<bpair<Utf8String, Utf8String>> handledUnits;

    int numberConversions = 0;
    int numberWhereUnitsFound = 0;
    auto lineProcessor = [&loadErrors, &conversionErrors, &numberConversions, &numberWhereUnitsFound, &handledUnits] (bvector<Utf8String>& tokens)
        {
        ++numberConversions;
        //passing 10000 to tolerance instead of the csv value
        if (TestUnitConversion(GetDouble(tokens[2]), tokens[3].c_str(), GetDouble(tokens[0]), tokens[1].c_str(), GetInt(tokens[4].c_str()), loadErrors, conversionErrors, handledUnits, true))
            ++numberWhereUnitsFound;
        };

    WString fileNameLong(fileName, BentleyCharEncoding::Utf8);
    ReadCSVFile(fileNameLong.c_str(), lineProcessor);

    Utf8PrintfString loadErrorString("%s - Attempted %d conversions, error loading :\n", fileName, numberConversions);

    for (auto const& val : loadErrors)
        loadErrorString.append(val + "\n");

    EXPECT_EQ(expectedMissingUnits, loadErrors.size()) << loadErrorString;

    Utf8PrintfString conversionErrorString("%s - Total number of conversions %d, units found for %d, %d passed, %d failed, %d skipped because of %d missing units, error Converting :\n",
                                            fileName, numberConversions, numberWhereUnitsFound, numberWhereUnitsFound - conversionErrors.size(), conversionErrors.size(), numberConversions - numberWhereUnitsFound, loadErrors.size());

    for (auto const& val : conversionErrors)
        conversionErrorString.append(val + "\n");

    EXPECT_EQ(0, conversionErrors.size()) << conversionErrorString;

    Utf8String outputfile = UnitsTestFixture::GetOutputDataPath(outputFileName);
    WriteToFile(outputfile.c_str(), handledUnits);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                                03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitConversionTests, TestTemperatureConversions)
    {
    bvector<Utf8String> loadErrors;
    bvector<Utf8String> conversionErrors;
    bvector<bpair<Utf8String, Utf8String>> handledUnits;

    // Expected values generated using the following equations and windows calculator
    // Celsius = (Fahrenheit–32)×5÷9
    // Kelvin = ((Fahrenheit–32)×5/9)+273.15
    // Rankine = Fahrenheit+459.67
    // Rømer = (Fahrenheit–32)×7/24+7.5
    TestUnitConversion(32, "FAHRENHEIT", 0, "CELSIUS", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(20, "FAHRENHEIT", -6.666666666666666666666666666, "CELSIUS", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(122, "FAHRENHEIT", 50, "CELSIUS", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(60, "FAHRENHEIT", 288.70555555555555555555555555556, "K", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(60, "FAHRENHEIT", 519.67, "RANKINE", 1, loadErrors, conversionErrors, handledUnits);
    //TestUnitConversion(61.1, "FAHRENHEIT", 15.9875, "ROMER", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(0, "FAHRENHEIT", -17.777777777777777777777777777778, "CELSIUS", 1, loadErrors, conversionErrors, handledUnits);
                                          //12345678901234567|234567890123
    TestUnitConversion(0, "FAHRENHEIT", 255.37222222222222222222222222222, "K", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(0, "FAHRENHEIT", 459.67, "RANKINE", 1, loadErrors, conversionErrors, handledUnits);
    //TestUnitConversion(0, "FAHRENHEIT", -1.8333333333333333333333333333333, "ROMER", 10, loadErrors, conversionErrors, handledUnits);

    //The usable precision of IEEE - 754 double is around 16 decimal digits - so excluding educational purposes, 
    //representations longer than that are just a waste of resources and computing power :
    //Users are not getting more informed when they see a 700 - digit - number on the screeen.
    //Configuration variables stored in that "more accurate" form are useless



    // Expected values generated using the following equations and windows calculator
    // Fahrenheit = Celsius×9÷5+32
    // Kelvin = Celsius+273.15
    // Rankine = Celsius×9/5+32+459.67
    // Rømer = Celsius×21/40+7.5
    TestUnitConversion(1, "CELSIUS", 33.8, "FAHRENHEIT", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(-15, "CELSIUS", 5, "FAHRENHEIT", 10, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(-25, "CELSIUS", -13, "FAHRENHEIT", 10, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(60, "CELSIUS", 140, "FAHRENHEIT", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(60, "CELSIUS", 333.15, "K", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(60, "CELSIUS", 599.67, "RANKINE", 1, loadErrors, conversionErrors, handledUnits);
    //TestUnitConversion(-14.3, "CELSIUS", -0.0075, "ROMER", 100, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(0, "CELSIUS", 32, "FAHRENHEIT", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(0, "CELSIUS", 273.15, "K", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(0, "CELSIUS", 491.67, "RANKINE", 1, loadErrors, conversionErrors, handledUnits);
    //TestUnitConversion(0, "CELSIUS", 7.5, "ROMER", 1, loadErrors, conversionErrors, handledUnits);

    TestUnitConversion(42, "K", -231.15, "CELSIUS", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42, "K", -384.07, "FAHRENHEIT", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42, "K", 75.6, "RANKINE", 1, loadErrors, conversionErrors, handledUnits);
    //TestUnitConversion(571.2, "K", 163.97625, "ROMER", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(0, "K", -273.15, "CELSIUS", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(0, "K", -459.67, "FAHRENHEIT", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(0, "K", 0, "RANKINE", 1, loadErrors, conversionErrors, handledUnits);
    //TestUnitConversion(0, "K", -135.90375, "ROMER", 1, loadErrors, conversionErrors, handledUnits);


    // Expected values generated using the following equations and windows calculator
    // Celsius = (Rankine–459.67–32)×5/9
    // Fahrenheit = Rankine–459.67
    // Kelvin = (Rankine–459.67–32)×5/9+273.15
    // Rømer = (Rankine–491.67)×7/24+7.5
    TestUnitConversion(42, "RANKINE", -249.81666666666666666666666666667, "CELSIUS", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42, "RANKINE", -417.67, "FAHRENHEIT", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42, "RANKINE", 23.333333333333333, "K", 1, loadErrors, conversionErrors, handledUnits);
    //TestUnitConversion(630, "RANKINE", 47.84625, "ROMER", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(0, "RANKINE", -273.15, "CELSIUS", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(0, "RANKINE", -459.67, "FAHRENHEIT", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(0, "RANKINE", 0, "K", 1, loadErrors, conversionErrors, handledUnits);
    //TestUnitConversion(0, "RANKINE", -135.90375, "ROMER", 1, loadErrors, conversionErrors, handledUnits);
    
    // Expected values generated using the following equations and windows calculator
    // Celsius = (Rømer–7.5)×40/21
    // Fahrenheit = (Rømer–7.5)×24/7+32
    // Kelvin = (Rømer–7.5)×40/21+273.15
    // Rankine = (Rømer–7.5)×24/7+491.67
    //TestUnitConversion(42, "ROMER", 65.714285714285714285714285714286, "CELSIUS", 1, loadErrors, conversionErrors, handledUnits);
    //TestUnitConversion(42, "ROMER", 150.28571428571428571428571428571, "FAHRENHEIT", 1, loadErrors, conversionErrors, handledUnits);
    //TestUnitConversion(42, "ROMER", 338.86428571428571428571428571429, "K", 1, loadErrors, conversionErrors, handledUnits);
    //TestUnitConversion(42, "ROMER", 609.95571428571428571428571428571, "RANKINE", 1, loadErrors, conversionErrors, handledUnits);
    //TestUnitConversion(0, "ROMER", -14.285714285714285714285714285714, "CELSIUS", 1, loadErrors, conversionErrors, handledUnits);
    //TestUnitConversion(0, "ROMER", 6.285714285714285714, "FAHRENHEIT", 1, loadErrors, conversionErrors, handledUnits);
    //TestUnitConversion(0, "ROMER", 258.8642857142857142, "K", 1, loadErrors, conversionErrors, handledUnits);
    //TestUnitConversion(0, "ROMER", 465.9557142857142857, "RANKINE", 1, loadErrors, conversionErrors, handledUnits);


    Utf8String loadErrorString("The following units were not found:\n");
    for (auto const& val : loadErrors)
        loadErrorString.append(val + "\n");

    Utf8String conversionErrorString("Failed to convert between the following units:\n");
    for (auto const& val : conversionErrors)
        conversionErrorString.append(val + "\n");

    EXPECT_EQ(0, loadErrors.size()) << loadErrorString;
    EXPECT_EQ(0, conversionErrors.size()) << conversionErrorString;
    Utf8String fileName = UnitsTestFixture::GetOutputDataPath(L"TestOffsetConversions_handledUnits.csv");
    WriteToFile(fileName.c_str(), handledUnits);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                                  03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitConversionTests, TestBasicConversion)
    {
    bvector<Utf8String> loadErrors;
    bvector<Utf8String> conversionErrors;
    bvector<bpair<Utf8String, Utf8String>> handledUnits;
    TestUnitConversion(10, "FT", 3048, "MM", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1, "GALLON", 3.785411784, "LITRE", 1000, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(100000, "FOOT_SQUARED", 100, "THOUSAND_FOOT_SQUARED", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(836127.36, "MILLIMETRE_SQUARED", 1.0, "YARD_SQUARED", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(3.17097919837647e-7, "YEAR", 10000.0, "MILLISECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1000000.0, "POUND", 1000000.0, "POUND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(2204622621.84878, "POUND", 1000000.0, "MEGAGRAM", 100000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.66666666666667e-02, "DEGREE", 1.0, "ANGLE_MINUTE", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.852e11, "CENTIMETRE_PER_HOUR", 1.0e6, "KNOT_INTERNATIONAL", 1000000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.65409011373578e-3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e6, "LITRE_PER_KILOMETRE_SQUARED_PER_DAY", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(2.816538995808e13, "GALLON_PER_DAY_PER_PERSON", 1234e6, "LITRE_PER_SECOND_PER_PERSON", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(4.4482216152605e5, "DYNE", 1.0, "POUND_FORCE", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(2.8316846592e3, "KILONEWTON_PER_FOOT_CUBED", 1.0e8, "NEWTON_PER_METRE_CUBED", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e9, "NEWTON_PER_METRE", 1.0e6, "NEWTON_PER_MILLIMETRE", 10000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(3.43774677078493e9, "DEGREE_PER_HOUR", 1.0e6, "RADIAN_PER_MINUTE", 100000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(2.65258238486492e3, "CYCLE_PER_SECOND", 1.0e6, "RADIAN_PER_MINUTE", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(8.92179121619709e5, "POUND_PER_ACRE", 1.0e6, "KILOGRAM_PER_HECTARE", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(8.92179121619701e6, "POUND_PER_ACRE", 1.0e6, "GRAM_PER_METRE_SQUARED", 10000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(8.54292974552351e7, "FOOT_POUNDAL", 1.0, "KILOWATT_HOUR", 10000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(2.37303604042319e7, "FOOT_POUNDAL", 1.0, "MEGAJOULE", 10000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(42, "KG/SEC", 42000.0, "G/SEC", 1000, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42, "CUB.M/SEC", 2.562997252e6, "CUB.IN/SEC", 100000, loadErrors, conversionErrors, handledUnits); // Exptected value has 10 digits of precision generated using http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    TestUnitConversion(2.326e6, "KILOJOULE_PER_KILOGRAM", 1.0e6, "BTU_PER_POUND_MASS", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(60, "GRAM_PER_MINUTE", 1.0, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(3.53146667214886e1, "KILONEWTON_PER_METRE_CUBED", 1.0, "KILONEWTON_PER_FOOT_CUBED", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(42.42, "KILOPASCAL_GAUGE", 6.15250086300203, "POUND_FORCE_PER_INCH_SQUARED_GAUGE", 100000000, loadErrors, conversionErrors, handledUnits, true); // Expected value from old system, difference is due to imprecise offset in old system.
    TestUnitConversion(42.42, "PERCENT_SLOPE", 0.4242, "METRE_PER_METRE", 1, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(0.42, "METRE_PER_METRE", 42.0, "PERCENT_SLOPE", 1, loadErrors, conversionErrors, handledUnits, true);
    ASSERT_EQ(3, loadErrors.size()) << BeStringUtilities::Join(loadErrors, ", "); // 3 known missing units : LITRE_PER_KILOMETRE_SQUARED_PER_DAY, GALLON_PER_DAY_PER_PERSON, LITRE_PER_SECOND_PER_PERSON
    ASSERT_EQ(0, conversionErrors.size()) << BeStringUtilities::Join(conversionErrors, ", ");
    Utf8String fileName = UnitsTestFixture::GetOutputDataPath(L"TestBasicConversion_handledUnits.csv");
    WriteToFile(fileName.c_str(), handledUnits);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitConversionTests, TestInvertedSlopeUnits)
    {
    bvector<Utf8String> loadErrors;
    bvector<Utf8String> conversionErrors;
    bvector<bpair<Utf8String, Utf8String>> handledUnits;
    TestUnitConversion(42.42, "HORIZONTAL/VERTICAL", 1.0 / 42.42, "VERTICAL/HORIZONTAL", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(0.0, "HORIZONTAL/VERTICAL", 0.0, "VERTICAL/HORIZONTAL", 1, loadErrors, conversionErrors, handledUnits, false, false, UnitsProblemCode::InvertingZero);
    ASSERT_EQ(0, loadErrors.size()) << BeStringUtilities::Join(loadErrors, ", ");
    ASSERT_EQ(0, conversionErrors.size()) << BeStringUtilities::Join(conversionErrors, ", ");
    Utf8String fileName = UnitsTestFixture::GetOutputDataPath(L"TestInvertedSlopeUnits_handledUnits.csv");
    WriteToFile(fileName.c_str(), handledUnits);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                                 03/16
+---------------+---------------+---------------+---------------+---------------+------*/
// NOTE: Disabled because we do not have active customers
//TEST_F(UnitsTests, TestLinearCostConversions)
//    {
//    bvector<Utf8String> loadErrors;
//    bvector<Utf8String> conversionErrors;
//    bvector<bpair<Utf8String, Utf8String>> handledUnits;
//    TestUnitConversion(4200.42, "US$/M", 4.20042, "US$/MM", 1, loadErrors, conversionErrors, handledUnits);
//    TestUnitConversion(4200.42, "US$/M", 4200.42, "US$/M", 1, loadErrors, conversionErrors, handledUnits);
//    TestUnitConversion(0.0, "US$/M", 0.0, "US$/MM", 1, loadErrors, conversionErrors, handledUnits);
//    TestUnitConversion(4200.42, "US$/MM", 4200420.0, "US$/M", 1, loadErrors, conversionErrors, handledUnits);
//    TestUnitConversion(4200.42, "US$/MM", 4200.42, "US$/MM", 1, loadErrors, conversionErrors, handledUnits);
//    TestUnitConversion(0.0, "US$/MM", 0.0, "US$/M", 1, loadErrors, conversionErrors, handledUnits);
//    ASSERT_EQ(0, loadErrors.size()) << BeStringUtilities::Join(loadErrors, ", ");
//    ASSERT_EQ(0, conversionErrors.size()) << BeStringUtilities::Join(conversionErrors, ", ");
//    Utf8String fileName = UnitsTestFixture::GetOutputDataPath(L"TestLinearCostConversions_handledUnits.csv");
//    WriteToFile(fileName.c_str(), handledUnits);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Robert.Schili                              03/16
+---------------+---------------+---------------+---------------+---------------+------*/
//This test is an open pot to add conversion tests which don't deserve their own method
TEST_F(UnitConversionTests, TestMiscConversions)
    {
    bvector<Utf8String> loadErrors;
    bvector<Utf8String> conversionErrors;
    bvector<bpair<Utf8String, Utf8String>> handledUnits;

    //Frequency
    TestUnitConversion(4.2, "HZ", 4.2e-3, "KHZ", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(4.2, "HZ", 4.2e-6, "MHZ", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(9, "MHZ", 9000, "KHZ", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(9, "KHZ", 9000, "HZ", 1, loadErrors, conversionErrors, handledUnits);

    //MASS
    TestUnitConversion(5, "TONNE", 5000, "KG", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(5000, "KG", 5, "TONNE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "TONNE", 42.42e15, "NG", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "TONNE", 42.42e3 / 0.45359237, "LBM", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "NG", 42.42e-12 / 0.45359237, "LBM", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "MKG", 42.42e-12 / 0.45359237, "KIPM", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "SLUG", (42.42 * 1000 * 0.45359237 * 9.80665) / 0.3048, "G", 1, loadErrors, conversionErrors, handledUnits); // 0.45359237 is conversion between LBM and KG, 9.80665 is std g
    TestUnitConversion(42.43, "SLUG", 6.1921930163e5, "G", 100000, loadErrors, conversionErrors, handledUnits); // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    TestUnitConversion(42.42, "KIPM", (42.42 * 1000 * 0.3048) / 9.80665, "SLUG", 1, loadErrors, conversionErrors, handledUnits);

    //DYNAMIC_VISCOSITY
    TestUnitConversion(4200.0, "POISE", 420.0, "PA-S", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "CENTIPOISE", 42.42e-2, "POISE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "PA-S", 42.42e3, "CENTIPOISE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "LBM/(FT*S)", (42.42 * 0.45359237) / 0.3048, "PA-S", 1, loadErrors, conversionErrors, handledUnits); // 0.45359237 is conversion between LBM and KG
    TestUnitConversion(42.43, "LBM/(FT*S)", 63.142796126, "PA-S", 100000, loadErrors, conversionErrors, handledUnits); // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    TestUnitConversion(63.142796126, "PA-S", 42.43, "LBM/(FT*S)", 100000, loadErrors, conversionErrors, handledUnits); // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html

    //FORCE
    TestUnitConversion(1000.0, "PDL", 138.254954376, "N", 10, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "PDL", 6.5922695314e-4, "SHORT_TON_FORCE", 10000, loadErrors, conversionErrors, handledUnits); // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    TestUnitConversion(42.42, "PDL", 5.8859549387e-4,"LONG_TON_FORCE", 100000, loadErrors, conversionErrors, handledUnits);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    TestUnitConversion(42.42, "PDL", 42.42 / 32.174048556430442 / 2000, "SHORT_TON_FORCE", 1, loadErrors, conversionErrors, handledUnits); // 32.174048556430442 is 9.80665 converted to ft/s^2 using out system, 2000 is the number of pounds in a short ton
    TestUnitConversion(42.42, "PDL", 42.42 / 32.174048556430442 / 2240, "LONG_TON_FORCE", 1, loadErrors, conversionErrors, handledUnits); // 32.174048556430442 is 9.80665 converted to ft/s^2 using out system, 2240 is the number of pounds in a long ton
    TestUnitConversion(42.42, "LBF", 42.42 * 32.174048556430442, "PDL", 1, loadErrors, conversionErrors, handledUnits); // 32.174048556430442 is 9.80665 converted to ft/s^2 using out system
    TestUnitConversion(42.43, "LBF", 42.43 * 32.174048556, "PDL", 100000, loadErrors, conversionErrors, handledUnits); //32.174048556 is 9.80665 converted to ft/s^2 using http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html

    //LENGTH
    //LIGHT_XXX units dropped from framework as they are not needed
    /*TestUnitConversion(0.0042, "LIGHT_YEAR", 2.469023e10, "MILE", 10, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(0.003, "LIGHT_HOUR", 3237758.5, "KM", 10, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(0.013, "LIGHT_MIN", 2.55728e8, "YARD", 10, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "LIGHT_SEC", 186282, "MILE", 10, loadErrors, conversionErrors, handledUnits);*/
    TestUnitConversion(42.42, "NAUT_MILE", 42.42 * 1852.0, "M", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "KM", 42.42 * 1000.0 / 1852.0, "NAUT_MILE", 1, loadErrors, conversionErrors, handledUnits);

    //MOLE
    TestUnitConversion(0.3, "MOL", 0.0003, "KMOL", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "KMOL", 42420.0, "MOL", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "LB-MOL", 42.42 * 453.59237, "MOL", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "KMOL", 42.42e3 / 453.59237, "LB-MOL", 1, loadErrors, conversionErrors, handledUnits);

    //ACCELERATION
    TestUnitConversion(42.42, "M/SEC.SQ", 4242.0, "CM/SEC.SQ", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "M/SEC.SQ", 42.42 / 0.3048, "FT/SEC.SQ", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "M/SEC.SQ", 42.42, "M/SEC.SQ", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "FT/SEC.SQ", 42.42 * 0.3048, "M/SEC.SQ", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "FT/SEC.SQ", 42.42 * 30.48, "CM/SEC.SQ", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "FT/SEC.SQ", 42.42 , "FT/SEC.SQ", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "CM/SEC.SQ", 0.4242, "M/SEC.SQ", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "CM/SEC.SQ", 42.42 / 30.48, "FT/SEC.SQ", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "CM/SEC.SQ", 42.42, "CM/SEC.SQ", 1, loadErrors, conversionErrors, handledUnits);

    TestUnitConversion(1.0, "STD_G", 9.80665, "M/SEC.SQ", 1, loadErrors, conversionErrors, handledUnits); // Exact constant value for standard gravity
    TestUnitConversion(1.0, "STD_G", 32.174048556, "FT/SEC.SQ", 100000, loadErrors, conversionErrors, handledUnits); // Documented value for standard gravity in m/s^2 converted to ft/s^2 using http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    TestUnitConversion(1.0, "STD_G", 32.174048556430446194225721784777, "FT/SEC.SQ", 1, loadErrors, conversionErrors, handledUnits); // Expected value is the result of 9.80665 / 0.3048 done on windows calculator

    //AREA
    TestUnitConversion(42.42, "SQ.MU", 42.42e-6, "SQ.MM", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "SQ.MU", 42.42e-10, "SQ.DM", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "SQ.MU", 42.42e-12 / 0.3048 / 0.3048, "SQ.FT", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "SQ.DM", 42.42e4, "SQ.MM", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "SQ.DM", 42.42e10, "SQ.MU", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "SQ.DM", 42.42e-2 / 0.3048 / 0.3048, "SQ.FT", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.43, "SQ.DM", 4.5671271898, "SQ.FT", 100000, loadErrors, conversionErrors, handledUnits);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    TestUnitConversion(42.42, "ARE", 42.42e-2, "HECTARE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "HECTARE", 4242.0, "ARE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "ARE", (42.42 * 10) / (66 * 66 * 0.3048 * 0.3048), "ACRE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.43, "ARE", 1.0484681336, "ACRE", 100000, loadErrors, conversionErrors, handledUnits);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html

    //ELECTRIC CURRENT
    TestUnitConversion(42.42, "A", 42.42e-3, "KILOAMPERE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "A", 42.42e3, "MILLIAMPERE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "A", 42.42e6, "MICROAMPERE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "KILOAMPERE", 42.42e9, "MICROAMPERE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "MILLIAMPERE", 42.42e3, "MICROAMPERE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42e3, "MICROAMPERE", 42.42, "MILLIAMPERE", 1, loadErrors, conversionErrors, handledUnits);

    //Volume Flow Rate
    TestUnitConversion(42.42, "CUB.IN/MIN", 42.42 / 60.0, "CUB.IN/SEC", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "CUB.IN/SEC", 42.42 * 60.0, "CUB.IN/MIN", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "CUB.IN/MIN", 42.42 / pow(12.0, 3) , "CUB.FT/MIN", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "ACRE_IN/DAY", 42.42 / 12.0, "ACRE_FT/DAY", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "ACRE_IN/DAY", (42.42 * 43560.0 * 144.0) / (24.0 * 60.0), "CUB.IN/MIN", 1, loadErrors, conversionErrors, handledUnits); // 43560 is number of sq ft in an acre
    TestUnitConversion(42.43, "ACRE_IN/DAY", 1.8482508e5, "CUB.IN/MIN", 1, loadErrors, conversionErrors, handledUnits);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    TestUnitConversion(42.42, "CUB.M/SEC", (42.42 * 60.0 * pow(12, 3)) / pow(0.3048, 3) , "CUB.IN/MIN", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.43, "CUB.M/SEC", 1.5535424772e8, "CUB.IN/MIN", 100000, loadErrors, conversionErrors, handledUnits);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    TestUnitConversion(42.42, "CUB.M/MIN", (42.42 * 24.0 * 60.0 * 12.0) / (pow(0.3048, 3) * 43560.0), "ACRE_IN/DAY", 1, loadErrors, conversionErrors, handledUnits); // 43560 is number of sq ft in an acre
    TestUnitConversion(42.43, "CUB.M/MIN", 594.40713084, "ACRE_IN/DAY", 10000, loadErrors, conversionErrors, handledUnits);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html

    //Force Density
    TestUnitConversion(42.42, "N/CUB.FT", 42.42 / pow(0.3048, 3), "N/CUB.M", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "KN/CUB.FT", 42.42e3, "N/CUB.FT", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "N/CUB.M", 42.42 * pow(0.3048, 3), "N/CUB.FT", 1, loadErrors, conversionErrors, handledUnits);

    // Heating Value
    TestUnitConversion(42.42, "J/KG", 42.42e-3, "KJ/KG", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "MEGAJ/KG", 42.42e6, "J/KG", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "J/KG", 42.42 * 0.45359237 / 1.05505585262e3, "BTU/LBM", 1, loadErrors, conversionErrors, handledUnits); // 1.05505585262e3 is J/BTU conversion, 0.45359237 is KG/LBM conversion

    //Molar Concentration
    TestUnitConversion(42.42, "MOL/CUB.DM", 42.42e6, "MICROMOL/CUB.DM", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "MOL/CUB.DM", 42.42e9, "NMOL/CUB.DM", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "MICROMOL/CUB.DM", 42.42e6, "PICOMOL/CUB.DM", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "PICOMOL/CUB.DM", 42.42e-12, "MOL/CUB.DM", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "MOL/CUB.FT", 42.42 / (pow(0.3048, 3) * 1000), "MOL/CUB.DM", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "NMOL/CUB.DM", 42.42e-6 * pow(0.3048, 3), "MOL/CUB.FT", 2, loadErrors, conversionErrors, handledUnits);

    //Pressure
    TestUnitConversion(42.42, "PA_GAUGE", 42.42 + 101325.0, "PA", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "PA", 42.42 - 101325.0, "PA_GAUGE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "KILOPASCAL_GAUGE", 42.42e3, "PA_GAUGE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "KILOPASCAL_GAUGE", 42.42 + 101.325, "KILOPASCAL", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "KILOPASCAL", 42.42 - 101.325, "KILOPASCAL_GAUGE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "KSI", 42.42e3, "PSI", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "PSI", 42.42e-3, "KSI", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "TORR", 42.42 * 101325.0 / 760.0, "PA", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.43, "TORR", 5656.8680921, "PA", 10000, loadErrors, conversionErrors, handledUnits);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    TestUnitConversion(42.43, "PSI", 2194.264589, "TORR", 10000, loadErrors, conversionErrors, handledUnits);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    TestUnitConversion(42.43, "TORR", 0.82045935072, "PSI", 10000, loadErrors, conversionErrors, handledUnits);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    TestUnitConversion(42.42, "ATM", 42.42 * 760.0, "TORR", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.43, "ATM", 32246.8, "TORR", 1, loadErrors, conversionErrors, handledUnits);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    TestUnitConversion(42.42, "TORR", 42.42 / 760.0, "ATM", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.43, "TORR", 0.055828947368, "ATM", 100000, loadErrors, conversionErrors, handledUnits);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    TestUnitConversion(42.43, "ATM", 623.54910655, "PSI", 100000, loadErrors, conversionErrors, handledUnits);  // conversion from http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html

    // Surface Flow Rate
    TestUnitConversion(42.42, "CUB.M/(SQ.M*SEC)", 42.42 * 60.0 * 60.0 * 24.0, "CUB.M/(SQ.M*DAY)", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "CUB.FT/(SQ.FT*SEC)", 42.42 * 0.3048, "CUB.M/(SQ.M*SEC)", 1, loadErrors, conversionErrors, handledUnits);

    //Time
    TestUnitConversion(1.0, "WEEK", 7.0, "DAY", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(14.0, "DAY", 2.0, "WEEK", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "YR", 3.1536e7, "S", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "YEAR_SIDEREAL", 3.155815e7, "S", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "YEAR_TROPICAL", 3.155693e7, "S", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(3.155693e13, "MKS", 1.0, "YEAR_TROPICAL", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(3.155815e10, "MS", 1.0, "YEAR_SIDEREAL", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "YEAR_TROPICAL",3.155693e13 , "MKS", 10, loadErrors, conversionErrors, handledUnits);

    // Torque
    TestUnitConversion(42.42, "N_CM", 0.4242, "N_M", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "LBF_FT", 42.42 * 9.80665 * 0.45359237 * 0.3048 * 100, "N_CM", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.43, "LBF_FT", 5752.7355548, "N_CM", 100000, loadErrors, conversionErrors, handledUnits);

    // Velocity
    TestUnitConversion(42.42, "MM/SEC", 42.42e-3, "M/SEC", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "KM/SEC", 42.42e6, "MM/SEC", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "YRD/SEC", 42.42 * 0.3048 * 3.0, "M/SEC", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42e6, "MM/SEC", 42.42, "KM/SEC", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "FT/MIN", 42.42 / (60.0 * 3.0), "YRD/SEC", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.43, "FT/MIN", 0.23572222222, "YRD/SEC", 100000, loadErrors, conversionErrors, handledUnits);

    //Volume
    TestUnitConversion(42.42, "CUB.MU", 42.42e-9, "CUB.MM", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "CUB.DM", 42.42e15, "CUB.MU", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "CUB.MM", 42.42e-18, "CUB.KM", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "CUB.KM", 42.42e18, "MICROLITRE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "MICROLITRE", 42.42e-6, "CUB.DM", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.42, "CUB.MILE", 42.42 * pow(5280.0 * 0.3048, 3) / 1.0e9, "CUB.KM", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(42.43, "CUB.MILE", 176.85595485, "CUB.KM", 100000, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(176.85595485, "CUB.KM", 42.43, "CUB.MILE", 100000, loadErrors, conversionErrors, handledUnits);

    //The following are not real conversions. These units are all alone in their phenomena, so we just test their 1:1 conversion to touch them once
    TestUnitConversion(1.0, "COULOMB", 1.0, "COULOMB", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "W/SQ.M", 1.0, "W/SQ.M", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "CD", 1.0, "CD", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "STERAD", 1.0, "STERAD", 1, loadErrors, conversionErrors, handledUnits);

    // Specific heat capacity molar
    TestUnitConversion(1.0, "J/(MOL*K)", 1000.0, "J/(KMOL*K)", 1, loadErrors, conversionErrors, handledUnits);

    ASSERT_EQ(0, loadErrors.size()) << BeStringUtilities::Join(loadErrors, ", ");
    ASSERT_EQ(0, conversionErrors.size()) << BeStringUtilities::Join(conversionErrors, ", ");
    Utf8String fileName = UnitsTestFixture::GetOutputDataPath(L"TestMiscConversions_handledUnits.csv");
    WriteToFile(fileName.c_str(), handledUnits);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitConversionTests, USCustomaryLengths)
    {
    bvector<Utf8String> loadErrors;
    bvector<Utf8String> conversionErrors;
    bvector<bpair<Utf8String, Utf8String>> handledUnits;
    // Conversion tests where expected value is taken directly out of  http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-8
    // Directly from exact values in tables
    TestUnitConversion(1.0, "MILE", 63360, "IN", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "MILE", 5280, "FT", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "MILE", 1760, "YRD", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "MILE", 80, "CHAIN", 1, loadErrors, conversionErrors, handledUnits);

    TestUnitConversion(1.0, "IN", 2.54, "CM", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "FT", 30.48, "CM", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "YRD", 91.44, "CM", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "CHAIN", 66.0 * 30.48, "CM", 1, loadErrors, conversionErrors, handledUnits); // Expected value derived from two table entries
    TestUnitConversion(1.0, "MILE", 160934.4, "CM", 1, loadErrors, conversionErrors, handledUnits);

    ASSERT_EQ(0, loadErrors.size()) << BeStringUtilities::Join(loadErrors, ", ");
    ASSERT_EQ(0, conversionErrors.size()) << BeStringUtilities::Join(conversionErrors, ", ");
    Utf8String fileName = UnitsTestFixture::GetOutputDataPath(L"TestUsCustomaryLengths_handledUnits.csv");
    WriteToFile(fileName.c_str(), handledUnits);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                                 03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitConversionTests, UsSurveyLengths)
    {
    bvector<Utf8String> loadErrors;
    bvector<Utf8String> conversionErrors;
    bvector<bpair<Utf8String, Utf8String>> handledUnits;
    // Conversion tests where expected value is taken directly out of  http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-8
    // Exact values from document used for these conversions
    TestUnitConversion(1.0, "FT", 0.999998, "US_SURVEY_FT", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "FT", 0.0254 * 39.37, "US_SURVEY_FT", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "US_SURVEY_FT", 1.0 / 0.999998, "FT", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "US_SURVEY_FT", 1200.0 / 3937.0, "M", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "M", 3937.0 / 1200.0, "US_SURVEY_FT", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "US_SURVEY_MILE", 5280.0 * 1200.0 / 3937.0, "M", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "US_SURVEY_MILE", 1.0 / 0.999998, "MILE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "MILE", 0.999998, "US_SURVEY_MILE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "M", 3937.0 / 1200.0 / 5280.0, "US_SURVEY_MILE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "US_SURVEY_CHAIN", 66, "US_SURVEY_FT", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "US_SURVEY_FT", 1.0 / 66.0, "US_SURVEY_CHAIN", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "M", 39.37, "US_SURVEY_IN", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(12.0, "US_SURVEY_IN", 1200.0 / 3937.0, "M", 1, loadErrors, conversionErrors, handledUnits);

    // Directly from exact values in tables
    TestUnitConversion(1.0, "US_SURVEY_MILE", 63360, "US_SURVEY_IN", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "US_SURVEY_MILE", 5280, "US_SURVEY_FT", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "US_SURVEY_MILE", 1760, "US_SURVEY_YRD", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "US_SURVEY_MILE", 80, "US_SURVEY_CHAIN", 1, loadErrors, conversionErrors, handledUnits);

    // Exact values do not exist in document
    TestUnitConversion(1.0, "US_SURVEY_FT", 0.3048006, "M", 100000000, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "US_SURVEY_CHAIN", 20.11684, "M", 100000000, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "US_SURVEY_YRD", 3.0 * 0.3048006, "M", 100000000, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "US_SURVEY_MILE", 1609.347, "M", 1000000000, loadErrors, conversionErrors, handledUnits);

    ASSERT_EQ(0, loadErrors.size()) << BeStringUtilities::Join(loadErrors, ", ");
    ASSERT_EQ(0, conversionErrors.size()) << BeStringUtilities::Join(conversionErrors, ", ");
    Utf8String fileName = UnitsTestFixture::GetOutputDataPath(L"TestUsSurveyLengths_handledUnits.csv");
    WriteToFile(fileName.c_str(), handledUnits);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                                  03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitConversionTests, USCustomaryAreas)
    {
    bvector<Utf8String> loadErrors;
    bvector<Utf8String> conversionErrors;
    bvector<bpair<Utf8String, Utf8String>> handledUnits;
    // Conversion tests where expected value is taken directly out of  http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-8
    // Directly from exact values in tables
    TestUnitConversion(1.0, "SQ.MILE", 4014489600, "SQ.IN", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.MILE", 27878400, "SQ.FT", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.MILE", 3097600, "SQ.YRD", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.MILE", 6400, "SQ.CHAIN", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.MILE", 640, "ACRE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.CHAIN", 0.1, "ACRE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "ACRE", 43560, "SQ.FT", 1, loadErrors, conversionErrors, handledUnits);

    TestUnitConversion(1.0, "SQ.IN", 0.00064516, "SQ.M", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.FT", 0.09290304, "SQ.M", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.YRD", 0.83612736, "SQ.M", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.CHAIN", 0.09290304 * 4356, "SQ.M", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "ACRE", 0.09290304 * 43560, "SQ.M", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.MILE", 2589988.110336, "SQ.M", 1, loadErrors, conversionErrors, handledUnits);
    ASSERT_EQ(0, loadErrors.size()) << BeStringUtilities::Join(loadErrors, ", ");
    ASSERT_EQ(0, conversionErrors.size()) << BeStringUtilities::Join(conversionErrors, ", ");
    Utf8String fileName = UnitsTestFixture::GetOutputDataPath(L"TestUsCustomaryAreas_handledUnits.csv");
    WriteToFile(fileName.c_str(), handledUnits);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                                  03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitConversionTests, USSurveyAreas)
    {
    bvector<Utf8String> loadErrors;
    bvector<Utf8String> conversionErrors;
    bvector<bpair<Utf8String, Utf8String>> handledUnits;
    // Conversion tests where expected value is taken directly out of  http://www.nist.gov/pml/wmd/pubs/upload/hb44-15-web-final.pdf, Appendix C. Section 4, Page C-8
    // Directly from exact values in tables
    TestUnitConversion(1.0, "SQ.US_SURVEY_MILE", 4014489600, "SQ.US_SURVEY_IN", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.US_SURVEY_MILE", 27878400, "SQ.US_SURVEY_FT", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.US_SURVEY_MILE", 3097600, "SQ.US_SURVEY_YRD", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.US_SURVEY_MILE", 6400, "SQ.US_SURVEY_CHAIN", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.US_SURVEY_MILE", 640, "US_SURVEY_ACRE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.US_SURVEY_CHAIN", 0.1, "US_SURVEY_ACRE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "US_SURVEY_ACRE", 43560, "SQ.US_SURVEY_FT", 1, loadErrors, conversionErrors, handledUnits);
    
    // Derived from exact values
    TestUnitConversion(1.0, "SQ.IN", pow(0.999998, 2), "SQ.US_SURVEY_IN", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.FT", pow(0.999998, 2), "SQ.US_SURVEY_FT", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.YRD", pow(0.999998, 2), "SQ.US_SURVEY_YRD", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.CHAIN", pow(0.999998, 2), "SQ.US_SURVEY_CHAIN", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "ACRE", pow(0.999998, 2), "US_SURVEY_ACRE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.MILE", pow(0.999998, 2), "SQ.US_SURVEY_MILE", 1, loadErrors, conversionErrors, handledUnits);

    // Exact values do not exist in document
    TestUnitConversion(1.0, "SQ.US_SURVEY_IN", 0.09290341 / 144.0, "SQ.M", 100000000, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.US_SURVEY_FT", 0.09290341, "SQ.M", 100000000, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.US_SURVEY_YRD", 9.0 * 0.09290341, "SQ.M", 100000000, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.US_SURVEY_CHAIN", 404.6873, "SQ.M", 1000000000, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "US_SURVEY_ACRE", 4046.873, "SQ.M", 1000000000, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.US_SURVEY_MILE", 2589998, "SQ.M", 1000000000, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.US_SURVEY_FT", 1.000004, "SQ.FT", 100000, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "SQ.US_SURVEY_MILE", 1.000004, "SQ.MILE", 100000, loadErrors, conversionErrors, handledUnits);

    ASSERT_EQ(0, loadErrors.size()) << BeStringUtilities::Join(loadErrors, ", ");
    ASSERT_EQ(0, conversionErrors.size()) << BeStringUtilities::Join(conversionErrors, ", ");
    Utf8String fileName = UnitsTestFixture::GetOutputDataPath(L"TestUsSurveyAreas_handledUnits.csv");
    WriteToFile(fileName.c_str(), handledUnits);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                                 02/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitConversionTests, UnitsConversions_Complex)
    {
    bvector<Utf8String> loadErrors;
    bvector<Utf8String> conversionErrors;
    bvector<bpair<Utf8String, Utf8String>> handledUnits;

    TestUnitConversion(30.48 * 60, "CENTIMETRE_PER_HOUR", 1.0, "FOOT_PER_MINUTE", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(30.48 * 3600, "CENTIMETRE_PER_HOUR", 1.0, "FOOT_PER_SECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(2.54 * 60, "CENTIMETRE_PER_HOUR", 1.0, "INCH_PER_MINUTE", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(2.54 * 3600, "CENTIMETRE_PER_HOUR", 1.0, "INCH_PER_SECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1853.184 * 100, "CENTIMETRE_PER_HOUR", 1.0, "KNOT", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(30.48 * 5280, "CENTIMETRE_PER_HOUR", 1.0, "MILE_PER_HOUR", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0 / 24.0, "CENTIMETRE_PER_HOUR", 1.0, "CENTIMETRE_PER_DAY", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(30.48 / 24.0, "CENTIMETRE_PER_HOUR", 1.0, "FOOT_PER_DAY", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(2.54 / 24.0, "CENTIMETRE_PER_HOUR", 1.0, "INCH_PER_DAY", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(100.0 / 24.0, "CENTIMETRE_PER_HOUR", 1.0, "METRE_PER_DAY", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(0.1 / 24.0, "CENTIMETRE_PER_HOUR", 1.0, "MILLIMETRE_PER_DAY", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(30.48 * 60e6, "CENTIMETRE_PER_HOUR", 1.0e6, "FOOT_PER_MINUTE", 10000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(30.48 * 3600e6, "CENTIMETRE_PER_HOUR", 1.0e6, "FOOT_PER_SECOND", 1000000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(2.54 * 60e6, "CENTIMETRE_PER_HOUR", 1.0e6, "INCH_PER_MINUTE", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(2.54 * 3600e6, "CENTIMETRE_PER_HOUR", 1.0e6, "INCH_PER_SECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1853.184 * 100 * 1e6, "CENTIMETRE_PER_HOUR", 1.0e6, "KNOT", 100000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e6 * 30.48 * 5280, "CENTIMETRE_PER_HOUR", 1.0e6, "MILE_PER_HOUR", 1000000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e6 / 24.0, "CENTIMETRE_PER_HOUR", 1.0e6, "CENTIMETRE_PER_DAY", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(30.48e6 / 24.0, "CENTIMETRE_PER_HOUR", 1.0e6, "FOOT_PER_DAY", 100000000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(2.54e6 / 24.0, "CENTIMETRE_PER_HOUR", 1.0e6, "INCH_PER_DAY", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e8 / 24.0, "CENTIMETRE_PER_HOUR", 1.0e6, "METRE_PER_DAY", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e5 / 24.0, "CENTIMETRE_PER_HOUR", 1.0e6, "MILLIMETRE_PER_DAY", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(9.80665 / 1.0e-5, "DYNE", 1.0, "KILOGRAM_FORCE", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1000 / 1.0e-5, "DYNE", 1.0, "KILONEWTON", 100000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(0.001 / 1.0e-5, "DYNE", 1.0, "MILLINEWTON", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0 / 1.0e-5, "DYNE", 1.0, "NEWTON", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(9.80665e6 / 1.0e-5, "DYNE", 1.0e6, "KILOGRAM_FORCE", 10000000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e5 / 1.01325e5, "ATMOSPHERE", 1.0, "BAR", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(((1 + 1.01325) * 1.0e5) / 1.01325e5, "ATMOSPHERE", 1.0, "BAR_GAUGE", 1000, loadErrors, conversionErrors, handledUnits, true); //Gauge Offset was inverse in Units 1.0.  Switched - to + to correct expected value
    TestUnitConversion(0.1 / 1.01325e5, "ATMOSPHERE", 1.0, "BARYE", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(2.989067e3 / 1.01325e5, "ATMOSPHERE", 1.0, "FOOT_OF_H2O_CONVENTIONAL", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(249.1083 / 1.01325e5, "ATMOSPHERE", 1.0, "INCH_OF_H2O_AT_32_FAHRENHEIT", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(2.49082e2 / 1.01325e5, "ATMOSPHERE", 1.0, "INCH_OF_H2O_AT_39_2_FAHRENHEIT", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(2.4884e2 / 1.01325e5, "ATMOSPHERE", 1.0, "INCH_OF_H2O_AT_60_FAHRENHEIT", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(3.38638e3 / 1.01325e5, "ATMOSPHERE", 1.0, "INCH_OF_HG_AT_32_FAHRENHEIT", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(3.386389e3 / 1.01325e5, "ATMOSPHERE", 1.0, "INCH_OF_HG_CONVENTIONAL", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(3.37685e3 / 1.01325e5, "ATMOSPHERE", 1.0, "INCH_OF_HG_AT_60_FAHRENHEIT", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(9.80665e4 / 1.01325e5, "ATMOSPHERE", 1.0, "KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(((1 + 101.325 / 98.0665) * 9.80665e4) / 1.01325e5, "ATMOSPHERE", 1.0, "KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED_GAUGE", 1000, loadErrors, conversionErrors, handledUnits, true); //Gauge Offset was inverse in Units 1.0.  Switched - to + to correct expected value
    TestUnitConversion(9.80665 / 1.01325e5, "ATMOSPHERE", 1.0, "KILOGRAM_FORCE_PER_METRE_SQUARED", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1000 / 1.01325e5, "ATMOSPHERE", 1.0, "KILOPASCAL", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(((1 + 101.325) * 1000) / 1.01325e5, "ATMOSPHERE", 1.0, "KILOPASCAL_GAUGE", 1000, loadErrors, conversionErrors, handledUnits, true); //Gauge Offset was inverse in Units 1.0.  Switched - to + to correct expected value
    TestUnitConversion(1000000 / 1.01325e5, "ATMOSPHERE", 1.0, "MEGAPASCAL", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(((1 + 101.325 / 1000) * 1000000) / 1.01325e5, "ATMOSPHERE", 1.0, "MEGAPASCAL_GAUGE", 1000, loadErrors, conversionErrors, handledUnits, true); //Gauge Offset was inverse in Units 1.0.  Switched - to + to correct expected value
    TestUnitConversion(9806.65 / 1.01325e5, "ATMOSPHERE", 1.0, "METRE_OF_H2O_CONVENTIONAL", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(9.80665 / 1.01325e5, "ATMOSPHERE", 1.0, "MILLIMETRE_OF_H2O_CONVENTIONAL", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0, "MILLIMETRE_OF_HG_AT_32_FAHRENHEIT", 1.33322e2 / 101325.0, "ATMOSPHERE", 1000, loadErrors, conversionErrors, handledUnits, true);  // KnowledgeDoor and hand calculation agree with actual value more than value from old system
    TestUnitConversion(1 / 1.01325e5, "ATMOSPHERE", 1.0, "NEWTON_PER_METRE_SQUARED", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1 / 1.01325e5, "ATMOSPHERE", 1.0, "PASCAL", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0, "POUND_FORCE_PER_FOOT_SQUARED", 47.88026 / 1.01325e5, "ATMOSPHERE", 100000000, loadErrors, conversionErrors, handledUnits, true);  // Uses NIST table conversion value for LBF/FT^2 to Pascal.  This has 7 significant digits so comparison must use reduced precision.
    TestUnitConversion(1.0, "POUND_FORCE_PER_INCH_SQUARED", 6.894757e3 / 1.01325e5, "ATMOSPHERE", 100000000, loadErrors, conversionErrors, handledUnits, true);  // Uses NIST table conversion value for LBF/IN^2 to Pascal.  This has 7 significant digits so comparison must use reduced precision.
    TestUnitConversion(1.0, "POUND_FORCE_PER_INCH_SQUARED_GAUGE", ((1 + 101.325 / 6.894757) * 6.894757e3) / 1.01325e5, "ATMOSPHERE", 10000000, loadErrors, conversionErrors, handledUnits, true);  // Uses NIST table conversion value for LBF/IN^2 to Pascal.  This has 7 significant digits so comparison must use reduced precision.  Gauge Offset was inverse in Units 1.0.  Switched - to + to correct expected value
    TestUnitConversion(1.0e2 / 1.01325e5, "ATMOSPHERE", 1.0, "MILLIBAR", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(100.0 / 1.01325e5, "ATMOSPHERE", 1.0, "HECTOPASCAL", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1000000.0 * 1.0e5 / 1.01325e5, "ATMOSPHERE", 1000000.0, "BAR", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(((1000000.0 + 1.01325) * 1.0e5) / 1.01325e5, "ATMOSPHERE", 1000000.0, "BAR_GAUGE", 1000, loadErrors, conversionErrors, handledUnits, true); //Gauge Offset was inverse in Units 1.0.  Switched - to + to correct expected value
    TestUnitConversion(100000 / 1.01325e5, "ATMOSPHERE", 1000000.0, "BARYE", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1000000.0 * 2.989067e3 / 1.01325e5, "ATMOSPHERE", 1000000.0, "FOOT_OF_H2O_CONVENTIONAL", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1000000.0 * 249.1083 / 1.01325e5, "ATMOSPHERE", 1000000.0, "INCH_OF_H2O_AT_32_FAHRENHEIT", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1000000.0 * 2.49082e2 / 1.01325e5, "ATMOSPHERE", 1000000.0, "INCH_OF_H2O_AT_39_2_FAHRENHEIT", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1000000.0 * 2.4884e2 / 1.01325e5, "ATMOSPHERE", 1000000.0, "INCH_OF_H2O_AT_60_FAHRENHEIT", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1000000.0 * 3.38638e3 / 1.01325e5, "ATMOSPHERE", 1000000.0, "INCH_OF_HG_AT_32_FAHRENHEIT", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1000000.0 * 3.386389e3 / 1.01325e5, "ATMOSPHERE", 1000000.0, "INCH_OF_HG_CONVENTIONAL", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1000000.0 * 3.37685e3 / 1.01325e5, "ATMOSPHERE", 1000000.0, "INCH_OF_HG_AT_60_FAHRENHEIT", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1000000.0 * 9.80665e4 / 1.01325e5, "ATMOSPHERE", 1000000.0, "KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(((1000000 + 101.325 / 98.0665) * 9.80665e4) / 1.01325e5, "ATMOSPHERE", 1000000.0, "KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED_GAUGE", 1000, loadErrors, conversionErrors, handledUnits, true); //Gauge Offset was inverse in Units 1.0.  Switched - to + to correct expected value
    TestUnitConversion(1000000.0 * 9.80665 / 1.01325e5, "ATMOSPHERE", 1000000.0, "KILOGRAM_FORCE_PER_METRE_SQUARED", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1000000000 / 1.01325e5, "ATMOSPHERE", 1000000.0, "KILOPASCAL", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(((1000000 + 101.325) * 1000) / 1.01325e5, "ATMOSPHERE", 1000000.0, "KILOPASCAL_GAUGE", 1000, loadErrors, conversionErrors, handledUnits, true); //Gauge Offset was inverse in Units 1.0.  Switched - to + to correct expected value
    TestUnitConversion(1000000000000 / 1.01325e5, "ATMOSPHERE", 1000000.0, "MEGAPASCAL", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(((1000000 + 101.325 / 1000) * 1000000) / 1.01325e5, "ATMOSPHERE", 1000000.0, "MEGAPASCAL_GAUGE", 1000, loadErrors, conversionErrors, handledUnits, true); //Gauge Offset was inverse in Units 1.0.  Switched - to + to correct expected value
    TestUnitConversion(1000000.0 * 9806.65 / 1.01325e5, "ATMOSPHERE", 1000000.0, "METRE_OF_H2O_CONVENTIONAL", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1000000.0 * 9.80665 / 1.01325e5, "ATMOSPHERE", 1000000.0, "MILLIMETRE_OF_H2O_CONVENTIONAL", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion((1000000.0 * 133.322) / 1.01325e5, "ATMOSPHERE", 1000000.0, "MILLIMETRE_OF_HG_AT_32_FAHRENHEIT", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1000000 / 1.01325e5, "ATMOSPHERE", 1000000.0, "NEWTON_PER_METRE_SQUARED", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1000000 / 1.01325e5, "ATMOSPHERE", 1000000.0, "PASCAL", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion((101325.0 / 760.0) / 1.01325e5, "ATMOSPHERE", 1.0, "TORR", 1, loadErrors, conversionErrors, handledUnits, true);// Changed approx. conversion factor for TORR from old system to exact definition, so changed expected value to match
    TestUnitConversion(1000000.0 * (101325.0 / 760.0) / 1.01325e5, "ATMOSPHERE", 1000000.0, "TORR", 1, loadErrors, conversionErrors, handledUnits, true); // Changed approx. conversion factor for TORR from old system to exact definition, so changed expected value to match
    TestUnitConversion(1.0e8 / 1.01325e5, "ATMOSPHERE", 1.0e6, "MILLIBAR", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(100.0e6 / 1.01325e5, "ATMOSPHERE", 1.0e6, "HECTOPASCAL", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0 / (1000.0 * 3600.0), "KILOWATT_HOUR_PER_METRE_CUBED", 1.0, "JOULE_PER_METRE_CUBED", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0 / 3600, "KILOWATT_HOUR_PER_METRE_CUBED", 1.0, "KILOJOULE_PER_METRE_CUBED", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0 / 3.6, "KILOWATT_HOUR_PER_METRE_CUBED", 1.0, "MEGAJOULE_PER_METRE_CUBED", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e6 / (1000 * 3600), "KILOWATT_HOUR_PER_METRE_CUBED", 1.0e6, "JOULE_PER_METRE_CUBED", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e6 / 3600, "KILOWATT_HOUR_PER_METRE_CUBED", 1.0e6, "KILOJOULE_PER_METRE_CUBED", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e6 / 3.6, "KILOWATT_HOUR_PER_METRE_CUBED", 1.0e6, "MEGAJOULE_PER_METRE_CUBED", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(3600 * 24, "GRAM_PER_DAY", 1.0, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(3.6 * 24, "KILOGRAM_PER_DAY", 1.0, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e6 * 3600 * 24, "MICROGRAM_PER_DAY", 1.0, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e6 * 3600, "MICROGRAM_PER_HOUR", 1.0, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e6 * 60, "MICROGRAM_PER_MINUTE", 1.0, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e3 * 3600 * 24, "MILLIGRAM_PER_DAY", 1.0, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e3 * 3600, "MILLIGRAM_PER_HOUR", 1.0, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e3 * 60, "MILLIGRAM_PER_MINUTE", 1.0, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(3600 * 24 * 1.0e6, "GRAM_PER_DAY", 1.0e6, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(3600 * 1.0e6, "GRAM_PER_HOUR", 1.0e6, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(60 * 1.0e6, "GRAM_PER_MINUTE", 1.0e6, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(3.6e6 * 24, "KILOGRAM_PER_DAY", 1.0e6, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e12 * 3600 * 24, "MICROGRAM_PER_DAY", 1.0e6, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e12 * 3600, "MICROGRAM_PER_HOUR", 1.0e6, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e12 * 60, "MICROGRAM_PER_MINUTE", 1.0e6, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e9 * 3600 * 24, "MILLIGRAM_PER_DAY", 1.0e6, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e9 * 3600, "MILLIGRAM_PER_HOUR", 1.0e6, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e9 * 60, "MILLIGRAM_PER_MINUTE", 1.0e6, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(5280.0 / 2, "FOOT_PER_MILE", 50.0, "PERCENT_SLOPE", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0 / 33.0, "VERTICAL_PER_HORIZONTAL", 33.0, "ONE_OVER_SLOPE", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0 / 7000.0, "KILOGRAM_PER_KILOGRAM", 1.0, "GRAIN_MASS_PER_POUND_MASS", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(5.0 / 9.0, "RECIPROCAL_DELTA_DEGREE_FAHRENHEIT", 1.0, "RECIPROCAL_DELTA_DEGREE_CELSIUS", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(5.0 / 9.0, "RECIPROCAL_DELTA_DEGREE_FAHRENHEIT", 1.0, "RECIPROCAL_DELTA_DEGREE_KELVIN", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(9.0 / 5.0, "RECIPROCAL_DELTA_DEGREE_CELSIUS", 1.0, "RECIPROCAL_DELTA_DEGREE_FAHRENHEIT", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(9.0 / 5.0, "RECIPROCAL_DELTA_DEGREE_CELSIUS", 1.0, "RECIPROCAL_DELTA_DEGREE_RANKINE", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0 / 0.3048, "ONE_PER_METRE", 1.0, "ONE_PER_FOOT", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0 / 1000.0, "ONE_PER_FOOT", 1.0, "ONE_PER_THOUSAND_FOOT", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0 / 5280.0, "ONE_PER_FOOT", 1.0, "ONE_PER_MILE", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e6 / 0.3048, "ONE_PER_METRE", 1.0e6, "ONE_PER_FOOT", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e6 / 1000.0, "ONE_PER_FOOT", 1.0e6, "ONE_PER_THOUSAND_FOOT", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e6 / 5280.0, "ONE_PER_FOOT", 1.0e6, "ONE_PER_MILE", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0 / 3600.0, "HERTZ", 1.0, "ONE_PER_HOUR", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0 / 31536000.0, "HERTZ", 1.0, "ONE_PER_YEAR", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0 / 86400.0, "HERTZ", 1.0, "ONE_PER_DAY", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0 / 60.0, "HERTZ", 1.0, "ONE_PER_MINUTE", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e6 / 3600.0, "HERTZ", 1.0e6, "ONE_PER_HOUR", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e6 / 31536000.0, "HERTZ", 1.0e6, "ONE_PER_YEAR", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e6 / 86400.0, "HERTZ", 1.0e6, "ONE_PER_DAY", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e6 / 60.0, "HERTZ", 1.0e6, "ONE_PER_MINUTE", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0 / 0.00023884589662749597, "JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0, "BTU_PER_POUND_MASS_PER_DELTA_DEGREE_RANKINE", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e6 / 0.00023884589662749597, "JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0e6, "BTU_PER_POUND_MASS_PER_DELTA_DEGREE_RANKINE", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0 / 0.42992261392949271, "KILOJOULE_PER_KILOMOLE", 1.0, "BTU_PER_POUND_MOLE", 1000, loadErrors, conversionErrors, handledUnits, true);
    TestUnitConversion(1.0e6 / 0.42992261392949271, "KILOJOULE_PER_KILOMOLE", 1.0e6, "BTU_PER_POUND_MOLE", 1000, loadErrors, conversionErrors, handledUnits, true);
    
    //Missing Units: KNOT, MEGAJOULE_PER_METRE_CUBED, GRAM_PER_DAY, ONE_OVER_SLOPE, ONE_PER_HOUR, ONE_PER_YEAR, ONE_PER_DAY, ONE_PER_MINUTE, KILOJOULE_PER_KILOMOLE, BTU_PER_POUND_MOLE
    EXPECT_EQ(10, loadErrors.size()) << "The following units were not found: " << BeStringUtilities::Join(loadErrors, ", ");
    EXPECT_EQ(0, conversionErrors.size()) << "Failed to convert between the following units: " << BeStringUtilities::Join(conversionErrors, ", ");
    Utf8String fileName = UnitsTestFixture::GetOutputDataPath(L"TestUnitsConversions_Complex_handledUnits.csv");
    WriteToFile(fileName.c_str(), handledUnits);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                      03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitConversionTests, UnitsConversion_CompareToRawOutputFromOldSystem)
    {
    TestConversionsLoadedFromCvsFile("ConversionsBetweenAllOldUnits.csv", L"TestConversionsBetweenAllOldUnits_handledUnits.csv", 112);
    // went from 107 to 109 because work per month units were removed, back to 107 because mass ratios added
    // Down to 103 with addition of: LITRE_PER_KILOMETRE_SQUARED_PER_SECOND, VOLT_AMPERE, KILOVOLT_AMPERE and MEGAVOLT_AMPERE
    // Up to 112 because THREAD_PITCH units removed ... units came from old system but Phen doesn't match units.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                                03/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitConversionTests, UnitsConversion)
    {
    TestConversionsLoadedFromCvsFile("unitcomparisondata.csv", L"Testunitcomparisondata_handledUnits.csv", 99);
    // went from 94 to 96 because work per month units were removed, back to 94 because mass ratios added
    // Down to 90 with addition of: LITRE_PER_KILOMETRE_SQUARED_PER_SECOND, VOLT_AMPERE, KILOVOLT_AMPERE and MEGAVOLT_AMPERE
    // Up to 99 because THREAD_PITCH units removed ... units came from old system but Phen doesn't match units.
    }

//---------------------------------------------------------------------------------------//
// @bsimethod                            Colin.Kerr                                  10/17
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(UnitConversionTests, ApparentPower_Conversions)
    {
    bvector<Utf8String> loadErrors;
    bvector<Utf8String> conversionErrors;
    bvector<bpair<Utf8String, Utf8String>> handledUnits;
    TestUnitConversion(1.0, "VA", 1.0e-3, "KVA", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "VA", 1.0e-6, "MVA", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "MVA", 1.0e6, "VA", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(0.0, "VA", 0.0, "MVA", 1, loadErrors, conversionErrors, handledUnits);

    ASSERT_EQ(0, loadErrors.size()) << BeStringUtilities::Join(loadErrors, ", ");
    ASSERT_EQ(0, conversionErrors.size()) << BeStringUtilities::Join(conversionErrors, ", ");
    Utf8String fileName = UnitsTestFixture::GetOutputDataPath(L"TestUsCustomaryLengths_handledUnits.csv");
    WriteToFile(fileName.c_str(), handledUnits);
    }

//---------------------------------------------------------------------------------------//
// @bsimethod                            Colin.Kerr                                  09/17
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(UnitConversionTests, VolumeRatio_Conversions)
    {
    bvector<Utf8String> loadErrors;
    bvector<Utf8String> conversionErrors;
    bvector<bpair<Utf8String, Utf8String>> handledUnits;
    TestUnitConversion(1.0, "CUB.M/CUB.M", 1.0, "LITRE/LITRE", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "CUB.M/CUB.M", 1.0, "CUB.M/CUB.M", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "LITRE/LITRE", 1.0, "CUB.M/CUB.M", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "LITRE/LITRE", 1.0, "LITRE/LITRE", 1, loadErrors, conversionErrors, handledUnits);

    ASSERT_EQ(0, loadErrors.size()) << BeStringUtilities::Join(loadErrors, ", ");
    ASSERT_EQ(0, conversionErrors.size()) << BeStringUtilities::Join(conversionErrors, ", ");
    Utf8String fileName = UnitsTestFixture::GetOutputDataPath(L"TestUsCustomaryLengths_handledUnits.csv");
    WriteToFile(fileName.c_str(), handledUnits);
    }

double pi = 3.1415926535897932;
//---------------------------------------------------------------------------------------//
// @bsimethod                            Colin.Kerr                                  02/18
//+---------------+---------------+---------------+---------------+---------------+------//
TEST_F(UnitConversionTests, AngularVelocity_Conversions)
    {
    bvector<Utf8String> loadErrors;
    bvector<Utf8String> conversionErrors;
    bvector<bpair<Utf8String, Utf8String>> handledUnits;
    TestUnitConversion(1.0, "RAD/SEC", 60.0, "RAD/MIN", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "RAD/MIN", 60.0, "RAD/HR", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "RAD/HR", 1.0 / (2.0 * pi * 60.0 * 60.0), "RPS", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "RPS", 60.0, "RPM", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "RPM", 60.0, "RPH", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "RPH", 360.0 / (60.0 * 60.0), "DEG/SEC", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "DEG/SEC", 60.0, "DEG/MIN", 1, loadErrors, conversionErrors, handledUnits);
    TestUnitConversion(1.0, "DEG/MIN", 60.0, "DEG/HR", 1, loadErrors, conversionErrors, handledUnits);

    ASSERT_EQ(0, loadErrors.size()) << BeStringUtilities::Join(loadErrors, ", ");
    ASSERT_EQ(0, conversionErrors.size()) << BeStringUtilities::Join(conversionErrors, ", ");
    Utf8String fileName = UnitsTestFixture::GetOutputDataPath(L"TestUsCustomaryLengths_handledUnits.csv");
    WriteToFile(fileName.c_str(), handledUnits);
    }

//TEST_F(UnitConversionTests, Money_Conversions)
//    {
//    bvector<Utf8String> loadErrors;
//    bvector<Utf8String> conversionErrors;
//    bvector<bpair<Utf8String, Utf8String>> handledUnits;
//    TestUnitConversion(1.0, "US$", 0.0, "Euro", 1, loadErrors, conversionErrors, handledUnits, false, false, UnitsProblemCode::UncomparableUnits);
//    TestUnitConversion(1.0, "Euro", 0.0, "US$", 1, loadErrors, conversionErrors, handledUnits, false, false, UnitsProblemCode::UncomparableUnits);
//    TestUnitConversion(1.0, "US$", 1.0, "US$", 1, loadErrors, conversionErrors, handledUnits);
//    TestUnitConversion(1.0, "Euro", 1.0, "Euro", 1, loadErrors, conversionErrors, handledUnits);
//    ASSERT_EQ(0, loadErrors.size()) << BeStringUtilities::Join(loadErrors, ", ");
//    ASSERT_EQ(0, conversionErrors.size()) << BeStringUtilities::Join(conversionErrors, ", ");
//    }

// TODO: Make this test pass when conversions fail and add more conversions to test a wide spectrum of dimenions.
//TEST_F(UnitConversionTests, TestConversionsThatShouldFail)
//    {
//    bvector<Utf8String> loadErrors;
//    bvector<Utf8String> conversionErrors;
//    TestUnitConversion(1.0, "JOULE", 1.0, "NEWTON_METRE", 1000, loadErrors, conversionErrors);
//    }

END_UNITS_UNITTESTS_NAMESPACE
