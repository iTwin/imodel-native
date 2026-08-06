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

/**
 * SchemaSync "upstream" (v2) spike tests.
 *
 * The v2 design inverts v1's direction: a schema import runs in the sync db FIRST, which decides ids
 * and physical layout exactly once, and the briefcase then adopts that answer instead of computing
 * its own. These tests establish whether the two premises that design rests on actually hold:
 *
 *   1. The sync db can run a schema import at all. It is a real ECDb with the full ec_* mirror, but
 *      Init dropped its data tables, and it has never been an import target before.
 *   2. The mapper reaches the SAME answer in the sync db as it would in a briefcase. If the two
 *      disagree, "decide once in the sync db, adopt everywhere" is not a valid substitute for
 *      "every briefcase decides for itself", and the design needs rework.
 *
 * These are spikes, not the final mechanism - they call ImportSchemas directly rather than through
 * any new API, precisely so that they measure the existing behaviour and nothing else.
 */
struct SchemaSyncUpstreamTestFixture : SchemaSyncTestFixture {};

namespace {

// A schema whose layout decisions are interesting: TablePerHierarchy with shared columns, so the
// mapper has to allocate shared-column ordinals rather than one column per property.
SchemaItem SharedColumnSchema(Utf8CP version = "01.00.00", int propertyCount = 8) {
    Utf8String properties;
    for (int i = 1; i <= propertyCount; ++i)
        properties.append(SqlPrintfString("<ECProperty propertyName=\"p%d\" typeName=\"int\" />\n", i).GetUtf8CP());

    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="UpstreamTest" alias="ut" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="baseProp" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Derived">
                <BaseClass>Base</BaseClass>
                %s
            </ECEntityClass>
        </ECSchema>)xml", version, properties.c_str());
    return SchemaItem(xml);
}

// A second schema that shares nothing with UpstreamTest - no reference either way, its own class,
// its own table. Used to prove that adopting one schema does not drag in the other.
SchemaItem UnrelatedSchema(Utf8CP version = "01.00.00") {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="UnrelatedTest" alias="unrel" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Loner">
                <ECProperty propertyName="value" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml", version);
    return SchemaItem(xml);
}

// A schema that references UpstreamTest, so adopting it must pull UpstreamTest along.
SchemaItem ReferencingSchema(Utf8CP version = "01.00.00") {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="ReferencingTest" alias="ref" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="UpstreamTest" version="01.00.00" alias="ut"/>
            <ECEntityClass typeName="Extra">
                <BaseClass>ut:Base</BaseClass>
                <ECProperty propertyName="extraProp" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml", version);
    return SchemaItem(xml);
}

// Compares every ec_ table between two files and reports, per table, where they differ. The point is
// diagnostic: a single failing run should say WHICH filter rule is wrong, not merely that some hash
// did not match. ec_cache_* is skipped - it is derived and rebuilt locally.
void ExpectEcTablesIdentical(ECDbR actual, ECDbR expected, Utf8CP context) {
    bvector<Utf8String> tables;
    Statement tableStmt;
    ASSERT_EQ(BE_SQLITE_OK, tableStmt.Prepare(expected, "SELECT name FROM main.sqlite_master WHERE type='table' AND name LIKE 'ec\\_%' ESCAPE '\\' AND name NOT LIKE 'ec\\_cache\\_%' ESCAPE '\\' ORDER BY name"));
    while (tableStmt.Step() == BE_SQLITE_ROW)
        tables.push_back(tableStmt.GetValueText(0));
    ASSERT_FALSE(tables.empty());

    auto rowCount = [](ECDbR db, Utf8CP table) -> int64_t {
        Statement stmt;
        if (stmt.Prepare(db, SqlPrintfString("SELECT COUNT(*) FROM main.[%s]", table).GetUtf8CP()) != BE_SQLITE_OK)
            return -1;
        return stmt.Step() == BE_SQLITE_ROW ? stmt.GetValueInt64(0) : -1;
    };
    auto contentHash = [](ECDbR db, Utf8CP table) -> Utf8String {
        Statement stmt;
        if (stmt.Prepare(db, SqlPrintfString("SELECT hex(sha3_query('SELECT * FROM [%s] ORDER BY ROWID'))", table).GetUtf8CP()) != BE_SQLITE_OK)
            return "";
        return stmt.Step() == BE_SQLITE_ROW ? Utf8String(stmt.GetValueText(0)) : Utf8String("");
    };

    for (auto const& table : tables) {
        const auto actualCount = rowCount(actual, table.c_str());
        const auto expectedCount = rowCount(expected, table.c_str());
        EXPECT_EQ(expectedCount, actualCount) << context << ": row count differs in " << table.c_str();
        if (actualCount == expectedCount)
            EXPECT_STREQ(contentHash(expected, table.c_str()).c_str(), contentHash(actual, table.c_str()).c_str())
                << context << ": row CONTENT differs in " << table.c_str() << " (same count, different values)";
    }
}

