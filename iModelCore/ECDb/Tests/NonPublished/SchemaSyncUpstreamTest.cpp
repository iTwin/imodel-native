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
 * SchemaSync "upstream" spike tests.
 *
 * The upstream flow inverts the direction of pull and push: a schema import runs in the sync db
 * FIRST, which decides ids
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

// Reads one table as one quoted text line per row, keyed by rowid. quote() keeps NULLs and types
// visible, which matters because two values that print the same can still hash differently.
bmap<int64_t, Utf8String> ReadTableRows(ECDbR db, Utf8CP table) {
    bvector<Utf8String> columns;
    {
    Statement stmt;
    if (stmt.Prepare(db, SqlPrintfString("pragma main.table_info(%s)", table).GetUtf8CP()) != BE_SQLITE_OK)
        return {};
    while (stmt.Step() == BE_SQLITE_ROW)
        columns.push_back(stmt.GetValueText(1));
    }

    Utf8String exprs;
    for (auto const& column : columns) {
        if (!exprs.empty())
            exprs.append(" || ' | ' || ");
        exprs.append(SqlPrintfString("'%s='||quote([%s])", column.c_str(), column.c_str()).GetUtf8CP());
    }

    bmap<int64_t, Utf8String> rows;
    Statement stmt;
    if (stmt.Prepare(db, SqlPrintfString("SELECT ROWID, %s FROM main.[%s]", exprs.c_str(), table).GetUtf8CP()) != BE_SQLITE_OK)
        return {};
    while (stmt.Step() == BE_SQLITE_ROW)
        rows[stmt.GetValueInt64(0)] = stmt.GetValueText(1);
    return rows;
}

// Names the rows that differ, rather than only reporting that a hash did not match. Without this a
// content mismatch says nothing about which column is wrong, and the interesting ones - a foreign
// key nulled by a cascade, a type that changed - are invisible in a checksum.
void ReportRowDifferences(ECDbR actual, ECDbR expected, Utf8CP table, Utf8CP context) {
    const auto actualRows = ReadTableRows(actual, table);
    const auto expectedRows = ReadTableRows(expected, table);

    int reported = 0;
    constexpr int maxReported = 10;
    for (auto const& entry : expectedRows) {
        const auto found = actualRows.find(entry.first);
        if (found == actualRows.end()) {
            printf("[upstream] %s: %s rowid=%lld only in expected: %s\n", context, table, (long long)entry.first, entry.second.c_str());
            ++reported;
        } else if (!entry.second.Equals(found->second)) {
            printf("[upstream] %s: %s rowid=%lld\n    expected: %s\n      actual: %s\n", context, table, (long long)entry.first, entry.second.c_str(), found->second.c_str());
            ++reported;
        }
        if (reported >= maxReported)
            return;
    }
    for (auto const& entry : actualRows) {
        if (expectedRows.find(entry.first) != expectedRows.end())
            continue;
        printf("[upstream] %s: %s rowid=%lld only in actual: %s\n", context, table, (long long)entry.first, entry.second.c_str());
        if (++reported >= maxReported)
            return;
    }
}

