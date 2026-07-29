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
                    <ShareColumns xmlns="ECDbMap.02.00.00" />
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
// Test 5: CrossBriefcaseColumnDeterminism
// Two briefcases that share a common base (one TPH class with properties already mapped)
// both import a property-adding schema upgrade through the shared sync channel.
// Because column assignment is keyed by content and coordinated through the sync-db
// reservation store, both briefcases must assign identical ec_Column.Id and
// ec_Column.Ordinal to each newly-added property.
// ---------------------------------------------------------------------------------------
TEST_F(SchemaSyncTestFixture, CrossBriefcaseColumnDeterminism)
    {
    ECDbHub hub;
    SchemaSyncDb schemaSyncDb("sync-db");

    // Create b1 and import the base schema (v1.0.0 with ClassA, p1, p2 in TPH).
    // After PullMergePush the sync-db reservation store is written for p1 and p2.
    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(schemaSyncDb.GetSyncDbUri(), "xxxxx", false));
    b1->SaveChanges();
    b1->PullMergePush("init");

    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, SchemaItem(BuildClassWithPropsSchema()), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("b1 import v1.0.0");

    // Create b2 after b1 pushed the base schema — b2 gets ClassA already in ec_ClassMap,
    // which is required for FindPrimaryTableForClass to resolve the physical table name
    // during column reservation.
    auto b2 = hub.CreateBriefcase();

    // Build v1.0.1: adds a third property p3 to ClassA.
    Utf8String schemaV101 =
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema1" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="ClassA">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00" />
                </ECCustomAttributes>
                <ECProperty propertyName="p1" typeName="int" />
                <ECProperty propertyName="p2" typeName="string" />
                <ECProperty propertyName="p3" typeName="double" />
            </ECEntityClass>
        </ECSchema>)xml";

    // b1 imports v1.0.1 — reserves the column ordinal + column id for p3 in the sync-db.
    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, SchemaItem(schemaV101.c_str()), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("b1 import v1.0.1");

    // b2 imports the same v1.0.1 — reads the already-reserved column id for p3 from
    // the sync-db and uses it, so it gets the identical ec_Column.Id.
    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b2, SchemaItem(schemaV101.c_str()), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    // Helper: return (ec_Column.Id, ec_Column.Ordinal) for the column that maps
    // the named property of ClassA in the given ECDb.
    auto getColumnInfo = [](ECDbCR db, Utf8CP propName) -> std::pair<uint64_t, int>
        {
        CachedStatementPtr stmt = db.GetCachedStatement(
            "SELECT c.Id, c.Ordinal "
            "FROM   ec_Column       c "
            "JOIN   ec_PropertyMap  pm ON pm.ColumnId = c.Id "
            "JOIN   ec_PropertyPath pp ON pp.Id = pm.PropertyPathId "
            "JOIN   ec_Class        cls ON cls.Id = pm.ClassId "
            "WHERE  cls.Name = 'ClassA' AND pp.AccessString = ?");
        if (stmt == nullptr)
            return {0, -1};
        stmt->BindText(1, propName, Statement::MakeCopy::No);
        if (stmt->Step() != BE_SQLITE_ROW)
            return {0, -1};
        return {static_cast<uint64_t>(stmt->GetValueInt64(0)), stmt->GetValueInt(1)};
        };

    auto [colId1_p3, colOrd1_p3] = getColumnInfo(*b1, "p3");
    auto [colId2_p3, colOrd2_p3] = getColumnInfo(*b2, "p3");

    ASSERT_GT(colId1_p3, UINT64_C(0)) << "b1: ec_Column.Id for p3 must be valid";
    ASSERT_GT(colId2_p3, UINT64_C(0)) << "b2: ec_Column.Id for p3 must be valid";
    ASSERT_GE(colOrd1_p3, 0)          << "b1: ec_Column.Ordinal for p3 must be non-negative";
    ASSERT_GE(colOrd2_p3, 0)          << "b2: ec_Column.Ordinal for p3 must be non-negative";

    EXPECT_EQ(colId1_p3, colId2_p3)
        << "ec_Column.Id for p3 must be identical across both briefcases (content-keyed column reservation)";
    EXPECT_EQ(colOrd1_p3, colOrd2_p3)
        << "ec_Column.Ordinal for p3 must be identical across both briefcases";
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

// ---------------------------------------------------------------------------------------
// Test 8: InitFromNonEmptyBase_ReservationStorePrePopulated
// When Init is called on a briefcase that already has a user schema imported, the sync-db
// reservation store must be pre-populated immediately after Init — without any subsequent
// ImportSchemas call.  This verifies the §4 Init-seeding behaviour.
// ---------------------------------------------------------------------------------------
TEST_F(SchemaSyncTestFixture, InitFromNonEmptyBase_ReservationStorePrePopulated)
    {
    Utf8String schemaXml =
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="SeedTestSchema" alias="sts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="SeedClass">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="SeedProp" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml";

    ECDbHub hub;
    SchemaSyncDb schemaSyncDb("sync-db");
    auto b1 = hub.CreateBriefcase();

    // Import the schema into b1 WITHOUT schema sync so that it is already persisted
    // in the local db before Init is called.
    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, SchemaItem(schemaXml.c_str()),
                     SchemaManager::SchemaImportOptions::None, SchemaSync::SyncDbUri{}));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    ECClassCP cls = b1->Schemas().GetClass("SeedTestSchema", "SeedClass");
    ASSERT_NE(cls, nullptr);
    ASSERT_TRUE(cls->HasId());
    uint64_t localClassId = cls->GetId().GetValue();

    // Now call Init — it should seed the existing ids into the sync-db.
    ASSERT_EQ(SchemaSync::Status::OK,
        b1->Schemas().GetSchemaSync().Init(schemaSyncDb.GetSyncDbUri(), "init-seed-test", false));

    // The sync-db reservation store must be present and the ec_Class counter must already
    // reflect the pre-existing class (no ImportSchemas has been called through sync yet).
    schemaSyncDb.WithReadOnly([&](ECDbCR syncDb)
        {
        ASSERT_TRUE(ReservationTableExists(syncDb))
            << "schema_reservation_ids must be created by Init";
        uint64_t classCounter = GetLastReservedId(syncDb, "ec_Class");
        EXPECT_GE(classCounter, localClassId)
            << "ec_Class counter must be >= the persisted id of SeedClass after Init seeding";

        ASSERT_TRUE(ColumnReservationTableExists(syncDb))
            << "schema_reservation_columns must be created by Init";
        EXPECT_TRUE(HasColumnReservationRow(syncDb))
            << "schema_reservation_columns must have at least one row after seeding SeedProp";
        });
    }

