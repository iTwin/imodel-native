/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"
#include "ECDbPublishedTests.h"
#include "MockHubApi.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

//======================================================================================
// File-local helpers for inspecting the sync-db reservation tables.
//======================================================================================

//---------------------------------------------------------------------------------------
// Returns true when the schema_reservation_ids table exists in syncDb.
//---------------------------------------------------------------------------------------
static bool ReservationTableExists(ECDbCR syncDb)
    {
    return syncDb.TableExists("schema_reservation_ids");
    }

//---------------------------------------------------------------------------------------
// Returns the LastReservedId counter for the given metadata table name (e.g. "ec_Class"),
// or 0 when the table/row is absent.
//---------------------------------------------------------------------------------------
static uint64_t GetLastReservedId(ECDbCR syncDb, Utf8CP resTableName)
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(syncDb,
            "SELECT [LastReservedId] FROM [schema_reservation_ids] WHERE [TableName]=?"))
        return 0;
    if (BE_SQLITE_OK != stmt.BindText(1, resTableName, Statement::MakeCopy::No))
        return 0;
    return (stmt.Step() == BE_SQLITE_ROW) ? static_cast<uint64_t>(stmt.GetValueInt64(0)) : 0;
    }

//---------------------------------------------------------------------------------------
// Returns true when the schema_reservation_columns table exists in syncDb.
//---------------------------------------------------------------------------------------
static bool ColumnReservationTableExists(ECDbCR syncDb)
    {
    return syncDb.TableExists("schema_reservation_columns");
    }

//---------------------------------------------------------------------------------------
// Returns true when schema_reservation_columns has at least one row.
//---------------------------------------------------------------------------------------
static bool HasColumnReservationRow(ECDbCR syncDb)
    {
    Statement stmt;
    if (BE_SQLITE_OK != stmt.Prepare(syncDb, "SELECT COUNT(*) FROM [schema_reservation_columns]"))
        return false;
    if (stmt.Step() != BE_SQLITE_ROW)
        return false;
    return stmt.GetValueInt(0) > 0;
    }

//======================================================================================
// Minimal schema XML builders (ECXML 3.1, no BisCore dependency).
//======================================================================================

//---------------------------------------------------------------------------------------
// Single entity class with no properties.
//---------------------------------------------------------------------------------------
static Utf8String BuildSingleClassSchema(Utf8CP version = "01.00.00")
    {
    Utf8String xml;
    xml.Sprintf(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema1" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="ClassA" />
        </ECSchema>)xml",
        version);
    return xml;
    }

//---------------------------------------------------------------------------------------
// Single entity class with two properties; uses TablePerHierarchy mapping so that
// schema_reservation_columns is exercised.
//---------------------------------------------------------------------------------------
static Utf8String BuildClassWithPropsSchema(Utf8CP version = "01.00.00")
    {
    Utf8String xml;
    xml.Sprintf(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema1" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="ClassA">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="p1" typeName="int" />
                <ECProperty propertyName="p2" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml",
        version);
    return xml;
    }

//======================================================================================
// Tests — all use SchemaImportOptions::None and pass the sync-db URI so that
// ImportSchemas internally calls ReserveSchemaImport.
//======================================================================================

// ---------------------------------------------------------------------------------------
// Test 1: ImportSingleClass_ReservationStorePopulated
// Verify that importing a schema with one entity class causes the sync-db reservation
// table to be created and populated with counters > 0 for ec_Class and ec_Schema.
// ---------------------------------------------------------------------------------------
TEST_F(SchemaSyncTestFixture, ImportSingleClass_ReservationStorePopulated)
    {
    ECDbHub hub;
    SchemaSyncDb schemaSyncDb("sync-db");
    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(schemaSyncDb.GetSyncDbUri(), "xxxxx", false));
    b1->SaveChanges();
    b1->PullMergePush("init");

    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, SchemaItem(BuildSingleClassSchema()), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    // The class must be visible in the briefcase.
    ASSERT_NE(b1->Schemas().GetClass("TestSchema1", "ClassA"), nullptr);

    // The sync-db reservation table must exist and contain counters > 0.
    schemaSyncDb.WithReadOnly([](ECDbCR syncDb)
        {
        ASSERT_TRUE(ReservationTableExists(syncDb)) << "schema_reservation_ids must exist after import";
        EXPECT_GT(GetLastReservedId(syncDb, "ec_Class"),  UINT64_C(0)) << "ec_Class LastReservedId must be > 0";
        EXPECT_GT(GetLastReservedId(syncDb, "ec_Schema"), UINT64_C(0)) << "ec_Schema LastReservedId must be > 0";
        });
    }

