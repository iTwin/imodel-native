/*--------------------------------------------------------------------------------------+
|
|  $Source: test/UnitsTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "UnitsTestFixture.h"
#include <fstream>
#include <sstream>

using namespace BentleyApi::Units;
BEGIN_UNITS_UNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                     Basanta.Kharel                 12/2015
//+---------------+---------------+---------------+---------------+---------------+------
struct UnitsTests : UnitsTestFixture
    {
    typedef std::function<void(bvector<Utf8String>&)> CSVLineProcessor;

    static bool TestUnitConversion(double fromVal, Utf8CP fromUnitName, double expectedVal, Utf8CP targetUnitName, int ulp, 
                                   bvector<Utf8String>& loadErrors, bvector<Utf8String>& conversionErrors, bool showDetailLogs = false);
    static void TestConversionsLoadedFromCvsFile(Utf8CP fileName);

    static Utf8String ParseUOM(Utf8CP unitName, bset<Utf8String>& notMapped)
        {
        UnitCP uom = LocateUOM(unitName);
        if (nullptr != uom)
            return uom->GetName();

        notMapped.insert(unitName);
        return "NULL";
        }

    static UnitCP LocateUOM(Utf8CP unitName)
        {
        return UnitRegistry::Instance().LookupUnit(unitName);
        }

    static void GetMapping(WCharCP file, bmap<Utf8String, Utf8String>& unitNameMap, bset<Utf8String>& notMapped)
        {
        auto lineProcessor = [&unitNameMap, &notMapped] (bvector<Utf8String>& tokens)
            {
            Utf8String newName1 = ParseUOM(tokens[1].begin(), notMapped);
            Utf8String newName2 = ParseUOM(tokens[3].begin(), notMapped);
            unitNameMap[tokens[1]] = newName1;
            unitNameMap[tokens[3]] = newName2;
            };

        ReadConversionCsvFile(file, lineProcessor);
        }

    static void ReadConversionCsvFile(WCharCP file, CSVLineProcessor lineProcessor)
        {
        Utf8String path = UnitsTestFixture::GetConversionDataPath(file);
        std::ifstream ifs(path.begin(), std::ifstream::in);
        std::string line;

        while (std::getline(ifs, line))
            {
            bvector<Utf8String> tokens;
            BeStringUtilities::Split(line.c_str(), ",", tokens);
            lineProcessor(tokens);
            }
        }

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
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                              Chris.Tartamella     02/16
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T> typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
static almost_equal(const T x, const T y, int ulp)
    {
    // the machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return std::abs(x - y) < std::numeric_limits<T>::epsilon() * std::abs(x + y) * ulp
        // unless the result is subnormal
        || std::abs(x - y) < std::numeric_limits<T>::min();
    }

/*---------------------------------------------------------------------------------**//**
// @bsiclass                                     Basanta.Kharel                 12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitsTests::TestUnitConversion (double fromVal, Utf8CP fromUnitName, double expectedVal, Utf8CP targetUnitName, int ulp, 
                                     bvector<Utf8String>& missingUnits, bvector<Utf8String>& conversionErrors, bool showDetailLogs)
    {
    //if either units are not in the library conversion is not possible
    //UnitsMapping test checks if all units are there and fails when a unit is not found
    UnitCP fromUnit = LocateUOM(fromUnitName);
    UnitCP targetUnit = LocateUOM(targetUnitName);
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

    PERFORMANCELOG.debugv("About to try to convert %.16g %s to %s", fromVal, fromUnit->GetName(), targetUnit->GetName());
    double convertedVal = fromUnit->Convert(fromVal, targetUnit);;

    //QuantityP q = SimpleQuantity(fromVal, fromUnitName);
    //if (nullptr == q)
    //    {
    //    Utf8PrintfString error("%s with Value %lf", fromUnitName, fromVal);
    //    loadErrors.push_back(error);
    //    return;
    //    }

    //double convertedVal = q->Value(targetUnitName);
    PERFORMANCELOG.debugv("Converted %s to %s.  Expected: %.17g  Actual: %.17g", fromUnit->GetName(), targetUnit->GetName(), expectedVal, convertedVal);
    //if (fabs(convertedVal - expectedVal) > tolerance)
    if(!almost_equal<double>(expectedVal, convertedVal, ulp))
        {
        Utf8PrintfString formattedText("Conversion from %s to %s. Input: %.17g \nOutput:   %.17g \nExpected: %.17g \nDiff:     %.17g   Diff/Exp: %.17g   ULP: %d\n", 
                                       fromUnitName, targetUnitName, fromVal, convertedVal, expectedVal, convertedVal - expectedVal, (convertedVal - expectedVal)/expectedVal, ulp);
        conversionErrors.push_back(formattedText);
        EXPECT_FALSE(true) << formattedText;
        }
    EXPECT_FALSE(std::isnan(convertedVal) || !std::isfinite(convertedVal)) << "Conversion from " << fromUnitName << " to " << targetUnitName << " resulted in an invalid number";
    //EXPECT_NEAR(expectedVal, convertedVal, tolerance)<<  "Conversion from "<< fromUnitName << " to " << targetUnitName <<". Input : " << fromVal << ", Output : " << convertedVal << ", ExpectedOutput : " << expectedVal << " Tolerance : " << tolerance<< "\n";

    if (showDetailLogs)
        {
        NativeLogging::LoggingConfig::SetSeverity("UnitsNative", NativeLogging::SEVERITY::LOG_ERROR);
        NativeLogging::LoggingConfig::SetSeverity("Performance", NativeLogging::SEVERITY::LOG_ERROR);
        }
    return true;
    }
    
TEST_F (UnitsTests, UnitsMapping)
    {
    bmap<Utf8String, Utf8String> unitNameMap;
    bset<Utf8String> notMapped;
    GetMapping(L"unitcomparisondata.csv", unitNameMap, notMapped);
    GetMapping(L"complex.csv", unitNameMap, notMapped);

    //output of oldUnit, newUnit mapping
    Utf8String mapOldtoNew = UnitsTestFixture::GetOutputDataPath(L"mapOldtoNew.csv");

    Utf8String guess= "";
    for (auto i : notMapped)
        {
        guess += i + ", ";
        }
    
    if (notMapped.size() > 0)
        PERFORMANCELOG.error(guess.c_str());
    //EXPECT_EQ (0, notMapped.size() ) << guess;
    }

// TODO: Make this test pass when conversions fail and add more conversions to test a wide spectrum of dimenions.
//TEST_F(UnitsTests, TestConversionsThatShouldFail)
//    {
//    bvector<Utf8String> loadErrors;
//    bvector<Utf8String> conversionErrors;
//    TestUnitConversion(1.0, "JOULE", 1.0, "NEWTON_METRE", 1000, loadErrors, conversionErrors);
//    }

TEST_F(UnitsTests, TestOffsetConversions)
    {
    bvector<Utf8String> loadErrors;
    bvector<Utf8String> conversionErrors;

    // Expected values generated using the following equations and windows calculator
    // Celsius = (Fahrenheit–32)×5÷9
    // Kelvin = ((Fahrenheit–32)×5/9)+273.15
    // Rankine = Fahrenheit+459.67
    // Rømer = (Fahrenheit–32)×7/24+7.5
    TestUnitConversion(32, "FAHRENHEIT", 0, "CELSIUS", 1000, loadErrors, conversionErrors);
    TestUnitConversion(20, "FAHRENHEIT", -6.666666666666666666666666666, "CELSIUS", 1000, loadErrors, conversionErrors);
    TestUnitConversion(122, "FAHRENHEIT", 50, "CELSIUS", 1000, loadErrors, conversionErrors);
    TestUnitConversion(60, "FAHRENHEIT", 288.705555555555, "KELVIN", 1000, loadErrors, conversionErrors);
    TestUnitConversion(60, "FAHRENHEIT", 519.67, "RANKINE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(61.1, "FAHRENHEIT", 15.9875, "ROMER", 1000, loadErrors, conversionErrors);
    TestUnitConversion(0, "FAHRENHEIT", -17.777777777777777777777777777778, "CELSIUS", 1000, loadErrors, conversionErrors);
    TestUnitConversion(0, "FAHRENHEIT", 255.37222222222222222222222222222, "KELVIN", 1000, loadErrors, conversionErrors);
    TestUnitConversion(0, "FAHRENHEIT", 459.67, "RANKINE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(0, "FAHRENHEIT", -1.8333333333333, "ROMER", 1000, loadErrors, conversionErrors);

    // Expected values generated using the following equations and windows calculator
    // Fahrenheit = Celsius×9÷5+32
    // Kelvin = Celsius+273.15
    // Rankine = Celsius×9/5+32+459.67
    // Rømer = Celsius×21/40+7.5
    TestUnitConversion(1, "CELSIUS", 33.8, "FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(-15, "CELSIUS", 5, "FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(-25, "CELSIUS", -13, "FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(60, "CELSIUS", 140, "FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(60, "CELSIUS", 333.15, "KELVIN", 1000, loadErrors, conversionErrors);
    TestUnitConversion(60, "CELSIUS", 599.67, "RANKINE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(-14.3, "CELSIUS", -0.0075, "ROMER", 1000, loadErrors, conversionErrors);
    TestUnitConversion(0, "CELSIUS", 32, "FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(0, "CELSIUS", 273.15, "KELVIN", 1000, loadErrors, conversionErrors);
    TestUnitConversion(0, "CELSIUS", 491.67, "RANKINE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(0, "CELSIUS", 7.5, "ROMER", 1000, loadErrors, conversionErrors);

    TestUnitConversion(42, "KELVIN", -231.15, "CELSIUS", 1000, loadErrors, conversionErrors);
    TestUnitConversion(42, "KELVIN", -384.07, "FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(42, "KELVIN", 75.6, "RANKINE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(571.2, "KELVIN", 163.97625, "ROMER", 1000, loadErrors, conversionErrors);
    TestUnitConversion(0, "KELVIN", -273.15, "CELSIUS", 1000, loadErrors, conversionErrors);
    TestUnitConversion(0, "KELVIN", -459.67, "FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(0, "KELVIN", 0, "RANKINE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(0, "KELVIN", -135.90375, "ROMER", 1000, loadErrors, conversionErrors);


    // Expected values generated using the following equations and windows calculator
    // Celsius = (Rankine–459.67–32)×5/9
    // Fahrenheit = Rankine–459.67
    // Kelvin = (Rankine–459.67–32)×5/9+273.15
    // Rømer = (Rankine–491.67)×7/24+7.5
    TestUnitConversion(42, "RANKINE", -249.81666666666, "CELSIUS", 1000, loadErrors, conversionErrors);
    TestUnitConversion(42, "RANKINE", -417.67, "FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(42, "RANKINE", 23.333333333333333, "KELVIN", 1000, loadErrors, conversionErrors);
    TestUnitConversion(630, "RANKINE", 47.84625, "ROMER", 1000, loadErrors, conversionErrors);
    TestUnitConversion(0, "RANKINE", -273.15, "CELSIUS", 1000, loadErrors, conversionErrors);
    TestUnitConversion(0, "RANKINE", -459.67, "FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(0, "RANKINE", 0, "KELVIN", 1000, loadErrors, conversionErrors);
    TestUnitConversion(0, "RANKINE", -135.90375, "ROMER", 1000, loadErrors, conversionErrors);
    
    // Expected values generated using the following equations and windows calculator
    // Celsius = (Rømer–7.5)×40/21
    // Fahrenheit = (Rømer–7.5)×24/7+32
    // Kelvin = (Rømer–7.5)×40/21+273.15
    // Rankine = (Rømer–7.5)×24/7+491.67
    TestUnitConversion(42, "ROMER", 65.714285714285714285714285714286, "CELSIUS", 1000, loadErrors, conversionErrors);
    TestUnitConversion(42, "ROMER", 150.28571428571428571428571428571, "FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(42, "ROMER", 338.86428571428571428571428571429, "KELVIN", 1000, loadErrors, conversionErrors);
    TestUnitConversion(42, "ROMER", 609.95571428571428571428571428571, "RANKINE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(0, "ROMER", -14.285714285714285714285714285714, "CELSIUS", 1000, loadErrors, conversionErrors);
    TestUnitConversion(0, "ROMER", 6.285714285714285714, "FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(0, "ROMER", 258.8642857142857142, "KELVIN", 1000, loadErrors, conversionErrors);
    TestUnitConversion(0, "ROMER", 465.9557142857142857, "RANKINE", 1000, loadErrors, conversionErrors);


    Utf8String loadErrorString("The following units were not found:\n");
    for (auto const& val : loadErrors)
        loadErrorString.append(val + "\n");

    Utf8String conversionErrorString("Failed to convert between the following units:\n");
    for (auto const& val : conversionErrors)
        conversionErrorString.append(val + "\n");

    if (loadErrors.size() > 0)
        PERFORMANCELOG.error(loadErrorString.c_str());
    //EXPECT_EQ(0, loadErrors.size()) << loadErrorString;
    EXPECT_EQ(0, conversionErrors.size()) << conversionErrorString;

    }

TEST_F(UnitsTests, TestBasicConversion)
    {
    bvector<Utf8String> loadErrors;
    bvector<Utf8String> conversionErrors;
    TestUnitConversion(10, "FT", 3048, "MM", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1, "GALLON", 3.785411784, "LITRE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(100000, "FOOT_SQUARED", 100, "THOUSAND_FOOT_SQUARED", 1000, loadErrors, conversionErrors);
    TestUnitConversion(836127.36, "MILLIMETRE_SQUARED", 1.0, "YARD_SQUARED", 1000, loadErrors, conversionErrors);
    TestUnitConversion(3.17097919837647e-7, "YEAR", 10000.0, "MILLISECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1000000.0, "POUND", 1000000.0, "POUND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(2204622621.84878, "POUND", 1000000.0, "MEGAGRAM", 100000, loadErrors, conversionErrors);
    TestUnitConversion(1.66666666666667e-02, "DEGREE", 1.0, "ANGLE_MINUTE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.852e11, "CENTIMETRE_PER_HOUR", 1.0e6, "KNOT_INTERNATIONAL", 1000000, loadErrors, conversionErrors);
    TestUnitConversion(1.65409011373578e-3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e6, "LITRE_PER_KILOMETRE_SQUARED_PER_DAY", 1000, loadErrors, conversionErrors);
    TestUnitConversion(2.816538995808e13, "GALLON_PER_DAY_PER_PERSON", 1234e6, "LITRE_PER_SECOND_PER_PERSON", 1000, loadErrors, conversionErrors);
    TestUnitConversion(4.4482216152605e5, "DYNE", 1.0, "POUND_FORCE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(2.8316846592e3, "KILONEWTON_PER_FOOT_CUBED", 1.0e8, "NEWTON_PER_METRE_CUBED", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e9, "NEWTON_PER_METRE", 1.0e6, "NEWTON_PER_MILLIMETRE", 10000, loadErrors, conversionErrors);
    TestUnitConversion(3.43774677078493e9, "DEGREE_PER_HOUR", 1.0e6, "RADIAN_PER_MINUTE", 100000, loadErrors, conversionErrors);
    TestUnitConversion(2.65258238486492e3, "CYCLE_PER_SECOND", 1.0e6, "RADIAN_PER_MINUTE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(8.92179121619709e5, "POUND_PER_ACRE", 1.0e6, "KILOGRAM_PER_HECTARE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(8.92179121619701e6, "POUND_PER_ACRE", 1.0e6, "GRAM_PER_METRE_SQUARED", 10000, loadErrors, conversionErrors);
    TestUnitConversion(8.54292974552351e7, "FOOT_POUNDAL", 1.0, "KILOWATT_HOUR", 10000, loadErrors, conversionErrors);
    TestUnitConversion(2.37303604042319e7, "FOOT_POUNDAL", 1.0, "MEGAJOULE", 10000, loadErrors, conversionErrors);
    TestUnitConversion(42, "KG/S", 42000.0, "G/S", 1000, loadErrors, conversionErrors);
    TestUnitConversion(42, "CUB.M/SEC", 2.562997252e6, "CUB.IN/SEC", 100000, loadErrors, conversionErrors); // Exptected value has 10 digits of precision generated using http://www.knowledgedoor.com/2/calculators/convert_to_new_units.html
    TestUnitConversion(2.326e6, "KILOJOULE_PER_KILOGRAM", 1.0e6, "BTU_PER_POUND_MASS", 1000, loadErrors, conversionErrors);
    TestUnitConversion(60, "GRAM_PER_MINUTE", 1.0, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(3.53146667214886e1, "KILONEWTON_PER_METRE_CUBED", 1.0, "KILONEWTON_PER_FOOT_CUBED", 1000, loadErrors, conversionErrors);
    TestUnitConversion(42.42, "KILOPASCAL_GAUGE", 6.15250086300203, "POUND_FORCE_PER_INCH_SQUARED_GAUGE", 100000000, loadErrors, conversionErrors); // Expected value from old system, difference is due to imprecise offset in old system.
    TestUnitConversion(42.42, "HORIZONTAL_PER_VERTICAL", 1.0 / 42.42, "VERTICAL_PER_HORIZONTAL", 10, loadErrors, conversionErrors);
    }

TEST_F(UnitsTests, CheckDimensionForEveryPhenomenon)
    {
    bvector<PhenomenonCP> allPhenomena;
    UnitRegistry::Instance().AllPhenomena(allPhenomena);
    for (auto const& phenomenon : allPhenomena)
        {
        PERFORMANCELOG.errorv("Dimension string for %s: %s", phenomenon->GetName(), phenomenon->GetPhenomenonDimension().c_str());
        }
    }

TEST_F(UnitsTests, PhenomenonAndUnitDimensionsMatch)
    {
    bvector<PhenomenonCP> allPhenomena;
    UnitRegistry::Instance().AllPhenomena(allPhenomena);
    for (auto const& phenomenon : allPhenomena)
        {
        for (auto const& unit : phenomenon->GetUnits())
            {
            EXPECT_TRUE(phenomenon->IsCompatible(*unit)) << "The unit " << unit->GetName() << " is not dimensionally compatible with the phenomenon it belongs to: " << phenomenon->GetName();
            }
        }
    }

TEST_F(UnitsTests, UnitsConversions_Complex)
    {
    bvector<Utf8String> loadErrors;
    bvector<Utf8String> conversionErrors;

    TestUnitConversion(30.48 * 60, "CENTIMETRE_PER_HOUR", 1.0, "FOOT_PER_MINUTE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(30.48 * 3600, "CENTIMETRE_PER_HOUR", 1.0, "FOOT_PER_SECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(2.54 * 60, "CENTIMETRE_PER_HOUR", 1.0, "INCH_PER_MINUTE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(2.54 * 3600, "CENTIMETRE_PER_HOUR", 1.0, "INCH_PER_SECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1853.184 * 100, "CENTIMETRE_PER_HOUR", 1.0, "KNOT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(30.48 * 5280, "CENTIMETRE_PER_HOUR", 1.0, "MILE_PER_HOUR", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0 / 24.0, "CENTIMETRE_PER_HOUR", 1.0, "CENTIMETRE_PER_DAY", 1000, loadErrors, conversionErrors);
    TestUnitConversion(30.48 / 24.0, "CENTIMETRE_PER_HOUR", 1.0, "FOOT_PER_DAY", 1000, loadErrors, conversionErrors);
    TestUnitConversion(2.54 / 24.0, "CENTIMETRE_PER_HOUR", 1.0, "INCH_PER_DAY", 1000, loadErrors, conversionErrors);
    TestUnitConversion(100.0 / 24.0, "CENTIMETRE_PER_HOUR", 1.0, "METRE_PER_DAY", 1000, loadErrors, conversionErrors);
    TestUnitConversion(0.1 / 24.0, "CENTIMETRE_PER_HOUR", 1.0, "MILLIMETRE_PER_DAY", 1000, loadErrors, conversionErrors);
    TestUnitConversion(30.48 * 60e6, "CENTIMETRE_PER_HOUR", 1.0e6, "FOOT_PER_MINUTE", 10000, loadErrors, conversionErrors);
    TestUnitConversion(30.48 * 3600e6, "CENTIMETRE_PER_HOUR", 1.0e6, "FOOT_PER_SECOND", 1000000, loadErrors, conversionErrors);
    TestUnitConversion(2.54 * 60e6, "CENTIMETRE_PER_HOUR", 1.0e6, "INCH_PER_MINUTE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(2.54 * 3600e6, "CENTIMETRE_PER_HOUR", 1.0e6, "INCH_PER_SECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1853.184 * 100 * 1e6, "CENTIMETRE_PER_HOUR", 1.0e6, "KNOT", 100000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e6 * 30.48 * 5280, "CENTIMETRE_PER_HOUR", 1.0e6, "MILE_PER_HOUR", 1000000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e6 / 24.0, "CENTIMETRE_PER_HOUR", 1.0e6, "CENTIMETRE_PER_DAY", 1000, loadErrors, conversionErrors);
    TestUnitConversion(30.48e6 / 24.0, "CENTIMETRE_PER_HOUR", 1.0e6, "FOOT_PER_DAY", 100000000, loadErrors, conversionErrors);
    TestUnitConversion(2.54e6 / 24.0, "CENTIMETRE_PER_HOUR", 1.0e6, "INCH_PER_DAY", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e8 / 24.0, "CENTIMETRE_PER_HOUR", 1.0e6, "METRE_PER_DAY", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e5 / 24.0, "CENTIMETRE_PER_HOUR", 1.0e6, "MILLIMETRE_PER_DAY", 1000, loadErrors, conversionErrors);
    TestUnitConversion(9.80665 / 1.0e-5, "DYNE", 1.0, "KILOGRAM_FORCE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1000 / 1.0e-5, "DYNE", 1.0, "KILONEWTON", 100000, loadErrors, conversionErrors);
    TestUnitConversion(0.001 / 1.0e-5, "DYNE", 1.0, "MILLINEWTON", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0 / 1.0e-5, "DYNE", 1.0, "NEWTON", 1000, loadErrors, conversionErrors);
    TestUnitConversion(9.80665e6 / 1.0e-5, "DYNE", 1.0e6, "KILOGRAM_FORCE", 10000000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e5 / 1.01325e5, "ATMOSPHERE", 1.0, "BAR", 1000, loadErrors, conversionErrors);
    TestUnitConversion(((1 - 1.01325) * 1.0e5) / 1.01325e5, "ATMOSPHERE", 1.0, "BAR_GAUGE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(0.1 / 1.01325e5, "ATMOSPHERE", 1.0, "BARYE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(2.989067e3 / 1.01325e5, "ATMOSPHERE", 1.0, "FOOT_OF_H2O_CONVENTIONAL", 1000, loadErrors, conversionErrors);
    TestUnitConversion(249.1083 / 1.01325e5, "ATMOSPHERE", 1.0, "INCH_OF_H2O_AT_32_FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(2.49082e2 / 1.01325e5, "ATMOSPHERE", 1.0, "INCH_OF_H2O_AT_39_2_FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(2.4884e2 / 1.01325e5, "ATMOSPHERE", 1.0, "INCH_OF_H2O_AT_60_FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(3.38638e3 / 1.01325e5, "ATMOSPHERE", 1.0, "INCH_OF_HG_AT_32_FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(3.386389e3 / 1.01325e5, "ATMOSPHERE", 1.0, "INCH_OF_HG_CONVENTIONAL", 1000, loadErrors, conversionErrors);
    TestUnitConversion(3.37685e3 / 1.01325e5, "ATMOSPHERE", 1.0, "INCH_OF_HG_AT_60_FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(9.80665e4 / 1.01325e5, "ATMOSPHERE", 1.0, "KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED", 1000, loadErrors, conversionErrors);
    TestUnitConversion(((1 - 101.325 / 98.0665) * 9.80665e4) / 1.01325e5, "ATMOSPHERE", 1.0, "KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED_GAUGE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(9.80665 / 1.01325e5, "ATMOSPHERE", 1.0, "KILOGRAM_FORCE_PER_METRE_SQUARED", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1000 / 1.01325e5, "ATMOSPHERE", 1.0, "KILOPASCAL", 1000, loadErrors, conversionErrors);
    TestUnitConversion(((1 - 101.325) * 1000) / 1.01325e5, "ATMOSPHERE", 1.0, "KILOPASCAL_GAUGE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1000000 / 1.01325e5, "ATMOSPHERE", 1.0, "MEGAPASCAL", 1000, loadErrors, conversionErrors);
    TestUnitConversion(((1 - 101.325 / 1000) * 1000000) / 1.01325e5, "ATMOSPHERE", 1.0, "MEGAPASCAL_GAUGE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(9806.65 / 1.01325e5, "ATMOSPHERE", 1.0, "METRE_OF_H2O_CONVENTIONAL", 1000, loadErrors, conversionErrors);
    TestUnitConversion(9.80665 / 1.01325e5, "ATMOSPHERE", 1.0, "MILLIMETRE_OF_H2O_CONVENTIONAL", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0, "MILLIMETRE_OF_HG_AT_32_FAHRENHEIT", 1.33322e2 / 101325.0, "ATMOSPHERE", 1000, loadErrors, conversionErrors);  // KnowledgeDoor and hand calculation agree with actual value more than value from old system
    TestUnitConversion(1 / 1.01325e5, "ATMOSPHERE", 1.0, "NEWTON_PER_METRE_SQUARED", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1 / 1.01325e5, "ATMOSPHERE", 1.0, "PASCAL", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0, "POUND_FORCE_PER_FOOT_SQUARED", 47.88026 / 1.01325e5, "ATMOSPHERE", 100000000, loadErrors, conversionErrors);  // Uses NIST table conversion value for LBF/FT^2 to Pascal.  This has 7 significant digits so comparison must use reduced precision.
    TestUnitConversion(1.0, "POUND_FORCE_PER_INCH_SQUARED", 6.894757e3 / 1.01325e5, "ATMOSPHERE", 100000000, loadErrors, conversionErrors);  // Uses NIST table conversion value for LBF/IN^2 to Pascal.  This has 7 significant digits so comparison must use reduced precision.
    TestUnitConversion(1.0, "POUND_FORCE_PER_INCH_SQUARED_GAUGE", ((1 - 101.325 / 6.894757) * 6.894757e3) / 1.01325e5, "ATMOSPHERE", 10000000, loadErrors, conversionErrors);  // Uses NIST table conversion value for LBF/IN^2 to Pascal.  This has 7 significant digits so comparison must use reduced precision.
    TestUnitConversion(1.333224e2 / 1.01325e5, "ATMOSPHERE", 1.0, "TORR", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e2 / 1.01325e5, "ATMOSPHERE", 1.0, "MILLIBAR", 1000, loadErrors, conversionErrors);
    TestUnitConversion(100.0 / 1.01325e5, "ATMOSPHERE", 1.0, "HECTOPASCAL", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1000000.0 * 1.0e5 / 1.01325e5, "ATMOSPHERE", 1000000.0, "BAR", 1000, loadErrors, conversionErrors);
    TestUnitConversion(((1000000.0 - 1.01325) * 1.0e5) / 1.01325e5, "ATMOSPHERE", 1000000.0, "BAR_GAUGE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(100000 / 1.01325e5, "ATMOSPHERE", 1000000.0, "BARYE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1000000.0 * 2.989067e3 / 1.01325e5, "ATMOSPHERE", 1000000.0, "FOOT_OF_H2O_CONVENTIONAL", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1000000.0 * 249.1083 / 1.01325e5, "ATMOSPHERE", 1000000.0, "INCH_OF_H2O_AT_32_FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1000000.0 * 2.49082e2 / 1.01325e5, "ATMOSPHERE", 1000000.0, "INCH_OF_H2O_AT_39_2_FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1000000.0 * 2.4884e2 / 1.01325e5, "ATMOSPHERE", 1000000.0, "INCH_OF_H2O_AT_60_FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1000000.0 * 3.38638e3 / 1.01325e5, "ATMOSPHERE", 1000000.0, "INCH_OF_HG_AT_32_FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1000000.0 * 3.386389e3 / 1.01325e5, "ATMOSPHERE", 1000000.0, "INCH_OF_HG_CONVENTIONAL", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1000000.0 * 3.37685e3 / 1.01325e5, "ATMOSPHERE", 1000000.0, "INCH_OF_HG_AT_60_FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1000000.0 * 9.80665e4 / 1.01325e5, "ATMOSPHERE", 1000000.0, "KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED", 1000, loadErrors, conversionErrors);
    TestUnitConversion(((1000000 - 101.325 / 98.0665) * 9.80665e4) / 1.01325e5, "ATMOSPHERE", 1000000.0, "KILOGRAM_FORCE_PER_CENTIMETRE_SQUARED_GAUGE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1000000.0 * 9.80665 / 1.01325e5, "ATMOSPHERE", 1000000.0, "KILOGRAM_FORCE_PER_METRE_SQUARED", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1000000000 / 1.01325e5, "ATMOSPHERE", 1000000.0, "KILOPASCAL", 1000, loadErrors, conversionErrors);
    TestUnitConversion(((1000000 - 101.325) * 1000) / 1.01325e5, "ATMOSPHERE", 1000000.0, "KILOPASCAL_GAUGE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1000000000000 / 1.01325e5, "ATMOSPHERE", 1000000.0, "MEGAPASCAL", 1000, loadErrors, conversionErrors);
    TestUnitConversion(((1000000 - 101.325 / 1000) * 1000000) / 1.01325e5, "ATMOSPHERE", 1000000.0, "MEGAPASCAL_GAUGE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1000000.0 * 9806.65 / 1.01325e5, "ATMOSPHERE", 1000000.0, "METRE_OF_H2O_CONVENTIONAL", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1000000.0 * 9.80665 / 1.01325e5, "ATMOSPHERE", 1000000.0, "MILLIMETRE_OF_H2O_CONVENTIONAL", 1000, loadErrors, conversionErrors);
    TestUnitConversion((1000000.0 * 133.322) / 1.01325e5, "ATMOSPHERE", 1000000.0, "MILLIMETRE_OF_HG_AT_32_FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1000000 / 1.01325e5, "ATMOSPHERE", 1000000.0, "NEWTON_PER_METRE_SQUARED", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1000000 / 1.01325e5, "ATMOSPHERE", 1000000.0, "PASCAL", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1000000.0 * 1.333224e2 / 1.01325e5, "ATMOSPHERE", 1000000.0, "TORR", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e8 / 1.01325e5, "ATMOSPHERE", 1.0e6, "MILLIBAR", 1000, loadErrors, conversionErrors);
    TestUnitConversion(100.0e6 / 1.01325e5, "ATMOSPHERE", 1.0e6, "HECTOPASCAL", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0 / (1000.0 * 3600.0), "KILOWATT_HOUR_PER_METRE_CUBED", 1.0, "JOULE_PER_METRE_CUBED", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0 / 3600, "KILOWATT_HOUR_PER_METRE_CUBED", 1.0, "KILOJOULE_PER_METRE_CUBED", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0 / 3.6, "KILOWATT_HOUR_PER_METRE_CUBED", 1.0, "MEGAJOULE_PER_METRE_CUBED", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e6 / (1000 * 3600), "KILOWATT_HOUR_PER_METRE_CUBED", 1.0e6, "JOULE_PER_METRE_CUBED", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e6 / 3600, "KILOWATT_HOUR_PER_METRE_CUBED", 1.0e6, "KILOJOULE_PER_METRE_CUBED", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e6 / 3.6, "KILOWATT_HOUR_PER_METRE_CUBED", 1.0e6, "MEGAJOULE_PER_METRE_CUBED", 1000, loadErrors, conversionErrors);
    TestUnitConversion(3600 * 24, "GRAM_PER_DAY", 1.0, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(3.6 * 24, "KILOGRAM_PER_DAY", 1.0, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e6 * 3600 * 24, "MICROGRAM_PER_DAY", 1.0, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e6 * 3600, "MICROGRAM_PER_HOUR", 1.0, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e6 * 60, "MICROGRAM_PER_MINUTE", 1.0, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e3 * 3600 * 24, "MILLIGRAM_PER_DAY", 1.0, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e3 * 3600, "MILLIGRAM_PER_HOUR", 1.0, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e3 * 60, "MILLIGRAM_PER_MINUTE", 1.0, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(3600 * 24 * 1.0e6, "GRAM_PER_DAY", 1.0e6, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(3600 * 1.0e6, "GRAM_PER_HOUR", 1.0e6, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(60 * 1.0e6, "GRAM_PER_MINUTE", 1.0e6, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(3.6e6 * 24, "KILOGRAM_PER_DAY", 1.0e6, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e12 * 3600 * 24, "MICROGRAM_PER_DAY", 1.0e6, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e12 * 3600, "MICROGRAM_PER_HOUR", 1.0e6, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e12 * 60, "MICROGRAM_PER_MINUTE", 1.0e6, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e9 * 3600 * 24, "MILLIGRAM_PER_DAY", 1.0e6, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e9 * 3600, "MILLIGRAM_PER_HOUR", 1.0e6, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e9 * 60, "MILLIGRAM_PER_MINUTE", 1.0e6, "GRAM_PER_SECOND", 1000, loadErrors, conversionErrors);
    TestUnitConversion(5280.0 / 2, "FOOT_PER_MILE", 50.0, "PERCENT_SLOPE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0 / 33.0, "VERTICAL_PER_HORIZONTAL", 33.0, "ONE_OVER_SLOPE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0 / 7000.0, "KILOGRAM_PER_KILOGRAM", 1.0, "GRAIN_MASS_PER_POUND_MASS", 1000, loadErrors, conversionErrors);
    TestUnitConversion(5.0 / 9.0, "RECIPROCAL_DELTA_DEGREE_FAHRENHEIT", 1.0, "RECIPROCAL_DELTA_DEGREE_CELSIUS", 1000, loadErrors, conversionErrors);
    TestUnitConversion(5.0 / 9.0, "RECIPROCAL_DELTA_DEGREE_FAHRENHEIT", 1.0, "RECIPROCAL_DELTA_DEGREE_KELVIN", 1000, loadErrors, conversionErrors);
    TestUnitConversion(9.0 / 5.0, "RECIPROCAL_DELTA_DEGREE_CELSIUS", 1.0, "RECIPROCAL_DELTA_DEGREE_FAHRENHEIT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(9.0 / 5.0, "RECIPROCAL_DELTA_DEGREE_CELSIUS", 1.0, "RECIPROCAL_DELTA_DEGREE_RANKINE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0 / 0.3048, "ONE_PER_METRE", 1.0, "ONE_PER_FOOT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0 / 1000.0, "ONE_PER_FOOT", 1.0, "ONE_PER_THOUSAND_FOOT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0 / 5280.0, "ONE_PER_FOOT", 1.0, "ONE_PER_MILE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e6 / 0.3048, "ONE_PER_METRE", 1.0e6, "ONE_PER_FOOT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e6 / 1000.0, "ONE_PER_FOOT", 1.0e6, "ONE_PER_THOUSAND_FOOT", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e6 / 5280.0, "ONE_PER_FOOT", 1.0e6, "ONE_PER_MILE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0 / 3600.0, "HERTZ", 1.0, "ONE_PER_HOUR", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0 / 31536000.0, "HERTZ", 1.0, "ONE_PER_YEAR", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0 / 86400.0, "HERTZ", 1.0, "ONE_PER_DAY", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0 / 60.0, "HERTZ", 1.0, "ONE_PER_MINUTE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e6 / 3600.0, "HERTZ", 1.0e6, "ONE_PER_HOUR", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e6 / 31536000.0, "HERTZ", 1.0e6, "ONE_PER_YEAR", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e6 / 86400.0, "HERTZ", 1.0e6, "ONE_PER_DAY", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e6 / 60.0, "HERTZ", 1.0e6, "ONE_PER_MINUTE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0 / 0.00023884589662749597, "JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0, "BTU_PER_POUND_MASS_PER_DELTA_DEGREE_RANKINE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e6 / 0.00023884589662749597, "JOULE_PER_KILOGRAM_DELTA_DEGREE_KELVIN", 1.0e6, "BTU_PER_POUND_MASS_PER_DELTA_DEGREE_RANKINE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0 / 0.42992261392949271, "KILOJOULE_PER_KILOMOLE", 1.0, "BTU_PER_POUND_MOLE", 1000, loadErrors, conversionErrors);
    TestUnitConversion(1.0e6 / 0.42992261392949271, "KILOJOULE_PER_KILOMOLE", 1.0e6, "BTU_PER_POUND_MOLE", 1000, loadErrors, conversionErrors);
    
    Utf8String loadErrorString("The following units were not found:\n");
    for (auto const& val : loadErrors)
        loadErrorString.append(val + "\n");
    
    Utf8String conversionErrorString("Failed to convert between the following units:\n");
    for (auto const& val : conversionErrors)
        conversionErrorString.append(val + "\n");
    
    if (loadErrors.size() > 0)
        PERFORMANCELOG.error(loadErrorString.c_str());
    //EXPECT_EQ(0, loadErrors.size()) << loadErrorString;
    EXPECT_EQ(0, conversionErrors.size()) << conversionErrorString;
    }

void UnitsTests::TestConversionsLoadedFromCvsFile(Utf8CP fileName)
    {
    bvector<Utf8String> loadErrors;
    bvector<Utf8String> conversionErrors;

    int numberConversions = 0;
    int numberWhereUnitsFound = 0;
    auto lineProcessor = [&loadErrors, &conversionErrors, &numberConversions, &numberWhereUnitsFound] (bvector<Utf8String>& tokens)
        {
        ++numberConversions;
        //passing 10000 to tolerance instead of the csv value
        if (TestUnitConversion(GetDouble(tokens[2]), tokens[3].c_str(), GetDouble(tokens[0]), tokens[1].c_str(), GetInt(tokens[4].c_str()), loadErrors, conversionErrors))
            ++numberWhereUnitsFound;
        };

    WString fileNameLong(fileName, BentleyCharEncoding::Utf8);
    ReadConversionCsvFile(fileNameLong.c_str(), lineProcessor);

    Utf8PrintfString loadErrorString("%s - Attempted %d conversions, error loading :\n", fileName, numberConversions);

    for (auto const& val : loadErrors)
        loadErrorString.append(val + "\n");

    if (loadErrors.size() > 0)
        PERFORMANCELOG.error(loadErrorString.c_str());
    //EXPECT_EQ(0, loadErrors.size()) << loadErrorString;

    Utf8PrintfString conversionErrorString("%s - Total number of conversions %d, units found for %d, %d passed, %d failed, %d skipped because of %d missing units, error Converting :\n",
                                            fileName, numberConversions, numberWhereUnitsFound, numberWhereUnitsFound - conversionErrors.size(), conversionErrors.size(), numberConversions - numberWhereUnitsFound, loadErrors.size());

    for (auto const& val : conversionErrors)
        conversionErrorString.append(val + "\n");

    EXPECT_EQ(0, conversionErrors.size()) << conversionErrorString;
    }

TEST_F(UnitsTests, UnitsConversion_CompareToRawOutputFromOldSystem)
    {
    TestConversionsLoadedFromCvsFile("ConversionsBetweenAllOldUnits.csv");
    }

TEST_F(UnitsTests, UnitsConversion)
    {
    TestConversionsLoadedFromCvsFile("unitcomparisondata.csv");
    }

void GetUnitsByName(UnitRegistry& hub, bvector<Utf8String>& unitNames)
    {
    for (auto const& unitName : unitNames)
        {
        auto unit = hub.LookupUnit(unitName.c_str());
        ASSERT_TRUE(unit != nullptr) << "Failed to get unit: " << unitName;
        }
    }

//void ReadFile(Utf8CP path, std::function<void(Utf8CP)> lineProcessor)
//    {
//    std::ifstream ifs(path, std::ifstream::in);
//    std::string line;
//
//    while (std::getline(ifs, line))
//        {
//        lineProcessor(line.c_str());
//        }
//    }
//
//TEST_F(UnitsTests, MergeListsOfUnits)
//    {
//    bvector<Utf8String> unitsList;
//    auto merger = [&unitsList] (Utf8CP token)
//        {
//        auto it = find(unitsList.begin(), unitsList.end(), token);
//        if (it == unitsList.end())
//            unitsList.push_back(token);
//        };
//
//    ReadFile("C:\\Source\\GraphiteTestData\\Second pass units lists\\units.txt.bak", merger);
//    ReadFile("C:\\Source\\GraphiteTestData\\Second pass units lists\\allowableUnits.txt", merger);
//    ReadFile("C:\\Source\\DgnDb0601Dev_1\\src\\Units\\test\\ConversionData\\NeededUnits.csv", merger);
//
//    sort(unitsList.begin(), unitsList.end());
//
//    ofstream fileStream("C:\\NeededUnits.csv", ofstream::out);
//    for (auto const& unit : unitsList)
//        fileStream << unit.c_str() << endl;
//    fileStream.close();
//    }

TEST_F(UnitsTests, TestEveryUnitIsAddedToItsPhenomenon)
    {
    UnitRegistry& hub = UnitRegistry::Instance();
    bvector<UnitCP> allUnits;
    hub.AllUnits(allUnits);
    for (auto const& unit : allUnits)
        {
        PhenomenonCP unitPhenomenon = unit->GetPhenomenon();
        ASSERT_NE(nullptr, unitPhenomenon) << "Unit " << unit->GetName() << " does not have phenomenon";
        auto it = find_if(unitPhenomenon->GetUnits().begin(), unitPhenomenon->GetUnits().end(),
                       [&unit] (UnitCP unitInPhenomenon) { return 0 == strcmp(unitInPhenomenon->GetName(), unit->GetName()); });
        ASSERT_NE(unitPhenomenon->GetUnits().end(), it) << "Unit " << unit->GetName() << " is not registered with it's phenomenon";
        }
    }

void TestUnitAndConstantsExist(const bvector<Utf8String>& unitNames, Utf8CP unitName, Utf8CP atorName)
    {
    UnitRegistry& reg = UnitRegistry::Instance();
    for (auto const& subUnitName : unitNames)
        {
        if (subUnitName.StartsWith("["))
            {
            Utf8String constantName = Utf8String(subUnitName.begin() + 1, subUnitName.end() - 1);
            UnitCP constant = reg.LookupConstant(constantName.c_str());
            ASSERT_NE(nullptr, constant) << "Could not find constant '" << constantName << "' for " << atorName << " of unit: " << unitName;
            }
        else
            {
            UnitCP subUnit = reg.LookupUnit(subUnitName.c_str());
            ASSERT_NE(nullptr, subUnit) << "Could not find Sub unit '" << subUnitName << "' for " << atorName << " of unit: " << unitName;
            }
        }
    }

TEST_F(UnitsTests, AllUnitsNeededForFirstReleaseExist)
    {
    bvector<Utf8String> missingUnits;
    bvector<Utf8String> foundUnits;
    auto lineProcessor = [&missingUnits, &foundUnits] (bvector<Utf8String>& lines)
        {
        for (auto const& unitName : lines)
            {
            UnitCP unit = UnitRegistry::Instance().LookupUnit(unitName.c_str());
            if (nullptr == unit)
                missingUnits.push_back(unitName);
            else
                foundUnits.push_back(unitName);
            }
        };

    ReadConversionCsvFile(L"NeededUnits.csv", lineProcessor);

    if (missingUnits.size() != 0)
        {
        Utf8String missingString = BeStringUtilities::Join(missingUnits, ", ");
        EXPECT_EQ(0, missingUnits.size()) << "Some needed units were not found\n" << missingString.c_str();
        }
    ASSERT_NE(0, foundUnits.size()) << "No units were found";
    }

void WriteLine(BeFile& file, Utf8CP line = nullptr)
    {
    Utf8String finalLine;
    if (Utf8String::IsNullOrEmpty(line))
        {
        finalLine.assign("\r\n");
        }
    else
        {
        finalLine.Sprintf("%s\r\n", line);
        }

    uint32_t bytesToWrite = static_cast<uint32_t>(finalLine.SizeInBytes() - 1); //not safe, but our line will not exceed 32bits.
    uint32_t bytesWritten;
    EXPECT_EQ(file.Write(&bytesWritten, finalLine.c_str(), bytesToWrite), BeFileStatus::Success);
    }

TEST_F(UnitsTests, PrintOutAllUnitsGroupedByPhenonmenon)
    {
    Utf8String fileName = UnitsTestFixture::GetOutputDataPath(L"AllUnitsByPhenomenon.csv");
    BeFile file;
    EXPECT_EQ(file.Create(fileName, true), BeFileStatus::Success);
    EXPECT_EQ(file.Open(fileName, BeFileAccess::Write), BeFileStatus::Success);

    bvector<PhenomenonCP> phenomena;
    UnitRegistry::Instance().AllPhenomena(phenomena);

    for (auto const& phenomenon : phenomena)
        {
        Utf8PrintfString line("Units for Phenomenon:,%s", phenomenon->GetName());
        WriteLine(file, line.c_str());

        line.Sprintf("Phenomenon Definition:,%s", phenomenon->GetDefinition());
        WriteLine(file, line.c_str());

        line.Sprintf("Phenomenon Dimension:,%s", phenomenon->GetPhenomenonDimension().c_str());
        WriteLine(file, line.c_str());

        WriteLine(file, "UnitName,UnitDefinition,UnitDimension");
        
        for (auto const& unit : phenomenon->GetUnits())
            {
            line.Sprintf("%s,%s,%s", unit->GetName(), unit->GetDefinition(), unit->GetUnitDimension().c_str());
            WriteLine(file, line.c_str());
            }

        WriteLine(file);
        }

    file.Close();
    }

struct UnitsPerformanceTests : UnitsTests {};

TEST_F(UnitsPerformanceTests, InitUnitsHub)
    {
    UnitRegistry::Clear();
    StopWatch timer("Init Units Hub", false);
    timer.Start();
    UnitRegistry& hub = UnitRegistry::Instance();
    timer.Stop();
    bvector<Utf8String> allUnitNames;
    hub.AllUnitNames(allUnitNames, false);

    PERFORMANCELOG.errorv("Time to load Units Hub with %lu units: %.17g", allUnitNames.size(), timer.GetElapsedSeconds());
    }

TEST_F(UnitsPerformanceTests, GetEveryUnitByName)
    {
    StopWatch timer("Get every unit by name", false);
    UnitRegistry::Clear();
    bvector<Utf8String> allUnitNames;
    timer.Start();
    UnitRegistry::Instance().AllUnitNames(allUnitNames, false);
    timer.Stop();
    PERFORMANCELOG.errorv("Time to get all %lu primary unit names: %.17g", allUnitNames.size(), timer.GetElapsedSeconds());
    
    timer.Start();
    GetUnitsByName(UnitRegistry::Instance(), allUnitNames);
    timer.Stop();
    PERFORMANCELOG.errorv("Time to get %lu units by name: %.17g", allUnitNames.size(), timer.GetElapsedSeconds());

    UnitRegistry::Clear();
    allUnitNames.clear();
    timer.Start();
    UnitRegistry::Instance().AllUnitNames(allUnitNames, true);
    timer.Stop();
    PERFORMANCELOG.errorv("Time to get all %lu primary unit names and synonyms: %.17g", allUnitNames.size(), timer.GetElapsedSeconds());

    timer.Start();
    GetUnitsByName(UnitRegistry::Instance(), allUnitNames);
    timer.Stop();
    PERFORMANCELOG.errorv("Time to get %lu units by name including using synonyms: %.17g", allUnitNames.size(), timer.GetElapsedSeconds());
    }

TEST_F(UnitsPerformanceTests, GenerateEveryConversionValue)
    {
    StopWatch timer("Evaluate every unit", false);
    UnitRegistry::Clear();
    UnitRegistry& hub = UnitRegistry::Instance();
    bvector<PhenomenonCP> allPhenomena;
    hub.AllPhenomena(allPhenomena);


    int numConversions = 0;
    timer.Start();
    for (auto const& phenomenon : allPhenomena)
        {
        if (!phenomenon->HasUnits())
            continue;
        UnitCP firstUnit = *phenomenon->GetUnits().begin();
        for (auto const& unit : phenomenon->GetUnits())
            {
            double conversion = firstUnit->Convert(42, unit);
            ASSERT_FALSE(std::isnan(conversion)) << "Generated conversion factor is invalid from " << firstUnit->GetName() << " to " << unit->GetName();
            ++numConversions;
            }
        }
    timer.Stop();
    PERFORMANCELOG.errorv("Time to Generate %d conversion factors: %.17g", numConversions, timer.GetElapsedSeconds());
    }

END_UNITS_UNITTESTS_NAMESPACE