// ---------------------------------------------------------------------------------------
// Test 9: InitSeeding_ExistingClassIdPreservedAcrossBriefcases
// b1 imports a schema without schema sync, then calls Init.  b2 (created after Init)
// imports a version bump that references the same base class.  The base class id in b2
// must match the id that was persisted in b1's local db at Init time (verified by
// comparing ids after both briefcases apply the upgrade).
// ---------------------------------------------------------------------------------------
TEST_F(SchemaSyncTestFixture, InitSeeding_ExistingClassIdPreservedAcrossBriefcases)
    {
    // v1.0.0: base schema with ClassA.
    Utf8String schemaV1 =
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema1" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="ClassA" />
        </ECSchema>)xml";

    // v1.0.1: adds ClassB.
    Utf8String schemaV2 =
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema1" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="ClassA" />
            <ECEntityClass typeName="ClassB" />
        </ECSchema>)xml";

    ECDbHub hub;
    SchemaSyncDb schemaSyncDb("sync-db");

    // b1 imports v1.0.0 without schema sync, then bootstraps the container.
    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, SchemaItem(schemaV1.c_str()),
                     SchemaManager::SchemaImportOptions::None, SchemaSync::SyncDbUri{}));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    ECClassCP clsA_before = b1->Schemas().GetClass("TestSchema1", "ClassA");
    ASSERT_NE(clsA_before, nullptr);
    uint64_t classAIdInB1 = clsA_before->GetId().GetValue();

    ASSERT_EQ(SchemaSync::Status::OK,
        b1->Schemas().GetSchemaSync().Init(schemaSyncDb.GetSyncDbUri(), "preserve-ids-test", false));
    b1->SaveChanges();
    b1->PullMergePush("b1 post-init sync");

    // b2 is created after Init — it receives ClassA from the container push.
    auto b2 = hub.CreateBriefcase();

    // Both briefcases import v1.0.1 (adds ClassB) through the shared sync channel.
    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, SchemaItem(schemaV2.c_str()),
                     SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("b1 v1.0.1");

    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b2, SchemaItem(schemaV2.c_str()),
                     SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    // ClassA must have the same id as it had in b1 before Init — the Init-seeding must
    // have captured it so that all subsequent briefcases share the same id.
    ECClassCP clsA_b2 = b2->Schemas().GetClass("TestSchema1", "ClassA");
    ASSERT_NE(clsA_b2, nullptr);
    EXPECT_EQ(classAIdInB1, clsA_b2->GetId().GetValue())
        << "ClassA id in b2 must match the id persisted in b1 before Init (Init-seeding)";

    // ClassB ids must agree across both briefcases (normal cross-briefcase determinism).
    ECClassCP clsB_b1 = b1->Schemas().GetClass("TestSchema1", "ClassB");
    ECClassCP clsB_b2 = b2->Schemas().GetClass("TestSchema1", "ClassB");
    ASSERT_NE(clsB_b1, nullptr);
    ASSERT_NE(clsB_b2, nullptr);
    EXPECT_EQ(clsB_b1->GetId().GetValue(), clsB_b2->GetId().GetValue())
        << "ClassB id must be identical across both briefcases";
    }

// ---------------------------------------------------------------------------------------
// Test 10: OverrideContainerReInit_SeedsFromCurrentLocal
// When Init is called with overrideContainer=true on a briefcase that already has a
// schema imported, the NEW sync container must be seeded from the current local
// baseline so that subsequent briefcases see consistent ids.
// ---------------------------------------------------------------------------------------
TEST_F(SchemaSyncTestFixture, OverrideContainerReInit_SeedsFromCurrentLocal)
    {
    Utf8String schemaXml =
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema1" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="ClassA" />
        </ECSchema>)xml";

    ECDbHub hub;
    SchemaSyncDb schemaSyncDb1("sync-db-1");
    SchemaSyncDb schemaSyncDb2("sync-db-2");
    auto b1 = hub.CreateBriefcase();

    // First Init + import via container-1.
    ASSERT_EQ(SchemaSync::Status::OK,
        b1->Schemas().GetSchemaSync().Init(schemaSyncDb1.GetSyncDbUri(), "container-1", false));
    b1->SaveChanges();
    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, SchemaItem(schemaXml.c_str()),
                     SchemaManager::SchemaImportOptions::None, schemaSyncDb1.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("b1 import");

    ECClassCP cls = b1->Schemas().GetClass("TestSchema1", "ClassA");
    ASSERT_NE(cls, nullptr);
    uint64_t classAId = cls->GetId().GetValue();

    // Re-Init b1 to a new container with overrideContainer=true — must seed from b1's
    // current local state (which includes ClassA).
    ASSERT_EQ(SchemaSync::Status::OK,
        b1->Schemas().GetSchemaSync().Init(schemaSyncDb2.GetSyncDbUri(), "container-2", true));

    schemaSyncDb2.WithReadOnly([&](ECDbCR syncDb)
        {
        ASSERT_TRUE(ReservationTableExists(syncDb))
            << "schema_reservation_ids must exist in the new container after override re-Init";
        uint64_t classCounter = GetLastReservedId(syncDb, "ec_Class");
        EXPECT_GE(classCounter, classAId)
            << "ec_Class counter in new container must reflect ClassA's persisted id";
        });
    }