// Does this file know the named schema?
bool HasSchema(ECDbR db, Utf8CP schemaName) {
    Statement stmt;
    if (stmt.Prepare(db, "SELECT 1 FROM main.ec_Schema WHERE Name=?") != BE_SQLITE_OK)
        return false;
    stmt.BindText(1, schemaName, Statement::MakeCopy::No);
    return stmt.Step() == BE_SQLITE_ROW;
}

// A TPH root with a shared-column pool. Two briefcases adding properties under it compete for the
// same pool, which is the situation that silently corrupted data under the reservation design.
SchemaItem MachinerySchema(Utf8CP version, bool withRating) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Machinery" alias="mch" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Machine">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>8</MaxSharedColumnsBeforeOverflow>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="name" typeName="string" />
                %s
            </ECEntityClass>
        </ECSchema>)xml", version, withRating ? R"xml(<ECProperty propertyName="rating" typeName="int" />)xml" : "");
    return SchemaItem(xml);
}

// A separate schema that adds a subclass under Machinery's hierarchy - so its property lands in the
// same shared-column pool as anything added to Machine itself.
SchemaItem TankSchema() {
    return SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="DemoB" alias="dmb" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="Machinery" version="01.00.00" alias="mch"/>
            <ECEntityClass typeName="Tank">
                <BaseClass>mch:Machine</BaseClass>
                <ECProperty propertyName="volume" typeName="double" />
            </ECEntityClass>
        </ECSchema>)xml");
}

// Runs one import through the sync db and adopts the result - the two steps of the v2 flow, which
// the real front door will eventually perform under a single container lock.
void ImportUpstream(TrackedECDb& briefcase, SchemaSyncDb& syncDb, std::vector<SchemaItem> const& schemas, bvector<Utf8String> const& adopt) {
    syncDb.WithReadWrite([&](ECDbR sync) {
        ASSERT_EQ(SchemaImportResult::OK, SchemaSyncTestFixture::ImportSchemas(sync, schemas));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
    });
    ASSERT_EQ(SchemaSync::Status::OK, briefcase.Schemas().GetSchemaSync().AdoptSchemas(syncDb.GetSyncDbUri(), adopt));
    ASSERT_EQ(BE_SQLITE_OK, briefcase.SaveChanges());
}

// After merging someone else's schema changeset, a briefcase holds the ec_ rows but not the physical
// columns: adopt deliberately does not track DDL, exactly as v1's pull does not, so every briefcase
// derives its own. This stands in for the post-merge hook the backend runs in the real system.
void MaterializeAfterMerge(TrackedECDb& db) {
    ASSERT_EQ(SchemaSync::Status::OK, db.Schemas().GetSchemaSync().UpdateDbSchema());
    ASSERT_EQ(BE_SQLITE_OK, db.SaveChanges());
}

// Which physical column does this property end up in?
Utf8String ColumnOf(ECDbR db, Utf8CP schemaName, Utf8CP className, Utf8CP accessString) {
    Statement stmt;
    if (stmt.Prepare(db, R"sql(
        SELECT col.Name FROM main.ec_PropertyMap pm
        JOIN main.ec_Column col ON col.Id = pm.ColumnId
        JOIN main.ec_PropertyPath pp ON pp.Id = pm.PropertyPathId
        JOIN main.ec_Class c ON c.Id = pm.ClassId
        JOIN main.ec_Schema s ON s.Id = c.SchemaId
        WHERE s.Name=? AND c.Name=? AND pp.AccessString=?)sql") != BE_SQLITE_OK)
        return "";
    stmt.BindText(1, schemaName, Statement::MakeCopy::No);
    stmt.BindText(2, className, Statement::MakeCopy::No);
    stmt.BindText(3, accessString, Statement::MakeCopy::No);
    return stmt.Step() == BE_SQLITE_ROW ? Utf8String(stmt.GetValueText(0)) : Utf8String("");
}

// Compact per-stage state dump. Used to localise where rows appear or vanish across a multi-step
// scenario, so one test run can say which step lost them rather than only that the end state is wrong.
void DumpFingerprint(ECDbR db, Utf8CP label) {
    Utf8String line;
    for (auto table : { "ec_Schema", "ec_SchemaReference", "ec_Class", "ec_ClassHasBaseClasses", "ec_Property", "ec_PropertyMap", "ec_Column" }) {
        Statement stmt;
        if (stmt.Prepare(db, SqlPrintfString("SELECT COUNT(*) FROM main.[%s]", table).GetUtf8CP()) != BE_SQLITE_OK)
            continue;
        if (stmt.Step() != BE_SQLITE_ROW)
            continue;
        line.append(SqlPrintfString(" %s=%d", table, stmt.GetValueInt(0)).GetUtf8CP());
    }
    printf("[upstream] %-40s%s\n", label, line.c_str());
}

} // namespace

