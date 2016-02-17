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
        UnitP uom = LocateUOM(unitName);
        if (nullptr != uom)
            return uom->GetName();

        notMapped.insert(unitName);
        return "NULL";
        }

    static UnitP LocateUOM(Utf8CP unitName)
        {
        QuantityHubP h = QuantityHub::Get();
        return h->GetUnit(unitName);
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
    if (nullptr == LocateUOM(fromUnitName) || nullptr == LocateUOM(targetUnitName))
        return;

    QuantityP q = SimpleQuantity(fromVal, fromUnitName);
    if (nullptr == q)
        {
        Utf8PrintfString error("%s with Value %lf", fromUnitName, fromVal);
        loadErrors.push_back(error);
        return;
        }

    double convertedVal = q->Value(targetUnitName);
    if (fabs(convertedVal - expectedVal) > tolerance)
        {
        Utf8PrintfString formattedText("Conversion from %s to %s. Input: %lf Output: %lf Expected: %lf Tolerance: %.9f", fromUnitName, targetUnitName, fromVal, convertedVal, expectedVal, tolerance);
        conversionErrors.push_back(formattedText);
        }
    //EXPECT_NEAR(expectedVal, convertedVal, tolerance)<<  "Conversion from "<< fromUnitName << " to " << targetUnitName <<". Input : " << fromVal << ", Output : " << convertedVal << ", ExpectedOutput : " << expectedVal << "Tolerance : " << tolerance<< "\n";
    }