// ---------------------------------------------------------------------------------------
// Test 11: TwoSchemaImport_ThenVersionBumpAddsSharedColumn
//
// b1 imports two schemas through the shared sync channel:
//   * TestSchema1 (v1.0.0): ABC (TPH root with ShareColumns, prop abcProp) and
//     DEF (base ABC, prop defProp).  Because ABC is a TPH root that shares columns, the
//     properties of ABC/DEF/XYZ all land in ABC's primary physical table as shared columns.
//   * TestSchema2 (v1.0.0, references TestSchema1): XYZ (base DEF, prop xyzProp).
//
// b2 is created after b1 pushes, then imports ONLY TestSchema1 with a version bump to
// v1.0.1 that adds a single new property defProp2 to DEF.
//
// The test inspects:
//   (a) the sync-db reservation state (id table + column table), and
//   (b) which property maps to which shared column in each briefcase.
//
// Expectations:
//   * defProp / xyzProp keep identical (ec_Column.Id, Ordinal) across b1 and b2 because
//     their shared-column assignment was reserved through the sync channel.
//   * defProp2 (added only in b2) gets a brand-new shared column whose ordinal does not
//     collide with the already-reserved xyzProp ordinal — the coordinated column counter
//     hands out a fresh slot above the existing high-water mark.
//   * All of abcProp/defProp/xyzProp/defProp2 live in the same physical (ABC) table.
// ---------------------------------------------------------------------------------------
TEST_F(SchemaSyncTestFixture, TwoSchemaImport_ThenVersionBumpAddsSharedColumn)
    {
    // TestSchema1 v1.0.0 — ABC (TPH + ShareColumns) and DEF : ABC.
    Utf8CP schema1_v100 =
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema1" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="ABC">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="abcProp" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="DEF">
                <BaseClass>ABC</BaseClass>
                <ECProperty propertyName="defProp" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml";

    // TestSchema2 v1.0.0 — references TestSchema1; XYZ : DEF.
    Utf8CP schema2_v100 =
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema2" alias="ts2" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="TestSchema1" version="01.00.00" alias="ts"/>
            <ECEntityClass typeName="XYZ">
                <BaseClass>ts:DEF</BaseClass>
                <ECProperty propertyName="xyzProp" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml";

    // TestSchema1 v1.0.1 — bumps version and adds a single property defProp2 to DEF.
    Utf8CP schema1_v101 =
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema1" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="ABC">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="abcProp" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="DEF">
                <BaseClass>ABC</BaseClass>
                <ECProperty propertyName="defProp" typeName="int" />
                <ECProperty propertyName="defProp2" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml";

    ECDbHub hub;
    SchemaSyncDb schemaSyncDb("sync-db");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(schemaSyncDb.GetSyncDbUri(), "xxxxx", false));
    b1->SaveChanges();
    b1->PullMergePush("init");

    // b1 imports the two schemas — TestSchema1 first (TestSchema2 references it).
    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, SchemaItem(schema1_v100), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, SchemaItem(schema2_v100), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("b1 import TestSchema1 + TestSchema2");

    // All three classes must be present in b1.
    ASSERT_NE(b1->Schemas().GetClass("TestSchema1", "ABC"), nullptr);
    ASSERT_NE(b1->Schemas().GetClass("TestSchema1", "DEF"), nullptr);
    ASSERT_NE(b1->Schemas().GetClass("TestSchema2", "XYZ"), nullptr);

    // b2 is created after b1 pushed — it receives all three classes from the container.
    auto b2 = hub.CreateBriefcase();

    // b2 imports ONLY TestSchema1 v1.0.1 (adds defProp2 to DEF).
    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b2, SchemaItem(schema1_v101), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    ECClassCP defB2 = b2->Schemas().GetClass("TestSchema1", "DEF");
    ASSERT_NE(defB2, nullptr);
    EXPECT_NE(defB2->GetPropertyP("defProp"),  nullptr) << "defProp must survive the version bump in b2";
    EXPECT_NE(defB2->GetPropertyP("defProp2"), nullptr) << "defProp2 must be added by the version bump in b2";

    // Helper: return the (columnName, columnId, ordinal, tableName) that a property of a
    // class maps to in the given ECDb.  Returns {"",0,-1,""} when unmapped.
    auto getColInfo = [](ECDbCR db, Utf8CP className, Utf8CP accessString)
        -> std::tuple<Utf8String, uint64_t, int, Utf8String>
        {
        CachedStatementPtr stmt = db.GetCachedStatement(
            "SELECT c.Name, c.Id, c.Ordinal, t.Name "
            "FROM   ec_Column       c "
            "JOIN   ec_Table        t   ON t.Id  = c.TableId "
            "JOIN   ec_PropertyMap  pm  ON pm.ColumnId = c.Id "
            "JOIN   ec_PropertyPath pp  ON pp.Id = pm.PropertyPathId "
            "JOIN   ec_Class        cls ON cls.Id = pm.ClassId "
            "WHERE  cls.Name = ? AND pp.AccessString = ?");
        if (stmt == nullptr)
            return {Utf8String(), UINT64_C(0), -1, Utf8String()};
        stmt->BindText(1, className, Statement::MakeCopy::No);
        stmt->BindText(2, accessString, Statement::MakeCopy::No);
        if (stmt->Step() != BE_SQLITE_ROW)
            return {Utf8String(), UINT64_C(0), -1, Utf8String()};
        return {Utf8String(stmt->GetValueText(0)),
                static_cast<uint64_t>(stmt->GetValueInt64(1)),
                stmt->GetValueInt(2),
                Utf8String(stmt->GetValueText(3))};
        };

    // --- Property -> shared column mapping in b1 ---
    auto [abcCol_b1, abcColId_b1, abcOrd_b1, abcTbl_b1] = getColInfo(*b1, "ABC", "abcProp");
    auto [defCol_b1, defColId_b1, defOrd_b1, defTbl_b1] = getColInfo(*b1, "DEF", "defProp");
    auto [xyzCol_b1, xyzColId_b1, xyzOrd_b1, xyzTbl_b1] = getColInfo(*b1, "XYZ", "xyzProp");

    // --- Property -> shared column mapping in b2 ---
    auto [defCol_b2,  defColId_b2,  defOrd_b2,  defTbl_b2]  = getColInfo(*b2, "DEF", "defProp");
    auto [def2Col_b2, def2ColId_b2, def2Ord_b2, def2Tbl_b2] = getColInfo(*b2, "DEF", "defProp2");
    auto [xyzCol_b2,  xyzColId_b2,  xyzOrd_b2,  xyzTbl_b2]  = getColInfo(*b2, "XYZ", "xyzProp");

    printf("[b1] abcProp  -> %s.%s (id=%llu ord=%d)\n", abcTbl_b1.c_str(), abcCol_b1.c_str(), (unsigned long long) abcColId_b1, abcOrd_b1);
    printf("[b1] defProp  -> %s.%s (id=%llu ord=%d)\n", defTbl_b1.c_str(), defCol_b1.c_str(), (unsigned long long) defColId_b1, defOrd_b1);
    printf("[b1] xyzProp  -> %s.%s (id=%llu ord=%d)\n", xyzTbl_b1.c_str(), xyzCol_b1.c_str(), (unsigned long long) xyzColId_b1, xyzOrd_b1);
    printf("[b2] defProp  -> %s.%s (id=%llu ord=%d)\n", defTbl_b2.c_str(), defCol_b2.c_str(), (unsigned long long) defColId_b2, defOrd_b2);
    printf("[b2] defProp2 -> %s.%s (id=%llu ord=%d)\n", def2Tbl_b2.c_str(), def2Col_b2.c_str(), (unsigned long long) def2ColId_b2, def2Ord_b2);
    printf("[b2] xyzProp  -> %s.%s (id=%llu ord=%d)\n", xyzTbl_b2.c_str(), xyzCol_b2.c_str(), (unsigned long long) xyzColId_b2, xyzOrd_b2);

    // Every property must be mapped to a valid, shared column.
    ASSERT_GT(abcColId_b1,  UINT64_C(0));
    ASSERT_GT(defColId_b1,  UINT64_C(0));
    ASSERT_GT(xyzColId_b1,  UINT64_C(0));
    ASSERT_GT(defColId_b2,  UINT64_C(0));
    ASSERT_GT(def2ColId_b2, UINT64_C(0));
    ASSERT_GT(xyzColId_b2,  UINT64_C(0));

    // All properties across the hierarchy share ABC's single primary physical table.
    EXPECT_STREQ(abcTbl_b1.c_str(), defTbl_b1.c_str());
    EXPECT_STREQ(abcTbl_b1.c_str(), xyzTbl_b1.c_str());
    EXPECT_STREQ(abcTbl_b1.c_str(), def2Tbl_b2.c_str());

    // Content-keyed shared-column assignment: properties reserved through the sync channel
    // must land in identical columns in both briefcases.
    EXPECT_EQ(defColId_b1, defColId_b2) << "defProp column id must match across b1/b2";
    EXPECT_EQ(defOrd_b1,   defOrd_b2)   << "defProp column ordinal must match across b1/b2";
    EXPECT_EQ(xyzColId_b1, xyzColId_b2) << "xyzProp column id must match across b1/b2";
    EXPECT_EQ(xyzOrd_b1,   xyzOrd_b2)   << "xyzProp column ordinal must match across b1/b2";

    // The newly added defProp2 must occupy a brand-new shared column that does not collide
    // with any previously reserved shared column (in particular xyzProp's).
    EXPECT_NE(def2ColId_b2, xyzColId_b2) << "defProp2 must not reuse xyzProp's shared column id";
    EXPECT_NE(def2Ord_b2,   xyzOrd_b2)   << "defProp2 must not reuse xyzProp's shared column ordinal";
    EXPECT_NE(def2ColId_b2, defColId_b2) << "defProp2 must not reuse defProp's shared column id";
    EXPECT_NE(def2Ord_b2,   defOrd_b2)   << "defProp2 must not reuse defProp's shared column ordinal";

    // --- Sync-db reservation state ---
    schemaSyncDb.WithReadOnly([&](ECDbCR syncDb)
        {
        ASSERT_TRUE(ReservationTableExists(syncDb)) << "schema_reservation_ids must exist";
        EXPECT_GT(GetLastReservedId(syncDb, "ec_Class"),    UINT64_C(0)) << "ec_Class counter must be set";
        EXPECT_GT(GetLastReservedId(syncDb, "ec_Property"), UINT64_C(0)) << "ec_Property counter must be set";

        ASSERT_TRUE(ColumnReservationTableExists(syncDb)) << "schema_reservation_columns must exist";
        EXPECT_TRUE(HasColumnReservationRow(syncDb))      << "schema_reservation_columns must have a row for ABC's shared table";
        });
    }

