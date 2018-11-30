/*--------------------------------------------------------------------------------------+
|
|  $Source: tests/Performance/UnitsPerformanceTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "../TestFixture/UnitsTestFixture.h"

#include <Bentley/BeNumerical.h>

BEGIN_UNITS_UNITTESTS_NAMESPACE

struct UnitsPerformanceTests : UnitsTestFixture {};

void GetUnitsByName(UnitRegistry& hub, bvector<Utf8String>& unitNames)
    {
    for (auto const& unitName : unitNames)
        {
        auto unit = hub.LookupUnit(unitName.c_str());
        ASSERT_TRUE(unit != nullptr) << "Failed to get unit: " << unitName;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitsPerformanceTests, InitUnitsHub)
    {
    StopWatch timer("Init Units Hub", false);
    timer.Start();
    UnitRegistry* hub = new UnitRegistry();
    timer.Stop();
    bvector<Utf8String> allUnitNames;
    hub->AllUnitNames(allUnitNames, false);

    PERFORMANCELOG.errorv("Time to load Units Hub with %lu units: %.17g", allUnitNames.size(), timer.GetElapsedSeconds());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                               07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitsPerformanceTests, GetEveryUnitByName)
    {
    UnitRegistry* hub;

    StopWatch timer("Get every unit by name", false);
    bvector<Utf8String> allUnitNames;
    hub = new UnitRegistry();
    timer.Start();
    hub->AllUnitNames(allUnitNames, false);
    timer.Stop();
    PERFORMANCELOG.errorv("Time to get all %lu primary unit names: %.17g", allUnitNames.size(), timer.GetElapsedSeconds());
    
    timer.Start();
    GetUnitsByName(*hub, allUnitNames);
    timer.Stop();
    PERFORMANCELOG.errorv("Time to get %lu units by name: %.17g", allUnitNames.size(), timer.GetElapsedSeconds());

    hub = new UnitRegistry();
    allUnitNames.clear();
    timer.Start();
    hub->AllUnitNames(allUnitNames, true);
    timer.Stop();
    PERFORMANCELOG.errorv("Time to get all %lu primary unit names and synonyms: %.17g", allUnitNames.size(), timer.GetElapsedSeconds());

    timer.Start();
    GetUnitsByName(*hub, allUnitNames);
    timer.Stop();
    PERFORMANCELOG.errorv("Time to get %lu units by name including using synonyms: %.17g", allUnitNames.size(), timer.GetElapsedSeconds());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                               07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitsPerformanceTests, GenerateEveryConversionValue)
    {
    StopWatch timer("Evaluate every unit", false);
    
    UnitRegistry* hub = new UnitRegistry();
    bvector<PhenomenonCP> allPhenomena;
    hub->AllPhenomena(allPhenomena);

    int numConversions = 0;
    timer.Start();
    for (auto const& phenomenon : allPhenomena)
        {
        if (!phenomenon->HasUnits())
            continue;
        UnitCP firstUnit = *phenomenon->GetUnits().begin();
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

END_UNITS_UNITTESTS_NAMESPACE