// ---------------------------------------------------------------------------------------
// Test 2: ImportClassWithProperties
// Verify that importing a class with properties writes both schema_reservation_ids
// (with an ec_Property row) and schema_reservation_columns (for the physical table).
// ---------------------------------------------------------------------------------------
TEST_F(SchemaSyncTestFixture, ImportClassWithProperties)
    {
    ECDbHub hub;
    SchemaSyncDb schemaSyncDb("sync-db");
    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(schemaSyncDb.GetSyncDbUri(), "xxxxx", false));
    b1->SaveChanges();
    b1->PullMergePush("init");

    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, SchemaItem(BuildClassWithPropsSchema()), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    // Both properties must be present on the class.
    ECClassCP cls = b1->Schemas().GetClass("TestSchema1", "ClassA");
    ASSERT_NE(cls, nullptr);
    EXPECT_EQ(2, cls->GetPropertyCount(false));
    EXPECT_NE(cls->GetPropertyP("p1"), nullptr);
    EXPECT_NE(cls->GetPropertyP("p2"), nullptr);

    schemaSyncDb.WithReadOnly([](ECDbCR syncDb)
        {
        // ec_Property counter must be present and > 0.
        ASSERT_TRUE(ReservationTableExists(syncDb));
        EXPECT_GT(GetLastReservedId(syncDb, "ec_Property"), UINT64_C(0)) << "ec_Property LastReservedId must be > 0";

        // Column-assignment table must exist and have at least one row.
        ASSERT_TRUE(ColumnReservationTableExists(syncDb)) << "schema_reservation_columns must exist";
        EXPECT_TRUE(HasColumnReservationRow(syncDb)) << "schema_reservation_columns must have at least one row";
        });
    }

// ---------------------------------------------------------------------------------------
// Test 3: AddPropertyByVersionBump
// Import v1.0.0 (one property p1), then v1.0.1 (adds p2). Both imports must succeed
// with SchemaImportResult::OK and both properties must be present afterward.
// ---------------------------------------------------------------------------------------
TEST_F(SchemaSyncTestFixture, AddPropertyByVersionBump)
    {
    ECDbHub hub;
    SchemaSyncDb schemaSyncDb("sync-db");
    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(schemaSyncDb.GetSyncDbUri(), "xxxxx", false));
    b1->SaveChanges();
    b1->PullMergePush("init");

    // v1.0.0 — single property p1.
    auto schemaV1 = SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema1" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="ClassA">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="p1" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml");

    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, schemaV1, SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("v1 import");

    // v1.0.1 — adds property p2.
    auto schemaV2 = SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema1" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="ClassA">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="p1" typeName="int" />
                <ECProperty propertyName="p2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml");

    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, schemaV2, SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    ECClassCP cls = b1->Schemas().GetClass("TestSchema1", "ClassA");
    ASSERT_NE(cls, nullptr);
    EXPECT_NE(cls->GetPropertyP("p1"), nullptr) << "p1 must be present after version bump";
    EXPECT_NE(cls->GetPropertyP("p2"), nullptr) << "p2 must be present after version bump";
    }

// ---------------------------------------------------------------------------------------
// Test 4: AddClassByVersionBump
// Import v1.0.0 (ClassA only), then v1.0.1 (ClassA + ClassB). Both classes must be
// present and both imports must return SchemaImportResult::OK.
// ---------------------------------------------------------------------------------------
TEST_F(SchemaSyncTestFixture, AddClassByVersionBump)
    {
    ECDbHub hub;
    SchemaSyncDb schemaSyncDb("sync-db");
    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(schemaSyncDb.GetSyncDbUri(), "xxxxx", false));
    b1->SaveChanges();
    b1->PullMergePush("init");

    // v1.0.0 — ClassA only.
    auto schemaV1 = SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema1" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="ClassA" />
        </ECSchema>)xml");

    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, schemaV1, SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("v1 import");

    // v1.0.1 — ClassA + ClassB.
    auto schemaV2 = SchemaItem(
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema1" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="ClassA" />
            <ECEntityClass typeName="ClassB" />
        </ECSchema>)xml");

    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, schemaV2, SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    ASSERT_NE(b1->Schemas().GetClass("TestSchema1", "ClassA"), nullptr) << "ClassA must be present after version bump";
    ASSERT_NE(b1->Schemas().GetClass("TestSchema1", "ClassB"), nullptr) << "ClassB must be present after version bump";
    }