//======================================================================================
// File-local helpers for Phase 1b / Gap A–F tests.
//======================================================================================

//---------------------------------------------------------------------------------------
// Returns the (ec_Column.Id, ec_Column.Ordinal) pair for the column that the named
// access string of the named mapped class maps to. Returns {0,-1} when not found.
//---------------------------------------------------------------------------------------
static std::pair<uint64_t, int> GetColumnIdAndOrdinal(ECDbCR db, Utf8CP className, Utf8CP accessString)
    {
    CachedStatementPtr stmt = db.GetCachedStatement(
        "SELECT c.Id, c.Ordinal "
        "FROM   ec_Column       c "
        "JOIN   ec_PropertyMap  pm ON pm.ColumnId  = c.Id "
        "JOIN   ec_PropertyPath pp ON pp.Id = pm.PropertyPathId "
        "JOIN   ec_Class        cls ON cls.Id = pm.ClassId "
        "WHERE  cls.Name = ? AND pp.AccessString = ?");
    if (stmt == nullptr) return {0, -1};
    stmt->BindText(1, className, Statement::MakeCopy::No);
    stmt->BindText(2, accessString, Statement::MakeCopy::No);
    return (stmt->Step() == BE_SQLITE_ROW)
        ? std::make_pair(static_cast<uint64_t>(stmt->GetValueInt64(0)), stmt->GetValueInt(1))
        : std::make_pair(UINT64_C(0), -1);
    }