// ---------------------------------------------------------------------------------------
// N1 spike, part 1: can the sync db run a schema import at all?
//
// The sync db is a real ECDb (same EC profile, full ec_* mirror) but Init dropped its data tables.
// An import there therefore has to both write ec_* rows and create the physical tables its mapping
// implies - inside a file that has none of them.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, ImportRunsInsideSyncDb)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-import-runs");
    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    syncDb.WithReadWrite([&](ECDbR sync) {
        // The sync db must present itself as a usable ECDb before an import can be expected to work.
        ASSERT_TRUE(sync.IsDbOpen());
        ASSERT_FALSE(sync.Schemas().GetSchemas(false).empty()) << "sync db should carry the briefcase's schemas after Init";

        // The import that the whole design depends on.
        ASSERT_EQ(SchemaImportResult::OK, ImportSchema(sync, SharedColumnSchema()))
            << "importing into the sync db failed - the v2 design does not survive this";
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());

        // The schema is really there, as metadata.
        ASSERT_TRUE(sync.Schemas().ContainsSchema("UpstreamTest"));
        ASSERT_NE(nullptr, sync.Schemas().GetClass("UpstreamTest", "Derived"));

        // ...and the mapping was recorded, which is the part the briefcase will later adopt.
        Statement stmt;
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(sync, R"sql(
            SELECT COUNT(*) FROM ec_PropertyMap pm
            JOIN ec_Class c ON c.Id = pm.ClassId
            JOIN ec_Schema s ON s.Id = c.SchemaId
            WHERE s.Name = 'UpstreamTest')sql"));
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        ASSERT_GT(stmt.GetValueInt(0), 0) << "no property maps recorded for the imported schema";

        ASSERT_TRUE(ForeignkeyCheck(sync)) << "import left the sync db with broken foreign keys";
    });
    }

// ---------------------------------------------------------------------------------------
// N1 spike, part 2: does the sync db's mapper reach the same answer as a briefcase's?
//
// This is the load-bearing assumption of v2. Both files start from the same ec_* state (Init pushed
// the briefcase's rows into the sync db), so if the mapper is a pure function of that state plus the
// incoming schema, both must produce identical ec_* content. The ecdb_schema checksum covers the
// logical rows; ecdb_map covers ids, tables, columns and property maps - the decisions we intend to
// stop recomputing per briefcase.
//
// sqlite_schema is deliberately NOT compared: the sync db has no data tables, so its physical shape
// differs by construction. That difference is exactly what step 2 of the design reconstructs locally.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, SyncDbMappingMatchesBriefcaseMapping)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-mapping-matches");

    // The control briefcase must be created BEFORE schema sync is initialised and pushed: once the
    // init changeset lands, every briefcase derived from it has schema sync enabled and can no longer
    // perform a plain import. This one stays a pristine, sync-free briefcase.
    auto b1 = hub.CreateBriefcase();
    auto control = hub.CreateBriefcase();

    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("init schema sync");

    const auto schema = SharedColumnSchema();

    // Control: an ordinary briefcase import, no schema sync involved.
    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*control, schema));
    ASSERT_EQ(BE_SQLITE_OK, control->SaveChanges());
    const auto briefcaseSchemaHash = GetSchemaHash(*control);
    const auto briefcaseMapHash = GetMapHash(*control);
    ASSERT_FALSE(briefcaseSchemaHash.empty());
    ASSERT_FALSE(briefcaseMapHash.empty());

    // Subject: the same import, run in the sync db instead.
    syncDb.WithReadWrite([&](ECDbR sync) {
        ASSERT_EQ(SchemaImportResult::OK, ImportSchema(sync, schema));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());

        EXPECT_STREQ(briefcaseSchemaHash.c_str(), GetSchemaHash(sync).c_str())
            << "logical schema rows differ between sync db and briefcase";
        EXPECT_STREQ(briefcaseMapHash.c_str(), GetMapHash(sync).c_str())
            << "MAPPING differs between sync db and briefcase - ids/columns were decided differently, "
               "so the briefcase cannot simply adopt the sync db's answer";
    });
    }