// Compares every ec_ table between two files and reports, per table, where they differ. The point is
// diagnostic: a single failing run should say WHICH filter rule is wrong, not merely that some hash
// did not match. ec_cache_* is skipped - it is derived and rebuilt locally.
void ExpectECTablesIdentical(ECDbR actual, ECDbR expected, Utf8CP context) {
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
        if (actualCount != expectedCount) {
            ReportRowDifferences(actual, expected, table.c_str(), context);
            continue;
        }
        if (!contentHash(expected, table.c_str()).Equals(contentHash(actual, table.c_str()))) {
            ADD_FAILURE() << context << ": row CONTENT differs in " << table.c_str() << " (same count, different values)";
            ReportRowDifferences(actual, expected, table.c_str(), context);
        }
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

// Runs one import through the sync db and adopts the result - the two steps of the upstream flow,
// which the real front door will eventually perform under a single container lock.
void ImportUpstream(TrackedECDb& briefcase, SchemaSyncDb& syncDb, std::vector<SchemaItem> const& schemas, bvector<Utf8String> const& adopt) {
    syncDb.WithReadWrite([&](ECDbR sync) {
        ASSERT_EQ(SchemaImportResult::OK, SchemaSyncTestFixture::ImportSchemas(sync, schemas, SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
    });
    ASSERT_EQ(SchemaSync::Status::OK, briefcase.Schemas().GetSchemaSync().AdoptSchemas(syncDb.GetSyncDbUri(), adopt));
    ASSERT_EQ(BE_SQLITE_OK, briefcase.SaveChanges());
}

// After merging someone else's schema changeset, a briefcase holds the ec_ rows but not the physical
// columns: adopt deliberately does not track DDL, exactly as a pull does not, so every briefcase
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

// ...and in which physical table does that column live?
Utf8String TableOf(ECDbR db, Utf8CP schemaName, Utf8CP className, Utf8CP accessString) {
    Statement stmt;
    if (stmt.Prepare(db, R"sql(
        SELECT t.Name FROM main.ec_PropertyMap pm
        JOIN main.ec_Column col ON col.Id = pm.ColumnId
        JOIN main.ec_Table t ON t.Id = col.TableId
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

// Holds schemas plus the context that owns them, so the caller can keep both alive with one object.
struct BriefcaseSchemas final {
    ECSchemaReadContextPtr m_context;
    bvector<ECSchemaPtr> m_owned;
    bvector<ECSchemaCP> m_refs;
    bvector<ECSchemaCP> const& Refs() const { return m_refs; }
    bool IsValid() const { return !m_refs.empty(); }
};

// Loads schema xml the way the ordinary import path does - against the briefcase, so references
// resolve to what it holds. That is exactly what SchemaSync is handed in production: JsInterop reads
// the files and DgnDb passes the objects down, and re-pointing them at the sync db is SchemaSync's
// own job. Loading them here rather than writing files keeps the tests on that path.
BriefcaseSchemas LoadSchemas(ECDbR briefcase, std::vector<SchemaItem> const& items) {
    BriefcaseSchemas result;
    result.m_context = ECSchemaReadContext::CreateContext();
    result.m_context->AddSchemaLocater(briefcase.GetSchemaLocater());
    for (auto const& item : items) {
        ECSchemaPtr schema;
        if (ECSchema::ReadFromXmlString(schema, item.GetXmlString().c_str(), *result.m_context) != SchemaReadStatus::Success || !schema.IsValid()) {
            result.m_owned.clear();
            result.m_refs.clear();
            return result;
        }
        result.m_owned.push_back(schema);
        result.m_refs.push_back(schema.get());
    }
    return result;
}

// Which version of a schema does this file hold?
Utf8String VersionOf(ECDbR db, Utf8CP schemaName) {
    Statement stmt;
    if (stmt.Prepare(db, "SELECT VersionDigit1, VersionDigit2, VersionDigit3 FROM main.ec_Schema WHERE Name=?") != BE_SQLITE_OK)
        return "";
    stmt.BindText(1, schemaName, Statement::MakeCopy::No);
    if (stmt.Step() != BE_SQLITE_ROW)
        return "";
    return Utf8PrintfString("%d.%d.%d", stmt.GetValueInt(0), stmt.GetValueInt(1), stmt.GetValueInt(2));
}

// The setup every entry-point test starts from: one briefcase initialises schema sync and pushes, a
// second one is created afterwards so it picks that up from the timeline.
void SetupSyncedPair(ECDbHub& hub, SchemaSyncDb& syncDb, std::unique_ptr<TrackedECDb>& b1, std::unique_ptr<TrackedECDb>& b2) {
    b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("init schema sync");
    b2 = hub.CreateBriefcase();
}

// Foreign keys are deferred while rows are being copied, so a violation surfaces at SaveChanges with
// nothing to say about its cause. This names the offending row before the commit swallows it.
void ExpectNoForeignKeyViolations(ECDbR db, Utf8CP context) {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "PRAGMA main.foreign_key_check"));
    int violations = 0;
    while (stmt.Step() == BE_SQLITE_ROW) {
        ++violations;
        printf("[upstream] FK violation (%s): table=%s rowid=%lld parent=%s fkid=%d\n",
            context, stmt.GetValueText(0), stmt.GetValueInt64(1), stmt.GetValueText(2), stmt.GetValueInt(3));
    }
    EXPECT_EQ(0, violations) << context << ": rows were copied whose parents are missing";
}

// Does the physical table exist, as opposed to merely being described by ec_Table?
bool HasPhysicalTable(ECDbR db, Utf8CP tableName) {
    Statement stmt;
    if (stmt.Prepare(db, "SELECT 1 FROM main.sqlite_master WHERE type='table' AND name=?") != BE_SQLITE_OK)
        return false;
    stmt.BindText(1, tableName, Statement::MakeCopy::No);
    return stmt.Step() == BE_SQLITE_ROW;
}

// Two siblings whose same-named property sits in DIFFERENT shared columns; hoisting it onto their
// common base forces those two columns to consolidate into one, which moves data. Shared columns are
// what makes this a remap at all - RemapManager only ever considers ColumnKind = 4.
SchemaItem RemapSchema(Utf8CP version, bool hoisted) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="RemapTest" alias="rmp" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
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
                %s
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
        </ECSchema>)xml", version, hoisted ? R"xml(<ECProperty propertyName="movingProp" typeName="string" />)xml" : "");
    return SchemaItem(xml);
}

// Reads a single string property back through ECSql, which is the only way that proves the physical
// layout and the metadata still agree after data has been moved between columns.
Utf8String ReadStringProperty(ECDbR db, Utf8CP ecsql) {
    ECSqlStatement stmt;
    if (stmt.Prepare(db, ecsql) != ECSqlStatus::Success)
        return "<prepare failed>";
    if (stmt.Step() != BE_SQLITE_ROW)
        return "<no row>";
    return Utf8String(stmt.GetValueText(0));
}

} // namespace

// ---------------------------------------------------------------------------------------
// N1 spike, part 1: can the sync db run a schema import at all?
//
// The sync db is a real ECDb (same EC profile, full ec_* mirror) but holds no data tables. The
// import therefore runs with DoNotCreateOrUpdateDataTables, writing ec_* rows and nothing else.
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
        ASSERT_EQ(SchemaImportResult::OK, ImportSchema(sync, SharedColumnSchema(), SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables))
            << "importing into the sync db failed - the design does not survive this";
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
// This is the load-bearing assumption of the whole approach. Both files start from the same ec_*
// state (Init pushed
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
        ASSERT_EQ(SchemaImportResult::OK, ImportSchema(sync, schema, SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables));
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
        ASSERT_EQ(SchemaImportResult::OK, ImportSchema(sync, first, SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
        ASSERT_EQ(SchemaImportResult::OK, ImportSchema(sync, second, SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());

        EXPECT_STREQ(briefcaseSchemaHash.c_str(), GetSchemaHash(sync).c_str())
            << "logical schema rows diverged after the second import";
        EXPECT_STREQ(briefcaseMapHash.c_str(), GetMapHash(sync).c_str())
            << "mapping diverged after the second import - accumulated layout state is not reproduced "
               "identically in the sync db";
    });
    }

// ---------------------------------------------------------------------------------------
// N1 spike, part 4: an import into the sync db creates no data tables.
//
// The sync db holds ec_ rows and nothing else. That is what DoNotCreateOrUpdateDataTables is for,
// and it is what keeps the sync db from accumulating tables that a later overwrite of its ec_ rows
// would leave describing a layout nobody agrees with.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, ImportIntoSyncDbCreatesNoDataTables)
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

        ASSERT_EQ(SchemaImportResult::OK, ImportSchema(sync, SharedColumnSchema(), SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());

        EXPECT_EQ(before, countRealTables(sync)) << "the import created data tables in a file that is supposed to hold metadata only";
        EXPECT_FALSE(HasPhysicalTable(sync, "ut_Base"));
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
        ASSERT_EQ(SchemaImportResult::OK, ImportSchema(sync, SharedColumnSchema(), SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
    });

    // Step 2: the briefcase adopts it.
    ASSERT_EQ(SchemaSync::Status::OK, b2->Schemas().GetSchemaSync().AdoptSchemas(syncDb.GetSyncDbUri(), { "UpstreamTest" }));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    ASSERT_TRUE(HasSchema(*b2, "UpstreamTest"));

    syncDb.WithReadOnly([&](ECDbR sync) {
        ExpectECTablesIdentical(*b2, sync, "after adopting the only new schema");
    });
    }

// ---------------------------------------------------------------------------------------
// The filtering oracle, and the reason adopt filters at all.
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
        ASSERT_EQ(SchemaImportResult::OK, ImportSchema(sync, SharedColumnSchema(), SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
        ASSERT_EQ(SchemaImportResult::OK, ImportSchema(sync, UnrelatedSchema(), SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables));
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
        ASSERT_EQ(SchemaImportResult::OK, ImportSchemas(sync, { SharedColumnSchema(), ReferencingSchema() }, SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
    });

    // Only the referencing schema is named; the referenced one has to follow on its own.
    ASSERT_EQ(SchemaSync::Status::OK, b2->Schemas().GetSchemaSync().AdoptSchemas(syncDb.GetSyncDbUri(), { "ReferencingTest" }));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    EXPECT_TRUE(HasSchema(*b2, "ReferencingTest"));
    EXPECT_TRUE(HasSchema(*b2, "UpstreamTest")) << "the referenced schema did not come along - closure is incomplete";
    EXPECT_TRUE(ForeignkeyCheck(*b2));

    syncDb.WithReadOnly([&](ECDbR sync) {
        ExpectECTablesIdentical(*b2, sync, "after adopting a schema plus its reference");
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
        ASSERT_EQ(SchemaImportResult::OK, ImportSchema(sync, SharedColumnSchema(), SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables));
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
// Does the upstream flow actually fix the failures that killed the earlier designs?
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
    // matters: where the allocation is made is the thing that changed, and it is settled the moment
    // the sync db decides it.
    for (auto* db : { b2.get() }) {
        const auto ratingCol = ColumnOf(*db, "Machinery", "Machine", "rating");
        const auto volumeCol = ColumnOf(*db, "DemoB", "Tank", "volume");
        EXPECT_FALSE(ratingCol.empty()) << "rating is not mapped";
        EXPECT_FALSE(volumeCol.empty()) << "volume is not mapped";
        EXPECT_STRNE(ratingCol.c_str(), volumeCol.c_str())
            << "rating and volume were double-booked into the same shared column - this is the "
               "silent corruption the sync db exists to prevent";
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
// When each briefcase allocated its own ids that was an edge case, because two of them rarely held
// byte-identical rows. Now that every briefcase gets its rows from the same authority,
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

    ExpectECTablesIdentical(*b2, *b1, "after both briefcases exchanged changesets");
    syncDb.WithReadOnly([&](ECDbR sync) {
        ExpectECTablesIdentical(*b1, sync, "briefcase vs sync db after convergence");
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
        EXPECT_NE(SchemaImportResult::OK, ImportSchema(sync, conflicting, SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables))
            << "the sync db accepted a conflicting property type - the authority is not authoritative";
    });
    }

// ---------------------------------------------------------------------------------------
// The remap gate, which keeps the update path additive.
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
        const auto result = ImportSchema(sync, hoisted, SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables);
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

//=======================================================================================
// The orchestration entry point.
//
// Everything above drives the two steps by hand. SchemaSync::ImportSchemas is the single call that
// does both: run the import in the sync db, then adopt the result here. The caller holds the
// container write lock around the whole call, exactly as it does for an ordinary import. These tests are
// about that call - the steps themselves are already covered.
//=======================================================================================

// ---------------------------------------------------------------------------------------
// One call, both steps: the sync db ends up holding the import, and so does the briefcase.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, ImportSchemasDoesBothSteps)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-entrypoint");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto schemas = LoadSchemas(*b2, { SharedColumnSchema() });
    ASSERT_TRUE(schemas.IsValid()) << "could not load the schema";

    ASSERT_EQ(SchemaSync::Status::OK, b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), schemas.Refs()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    EXPECT_TRUE(HasSchema(*b2, "UpstreamTest")) << "the briefcase did not adopt what it imported";
    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_TRUE(HasSchema(sync, "UpstreamTest")) << "the sync db did not receive the import";
        ExpectECTablesIdentical(*b2, sync, "after a single ImportSchemas call");
    });

    // And the result has to be a working file, not merely a consistent-looking one.
    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO ut.Derived(baseProp,p1,p8) VALUES('hello',42,99)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    }

// ---------------------------------------------------------------------------------------
// The sync db has to be a valid ECDb, not only a correct set of ec_ rows.
//
// Every other test here starts from an empty briefcase, so its sync db describes no tables it does
// not have. A real iModel is the opposite: Init mirrors the whole ec_ mirror into the sync db and
// then drops the data tables, which was harmless while nothing ever imported there.
// Now that the import runs in the sync db, DbMapValidator refuses one outright the moment it
// loads a table describing a non-virtual column the file lacks - which CreateOrUpdateIndexesInDb
// does for every table that has an index, after the point where the columns would have been added.
//
// So this is the smallest scenario that resembles production, and it belongs next to the entry
// point rather than in the upgrade section: nothing here is upgrading anything.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, ImportSchemasWorksOnASyncDbInitialisedFromABriefcaseWithTables)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-nonempty-init");

    // A briefcase that already carries a schema of its own before schema sync is switched on.
    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, SharedColumnSchema()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    ASSERT_TRUE(HasPhysicalTable(*b1, "ut_Base")) << "the scenario did not set itself up";

    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("init schema sync");

    // The sync db now describes ut_Base and, as Init leaves it, does not have it.
    const auto schemas = LoadSchemas(*b1, { UnrelatedSchema() });
    ASSERT_TRUE(schemas.IsValid());

    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), schemas.Refs()))
        << "an import into a sync db made from a non-empty briefcase failed";
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    EXPECT_TRUE(HasSchema(*b1, "UnrelatedTest"));
    EXPECT_TRUE(HasSchema(*b1, "UpstreamTest")) << "the schema that predated schema sync was disturbed";
    }