//---------------------------------------------------------------------------------------
// Returns the ec_Column.Id of the column with the given Kind in the given physical table.
// Kind: 1 = ECInstanceId, 2 = ECClassId. Returns 0 when not found.
//---------------------------------------------------------------------------------------
static uint64_t GetSystemColumnId(ECDbCR db, Utf8CP tableName, int kind)
    {
    CachedStatementPtr stmt = db.GetCachedStatement(
        "SELECT c.Id FROM ec_Column c "
        "JOIN   ec_Table t ON t.Id = c.TableId "
        "WHERE  t.Name = ? AND c.ColumnKind = ?");
    if (stmt == nullptr) return 0;
    stmt->BindText(1, tableName, Statement::MakeCopy::No);
    stmt->BindInt(2, kind);
    return (stmt->Step() == BE_SQLITE_ROW) ? static_cast<uint64_t>(stmt->GetValueInt64(0)) : 0;
    }

//---------------------------------------------------------------------------------------
// Returns the ec_Table.Id for the given physical table name. Returns 0 if absent.
//---------------------------------------------------------------------------------------
static uint64_t GetTableId(ECDbCR db, Utf8CP tableName)
    {
    CachedStatementPtr stmt = db.GetCachedStatement("SELECT Id FROM ec_Table WHERE Name = ?");
    if (stmt == nullptr) return 0;
    stmt->BindText(1, tableName, Statement::MakeCopy::No);
    return (stmt->Step() == BE_SQLITE_ROW) ? static_cast<uint64_t>(stmt->GetValueInt64(0)) : 0;
    }

//---------------------------------------------------------------------------------------
// Returns the ec_Index.Id for the given index name in the given physical table. Returns 0.
//---------------------------------------------------------------------------------------
static uint64_t GetIndexId(ECDbCR db, Utf8CP tableName, Utf8CP indexName)
    {
    CachedStatementPtr stmt = db.GetCachedStatement(
        "SELECT idx.Id FROM ec_Index idx "
        "JOIN   ec_Table t ON t.Id = idx.TableId "
        "WHERE  t.Name = ? AND idx.Name = ?");
    if (stmt == nullptr) return 0;
    stmt->BindText(1, tableName, Statement::MakeCopy::No);
    stmt->BindText(2, indexName, Statement::MakeCopy::No);
    return (stmt->Step() == BE_SQLITE_ROW) ? static_cast<uint64_t>(stmt->GetValueInt64(0)) : 0;
    }

// ---------------------------------------------------------------------------------------
// Test 12: GapA_NonSharedColumnIdDeterminism
// Gap A: a class with OwnTable strategy (non-shared columns) creates ec_Column rows
// whose ids must be content-keyed — identical across two briefcases that import the
// same schema independently through the shared sync channel.
// ---------------------------------------------------------------------------------------
TEST_F(SchemaSyncTestFixture, GapA_NonSharedColumnIdDeterminism)
    {
    // OwnTable class — no shared-column strategy, so columns are non-shared (Gap A).
    Utf8CP schemaXml =
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="GapASchema" alias="ga" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="OwnTableClass">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>OwnTable</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="OwnProp1" typeName="int" />
                <ECProperty propertyName="OwnProp2" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml";

    ECDbHub hub;
    SchemaSyncDb schemaSyncDb("sync-db");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(schemaSyncDb.GetSyncDbUri(), "gap-a", false));
    b1->SaveChanges();
    b1->PullMergePush("init");

    // b1 imports first — reserves column ids in sync-db.
    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, SchemaItem(schemaXml), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("b1 import");

    // b2 imports through the same sync channel — reads reserved ids.
    auto b2 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b2, SchemaItem(schemaXml), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    auto [id1_p1, ord1_p1] = GetColumnIdAndOrdinal(*b1, "OwnTableClass", "OwnProp1");
    auto [id2_p1, ord2_p1] = GetColumnIdAndOrdinal(*b2, "OwnTableClass", "OwnProp1");
    auto [id1_p2, ord1_p2] = GetColumnIdAndOrdinal(*b1, "OwnTableClass", "OwnProp2");
    auto [id2_p2, ord2_p2] = GetColumnIdAndOrdinal(*b2, "OwnTableClass", "OwnProp2");

    ASSERT_GT(id1_p1, UINT64_C(0)) << "b1 OwnProp1 column must be mapped";
    ASSERT_GT(id2_p1, UINT64_C(0)) << "b2 OwnProp1 column must be mapped";
    EXPECT_EQ(id1_p1, id2_p1) << "Gap A: OwnProp1 ec_Column.Id must match across briefcases";
    EXPECT_EQ(id1_p2, id2_p2) << "Gap A: OwnProp2 ec_Column.Id must match across briefcases";
    }