// ---------------------------------------------------------------------------------------
// N1 spike, part 3: does the agreement survive a second, dependent import?
//
// One import proves little: both files were pristine. The real question is whether the sync db keeps
// agreeing once layout decisions accumulate - a second import that spills into an overflow table has
// to reuse and extend the first one's shared-column allocations, which is where order dependence and
// local file state actually bite.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, MappingStillMatchesAfterDependentImport)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-mapping-second-import");

    // Control created before init is pushed - see the note in SyncDbMappingMatchesBriefcaseMapping.
    auto b1 = hub.CreateBriefcase();
    auto control = hub.CreateBriefcase();

    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("init schema sync");

    // First import: 8 properties against a 4-column shared budget, so it already overflows.
    const auto first = SharedColumnSchema("01.00.00", 8);
    // Second import: same schema grown to 20 properties, forcing further allocation on top of the
    // layout the first import established.
    const auto second = SharedColumnSchema("01.00.01", 20);

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*control, first));
    ASSERT_EQ(BE_SQLITE_OK, control->SaveChanges());
    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*control, second));
    ASSERT_EQ(BE_SQLITE_OK, control->SaveChanges());
    const auto briefcaseSchemaHash = GetSchemaHash(*control);
    const auto briefcaseMapHash = GetMapHash(*control);

    syncDb.WithReadWrite([&](ECDbR sync) {
        ASSERT_EQ(SchemaImportResult::OK, ImportSchema(sync, first));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
        ASSERT_EQ(SchemaImportResult::OK, ImportSchema(sync, second));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());

        EXPECT_STREQ(briefcaseSchemaHash.c_str(), GetSchemaHash(sync).c_str())
            << "logical schema rows diverged after the second import";
        EXPECT_STREQ(briefcaseMapHash.c_str(), GetMapHash(sync).c_str())
            << "mapping diverged after the second import - accumulated layout state is not reproduced "
               "identically in the sync db";
    });
    }

// ---------------------------------------------------------------------------------------
// N1 spike, part 4: what does an import create physically inside the sync db?
//
// Not an assertion of desired behaviour so much as a record of actual behaviour. The design assumes
// the sync db's physical tables are throwaway scratch - nobody reads data from it - and that the
// briefcase reconstructs its own tables from the adopted ec_* rows. This test pins down what actually
// lands there, so a later change to that behaviour is visible rather than silent.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, RecordsPhysicalTablesCreatedInSyncDb)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-physical-tables");
    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    auto countRealTables = [](ECDbCR db) {
        Statement stmt;
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "SELECT COUNT(*) FROM main.sqlite_master WHERE type='table' AND name NOT LIKE 'ec\\_%' ESCAPE '\\' AND name NOT LIKE 'be\\_%' ESCAPE '\\' AND name NOT LIKE 'sqlite\\_%' ESCAPE '\\'"));
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
        return stmt.GetValueInt(0);
    };

    syncDb.WithReadWrite([&](ECDbR sync) {
        const int before = countRealTables(sync);

        ASSERT_EQ(SchemaImportResult::OK, ImportSchema(sync, SharedColumnSchema()));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());

        const int after = countRealTables(sync);
        // Whatever the numbers are, the point is that they are stable and that the import did not
        // fail for want of pre-existing data tables. Log them so the spike leaves a record.
        printf("[upstream spike] non-profile tables in sync db: before=%d after=%d\n", before, after);
        EXPECT_GE(after, before) << "an import should never remove tables from the sync db";
    });
    }

//=======================================================================================
// Step 2: adopting the sync db's answer into a briefcase.
//
// Everything above establishes that the sync db can decide. These test that a briefcase can take
// that decision over - copying exactly the rows that belong to the schemas it asked for, and
// materialising the physical tables those rows imply.
//=======================================================================================