// ---------------------------------------------------------------------------------------
// Why the entry point takes file paths rather than ECSchema objects.
//
// The sync db is ahead of the briefcase - somebody else imported Machinery 1.0.1 and has not pushed
// it. The briefcase now imports DemoB, which references Machinery. Deserializing DemoB against the
// BRIEFCASE would resolve that reference to 1.0.0 and compute mappings against the wrong version;
// deserializing it against the sync db resolves to 1.0.1, which is the one that actually decides
// layout. The briefcase therefore ends up on 1.0.1 as well, without having asked for it - the
// reference auto-update that was agreed on the thread.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, ImportSchemasResolvesReferencesAgainstSyncDb)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-entrypoint-refs");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    // Both briefcases reach Machinery 1.0.0 through the timeline.
    ImportUpstream(*b1, syncDb, { MachinerySchema("01.00.00", false) }, { "Machinery" });
    b1->PullMergePush("add Machinery 1.0.0");
    b2->PullMergePush("pick up Machinery 1.0.0");
    MaterializeAfterMerge(*b2);
    ASSERT_STREQ("1.0.0", VersionOf(*b2, "Machinery").c_str());

    // b1 moves the sync db to 1.0.1 and keeps it to itself - no push.
    ImportUpstream(*b1, syncDb, { MachinerySchema("01.00.01", true) }, { "Machinery" });

    // b2 imports a schema that references Machinery, still believing it is at 1.0.0.
    const auto tank = LoadSchemas(*b2, { TankSchema() });
    ASSERT_TRUE(tank.IsValid());
    ASSERT_EQ(SchemaSync::Status::OK, b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), tank.Refs()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    EXPECT_TRUE(HasSchema(*b2, "DemoB"));
    EXPECT_STREQ("1.0.1", VersionOf(*b2, "Machinery").c_str())
        << "the referenced schema was not updated to the version the sync db decided against";

    // The proof that this matters: rating (1.0.1, b1's) and volume (DemoB, b2's) compete for the same
    // shared-column pool. If b2 had mapped against 1.0.0 it would not have known rating existed.
    const auto ratingColumn = ColumnOf(*b2, "Machinery", "Machine", "rating");
    const auto volumeColumn = ColumnOf(*b2, "DemoB", "Tank", "volume");
    EXPECT_FALSE(ratingColumn.empty()) << "rating did not come along with the reference update";
    EXPECT_STRNE(ratingColumn.c_str(), volumeColumn.c_str())
        << "rating and volume were placed in the same shared column";
    }