// ---------------------------------------------------------------------------------------
// Test 13: GapB_SystemColumnIdDeterminism
// Gap B: the ECInstanceId and ECClassId system columns of a newly-created physical table
// must receive content-keyed ids — identical across two briefcases.
// ---------------------------------------------------------------------------------------
TEST_F(SchemaSyncTestFixture, GapB_SystemColumnIdDeterminism)
    {
    Utf8CP schemaXml =
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="GapBSchema" alias="gb" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="TphClass">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="Prop1" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml";

    ECDbHub hub;
    SchemaSyncDb schemaSyncDb("sync-db");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(schemaSyncDb.GetSyncDbUri(), "gap-b", false));
    b1->SaveChanges();
    b1->PullMergePush("init");

    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, SchemaItem(schemaXml), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("b1 import");

    auto b2 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b2, SchemaItem(schemaXml), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    // The physical table name follows the default naming convention.
    Utf8CP tableName = "gb_TphClass";

    uint64_t instId_b1 = GetSystemColumnId(*b1, tableName, 1 /*ECInstanceId*/);
    uint64_t instId_b2 = GetSystemColumnId(*b2, tableName, 1 /*ECInstanceId*/);
    uint64_t classId_b1 = GetSystemColumnId(*b1, tableName, 2 /*ECClassId*/);
    uint64_t classId_b2 = GetSystemColumnId(*b2, tableName, 2 /*ECClassId*/);

    ASSERT_GT(instId_b1, UINT64_C(0))  << "b1 ECInstanceId column must exist in " << tableName;
    ASSERT_GT(instId_b2, UINT64_C(0))  << "b2 ECInstanceId column must exist in " << tableName;
    EXPECT_EQ(instId_b1, instId_b2)    << "Gap B: ECInstanceId column id must match across briefcases";
    ASSERT_GT(classId_b1, UINT64_C(0)) << "b1 ECClassId column must exist in " << tableName;
    EXPECT_EQ(classId_b1, classId_b2)  << "Gap B: ECClassId column id must match across briefcases";
    }

// ---------------------------------------------------------------------------------------
// Test 14: GapC_NavFKColumnIdDeterminism
// Gap C: the FK column (and RelECClassId column) produced for a navigation property must
// receive content-keyed ids — identical across two briefcases.
// ---------------------------------------------------------------------------------------
TEST_F(SchemaSyncTestFixture, GapC_NavFKColumnIdDeterminism)
    {
    // A simple one-to-many relationship with a nav property on the target class.
    Utf8CP schemaXml =
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="GapCSchema" alias="gc" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Parent">
                <ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="Child">
                <ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
                <ECNavigationProperty propertyName="Owner" relationshipName="OwnsChildren" direction="Backward" />
            </ECEntityClass>
            <ECRelationshipClass typeName="OwnsChildren" strength="referencing" strengthDirection="Forward" modifier="None">
                <Source multiplicity="(0..1)"  roleLabel="owns"     polymorphic="true"><Class class="Parent" /></Source>
                <Target multiplicity="(0..*)"  roleLabel="owned by" polymorphic="true"><Class class="Child"  /></Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

    ECDbHub hub;
    SchemaSyncDb schemaSyncDb("sync-db");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(schemaSyncDb.GetSyncDbUri(), "gap-c", false));
    b1->SaveChanges();
    b1->PullMergePush("init");

    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, SchemaItem(schemaXml), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("b1 import");

    auto b2 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b2, SchemaItem(schemaXml), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    // The nav property FK leaf is "Owner.Id"; "Owner.RelECClassId" may be virtual.
    auto [idB1_fk, ordB1_fk] = GetColumnIdAndOrdinal(*b1, "Child", "Owner.Id");
    auto [idB2_fk, ordB2_fk] = GetColumnIdAndOrdinal(*b2, "Child", "Owner.Id");

    ASSERT_GT(idB1_fk, UINT64_C(0)) << "b1 nav FK column (Owner.Id) must be mapped";
    ASSERT_GT(idB2_fk, UINT64_C(0)) << "b2 nav FK column (Owner.Id) must be mapped";
    EXPECT_EQ(idB1_fk, idB2_fk) << "Gap C: nav FK column id (Owner.Id) must match across briefcases";
    }

// ---------------------------------------------------------------------------------------
// Test 15: GapD_LinkTableReservation
// Gap D: importing a link-table relationship class must produce content-keyed ids for the
// link table (ec_Table), its system columns, and the relationship's property maps —
// identical across two briefcases.
// ---------------------------------------------------------------------------------------
TEST_F(SchemaSyncTestFixture, GapD_LinkTableReservation)
    {
    // M:N relationship → link table (no nav property, both multiplicities > 1).
    Utf8CP schemaXml =
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="GapDSchema" alias="gd" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Left">
                <ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="Right">
                <ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
            </ECEntityClass>
            <ECRelationshipClass typeName="LeftToRight" strength="referencing" strengthDirection="Forward" modifier="None">
                <Source multiplicity="(0..*)" roleLabel="left"  polymorphic="true"><Class class="Left"  /></Source>
                <Target multiplicity="(0..*)" roleLabel="right" polymorphic="true"><Class class="Right" /></Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

    ECDbHub hub;
    SchemaSyncDb schemaSyncDb("sync-db");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(schemaSyncDb.GetSyncDbUri(), "gap-d", false));
    b1->SaveChanges();
    b1->PullMergePush("init");

    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, SchemaItem(schemaXml), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("b1 import");

    auto b2 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b2, SchemaItem(schemaXml), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    Utf8CP linkTable = "gd_LeftToRight";

    // ec_Table id must be content-keyed and identical.
    uint64_t tblId_b1 = GetTableId(*b1, linkTable);
    uint64_t tblId_b2 = GetTableId(*b2, linkTable);
    ASSERT_GT(tblId_b1, UINT64_C(0)) << "link table must exist in b1";
    ASSERT_GT(tblId_b2, UINT64_C(0)) << "link table must exist in b2";
    EXPECT_EQ(tblId_b1, tblId_b2)    << "Gap D: link table ec_Table.Id must match across briefcases";

    // System column ids (ECInstanceId/ECClassId kind) must match.
    uint64_t instId_b1  = GetSystemColumnId(*b1, linkTable, 1);
    uint64_t instId_b2  = GetSystemColumnId(*b2, linkTable, 1);
    uint64_t classId_b1 = GetSystemColumnId(*b1, linkTable, 2);
    uint64_t classId_b2 = GetSystemColumnId(*b2, linkTable, 2);
    ASSERT_GT(instId_b1, UINT64_C(0)) << "b1 link table ECInstanceId column must exist";
    EXPECT_EQ(instId_b1, instId_b2)   << "Gap D: link table ECInstanceId column id must match";
    ASSERT_GT(classId_b1, UINT64_C(0)) << "b1 link table ECClassId column must exist";
    EXPECT_EQ(classId_b1, classId_b2)  << "Gap D: link table ECClassId column id must match";
    }

