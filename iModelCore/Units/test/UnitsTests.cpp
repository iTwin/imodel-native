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

    static void TestUnitConversion(double fromVal, Utf8CP fromUnitName, double expectedVal, Utf8CP targetUnitName, double tolerance, bvector<Utf8String>& loadErrors, bvector<Utf8String>& conversionErrors);

    static Utf8String ParseUOM(Utf8CP unitName, bset<Utf8String>& notMapped)
        {
        UnitPtr uom = LocateUOM(unitName);
        if (uom.IsValid())
            return uom->GetName();

        notMapped.insert(unitName);
        return "NULL";
        }

    static UnitPtr LocateUOM(Utf8CP unitName)
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
void UnitsTests::TestUnitConversion (double fromVal, Utf8CP fromUnitName, double expectedVal, Utf8CP targetUnitName, double tolerance, bvector<Utf8String>& loadErrors, bvector<Utf8String>& conversionErrors)
    {
    //if either units are not in the library conversion is not possible
    //UnitsMapping test checks if all units are there and fails when a unit is not found
    if (!LocateUOM(fromUnitName).IsValid() || !LocateUOM(targetUnitName).IsValid())
        return;

    //QuantityP q = SimpleQuantity(fromVal, fromUnitName);
    //if (nullptr == q)
    //    {
    //    Utf8PrintfString error("%s with Value %lf", fromUnitName, fromVal);
    //    loadErrors.push_back(error);
    //    return;
    //    }

    //double convertedVal = q->Value(targetUnitName);
    //if (fabs(convertedVal - expectedVal) > tolerance)
    //    {
    //    Utf8PrintfString formattedText("Conversion from %s to %s. Input: %lf Output: %lf Expected: %lf Tolerance: %.9f", fromUnitName, targetUnitName, fromVal, convertedVal, expectedVal, tolerance);
    //    conversionErrors.push_back(formattedText);
    //    }
    //EXPECT_NEAR(expectedVal, convertedVal, tolerance)<<  "Conversion from "<< fromUnitName << " to " << targetUnitName <<". Input : " << fromVal << ", Output : " << convertedVal << ", ExpectedOutput : " << expectedVal << "Tolerance : " << tolerance<< "\n";
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

TEST_F(UnitsTests, UnitsConversion)
    {
    bvector<Utf8String> loadErrors;
    bvector<Utf8String> conversionErrors;

    auto lineProcessor = [&loadErrors, &conversionErrors](bvector<Utf8String>& tokens)
        {
        //passing 1.0e-6 to tolerance instead of the csv value
        TestUnitConversion(GetDouble(tokens[0]), tokens[1].c_str(), GetDouble(tokens[2]), tokens[3].c_str(), 1.0e-6, loadErrors, conversionErrors);
        };

    ReadConversionCsvFile(L"unitcomparisondata.csv", lineProcessor);

    Utf8String loadErrorString = "Error loading :\n";

    for (auto const& val : loadErrors)
        loadErrorString.append(val + "\n");

    EXPECT_EQ(0, loadErrors.size()) << loadErrorString;

    Utf8String conversionErrorString = "Error Converting :\n";

    for (auto const& val : conversionErrors)
        conversionErrorString.append(val + "\n");

    EXPECT_EQ(0, conversionErrors.size()) << conversionErrorString;
    
    }

struct UnitsPerformanceTests : UnitsTests {};

void GetAllUnitNames(UnitRegistry& hub, bvector<Utf8CP>& allUnitNames, bool includeSynonyms)
    {
    for (auto const& unitNameValue : hub.AllUnits())
        {
        allUnitNames.push_back(unitNameValue.first.c_str());
        // TODO: Synonyms?
        }
    }

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

void GetUnitsByName(UnitRegistry& hub, bvector<Utf8CP>& unitNames)
    {
    for (auto const& unitName : unitNames)
        {
        auto unit = hub.LookupUnit(unitName);
        ASSERT_TRUE(unit.IsValid()) << "Failed to get unit: " << unitName;
        }
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
    //StopWatch timer("Evaluate every unit", false);
    //UnitRegistry::Clear();
    //UnitRegistry& hub = UnitRegistry::Instance();
    //bvector<UnitP> allUnits;
    //for (auto const& phenomenon : hub->PhenomList())
    //    {
    //    for (auto const& unit : phenomenon->Units())
    //        {
    //        allUnits.push_back(unit);
    //        }
    //    }

    //timer.Start();
    //for (auto const& unit : allUnits)
    //    {
    //    ASSERT_FALSE(unit->IsEvaled()) << "Unit already evaluated: " << unit->GetName();
    //    unit->Evaluate();
    //    }
    //timer.Stop();
    //PERFORMANCELOG.errorv("Time to evaluate %lu units: %lf", allUnits.size(), timer.GetElapsedSeconds());
    }

END_UNITS_UNITTESTS_NAMESPACE