// ---------------------------------------------------------------------------------------
// A data-moving change is refused, and refused cleanly.
//
// The additive path disallows data transforms, so this has to come back as a distinct status the
// caller can route to the upgrade front door - and, just as important, both the sync db and the
// briefcase must be exactly as they were.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, ImportSchemasRefusesDataTransform)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-entrypoint-transform");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    // Same scenario as SyncDbRefusesImportNeedingDataTransform: two siblings whose same-named
    // property sits in different shared columns, then hoisted onto their common base.
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

    const auto initialSchemas = LoadSchemas(*b2, { initial });
    ASSERT_TRUE(initialSchemas.IsValid());

    ASSERT_EQ(SchemaSync::Status::OK, b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), initialSchemas.Refs()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    ASSERT_STREQ("1.0.0", VersionOf(*b2, "RemapTest").c_str());

    EXPECT_EQ(SchemaSync::Status::ERROR_DATA_TRANSFORM_REQUIRED,
              b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { hoisted }).Refs()))
        << "a data-moving change was accepted on the additive path";

    EXPECT_STREQ("1.0.0", VersionOf(*b2, "RemapTest").c_str()) << "the briefcase was changed by a refused import";
    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_STREQ("1.0.0", VersionOf(sync, "RemapTest").c_str()) << "the sync db was changed by a refused import";
    });
    }

