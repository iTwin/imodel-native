/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <set>
#include <ECObjects/SchemaComparer.h>
#include <Bentley/BeDirectoryIterator.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass
//+---------------+---------------+---------------+---------------+---------------+------
struct SchemaGraphTestFixture : public ECDbTestFixture
    {
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaGraphTestFixture, MissingImportSchemaInTheMiddle)
    {
    // Test produces a real bug that we had with RoadRailPhysical and transformer.
    // The db contains an older version of BisCore and LinearReferecing, and the
    // incoming schemas are an updated BisCore plus RoadRailPhysical, but not LinearReferencing.
    // This caused an unclean schema graph with 2 BisCore versions in it.

    NativeLogging::Logging::SetLogger(&NativeLogging::ConsoleLogger::GetLogger());
    NativeLogging::ConsoleLogger::GetLogger().SetSeverity("ECDb", BentleyApi::NativeLogging::LOG_TRACE);
    NativeLogging::ConsoleLogger::GetLogger().SetSeverity("ECObjectsNative", BentleyApi::NativeLogging::LOG_TRACE);

    SchemaItem initialBisCoreXml(
    R"schema(<?xml version='1.0' encoding='utf-8' ?>
    <ECSchema schemaName="BisCore" alias="bis" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
        <ECEntityClass typeName="Element" />
        <ECEntityClass typeName="ISubModeledElement" modifier="Abstract">
            <ECCustomAttributes>
                <IsMixin xmlns="CoreCustomAttributes.01.00.03">
                    <AppliesToEntityClass>Element</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>
        </ECEntityClass>
    </ECSchema>)schema");

    SchemaItem initialLinearReferencingXml(
    R"schema(<?xml version='1.0' encoding='utf-8' ?>
    <ECSchema schemaName="LinearReferencing" alias="lr" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
        <ECSchemaReference name="BisCore" version="01.00.00" alias="bis"/>
        <ECEntityClass typeName="LinearPhysicalElement">
            <BaseClass>bis:Element</BaseClass>
        </ECEntityClass>
        <ECEntityClass typeName="ILinearElementSource" modifier="Abstract">
            <ECCustomAttributes>
                <IsMixin xmlns="CoreCustomAttributes.01.00.03">
                    <AppliesToEntityClass>bis:Element</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>
        </ECEntityClass>
    </ECSchema>)schema");

    std::vector<SchemaItem> initialSchemas { initialBisCoreXml, initialLinearReferencingXml };
    ASSERT_EQ(DbResult::BE_SQLITE_OK, SetupECDbForCurrentTest());

    ASSERT_EQ(BentleyStatus::SUCCESS, GetHelper().ImportSchemas(initialSchemas));

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    StringSchemaLocater locater;
    SchemaKey bisCoreKey ("BisCore", 1, 0, 1);
    
    locater.AddSchemaString(bisCoreKey, 
    R"schema(<?xml version='1.0' encoding='utf-8' ?>
    <ECSchema schemaName="BisCore" alias="bis" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
        <ECEntityClass typeName="Element" />
        <ECEntityClass typeName="ISubModeledElement" modifier="Abstract">
            <ECCustomAttributes>
                <IsMixin xmlns="CoreCustomAttributes.01.00.03">
                    <AppliesToEntityClass>Element</AppliesToEntityClass>
                </IsMixin>
            </ECCustomAttributes>
        </ECEntityClass>
    </ECSchema>)schema");

    SchemaKey roadRailPhysicalKey ("RoadRailPhysical", 1, 0, 0);
    locater.AddSchemaString(roadRailPhysicalKey,
    R"schema(<?xml version='1.0' encoding='utf-8' ?>
    <ECSchema schemaName="RoadRailPhysical" alias="rrphys" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="BisCore" version="01.00.01" alias="bis"/>
        <ECSchemaReference name = "LinearReferencing" version = "01.00.00" alias = "lr"/>
        <ECEntityClass typeName="Corridor">
            <BaseClass>lr:LinearPhysicalElement</BaseClass>
            <BaseClass>bis:ISubModeledElement</BaseClass>
            <BaseClass>lr:ILinearElementSource</BaseClass>
        </ECEntityClass>
    </ECSchema>)schema");
    context->AddSchemaLocater(locater);
    context->SetFinalSchemaLocater(m_ecdb.GetSchemaLocater());
    SchemaKey bisCoreRequestedKey("BisCore", 1, 0, 0);
    ECSchemaPtr bisCore = context->LocateSchema(bisCoreRequestedKey, SchemaMatchType::LatestReadCompatible);
    ASSERT_TRUE(bisCore.IsValid());
    SchemaKey roadRailPhysicalRequestedKey("RoadRailPhysical", 1, 0, 0);
    ECSchemaPtr roadRailPhysical = context->LocateSchema(roadRailPhysicalRequestedKey, SchemaMatchType::LatestReadCompatible);
    ASSERT_TRUE(roadRailPhysical.IsValid());

    }

END_ECDBUNITTESTS_NAMESPACE