// ---------------------------------------------------------------------------------------
// Test 16: GapE_JoinedTableReservation
// Gap E: classes in a JoinedTablePerDirectSubclass hierarchy get their own physical table
// whose ec_Table.Id must be content-keyed and identical across two briefcases.
// ---------------------------------------------------------------------------------------
TEST_F(SchemaSyncTestFixture, GapE_JoinedTableReservation)
    {
    // Root has JoinedTablePerDirectSubclass — subclasses each get their own table.
    Utf8CP schemaXml =
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="GapESchema" alias="ge" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Base" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00" />
                </ECCustomAttributes>
                <ECProperty propertyName="BaseProp" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Sub1">
                <BaseClass>Base</BaseClass>
                <ECProperty propertyName="Sub1Prop" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml";

    ECDbHub hub;
    SchemaSyncDb schemaSyncDb("sync-db");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(schemaSyncDb.GetSyncDbUri(), "gap-e", false));
    b1->SaveChanges();
    b1->PullMergePush("init");

    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, SchemaItem(schemaXml), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("b1 import");

    auto b2 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b2, SchemaItem(schemaXml), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    // Sub1 gets its own joined table named after itself.
    Utf8CP joinedTable = "ge_Sub1";

    uint64_t tblId_b1 = GetTableId(*b1, joinedTable);
    uint64_t tblId_b2 = GetTableId(*b2, joinedTable);
    ASSERT_GT(tblId_b1, UINT64_C(0)) << "joined table ge_Sub1 must exist in b1";
    ASSERT_GT(tblId_b2, UINT64_C(0)) << "joined table ge_Sub1 must exist in b2";
    EXPECT_EQ(tblId_b1, tblId_b2)    << "Gap E: joined table ec_Table.Id must match across briefcases";

    // Sub1Prop column id in the joined table must also match.
    auto [colId_b1, ord_b1] = GetColumnIdAndOrdinal(*b1, "Sub1", "Sub1Prop");
    auto [colId_b2, ord_b2] = GetColumnIdAndOrdinal(*b2, "Sub1", "Sub1Prop");
    ASSERT_GT(colId_b1, UINT64_C(0)) << "Sub1Prop column must be mapped in b1";
    EXPECT_EQ(colId_b1, colId_b2)    << "Gap E: Sub1Prop column id must match across briefcases";
    }

// ---------------------------------------------------------------------------------------
// Test 17: GapF_IndexIdDeterminism
// Gap F: the auto-generated ECClassId index on a newly-created physical table must
// receive a content-keyed ec_Index.Id — identical across two briefcases.
// ---------------------------------------------------------------------------------------
TEST_F(SchemaSyncTestFixture, GapF_IndexIdDeterminism)
    {
    Utf8CP schemaXml =
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="GapFSchema" alias="gf" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="IndexedClass">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="Val" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml";

    ECDbHub hub;
    SchemaSyncDb schemaSyncDb("sync-db");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(schemaSyncDb.GetSyncDbUri(), "gap-f", false));
    b1->SaveChanges();
    b1->PullMergePush("init");

    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, SchemaItem(schemaXml), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("b1 import");

    auto b2 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b2, SchemaItem(schemaXml), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    Utf8CP tableName  = "gf_IndexedClass";
    Utf8CP indexName  = "ix_gf_IndexedClass_ecclassid";

    uint64_t idxId_b1 = GetIndexId(*b1, tableName, indexName);
    uint64_t idxId_b2 = GetIndexId(*b2, tableName, indexName);

    ASSERT_GT(idxId_b1, UINT64_C(0)) << "auto ECClassId index must exist in b1";
    ASSERT_GT(idxId_b2, UINT64_C(0)) << "auto ECClassId index must exist in b2";
    EXPECT_EQ(idxId_b1, idxId_b2)    << "Gap F: ECClassId index ec_Index.Id must match across briefcases";
    }

