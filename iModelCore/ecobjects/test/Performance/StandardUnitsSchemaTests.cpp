/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Performance/StandardUnitsSchemaTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "PerformanceTestFixture.h"

#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeNumerical.h>

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct UnitsPerformanceTest : PerformanceTestFixture {};

//void GetUnitsByName(SchemaUnitContextR context, bvector<Utf8String>& unitNames)
//    {
//    for (auto const& unitName : unitNames)
//        {
//        auto unit = context.LookupUnit(unitName.c_str());
//        ASSERT_TRUE(unit != nullptr) << "Failed to get unit: " << unitName;
//        }
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitsPerformanceTest, LoadUnitsSchema)
    {
    StopWatch timer("Load Units Schema", false);

    ECSchemaReadContextPtr context;
    ECSchemaPtr unitsSchema;

    BeFileName assetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsDir);
    assetsDir.append(L"ECSchemas\\Standard\\");
    assetsDir.append(L"Units.01.00.00.ecschema.xml");

    double overallTime = 0;

    for (int i = 0; i < 10000; i++)
        {
        unitsSchema = nullptr;
        context = ECSchemaReadContext::CreateContext();
        timer.Start();
        ECSchema::ReadFromXmlFile(unitsSchema, assetsDir, *context);
        timer.Stop();
        overallTime += timer.GetElapsedSeconds();
        }

    PERFORMANCELOG.errorv("Time to load the Standard Units Schema 10000 times with %lu units: %.17g", unitsSchema->GetUnitCount(), overallTime);

    LOGTODB(TEST_DETAILS, overallTime, 10000, "Loading the Units Schema");

    bmap<Utf8String, double> results;
    results["LoadUnitsSchema"] = overallTime;
    LogResultsToFile(results);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                               07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
//TEST_F(UnitsPerformanceTest, GetEveryUnitByName)
//    {
//    StopWatch timer("Get every unit by name", false);
//    bvector<Utf8String> allUnitNames;
//
//    timer.Start();
//    GetUnitsSchema()->GetUnitsContext().AllUnitNames(allUnitNames, false);
//    timer.Stop();
//    PERFORMANCELOG.errorv("Time to get all %lu primary unit names: %.17g", allUnitNames.size(), timer.GetElapsedSeconds());
//    
//    timer.Start();
//    GetUnitsByName(*hub, allUnitNames);
//    timer.Stop();
//    PERFORMANCELOG.errorv("Time to get %lu units by name: %.17g", allUnitNames.size(), timer.GetElapsedSeconds());
//
//    hub = new UnitRegistry();
//    allUnitNames.clear();
//    timer.Start();
//    hub->AllUnitNames(allUnitNames, true);
//    timer.Stop();
//    PERFORMANCELOG.errorv("Time to get all %lu primary unit names and synonyms: %.17g", allUnitNames.size(), timer.GetElapsedSeconds());
//
//    timer.Start();
//    GetUnitsByName(*hub, allUnitNames);
//    timer.Stop();
//    PERFORMANCELOG.errorv("Time to get %lu units by name including using synonyms: %.17g", allUnitNames.size(), timer.GetElapsedSeconds());
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                               07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitsPerformanceTest, GenerateEveryConversionValue)
    {
    StopWatch timer("Evaluate every unit", false);
    
    bvector<Units::PhenomenonCP> allPhenomena;
    GetUnitsSchema()->GetUnitsContext().AllPhenomena(allPhenomena);

    int numConversions = 0;
    timer.Start();
    for (auto const& phenomenon : allPhenomena)
        {
        if (!phenomenon->HasUnits())
            continue;
        Units::UnitCP firstUnit = *phenomenon->GetUnits().begin();
        for (auto const& unit : phenomenon->GetUnits())
            {
            double converted;
            firstUnit->Convert(converted, 42, unit);
            ASSERT_FALSE(BeNumerical::BeIsnan(converted)) << "Generated conversion factor is invalid from " << firstUnit->GetName() << " to " << unit->GetName();
            ++numConversions;
            }
        }
    timer.Stop();
    PERFORMANCELOG.errorv("Time to Generate %d conversion factors: %.17g", numConversions, timer.GetElapsedSeconds());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                               07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
//TEST_F(UnitsPerformanceTests, ConvertManyValues)
//    {
//    StopWatch timer("Do Many Conversions", false);
//    UnitRegistry::Clear();
//    UnitCP unitA = UnitRegistry::Get().LookupUnit("CUB.M/(SQ.M*DAY)");
//    UnitCP unitB = UnitRegistry::Get().LookupUnit("CUB.FT/(ACRE*SEC)");
//
//    int numTimes = 1000000;
//    timer.Start();
//    for (int i = 0; i < numTimes; ++i)
//        unitA->Convert(42.42, unitB);
//    timer.Stop();
//    PERFORMANCELOG.errorv("Time to covert between %s and %s %d times: %.15g", unitA->GetName(), unitB->GetName(), numTimes, timer.GetElapsedSeconds());
//    }

END_BENTLEY_ECN_TEST_NAMESPACE