//---------------------------------------------------------------------------------------
// @bsiclass                                     Basanta.Kharel                 12/2015
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(UnitsTests, Quant)
    {
    //char* dimens = "A(2) * T(-1) * [LightSpeed] * M(-3)  * R(7)"; //initialized but not used
    //USING_NAMESPACE_BENTLEY
    Utf8String debugOutput = UnitsTestFixture::GetOutputDataPath(L"QuantDebug.txt");
    BISQOpenLogFile(debugOutput.c_str());
    BISQDebugMessage("Quantity Service", true);
    BISQDebugMessage("Base Phenom List:", true);
    QuantityHub::Get()->PrintBasePhenoms();

    // SortPhenomena();
    QuantityHub::Get()->SetAllUnitIndex();

    BISQDebugMessage("Phenom List:", true);
    BISQPrintPhenomList();
    BISQDebugMessage("\nSystem List:", true);
    BISQPrintSystemList();
    BISQDebugMessage("\nConst List:", true);
    BISQPrintAllConst();
    BISQDebugMessage("\nUOM List:", true);
    BISQPrintAllUOM();


    BISQDebugMessage("\n\n\nIncomplete Phenomena:", true);
    BISQPrintIncompletePhenom();

    BISQDebugMessage("\n\n\nAccumulated Errors:", true);
    ReportProblems();

#define CaseNum 7
    Utf8CP expr[CaseNum]{ "FT", "[C] * YR", "LIGHT_YEAR", "N*MM(-1)", "N*CM", "BTU*HR(-1)*IN*FT(-2)*FAHRENHEIT(-1)", "BTU" };
    expr;
    Utf8CP pName[CaseNum]{ BISPH_LENGTH, BISPH_LENGTH, BISPH_LENGTH, BISPH_LINLOAD, BISPH_TORQUE, BISPH_HEATTRANS, BISPH_WORK };
    pName;
    //    PhenomenonP ph;
    //    BISQUomLinkCollectorP lc;
    //    Utf8P asis;

    //for (int i = 0; i < 3; i++)
    //    {
    //    ph = GetPhenomenon(pName[i], false);
    //    lc = new BISQUomLinkCollector(expr[i], 1.0, 0.0);
    //    BISQDebugLine("\nprocessing %s  PhenomFormula: %s", expr[i], ph->GetNormDimension(false));
    //    lc->DebugPrint("FirstPass ");
    //    lc->ConvertToPrimary();
    //    lc->DebugPrint("After ConvPrim ");
    //    //BISQDebugLine("Curr Sig = %s", lc->CurrentSignature(false));
    //    //BISQDebugLine("Prim Sig = %s", lc->CurrentSignature(true));
    //    lc->Compress();
    //    lc->DebugPrint("After Compress ");
    //    //asis = BISQNewText(lc->CurrentSignature(false));
    //    //BISQDebugLine("Primary after compression %s  Asis: %s", lc->CurrentSignature(true), asis);
    //    delete lc;
    //   // delete asis;
    //    }

    //#define NamNum 17
    //    Utf8P unam[NamNum] {"CELSIUS", "FAHRENHEIT", "MG", "MILLION_GALLON", "LITRE", "GALLON","LIGHT_YEAR", "MM", "FT", "MILE", "INCH", "G", "LBM", "SLUG", "NAUT_MILE", "GRAIN", "US_SURVEY_YARD" };
    //
    //
    //    BISQDebugLine("\nTesting Evaluation");
    //    for (int i = 0; i < NamNum; i++)
    //        {
    //        BISQUomP uom = GetUnit(unam[i]);
    //        BISQDebugLine("\n=============== UOM: %s", uom->GetName());
    //        if (BISQNotNull(uom))
    //          uom->Evaluate();
    //        }
    //for (double x = -3.0; x < 3.0; x += 1.0)
    //    {
    //    BISQDebugLine("IsNotableExponent(%0.1f) = %s", x, IsNotableExponent(x) ? "true" : "false");
    //    }

    BISQDebugLine("\n\n!!!!!!!!!!!---Test  Converting:  ");

    ShowConversion(SimpleQuantity(1.0, "MILE"), "KM", true);

    QuantityP q = SimpleQuantity(625.73, "KM"); //625.73_KM
    ShowConversion(q, "MILE", false);
    ShowConversion(q, "FT", false);
    ShowConversion(q, "FATHOM", false);
    ShowConversion(q, "NAUT_MILE", true);
    ShowConversion(SimpleQuantity(150000000, "KM"), "LIGHT_MIN", true);

    q = SimpleQuantity(14.593903, "KG");
    ShowConversion(q, "LBM", false);
    ShowConversion(q, "MG", false);
    ShowConversion(q, "TON", false);
    ShowConversion(q, "SLUG", true);

    q = SimpleQuantity(72.0, "KM/HR");
    ShowConversion(q, "M/S", false);
    ShowConversion(q, "MPH", true);


    ShowConversion(SimpleQuantity(1.0, "IN"), "MM", true);

    ShowConversion(SimpleQuantity(10.0, "CELSIUS"), "KELVIN", true);
    ShowConversion(SimpleQuantity(32.0, "FAHRENHEIT"), "KELVIN", true);
    ShowConversion(SimpleQuantity(55.0, "FAHRENHEIT"), "CELSIUS", true);

    ShowConversion(SimpleQuantity(1.0, "KJ"), "J", true);
    ShowConversion(SimpleQuantity(1.0, "CAL"), "J", true);
    ShowConversion(SimpleQuantity(1.0, "J"), "CAL", true);
    ShowConversion(SimpleQuantity(1.0, "CUB.M"), "CUB.FT", true);

    QuantityP len = SimpleQuantity(3.0, "FT");
    QuantityP vol = Volume(len, SimpleQuantity(8.0, "IN"), SimpleQuantity(2.0, "YARD"));

    ShowConversion(vol, "LITRE", false);
    ShowConversion(vol, "CUB.IN", false);
    ShowConversion(vol, "GALLON", true);
    ////ReportProblems();
    BISQCompletePrinting();

    
    }
    