// ---------------------------------------------------------------------------------------
// The completeness oracle.
//
// A briefcase that is otherwise level with the sync db, and then adopts the one schema the sync db
// gained, must end up byte-for-byte identical to it in every ec_ table. Too few rows copied and the
// content differs; the per-table comparison says which rule is at fault.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, AdoptMakesBriefcaseMatchSyncDb)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-adopt-complete");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("init schema sync");

    // b2 is level with the sync db and has schema sync enabled.
    auto b2 = hub.CreateBriefcase();

    // Step 1: the import happens in the sync db.
    syncDb.WithReadWrite([&](ECDbR sync) {
        ASSERT_EQ(SchemaImportResult::OK, ImportSchema(sync, SharedColumnSchema()));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
    });

    // Step 2: the briefcase adopts it.
    ASSERT_EQ(SchemaSync::Status::OK, b2->Schemas().GetSchemaSync().AdoptSchemas(syncDb.GetSyncDbUri(), { "UpstreamTest" }));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    ASSERT_TRUE(HasSchema(*b2, "UpstreamTest"));

    syncDb.WithReadOnly([&](ECDbR sync) {
        ExpectEcTablesIdentical(*b2, sync, "after adopting the only new schema");
    });
    }

// ---------------------------------------------------------------------------------------
// The filtering oracle, and the reason v2 filters at all.
//
// Two schemas land in the sync db; the briefcase asks for one. The other must not appear - it stands
// for a schema some other briefcase imported and has not pushed yet, which has no business showing
// up in this briefcase's changeset.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, AdoptLeavesUnrelatedSchemasBehind)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-adopt-filtered");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("init schema sync");

    auto b2 = hub.CreateBriefcase();

    syncDb.WithReadWrite([&](ECDbR sync) {
        ASSERT_EQ(SchemaImportResult::OK, ImportSchema(sync, SharedColumnSchema()));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
        ASSERT_EQ(SchemaImportResult::OK, ImportSchema(sync, UnrelatedSchema()));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
    });

    ASSERT_EQ(SchemaSync::Status::OK, b2->Schemas().GetSchemaSync().AdoptSchemas(syncDb.GetSyncDbUri(), { "UpstreamTest" }));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    EXPECT_TRUE(HasSchema(*b2, "UpstreamTest")) << "the requested schema was not adopted";
    EXPECT_FALSE(HasSchema(*b2, "UnrelatedTest")) << "an unrequested schema leaked into the briefcase";

    // Nothing belonging to the unrelated schema may have come along either.
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*b2, "SELECT COUNT(*) FROM main.ec_Class WHERE Name='Loner'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(0, stmt.GetValueInt(0)) << "the unrelated schema's class leaked in";

    // ...and the briefcase must still be internally consistent.
    EXPECT_TRUE(ForeignkeyCheck(*b2)) << "filtered adopt left dangling foreign keys";
    }

// ---------------------------------------------------------------------------------------
// The closure oracle.
//
// Asking for a schema means asking for everything it stands on. A briefcase that adopts a schema
// referencing another must receive both, or it holds rows pointing at classes it does not have.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, AdoptPullsReferencedSchemas)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-adopt-closure");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("init schema sync");

    auto b2 = hub.CreateBriefcase();

    syncDb.WithReadWrite([&](ECDbR sync) {
        ASSERT_EQ(SchemaImportResult::OK, ImportSchemas(sync, { SharedColumnSchema(), ReferencingSchema() }));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
    });

    // Only the referencing schema is named; the referenced one has to follow on its own.
    ASSERT_EQ(SchemaSync::Status::OK, b2->Schemas().GetSchemaSync().AdoptSchemas(syncDb.GetSyncDbUri(), { "ReferencingTest" }));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    EXPECT_TRUE(HasSchema(*b2, "ReferencingTest"));
    EXPECT_TRUE(HasSchema(*b2, "UpstreamTest")) << "the referenced schema did not come along - closure is incomplete";
    EXPECT_TRUE(ForeignkeyCheck(*b2));

    syncDb.WithReadOnly([&](ECDbR sync) {
        ExpectEcTablesIdentical(*b2, sync, "after adopting a schema plus its reference");
    });
    }

