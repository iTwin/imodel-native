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

    static void TestUnitConversion(double fromVal, Utf8CP fromUnitName, double expectedVal, Utf8CP targetUnitName, double tolerance, 
                                   bvector<Utf8String>& loadErrors, bvector<Utf8String>& conversionErrors, bool showDetailLogs = false);

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

    void GetMapping(WCharCP file, bmap<Utf8String, Utf8String>& unitNameMap, bset<Utf8String>& notMapped)
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

    void ReadConversionCsvFile(WCharCP file, CSVLineProcessor lineProcessor)
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
    };

/*---------------------------------------------------------------------------------**//**
// @bsiclass                                     Basanta.Kharel                 12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void UnitsTests::TestUnitConversion (double fromVal, Utf8CP fromUnitName, double expectedVal, Utf8CP targetUnitName, double tolerance, 
                                     bvector<Utf8String>& loadErrors, bvector<Utf8String>& conversionErrors, bool showDetailLogs)
    {
    //if either units are not in the library conversion is not possible
    //UnitsMapping test checks if all units are there and fails when a unit is not found
    UnitCP fromUnit = LocateUOM(fromUnitName);
    UnitCP targetUnit = LocateUOM(targetUnitName);
    if (nullptr == fromUnit || nullptr == targetUnit)
        {
        Utf8PrintfString loadError("Could not convert from %s to %s because could not load one of the units", fromUnitName, targetUnitName);
        loadErrors.push_back(loadError);
        return;
        }

    // Bad
    if (showDetailLogs)
        {
        NativeLogging::LoggingConfig::SetSeverity("UnitsNative", NativeLogging::SEVERITY::LOG_TRACE);
        NativeLogging::LoggingConfig::SetSeverity("Performance", NativeLogging::SEVERITY::LOG_TRACE);
        }

    PERFORMANCELOG.debugv("About to try to convert from %s to %s", fromUnit->GetName(), targetUnit->GetName());
    double conversionFactor = fromUnit->GetConversionTo(targetUnit);
    double convertedVal = conversionFactor * fromVal;

    //QuantityP q = SimpleQuantity(fromVal, fromUnitName);
    //if (nullptr == q)
    //    {
    //    Utf8PrintfString error("%s with Value %lf", fromUnitName, fromVal);
    //    loadErrors.push_back(error);
    //    return;
    //    }

    //double convertedVal = q->Value(targetUnitName);
    PERFORMANCELOG.debugv("Converted %s to %s.  Expected: %lf  Actual: %lf", fromUnit->GetName(), targetUnit->GetName(), expectedVal, convertedVal);
    if (fabs(convertedVal - expectedVal) > tolerance)
        {
        Utf8PrintfString formattedText("Conversion from %s to %s. Input: %lf Output: %lf Expected: %lf Tolerance: %.9f", fromUnitName, targetUnitName, fromVal, convertedVal, expectedVal, tolerance);
        conversionErrors.push_back(formattedText);
        }
    ASSERT_FALSE(std::isnan(convertedVal) || !std::isfinite(convertedVal)) << "Conversion from " << fromUnitName << " to " << targetUnitName << " resulted in an invalid number";
    EXPECT_NEAR(expectedVal, convertedVal, tolerance)<<  "Conversion from "<< fromUnitName << " to " << targetUnitName <<". Input : " << fromVal << ", Output : " << convertedVal << ", ExpectedOutput : " << expectedVal << "Tolerance : " << tolerance<< "\n";

    if (showDetailLogs)
        {
        NativeLogging::LoggingConfig::SetSeverity("UnitsNative", NativeLogging::SEVERITY::LOG_ERROR);
        NativeLogging::LoggingConfig::SetSeverity("Performance", NativeLogging::SEVERITY::LOG_ERROR);
        }
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
    EXPECT_EQ (0, notMapped.size() ) << guess;
    }

TEST_F(UnitsTests, TestBasiConversion)
    {
    bvector<Utf8String> loadErrors;
    bvector<Utf8String> conversionErrors;
    TestUnitConversion(10, "FT", 3048, "MM", 1.0e-8, loadErrors, conversionErrors);
    TestUnitConversion(1, "GALLON", 3.785411784, "LITRE", 1.0e-8, loadErrors, conversionErrors);
    TestUnitConversion(100000, "FOOT_SQUARED", 100, "THOUSAND_FOOT_SQUARED", 1.0e-8, loadErrors, conversionErrors);
    TestUnitConversion(836127.36, "MILLIMETRE_SQUARED", 1.0, "YARD_SQUARED", 1.0e-8, loadErrors, conversionErrors);
    TestUnitConversion(3.17097919837647e-7, "YEAR", 10000.0, "MILLISECOND", 1.0e-8, loadErrors, conversionErrors);
    TestUnitConversion(1000000.0, "POUND", 1000000.0, "POUND", 1.0e-8, loadErrors, conversionErrors);
    TestUnitConversion(2204622621.84878, "POUND", 1000000.0, "MEGAGRAM", 1.0e-5, loadErrors, conversionErrors);
    TestUnitConversion(1.66666666666667e-02, "DEGREE", 1.0, "ANGLE_MINUTE", 1.0e-8, loadErrors, conversionErrors);
    TestUnitConversion(1.852e11, "CENTIMETRE_PER_HOUR", 1.0e6, "KNOT_INTERNATIONAL", 1.0e-4, loadErrors, conversionErrors);
    TestUnitConversion(1.65409011373578e-3, "FOOT_CUBED_PER_ACRE_PER_SECOND", 1.0e6, "LITRE_PER_KILOMETRE_SQUARED_PER_DAY", 1.0e-8, loadErrors, conversionErrors);
    TestUnitConversion(2.816538995808e13, "GALLON_PER_DAY_PER_PERSON", 1234e6, "LITRE_PER_SECOND_PER_PERSON", 1.0e-8, loadErrors, conversionErrors);
    TestUnitConversion(4.4482216152605e5, "DYNE", 1.0, "POUND_FORCE", 1.0e-8, loadErrors, conversionErrors);
    TestUnitConversion(2.8316846592e3, "KILONEWTON_PER_FOOT_CUBED", 1.0e8, "NEWTON_PER_METRE_CUBED", 1.0e-8, loadErrors, conversionErrors);
    TestUnitConversion(1.0e9, "NEWTON_PER_METRE", 1.0e6, "NEWTON_PER_MILLIMETRE", 1.0e-6, loadErrors, conversionErrors);
    TestUnitConversion(3.43774677078493e9, "DEGREE_PER_HOUR", 1.0e6, "RADIAN_PER_MINUTE", 1.0e-5, loadErrors, conversionErrors);
    TestUnitConversion(2.65258238486492e3, "CYCLE_PER_SECOND", 1.0e6, "RADIAN_PER_MINUTE", 1.0e-8, loadErrors, conversionErrors);
    TestUnitConversion(8.92179121619709e5, "POUND_PER_ACRE", 1.0e6, "KILOGRAM_PER_HECTARE", 1.0e-8, loadErrors, conversionErrors);
    TestUnitConversion(8.92179121619701e6, "POUND_PER_ACRE", 1.0e6, "GRAM_PER_METRE_SQUARED", 1.0e-7, loadErrors, conversionErrors);
    TestUnitConversion(8.54292974552351e7, "FOOT_POUNDAL", 1.0, "KILOWATT_HOUR", 1.0e-6, loadErrors, conversionErrors);
    TestUnitConversion(2.37303604042319e7, "FOOT_POUNDAL", 1.0, "MEGAJOULE", 1e-7, loadErrors, conversionErrors);
    }

TEST_F(UnitsTests, UnitsConversion)
    {
    bvector<Utf8String> loadErrors;
    bvector<Utf8String> conversionErrors;

    int numberAttempted = 0;
    auto lineProcessor = [&loadErrors, &conversionErrors, &numberAttempted](bvector<Utf8String>& tokens)
        {
        ++numberAttempted;
        //passing 1.0e-6 to tolerance instead of the csv value
        TestUnitConversion(GetDouble(tokens[0]), tokens[1].c_str(), GetDouble(tokens[2]), tokens[3].c_str(), 1.0e-6, loadErrors, conversionErrors);
        };

    ReadConversionCsvFile(L"unitcomparisondata.csv", lineProcessor);

    Utf8PrintfString loadErrorString ("Attempted to load %d, error loading :\n", numberAttempted);

    for (auto const& val : loadErrors)
        loadErrorString.append(val + "\n");

    EXPECT_EQ(0, loadErrors.size()) << loadErrorString;

    Utf8PrintfString conversionErrorString("Attempted to convert %d, %d failed, %d skipped because of missing units, error Converting :\n", numberAttempted - loadErrors.size(), conversionErrors.size(), loadErrors.size());

    for (auto const& val : conversionErrors)
        conversionErrorString.append(val + "\n");

    EXPECT_EQ(0, conversionErrors.size()) << conversionErrorString;
    }

void GetAllUnitNames(UnitRegistry& hub, bvector<Utf8CP>& allUnitNames, bool includeSynonyms)
    {
    for (auto const& unitNameValue : hub.AllUnits())
        {
        allUnitNames.push_back(unitNameValue.first.c_str());
        }
    }

void GetUnitsByName(UnitRegistry& hub, bvector<Utf8CP>& unitNames)
    {
    for (auto const& unitName : unitNames)
        {
        auto unit = hub.LookupUnit(unitName);
        ASSERT_TRUE(unit != nullptr) << "Failed to get unit: " << unitName;
        }
    }

void GetAllUnits(UnitRegistry& hub, bvector<UnitCP>& allUnits)
    {
    bvector<Utf8CP> allUnitNames;
    GetAllUnitNames(hub, allUnitNames, false);
    for (auto const& unitName : allUnitNames)
        {
        auto unit = hub.LookupUnit(unitName);
        ASSERT_TRUE(unit != nullptr) << "Failed to get unit: " << unitName;
        allUnits.push_back(unit);
        }
    }
//
//TEST_F(UnitsTests, UnitExpressions)
//    {
//    bvector<UnitCP> allUnits;
//    GetAllUnits(UnitRegistry::Instance(), allUnits);
//    for (auto const& unit : allUnits)
//        {
//        PERFORMANCELOG.error(unit->GetName());
//        PERFORMANCELOG.errorv("Numerator:   %s", BeStringUtilities::Join(unit->Numerator(), "*").c_str());
//        PERFORMANCELOG.errorv("Denominator: %s", BeStringUtilities::Join(unit->Denominator(), "*").c_str());
//        }
//    }

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

//TEST_F(UnitsTests, EveryUnitIsMadeUpOfRegisteredUnits)
//    {
//    UnitRegistry& reg = UnitRegistry::Instance();
//    bvector<UnitCP> allUnits;
//    GetAllUnits(reg, allUnits);
//    for (auto const& unit : allUnits)
//        {
//        TestUnitAndConstantsExist(unit->Numerator(), unit->GetName(), "numerator");
//        TestUnitAndConstantsExist(unit->Denominator(), unit->GetName(), "denominator");
//        }
//    }

struct UnitsPerformanceTests : UnitsTests {};

TEST_F(UnitsPerformanceTests, InitUnitsHub)
    {
    UnitRegistry::Clear();
    StopWatch timer("Init Units Hub", false);
    timer.Start();
    UnitRegistry& hub = UnitRegistry::Instance();
    timer.Stop();
    bvector<Utf8CP> allUnitNames;
    GetAllUnitNames(hub, allUnitNames, false);

    PERFORMANCELOG.errorv("Time to load Units Hub with %lu units: %lf", allUnitNames.size(), timer.GetElapsedSeconds());
    }

TEST_F(UnitsPerformanceTests, GetEveryUnitByName)
    {
    StopWatch timer("Get every unit by name", false);
    UnitRegistry::Clear();
    bvector<Utf8CP> allUnitNames;
    timer.Start();
    GetAllUnitNames(UnitRegistry::Instance(), allUnitNames, false);
    timer.Stop();
    PERFORMANCELOG.errorv("Time to get all %lu primary unit names: %lf", allUnitNames.size(), timer.GetElapsedSeconds());
    
    timer.Start();
    GetUnitsByName(UnitRegistry::Instance(), allUnitNames);
    timer.Stop();
    PERFORMANCELOG.errorv("Time to get %lu units by name: %lf", allUnitNames.size(), timer.GetElapsedSeconds());

    UnitRegistry::Clear();
    allUnitNames.clear();
    timer.Start();
    GetAllUnitNames(UnitRegistry::Instance(), allUnitNames, true);
    timer.Stop();
    PERFORMANCELOG.errorv("Time to get all %lu primary unit names and synonyms: %lf", allUnitNames.size(), timer.GetElapsedSeconds());

    timer.Start();
    GetUnitsByName(UnitRegistry::Instance(), allUnitNames);
    timer.Stop();
    PERFORMANCELOG.errorv("Time to get %lu units by name including using synonyms: %lf", allUnitNames.size(), timer.GetElapsedSeconds());
    }

TEST_F(UnitsPerformanceTests, GenerateEveryConversionValue)
    {
    StopWatch timer("Evaluate every unit", false);
    UnitRegistry::Clear();
    UnitRegistry& hub = UnitRegistry::Instance();
    bvector<UnitCP> allUnits;
    GetAllUnits(hub, allUnits);
    bmap<Utf8CP, bvector<UnitCP>*> unitsByPhenomenon;
    for (auto const& unit : allUnits)
        {
        bvector<UnitCP>* unitsInPhenomon;
        auto it = unitsByPhenomenon.find(unit->GetPhenomenon());
        if (it == unitsByPhenomenon.end())
            {
            unitsInPhenomon = new bvector<UnitCP>();
            unitsByPhenomenon.insert(bpair<Utf8CP, bvector<UnitCP>*> (unit->GetPhenomenon(), unitsInPhenomon));
            unitsInPhenomon->push_back(unit);
            }
        else
            {
            (*it).second->push_back(unit);
            }
        }
    int numConversions = 0;
    timer.Start();
    for (auto const& phenomenon : unitsByPhenomenon)
        {
        UnitCP firstUnit = *phenomenon.second->begin();
        for (auto const& unit : *phenomenon.second)
            {
            double conversion = firstUnit->GetConversionTo(unit);
            ASSERT_FALSE(std::isnan(conversion)) << "Generated conversion factor is invalid from " << firstUnit->GetName() << " to " << unit->GetName();
            ++numConversions;
            }
        }
    timer.Stop();
    PERFORMANCELOG.errorv("Time to Generate %d conversion factors: %lf", numConversions, timer.GetElapsedSeconds());
    }

END_UNITS_UNITTESTS_NAMESPACE
