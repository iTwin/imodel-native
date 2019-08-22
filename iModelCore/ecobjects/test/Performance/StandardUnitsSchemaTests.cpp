/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "PerformanceTestFixture.h"

#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeNumerical.h>

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

struct UnitsPerformanceTest : PerformanceTestFixture {};

void GetUnitsByName(SchemaUnitContextCR context, bvector<Utf8String>& unitNames)
    {
    for (auto const& unitName : unitNames)
        {
        auto unit = context.LookupUnit(unitName.c_str());
        ASSERT_TRUE(unit != nullptr) << "Failed to get unit: " << unitName;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                                07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitsPerformanceTest, LoadUnitsSchema)
    {
    StopWatch timer("Load Units Schema", false);

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    ECSchemaPtr unitsSchema;

    BeFileName assetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(assetsDir);
    assetsDir.append(L"ECSchemas\\Standard\\");
    assetsDir.append(L"Units.01.00.01.ecschema.xml");

    timer.Start();
    ECSchema::ReadFromXmlFile(unitsSchema, assetsDir.c_str(), *context);
    timer.Stop();

    EXPECT_EQ(456, unitsSchema->GetUnitCount());
    EXPECT_EQ(67, unitsSchema->GetPhenomenonCount());
    EXPECT_EQ(12, unitsSchema->GetUnitSystemCount());

    PERFORMANCELOG.infov("Time to load the Standard Units Schema with %lu units: %.17g", unitsSchema->GetUnitCount(), timer.GetElapsedSeconds());

    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), 1, "Loading the Units Schema");

    bmap<Utf8String, double> results;
    results["LoadUnitsSchema"] = timer.GetElapsedSeconds();
    LogResultsToFile(results);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                               07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitsPerformanceTest, GetEveryUnitByName)
    {
    StopWatch timer("Get every unit by name", false);

    SchemaUnitContextCR context = GetUnitsSchema()->GetUnitsContext();

    bvector<Units::UnitCP> allUnits;
    timer.Start();
    context.AllUnits(allUnits);
    timer.Stop();
    PERFORMANCELOG.infov("Time to get all %lu units from the Standard Units Schema: %.17g", allUnits.size(), timer.GetElapsedSeconds());

    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), 1, "Load all Units from the Standard Units Schema");

    bvector<Utf8String> allUnitNames;
    for (auto unit : allUnits)
        allUnitNames.push_back(unit->GetName().c_str());

    timer.Start();
    GetUnitsByName(context, allUnitNames);
    timer.Stop();
    PERFORMANCELOG.infov("Time to get %lu units by name: %.17g", allUnitNames.size(), timer.GetElapsedSeconds());

    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), 1, "Get all Unit names from the Standard Units Schema");
    }

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
    PERFORMANCELOG.infov("Time to Generate %d conversion factors: %.17g", numConversions, timer.GetElapsedSeconds());

    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), 1, "Generate conversion factors for all Units within the Phenomena");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            Colin.Kerr                               07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(UnitsPerformanceTest, ConvertManyValues)
    {
    StopWatch timer("Do Many Conversions", false);
    SchemaUnitContextCR context = GetUnitsSchema()->GetUnitsContext();
    ECUnitCP unitA = context.LookupUnit("CUB_M_PER_SQ_M_DAY");
    ECUnitCP unitB = context.LookupUnit("CUB_FT_PER_ACRE_SEC");

    double converted;

    int numTimes = 1000000;
    timer.Start();
    for (int i = 0; i < numTimes; ++i)
        unitA->Convert(converted, 42.42, unitB);
    timer.Stop();
    PERFORMANCELOG.infov("Time to convert between %s and %s %d times: %.15g", unitA->GetName().c_str(), unitB->GetName().c_str(), numTimes, timer.GetElapsedSeconds());

    LOGTODB(TEST_DETAILS, timer.GetElapsedSeconds(), numTimes, "Convert between two units. (Conversion is not cached)");
    }

END_BENTLEY_ECN_TEST_NAMESPACE