// ---------------------------------------------------------------------------------------
// The point of the whole exercise: an adopted schema has to be usable.
//
// Copying metadata is not enough - the physical tables the adopted rows describe must exist locally
// and agree with them, or the first insert fails. This is the end-to-end proof that step 2 leaves a
// working briefcase rather than a plausible-looking one.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, AdoptedSchemaAcceptsData)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-adopt-usable");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("init schema sync");

    auto b2 = hub.CreateBriefcase();

    syncDb.WithReadWrite([&](ECDbR sync) {
        ASSERT_EQ(SchemaImportResult::OK, ImportSchema(sync, SharedColumnSchema()));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
    });

    ASSERT_EQ(SchemaSync::Status::OK, b2->Schemas().GetSchemaSync().AdoptSchemas(syncDb.GetSyncDbUri(), { "UpstreamTest" }));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO ut.Derived(baseProp,p1,p8) VALUES('hello',42,99)"))
        << "could not prepare an insert against the adopted class";
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << "insert into the adopted class failed";
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    // p1 and p8 straddle the shared-column budget, so reading both back proves the overflow table
    // was materialised too, not just the primary one.
    ECSqlStatement select;
    ASSERT_EQ(ECSqlStatus::Success, select.Prepare(*b2, "SELECT baseProp,p1,p8 FROM ut.Derived WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, select.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, select.Step());
    EXPECT_STREQ("hello", select.GetValueText(0));
    EXPECT_EQ(42, select.GetValueInt(1));
    EXPECT_EQ(99, select.GetValueInt(2));
    }

//=======================================================================================
// Does v2 actually fix the failures that killed the earlier designs?
//
// The reservation design failed on three concurrent-import cases (issue 2192). These tests take the
// two that a mapping authority is supposed to fix and check that it does. They use no new production
// code - only the two steps proven above - so a failure here is a finding about the design, not a
// bug in a helper.
//=======================================================================================

// ---------------------------------------------------------------------------------------
// The corruption case the whole redesign is for.
//
// Two briefcases import concurrently, neither having seen the other's changeset. One adds a property
// to a shared-column class; the other adds a subclass with its own property. Both are textbook
// additive updates. Under per-briefcase mapping each mapper, looking at its own file, put both
// properties in the SAME shared column - the merge stayed clean and one physical cell silently held
// both values. With the sync db deciding, the second import sees the first one's allocation.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, ConcurrentImportsDoNotShareAColumn)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-concurrent-columns");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("init schema sync");

    auto b2 = hub.CreateBriefcase();

    // Common starting point: both briefcases know Machinery 1.0.0.
    ImportUpstream(*b1, syncDb, { MachinerySchema("01.00.00", false) }, { "Machinery" });
    b1->PullMergePush("add Machinery");
    b2->PullMergePush("pick up Machinery");
    MaterializeAfterMerge(*b2);
    ASSERT_TRUE(HasSchema(*b2, "Machinery"));

    // Now the two imports race. Neither briefcase pushes before the other imports, so neither can
    // see the other's change except through the sync db.
    ImportUpstream(*b1, syncDb, { MachinerySchema("01.00.01", true) }, { "Machinery" });
    ImportUpstream(*b2, syncDb, { TankSchema() }, { "DemoB" });

    DumpFingerprint(*b1, "b1 after adopting Machinery 1.0.1");
    DumpFingerprint(*b2, "b2 after adopting DemoB");
    syncDb.WithReadOnly([&](ECDbR sync) { DumpFingerprint(sync, "sync db after both imports"); });

    // b2 asked only for DemoB, but DemoB references Machinery, so b2 picks up b1's still-unpushed
    // 1.0.1 through the closure. That is the agreed "updating references is fair game" behaviour.
    EXPECT_TRUE(HasSchema(*b2, "DemoB"));
    Statement ratingCheck;
    ASSERT_EQ(BE_SQLITE_OK, ratingCheck.Prepare(*b2, "SELECT COUNT(*) FROM main.ec_Property WHERE Name='rating'"));
    ASSERT_EQ(BE_SQLITE_ROW, ratingCheck.Step());
    EXPECT_EQ(1, ratingCheck.GetValueInt(0)) << "the referenced schema's new property did not come along";

    // Check the adopt landed completely BEFORE any changeset traffic. If this holds and the final
    // comparison still fails, the loss happened during the exchange, not during adopt.
    EXPECT_FALSE(ColumnOf(*b2, "DemoB", "Tank", "volume").empty())
        << "volume is not mapped in b2 immediately after adopt - the loss is in AdoptSchemas";
    {
    Statement tankBase;
    ASSERT_EQ(BE_SQLITE_OK, tankBase.Prepare(*b2, R"sql(
        SELECT COUNT(*) FROM main.ec_ClassHasBaseClasses hb
        JOIN main.ec_Class c ON c.Id = hb.ClassId WHERE c.Name='Tank')sql"));
    ASSERT_EQ(BE_SQLITE_ROW, tankBase.Step());
    EXPECT_EQ(1, tankBase.GetValueInt(0)) << "Tank's base-class link missing in b2 right after adopt";
    }

    // b2 holds both changes already - its own DemoB and, through the closure, b1's still-unpushed
    // Machinery 1.0.1 - so the allocation can be judged here, before any changeset traffic. That
    // matters: the allocation is the thing v2 changes, and it is settled the moment the sync db
    // decides it.
    for (auto* db : { b2.get() }) {
        const auto ratingCol = ColumnOf(*db, "Machinery", "Machine", "rating");
        const auto volumeCol = ColumnOf(*db, "DemoB", "Tank", "volume");
        EXPECT_FALSE(ratingCol.empty()) << "rating is not mapped";
        EXPECT_FALSE(volumeCol.empty()) << "volume is not mapped";
        EXPECT_STRNE(ratingCol.c_str(), volumeCol.c_str())
            << "rating and volume were double-booked into the same shared column - this is the "
               "silent corruption v2 exists to prevent";
    }

    // And the data has to behave: writing one property must not be readable as the other.
    ECInstanceKey key;
    {
    ECSqlStatement insert;
    ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(*b2, "INSERT INTO dmb.Tank(name,rating,volume) VALUES('t1',7,99.5)"));
    ASSERT_EQ(BE_SQLITE_DONE, insert.Step(key));
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    ECSqlStatement select;
    ASSERT_EQ(ECSqlStatus::Success, select.Prepare(*b2, "SELECT rating,volume FROM dmb.Tank WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, select.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, select.Step());
    EXPECT_EQ(7, select.GetValueInt(0)) << "rating was clobbered by volume";
    EXPECT_DOUBLE_EQ(99.5, select.GetValueDouble(1)) << "volume was clobbered by rating";
    }

// ---------------------------------------------------------------------------------------
// Convergence across the changeset exchange.
//
// The same scenario carried through to both briefcases exchanging changesets. This failed until the
// conflict policy was fixed: applying a changeset that re-inserts rows the briefcase already holds
// used to resolve as Replace, which DELETEs the existing row before re-inserting it, and almost
// every ec_ foreign key is ON DELETE CASCADE - so the delete took the row's children with it and the
// re-insert restored only the parent. Measured losses matched exactly: ec_SchemaReference -1
// (DemoB->Machinery), ec_ClassHasBaseClasses -1 (Tank->Machine), ec_PropertyMap -5 (Tank's),
// ec_Column -1 (volume), while ec_Schema and ec_Class stayed level because those rows were deleted
// and immediately re-created.
//
// Under v1 that was an edge case, because each briefcase allocated its own ids and rarely held rows
// byte-identical to someone else's. Under v2 every briefcase gets its rows from the same authority,
// so receiving a changeset full of rows you already hold is the ordinary path - which is what makes
// the conflict policy a correctness concern rather than a tuning detail.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, ConcurrentImportsConvergeAfterExchange)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-converge-exchange");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("init schema sync");

    auto b2 = hub.CreateBriefcase();

    ImportUpstream(*b1, syncDb, { MachinerySchema("01.00.00", false) }, { "Machinery" });
    b1->PullMergePush("add Machinery");
    b2->PullMergePush("pick up Machinery");
    MaterializeAfterMerge(*b2);

    ImportUpstream(*b1, syncDb, { MachinerySchema("01.00.01", true) }, { "Machinery" });
    ImportUpstream(*b2, syncDb, { TankSchema() }, { "DemoB" });

    b1->PullMergePush("b1 pushes rating");
    b2->PullMergePush("b2 merges rating, pushes tank");
    MaterializeAfterMerge(*b2);
    b1->PullMergePush("b1 merges tank");
    MaterializeAfterMerge(*b1);

    ExpectEcTablesIdentical(*b2, *b1, "after both briefcases exchanged changesets");
    syncDb.WithReadOnly([&](ECDbR sync) {
        ExpectEcTablesIdentical(*b1, sync, "briefcase vs sync db after convergence");
    });
    }

// ---------------------------------------------------------------------------------------
// The authority catching what a reservation ledger could not.
//
// Two briefcases add the same property name with different types. A key-to-id ledger hands both the
// same id and cannot tell they disagree, so the first pusher silently wins and the loser's data is
// read through the wrong type. The sync db holds the actual definition, so the second import has
// something to be checked against - and must be refused rather than quietly accepted.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, SyncDbRefusesConflictingPropertyType)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-conflicting-type");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("init schema sync");

    ImportUpstream(*b1, syncDb, { MachinerySchema("01.00.00", false) }, { "Machinery" });

    // b1's version of 1.0.1 adds rating as an int, and lands in the sync db.
    ImportUpstream(*b1, syncDb, { MachinerySchema("01.00.01", true) }, { "Machinery" });

    // b2 now tries the same version number with a different type for the same property.
    auto conflicting = SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="Machinery" alias="mch" version="01.00.02" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Machine">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>8</MaxSharedColumnsBeforeOverflow>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="name" typeName="string" />
                <ECProperty propertyName="rating" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml");

    syncDb.WithReadWrite([&](ECDbR sync) {
        // Whether ECDb calls this an outright error or a data transform, the point is the same: it
        // is seen and refused, instead of being handed a reserved id and silently diverging.
        EXPECT_NE(SchemaImportResult::OK, ImportSchema(sync, conflicting))
            << "the sync db accepted a conflicting property type - the authority is not authoritative";
    });
    }

// ---------------------------------------------------------------------------------------
// The remap gate, which v2 relies on to keep the update path additive.
//
// Step 1 runs with data transforms disallowed, so anything that would move existing data has to be
// refused here and routed to the upgrade front door instead. If this ever silently succeeded, the
// sync db would hold a layout no briefcase could reach without moving its own data.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, SyncDbRefusesImportNeedingDataTransform)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-remap-refused");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("init schema sync");

    // Start with the same property on two sibling subclasses, deliberately landing in DIFFERENT
    // shared columns: LeafA declares a filler property first, so its movingProp takes the later
    // slot, while LeafB - whose rows are disjoint from LeafA's - reuses the earlier one. Shared
    // columns are essential here: every remap query in RemapManager filters on ColumnKind = 4, so a
    // property sitting in a dedicated column is never a remap candidate and cannot trip the gate.
    auto initial = SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="RemapTest" alias="rmp" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>8</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="baseProp" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="LeafA">
                <BaseClass>Base</BaseClass>
                <ECProperty propertyName="filler" typeName="string" />
                <ECProperty propertyName="movingProp" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="LeafB">
                <BaseClass>Base</BaseClass>
                <ECProperty propertyName="movingProp" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml");

    ImportUpstream(*b1, syncDb, { initial }, { "RemapTest" });

    // Confirm the premise before relying on it. If the siblings happened to share a slot, hoisting
    // would move nothing and the rest of the test would prove nothing.
    syncDb.WithReadOnly([&](ECDbR sync) {
        const auto colA = ColumnOf(sync, "RemapTest", "LeafA", "movingProp");
        const auto colB = ColumnOf(sync, "RemapTest", "LeafB", "movingProp");
        printf("[upstream] LeafA.movingProp=%s LeafB.movingProp=%s\n", colA.c_str(), colB.c_str());
        ASSERT_STRNE(colA.c_str(), colB.c_str())
            << "the siblings already share a column, so hoisting cannot force a move - this "
               "scenario no longer tests what it claims to";
    });

    // Now hoist the property onto their common ancestor. Both siblings' copies become overrides of
    // the ancestor's property and must consolidate into one column, so at least one has to move.
    auto hoisted = SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="RemapTest" alias="rmp" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>8</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="baseProp" typeName="string" />
                <ECProperty propertyName="movingProp" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="LeafA">
                <BaseClass>Base</BaseClass>
                <ECProperty propertyName="filler" typeName="string" />
                <ECProperty propertyName="movingProp" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="LeafB">
                <BaseClass>Base</BaseClass>
                <ECProperty propertyName="movingProp" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml");

    syncDb.WithReadWrite([&](ECDbR sync) {
        const auto result = ImportSchema(sync, hoisted);
        // The sync db holds no data rows, but that is irrelevant: the transform statement list is
        // built from mapping changes, not from row counts, so the gate still fires here.
        EXPECT_NE(SchemaImportResult::OK, result)
            << "a data-moving change was accepted on the additive path; it must be routed to the "
               "upgrade front door instead";
        // Record which flavour of refusal we get - it decides what the front door has to catch.
        printf("[upstream] data-transform import returned %d (DataTransformRequired=%d)\n",
               (int)result, (int)SchemaImportResult::ERROR_DATA_TRANSFORM_REQUIRED);
    });
    }

END_ECDBUNITTESTS_NAMESPACE