// ---------------------------------------------------------------------------------------
// Instances that already existed when a class grew into an overflow table.
//
// A normal import ends by giving every existing instance a matching row in any overflow table it
// just created - otherwise a write to a property that landed there has nowhere to go. That step now
// runs in the sync db, which holds no data, so it does nothing; and adopt only materialises
// tables and indexes. The importing briefcase's own instances therefore have to be caught up here,
// or they silently drop writes to every property that spilled over.
//
// Only pre-existing instances can reach this. One inserted after the widening gets its overflow row
// from the insert itself, which is why the earlier adopt tests do not see it.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, ImportSchemasCatchesUpInstancesThatSpillIntoOverflow)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-entrypoint-overflow");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    // Two properties fit inside the shared-column budget; eight do not.
    const auto narrow = LoadSchemas(*b2, { SharedColumnSchema("01.00.00", 2) });
    ASSERT_TRUE(narrow.IsValid());
    ASSERT_TRUE(narrow.IsValid());

    ASSERT_EQ(SchemaSync::Status::OK, b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), narrow.Refs()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    ASSERT_TRUE(TableOf(*b2, "UpstreamTest", "Derived", "p1").length() > 0);

    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO ut.Derived(baseProp,p1) VALUES('before',1)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    // Widening the class pushes the later properties into an overflow table.
    ASSERT_EQ(SchemaSync::Status::OK, b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { SharedColumnSchema("01.00.01", 8) }).Refs()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    const auto primaryTable = TableOf(*b2, "UpstreamTest", "Derived", "p1");
    const auto overflowTable = TableOf(*b2, "UpstreamTest", "Derived", "p8");
    printf("[upstream] p1 -> %s, p8 -> %s\n", primaryTable.c_str(), overflowTable.c_str());
    ASSERT_FALSE(overflowTable.empty()) << "p8 was not mapped at all";
    ASSERT_STRNE(primaryTable.c_str(), overflowTable.c_str())
        << "nothing spilled into an overflow table, so this scenario no longer tests what it claims to";

    // The precise thing that has to have happened: the instance that predates the widening now has
    // a row in the overflow table.
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*b2, SqlPrintfString("SELECT COUNT(*) FROM main.[%s] WHERE Id=?", overflowTable.c_str()).GetUtf8CP()));
    stmt.BindId(1, key.GetInstanceId());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(1, stmt.GetValueInt(0)) << "the pre-existing instance was never given an overflow row";
    }

    // ...and the consequence of it not having one: the write goes nowhere.
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "UPDATE ut.Derived SET p8=? WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(1, 77));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    ECSqlStatement select;
    ASSERT_EQ(ECSqlStatus::Success, select.Prepare(*b2, "SELECT baseProp,p1,p8 FROM ut.Derived WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, select.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, select.Step());
    EXPECT_STREQ("before", select.GetValueText(0)) << "the widening lost the instance's original data";
    EXPECT_EQ(1, select.GetValueInt(1));
    EXPECT_EQ(77, select.GetValueInt(2)) << "the write to the overflow property did not persist";
    }

// ---------------------------------------------------------------------------------------
// The corruption case again, this time all the way through the real entry point.
//
// ConcurrentImportsDoNotShareAColumn establishes the property with the two steps driven by hand.
// This repeats it through SchemaSync::ImportSchemas, so what is measured is the thing that will
// actually ship.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, ConcurrentImportsThroughEntryPointConverge)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-entrypoint-converge");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto machinery100 = MachinerySchema("01.00.00", false);
    const auto machinery101 = MachinerySchema("01.00.01", true);

    auto& sync1 = b1->Schemas().GetSchemaSync();
    auto& sync2 = b2->Schemas().GetSchemaSync();

    ASSERT_EQ(SchemaSync::Status::OK, sync1.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b1, { machinery100 }).Refs()));
    ExpectNoForeignKeyViolations(*b1, "b1 after adopting Machinery 1.0.0");
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    // The import ran in the sync db, but the sync db holds ec_ rows only: its ec_Table row for
    // mch_Machine describes a table the briefcase will build, not one the sync db has.
    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_FALSE(HasPhysicalTable(sync, "mch_Machine"))
            << "the import created a data table in the sync db";
    });

    b1->PullMergePush("add Machinery");
    b2->PullMergePush("pick up Machinery");
    MaterializeAfterMerge(*b2);

    // Neither briefcase has seen the other's change; both are additive; both go through the sync db.
    ASSERT_EQ(SchemaSync::Status::OK, sync1.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b1, { machinery101 }).Refs()));
    ExpectNoForeignKeyViolations(*b1, "b1 after adopting Machinery 1.0.1");
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { TankSchema() }).Refs()));
    ExpectNoForeignKeyViolations(*b2, "b2 after adopting DemoB");
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    // The corruption case: one physical cell holding both rating and volume.
    EXPECT_STRNE(ColumnOf(*b2, "Machinery", "Machine", "rating").c_str(),
                 ColumnOf(*b2, "DemoB", "Tank", "volume").c_str())
        << "two concurrent additive imports were given the same shared column";

    b1->PullMergePush("b1 pushes rating");
    b2->PullMergePush("b2 merges rating, pushes tank");
    MaterializeAfterMerge(*b2);
    b1->PullMergePush("b1 merges tank");
    MaterializeAfterMerge(*b1);

    ExpectECTablesIdentical(*b2, *b1, "after both briefcases exchanged changesets");
    syncDb.WithReadOnly([&](ECDbR sync) {
        ExpectECTablesIdentical(*b1, sync, "briefcase vs sync db after convergence");
    });
    }