// ---------------------------------------------------------------------------------------
// Test 6: CrossBriefcaseIdDeterminism
// Two briefcases from an identical base each import the same schema through the shared
// sync channel.  Because reservation is content-keyed, both briefcases must end up with
// identical ec_Class ids for ClassA.
// ---------------------------------------------------------------------------------------
TEST_F(SchemaSyncTestFixture, CrossBriefcaseIdDeterminism)
    {
    ECDbHub hub;
    SchemaSyncDb schemaSyncDb("sync-db");
    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(schemaSyncDb.GetSyncDbUri(), "xxxxx", false));
    b1->SaveChanges();
    b1->PullMergePush("init");

    auto b2 = hub.CreateBriefcase();

    // b1 imports first — reserves the content-keyed ids in the sync-db.
    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, SchemaItem(BuildSingleClassSchema()), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("b1 import");

    // b2 imports the identical schema through the same shared sync channel.
    // ReserveSchemaImport will find the keys already present and return the same ids.
    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b2, SchemaItem(BuildSingleClassSchema()), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    ECClassCP cls1 = b1->Schemas().GetClass("TestSchema1", "ClassA");
    ECClassCP cls2 = b2->Schemas().GetClass("TestSchema1", "ClassA");
    ASSERT_NE(cls1, nullptr);
    ASSERT_NE(cls2, nullptr);

    // Content-keyed ids must be identical across both briefcases.
    EXPECT_EQ(cls1->GetId().GetValue(), cls2->GetId().GetValue())
        << "ClassA id must be identical across both briefcases (content-keyed determinism)";
    }

// ---------------------------------------------------------------------------------------
// Test 7: ReimportSameSchemaIsIdempotent
// Import the same schema version twice.  The second import must return OK, the
// LastReservedId counter in schema_reservation_ids must not advance, and the class id
// must remain unchanged.
// ---------------------------------------------------------------------------------------
TEST_F(SchemaSyncTestFixture, ReimportSameSchemaIsIdempotent)
    {
    ECDbHub hub;
    SchemaSyncDb schemaSyncDb("sync-db");
    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(schemaSyncDb.GetSyncDbUri(), "xxxxx", false));
    b1->SaveChanges();
    b1->PullMergePush("init");

    Utf8String schemaXml = BuildSingleClassSchema();

    // First import.
    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, SchemaItem(schemaXml.c_str()), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("first import");

    uint64_t lastIdAfterFirst = 0;
    schemaSyncDb.WithReadOnly([&](ECDbCR syncDb)
        {
        lastIdAfterFirst = GetLastReservedId(syncDb, "ec_Class");
        });
    ASSERT_GT(lastIdAfterFirst, UINT64_C(0)) << "ec_Class LastReservedId must be set after first import";

    ECClassCP cls = b1->Schemas().GetClass("TestSchema1", "ClassA");
    ASSERT_NE(cls, nullptr);
    uint64_t classIdAfterFirst = cls->GetId().GetValue();

    // Second import of the same schema version — idempotent update must succeed.
    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, SchemaItem(schemaXml.c_str()), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    // Counter must not have advanced — all keys were already reserved.
    uint64_t lastIdAfterSecond = 0;
    schemaSyncDb.WithReadOnly([&](ECDbCR syncDb)
        {
        lastIdAfterSecond = GetLastReservedId(syncDb, "ec_Class");
        });
    EXPECT_EQ(lastIdAfterFirst, lastIdAfterSecond)
        << "LastReservedId must not advance on re-import of the same schema";

    // Class id must be unchanged after the re-import.
    cls = b1->Schemas().GetClass("TestSchema1", "ClassA");
    ASSERT_NE(cls, nullptr);
    EXPECT_EQ(classIdAfterFirst, cls->GetId().GetValue())
        << "ClassA id must not change after re-import of the same schema";
    }

END_ECDBUNITTESTS_NAMESPACE