// ---------------------------------------------------------------------------------------
// Test 18: GapF_NavPropertyFKIndexDeterminism
// Gap F (nav index variant): the auto-generated FK index for a navigation property must
// receive a content-keyed ec_Index.Id — identical across two briefcases.
// ---------------------------------------------------------------------------------------
TEST_F(SchemaSyncTestFixture, GapF_NavPropertyFKIndexDeterminism)
    {
    Utf8CP schemaXml =
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="GapFNavSchema" alias="gfn" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Parent">
                <ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="Child">
                <ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
                <ECNavigationProperty propertyName="Owner" relationshipName="ParentOwnsChild" direction="Backward" />
            </ECEntityClass>
            <ECRelationshipClass typeName="ParentOwnsChild" strength="referencing" strengthDirection="Forward" modifier="None">
                <Source multiplicity="(0..1)"  roleLabel="owns"     polymorphic="true"><Class class="Parent" /></Source>
                <Target multiplicity="(0..*)"  roleLabel="owned by" polymorphic="true"><Class class="Child"  /></Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

    ECDbHub hub;
    SchemaSyncDb schemaSyncDb("sync-db");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(schemaSyncDb.GetSyncDbUri(), "gap-f-nav", false));
    b1->SaveChanges();
    b1->PullMergePush("init");

    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, SchemaItem(schemaXml), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("b1 import");

    auto b2 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b2, SchemaItem(schemaXml), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    Utf8CP childTable = "gfn_Child";

    // The nav-property FK index name follows ix_<table>_fk_<schemaAlias>_<relName>_target.
    Utf8String fkIndexName;
    fkIndexName.Sprintf("ix_%s_fk_gfn_ParentOwnsChild_target", childTable);

    uint64_t idxId_b1 = GetIndexId(*b1, childTable, fkIndexName.c_str());
    uint64_t idxId_b2 = GetIndexId(*b2, childTable, fkIndexName.c_str());

    ASSERT_GT(idxId_b1, UINT64_C(0)) << "nav FK index must exist in b1: " << fkIndexName;
    ASSERT_GT(idxId_b2, UINT64_C(0)) << "nav FK index must exist in b2: " << fkIndexName;
    EXPECT_EQ(idxId_b1, idxId_b2)    << "Gap F: nav FK index ec_Index.Id must match across briefcases";
    }

// ---------------------------------------------------------------------------------------
// Test 19: ClassHierarchyStore_GuaranteesCorrectSlotReuseAcrossBriefcases
// §3a.1b: The persisted class-hierarchy store must drive the slot-reuse decision even
// when the occupant class was reserved by a different briefcase and is absent from the
// current in-memory schema graph.
//
// Setup: b1 imports Schema1 v1.0.0 (ABC:TPH+ShareColumns, DEF:ABC).
//        b2 imports Schema1 v1.0.1 that adds a SIBLING class GHI:ABC with one property.
//        At the time b2 reserves, DEF is not in b2's in-memory schema graph but its
//        hierarchy entry (DEF ancestor = ABC) is in the persisted hierarchy store.
//        The slot-reuse test must therefore treat GHI's new property slot correctly:
//        GHI is a sibling of DEF (neither is an ancestor nor a descendant of the other),
//        so GHI MUST reuse DEF's slot rather than extending the high-water mark.
// ---------------------------------------------------------------------------------------
TEST_F(SchemaSyncTestFixture, ClassHierarchyStore_GuaranteesCorrectSlotReuseAcrossBriefcases)
    {
    // v1.0.0: ABC (TPH root, ShareColumns) + DEF : ABC (one property).
    Utf8CP schema_v100 =
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="HierTest" alias="ht" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="ABC" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow></ShareColumns>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="DEF">
                <BaseClass>ABC</BaseClass>
                <ECProperty propertyName="defProp" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml";

    // v1.0.1: adds a sibling class GHI:ABC with one property.
    // GHI and DEF are siblings — neither is an ancestor/descendant of the other.
    // The slot-reuse rule: GHI's property MAY reuse DEF's shared slot.
    Utf8CP schema_v101 =
        R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="HierTest" alias="ht" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="ABC" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow></ShareColumns>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="DEF">
                <BaseClass>ABC</BaseClass>
                <ECProperty propertyName="defProp" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="GHI">
                <BaseClass>ABC</BaseClass>
                <ECProperty propertyName="ghiProp" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml";

    ECDbHub hub;
    SchemaSyncDb schemaSyncDb("sync-db");

    // b1 imports v1.0.0 — reserves defProp into slot ordinal N and writes the hierarchy
    // store (DEF ancestors = {ABC}).
    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(schemaSyncDb.GetSyncDbUri(), "hier-test", false));
    b1->SaveChanges();
    b1->PullMergePush("init");

    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b1, SchemaItem(schema_v100), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("b1 v1.0.0");

    auto [defColId_b1, defOrd_b1] = GetColumnIdAndOrdinal(*b1, "DEF", "defProp");
    ASSERT_GT(defColId_b1, UINT64_C(0)) << "defProp must be mapped in b1";
    ASSERT_GE(defOrd_b1, 0);

    // b2 imports v1.0.1 directly (DEF is in b2's base from the push, GHI is new).
    // b2's in-memory import graph for v1.0.1 contains ABC, DEF, and GHI.
    // Because the hierarchy store was seeded by b1 (DEF→ABC entry), b2 knows that
    // GHI (a sibling of DEF) may reuse DEF's slot.
    auto b2 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b2, SchemaItem(schema_v101), SchemaManager::SchemaImportOptions::None, schemaSyncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    auto [defColId_b2, defOrd_b2] = GetColumnIdAndOrdinal(*b2, "DEF", "defProp");
    auto [ghiColId_b2, ghiOrd_b2] = GetColumnIdAndOrdinal(*b2, "GHI", "ghiProp");

    ASSERT_GT(defColId_b2, UINT64_C(0)) << "defProp must be mapped in b2";
    ASSERT_GT(ghiColId_b2, UINT64_C(0)) << "ghiProp must be mapped in b2";

    // defProp ids must be identical across b1 and b2.
    EXPECT_EQ(defColId_b1, defColId_b2) << "defProp column id must match across briefcases";
    EXPECT_EQ(defOrd_b1,   defOrd_b2)   << "defProp column ordinal must match";

    // ghiProp is a SIBLING of defProp (GHI and DEF both inherit from ABC), so the
    // hierarchy store must allow slot reuse: ghiProp must land in the SAME slot as defProp.
    EXPECT_EQ(ghiOrd_b2, defOrd_b2)
        << "§3a.1b: ghiProp (sibling of defProp) must reuse defProp's shared slot "
           "(hierarchy store must classify GHI as sibling, not descendant, of DEF)";
    }

END_ECDBUNITTESTS_NAMESPACE