//=======================================================================================
// The upgrade path.
//
// A change that moves data is refused by ImportSchemas, and comes here instead. The direction is
// inverted a second time: the import runs on the BRIEFCASE, with transforms allowed, and the sync db
// is then overwritten from the result. That is only legal while the exclusive schema lock is held -
// which is also what makes the overwrite's deletes safe, since the lock cannot be acquired while
// anybody else holds one, so nobody can be holding local changes.
//=======================================================================================

// ---------------------------------------------------------------------------------------
// The change ImportSchemas refuses goes through here, and the data it moves survives.
//
// This is the property the whole upgrade path exists for: after hoisting a property that lived in
// two different shared columns, both instances must still read back what was written to them.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, UpgradeSchemasMovesDataAndOverwritesSyncDb)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-upgrade");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto initial = RemapSchema("01.00.00", false);
    const auto hoisted = RemapSchema("01.00.01", true);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { initial }).Refs()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    // Data that predates the move, in both of the columns that are about to be consolidated.
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO rmp.LeafA(baseProp,filler,movingProp) VALUES('a','f','from A')"));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO rmp.LeafB(baseProp,movingProp) VALUES('b','from B')"));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    // Confirm the two really are in different columns, or the upgrade moves nothing and the test
    // proves nothing.
    ASSERT_STRNE(ColumnOf(*b2, "RemapTest", "LeafA", "movingProp").c_str(),
                 ColumnOf(*b2, "RemapTest", "LeafB", "movingProp").c_str())
        << "the siblings already share a column, so hoisting cannot force a move";

    // The additive path must refuse it before the upgrade path is entitled to run.
    ASSERT_EQ(SchemaSync::Status::ERROR_DATA_TRANSFORM_REQUIRED,
              sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { hoisted }).Refs()));

    ASSERT_EQ(SchemaSync::Status::OK, sync2.UpgradeSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { hoisted }).Refs()));
    ExpectNoForeignKeyViolations(*b2, "b2 after upgrading");
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    EXPECT_STREQ("1.0.1", VersionOf(*b2, "RemapTest").c_str());
    EXPECT_STREQ("from A", ReadStringProperty(*b2, "SELECT movingProp FROM rmp.LeafA").c_str())
        << "data was lost when the column it lived in was consolidated";
    EXPECT_STREQ("from B", ReadStringProperty(*b2, "SELECT movingProp FROM rmp.LeafB").c_str())
        << "data was lost when the column it lived in was consolidated";

    // The sync db was written from the briefcase, so the two must now say exactly the same thing.
    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_STREQ("1.0.1", VersionOf(sync, "RemapTest").c_str()) << "the sync db did not receive the upgrade";
        ExpectECTablesIdentical(*b2, sync, "briefcase vs sync db after an upgrade");
    });
    }