TEST_F (UnitsTests, UnitsMapping)
    {
    bmap<Utf8String, Utf8String> unitNameMap;
    bset<Utf8String> notMapped;
    GetMapping(L"unitcomparisondata.csv", unitNameMap, notMapped);
    GetMapping(L"complex.csv", unitNameMap, notMapped);

    //output of oldUnit, newUnit mapping
    Utf8String mapOldtoNew = UnitsTestFixture::GetOutputDataPath(L"mapOldtoNew.csv");
    BISQOpenLogFile(mapOldtoNew.c_str());
    for (auto pair : unitNameMap)
        {
        Utf8String map = pair.first + "," + pair.second;
        BISQDebugMessage(map.c_str(), true);
        }

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

struct QuantityHubPerformance : QuantityHub
    {
    public:
        static QuantityHubP Get()
            {
            return QuantityHub::Get(true);
            }
    };

void GetAllUnitNames(QuantityHubP hub, bvector<Utf8CP>& allUnitNames, bool includeSynonyms)
    {
    for (auto const& phenomenon : hub->PhenomList())
        {
        for (auto const& unit : phenomenon->Units())
            {
            allUnitNames.push_back(unit->GetName().c_str());
            if (includeSynonyms)
                for (auto const& synonym : unit->GetSynonyms())
                    allUnitNames.push_back(synonym.c_str());
            }
        }
    }

TEST_F(UnitsPerformanceTests, InitUnitsHub)
    {
    StopWatch timer("Init Units Hub", false);
    timer.Start();
    QuantityHubP hub = QuantityHubPerformance::Get();
    timer.Stop();
    bvector<Utf8CP> allUnitNames;
    GetAllUnitNames(hub, allUnitNames, false);

    PERFORMANCELOG.errorv("Time to load Units Hub with %lu units: %lf", allUnitNames.size(), timer.GetElapsedSeconds());
    }

void GetUnitsByName(QuantityHubP hub, bvector<Utf8CP>& unitNames)
    {
    for (auto const& unitName : unitNames)
        {
        auto unit = hub->GetUnit(unitName);
        ASSERT_NE(nullptr, unit) << "Failed to get unit: " << unitName;
        }
    }

TEST_F(UnitsPerformanceTests, GetEveryUnitByName)
    {
    StopWatch timer("Get every unit by name", false);
    QuantityHubP hub = QuantityHubPerformance::Get();
    bvector<Utf8CP> allUnitNames;
    timer.Start();
    GetAllUnitNames(hub, allUnitNames, false);
    timer.Stop();
    PERFORMANCELOG.errorv("Time to get all %lu primary unit names: %lf", allUnitNames.size(), timer.GetElapsedSeconds());
    
    timer.Start();
    GetUnitsByName(hub, allUnitNames);
    timer.Stop();
    PERFORMANCELOG.errorv("Time to get %lu units by name: %lf", allUnitNames.size(), timer.GetElapsedSeconds());

    hub = QuantityHubPerformance::Get();
    allUnitNames.clear();
    timer.Start();
    GetAllUnitNames(hub, allUnitNames, true);
    timer.Stop();
    PERFORMANCELOG.errorv("Time to get all %lu primary unit names and synonyms: %lf", allUnitNames.size(), timer.GetElapsedSeconds());

    timer.Start();
    GetUnitsByName(hub, allUnitNames);
    timer.Stop();
    PERFORMANCELOG.errorv("Time to get %lu units by name including using synonyms: %lf", allUnitNames.size(), timer.GetElapsedSeconds());
    }

TEST_F(UnitsPerformanceTests, GenerateEveryConversionValue)
    {
    StopWatch timer("Evaluate every unit", false);
    QuantityHubP hub = QuantityHubPerformance::Get();
    bvector<UnitP> allUnits;
    for (auto const& phenomenon : hub->PhenomList())
        {
        for (auto const& unit : phenomenon->Units())
            {
            allUnits.push_back(unit);
            }
        }

    timer.Start();
    for (auto const& unit : allUnits)
        {
        ASSERT_FALSE(unit->IsEvaled()) << "Unit already evaluated: " << unit->GetName();
        unit->Evaluate();
        }
    timer.Stop();
    PERFORMANCELOG.errorv("Time to evaluate %lu units: %lf", allUnits.size(), timer.GetElapsedSeconds());
    }

END_UNITS_UNITTESTS_NAMESPACE