// ---------------------------------------------------------------------------------------
// The overwrite deletes what nobody kept - which is the point, not a side effect.
//
// b1 imports a schema through the sync db and never pushes it, so the sync db holds rows no
// changeset carries. b2 then upgrades. Because the upgrade holds the exclusive schema lock, b1
// cannot have been holding local changes, so those rows can only be work that was abandoned - and
// the overwrite is what reclaims them, ids and shared column ordinals included.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, UpgradeSchemasDropsAbandonedSyncDbState)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-upgrade-cleanup");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto initial = RemapSchema("01.00.00", false);
    const auto hoisted = RemapSchema("01.00.01", true);
    const auto abandoned = UnrelatedSchema();

    auto& sync1 = b1->Schemas().GetSchemaSync();
    auto& sync2 = b2->Schemas().GetSchemaSync();

    // Everyone reaches RemapTest 1.0.0 through the timeline.
    ASSERT_EQ(SchemaSync::Status::OK, sync1.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b1, { initial }).Refs()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("add RemapTest 1.0.0");
    b2->PullMergePush("pick up RemapTest 1.0.0");
    MaterializeAfterMerge(*b2);

    // b1 imports something else and never pushes it. From the timeline's point of view this never
    // happened; from the sync db's point of view it did.
    ASSERT_EQ(SchemaSync::Status::OK, sync1.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b1, { abandoned }).Refs()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    syncDb.WithReadOnly([&](ECDbR sync) {
        ASSERT_TRUE(HasSchema(sync, "UnrelatedTest")) << "the scenario did not set itself up";
    });
    ASSERT_FALSE(HasSchema(*b2, "UnrelatedTest")) << "b2 was not supposed to learn about it";

    // b2 upgrades. It holds the exclusive schema lock, so b1 cannot be holding anything.
    ASSERT_EQ(SchemaSync::Status::OK, sync2.UpgradeSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { hoisted }).Refs()));
    ExpectNoForeignKeyViolations(*b2, "b2 after upgrading over abandoned state");
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_FALSE(HasSchema(sync, "UnrelatedTest"))
            << "abandoned rows survived the overwrite, so the sync db still describes a schema nobody has";
        EXPECT_STREQ("1.0.1", VersionOf(sync, "RemapTest").c_str());
        ExpectECTablesIdentical(*b2, sync, "briefcase vs sync db after the overwrite");
    });

    // And the sync db is still usable as the authority afterwards: the next additive import runs
    // against it and lands in both places.
    const auto unrelatedAgain = LoadSchemas(*b2, { UnrelatedSchema() });
    ASSERT_TRUE(unrelatedAgain.IsValid());
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), unrelatedAgain.Refs()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_TRUE(HasSchema(*b2, "UnrelatedTest"));
    syncDb.WithReadOnly([&](ECDbR sync) {
        ExpectECTablesIdentical(*b2, sync, "briefcase vs sync db after re-importing over reclaimed state");
    });
    }

//=======================================================================================
// Guards.
//=======================================================================================

// ---------------------------------------------------------------------------------------
// Neither entry point runs while the two files are on different EC profile versions.
//
// Pull and push each tolerate skew in one direction. Deciding the mapping in one file and adopting
// it in the other cannot: a version difference means they could map
// the same schema differently. Aligning them is a maintenance-mode job.
//
// The two cases use opposite skew directions on purpose. VerifySyncDb already refuses the
// other direction for each call, with a less specific error, so this is where the new guard is the
// one doing the work.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, EntryPointsRefuseProfileVersionSkew)
    {
    auto shiftSyncDbProfileVersion = [](SchemaSyncDb& syncDb, int by) {
        syncDb.WithReadWrite([&](ECDbR sync) {
            const auto current = sync.GetECDbProfileVersion();
            const ProfileVersion shifted(current.GetMajor(), current.GetMinor(), current.GetSub1(), (uint16_t)(current.GetSub2() + by));
            ASSERT_EQ(BE_SQLITE_OK, sync.SavePropertyString(PropertySpec("SchemaVersion", "ec_Db"), shifted.ToJson()));
            ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
        });
    };

    // ImportSchemas adopts from the sync db, so VerifySyncDb lets the sync db be ahead. The guard does not.
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-profile-skew-import");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);
    shiftSyncDbProfileVersion(syncDb, 1);

    const auto schemas = LoadSchemas(*b2, { SharedColumnSchema() });
    ASSERT_TRUE(schemas.IsValid());
    EXPECT_EQ(SchemaSync::Status::ERROR_PROFILE_VERSION_MISMATCH,
              b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), schemas.Refs()));
    EXPECT_FALSE(HasSchema(*b2, "UpstreamTest")) << "the import was refused but still changed the briefcase";
    }

    // UpgradeSchemas writes to the sync db, so VerifySyncDb lets the sync db be behind. The guard does not.
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-profile-skew-upgrade");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);
    shiftSyncDbProfileVersion(syncDb, -1);

    const auto schemas = LoadSchemas(*b2, { SharedColumnSchema() });
    ASSERT_TRUE(schemas.IsValid());
    EXPECT_EQ(SchemaSync::Status::ERROR_PROFILE_VERSION_MISMATCH,
              b2->Schemas().GetSchemaSync().UpgradeSchemas(syncDb.GetSyncDbUri(), schemas.Refs()));
    EXPECT_FALSE(HasSchema(*b2, "UpstreamTest")) << "the upgrade was refused but still changed the briefcase";
    }
    }

// ---------------------------------------------------------------------------------------
// The old import path refuses a data-moving change on a file that uses schema sync.
//
// It used not to be refused, which is not the same as being supported: the data moves in the local
// briefcase and only the resulting ec_ rows reach the sync db, so every other briefcase adopts the
// new layout with its data still in the old columns. That is what the upgrade path exists to fix.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, PlainImportRoutesByTransformOption)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-routing");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto initial = RemapSchema("01.00.00", false);
    const auto hoisted = RemapSchema("01.00.01", true);

    // No option: the additive path. The sync db decides and b2 adopts.
    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b2, initial, SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_TRUE(HasSchema(sync, "RemapTest")) << "the additive path did not go through the sync db";
        ExpectECTablesIdentical(*b2, sync, "after an additive import through the ordinary path");
    });

    // Still no option, but the change moves data: refused, and the caller is told why so it can
    // take the exclusive lock and come back.
    ASSERT_EQ(SchemaImportResult::ERROR_DATA_TRANSFORM_REQUIRED,
              ImportSchema(*b2, hoisted, SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));

    // With the option, the same change goes through the upgrade path.
    ASSERT_EQ(SchemaImportResult::OK,
              ImportSchema(*b2, hoisted, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    EXPECT_STREQ("1.0.1", VersionOf(*b2, "RemapTest").c_str());
    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_STREQ("1.0.1", VersionOf(sync, "RemapTest").c_str()) << "the sync db was not rebuilt from the upgraded briefcase";
        ExpectECTablesIdentical(*b2, sync, "after an upgrade through the ordinary path");
    });
    }

// ---------------------------------------------------------------------------------------
// One briefcase remaps, the other pulls - and its data has to survive the move.
//
// The upgrade moves data between columns in the briefcase that performs it. Every other briefcase
// gets the new layout from the changeset and the data movement with it, so the question is whether
// rows written *before* the upgrade, in a briefcase that did not perform it, still read back. This
// is the cross-briefcase half of the upgrade path, and the reason data transforms were never safe
// while every briefcase decided its own layout.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncUpstreamTestFixture, RemapInOneBriefcaseSurvivesInTheOther)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-remap-across-briefcases");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto initial = RemapSchema("01.00.00", false);
    const auto hoisted = RemapSchema("01.00.01", true);

    // Both briefcases reach 1.0.0 through the timeline.
    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, initial, SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("add RemapTest 1.0.0");
    b2->PullMergePush("pick up RemapTest 1.0.0");
    MaterializeAfterMerge(*b2);

    // Both write into the columns that are about to be consolidated.
    for (auto* briefcase : { b1.get(), b2.get() }) {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*briefcase, "INSERT INTO rmp.LeafB(baseProp,movingProp) VALUES('b','before the move')"));
        ECInstanceKey key;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
        ASSERT_EQ(BE_SQLITE_OK, briefcase->SaveChanges());
    }
    b2->PullMergePush("b2's row, written before the upgrade");
    b1->PullMergePush("b1's row");

    // b1 upgrades under the exclusive lock and pushes both the changeset and the sync db.
    ASSERT_EQ(SchemaImportResult::OK,
              ImportSchema(*b1, hoisted, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    ExpectNoForeignKeyViolations(*b1, "b1 after upgrading");
    b1->PullMergePush("hoist movingProp");

    // b2 learns about it from the timeline alone.
    b2->PullMergePush("pick up the hoist");
    MaterializeAfterMerge(*b2);

    EXPECT_STREQ("1.0.1", VersionOf(*b2, "RemapTest").c_str());
    EXPECT_STREQ("before the move", ReadStringProperty(*b1, "SELECT movingProp FROM rmp.LeafB").c_str())
        << "the briefcase that performed the remap lost its own data";
    EXPECT_STREQ("before the move", ReadStringProperty(*b2, "SELECT movingProp FROM rmp.LeafB").c_str())
        << "a briefcase that only pulled the remap cannot read data written before it";

    ExpectECTablesIdentical(*b2, *b1, "after the remap reached the second briefcase");
    syncDb.WithReadOnly([&](ECDbR sync) {
        ExpectECTablesIdentical(*b1, sync, "briefcase vs sync db after a remap");
    });

    // And the file still works afterwards, in both.
    for (auto* briefcase : { b1.get(), b2.get() }) {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*briefcase, "INSERT INTO rmp.LeafA(baseProp,filler,movingProp) VALUES('a','f','after the move')"));
        ECInstanceKey key;
        EXPECT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << "writing through the consolidated column failed";
        ASSERT_EQ(BE_SQLITE_OK, briefcase->SaveChanges());
    }
    }

END_ECDBUNITTESTS_NAMESPACE
