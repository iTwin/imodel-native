/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"
#include "ECDbPublishedTests.h"
#include "MockHubApi.h"
#include <algorithm>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

// SchemaSyncTest.cpp covers what the sync db does with a schema once it is there. This file covers
// how it gets there: the two steps of an import, the entry points that drive them, and whether two
// briefcases end up with the same ec_ rows and the same physical schema.
struct SchemaSyncImportTestFixture : SchemaSyncTestFixture {};

//! The extended tier of the v2 suite. See SchemaSyncExtendedTests in SchemaSyncTest.cpp - same
//! split, same reason. This is where the permutation matrices live; the behaviours they permute
//! each have a representative on SchemaSyncImportTestFixture.
struct SchemaSyncImportExtendedTests : SchemaSyncImportTestFixture
    {
    ECDB_EXTENDED_TIER_GATE(SchemaSyncImportTestFixture)
    };

namespace {

// TablePerHierarchy with shared columns, so the mapper allocates ordinals rather than one column
// per property.
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

// Shares nothing with UpstreamTest, so adopting one must not drag in the other.
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

// ...and the named class within it?
bool HasClass(ECDbR db, Utf8CP schemaName, Utf8CP className) {
    Statement stmt;
    if (stmt.Prepare(db, R"sql(
        SELECT 1 FROM main.ec_Class c
        JOIN main.ec_Schema s ON s.Id = c.SchemaId
        WHERE s.Name=? AND c.Name=?)sql") != BE_SQLITE_OK)
        return false;
    stmt.BindText(1, schemaName, Statement::MakeCopy::No);
    stmt.BindText(2, className, Statement::MakeCopy::No);
    return stmt.Step() == BE_SQLITE_ROW;
}

int64_t CountClassTableMappings(ECDbR db, Utf8CP schemaName, Utf8CP className) {
    Statement stmt;
    if (stmt.Prepare(db, R"sql(
        SELECT COUNT(*) FROM main.ec_cache_ClassHasTables cht
        JOIN main.ec_Class c ON c.Id=cht.ClassId
        JOIN main.ec_Schema s ON s.Id=c.SchemaId
        WHERE s.Name=? AND c.Name=?)sql") != BE_SQLITE_OK)
        return -1;
    stmt.BindText(1, schemaName, Statement::MakeCopy::No);
    stmt.BindText(2, className, Statement::MakeCopy::No);
    return stmt.Step() == BE_SQLITE_ROW ? stmt.GetValueInt64(0) : -1;
}

int64_t CountClassHierarchyEntries(ECDbR db, Utf8CP schemaName, Utf8CP className) {
    Statement stmt;
    if (stmt.Prepare(db, R"sql(
        SELECT COUNT(*) FROM main.ec_cache_ClassHierarchy ch
        JOIN main.ec_Class c ON c.Id=ch.ClassId
        JOIN main.ec_Schema s ON s.Id=c.SchemaId
        WHERE s.Name=? AND c.Name=?)sql") != BE_SQLITE_OK)
        return -1;
    stmt.BindText(1, schemaName, Statement::MakeCopy::No);
    stmt.BindText(2, className, Statement::MakeCopy::No);
    return stmt.Step() == BE_SQLITE_ROW ? stmt.GetValueInt64(0) : -1;
}

// Does this file know the named schema?
bool HasSchema(ECDbR db, Utf8CP schemaName) {
    Statement stmt;
    if (stmt.Prepare(db, "SELECT 1 FROM main.ec_Schema WHERE Name=?") != BE_SQLITE_OK)
        return false;
    stmt.BindText(1, schemaName, Statement::MakeCopy::No);
    return stmt.Step() == BE_SQLITE_ROW;
}

int64_t CountRows(ECDbR db, Utf8CP table) {
    Statement stmt;
    if (stmt.Prepare(db, SqlPrintfString("SELECT COUNT(*) FROM main.[%s]", table).GetUtf8CP()) != BE_SQLITE_OK)
        return -1;
    return stmt.Step() == BE_SQLITE_ROW ? stmt.GetValueInt64(0) : -1;
}

bool HasTrigger(ECDbR db, Utf8CP triggerName) {
    Statement stmt;
    if (stmt.Prepare(db, "SELECT 1 FROM main.sqlite_master WHERE type='trigger' AND name=?") != BE_SQLITE_OK)
        return false;
    stmt.BindText(1, triggerName, Statement::MakeCopy::No);
    return stmt.Step() == BE_SQLITE_ROW;
}

// A nav property and a link table - both of DerivedDbStructures' foreign-key passes. Without the
// ForeignKeyConstraint custom attribute the nav property is logical and emits no constraint.
SchemaItem RelationshipSchema() {
    return SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="RelTest" alias="rel" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Parent">
                <ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
                <ECProperty propertyName="name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Child">
                <ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
                <ECProperty propertyName="name" typeName="string" />
                <ECNavigationProperty propertyName="Owner" relationshipName="ParentOwnsChild" direction="Backward">
                    <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.02.00.00">
                            <OnDeleteAction>Cascade</OnDeleteAction>
                        </ForeignKeyConstraint>
                    </ECCustomAttributes>
                </ECNavigationProperty>
            </ECEntityClass>
            <ECEntityClass typeName="Tag">
                <ECProperty propertyName="name" typeName="string" />
            </ECEntityClass>
            <ECRelationshipClass typeName="ParentOwnsChild" strength="embedding" modifier="None">
                <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true"><Class class="Parent"/></Source>
                <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true"><Class class="Child"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="ChildHasTags" strength="referencing" modifier="None">
                <Source multiplicity="(0..*)" roleLabel="has" polymorphic="true"><Class class="Child"/></Source>
                <Target multiplicity="(0..*)" roleLabel="belongs to" polymorphic="true"><Class class="Tag"/></Target>
            </ECRelationshipClass>
        </ECSchema>)xml");
}

// Each subclass gets its own table, whose foreign key back to the parent is the child-table pass.
SchemaItem JoinedTableSchema() {
    return SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="JoinedTest" alias="jnd" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Base" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00" />
                </ECCustomAttributes>
                <ECProperty propertyName="baseProp" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Sub1">
                <BaseClass>Base</BaseClass>
                <ECProperty propertyName="sub1Prop" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Sub2">
                <BaseClass>Base</BaseClass>
                <ECProperty propertyName="sub2Prop" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml");
}

// Populates the ec_ tables no other test here touches. Two traps that stop it loading at all: item
// names are compared case-insensitively across all item types, and `numerator` scales a unit UP
// relative to its definition, so a composite's units have to go largest first.
SchemaItem UnitsAndFormatsSchema() {
    return SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="UnitTest" alias="unt" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="TESTSYS" displayLabel="Test system" />
            <Phenomenon typeName="TESTLENGTH" definition="TESTLENGTH" displayLabel="Test length" />
            <Unit typeName="SMALL" definition="SMALL" phenomenon="TESTLENGTH" unitSystem="TESTSYS" />
            <Unit typeName="BIG" definition="SMALL" numerator="100.0" phenomenon="TESTLENGTH" unitSystem="TESTSYS" />
            <Format typeName="BigThenSmall" displayLabel="Big then small" type="Fractional" precision="4"
                    formatTraits="TrailZeroes|KeepSingleZero" decimalSeparator="." thousandSeparator="," uomSeparator=" ">
                <Composite spacer="-" includeZero="True">
                    <Unit label="b">BIG</Unit>
                    <Unit label="s">SMALL</Unit>
                </Composite>
            </Format>
            <KindOfQuantity typeName="MeasuredLength" description="A length" persistenceUnit="BIG" presentationUnits="BigThenSmall" relativeError="0.001" />
            <ECEnumeration typeName="Colour" backingTypeName="int" isStrict="true">
                <ECEnumerator name="Red" value="1" displayLabel="Red" />
                <ECEnumerator name="Green" value="2" displayLabel="Green" />
            </ECEnumeration>
            <ECEntityClass typeName="Measured">
                <ECProperty propertyName="length" typeName="double" kindOfQuantity="MeasuredLength" />
                <ECProperty propertyName="colour" typeName="Colour" />
            </ECEntityClass>
        </ECSchema>)xml");
}

// The one custom attribute that makes the mapper emit a trigger.
SchemaItem TimeStampSchema() {
    return SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="StampTest" alias="stp" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00" alias="CoreCA" />
            <ECEntityClass typeName="Stamped">
                <ECCustomAttributes>
                    <ClassHasCurrentTimeStampProperty xmlns="CoreCustomAttributes.01.00">
                        <PropertyName>LastMod</PropertyName>
                    </ClassHasCurrentTimeStampProperty>
                </ECCustomAttributes>
                <ECProperty propertyName="name" typeName="string" />
                <ECProperty propertyName="LastMod" typeName="dateTime" readOnly="True" />
            </ECEntityClass>
        </ECSchema>)xml");
}

// Siblings may share a slot, so one physical column serves both.
SchemaItem SiblingSlotSchema(Utf8CP version, bool withSecondSibling) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="SlotTest" alias="slt" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Root" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow></ShareColumns>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="First">
                <BaseClass>Root</BaseClass>
                <ECProperty propertyName="firstProp" typeName="int" />
            </ECEntityClass>
            %s
        </ECSchema>)xml", version, withSecondSibling ? R"xml(
            <ECEntityClass typeName="Second">
                <BaseClass>Root</BaseClass>
                <ECProperty propertyName="secondProp" typeName="int" />
            </ECEntityClass>)xml" : "");
    return SchemaItem(xml);
}

// Two briefcases adding properties under this root compete for the same shared-column pool.
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

// A subclass under Machinery, so its property lands in the same shared-column pool.
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

// The two steps SchemaSync::ImportSchemas performs, driven separately so a failure names which broke.
void ImportThroughSyncDb(TrackedECDb& briefcase, SchemaSyncDb& syncDb, std::vector<SchemaItem> const& schemas, bvector<Utf8String> const& adopt) {
    syncDb.WithReadWrite([&](ECDbR sync) {
        ASSERT_EQ(SchemaImportResult::OK, SchemaSyncTestFixture::ImportSchemas(sync, schemas, SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
    });
    ASSERT_EQ(SchemaSync::Status::OK, briefcase.Schemas().GetSchemaSync().AdoptSchemas(syncDb.GetSyncDbUri(), adopt));
    ASSERT_EQ(BE_SQLITE_OK, briefcase.SaveChanges());
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

// ...and is that column a dedicated one or a slot out of the shared pool? DbColumn::Kind is internal to
// ECDb, so its persisted values are spelled out here: 0 is a column of its own, 4 is a shared one. The
// two take different routes through SchemaWriter::DeleteProperty, so a test about deletion has to say
// which one it is exercising.
constexpr int COLUMN_KIND_DEFAULT = 0;

int ColumnKindOf(ECDbR db, Utf8CP schemaName, Utf8CP className, Utf8CP accessString) {
    Statement stmt;
    if (stmt.Prepare(db, R"sql(
        SELECT col.ColumnKind FROM main.ec_PropertyMap pm
        JOIN main.ec_Column col ON col.Id = pm.ColumnId
        JOIN main.ec_PropertyPath pp ON pp.Id = pm.PropertyPathId
        JOIN main.ec_Class c ON c.Id = pm.ClassId
        JOIN main.ec_Schema s ON s.Id = c.SchemaId
        WHERE s.Name=? AND c.Name=? AND pp.AccessString=?)sql") != BE_SQLITE_OK)
        return -1;
    stmt.BindText(1, schemaName, Statement::MakeCopy::No);
    stmt.BindText(2, className, Statement::MakeCopy::No);
    stmt.BindText(3, accessString, Statement::MakeCopy::No);
    return stmt.Step() == BE_SQLITE_ROW ? stmt.GetValueInt(0) : -1;
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

// Holds schemas plus the context that owns them, so the caller can keep both alive with one object.
struct BriefcaseSchemas final {
    ECSchemaReadContextPtr m_context;
    bvector<ECSchemaPtr> m_owned;
    bvector<ECSchemaCP> m_refs;
    bvector<ECSchemaCP> const& Refs() const { return m_refs; }
    bool IsValid() const { return !m_refs.empty(); }
};

// Loads against the briefcase, as the ordinary import path does. Re-pointing at the sync db is
// SchemaSync's own job.
BriefcaseSchemas LoadSchemas(ECDbR briefcase, std::vector<SchemaItem> const& items) {
    BriefcaseSchemas result;
    result.m_context = ECSchemaReadContext::CreateContext();
    result.m_context->AddSchemaLocater(briefcase.GetSchemaLocater());
    for (auto const& item : items) {
        ECSchemaPtr schema;
        const auto readStatus = ECSchema::ReadFromXmlString(schema, item.GetXmlString().c_str(), *result.m_context);
        if (readStatus != SchemaReadStatus::Success || !schema.IsValid()) {
            // Without this the caller sees ImportSchemas fail on an empty list, which says nothing
            // about the schema being unreadable.
            ADD_FAILURE() << "could not read the test schema (SchemaReadStatus " << (int)readStatus << "):\n" << item.GetXmlString().c_str();
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

// b2 is created after the init changeset, so it picks schema sync up from the timeline.
void SetupSyncedPair(ECDbHub& hub, SchemaSyncDb& syncDb, std::unique_ptr<TrackedECDb>& b1, std::unique_ptr<TrackedECDb>& b2) {
    b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("init schema sync");
    b2 = hub.CreateBriefcase();
}

// Does the physical table exist, as opposed to merely being described by ec_Table?
bool HasPhysicalTable(ECDbR db, Utf8CP tableName) {
    Statement stmt;
    if (stmt.Prepare(db, "SELECT 1 FROM main.sqlite_master WHERE type='table' AND name=?") != BE_SQLITE_OK)
        return false;
    stmt.BindText(1, tableName, Statement::MakeCopy::No);
    return stmt.Step() == BE_SQLITE_ROW;
}

// Hoisting movingProp onto the common base consolidates two shared columns into one, which moves
// data. RemapManager only ever considers ColumnKind = 4, so shared columns are what makes it a remap.
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

// Reading through ECSql is what proves the layout and the metadata still agree after a move.
Utf8String ReadStringProperty(ECDbR db, Utf8CP ecsql) {
    ECSqlStatement stmt;
    if (stmt.Prepare(db, ecsql) != ECSqlStatus::Success)
        return "<prepare failed>";
    if (stmt.Step() != BE_SQLITE_ROW)
        return "<no row>";
    return Utf8String(stmt.GetValueText(0));
}

// The CREATE statement SQLite kept for a table, index or trigger.
Utf8String DdlOf(ECDbR db, Utf8CP objectName) {
    Statement stmt;
    if (stmt.Prepare(db, "SELECT sql FROM main.sqlite_master WHERE name=?") != BE_SQLITE_OK)
        return "";
    stmt.BindText(1, objectName, Statement::MakeCopy::No);
    return stmt.Step() == BE_SQLITE_ROW ? Utf8String(stmt.GetValueText(0)) : Utf8String("");
}

// b1 imports and keeps it; b2 imports the same schema, so the sync db decides nothing the second
// time and no DDL exists anywhere for b2 to replay. Compared down to the DDL, since foreign keys and
// triggers live nowhere else.
// @param checkAdopted assertions against b2, without which two equally broken files still compare equal.
void ExpectSchemaConvergesWithNoDeltaToReplay(Utf8CP containerName, std::vector<SchemaItem> const& schemas, Utf8CP context,
                                              std::function<void(ECDbR)> checkAdopted = nullptr) {
    ECDbHub hub;
    SchemaSyncDb syncDb(containerName);
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    for (auto const& schema : schemas) {
        ASSERT_EQ(SchemaImportResult::OK, SchemaSyncTestFixture::ImportSchema(*b1, schema, SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()))
            << context << ": the first briefcase could not import the schema at all";
        ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    }
    for (auto const& schema : schemas) {
        ASSERT_EQ(SchemaImportResult::OK, SchemaSyncTestFixture::ImportSchema(*b2, schema, SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
        ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    }

    SchemaSyncTestFixture::ExpectNoForeignKeyViolations(*b2, context);
    if (checkAdopted != nullptr)
        checkAdopted(*b2);
    SchemaSyncTestFixture::ExpectECTablesIdentical(*b2, *b1, context);
    SchemaSyncTestFixture::ExpectPhysicalSchemaIdentical(*b2, *b1, context);
}

// Versions differ only in metadata, so concurrent editors disagree about a row's contents rather
// than about layout. From 1.0.1 a second class carries a label of its own.
SchemaItem MetadataOnlySchema(Utf8CP version, Utf8CP labelOfExisting, Utf8CP labelOfAdded = nullptr) {
    Utf8String added;
    if (labelOfAdded != nullptr)
        added.Sprintf(R"xml(<ECEntityClass typeName="Added" displayLabel="%s">
                <ECProperty propertyName="value" typeName="string" />
            </ECEntityClass>)xml", labelOfAdded);

    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="LabelTest" alias="lbl" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <ECEntityClass typeName="Existing" displayLabel="%s">
                <ECProperty propertyName="value" typeName="string" />
            </ECEntityClass>
            %s
        </ECSchema>)xml", version, labelOfExisting, added.c_str());
    return SchemaItem(xml);
}

// The value the two importers disagree about.
Utf8String DisplayLabelOf(ECDbR db, Utf8CP schemaName, Utf8CP className) {
    Statement stmt;
    if (stmt.Prepare(db, R"sql(
        SELECT c.DisplayLabel FROM main.ec_Class c
        JOIN main.ec_Schema s ON s.Id = c.SchemaId
        WHERE s.Name=? AND c.Name=?)sql") != BE_SQLITE_OK)
        return "<prepare failed>";
    stmt.BindText(1, schemaName, Statement::MakeCopy::No);
    stmt.BindText(2, className, Statement::MakeCopy::No);
    if (stmt.Step() != BE_SQLITE_ROW)
        return "<no such class>";
    Utf8CP label = stmt.GetValueText(0);
    return label == nullptr ? Utf8String("<null>") : Utf8String(label);
}

// Nothing constrains this: an update takes only a shared lock, so either briefcase may push first.
enum class PushOrder {
    ImportOrder,        // the briefcase that imported first also pushes first
    ReverseImportOrder, // the other way round
};

// Two briefcases import overlapping edits before either pushes, so the sync db serialises them and
// ends holding the second importer's answer - what everybody has to converge on. A third briefcase
// imports nothing and learns only from the timeline.
struct ConcurrentEditScenario final {
    ECDbHub m_hub;
    SchemaSyncDb m_syncDb;
    std::unique_ptr<TrackedECDb> m_firstImporter;
    std::unique_ptr<TrackedECDb> m_secondImporter;
    std::unique_ptr<TrackedECDb> m_bystander;

    explicit ConcurrentEditScenario(Utf8CP containerName) : m_syncDb(containerName) {}

    void Start(std::vector<SchemaItem> const& baseSchemas) {
        m_firstImporter = m_hub.CreateBriefcase();
        ASSERT_EQ(SchemaSync::Status::OK, m_firstImporter->Schemas().GetSchemaSync().Init(m_syncDb.GetSyncDbUri(), "upstream-container", false));
        ASSERT_EQ(BE_SQLITE_OK, m_firstImporter->SaveChanges());
        m_firstImporter->PullMergePush("init schema sync");
        m_secondImporter = m_hub.CreateBriefcase();
        m_bystander = m_hub.CreateBriefcase();

        for (auto const& schema : baseSchemas) {
            ASSERT_EQ(SchemaImportResult::OK, SchemaSyncTestFixture::ImportSchema(*m_firstImporter, schema, SchemaManager::SchemaImportOptions::None, m_syncDb.GetSyncDbUri()));
            ASSERT_EQ(BE_SQLITE_OK, m_firstImporter->SaveChanges());
        }
        m_firstImporter->PullMergePush("the base schemas");
        for (auto* other : { m_secondImporter.get(), m_bystander.get() }) {
            other->PullMergePush("pick up the base schemas");
        }
    }

    void ImportConcurrently(SchemaItem const& firstEdit, SchemaItem const& secondEdit) {
        ASSERT_EQ(SchemaImportResult::OK, SchemaSyncTestFixture::ImportSchema(*m_firstImporter, firstEdit, SchemaManager::SchemaImportOptions::None, m_syncDb.GetSyncDbUri()));
        ASSERT_EQ(BE_SQLITE_OK, m_firstImporter->SaveChanges());
        ASSERT_EQ(SchemaImportResult::OK, SchemaSyncTestFixture::ImportSchema(*m_secondImporter, secondEdit, SchemaManager::SchemaImportOptions::None, m_syncDb.GetSyncDbUri()));
        ASSERT_EQ(BE_SQLITE_OK, m_secondImporter->SaveChanges());
    }

    void Exchange(PushOrder order) {
        auto* leads = order == PushOrder::ImportOrder ? m_firstImporter.get() : m_secondImporter.get();
        auto* trails = order == PushOrder::ImportOrder ? m_secondImporter.get() : m_firstImporter.get();

        leads->PullMergePush("first to push");
        trails->PullMergePush("second to push - merges the other's changeset while holding its own");
        leads->PullMergePush("and back, this time with nothing local");
        m_bystander->PullMergePush("a briefcase that imported nothing");
    }

    void ExpectEverybodyHolds(Utf8CP context, Utf8CP schemaVersion, Utf8CP className, Utf8CP displayLabel) {
        m_syncDb.WithReadOnly([&](ECDbR sync) {
            EXPECT_STREQ(schemaVersion, VersionOf(sync, "LabelTest").c_str()) << context << ": the sync db does not hold the version this test is built on";
            EXPECT_STREQ(displayLabel, DisplayLabelOf(sync, "LabelTest", className).c_str()) << context << ": the sync db does not hold the label this test is built on";
        });

        struct { Utf8CP name; TrackedECDb* db; } briefcases[] = {
            { "the briefcase that imported first", m_firstImporter.get() },
            { "the briefcase that imported second", m_secondImporter.get() },
            { "the briefcase that only pulled", m_bystander.get() },
        };
        for (auto const& briefcase : briefcases) {
            EXPECT_STREQ(schemaVersion, VersionOf(*briefcase.db, "LabelTest").c_str()) << context << ": " << briefcase.name << " is on a different schema version than the sync db";
            EXPECT_STREQ(displayLabel, DisplayLabelOf(*briefcase.db, "LabelTest", className).c_str()) << context << ": " << briefcase.name << " holds a display label the sync db never decided on";
        }
        m_syncDb.WithReadOnly([&](ECDbR sync) {
            for (auto const& briefcase : briefcases)
                SchemaSyncTestFixture::ExpectECTablesIdentical(*briefcase.db, sync, Utf8PrintfString("%s: %s", context, briefcase.name).c_str());
        });
    }
};

} // namespace

// ---------------------------------------------------------------------------------------
// Init mirrors this briefcase, so this briefcase has to be what everyone else will see. A briefcase
// behind the tip seeds a sync db missing schemas the others already hold, and the next import into
// it hands out ids they are using for something else.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, InitRefusesBriefcaseBehindTheTip)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-init-behind-tip");

    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b2, UnrelatedSchema()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, b2->PullMergePush("schema b1 has not seen"));

    ASSERT_FALSE(HasSchema(*b1, "UnrelatedTest")) << "b1 is supposed to be behind for this test to mean anything";
    ASSERT_EQ(SchemaSync::Status::ERROR_BRIEFCASE_NOT_LEVEL_WITH_TIMELINE,
        b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    EXPECT_FALSE(b1->Schemas().GetSchemaSync().IsEnabled()) << "a refused Init must leave the briefcase alone";

    ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("catch up"));
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_TRUE(HasSchema(sync, "UnrelatedTest")) << "the sync db must know every schema the timeline already carries";
    });
    }

// ---------------------------------------------------------------------------------------
// The mirror image: unpushed changes would put the sync db ahead of the timeline, and if the push
// then never happens no briefcase can ever reach the state it describes.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, InitRefusesBriefcaseHoldingUnpushedChanges)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-init-unpushed");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, UnrelatedSchema()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    ASSERT_EQ(SchemaSync::Status::ERROR_BRIEFCASE_NOT_LEVEL_WITH_TIMELINE,
        b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    EXPECT_FALSE(b1->Schemas().GetSchemaSync().IsEnabled());

    ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("push the schema first"));
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    }

// ---------------------------------------------------------------------------------------
// The sync db mirrors the briefcase's metadata, which includes the profile properties describing
// the ec_ tables - ec_Db/InitialSchemaVersion among them, which DbMappingManager still branches on
// when it decides index names. What must not travel is localDbInfo, which would make the sync db
// look like a schema sync client of itself.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, InitMirrorsProfilePropertiesWithoutLocalSyncState)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-init-profile-props");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    const auto initialVersion = PropertySpec("InitialSchemaVersion", "ec_Db");
    const auto profileVersion = PropertySpec("SchemaVersion", "ec_Db");
    const auto localDbInfo = PropertySpec("localDbInfo", "ec_Db");
    const auto syncDbInfo = PropertySpec("syncDbInfo", "ec_Db");

    Utf8String briefcaseInitial, briefcaseProfile;
    ASSERT_EQ(BE_SQLITE_ROW, b1->QueryProperty(briefcaseInitial, initialVersion));
    ASSERT_EQ(BE_SQLITE_ROW, b1->QueryProperty(briefcaseProfile, profileVersion));

    syncDb.WithReadOnly([&](ECDbR sync) {
        Utf8String value;
        ASSERT_EQ(BE_SQLITE_ROW, sync.QueryProperty(value, initialVersion))
            << "the sync db decides the mapping and has to see the same InitialSchemaVersion the briefcase would";
        EXPECT_STREQ(briefcaseInitial.c_str(), value.c_str());

        ASSERT_EQ(BE_SQLITE_ROW, sync.QueryProperty(value, profileVersion));
        EXPECT_STREQ(briefcaseProfile.c_str(), value.c_str());

        EXPECT_EQ(BE_SQLITE_ROW, sync.QueryProperty(value, syncDbInfo));
        EXPECT_NE(BE_SQLITE_ROW, sync.QueryProperty(value, localDbInfo))
            << "the sync db must not look like a schema sync client";
        EXPECT_FALSE(sync.Schemas().GetSchemaSync().IsEnabled());
    });
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, SyncDbMappingMatchesBriefcaseMapping)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-mapping-matches");

    auto b1 = hub.CreateBriefcase();
    auto control = hub.CreateBriefcase();

    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("init schema sync");

    const auto schema = SharedColumnSchema();

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*control, schema));
    ASSERT_EQ(BE_SQLITE_OK, control->SaveChanges());
    const auto briefcaseSchemaHash = GetSchemaHash(*control);
    const auto briefcaseMapHash = GetMapHash(*control);
    ASSERT_FALSE(briefcaseSchemaHash.empty());
    ASSERT_FALSE(briefcaseMapHash.empty());

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
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, MappingStillMatchesAfterDependentImport)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-mapping-second-import");

    auto b1 = hub.CreateBriefcase();
    auto control = hub.CreateBriefcase();

    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("init schema sync");

    const auto first = SharedColumnSchema("01.00.00", 8);
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
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, ImportIntoSyncDbCreatesNoDataTables)
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
//=======================================================================================

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, AdoptMakesBriefcaseMatchSyncDb)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-adopt-complete");

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

    ASSERT_TRUE(HasSchema(*b2, "UpstreamTest"));

    syncDb.WithReadOnly([&](ECDbR sync) {
        ExpectECTablesIdentical(*b2, sync, "after adopting the only new schema");
    });
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, AdoptLeavesUnrelatedSchemasBehind)
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

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*b2, "SELECT COUNT(*) FROM main.ec_Class WHERE Name='Loner'"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(0, stmt.GetValueInt(0)) << "the unrelated schema's class leaked in";

    EXPECT_TRUE(ForeignkeyCheck(*b2)) << "filtered adopt left dangling foreign keys";
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// AdoptLeavesUnrelatedSchemasBehind writes both schemas into the sync db by hand, which proves the
// closure filter over rows that are simply sitting there. This is the case the filter exists for:
// the extra rows belong to a real briefcase's real import that has not been pushed, so they describe
// a schema no changeset has ever mentioned. A briefcase that adopted them would build tables for a
// schema it cannot explain to anyone, and would publish ids the timeline never agreed to.
TEST_F(SchemaSyncImportTestFixture, AnUnpushedImportStaysOutOfEveryOtherBriefcase)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-unpushed-isolation");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto b3 = hub.CreateBriefcase();

    // b1 imports and keeps it local. The sync db now holds UnrelatedTest; the timeline does not.
    ASSERT_EQ(SchemaSync::Status::OK,
              b1->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b1, { UnrelatedSchema() }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    ASSERT_TRUE(HasSchema(*b1, "UnrelatedTest")) << "the importing briefcase did not get its own schema";

    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_TRUE(HasSchema(sync, "UnrelatedTest")) << "the sync db did not record the unpushed import";
    });

    // b2 imports something else through the same sync db, which is where the filtering happens.
    ASSERT_EQ(SchemaSync::Status::OK,
              b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { SharedColumnSchema() }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    EXPECT_TRUE(HasSchema(*b2, "UpstreamTest")) << "the schema b2 asked for was not adopted";
    EXPECT_FALSE(HasSchema(*b2, "UnrelatedTest")) << "b1's unpushed import leaked into b2";

    Statement lonerStmt;
    ASSERT_EQ(BE_SQLITE_OK, lonerStmt.Prepare(*b2, "SELECT COUNT(*) FROM main.ec_Class WHERE Name='Loner'"));
    ASSERT_EQ(BE_SQLITE_ROW, lonerStmt.Step());
    EXPECT_EQ(0, lonerStmt.GetValueInt(0)) << "the class of b1's unpushed import leaked into b2";

    EXPECT_FALSE(HasPhysicalTable(*b2, "unrel_Loner")) << "b2 built a table for a schema it never adopted";
    EXPECT_TRUE(ForeignkeyCheck(*b2)) << "filtering the unpushed rows out left dangling references";

    // Pushing b2 must not hand it to anybody else either - the changeset carries only what b2 adopted.
    ASSERT_EQ(BE_SQLITE_OK, b2->PullMergePush("add UpstreamTest while UnrelatedTest is unpushed"));
    ASSERT_EQ(BE_SQLITE_OK, b3->PullMergePush("pick up UpstreamTest"));
    EXPECT_TRUE(HasSchema(*b3, "UpstreamTest"));
    EXPECT_FALSE(HasSchema(*b3, "UnrelatedTest")) << "b1's unpushed import reached a third briefcase through the timeline";

    // b1 still holds it, and pushing is what finally publishes it. The two schemas share the sync db
    // and neither references the other, so b1's push has to survive b2's having landed first.
    ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("publish the import b1 had been sitting on"));
    ASSERT_EQ(BE_SQLITE_OK, b2->PullMergePush("catch up"));
    ASSERT_EQ(BE_SQLITE_OK, b3->PullMergePush("catch up"));

    for (auto* bc : { b1.get(), b2.get(), b3.get() })
        {
        EXPECT_TRUE(HasSchema(*bc, "UnrelatedTest")) << "the formerly unpushed schema did not arrive after it was pushed";
        EXPECT_TRUE(HasSchema(*bc, "UpstreamTest"));
        VerifyFileIsSound(*bc, "after the unpushed import was finally published");
        }

    ExpectECTablesIdentical(*b2, *b1, "b2 against b1 once everything is pushed");
    ExpectECTablesIdentical(*b3, *b1, "b3 against b1 once everything is pushed");
    ExpectPhysicalSchemaIdentical(*b2, *b1, "b2 against b1 once everything is pushed");
    ExpectPhysicalSchemaIdentical(*b3, *b1, "b3 against b1 once everything is pushed");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, AdoptPullsReferencedSchemas)
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
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, AdoptedSchemaAcceptsData)
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

    // p1 and p8 straddle the shared-column budget, so this covers the overflow table too.
    ECSqlStatement select;
    ASSERT_EQ(ECSqlStatus::Success, select.Prepare(*b2, "SELECT baseProp,p1,p8 FROM ut.Derived WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, select.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, select.Step());
    EXPECT_STREQ("hello", select.GetValueText(0));
    EXPECT_EQ(42, select.GetValueInt(1));
    EXPECT_EQ(99, select.GetValueInt(2));
    }

//=======================================================================================
// Concurrent imports, with the two steps driven by hand.
//=======================================================================================

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, ConcurrentImportsDoNotShareAColumn)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-concurrent-columns");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("init schema sync");

    auto b2 = hub.CreateBriefcase();

    ImportThroughSyncDb(*b1, syncDb, { MachinerySchema("01.00.00", false) }, { "Machinery" });
    b1->PullMergePush("add Machinery");
    b2->PullMergePush("pick up Machinery");
    ASSERT_TRUE(HasSchema(*b2, "Machinery"));

    ImportThroughSyncDb(*b1, syncDb, { MachinerySchema("01.00.01", true) }, { "Machinery" });
    ImportThroughSyncDb(*b2, syncDb, { TankSchema() }, { "DemoB" });

    EXPECT_TRUE(HasSchema(*b2, "DemoB"));
    Statement ratingCheck;
    ASSERT_EQ(BE_SQLITE_OK, ratingCheck.Prepare(*b2, "SELECT COUNT(*) FROM main.ec_Property WHERE Name='rating'"));
    ASSERT_EQ(BE_SQLITE_ROW, ratingCheck.Step());
    EXPECT_EQ(1, ratingCheck.GetValueInt(0)) << "the referenced schema's new property did not come along";

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

    for (auto* db : { b2.get() }) {
        const auto ratingCol = ColumnOf(*db, "Machinery", "Machine", "rating");
        const auto volumeCol = ColumnOf(*db, "DemoB", "Tank", "volume");
        EXPECT_FALSE(ratingCol.empty()) << "rating is not mapped";
        EXPECT_FALSE(volumeCol.empty()) << "volume is not mapped";
        EXPECT_STRNE(ratingCol.c_str(), volumeCol.c_str())
            << "rating and volume were double-booked into the same shared column - this is the "
               "silent corruption the sync db exists to prevent";
    }

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
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, ConcurrentImportsConvergeAfterExchange)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-converge-exchange");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("init schema sync");

    auto b2 = hub.CreateBriefcase();

    ImportThroughSyncDb(*b1, syncDb, { MachinerySchema("01.00.00", false) }, { "Machinery" });
    b1->PullMergePush("add Machinery");
    b2->PullMergePush("pick up Machinery");

    ImportThroughSyncDb(*b1, syncDb, { MachinerySchema("01.00.01", true) }, { "Machinery" });
    ImportThroughSyncDb(*b2, syncDb, { TankSchema() }, { "DemoB" });

    b1->PullMergePush("b1 pushes rating");
    b2->PullMergePush("b2 merges rating, pushes tank");
    b1->PullMergePush("b1 merges tank");

    ExpectECTablesIdentical(*b2, *b1, "after both briefcases exchanged changesets");
    syncDb.WithReadOnly([&](ECDbR sync) {
        ExpectECTablesIdentical(*b1, sync, "briefcase vs sync db after convergence");
    });
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, SyncDbRefusesConflictingPropertyType)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-conflicting-type");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("init schema sync");

    ImportThroughSyncDb(*b1, syncDb, { MachinerySchema("01.00.00", false) }, { "Machinery" });

    ImportThroughSyncDb(*b1, syncDb, { MachinerySchema("01.00.01", true) }, { "Machinery" });

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
        EXPECT_NE(SchemaImportResult::OK, ImportSchema(sync, conflicting, SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables))
            << "the sync db accepted a conflicting property type - the authority is not authoritative";
    });
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, SyncDbRefusesImportNeedingDataTransform)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-remap-refused");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("init schema sync");

    // Shared columns are what makes this a remap: RemapManager only considers ColumnKind = 4.
    ImportThroughSyncDb(*b1, syncDb, { RemapSchema("01.00.00", false) }, { "RemapTest" });

    syncDb.WithReadOnly([&](ECDbR sync) {
        ASSERT_STRNE(ColumnOf(sync, "RemapTest", "LeafA", "movingProp").c_str(),
                     ColumnOf(sync, "RemapTest", "LeafB", "movingProp").c_str())
            << "the siblings already share a column, so hoisting cannot force a move - this "
               "scenario no longer tests what it claims to";
    });

    syncDb.WithReadWrite([&](ECDbR sync) {
        // The transform list is built from mapping changes, not row counts, so no data is needed.
        EXPECT_EQ(SchemaImportResult::ERROR_DATA_TRANSFORM_REQUIRED,
                  ImportSchema(sync, RemapSchema("01.00.01", true), SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables))
            << "a data-moving change was accepted on the additive path; it must be routed to the "
               "upgrade front door instead";
    });
    }

//=======================================================================================
// The orchestration entry point: SchemaSync::ImportSchemas does both steps in one call.
//=======================================================================================

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, ImportSchemasDoesBothSteps)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-entrypoint");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto schemas = LoadSchemas(*b2, { SharedColumnSchema() });
    ASSERT_TRUE(schemas.IsValid()) << "could not load the schema";

    ASSERT_EQ(SchemaSync::Status::OK, b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), schemas.Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    EXPECT_TRUE(HasSchema(*b2, "UpstreamTest")) << "the briefcase did not adopt what it imported";
    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_TRUE(HasSchema(sync, "UpstreamTest")) << "the sync db did not receive the import";
        ExpectECTablesIdentical(*b2, sync, "after a single ImportSchemas call");
    });

    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO ut.Derived(baseProp,p1,p8) VALUES('hello',42,99)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, ImportSchemasWorksOnASyncDbInitialisedFromABriefcaseWithTables)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-nonempty-init");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, SharedColumnSchema()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("schema that predates schema sync"));
    ASSERT_TRUE(HasPhysicalTable(*b1, "ut_Base")) << "the scenario did not set itself up";

    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("init schema sync");

    const auto schemas = LoadSchemas(*b1, { UnrelatedSchema() });
    ASSERT_TRUE(schemas.IsValid());

    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), schemas.Refs(), SchemaManager::SchemaImportOptions::None))
        << "an import into a sync db made from a non-empty briefcase failed";
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    EXPECT_TRUE(HasSchema(*b1, "UnrelatedTest"));
    EXPECT_TRUE(HasSchema(*b1, "UpstreamTest")) << "the schema that predated schema sync was disturbed";
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, ImportSchemasResolvesReferencesAgainstSyncDb)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-entrypoint-refs");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    ImportThroughSyncDb(*b1, syncDb, { MachinerySchema("01.00.00", false) }, { "Machinery" });
    b1->PullMergePush("add Machinery 1.0.0");
    b2->PullMergePush("pick up Machinery 1.0.0");
    ASSERT_STREQ("1.0.0", VersionOf(*b2, "Machinery").c_str());

    ImportThroughSyncDb(*b1, syncDb, { MachinerySchema("01.00.01", true) }, { "Machinery" });

    const auto tank = LoadSchemas(*b2, { TankSchema() });
    ASSERT_TRUE(tank.IsValid());
    ASSERT_EQ(SchemaSync::Status::OK, b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), tank.Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    EXPECT_TRUE(HasSchema(*b2, "DemoB"));
    EXPECT_STREQ("1.0.1", VersionOf(*b2, "Machinery").c_str())
        << "the referenced schema was not updated to the version the sync db decided against";

    // rating and volume compete for the same pool; mapping against 1.0.0 would not have seen rating.
    const auto ratingColumn = ColumnOf(*b2, "Machinery", "Machine", "rating");
    const auto volumeColumn = ColumnOf(*b2, "DemoB", "Tank", "volume");
    EXPECT_FALSE(ratingColumn.empty()) << "rating did not come along with the reference update";
    EXPECT_STRNE(ratingColumn.c_str(), volumeColumn.c_str())
        << "rating and volume were placed in the same shared column";
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, ImportSchemasRefusesDataTransform)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-entrypoint-transform");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto initialSchemas = LoadSchemas(*b2, { RemapSchema("01.00.00", false) });
    ASSERT_TRUE(initialSchemas.IsValid());

    ASSERT_EQ(SchemaSync::Status::OK, b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), initialSchemas.Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    ASSERT_STREQ("1.0.0", VersionOf(*b2, "RemapTest").c_str());

    EXPECT_EQ(SchemaSync::Status::ERROR_DATA_TRANSFORM_REQUIRED,
              b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { RemapSchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None))
        << "a data-moving change was accepted on the additive path";

    EXPECT_STREQ("1.0.0", VersionOf(*b2, "RemapTest").c_str()) << "the briefcase was changed by a refused import";
    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_STREQ("1.0.0", VersionOf(sync, "RemapTest").c_str()) << "the sync db was changed by a refused import";
    });
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, ImportSchemasCatchesUpInstancesThatSpillIntoOverflow)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-entrypoint-overflow");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    // Two properties fit the shared-column budget; eight do not.
    const auto narrow = LoadSchemas(*b2, { SharedColumnSchema("01.00.00", 2) });
    ASSERT_TRUE(narrow.IsValid());
    ASSERT_TRUE(narrow.IsValid());

    ASSERT_EQ(SchemaSync::Status::OK, b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), narrow.Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    ASSERT_TRUE(TableOf(*b2, "UpstreamTest", "Derived", "p1").length() > 0);

    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO ut.Derived(baseProp,p1) VALUES('before',1)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    ASSERT_EQ(SchemaSync::Status::OK, b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { SharedColumnSchema("01.00.01", 8) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    const auto primaryTable = TableOf(*b2, "UpstreamTest", "Derived", "p1");
    const auto overflowTable = TableOf(*b2, "UpstreamTest", "Derived", "p8");
    ASSERT_FALSE(overflowTable.empty()) << "p8 was not mapped at all";
    ASSERT_STRNE(primaryTable.c_str(), overflowTable.c_str())
        << "nothing spilled into an overflow table, so this scenario no longer tests what it claims to";

    // The instance that predates the widening needs a row in the overflow table.
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(*b2, SqlPrintfString("SELECT COUNT(*) FROM main.[%s] WHERE Id=?", overflowTable.c_str()).GetUtf8CP()));
    stmt.BindId(1, key.GetInstanceId());
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(1, stmt.GetValueInt(0)) << "the pre-existing instance was never given an overflow row";
    }

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
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, ConcurrentImportsThroughEntryPointConverge)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-entrypoint-converge");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto machinery100 = MachinerySchema("01.00.00", false);
    const auto machinery101 = MachinerySchema("01.00.01", true);

    auto& sync1 = b1->Schemas().GetSchemaSync();
    auto& sync2 = b2->Schemas().GetSchemaSync();

    ASSERT_EQ(SchemaSync::Status::OK, sync1.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b1, { machinery100 }).Refs(), SchemaManager::SchemaImportOptions::None));
    ExpectNoForeignKeyViolations(*b1, "b1 after adopting Machinery 1.0.0");
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_FALSE(HasPhysicalTable(sync, "mch_Machine"))
            << "the import created a data table in the sync db";
    });

    b1->PullMergePush("add Machinery");
    b2->PullMergePush("pick up Machinery");

    ASSERT_EQ(SchemaSync::Status::OK, sync1.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b1, { machinery101 }).Refs(), SchemaManager::SchemaImportOptions::None));
    ExpectNoForeignKeyViolations(*b1, "b1 after adopting Machinery 1.0.1");
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { TankSchema() }).Refs(), SchemaManager::SchemaImportOptions::None));
    ExpectNoForeignKeyViolations(*b2, "b2 after adopting DemoB");
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    EXPECT_STRNE(ColumnOf(*b2, "Machinery", "Machine", "rating").c_str(),
                 ColumnOf(*b2, "DemoB", "Tank", "volume").c_str())
        << "two concurrent additive imports were given the same shared column";

    b1->PullMergePush("b1 pushes rating");
    b2->PullMergePush("b2 merges rating, pushes tank");
    b1->PullMergePush("b1 merges tank");

    ExpectECTablesIdentical(*b2, *b1, "after both briefcases exchanged changesets");
    syncDb.WithReadOnly([&](ECDbR sync) {
        ExpectECTablesIdentical(*b1, sync, "briefcase vs sync db after convergence");
    });
    }

// ---------------------------------------------------------------------------------------
// Two briefcases import divergent versions of the same schema through one shared sync db before
// either pushes. The first takes it to 1.0.2 (two properties). The sync db is written immediately,
// so the second briefcase's attempt to import the older 1.0.1 (one property) is a downgrade the sync
// db refuses. After both push, a briefcase built fresh from the timeline holds the surviving 1.0.2.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, ConcurrentImportsThroughSyncDbSameSchemaDivergentVersions)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-divergent-versions");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    // 1.0.1 has one property (p1); 1.0.2 keeps p1 and adds a second (p2).
    const auto v101 = SharedColumnSchema("01.00.01", 1);
    const auto v102 = SharedColumnSchema("01.00.02", 2);

    // 1) The first briefcase imports 1.0.2 through the sync db, without pushing to the timeline.
    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, v102, SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    EXPECT_STREQ("1.0.2", VersionOf(*b1, "UpstreamTest").c_str());
    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_STREQ("1.0.2", VersionOf(sync, "UpstreamTest").c_str()) << "the import did not reach the sync db";
    });

    // 2) The second briefcase imports the older 1.0.1 through the same sync db, without pushing. The
    //    sync db already holds 1.0.2, so this is a downgrade. What happens is b2 gets schema version 1.0.2.
    EXPECT_EQ(SchemaImportResult::OK, ImportSchema(*b2, v101, SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()))
        << "the sync db accepted an older schema version over a newer one";
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_STREQ("1.0.2", VersionOf(*b2, "UpstreamTest").c_str());
    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_STREQ("1.0.2", VersionOf(sync, "UpstreamTest").c_str()) << "the import did not reach the sync db";
    });

    // 3) Both push. b1 carries 1.0.2 to the timeline; b2 has nothing local, so it merely catches up.
    b1->PullMergePush("push 1.0.2");
    b2->PullMergePush("b2 just merges 1.0.2");
    EXPECT_STREQ("1.0.2", VersionOf(*b2, "UpstreamTest").c_str()) << "b2 did not converge on the surviving version";

    // 4) A briefcase built fresh from the timeline holds 1.0.2, with both properties.
    auto b3 = hub.CreateBriefcase();
    EXPECT_STREQ("1.0.2", VersionOf(*b3, "UpstreamTest").c_str());
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(*b3, "SELECT p1, p2 FROM ut.Derived"))
        << "the third briefcase is missing a property that 1.0.2 should have";
    ExpectNoForeignKeyViolations(*b3, "third briefcase built from the timeline");
    ExpectECTablesIdentical(*b3, *b1, "third briefcase vs the briefcase that imported 1.0.2");
    }

//=======================================================================================
// The upgrade path: the import runs on the briefcase, and the sync db is rebuilt from the result.
// Legal only under the exclusive schema lock, which is what makes the overwrite's deletes safe.
//=======================================================================================

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, UpgradeSchemasMovesDataAndOverwritesSyncDb)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-upgrade");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto initial = RemapSchema("01.00.00", false);
    const auto hoisted = RemapSchema("01.00.01", true);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { initial }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

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

    ASSERT_STRNE(ColumnOf(*b2, "RemapTest", "LeafA", "movingProp").c_str(),
                 ColumnOf(*b2, "RemapTest", "LeafB", "movingProp").c_str())
        << "the siblings already share a column, so hoisting cannot force a move";

    ASSERT_EQ(SchemaSync::Status::ERROR_DATA_TRANSFORM_REQUIRED,
              sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { hoisted }).Refs(), SchemaManager::SchemaImportOptions::None));

    ASSERT_EQ(SchemaSync::Status::OK, sync2.UpgradeSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { hoisted }).Refs(), SchemaManager::SchemaImportOptions::None, nullptr));
    ExpectNoForeignKeyViolations(*b2, "b2 after upgrading");
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    EXPECT_STREQ("1.0.1", VersionOf(*b2, "RemapTest").c_str());
    EXPECT_STREQ("from A", ReadStringProperty(*b2, "SELECT movingProp FROM rmp.LeafA").c_str())
        << "data was lost when the column it lived in was consolidated";
    EXPECT_STREQ("from B", ReadStringProperty(*b2, "SELECT movingProp FROM rmp.LeafB").c_str())
        << "data was lost when the column it lived in was consolidated";

    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_STREQ("1.0.1", VersionOf(sync, "RemapTest").c_str()) << "the sync db did not receive the upgrade";
        ExpectECTablesIdentical(*b2, sync, "briefcase vs sync db after an upgrade");
    });
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, UpgradeSchemasDropsAbandonedSyncDbState)
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

    ASSERT_EQ(SchemaSync::Status::OK, sync1.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b1, { initial }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("add RemapTest 1.0.0");
    b2->PullMergePush("pick up RemapTest 1.0.0");

    // b1 imports something else and never pushes it: the sync db has it, the timeline does not.
    ASSERT_EQ(SchemaSync::Status::OK, sync1.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b1, { abandoned }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    syncDb.WithReadOnly([&](ECDbR sync) {
        ASSERT_TRUE(HasSchema(sync, "UnrelatedTest")) << "the scenario did not set itself up";
    });
    ASSERT_FALSE(HasSchema(*b2, "UnrelatedTest")) << "b2 was not supposed to learn about it";

    ASSERT_EQ(SchemaSync::Status::OK, sync2.UpgradeSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { hoisted }).Refs(), SchemaManager::SchemaImportOptions::None, nullptr));
    ExpectNoForeignKeyViolations(*b2, "b2 after upgrading over abandoned state");
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_FALSE(HasSchema(sync, "UnrelatedTest"))
            << "abandoned rows survived the overwrite, so the sync db still describes a schema nobody has";
        EXPECT_STREQ("1.0.1", VersionOf(sync, "RemapTest").c_str());
        ExpectECTablesIdentical(*b2, sync, "briefcase vs sync db after the overwrite");
    });

    const auto unrelatedAgain = LoadSchemas(*b2, { UnrelatedSchema() });
    ASSERT_TRUE(unrelatedAgain.IsValid());
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), unrelatedAgain.Refs(), SchemaManager::SchemaImportOptions::None));
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
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, EntryPointsRefuseProfileVersionSkew)
    {
    auto shiftSyncDbProfileVersion = [](SchemaSyncDb& syncDb, int by) {
        syncDb.WithReadWrite([&](ECDbR sync) {
            const auto current = sync.GetECDbProfileVersion();
            const ProfileVersion shifted(current.GetMajor(), current.GetMinor(), current.GetSub1(), (uint16_t)(current.GetSub2() + by));
            ASSERT_EQ(BE_SQLITE_OK, sync.SavePropertyString(PropertySpec("SchemaVersion", "ec_Db"), shifted.ToJson()));
            ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
        });
    };

    // VerifySyncDb lets the sync db be ahead on a pull. The new guard does not.
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-profile-skew-import");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);
    shiftSyncDbProfileVersion(syncDb, 1);

    const auto schemas = LoadSchemas(*b2, { SharedColumnSchema() });
    ASSERT_TRUE(schemas.IsValid());
    EXPECT_EQ(SchemaSync::Status::ERROR_PROFILE_VERSION_MISMATCH,
              b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), schemas.Refs(), SchemaManager::SchemaImportOptions::None));
    EXPECT_FALSE(HasSchema(*b2, "UpstreamTest")) << "the import was refused but still changed the briefcase";
    }

    // ...and behind on a push. Opposite skew, so the new guard is what refuses each one.
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-profile-skew-upgrade");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);
    shiftSyncDbProfileVersion(syncDb, -1);

    const auto schemas = LoadSchemas(*b2, { SharedColumnSchema() });
    ASSERT_TRUE(schemas.IsValid());
    EXPECT_EQ(SchemaSync::Status::ERROR_PROFILE_VERSION_MISMATCH,
              b2->Schemas().GetSchemaSync().UpgradeSchemas(syncDb.GetSyncDbUri(), schemas.Refs(), SchemaManager::SchemaImportOptions::None, nullptr));
    EXPECT_FALSE(HasSchema(*b2, "UpstreamTest")) << "the upgrade was refused but still changed the briefcase";
    }
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, PlainImportRoutesByTransformOption)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-routing");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto initial = RemapSchema("01.00.00", false);
    const auto hoisted = RemapSchema("01.00.01", true);

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b2, initial, SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_TRUE(HasSchema(sync, "RemapTest")) << "the additive path did not go through the sync db";
        ExpectECTablesIdentical(*b2, sync, "after an additive import through the ordinary path");
    });

    ASSERT_EQ(SchemaImportResult::ERROR_DATA_TRANSFORM_REQUIRED,
              ImportSchema(*b2, hoisted, SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));

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
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, RemapInOneBriefcaseSurvivesInTheOther)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-remap-across-briefcases");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto initial = RemapSchema("01.00.00", false);
    const auto hoisted = RemapSchema("01.00.01", true);

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, initial, SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("add RemapTest 1.0.0");
    b2->PullMergePush("pick up RemapTest 1.0.0");

    for (auto* briefcase : { b1.get(), b2.get() }) {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*briefcase, "INSERT INTO rmp.LeafB(baseProp,movingProp) VALUES('b','before the move')"));
        ECInstanceKey key;
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
        ASSERT_EQ(BE_SQLITE_OK, briefcase->SaveChanges());
    }
    b2->PullMergePush("b2's row, written before the upgrade");
    b1->PullMergePush("b1's row");

    ASSERT_EQ(SchemaImportResult::OK,
              ImportSchema(*b1, hoisted, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    ExpectNoForeignKeyViolations(*b1, "b1 after upgrading");
    b1->PullMergePush("hoist movingProp");

    b2->PullMergePush("pick up the hoist");

    EXPECT_STREQ("1.0.1", VersionOf(*b2, "RemapTest").c_str());
    EXPECT_STREQ("before the move", ReadStringProperty(*b1, "SELECT movingProp FROM rmp.LeafB").c_str())
        << "the briefcase that performed the remap lost its own data";
    EXPECT_STREQ("before the move", ReadStringProperty(*b2, "SELECT movingProp FROM rmp.LeafB").c_str())
        << "a briefcase that only pulled the remap cannot read data written before it";

    ExpectECTablesIdentical(*b2, *b1, "after the remap reached the second briefcase");
    syncDb.WithReadOnly([&](ECDbR sync) {
        ExpectECTablesIdentical(*b1, sync, "briefcase vs sync db after a remap");
    });

    for (auto* briefcase : { b1.get(), b2.get() }) {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*briefcase, "INSERT INTO rmp.LeafA(baseProp,filler,movingProp) VALUES('a','f','after the move')"));
        ECInstanceKey key;
        EXPECT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << "writing through the consolidated column failed";
        ASSERT_EQ(BE_SQLITE_OK, briefcase->SaveChanges());
    }
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, OrdinaryImportLeavesCacheTablesCurrent)
    {
    ECDbHub hub;
    auto db = hub.CreateBriefcase();

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*db, RelationshipSchema()));

    for (Utf8CP className : { "Parent", "Child", "Tag" }) {
        EXPECT_EQ(1, CountClassHierarchyEntries(*db, "RelTest", className)) << className;
        EXPECT_EQ(1, CountClassTableMappings(*db, "RelTest", className)) << className;
    }
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, OrdinaryImportDerivesRelationshipForeignKeysFromFreshCache)
    {
    ECDbHub hub;
    auto db = hub.CreateBriefcase();

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*db, RelationshipSchema()));

    const auto childDdl = DdlOf(*db, "rel_Child");
    EXPECT_TRUE(childDdl.ContainsI("FOREIGN KEY")) << "the navigation foreign key was not derived: " << childDdl.c_str();
    }

//=======================================================================================
// Scenario coverage carried over from the two earlier v2 prototypes. Both of their questions come
// down to one here: does an adopting briefcase end up with the same file, physical schema included.
//=======================================================================================

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, RelationshipsConvergeWithNoDeltaToReplay)
    {
    ExpectSchemaConvergesWithNoDeltaToReplay("upstream-relationships", { RelationshipSchema() },
        "a nav property and a link table adopted from the sync db",
        [](ECDbR adopted) {
            EXPECT_TRUE(HasPhysicalTable(adopted, "rel_ChildHasTags")) << "the link table was never built";
            EXPECT_FALSE(ColumnOf(adopted, "RelTest", "Child", "Owner.Id").empty()) << "the nav property was not mapped";

            // Two equally broken files compare equal, so assert the constraint is there.
            const auto childDdl = DdlOf(adopted, "rel_Child");
            EXPECT_TRUE(childDdl.ContainsI("FOREIGN KEY")) << "the nav property's foreign key was not derived: " << childDdl.c_str();
            const auto linkDdl = DdlOf(adopted, "rel_ChildHasTags");
            EXPECT_TRUE(linkDdl.ContainsI("FOREIGN KEY")) << "the link table's foreign keys were not derived: " << linkDdl.c_str();
        });
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, JoinedTablesConvergeWithNoDeltaToReplay)
    {
    ExpectSchemaConvergesWithNoDeltaToReplay("upstream-joined-tables", { JoinedTableSchema() },
        "a joined-table hierarchy adopted from the sync db",
        [](ECDbR adopted) {
            EXPECT_TRUE(HasPhysicalTable(adopted, "jnd_Sub1")) << "the joined table was never built";
            const auto joinedDdl = DdlOf(adopted, "jnd_Sub1");
            EXPECT_TRUE(joinedDdl.ContainsI("FOREIGN KEY")) << "the joined table's foreign key was not derived: " << joinedDdl.c_str();
        });
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, UnitsFormatsAndKindsOfQuantityAreAdopted)
    {
    ExpectSchemaConvergesWithNoDeltaToReplay("upstream-units", { UnitsAndFormatsSchema() },
        "units, formats, a kind of quantity and an enumeration adopted from the sync db",
        [](ECDbR adopted) {
            for (auto table : { "ec_UnitSystem", "ec_Phenomenon", "ec_Unit", "ec_Format", "ec_FormatCompositeUnit", "ec_KindOfQuantity", "ec_Enumeration" })
                EXPECT_GT(CountRows(adopted, table), 0) << table << " is empty, so the filter rule for it is still unmeasured";
        });
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, SiblingClassesShareTheSlotTheSyncDbGaveThem)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-sibling-slots");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, SiblingSlotSchema("01.00.00", false), SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("SlotTest 1.0.0");

    const auto firstColumn = ColumnOf(*b1, "SlotTest", "First", "firstProp");
    ASSERT_FALSE(firstColumn.empty()) << "firstProp was not mapped";

    // Nothing tells b2 which slot firstProp took except the adopted rows.
    b2->PullMergePush("pick up SlotTest 1.0.0");
    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b2, SiblingSlotSchema("01.00.01", true), SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    EXPECT_STREQ(firstColumn.c_str(), ColumnOf(*b2, "SlotTest", "First", "firstProp").c_str())
        << "the property that was already mapped moved";
    EXPECT_STREQ(firstColumn.c_str(), ColumnOf(*b2, "SlotTest", "Second", "secondProp").c_str())
        << "the sibling was given a slot of its own instead of reusing the one next to it";

    b2->PullMergePush("SlotTest 1.0.1");
    b1->PullMergePush("pick up SlotTest 1.0.1");

    ExpectECTablesIdentical(*b1, *b2, "after a sibling class joined a shared-column pool");
    ExpectPhysicalSchemaIdentical(*b1, *b2, "after a sibling class joined a shared-column pool");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, CurrentTimeStampTriggerReachesEveryBriefcase)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-timestamp-trigger");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, TimeStampSchema(), SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    Utf8CP triggerName = "stp_Stamped_CurrentTimeStamp";
    EXPECT_TRUE(HasTrigger(*b1, triggerName)) << "the briefcase that adopted the schema has no trigger";

    b1->PullMergePush("add StampTest");
    b2->PullMergePush("pick up StampTest");
    EXPECT_TRUE(HasTrigger(*b2, triggerName)) << "a briefcase that merged the schema changeset has no trigger";

    auto b3 = hub.CreateBriefcase();
    EXPECT_TRUE(HasTrigger(*b3, triggerName)) << "a briefcase built from the whole timeline has no trigger";

    ExpectPhysicalSchemaIdentical(*b2, *b1, "briefcase that merged the changeset");
    ExpectPhysicalSchemaIdentical(*b3, *b1, "briefcase built from the timeline");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, BriefcaseFromTheTimelineConvergesWithTheImporter)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-timeline-briefcase");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, RelationshipSchema(), SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("b1 adds RelTest");

    b2->PullMergePush("b2 picks up RelTest");
    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b2, UnitsAndFormatsSchema(), SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("b2 adds UnitTest");

    b1->PullMergePush("b1 picks up UnitTest");

    auto b3 = hub.CreateBriefcase();

    EXPECT_TRUE(HasSchema(*b3, "RelTest"));
    EXPECT_TRUE(HasSchema(*b3, "UnitTest"));
    ExpectNoForeignKeyViolations(*b3, "briefcase built from the timeline");
    ExpectECTablesIdentical(*b3, *b1, "briefcase built from the timeline");
    ExpectPhysicalSchemaIdentical(*b3, *b1, "briefcase built from the timeline");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, EnablingSchemaSyncOnABriefcaseWithSchemasAndData)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-init-populated");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, MachinerySchema("01.00.00", false)));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b1, "INSERT INTO mch.Machine(name) VALUES('pump')"));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("Machinery, and one machine");

    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "populated-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("enable schema sync");

    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_TRUE(HasSchema(sync, "Machinery")) << "the sync db was not seeded with the schemas the briefcase already had";
        EXPECT_FALSE(HasPhysicalTable(sync, "mch_Machine")) << "the sync db was seeded with a data table";
    });

    auto b2 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b2, TankSchema(), SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    ExpectNoForeignKeyViolations(*b2, "b2 after importing on top of a seeded sync db");
    b2->PullMergePush("add Tank");

    b1->PullMergePush("pick up Tank");

    EXPECT_STREQ("pump", ReadStringProperty(*b1, "SELECT name FROM mch.Machine").c_str())
        << "enabling schema sync lost data the briefcase already held";
    ExpectECTablesIdentical(*b1, *b2, "after schema sync was enabled on a briefcase that already had schemas");
    ExpectPhysicalSchemaIdentical(*b1, *b2, "after schema sync was enabled on a briefcase that already had schemas");
    }

// ---------------------------------------------------------------------------------------
// The entry points that seed the sync db attach it and detach it again, and SQLite refuses to
// detach a db while this connection holds a cursor open. The instance reader parks a prepared
// statement on the row it last read, so any caller that read an instance before enabling sync has
// one open. Init and OverwriteSyncDb clear the ECDb cache for that reason.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, EnablingSchemaSyncWhileTheInstanceReaderHoldsARow)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-init-reader-parked");

    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, MachinerySchema("01.00.00", false)));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b1, "INSERT INTO mch.Machine(name) VALUES('pump')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("Machinery, and one machine");

    const auto position = InstanceReader::Position(key.GetInstanceId(), key.GetClassId());
    ASSERT_TRUE(b1->GetInstanceReader().Seek(position, [](InstanceReader::IRowContext const& row, auto) {
        EXPECT_FALSE(row.GetJson().Stringify().empty());
    }));

    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "reader-parked-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("enable schema sync");

    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_TRUE(HasSchema(sync, "Machinery")) << "the sync db was not seeded with the schemas the briefcase already had";
    });
    EXPECT_STREQ("pump", ReadStringProperty(*b1, "SELECT name FROM mch.Machine").c_str())
        << "enabling schema sync lost data the briefcase already held";
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, OverwriteSyncDbFollowsAChangeMadeOnlyOnTheBriefcase)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-overwrite-entrypoint");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, RemapSchema("01.00.00", false), SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("RemapTest 1.0.0");
    b2->PullMergePush("pick up RemapTest 1.0.0");

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b1, "INSERT INTO rmp.LeafB(baseProp,movingProp) VALUES('b','before the move')"));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    // Somebody imports and walks away: state the sync db holds that no briefcase has.
    {
    auto abandoned = hub.CreateBriefcase();
    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*abandoned, UnrelatedSchema(), SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, abandoned->AbandonChanges());
    }
    syncDb.WithReadOnly([&](ECDbR sync) { ASSERT_TRUE(HasSchema(sync, "UnrelatedTest")); });

    auto& sync = b1->Schemas().GetSchemaSync();
    sync.DisableSchemaSync();
    const auto localResult = ImportSchema(*b1, RemapSchema("01.00.01", true), SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade);
    sync.ReEnableSchemaSync();
    ASSERT_EQ(SchemaImportResult::OK, localResult);
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    ASSERT_EQ(SchemaSync::Status::OK, sync.OverwriteSyncDb(syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    ExpectNoForeignKeyViolations(*b1, "b1 after overwriting the sync db");

    syncDb.WithReadOnly([&](ECDbR syncConn) {
        EXPECT_FALSE(HasSchema(syncConn, "UnrelatedTest")) << "the overwrite kept state no briefcase has";
        EXPECT_STREQ("1.0.1", VersionOf(syncConn, "RemapTest").c_str());
        ExpectECTablesIdentical(syncConn, *b1, "sync db rebuilt from a briefcase that upgraded locally");
    });

    EXPECT_STREQ("before the move", ReadStringProperty(*b1, "SELECT movingProp FROM rmp.LeafB").c_str())
        << "the local upgrade lost its own data";

    b1->PullMergePush("hoist movingProp");
    b2->PullMergePush("pick up the hoist");
    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b2, UnrelatedSchema(), SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    ExpectNoForeignKeyViolations(*b2, "b2 importing against the rebuilt sync db");
    }

//=======================================================================================
// Concurrent edits to the same ec_ row.
//
// The sync db resolves concurrent imports by the order it granted the container write lock, so the
// last briefcase to import wins there. Changesets reach the timeline in push order, which nothing
// ties to import order since an update takes only a shared lock. Both sides carry the sync db data
// version they were produced against, and the conflict handlers keep the later one, so the sync db's
// answer survives either push order. These tests run the same edits both ways round.
//=======================================================================================

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, ConcurrentLabelEditsInReversedPushOrder)
    {
    ConcurrentEditScenario scenario("upstream-label-edit-reversed");
    scenario.Start({ MetadataOnlySchema("01.00.00", "before anybody edited it") });

    scenario.ImportConcurrently(MetadataOnlySchema("01.00.01", "relabelled by the first importer"),
                                MetadataOnlySchema("01.00.02", "relabelled by the second importer"));
    scenario.Exchange(PushOrder::ReverseImportOrder);

    scenario.ExpectEverybodyHolds("concurrent relabel, pushed in reverse", "1.0.2", "Existing", "relabelled by the second importer");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, ConcurrentEditsToANewClassInReversedPushOrder)
    {
    ConcurrentEditScenario scenario("upstream-new-class-reversed");
    scenario.Start({ MetadataOnlySchema("01.00.00", "unchanged throughout") });

    scenario.ImportConcurrently(MetadataOnlySchema("01.00.01", "unchanged throughout", "labelled by the first importer"),
                                MetadataOnlySchema("01.00.02", "unchanged throughout", "labelled by the second importer"));
    scenario.Exchange(PushOrder::ReverseImportOrder);

    scenario.ExpectEverybodyHolds("concurrent edits to a new class, pushed in reverse", "1.0.2", "Added", "labelled by the second importer");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, ConcurrentLabelEditsInPushOrderMatchingImportOrder)
    {
    ConcurrentEditScenario scenario("upstream-label-edit-in-order");
    scenario.Start({ MetadataOnlySchema("01.00.00", "before anybody edited it") });

    scenario.ImportConcurrently(MetadataOnlySchema("01.00.01", "relabelled by the first importer"),
                                MetadataOnlySchema("01.00.02", "relabelled by the second importer"));
    scenario.Exchange(PushOrder::ImportOrder);

    scenario.ExpectEverybodyHolds("concurrent relabel, pushed in import order", "1.0.2", "Existing", "relabelled by the second importer");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, ConcurrentEditsToANewClassInPushOrderMatchingImportOrder)
    {
    ConcurrentEditScenario scenario("upstream-new-class-in-order");
    scenario.Start({ MetadataOnlySchema("01.00.00", "unchanged throughout") });

    scenario.ImportConcurrently(MetadataOnlySchema("01.00.01", "unchanged throughout", "labelled by the first importer"),
                                MetadataOnlySchema("01.00.02", "unchanged throughout", "labelled by the second importer"));
    scenario.Exchange(PushOrder::ImportOrder);

    scenario.ExpectEverybodyHolds("concurrent edits to a new class, pushed in import order", "1.0.2", "Added", "labelled by the second importer");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, UpgradeAfterAConcurrentEditDoesNotRollTheSyncDbBack)
    {
    ConcurrentEditScenario scenario("upstream-stale-overwrite");
    scenario.Start({ MetadataOnlySchema("01.00.00", "before anybody edited it"), RemapSchema("01.00.00", false) });

    scenario.ImportConcurrently(MetadataOnlySchema("01.00.01", "relabelled by the first importer"),
                                MetadataOnlySchema("01.00.02", "relabelled by the second importer"));
    // Import order is the direction in which the exchange has to overrule the first importer's own edit.
    scenario.Exchange(PushOrder::ImportOrder);

    auto& upgrader = *scenario.m_firstImporter;

    // Any upgrade will do; this one moves data, so it runs on the briefcase and then overwrites the sync db.
    ASSERT_EQ(SchemaImportResult::OK,
              ImportSchema(upgrader, RemapSchema("01.00.01", true), SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade, scenario.m_syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, upgrader.SaveChanges());

    scenario.m_syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_STREQ("1.0.2", VersionOf(sync, "LabelTest").c_str())
            << "the upgrade rolled the sync db back to " << VersionOf(sync, "LabelTest").c_str();
        EXPECT_STREQ("relabelled by the second importer", DisplayLabelOf(sync, "LabelTest", "Existing").c_str())
            << "the overwrite replaced the sync db's label with the one the upgrading briefcase held";
    });
    }

//=======================================================================================
// Data survival.
//
// The update path's whole claim is that it never moves or destroys data. Everything above this
// point checks metadata, mapping or DDL, so the claim itself was untested. These take an
// InstanceCensus before the change and compare it after.
//=======================================================================================

// A hierarchy with shared columns, so added properties land in the shared pool and eventually spill
// into overflow - the arrangement most likely to disturb data that is already there.
SchemaItem CensusSchema(Utf8CP version, bool withExtraProperties) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="CensusTest" alias="cen" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Asset">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="name" typeName="string" />
                %s
            </ECEntityClass>
            <ECEntityClass typeName="Pump">
                <BaseClass>Asset</BaseClass>
                <ECProperty propertyName="flowRate" typeName="double" />
                %s
            </ECEntityClass>
        </ECSchema>)xml",
        version,
        withExtraProperties ? "<ECProperty propertyName=\"owner\" typeName=\"string\" />" : "",
        withExtraProperties ? "<ECProperty propertyName=\"pressure\" typeName=\"double\" />"
                              "<ECProperty propertyName=\"serial\" typeName=\"string\" />"
                              "<ECProperty propertyName=\"spare\" typeName=\"int\" />" : "");
    return SchemaItem(xml);
}

// Two instances, one of each class, with every property of the initial version set.
void InsertCensusInstances(ECDbR db, Utf8CP nameSuffix) {
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO cen.Asset(name) VALUES(?)"));
    stmt.BindText(1, Utf8PrintfString("asset-%s", nameSuffix).c_str(), IECSqlBinder::MakeCopy::Yes);
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO cen.Pump(name,flowRate) VALUES(?,42.5)"));
    stmt.BindText(1, Utf8PrintfString("pump-%s", nameSuffix).c_str(), IECSqlBinder::MakeCopy::Yes);
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }
    ASSERT_EQ(BE_SQLITE_OK, db.SaveChanges());
}

SchemaItem AddedClassSchema(Utf8CP version, bool withAddedClass) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
<ECSchema schemaName="AddClassTest" alias="acl" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
<ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
<ECEntityClass typeName="Asset">
<ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
<ECProperty propertyName="name" typeName="string" />
</ECEntityClass>
%s
</ECSchema>)xml", version, withAddedClass ? R"xml(
<ECEntityClass typeName="Sensor">
<ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
<ECProperty propertyName="reading" typeName="double" />
</ECEntityClass>)xml" : "");
    return SchemaItem(xml);
}

SchemaItem SubclassPropertySchema(Utf8CP version, bool withAddedProperty) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
<ECSchema schemaName="SubclassPropertyTest" alias="spt" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
<ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
<ECEntityClass typeName="Asset">
<ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
<ECProperty propertyName="name" typeName="string" />
</ECEntityClass>
<ECEntityClass typeName="Pump">
<BaseClass>Asset</BaseClass>
<ECProperty propertyName="flowRate" typeName="double" />
%s
</ECEntityClass>
</ECSchema>)xml", version, withAddedProperty ? R"xml(<ECProperty propertyName="pressure" typeName="double" />)xml" : "");
    return SchemaItem(xml);
}

SchemaItem MixinSchema(Utf8CP version, bool withMixin) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
<ECSchema schemaName="MixinTest" alias="mxt" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
<ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
%s
<ECEntityClass typeName="Asset">
<ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
<ECProperty propertyName="name" typeName="string" />
</ECEntityClass>
<ECEntityClass typeName="Pump">
<BaseClass>Asset</BaseClass>
%s
<ECProperty propertyName="flowRate" typeName="double" />
</ECEntityClass>
</ECSchema>)xml", version,
withMixin ? R"xml(<ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
<ECEntityClass typeName="AuditMixin" modifier="Abstract">
<ECCustomAttributes>
<IsMixin xmlns="CoreCustomAttributes.01.00.00"><AppliesToEntityClass>Pump</AppliesToEntityClass></IsMixin>
</ECCustomAttributes>
<ECProperty propertyName="auditCode" typeName="string" />
</ECEntityClass>)xml" : "",
withMixin ? R"xml(<BaseClass>AuditMixin</BaseClass>)xml" : "");
    return SchemaItem(xml);
}

SchemaItem NavigationPropertySchema(Utf8CP version, bool withNavigationProperty) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
<ECSchema schemaName="NavigationPropertyTest" alias="nav" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
<ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
<ECEntityClass typeName="Parent">
<ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
<ECProperty propertyName="name" typeName="string" />
</ECEntityClass>
<ECEntityClass typeName="Child">
<ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
<ECProperty propertyName="name" typeName="string" />
%s
</ECEntityClass>
%s
</ECSchema>)xml", version,
withNavigationProperty ? R"xml(<ECNavigationProperty propertyName="Owner" relationshipName="ParentOwnsChild" direction="Backward" />)xml" : "",
withNavigationProperty ? R"xml(<ECRelationshipClass typeName="ParentOwnsChild" strength="embedding" modifier="None">
<Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true"><Class class="Parent" /></Source>
<Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true"><Class class="Child" /></Target>
</ECRelationshipClass>)xml" : "");
    return SchemaItem(xml);
}

SchemaItem KindOfQuantitySchema(Utf8CP version, bool useSecondKindOfQuantity) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
<ECSchema schemaName="KindOfQuantityTest" alias="koq" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
<UnitSystem typeName="TESTSYS" displayLabel="Test system" />
<Phenomenon typeName="TESTLENGTH" definition="TESTLENGTH" displayLabel="Test length" />
<Unit typeName="SMALL" definition="SMALL" phenomenon="TESTLENGTH" unitSystem="TESTSYS" />
<KindOfQuantity typeName="LengthOne" description="Length one" persistenceUnit="SMALL" relativeError="0.001" />
%s
<ECEntityClass typeName="Measured">
<ECProperty propertyName="length" typeName="double" kindOfQuantity="%s" />
<ECProperty propertyName="label" typeName="string" />
</ECEntityClass>
</ECSchema>)xml", version,
useSecondKindOfQuantity ? R"xml(<KindOfQuantity typeName="LengthTwo" description="Length two" persistenceUnit="SMALL" relativeError="0.002" />)xml" : "",
useSecondKindOfQuantity ? "LengthTwo" : "LengthOne");
    return SchemaItem(xml);
}

SchemaItem EnumerationSchema(Utf8CP version, bool withBlueEnumerator) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
<ECSchema schemaName="EnumerationTest" alias="enm" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
<ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
<ECEnumeration typeName="Colour" backingTypeName="int" isStrict="true">
<ECEnumerator name="Red" value="1" displayLabel="Red" />
<ECEnumerator name="Green" value="2" displayLabel="Green" />
%s
</ECEnumeration>
<ECEntityClass typeName="Paint">
<ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
<ECProperty propertyName="name" typeName="string" />
<ECProperty propertyName="colour" typeName="Colour" />
</ECEntityClass>
</ECSchema>)xml", version,
withBlueEnumerator ? R"xml(<ECEnumerator name="Blue" value="3" displayLabel="Blue" />)xml" : "");
    return SchemaItem(xml);
}

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, DataSurvivesPropertiesAddedThroughTheSyncDb)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-census-add");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    InsertCensusInstances(*b2, "b2");
    const auto before = InstanceCensus::Take(*b2);
    ASSERT_EQ(2u, before.GetInstanceCount()) << "the census did not see the rows the test just inserted";

    // Four new properties against a budget of four shared columns, so some of them spill to overflow.
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    ExpectCensusPreserved(before, InstanceCensus::Take(*b2), "after adding properties through the sync db");
    VerifyFileIsSound(*b2, "importer after adding properties");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, DataSurvivesOnABriefcaseThatOnlyPulls)
    {
    // The briefcase that did not import is the one at risk: it materialises its tables from the ec_
    // rows the changeset carried rather than from DDL anybody sent it.
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-census-puller");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add CensusTest 1.0.0");

    b1->PullMergePush("pick up CensusTest");
    InsertCensusInstances(*b1, "b1");
    // Both briefcases have to hold the rows before the schema changes. UpgradeECInstances runs while
    // a rebase has the local txns reversed, so rows still sitting in an unpushed txn never get their
    // overflow row.
    b1->PullMergePush("add instances");
    b2->PullMergePush("pick up the instances");
    const auto before = InstanceCensus::Take(*b1);
    ASSERT_EQ(2u, before.GetInstanceCount());

    // b2 imports; b1 only ever pulls.
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add properties to CensusTest");

    b1->PullMergePush("pick up the added properties");

    ExpectCensusPreserved(before, InstanceCensus::Take(*b1), "on the briefcase that only pulled");
    VerifyFileIsSound(*b1, "puller after the schema change");

    // The new properties have to be usable there, not merely harmless.
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b1, "SELECT owner,pressure,serial,spare FROM cen.Pump"))
        << "the pulling briefcase did not materialise the added properties";
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// One changeset carrying both a new class and rows of that class. A schema sync changeset ships
// ec_ rows and no DDL, so the pulling briefcase has no such table when the changeset arrives. The
// tables have to be built between the schema pass and the data pass of the apply, which is where
// TxnManager::ApplyChanges puts AfterSchemaChangeSetApplied. Get the order wrong and the rows are
// dropped without an error.
TEST_F(SchemaSyncImportTestFixture, DataInsertedInTheSameChangesetAsItsClassReachesAPullingBriefcase)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-census-same-changeset");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    // No push in between. MakeChangeset merges every local changeset into one, so the class and the
    // rows leave the briefcase together - which is what an app doing an import and a write in one
    // unit of work produces.
    InsertCensusInstances(*b2, "same-changeset");
    const auto before = InstanceCensus::Take(*b2);
    ASSERT_EQ(2u, before.GetInstanceCount());
    b2->PullMergePush("add CensusTest and its instances");

    b1->PullMergePush("pick up the class and its rows in one changeset");

    ExpectCensusPreserved(before, InstanceCensus::Take(*b1), "on the briefcase that pulled class and rows together");
    ExpectPhysicalSchemaIdentical(*b1, *b2, "puller vs importer after class and rows arrived together");
    VerifyFileIsSound(*b1, "puller after class and rows arrived together");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// b1 holds committed but unpushed rows when a schema change spills their class into the overflow
// table. The rebase reverses the local txn, so those rows are not in the file while the incoming
// changesets are applied. The catch-up therefore runs after the txn is reinstated, or the rows come
// back without an overflow row and cen.Pump reads as empty - ECDb inner-joins the overflow table.
TEST_F(SchemaSyncImportTestFixture, DataSurvivesASpillArrivingOnTopOfUnpushedRows)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-census-unpushed");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add CensusTest");
    b1->PullMergePush("pick up CensusTest");

    InsertCensusInstances(*b1, "unpushed");
    const auto before = InstanceCensus::Take(*b1);
    ASSERT_EQ(2u, before.GetInstanceCount());

    // b2 spills Pump into overflow while b1's rows are still sitting in an unpushed txn.
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add properties that spill to overflow");
    ASSERT_STRNE(TableOf(*b2, "CensusTest", "Pump", "flowRate").c_str(), TableOf(*b2, "CensusTest", "Pump", "spare").c_str())
        << "the added properties did not spill to overflow";

    b1->PullMergePush("rebase the unpushed rows onto the spilled schema");

    ExpectCensusPreserved(before, InstanceCensus::Take(*b1), "the briefcase whose unpushed rows were rebased");
    VerifyFileIsSound(*b1, "rebased briefcase after the spill");

    // The overflow rows have to be in what b1 pushed. A briefcase that only receives the rebased
    // changeset gets a pure data changeset and runs no catch-up of its own.
    b2->PullMergePush("pick up the rebased rows");
    ExpectCensusPreserved(before, InstanceCensus::Take(*b2), "the briefcase that received the rebased rows");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// The same spill with the rows spread over three unpushed txns, so the catch-up at the end of the
// rebase has to cover all of them.
TEST_F(SchemaSyncImportTestFixture, DataSurvivesASpillArrivingOnTopOfSeveralUnpushedTxns)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-census-unpushed-txns");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add CensusTest");
    b1->PullMergePush("pick up CensusTest");

    // InsertCensusInstances ends in SaveChanges, so each call is its own local txn.
    InsertCensusInstances(*b1, "first");
    InsertCensusInstances(*b1, "second");
    InsertCensusInstances(*b1, "third");
    const auto before = InstanceCensus::Take(*b1);
    ASSERT_EQ(6u, before.GetInstanceCount());

    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add properties that spill to overflow");

    b1->PullMergePush("rebase three unpushed txns onto the spilled schema");

    ExpectCensusPreserved(before, InstanceCensus::Take(*b1), "the briefcase whose three unpushed txns were rebased");
    VerifyFileIsSound(*b1, "rebased briefcase after the spill");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// ResetInstanceIdSequence runs from the same hook as UpgradeECInstances, so it also sees the file
// with the local txns reversed. It is safe where the overflow catch-up is not, and this pins why:
// the sequence lives in be_Local, which a changeset apply does not touch, and the reset takes
// max(sequence, max id in the tables) - so the high-water mark of the reversed rows survives.
// Asset carries no overflow property, which keeps this test independent of the spill.
TEST_F(SchemaSyncImportTestFixture, ARebaseDoesNotReuseTheInstanceIdsOfReinstatedRows)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-census-id-sequence");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add CensusTest");
    b1->PullMergePush("pick up CensusTest");

    ECInstanceKey unpushed;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b1, "INSERT INTO cen.Asset(name) VALUES('unpushed')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(unpushed));
    }
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add properties that spill to overflow");

    b1->PullMergePush("rebase the unpushed row onto the spilled schema");

    ECInstanceKey afterRebase;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b1, "INSERT INTO cen.Asset(name) VALUES('after-rebase')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(afterRebase));
    }
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    EXPECT_GT(afterRebase.GetInstanceId().GetValue(), unpushed.GetInstanceId().GetValue())
        << "the rebase handed out an instance id it had already used for a reinstated row";

    // Both rows have to be there, so the second insert did not overwrite the first.
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b1, "SELECT COUNT(*) FROM cen.Asset"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(2, stmt.GetValueInt(0));
    VerifyFileIsSound(*b1, "rebased briefcase after inserting again");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, DataSurvivesAClassAddedThroughTheSyncDb)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-census-class");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { AddedClassSchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add AddClassTest");
    b1->PullMergePush("pick up AddClassTest");

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO acl.Asset(name) VALUES('asset')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add AddClassTest");
    b1->PullMergePush("pick up AddClassTest");

    const auto beforeImporter = InstanceCensus::Take(*b2);
    const auto beforePuller = InstanceCensus::Take(*b1);
    ASSERT_EQ(1u, beforeImporter.GetInstanceCount());
    ASSERT_EQ(1u, beforePuller.GetInstanceCount());

    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { AddedClassSchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_TRUE(HasClass(*b2, "AddClassTest", "Sensor"));
    ExpectCensusPreserved(beforeImporter, InstanceCensus::Take(*b2), "importer after adding a class");

    b2->PullMergePush("add Sensor");
    b1->PullMergePush("pick up Sensor");
    EXPECT_TRUE(HasClass(*b1, "AddClassTest", "Sensor"));
    ExpectCensusPreserved(beforePuller, InstanceCensus::Take(*b1), "puller after adding a class");
    VerifyFileIsSound(*b2, "importer after adding a class");
    VerifyFileIsSound(*b1, "puller after adding a class");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, DataSurvivesAPropertyAddedToASubclassThroughTheSyncDb)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-census-subclass-property");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { SubclassPropertySchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add SubclassPropertyTest");
    b1->PullMergePush("pick up SubclassPropertyTest");

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO spt.Asset(name) VALUES('asset')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO spt.Pump(name,flowRate) VALUES('pump',42.5)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add SubclassPropertyTest");
    b1->PullMergePush("pick up SubclassPropertyTest");

    const auto beforeImporter = InstanceCensus::Take(*b2);
    const auto beforePuller = InstanceCensus::Take(*b1);
    ASSERT_EQ(2u, beforeImporter.GetInstanceCount());
    ASSERT_EQ(2u, beforePuller.GetInstanceCount());

    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { SubclassPropertySchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    ExpectCensusPreserved(beforeImporter, InstanceCensus::Take(*b2), "importer after adding a subclass property");

    b2->PullMergePush("add subclass property");
    b1->PullMergePush("pick up subclass property");
    ExpectCensusPreserved(beforePuller, InstanceCensus::Take(*b1), "puller after adding a subclass property");
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b1, "SELECT pressure FROM spt.Pump"));
    VerifyFileIsSound(*b2, "importer after adding a subclass property");
    VerifyFileIsSound(*b1, "puller after adding a subclass property");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, DataSurvivesAMixinAddedThroughTheSyncDb)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-census-mixin");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { MixinSchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add MixinTest");
    b1->PullMergePush("pick up MixinTest");

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO mxt.Pump(name,flowRate) VALUES('pump',42.5)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add MixinTest");
    b1->PullMergePush("pick up MixinTest");

    const auto beforeImporter = InstanceCensus::Take(*b2);
    const auto beforePuller = InstanceCensus::Take(*b1);
    ASSERT_EQ(1u, beforeImporter.GetInstanceCount());
    ASSERT_EQ(1u, beforePuller.GetInstanceCount());

    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { MixinSchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_TRUE(HasClass(*b2, "MixinTest", "AuditMixin"));
    ExpectCensusPreserved(beforeImporter, InstanceCensus::Take(*b2), "importer after adding a mixin");

    b2->PullMergePush("add AuditMixin");
    b1->PullMergePush("pick up AuditMixin");
    EXPECT_TRUE(HasClass(*b1, "MixinTest", "AuditMixin"));
    ExpectCensusPreserved(beforePuller, InstanceCensus::Take(*b1), "puller after adding a mixin");
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b1, "SELECT auditCode FROM mxt.Pump"));
    VerifyFileIsSound(*b2, "importer after adding a mixin");
    VerifyFileIsSound(*b1, "puller after adding a mixin");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, DataSurvivesANavigationPropertyAddedThroughTheSyncDb)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-census-navigation");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { NavigationPropertySchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add NavigationPropertyTest");
    b1->PullMergePush("pick up NavigationPropertyTest");

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO nav.Parent(name) VALUES('parent')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO nav.Child(name) VALUES('child')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add NavigationPropertyTest");
    b1->PullMergePush("pick up NavigationPropertyTest");

    const auto beforeImporter = InstanceCensus::Take(*b2);
    const auto beforePuller = InstanceCensus::Take(*b1);
    ASSERT_EQ(2u, beforeImporter.GetInstanceCount());
    ASSERT_EQ(2u, beforePuller.GetInstanceCount());

    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { NavigationPropertySchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_TRUE(HasClass(*b2, "NavigationPropertyTest", "ParentOwnsChild"));
    ExpectCensusPreserved(beforeImporter, InstanceCensus::Take(*b2), "importer after adding a navigation property");

    b2->PullMergePush("add navigation property");
    b1->PullMergePush("pick up navigation property");
    EXPECT_TRUE(HasClass(*b1, "NavigationPropertyTest", "ParentOwnsChild"));
    ExpectCensusPreserved(beforePuller, InstanceCensus::Take(*b1), "puller after adding a navigation property");
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b1, "SELECT Owner FROM nav.Child"));
    VerifyFileIsSound(*b2, "importer after adding a navigation property");
    VerifyFileIsSound(*b1, "puller after adding a navigation property");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, DataSurvivesAKindOfQuantityChangedOnAnExistingPropertyThroughTheSyncDb)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-census-koq");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { KindOfQuantitySchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add KindOfQuantityTest");
    b1->PullMergePush("pick up KindOfQuantityTest");

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO koq.Measured(length,label) VALUES(12.5,'measured')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add KindOfQuantityTest");
    b1->PullMergePush("pick up KindOfQuantityTest");

    const auto beforeImporter = InstanceCensus::Take(*b2);
    const auto beforePuller = InstanceCensus::Take(*b1);
    ASSERT_EQ(1u, beforeImporter.GetInstanceCount());
    ASSERT_EQ(1u, beforePuller.GetInstanceCount());

    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { KindOfQuantitySchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    ExpectCensusPreserved(beforeImporter, InstanceCensus::Take(*b2), "importer after changing a kind of quantity");

    b2->PullMergePush("change KindOfQuantity");
    b1->PullMergePush("pick up changed KindOfQuantity");
    ExpectCensusPreserved(beforePuller, InstanceCensus::Take(*b1), "puller after changing a kind of quantity");
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b1, "SELECT length FROM koq.Measured"));
    VerifyFileIsSound(*b2, "importer after changing a kind of quantity");
    VerifyFileIsSound(*b1, "puller after changing a kind of quantity");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, DataSurvivesAnEnumeratorAddedToAnExistingEnumerationThroughTheSyncDb)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-census-enumerator");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { EnumerationSchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add EnumerationTest");
    b1->PullMergePush("pick up EnumerationTest");

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO enm.Paint(name,colour) VALUES('paint',1)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add EnumerationTest");
    b1->PullMergePush("pick up EnumerationTest");

    const auto beforeImporter = InstanceCensus::Take(*b2);
    const auto beforePuller = InstanceCensus::Take(*b1);
    ASSERT_EQ(1u, beforeImporter.GetInstanceCount());
    ASSERT_EQ(1u, beforePuller.GetInstanceCount());

    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { EnumerationSchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    ExpectCensusPreserved(beforeImporter, InstanceCensus::Take(*b2), "importer after adding an enumerator");

    b2->PullMergePush("add Blue");
    b1->PullMergePush("pick up Blue");
    ExpectCensusPreserved(beforePuller, InstanceCensus::Take(*b1), "puller after adding an enumerator");
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b1, "SELECT colour FROM enm.Paint"));
    VerifyFileIsSound(*b2, "importer after adding an enumerator");
    VerifyFileIsSound(*b1, "puller after adding an enumerator");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// The importer holds the data here; DataSurvivesOnABriefcaseThatOnlyPulls covers the other side.
// The shared-column budget stays at 4 - ECDbMap custom attributes cannot be modified on an existing
// class, so the spill has to come from the added properties.
TEST_F(SchemaSyncImportTestFixture, DataSurvivesWhenAddedPropertiesSpillToOverflowThroughTheSyncDb)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-census-overflow");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add CensusTest");
    b1->PullMergePush("pick up CensusTest");

    InsertCensusInstances(*b2, "overflow");
    b2->PullMergePush("add instances");
    b1->PullMergePush("pick up the instances");

    const auto beforeImporter = InstanceCensus::Take(*b2);
    const auto beforePuller = InstanceCensus::Take(*b1);
    ASSERT_EQ(2u, beforeImporter.GetInstanceCount());
    ASSERT_EQ(2u, beforePuller.GetInstanceCount());

    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    const auto primaryTable = TableOf(*b2, "CensusTest", "Pump", "flowRate");
    const auto overflowTable = TableOf(*b2, "CensusTest", "Pump", "spare");
    ASSERT_FALSE(primaryTable.empty());
    ASSERT_FALSE(overflowTable.empty());
    EXPECT_STRNE(primaryTable.c_str(), overflowTable.c_str()) << "the added properties did not spill to overflow";
    ExpectCensusPreserved(beforeImporter, InstanceCensus::Take(*b2), "importer after the added properties spilled to overflow");

    b2->PullMergePush("add properties that spill to overflow");
    b1->PullMergePush("pick up the spilled properties");
    ExpectCensusPreserved(beforePuller, InstanceCensus::Take(*b1), "puller after the added properties spilled to overflow");
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b1, "SELECT owner,pressure,serial,spare FROM cen.Pump"));
    VerifyFileIsSound(*b2, "importer after the added properties spilled to overflow");
    VerifyFileIsSound(*b1, "puller after the added properties spilled to overflow");
    }

//=======================================================================================
// Deletions the update path refuses.
//
// Both report ERROR_DATA_DELETION_REQUIRED so a caller can route them to the upgrade path, the
// same way ERROR_DATA_TRANSFORM_REQUIRED routes a remap.
//=======================================================================================

// The same schema with Pump removed entirely.
SchemaItem CensusSchemaWithoutPump(Utf8CP version) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="CensusTest" alias="cen" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Asset">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="name" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml", version);
    return SchemaItem(xml);
}

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, DeletingAClassReportsDataDeletionRequired)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-delete-class");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    InsertCensusInstances(*b2, "b2");

    const auto before = InstanceCensus::Take(*b2);

    // Deleting Pump destroys its instances, which an update may not do.
    EXPECT_EQ(SchemaSync::Status::ERROR_DATA_DELETION_REQUIRED,
              sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchemaWithoutPump("02.00.00") }).Refs(), SchemaManager::SchemaImportOptions::None))
        << "a class delete has to be refused with a status the caller can route to the upgrade path";

    // A refused import changes nothing on either side.
    ExpectCensusPreserved(before, InstanceCensus::Take(*b2), "after the refused class delete");
    EXPECT_TRUE(HasClass(*b2, "CensusTest", "Pump")) << "the refused delete removed the class anyway";
    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_TRUE(HasClass(sync, "CensusTest", "Pump")) << "the refused delete was left behind in the sync db";
    });
    VerifyFileIsSound(*b2, "after the refused class delete");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, TheUpgradePathPerformsTheDeleteTheUpdatePathRefused)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-delete-upgrade");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    InsertCensusInstances(*b2, "b2");

    // Same delete, through the door the refusal points at.
    ASSERT_EQ(SchemaImportResult::OK,
              ImportSchema(*b2, CensusSchemaWithoutPump("02.00.00"), SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade, syncDb.GetSyncDbUri()))
        << "the upgrade path has to accept what the update path refused";
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    EXPECT_FALSE(HasClass(*b2, "CensusTest", "Pump")) << "the upgrade did not delete the class";
    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_FALSE(HasClass(sync, "CensusTest", "Pump")) << "the sync db was not overwritten with the deleted state";
    });

    // The Asset instance is not part of what was deleted and has to still be there.
    const auto after = InstanceCensus::Take(*b2);
    EXPECT_EQ(1u, after.GetInstanceCount()) << "the upgrade took more than the deleted class's instances";
    VerifyFileIsSound(*b2, "after the upgrade delete");
    }

// The same schema with Pump's flowRate removed. Pump is TPH with ShareColumns, so flowRate lives in
// a shared column and dropping it takes the second refusal site - SchemaWriter's UPDATE ... SET NULL.
SchemaItem CensusSchemaWithoutFlowRate(Utf8CP version) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="CensusTest" alias="cen" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Asset">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Pump">
                <BaseClass>Asset</BaseClass>
            </ECEntityClass>
        </ECSchema>)xml", version);
    return SchemaItem(xml);
}

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, DeletingASharedColumnPropertyReportsDataDeletionRequired)
    {
    // The other refusal site. A class delete destroys instances; this one only clears a column, so it
    // is the easier of the two to let through by accident.
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-delete-property");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    InsertCensusInstances(*b2, "b2");

    const auto before = InstanceCensus::Take(*b2);

    ScopedDisableFailOnAssertion disableFailOnAssertion;
    EXPECT_EQ(SchemaSync::Status::ERROR_DATA_DELETION_REQUIRED,
              sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchemaWithoutFlowRate("02.00.00") }).Refs(), SchemaManager::SchemaImportOptions::None))
        << "dropping a shared-column property clears the column, so it has to be refused too";

    ExpectCensusPreserved(before, InstanceCensus::Take(*b2), "after the refused property delete");
    VerifyFileIsSound(*b2, "after the refused property delete");
    }

//=======================================================================================
// A profile upgrade that alters an ec_ table.
//=======================================================================================

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, SyncDbFollowsAnECTableColumnAddedOnTheBriefcase)
    {
    // Stands in for a future ECDb profile upgrade that widens an ec_ table. The briefcase gets the
    // column from the profile upgrade; the sync db can only get it from the mirror step, which is
    // the half that has no other test.
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-profile-column");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    InsertCensusInstances(*b2, "b2");

    ASSERT_EQ(BE_SQLITE_OK, b2->ExecuteDdl("ALTER TABLE main.ec_Property ADD COLUMN SimulatedProfileColumn INTEGER"))
        << "could not simulate the profile upgrade";
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    ASSERT_EQ(SchemaSync::Status::OK, sync2.OverwriteSyncDb(syncDb.GetSyncDbUri()))
        << "the sync db has to be rebuilt from the briefcase after a profile upgrade";

    syncDb.WithReadOnly([&](ECDbR sync) {
        bvector<Utf8String> columns;
        sync.GetColumns(columns, "ec_Property");
        EXPECT_TRUE(std::find(columns.begin(), columns.end(), Utf8String("SimulatedProfileColumn")) != columns.end())
            << "the mirror step did not carry the new ec_Property column into the sync db";
    });

    // The pair still has to work afterwards, and the data still has to be there.
    const auto before = InstanceCensus::Take(*b2);
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None))
        << "an ordinary update stopped working after the simulated profile upgrade";
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    ExpectCensusPreserved(before, InstanceCensus::Take(*b2), "after importing across the simulated profile upgrade");
    VerifyFileIsSound(*b2, "briefcase after the simulated profile upgrade");
    syncDb.WithReadOnly([&](ECDbR sync) { VerifyFileIsSound(sync, "sync db after the simulated profile upgrade"); });
    }

//=======================================================================================
// The permutation matrix.
//
// Everything above tests one arrangement each. This walks combinations: N briefcases, each
// importing one of a few schema shapes, in every order, pushed in every order. Each round ends
// with the same three questions - do all N agree on ec_, do they agree on the physical schema,
// and did anybody lose data.
//
// This is what the extended tier is for. It is minutes, not seconds.
//=======================================================================================

// One briefcase's move in a round.
struct MatrixMove {
    Utf8CP m_label;
    SchemaItem m_schema;
};

// The class every round then widens, at a version below every move. Seeded on the timeline before a
// round's briefcases exist, so all of them start from it and can hold rows in it. Four shared columns
// and one property, so round N's fromA1..N / fromB1..N push it into overflow - which is what makes
// the rows worth counting: an instance that predates the spill needs an overflow row it never had.
SchemaItem MatrixBaselineSchema() {
    return SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="MatrixShared" alias="mxs" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Shared">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow></ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="base" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml");
}

// Put a schema on the timeline through the sync db, so briefcases created afterwards start from it.
void SeedThroughSyncDb(TrackedECDb& seed, SchemaSyncDb& syncDb, SchemaItem const& schema, Utf8CP context) {
    const auto loaded = LoadSchemas(seed, { schema });
    ASSERT_TRUE(loaded.IsValid()) << context << ": could not load the baseline schema";
    ASSERT_EQ(SchemaSync::Status::OK,
              seed.Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), loaded.Refs(), SchemaManager::SchemaImportOptions::None))
        << context << ": the baseline import was refused";
    ASSERT_EQ(BE_SQLITE_OK, seed.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, seed.PullMergePush(Utf8PrintfString("%s: baseline", context).c_str()))
        << context << ": could not push the baseline";
}

// One row of the baseline class. Left unpushed, so the round's concurrent imports also have to
// rebase it rather than merely merge alongside it.
void InsertMatrixRow(ECDbR db, Utf8CP name) {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO mxs.Shared(base) VALUES(?)"));
    stmt.BindText(1, name, IECSqlBinder::MakeCopy::Yes);
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_EQ(BE_SQLITE_OK, db.SaveChanges());
}

// The same, for the tests that widen SharedColumnSchema's Derived instead.
void InsertDerivedRow(ECDbR db, Utf8CP name) {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO ut.Derived(baseProp,p1) VALUES(?,1)"));
    stmt.BindText(1, name, IECSqlBinder::MakeCopy::Yes);
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_EQ(BE_SQLITE_OK, db.SaveChanges());
}

// The shapes a briefcase can bring to a round. Deliberately overlapping: two of them touch the same
// class, so the sync db has to serialise them onto different shared columns.
//
// Shapes are cumulative across rounds. A caller that reuses one sync db for several rounds leaves
// each round's properties in it, so round N has to carry rounds 1..N-1 as well - a version that
// dropped an earlier property would be a deletion, which the update path refuses by design.
std::vector<MatrixMove> MatrixMoves(int round) {
    Utf8String aProps, bProps;
    for (int i = 1; i <= round; ++i) {
        aProps.append(Utf8PrintfString("<ECProperty propertyName=\"fromA%d\" typeName=\"int\" />", i));
        bProps.append(Utf8PrintfString("<ECProperty propertyName=\"fromB%d\" typeName=\"double\" />", i));
    }

    Utf8String a, b, c;
    a.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="MatrixShared" alias="mxs" version="01.00.%02d" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Shared">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow></ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="base" typeName="string" />
                %s
            </ECEntityClass>
        </ECSchema>)xml", round, aProps.c_str());

    // Same class, one more property. B carries A's properties too, because both are versions of one
    // schema and B has the higher version, so either import order ends at the union.
    b.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="MatrixShared" alias="mxs" version="01.00.%02d" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Shared">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow></ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="base" typeName="string" />
                %s
                %s
            </ECEntityClass>
        </ECSchema>)xml", round + 50, aProps.c_str(), bProps.c_str());

    // An unrelated schema, so the round also covers two briefcases that do not collide at all.
    c.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="MatrixIsolated%d" alias="mxi%d" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Standalone">
                <ECProperty propertyName="value" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml", round, round);

    return { { "shared-A", SchemaItem(a) }, { "shared-B", SchemaItem(b) }, { "isolated", SchemaItem(c) } };
}

// Every briefcase ends the round holding the same metadata and the same physical layout, and
// nobody lost an instance.
void ExpectAllConverged(std::vector<TrackedECDb*> const& briefcases,
                        std::vector<SchemaSyncTestFixture::InstanceCensus> const& before, Utf8CP context) {
    ASSERT_FALSE(briefcases.empty());
    ASSERT_EQ(briefcases.size(), before.size());
    for (size_t i = 0; i < briefcases.size(); ++i) {
        const Utf8PrintfString where("%s: briefcase %d", context, (int)i);
        // A census taken before anything was inserted is preserved by every possible outcome,
        // including losing the file. Fail here rather than report a pass that means nothing.
        ASSERT_GT(before[i].GetInstanceCount(), 0u)
            << where.c_str() << ": the census was empty, so preserving it proves nothing";
        SchemaSyncTestFixture::VerifyFileIsSound(*briefcases[i], where.c_str());
        SchemaSyncTestFixture::ExpectCensusPreserved(before[i], SchemaSyncTestFixture::InstanceCensus::Take(*briefcases[i]), where.c_str());
        if (i == 0)
            continue;
        SchemaSyncTestFixture::ExpectECTablesIdentical(*briefcases[i], *briefcases[0], where.c_str());
        SchemaSyncTestFixture::ExpectPhysicalSchemaIdentical(*briefcases[i], *briefcases[0], where.c_str());
    }
}

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, ConcurrentImportsConvergeAcrossEveryOrdering)
    {
    // Three briefcases, three schema shapes, every assignment of shape to briefcase, and both push
    // orders. 3! x 2 = 12 rounds, each ending in a full convergence check.
    const int briefcaseCount = 3;
    std::vector<int> order{ 0, 1, 2 };
    int round = 0;

    do {
        for (int reversePush = 0; reversePush <= 1; ++reversePush) {
            ++round;
            const Utf8PrintfString context("round %d (assignment %d%d%d, push %s)", round,
                                           order[0], order[1], order[2], reversePush ? "reversed" : "in import order");

            ECDbHub hub;
            SchemaSyncDb syncDb(Utf8PrintfString("upstream-matrix-%d", round).c_str());
            std::unique_ptr<TrackedECDb> seed, unused;
            SetupSyncedPair(hub, syncDb, seed, unused);

            // The class the round then widens, on the timeline before the other briefcases exist.
            SeedThroughSyncDb(*seed, syncDb, MatrixBaselineSchema(), context.c_str());
            if (CurrentTestHasFailed())
                return;

            std::vector<std::unique_ptr<TrackedECDb>> briefcases;
            briefcases.push_back(std::move(seed));
            for (int i = 1; i < briefcaseCount; ++i)
                briefcases.push_back(hub.CreateBriefcase());

            // Something to lose, before anybody changes the schema. Unpushed, so the round's
            // changesets have to rebase over these rows as well as merge with each other.
            std::vector<InstanceCensus> before;
            for (size_t i = 0; i < briefcases.size(); ++i) {
                InsertMatrixRow(*briefcases[i], Utf8PrintfString("%s: bc%d", context.c_str(), (int)i).c_str());
                before.push_back(InstanceCensus::Take(*briefcases[i]));
            }
            if (CurrentTestHasFailed())
                return;

            const auto moves = MatrixMoves(round);
            for (int i = 0; i < briefcaseCount; ++i) {
                auto& bc = *briefcases[i];
                auto& sync = bc.Schemas().GetSchemaSync();
                const auto& move = moves[order[i]];
                const auto loaded = LoadSchemas(bc, { move.m_schema });
                ASSERT_TRUE(loaded.IsValid()) << context.c_str() << ": could not load " << move.m_label;

                // A briefcase importing a schema version the sync db already moved past is a no-op
                // there, which is a legal outcome rather than a failure.
                const auto rc = sync.ImportSchemas(syncDb.GetSyncDbUri(), loaded.Refs(), SchemaManager::SchemaImportOptions::None);
                ASSERT_EQ(SchemaSync::Status::OK, rc) << context.c_str() << ": " << move.m_label << " was refused";
                ASSERT_EQ(BE_SQLITE_OK, bc.SaveChanges());
            }

            for (int i = 0; i < briefcaseCount; ++i) {
                const int idx = reversePush ? briefcaseCount - 1 - i : i;
                ASSERT_EQ(BE_SQLITE_OK, briefcases[idx]->PullMergePush(Utf8PrintfString("%s: briefcase %d", context.c_str(), idx).c_str()))
                    << context.c_str() << ": briefcase " << idx << " could not push";
            }

            // Everyone catches up, then materialises what the changesets described.
            for (size_t i = 0; i < briefcases.size(); ++i) {
                ASSERT_EQ(BE_SQLITE_OK, briefcases[i]->PullMergePush("catch up"))
                    << context.c_str() << ": briefcase " << (int)i << " could not catch up";
            }

            std::vector<TrackedECDb*> raw;
            for (auto& bc : briefcases)
                raw.push_back(bc.get());
            ExpectAllConverged(raw, before, context.c_str());

            // Bundles the sync db's own invariants with per-briefcase row containment, and skips the
            // containment check for any briefcase not level with the sync db - which is where it
            // legitimately does not hold.
            VerifySchemaSyncRules(syncDb, std::vector<ECDb*>(raw.begin(), raw.end()), context.c_str());
            // One broken ordering is enough to look at; the rest would repeat it.
            if (CurrentTestHasFailed())
                return;
        }
    } while (std::next_permutation(order.begin(), order.end()));
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, ABriefcaseThatAbandonsItsWorkResynchronises)
    {
    // The sync db keeps what it decided during an import that was never pushed - only an upgrade
    // cleans that up. So a briefcase that abandons has to come back and agree with everyone else
    // anyway, across repeated rounds.
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-abandon-resync");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto b3 = hub.CreateBriefcase();

    for (int round = 1; round <= 4; ++round) {
        const Utf8PrintfString context("abandon round %d", round);
        const auto moves = MatrixMoves(round);

        // b2 imports and abandons without pushing.
        auto& sync2 = b2->Schemas().GetSchemaSync();
        ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { moves[0].m_schema }).Refs(), SchemaManager::SchemaImportOptions::None))
            << context.c_str();
        ASSERT_EQ(BE_SQLITE_OK, b2->AbandonChanges());

        // b1 then imports something else through the same sync db and pushes.
        auto& sync1 = b1->Schemas().GetSchemaSync();
        ASSERT_EQ(SchemaSync::Status::OK, sync1.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b1, { moves[2].m_schema }).Refs(), SchemaManager::SchemaImportOptions::None))
            << context.c_str();
        ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
        b1->PullMergePush(context.c_str());

        // b2 comes back and has to end up level with everyone, its abandoned work notwithstanding.
        // b2 may re-adopt the orphaned metadata from the sync db and push it, so b1 has to pull
        // afterwards - otherwise this compares a caught-up briefcase against a stale one.
        b2->PullMergePush("b2 rejoins");
        b3->PullMergePush("b3 catches up");
        b1->PullMergePush("b1 catches up");

        ExpectECTablesIdentical(*b2, *b1, context.c_str());
        ExpectECTablesIdentical(*b3, *b1, context.c_str());
        ExpectPhysicalSchemaIdentical(*b2, *b1, context.c_str());
        ExpectPhysicalSchemaIdentical(*b3, *b1, context.c_str());
        VerifyFileIsSound(*b2, context.c_str());
        }
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, ProfileUpgradeReachesTheSyncDbAndTheOtherBriefcase)
    {
    // A profile upgrade widens an ec_ table and bumps the EC profile version in be_Prop. Both have
    // to reach the sync db and every other briefcase. A profile upgrade holds the exclusive lock,
    // so nothing here races - the question is only whether the change propagates.
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-profile-upgrade");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add CensusTest");
    b1->PullMergePush("pick up CensusTest");

    InsertCensusInstances(*b2, "b2");
    b2->PullMergePush("insert census data");
    b1->PullMergePush("pick up census data");

    const auto beforeImporter = InstanceCensus::Take(*b2);
    const auto beforePuller = InstanceCensus::Take(*b1);
    ASSERT_EQ(2u, beforeImporter.GetInstanceCount());
    ASSERT_EQ(2u, beforePuller.GetInstanceCount());

    // The profile upgrade itself: a column on the end of an ec_ table, and a higher EC profile
    // version to go with it.
    const auto oldVersion = b2->GetECDbProfileVersion();
    const ProfileVersion upgradedVersion(oldVersion.GetMajor(), oldVersion.GetMinor(), oldVersion.GetSub1(), (uint16_t)(oldVersion.GetSub2() + 1));
    ASSERT_EQ(BE_SQLITE_OK, b2->ExecuteDdl("ALTER TABLE main.ec_Property ADD COLUMN SimulatedProfileColumn INTEGER"))
        << "could not widen ec_Property";
    ASSERT_EQ(BE_SQLITE_OK, b2->SavePropertyString(PropertySpec("SchemaVersion", "ec_Db"), upgradedVersion.ToJson()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    // The sync db can only learn the new table shape from the mirror step.
    ASSERT_EQ(SchemaSync::Status::OK, sync2.OverwriteSyncDb(syncDb.GetSyncDbUri()))
        << "the sync db has to be rebuilt from the briefcase after a profile upgrade";

    syncDb.WithReadOnly([&](ECDbR sync) {
        bvector<Utf8String> columns;
        sync.GetColumns(columns, "ec_Property");
        EXPECT_TRUE(std::find(columns.begin(), columns.end(), Utf8String("SimulatedProfileColumn")) != columns.end())
            << "the mirror step did not carry the new ec_Property column into the sync db";

        // SchemaSyncHelper::UpdateProfileVersion can push the briefcase's version into the sync db,
        // but its briefcase-to-sync-db direction has no caller, so this records what actually happens.
        EXPECT_EQ(upgradedVersion.GetSub2(), sync.GetECDbProfileVersion().GetSub2())
            << "the sync db kept the old EC profile version after the briefcase was upgraded";
    });

    // The other briefcase learns both from the changeset - the DDL rides along with it, and be_Prop
    // is an ordinary tracked table.
    b2->PullMergePush("push the profile upgrade");
    b1->PullMergePush("pick up the profile upgrade");

    {
    bvector<Utf8String> columns;
    b1->GetColumns(columns, "ec_Property");
    EXPECT_TRUE(std::find(columns.begin(), columns.end(), Utf8String("SimulatedProfileColumn")) != columns.end())
        << "the other briefcase never got the widened ec_Property";
    }
    EXPECT_EQ(upgradedVersion.GetSub2(), b1->GetECDbProfileVersion().GetSub2())
        << "the other briefcase got the widened ec_Property without the profile version that describes it";
    EXPECT_EQ(upgradedVersion.GetSub2(), b1->GetECDbProfileVersion().GetSub2())
        << "the other briefcase kept the old EC profile version";

    // Everything still has to work across the upgraded pair, and nobody may have lost data.
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None))
        << "an ordinary update stopped working after the profile upgrade";
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add properties after the profile upgrade");
    b1->PullMergePush("pick up properties after the profile upgrade");

    ExpectCensusPreserved(beforeImporter, InstanceCensus::Take(*b2), "importer across the profile upgrade");
    ExpectCensusPreserved(beforePuller, InstanceCensus::Take(*b1), "puller across the profile upgrade");
    ExpectECTablesIdentical(*b1, *b2, "after the profile upgrade");
    VerifyFileIsSound(*b2, "importer after the profile upgrade");
    VerifyFileIsSound(*b1, "puller after the profile upgrade");
    }

//=======================================================================================
// The redundancy tier.
//
// One variant of each test above, differing along an axis that could plausibly change the
// answer: a briefcase that only pulls, a class that spills to overflow, a rebase over unpushed
// rows, a joined table instead of shared columns, several rounds instead of one. The mechanisms
// here are indirect enough that a single arrangement per behaviour finds bugs by luck.
//
// Extended tier, so none of this runs on an ordinary build.
//=======================================================================================


// ---- variants of the tests above, group A ----

namespace {

SchemaItem BatchADataSchema(Utf8CP schemaName, Utf8CP alias) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="%s" alias="%s" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Record">
                <ECProperty propertyName="value" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml", schemaName, alias);
    return SchemaItem(xml);
}

SchemaItem BatchASharedColumnSchema(Utf8CP schemaName, Utf8CP alias, Utf8CP version, int propertyCount) {
    Utf8String properties;
    for (int i = 1; i <= propertyCount; ++i)
        properties.append(SqlPrintfString("<ECProperty propertyName=\"p%d\" typeName=\"int\" />\n", i).GetUtf8CP());

    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="%s" alias="%s" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
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
        </ECSchema>)xml", schemaName, alias, version, properties.c_str());
    return SchemaItem(xml);
}

SchemaItem BatchAJoinedSchema(Utf8CP schemaName, Utf8CP alias) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="%s" alias="%s" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Base" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00" />
                </ECCustomAttributes>
                <ECProperty propertyName="baseProp" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Sub1">
                <BaseClass>Base</BaseClass>
                <ECProperty propertyName="sub1Prop" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Sub2">
                <BaseClass>Base</BaseClass>
                <ECProperty propertyName="sub2Prop" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml", schemaName, alias);
    return SchemaItem(xml);
}

SchemaItem BatchARelationshipSchema(Utf8CP schemaName, Utf8CP alias) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="%s" alias="%s" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Parent">
                <ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
                <ECProperty propertyName="name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Child">
                <ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
                <ECProperty propertyName="name" typeName="string" />
                <ECNavigationProperty propertyName="Owner" relationshipName="ParentOwnsChild" direction="Backward">
                    <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.02.00.00"><OnDeleteAction>Cascade</OnDeleteAction></ForeignKeyConstraint>
                    </ECCustomAttributes>
                </ECNavigationProperty>
            </ECEntityClass>
            <ECEntityClass typeName="Tag">
                <ECProperty propertyName="name" typeName="string" />
            </ECEntityClass>
            <ECRelationshipClass typeName="ParentOwnsChild" strength="embedding" modifier="None">
                <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true"><Class class="Parent"/></Source>
                <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true"><Class class="Child"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="ChildHasTags" strength="referencing" modifier="None">
                <Source multiplicity="(0..*)" roleLabel="has" polymorphic="true"><Class class="Child"/></Source>
                <Target multiplicity="(0..*)" roleLabel="belongs to" polymorphic="true"><Class class="Tag"/></Target>
            </ECRelationshipClass>
        </ECSchema>)xml", schemaName, alias);
    return SchemaItem(xml);
}

SchemaItem BatchAStandaloneSchema(Utf8CP schemaName, Utf8CP alias, Utf8CP className) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="%s" alias="%s" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="%s">
                <ECProperty propertyName="value" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml", schemaName, alias, className);
    return SchemaItem(xml);
}

SchemaItem BatchAReferencingSchema(Utf8CP schemaName, Utf8CP alias, Utf8CP referencedSchemaName, Utf8CP referencedAlias,
                                   Utf8CP className, Utf8CP referencedClassName) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="%s" alias="%s" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="%s" version="01.00.00" alias="%s"/>
            <ECEntityClass typeName="%s">
                <BaseClass>%s:%s</BaseClass>
                <ECProperty propertyName="value" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml", schemaName, alias, referencedSchemaName, referencedAlias, className, referencedAlias, referencedClassName);
    return SchemaItem(xml);
}

SchemaItem BatchAMachinerySchema(Utf8CP schemaName, Utf8CP alias, Utf8CP version, int propertyCount) {
    Utf8String properties;
    for (int i = 1; i <= propertyCount; ++i)
        properties.append(SqlPrintfString("<ECProperty propertyName=\"p%d\" typeName=\"int\" />\n", i).GetUtf8CP());

    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="%s" alias="%s" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Machine">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow></ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="name" typeName="string" />
                %s
            </ECEntityClass>
        </ECSchema>)xml", schemaName, alias, version, properties.c_str());
    return SchemaItem(xml);
}

SchemaItem BatchATankSchema(Utf8CP schemaName, Utf8CP alias, Utf8CP referencedSchemaName, Utf8CP referencedAlias) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="%s" alias="%s" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="%s" version="01.00.00" alias="%s"/>
            <ECEntityClass typeName="Tank">
                <BaseClass>%s:Machine</BaseClass>
                <ECProperty propertyName="volume" typeName="double" />
            </ECEntityClass>
        </ECSchema>)xml", schemaName, alias, referencedSchemaName, referencedAlias, referencedAlias);
    return SchemaItem(xml);
}

} // namespace

// ---------------------------------------------------------------------------------------
// A data-only changeset still leaves the briefcase behind the timeline. Init checks the timeline
// position, so a missing row in an already-known class must be refused just like a missing schema.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, InitRefusesBriefcaseBehindTheTipOnADataOnlyChangeset)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-batch-a-init-data-gap");
    auto b1 = hub.CreateBriefcase();
    auto b2 = hub.CreateBriefcase();

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, BatchADataSchema("BatchAInitDataGapTest", "aidg")));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("add the shared data schema"));
    ASSERT_EQ(BE_SQLITE_OK, b2->PullMergePush("pick up the shared data schema"));

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO aidg.Record([value]) VALUES('only on b2')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, b2->PullMergePush("push a data-only changeset"));

    EXPECT_TRUE(HasSchema(*b1, "BatchAInitDataGapTest"));
    EXPECT_EQ(0, CountRows(*b1, "aidg_Record"));
    EXPECT_EQ(SchemaSync::Status::ERROR_BRIEFCASE_NOT_LEVEL_WITH_TIMELINE,
              b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "batch-a-container", false));
    EXPECT_FALSE(b1->Schemas().GetSchemaSync().IsEnabled()) << "a refused Init must leave the briefcase alone";

    ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("catch up the data-only changeset"));
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "batch-a-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_TRUE(HasSchema(sync, "BatchAInitDataGapTest"));
        SchemaSyncTestFixture::VerifySyncDbHoldsOnlyMetadata(sync, "after accepting the data-only gap");
    });
    }

// ---------------------------------------------------------------------------------------
// The local transaction is pure data, but Init still has to reject it because the sync db cannot
// describe a state that has not reached the timeline.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, InitRefusesBriefcaseHoldingUnpushedDataChanges)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-batch-a-init-unpushed-data");
    auto b1 = hub.CreateBriefcase();

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, BatchADataSchema("BatchAInitUnpushedDataTest", "aiud")));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("push the shared data schema"));

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b1, "INSERT INTO aiud.Record([value]) VALUES('still local')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    EXPECT_EQ(1, CountRows(*b1, "aiud_Record"));

    ASSERT_EQ(SchemaSync::Status::ERROR_BRIEFCASE_NOT_LEVEL_WITH_TIMELINE,
              b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "batch-a-container", false));
    EXPECT_FALSE(b1->Schemas().GetSchemaSync().IsEnabled());

    ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("push the local data"));
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "batch-a-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    }

// ---------------------------------------------------------------------------------------
// Init is seeded from a populated briefcase here. Profile metadata must mirror, while the existing
// row and its physical data table stay local to the briefcase.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, InitFromABriefcaseHoldingSchemasAndDataMirrorsMetadataOnly)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-batch-a-init-populated");
    auto b1 = hub.CreateBriefcase();

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, BatchADataSchema("BatchAInitPopulatedTest", "aipd")));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b1, "INSERT INTO aipd.Record([value]) VALUES('seeded row')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("push the schema and its row"));

    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "batch-a-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    const auto initialVersion = PropertySpec("InitialSchemaVersion", "ec_Db");
    const auto profileVersion = PropertySpec("SchemaVersion", "ec_Db");
    const auto localDbInfo = PropertySpec("localDbInfo", "ec_Db");
    const auto syncDbInfo = PropertySpec("syncDbInfo", "ec_Db");
    Utf8String briefcaseInitial, briefcaseProfile;
    ASSERT_EQ(BE_SQLITE_ROW, b1->QueryProperty(briefcaseInitial, initialVersion));
    ASSERT_EQ(BE_SQLITE_ROW, b1->QueryProperty(briefcaseProfile, profileVersion));

    syncDb.WithReadOnly([&](ECDbR sync) {
        Utf8String value;
        ASSERT_EQ(BE_SQLITE_ROW, sync.QueryProperty(value, initialVersion));
        EXPECT_STREQ(briefcaseInitial.c_str(), value.c_str());
        ASSERT_EQ(BE_SQLITE_ROW, sync.QueryProperty(value, profileVersion));
        EXPECT_STREQ(briefcaseProfile.c_str(), value.c_str());
        EXPECT_EQ(BE_SQLITE_ROW, sync.QueryProperty(value, syncDbInfo));
        EXPECT_NE(BE_SQLITE_ROW, sync.QueryProperty(value, localDbInfo));
        EXPECT_FALSE(sync.Schemas().GetSchemaSync().IsEnabled());
        EXPECT_FALSE(HasPhysicalTable(sync, "aipd_Record")) << "Init copied a data table into the sync db";
        SchemaSyncTestFixture::VerifySyncDbHoldsOnlyMetadata(sync, "Init from a populated briefcase");
    });
    }

// ---------------------------------------------------------------------------------------
// Joined-table and relationship mappings exercise child-table, link-table and navigation-property
// allocation, which can diverge from the shared-column case even when the hashes look similar.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, SyncDbMappingMatchesBriefcaseMappingForJoinedTablesAndRelationships)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-batch-a-joined-relationship-mapping");
    auto b1 = hub.CreateBriefcase();
    auto control = hub.CreateBriefcase();

    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "batch-a-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("init schema sync"));

    const std::vector<SchemaItem> schemas = {
        BatchAJoinedSchema("BatchAJoinedMappingTest", "bamj"),
        BatchARelationshipSchema("BatchARelationshipMappingTest", "bamr"),
    };
    ASSERT_EQ(SchemaImportResult::OK, SchemaSyncTestFixture::ImportSchemas(*control, schemas));
    ASSERT_EQ(BE_SQLITE_OK, control->SaveChanges());
    const auto briefcaseSchemaHash = SchemaSyncTestFixture::GetSchemaHash(*control);
    const auto briefcaseMapHash = SchemaSyncTestFixture::GetMapHash(*control);

    syncDb.WithReadWrite([&](ECDbR sync) {
        ASSERT_EQ(SchemaImportResult::OK, SchemaSyncTestFixture::ImportSchemas(sync, schemas,
            SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
        EXPECT_STREQ(briefcaseSchemaHash.c_str(), SchemaSyncTestFixture::GetSchemaHash(sync).c_str())
            << "logical schema rows differ between the sync db and the joined/relationship briefcase";
        EXPECT_STREQ(briefcaseMapHash.c_str(), SchemaSyncTestFixture::GetMapHash(sync).c_str())
            << "joined-table or relationship mapping was allocated differently in the sync db";
    });
    }

// ---------------------------------------------------------------------------------------
// The second import creates an overflow child table, so the comparison includes accumulated state
// across a table boundary rather than only the primary shared-column pool.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, MappingStillMatchesWhenTheSecondImportSpillsToOverflow)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-batch-a-mapping-overflow");
    auto b1 = hub.CreateBriefcase();
    auto control = hub.CreateBriefcase();

    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "batch-a-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("init schema sync"));

    const auto first = BatchASharedColumnSchema("BatchAOverflowMappingTest", "baom", "01.00.00", 2);
    const auto second = BatchASharedColumnSchema("BatchAOverflowMappingTest", "baom", "01.00.01", 8);
    ASSERT_EQ(SchemaImportResult::OK, SchemaSyncTestFixture::ImportSchema(*control, first));
    ASSERT_EQ(BE_SQLITE_OK, control->SaveChanges());
    ASSERT_EQ(SchemaImportResult::OK, SchemaSyncTestFixture::ImportSchema(*control, second));
    ASSERT_EQ(BE_SQLITE_OK, control->SaveChanges());
    const auto briefcaseSchemaHash = SchemaSyncTestFixture::GetSchemaHash(*control);
    const auto briefcaseMapHash = SchemaSyncTestFixture::GetMapHash(*control);

    syncDb.WithReadWrite([&](ECDbR sync) {
        ASSERT_EQ(SchemaImportResult::OK, SchemaSyncTestFixture::ImportSchema(sync, first,
            SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
        ASSERT_EQ(SchemaImportResult::OK, SchemaSyncTestFixture::ImportSchema(sync, second,
            SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
        EXPECT_STREQ(briefcaseSchemaHash.c_str(), SchemaSyncTestFixture::GetSchemaHash(sync).c_str());
        EXPECT_STREQ(briefcaseMapHash.c_str(), SchemaSyncTestFixture::GetMapHash(sync).c_str());
        EXPECT_STRNE(TableOf(sync, "BatchAOverflowMappingTest", "Derived", "p1").c_str(),
                     TableOf(sync, "BatchAOverflowMappingTest", "Derived", "p8").c_str())
            << "the second import did not leave an overflow table in the accumulated mapping";
    });
    }

// ---------------------------------------------------------------------------------------
// Joined tables and link-table relationships are physical shapes that a metadata-only import could
// accidentally materialise, so check both the broad metadata oracle and the named tables.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, ImportIntoSyncDbCreatesNoDataTablesForJoinedTablesAndLinkTables)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-batch-a-joined-link-tables");
    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "batch-a-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    const std::vector<SchemaItem> schemas = {
        BatchAJoinedSchema("BatchAPhysicalJoinedTest", "bapj"),
        BatchARelationshipSchema("BatchAPhysicalRelationshipTest", "bapr"),
    };
    syncDb.WithReadWrite([&](ECDbR sync) {
        ASSERT_EQ(SchemaImportResult::OK, SchemaSyncTestFixture::ImportSchemas(sync, schemas,
            SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
        SchemaSyncTestFixture::VerifySyncDbHoldsOnlyMetadata(sync, "joined and relationship metadata import");
        EXPECT_FALSE(HasPhysicalTable(sync, "bapj_Sub1")) << "the joined child table leaked into the sync db";
        EXPECT_FALSE(HasPhysicalTable(sync, "bapr_ChildHasTags")) << "the relationship link table leaked into the sync db";
    });
    }

// ---------------------------------------------------------------------------------------
// Three independent sync-db imports are adopted in one operation, exercising the accumulated
// closure and mapping state instead of the single-schema delta path.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, AdoptCarriesSeveralAccumulatedImportsInOneStep)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-batch-a-adopt-accumulated");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto first = BatchAStandaloneSchema("BatchAAdoptAccumulatedOne", "baao", "First");
    const auto second = BatchAStandaloneSchema("BatchAAdoptAccumulatedTwo", "baat", "Second");
    const auto third = BatchAStandaloneSchema("BatchAAdoptAccumulatedThree", "baath", "Third");
    syncDb.WithReadWrite([&](ECDbR sync) {
        ASSERT_EQ(SchemaImportResult::OK, SchemaSyncTestFixture::ImportSchema(sync, first,
            SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
        ASSERT_EQ(SchemaImportResult::OK, SchemaSyncTestFixture::ImportSchema(sync, second,
            SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
        ASSERT_EQ(SchemaImportResult::OK, SchemaSyncTestFixture::ImportSchema(sync, third,
            SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
    });

    ASSERT_EQ(SchemaSync::Status::OK, b2->Schemas().GetSchemaSync().AdoptSchemas(syncDb.GetSyncDbUri(),
        { "BatchAAdoptAccumulatedOne", "BatchAAdoptAccumulatedTwo", "BatchAAdoptAccumulatedThree" }));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_TRUE(HasClass(*b2, "BatchAAdoptAccumulatedOne", "First"));
    EXPECT_TRUE(HasClass(*b2, "BatchAAdoptAccumulatedTwo", "Second"));
    EXPECT_TRUE(HasClass(*b2, "BatchAAdoptAccumulatedThree", "Third"));
    SchemaSyncTestFixture::ExpectNoForeignKeyViolations(*b2, "after adopting several accumulated imports");
    syncDb.WithReadOnly([&](ECDbR sync) {
        SchemaSyncTestFixture::ExpectECTablesIdentical(*b2, sync, "after adopting several accumulated imports");
    });
    }

// ---------------------------------------------------------------------------------------
// The two sibling schemas share a referenced schema. Selecting one sibling must follow that edge
// without widening the selection to the other sibling.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, AdoptLeavesASiblingBehindWhenBothShareAReferencedSchema)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-batch-a-adopt-shared-reference");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto common = BatchAStandaloneSchema("BatchASharedReferenceTest", "basr", "CommonBase");
    const auto siblingOne = BatchAReferencingSchema("BatchASiblingOneTest", "bas1", "BatchASharedReferenceTest", "basr", "SiblingOne", "CommonBase");
    const auto siblingTwo = BatchAReferencingSchema("BatchASiblingTwoTest", "bas2", "BatchASharedReferenceTest", "basr", "SiblingTwo", "CommonBase");
    syncDb.WithReadWrite([&](ECDbR sync) {
        ASSERT_EQ(SchemaImportResult::OK, SchemaSyncTestFixture::ImportSchemas(sync, { common, siblingOne, siblingTwo },
            SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
    });

    ASSERT_EQ(SchemaSync::Status::OK, b2->Schemas().GetSchemaSync().AdoptSchemas(syncDb.GetSyncDbUri(), { "BatchASiblingOneTest" }));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_TRUE(HasSchema(*b2, "BatchASiblingOneTest"));
    EXPECT_TRUE(HasClass(*b2, "BatchASiblingOneTest", "SiblingOne"));
    EXPECT_TRUE(HasSchema(*b2, "BatchASharedReferenceTest"));
    EXPECT_TRUE(HasClass(*b2, "BatchASharedReferenceTest", "CommonBase"));
    EXPECT_FALSE(HasSchema(*b2, "BatchASiblingTwoTest"));
    EXPECT_FALSE(HasClass(*b2, "BatchASiblingTwoTest", "SiblingTwo"));
    SchemaSyncTestFixture::ExpectNoForeignKeyViolations(*b2, "after adopting one of two siblings");
    }

// ---------------------------------------------------------------------------------------
// A two-level reference chain makes the adopt closure walk through an already referenced schema
// before it reaches the root's base class.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, AdoptPullsATwoLevelReferenceChain)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-batch-a-adopt-two-level-chain");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto c = BatchAStandaloneSchema("BatchAChainCTest", "bachc", "CBase");
    const auto b = BatchAReferencingSchema("BatchAChainBTest", "bachb", "BatchAChainCTest", "bachc", "BBase", "CBase");
    const auto a = BatchAReferencingSchema("BatchAChainATest", "bacha", "BatchAChainBTest", "bachb", "ABase", "BBase");
    syncDb.WithReadWrite([&](ECDbR sync) {
        ASSERT_EQ(SchemaImportResult::OK, SchemaSyncTestFixture::ImportSchemas(sync, { c, b, a },
            SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
    });

    ASSERT_EQ(SchemaSync::Status::OK, b2->Schemas().GetSchemaSync().AdoptSchemas(syncDb.GetSyncDbUri(), { "BatchAChainATest" }));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_TRUE(HasClass(*b2, "BatchAChainATest", "ABase"));
    EXPECT_TRUE(HasClass(*b2, "BatchAChainBTest", "BBase"));
    EXPECT_TRUE(HasClass(*b2, "BatchAChainCTest", "CBase"));
    SchemaSyncTestFixture::ExpectNoForeignKeyViolations(*b2, "after adopting a two-level reference chain");
    syncDb.WithReadOnly([&](ECDbR sync) {
        SchemaSyncTestFixture::ExpectECTablesIdentical(*b2, sync, "after adopting a two-level reference chain");
    });
    }

// ---------------------------------------------------------------------------------------
// The adopted file must rebuild both foreign-key shapes before it accepts rows: a navigation
// property stored on the child and a link-table relationship with two independent ends.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, AnAdoptedSchemaAcceptsRelationshipInstances)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-batch-a-adopt-relationship-data");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto schema = BatchARelationshipSchema("BatchAAdoptRelationshipDataTest", "ba10");
    syncDb.WithReadWrite([&](ECDbR sync) {
        ASSERT_EQ(SchemaImportResult::OK, SchemaSyncTestFixture::ImportSchema(sync, schema,
            SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables));
        ASSERT_EQ(BE_SQLITE_OK, sync.SaveChanges());
    });
    ASSERT_EQ(SchemaSync::Status::OK, b2->Schemas().GetSchemaSync().AdoptSchemas(syncDb.GetSyncDbUri(), { "BatchAAdoptRelationshipDataTest" }));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    ECInstanceKey parentKey, childKey, tagKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO ba10.Parent(name) VALUES('parent')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(parentKey));
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO ba10.Child(name,Owner.Id) VALUES('child',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, parentKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(childKey));
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO ba10.Tag(name) VALUES('tag')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(tagKey));
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2,
        "INSERT INTO ba10.ChildHasTags(SourceECInstanceId,TargetECInstanceId) VALUES(?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, childKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, tagKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    EXPECT_EQ(1, CountRows(*b2, "ba10_ChildHasTags"));
    EXPECT_TRUE(DdlOf(*b2, "ba10_Child").ContainsI("FOREIGN KEY")) << "the adopted navigation foreign key was not rebuilt";
    EXPECT_TRUE(DdlOf(*b2, "ba10_ChildHasTags").ContainsI("FOREIGN KEY")) << "the adopted link-table foreign keys were not rebuilt";
    SchemaSyncTestFixture::ExpectNoForeignKeyViolations(*b2, "after inserting adopted relationship instances");
    SchemaSyncTestFixture::VerifyFileIsSound(*b2, "after inserting adopted relationship instances");
    }

// ---------------------------------------------------------------------------------------
// The competing allocation is in the overflow pool: the first import fills the primary budget and
// the dependent briefcase then receives a distinct overflow column for its own property.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, ConcurrentImportsDoNotShareAnOverflowColumn)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-batch-a-concurrent-overflow-column");
    auto b1 = hub.CreateBriefcase();
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "batch-a-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("init schema sync"));
    auto b2 = hub.CreateBriefcase();

    const auto machinery100 = BatchAMachinerySchema("BatchAOverflowRaceMachineryTest", "baor", "01.00.00", 0);
    const auto machinery101 = BatchAMachinerySchema("BatchAOverflowRaceMachineryTest", "baor", "01.00.01", 8);
    const auto tank = BatchATankSchema("BatchAOverflowRaceTankTest", "baot", "BatchAOverflowRaceMachineryTest", "baor");

    ImportThroughSyncDb(*b1, syncDb, { machinery100 }, { "BatchAOverflowRaceMachineryTest" });
    ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("add the base machinery schema"));
    ASSERT_EQ(BE_SQLITE_OK, b2->PullMergePush("pick up the base machinery schema"));
    ASSERT_TRUE(HasSchema(*b2, "BatchAOverflowRaceMachineryTest"));

    ImportThroughSyncDb(*b1, syncDb, { machinery101 }, { "BatchAOverflowRaceMachineryTest" });
    ImportThroughSyncDb(*b2, syncDb, { tank }, { "BatchAOverflowRaceTankTest" });

    const auto primaryTable = TableOf(*b2, "BatchAOverflowRaceMachineryTest", "Machine", "p1");
    const auto overflowTable = TableOf(*b2, "BatchAOverflowRaceMachineryTest", "Machine", "p8");
    const auto volumeTable = TableOf(*b2, "BatchAOverflowRaceTankTest", "Tank", "volume");
    const auto overflowColumn = ColumnOf(*b2, "BatchAOverflowRaceMachineryTest", "Machine", "p8");
    const auto volumeColumn = ColumnOf(*b2, "BatchAOverflowRaceTankTest", "Tank", "volume");
    ASSERT_FALSE(primaryTable.empty()) << "the primary shared table was not mapped";
    ASSERT_FALSE(overflowTable.empty()) << "the first importer did not spill into overflow";
    ASSERT_FALSE(volumeTable.empty()) << "the dependent property was not mapped";
    EXPECT_STRNE(primaryTable.c_str(), overflowTable.c_str());
    EXPECT_STRNE(primaryTable.c_str(), volumeTable.c_str()) << "the dependent property stayed in the primary table";
    EXPECT_STRNE(overflowColumn.c_str(), volumeColumn.c_str())
        << "the overflow column allocated to the two concurrent imports was shared";

    ECInstanceKey key;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO baot.Tank(name,p8,volume) VALUES('tank',8,99.5)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    ECSqlStatement select;
    ASSERT_EQ(ECSqlStatus::Success, select.Prepare(*b2, "SELECT p8,volume FROM baot.Tank WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, select.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, select.Step());
    EXPECT_EQ(8, select.GetValueInt(0));
    EXPECT_DOUBLE_EQ(99.5, select.GetValueDouble(1));
    SchemaSyncTestFixture::ExpectNoForeignKeyViolations(*b2, "after concurrent overflow allocation");
    SchemaSyncTestFixture::VerifyFileIsSound(*b2, "after concurrent overflow allocation");
    }

// ---------------------------------------------------------------------------------------
// A pull-only briefcase and a fresh briefcase from the whole timeline both have to reconstruct the
// same physical layout that the two importing briefcases reached by exchanging changesets.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, ConcurrentImportsConvergeAfterExchangeWithABriefcaseThatOnlyPulls)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-batch-a-converge-pull-only");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);
    auto b3 = hub.CreateBriefcase();

    const auto machinery100 = BatchAMachinerySchema("BatchAConvergeMachineryTest", "bacm", "01.00.00", 0);
    const auto machinery101 = BatchAMachinerySchema("BatchAConvergeMachineryTest", "bacm", "01.00.01", 1);
    const auto tank = BatchATankSchema("BatchAConvergeTankTest", "bact", "BatchAConvergeMachineryTest", "bacm");

    ImportThroughSyncDb(*b1, syncDb, { machinery100 }, { "BatchAConvergeMachineryTest" });
    ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("add the base convergence schema"));
    ASSERT_EQ(BE_SQLITE_OK, b2->PullMergePush("pick up the base convergence schema"));
    ASSERT_EQ(BE_SQLITE_OK, b3->PullMergePush("the pull-only briefcase picks up the base schema"));

    ImportThroughSyncDb(*b1, syncDb, { machinery101 }, { "BatchAConvergeMachineryTest" });
    ImportThroughSyncDb(*b2, syncDb, { tank }, { "BatchAConvergeTankTest" });

    ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("the first importer pushes its property"));
    ASSERT_EQ(BE_SQLITE_OK, b2->PullMergePush("the second importer merges and pushes its tank"));
    ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("the first importer picks up the tank"));
    ASSERT_EQ(BE_SQLITE_OK, b3->PullMergePush("the pull-only briefcase picks up both imports"));
    auto b4 = hub.CreateBriefcase();

    std::vector<TrackedECDb*> briefcases = { b1.get(), b2.get(), b3.get(), b4.get() };
    for (size_t i = 1; i < briefcases.size(); ++i) {
        const auto context = Utf8PrintfString("briefcase %d after convergence", (int)i);
        SchemaSyncTestFixture::ExpectECTablesIdentical(*briefcases[i], *b1, context.c_str());
        SchemaSyncTestFixture::ExpectPhysicalSchemaIdentical(*briefcases[i], *b1, context.c_str());
        SchemaSyncTestFixture::ExpectNoForeignKeyViolations(*briefcases[i], context.c_str());
        SchemaSyncTestFixture::VerifyFileIsSound(*briefcases[i], context.c_str());
    }
    syncDb.WithReadOnly([&](ECDbR sync) {
        SchemaSyncTestFixture::ExpectECTablesIdentical(*b1, sync, "converged briefcase vs sync db");
        SchemaSyncTestFixture::VerifySyncDbHoldsOnlyMetadata(sync, "converged sync db");
    });
    }


// ---- variants of the tests above, group B ----

namespace {

SchemaItem BatchBSharedColumnSchema(Utf8CP schemaName, Utf8CP alias, Utf8CP version, int propertyCount) {
    Utf8String properties;
    for (int i = 1; i <= propertyCount; ++i)
        properties.append(SqlPrintfString("<ECProperty propertyName=\"p%d\" typeName=\"int\" />\n", i).GetUtf8CP());

    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="%s" alias="%s" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow></ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="baseProp" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Derived">
                <BaseClass>Base</BaseClass>
                %s
            </ECEntityClass>
        </ECSchema>)xml", schemaName, alias, version, properties.c_str());
    return SchemaItem(xml);
}

SchemaItem BatchBReferencingSchema(Utf8CP schemaName, Utf8CP alias, Utf8CP version, Utf8CP referencedSchemaName, Utf8CP referencedAlias) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="%s" alias="%s" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="%s" version="01.00.00" alias="%s"/>
            <ECEntityClass typeName="Extra">
                <BaseClass>%s:Base</BaseClass>
                <ECProperty propertyName="extraProp" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml", schemaName, alias, version, referencedSchemaName, referencedAlias, referencedAlias);
    return SchemaItem(xml);
}

SchemaItem BatchBUnrelatedSchema() {
    return SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BatchBUnrelatedTest" alias="bbu" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Loner">
                <ECProperty propertyName="value" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml");
}

SchemaItem BatchBRemapSchema(Utf8CP schemaName, Utf8CP alias, Utf8CP version, bool hoisted) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="%s" alias="%s" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Base">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
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
        </ECSchema>)xml", schemaName, alias, version,
        hoisted ? R"xml(<ECProperty propertyName="movingProp" typeName="string" />)xml" : "");
    return SchemaItem(xml);
}

SchemaItem BatchBMachinerySchema(Utf8CP schemaName, Utf8CP alias, Utf8CP version, bool withRating) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="%s" alias="%s" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Machine">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>8</MaxSharedColumnsBeforeOverflow></ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="name" typeName="string" />
                %s
            </ECEntityClass>
        </ECSchema>)xml", schemaName, alias, version,
        withRating ? R"xml(<ECProperty propertyName="rating" typeName="int" />)xml" : "");
    return SchemaItem(xml);
}

SchemaItem BatchBTankSchema(Utf8CP schemaName, Utf8CP alias, Utf8CP version, Utf8CP baseSchemaName, Utf8CP baseAlias) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="%s" alias="%s" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="%s" version="01.00.00" alias="%s"/>
            <ECEntityClass typeName="Tank">
                <BaseClass>%s:Machine</BaseClass>
                <ECProperty propertyName="volume" typeName="double" />
            </ECEntityClass>
        </ECSchema>)xml", schemaName, alias, version, baseSchemaName, baseAlias, baseAlias);
    return SchemaItem(xml);
}

SchemaItem BatchBUnitsSchema(Utf8CP version, bool useBigUnit) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BatchBUnitsTest" alias="bunits" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="BATCHSYS" displayLabel="Batch B system" />
            <Phenomenon typeName="BATCHLENGTH" definition="BATCHLENGTH" displayLabel="Batch B length" />
            <Unit typeName="BATCHSMALL" definition="BATCHSMALL" phenomenon="BATCHLENGTH" unitSystem="BATCHSYS" />
            <Unit typeName="BATCHBIG" definition="BATCHSMALL" numerator="100.0" phenomenon="BATCHLENGTH" unitSystem="BATCHSYS" />
            <KindOfQuantity typeName="BatchLengthSmall" description="A small batch B length" persistenceUnit="BATCHSMALL" relativeError="0.001" />
            <KindOfQuantity typeName="BatchLengthBig" description="A big batch B length" persistenceUnit="BATCHBIG" relativeError="0.001" />
            <ECEntityClass typeName="Measured">
                <ECProperty propertyName="length" typeName="double" kindOfQuantity="%s" />
            </ECEntityClass>
        </ECSchema>)xml", version, useBigUnit ? "BatchLengthBig" : "BatchLengthSmall");
    return SchemaItem(xml);
}

SchemaItem BatchBRelationshipSchema() {
    return SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BatchBCleanupRelationship" alias="bcr" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Parent">
                <ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
                <ECProperty propertyName="name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Child">
                <ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
                <ECProperty propertyName="name" typeName="string" />
                <ECNavigationProperty propertyName="Owner" relationshipName="ParentOwnsChild" direction="Backward">
                    <ECCustomAttributes><ForeignKeyConstraint xmlns="ECDbMap.02.00.00"><OnDeleteAction>Cascade</OnDeleteAction></ForeignKeyConstraint></ECCustomAttributes>
                </ECNavigationProperty>
            </ECEntityClass>
            <ECEntityClass typeName="Tag">
                <ECProperty propertyName="name" typeName="string" />
            </ECEntityClass>
            <ECRelationshipClass typeName="ParentOwnsChild" strength="embedding" modifier="None">
                <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true"><Class class="Parent"/></Source>
                <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true"><Class class="Child"/></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="ChildHasTags" strength="referencing" modifier="None">
                <Source multiplicity="(0..*)" roleLabel="has" polymorphic="true"><Class class="Child"/></Source>
                <Target multiplicity="(0..*)" roleLabel="belongs to" polymorphic="true"><Class class="Tag"/></Target>
            </ECRelationshipClass>
        </ECSchema>)xml");
}

SchemaItem BatchBDeletionSchema(Utf8CP version, bool withFlowRate) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BatchBDeletionTest" alias="bdel" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECCustomAttributes><DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/></ECCustomAttributes>
            <ECEntityClass typeName="Asset">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow></ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Pump">
                <BaseClass>Asset</BaseClass>
                %s
            </ECEntityClass>
        </ECSchema>)xml", version,
        withFlowRate ? R"xml(<ECProperty propertyName="flowRate" typeName="double" />)xml" : "");
    return SchemaItem(xml);
}

} // namespace

// ---------------------------------------------------------------------------------------
// A KindOfQuantity change can move the stored value even though the primitive ECProperty type is
// unchanged. Its persistence unit is recorded in ec_KindOfQuantity, so the sync-db gate must reject
// the change through that table's path as well.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, SyncDbRefusesAChangedPersistenceUnit)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-persistence-unit");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto initial = BatchBUnitsSchema("01.00.00", false);
    const auto changed = BatchBUnitsSchema("01.00.01", true);
    ImportThroughSyncDb(*b1, syncDb, { initial }, { "BatchBUnitsTest" });

    ECSqlStatement insert;
    ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(*b1, "INSERT INTO bunits.Measured(length) VALUES(12.5)"));
    ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    syncDb.WithReadWrite([&](ECDbR sync) {
        EXPECT_EQ(SchemaImportResult::ERROR,
                  ImportSchema(sync, changed, SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables))
            << "changing a KindOfQuantity persistence unit should remain unsupported";
        ASSERT_EQ(BE_SQLITE_OK, sync.AbandonChanges());
        EXPECT_STREQ("1.0.0", VersionOf(sync, "BatchBUnitsTest").c_str());
    });
    }

// ---------------------------------------------------------------------------------------
// The target class is empty while another sibling has data. The changed shared-column mapping is
// enough to build a transform list, so the refusal cannot depend on finding a row in that class.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, TheTransformGateFiresOnAnEmptyClass)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-empty-transform");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto initial = BatchBRemapSchema("BatchBEmptyRemapTest", "ber", "01.00.00", false);
    const auto hoisted = BatchBRemapSchema("BatchBEmptyRemapTest", "ber", "01.00.01", true);
    ASSERT_EQ(SchemaSync::Status::OK,
              b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { initial }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    ECSqlStatement insert;
    ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(*b2, "INSERT INTO ber.LeafB(baseProp,movingProp) VALUES('only sibling', 'kept')"));
    ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    const auto before = InstanceCensus::Take(*b2);

    ECSqlStatement emptyClass;
    ASSERT_EQ(ECSqlStatus::Success, emptyClass.Prepare(*b2, "SELECT ECInstanceId FROM ber.LeafA"));
    EXPECT_EQ(BE_SQLITE_DONE, emptyClass.Step()) << "LeafA must remain empty for this gate test";

    syncDb.WithReadWrite([&](ECDbR sync) {
        EXPECT_EQ(SchemaImportResult::ERROR_DATA_TRANSFORM_REQUIRED,
                  ImportSchema(sync, hoisted, SchemaManager::SchemaImportOptions::DoNotCreateOrUpdateDataTables));
        ASSERT_EQ(BE_SQLITE_OK, sync.AbandonChanges());
        EXPECT_STREQ("1.0.0", VersionOf(sync, "BatchBEmptyRemapTest").c_str());
    });
    ExpectCensusPreserved(before, InstanceCensus::Take(*b2), "after refusing a transform for an empty class");
    }

// ---------------------------------------------------------------------------------------
// The second schema references the first in one ImportSchemas call. ReloadAgainstSyncDb has to
// resolve the sibling through its already re-pointed copy rather than treating it as an external load.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, ImportSchemasDoesBothStepsForSeveralSchemasInOneCall)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-several-schemas");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto base = BatchBSharedColumnSchema("BatchBDependencyBase", "bbase", "01.00.00", 2);
    const auto dependent = BatchBReferencingSchema("BatchBDependencyChild", "bchild", "01.00.00", "BatchBDependencyBase", "bbase");
    const auto loaded = LoadSchemas(*b2, { base, dependent });
    ASSERT_TRUE(loaded.IsValid()) << "could not load the sibling schemas";
    ASSERT_EQ(SchemaSync::Status::OK,
              b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), loaded.Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    EXPECT_TRUE(HasSchema(*b2, "BatchBDependencyBase"));
    EXPECT_TRUE(HasSchema(*b2, "BatchBDependencyChild"));
    EXPECT_TRUE(HasClass(*b2, "BatchBDependencyChild", "Extra"));
    syncDb.WithReadOnly([&](ECDbR sync) {
        ExpectECTablesIdentical(*b2, sync, "after importing dependent schemas in one call");
    });
    }

// ---------------------------------------------------------------------------------------
// The sync db starts from a briefcase that already has both tables and instances. The additive
// import must preserve that census while it mirrors the new schema rows.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, ImportSchemasWorksOnASyncDbInitialisedFromABriefcaseWithTablesAndData)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-nonempty-data-init");
    auto b1 = hub.CreateBriefcase();
    const auto seeded = BatchBSharedColumnSchema("BatchBSeededTest", "bseed", "01.00.00", 2);

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, seeded));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    ECSqlStatement insert;
    ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(*b1, "INSERT INTO bseed.Derived(baseProp,p1,p2) VALUES('seeded',11,22)"));
    ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    const auto before = InstanceCensus::Take(*b1);
    ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("schema and data before schema sync"));

    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "upstream-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("init schema sync from populated briefcase"));

    const auto added = LoadSchemas(*b1, { BatchBUnrelatedSchema() });
    ASSERT_TRUE(added.IsValid());
    ASSERT_EQ(SchemaSync::Status::OK,
              b1->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), added.Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    ExpectCensusPreserved(before, InstanceCensus::Take(*b1), "after importing through a populated sync db");
    EXPECT_TRUE(HasSchema(*b1, "BatchBSeededTest"));
    EXPECT_TRUE(HasSchema(*b1, "BatchBUnrelatedTest"));
    syncDb.WithReadOnly([&](ECDbR sync) {
        ExpectECTablesIdentical(*b1, sync, "populated briefcase versus the initialised sync db");
    });
    VerifyFileIsSound(*b1, "briefcase after importing through a populated sync db");
    }

// ---------------------------------------------------------------------------------------
// The briefcase resolves its dependent schema against an older local copy, while the sync db has
// already decided a newer base. The entry point must re-resolve the reference before mapping Tank.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, ImportSchemasResolvesAReferenceAgainstANewerCopyInTheSyncDb)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-newer-reference");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto base100 = BatchBMachinerySchema("BatchBReferenceAuthority", "bra", "01.00.00", false);
    const auto base101 = BatchBMachinerySchema("BatchBReferenceAuthority", "bra", "01.00.01", true);
    const auto dependent = BatchBTankSchema("BatchBReferenceDependent", "brd", "01.00.00", "BatchBReferenceAuthority", "bra");

    ImportThroughSyncDb(*b1, syncDb, { base100 }, { "BatchBReferenceAuthority" });
    b1->PullMergePush("publish the old referenced version");
    b2->PullMergePush("pick up the old referenced version");
    ASSERT_STREQ("1.0.0", VersionOf(*b2, "BatchBReferenceAuthority").c_str());

    ImportThroughSyncDb(*b1, syncDb, { base101 }, { "BatchBReferenceAuthority" });
    syncDb.WithReadOnly([&](ECDbR sync) {
        ASSERT_STREQ("1.0.1", VersionOf(sync, "BatchBReferenceAuthority").c_str());
    });

    const auto loaded = LoadSchemas(*b2, { dependent });
    ASSERT_TRUE(loaded.IsValid());
    ASSERT_EQ(SchemaSync::Status::OK,
              b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), loaded.Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    EXPECT_TRUE(HasSchema(*b2, "BatchBReferenceDependent"));
    EXPECT_STREQ("1.0.1", VersionOf(*b2, "BatchBReferenceAuthority").c_str());
    const auto ratingColumn = ColumnOf(*b2, "BatchBReferenceAuthority", "Machine", "rating");
    const auto volumeColumn = ColumnOf(*b2, "BatchBReferenceDependent", "Tank", "volume");
    EXPECT_FALSE(ratingColumn.empty());
    EXPECT_FALSE(volumeColumn.empty());
    EXPECT_STRNE(ratingColumn.c_str(), volumeColumn.c_str());
    syncDb.WithReadOnly([&](ECDbR sync) {
        ExpectECTablesIdentical(*b2, sync, "after resolving a reference against the newer sync-db copy");
    });
    }

// ---------------------------------------------------------------------------------------
// A refused transform must leave the sync db's metadata and data-version marker untouched. An
// ordinary additive import immediately afterwards proves that the refusal did not poison the pair.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, ARefusedTransformLeavesTheSyncDbUntouchedAndTheNextImportWorks)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-transform-rollback");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto initial = BatchBRemapSchema("BatchBTransformRollback", "btr", "01.00.00", false);
    const auto hoisted = BatchBRemapSchema("BatchBTransformRollback", "btr", "01.00.01", true);
    ASSERT_EQ(SchemaSync::Status::OK,
              b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { initial }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    Utf8String beforeSchemaHash, beforeMapHash;
    const auto beforeDataVersion = SchemaSync::SyncDbInfo::From(syncDb.GetSyncDbUri()).GetDataVersion();
    syncDb.WithReadOnly([&](ECDbR sync) {
        beforeSchemaHash = SchemaSyncTestFixture::GetSchemaHash(sync);
        beforeMapHash = SchemaSyncTestFixture::GetMapHash(sync);
        ExpectECTablesIdentical(*b2, sync, "sync db state before refusing the transform");
    });

    EXPECT_EQ(SchemaSync::Status::ERROR_DATA_TRANSFORM_REQUIRED,
              b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { hoisted }).Refs(), SchemaManager::SchemaImportOptions::None));
    EXPECT_STREQ("1.0.0", VersionOf(*b2, "BatchBTransformRollback").c_str());
    EXPECT_EQ(beforeDataVersion, SchemaSync::SyncDbInfo::From(syncDb.GetSyncDbUri()).GetDataVersion());
    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_STREQ(beforeSchemaHash.c_str(), SchemaSyncTestFixture::GetSchemaHash(sync).c_str());
        EXPECT_STREQ(beforeMapHash.c_str(), SchemaSyncTestFixture::GetMapHash(sync).c_str());
        ExpectECTablesIdentical(sync, *b2, "sync db after refusing the transform");
    });

    const auto added = LoadSchemas(*b2, { BatchBUnrelatedSchema() });
    ASSERT_TRUE(added.IsValid());
    ASSERT_EQ(SchemaSync::Status::OK,
              b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), added.Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_TRUE(HasSchema(*b2, "BatchBUnrelatedTest"));
    syncDb.WithReadOnly([&](ECDbR sync) {
        ExpectECTablesIdentical(*b2, sync, "after the next additive import");
    });
    }

// ---------------------------------------------------------------------------------------
// The importer creates the overflow row, while this briefcase only receives the changeset. Its
// catch-up therefore exercises ECDb::_AfterDataChangeSetApplied and materialises the row from ec_.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, TheOverflowCatchUpReachesABriefcaseThatOnlyPulls)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-overflow-puller");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto narrow = BatchBSharedColumnSchema("BatchBOverflowPuller", "bov", "01.00.00", 2);
    const auto wide = BatchBSharedColumnSchema("BatchBOverflowPuller", "bov", "01.00.01", 8);
    ASSERT_EQ(SchemaSync::Status::OK,
              b1->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b1, { narrow }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    ECInstanceKey key;
    ECSqlStatement insert;
    ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(*b1, "INSERT INTO bov.Derived(baseProp,p1) VALUES('before',1)"));
    ASSERT_EQ(BE_SQLITE_DONE, insert.Step(key));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("publish the narrow schema and its data");
    b2->PullMergePush("pull the narrow schema and its data");
    const auto beforePuller = InstanceCensus::Take(*b2);

    ASSERT_EQ(SchemaSync::Status::OK,
              b1->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b1, { wide }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    const auto overflowTable = TableOf(*b1, "BatchBOverflowPuller", "Derived", "p8");
    ASSERT_FALSE(overflowTable.empty());
    ASSERT_EQ(BE_SQLITE_OK, b1->PullMergePush("publish the overflow mapping"));
    ASSERT_EQ(BE_SQLITE_OK, b2->PullMergePush("catch up the overflow mapping"));

    ExpectCensusPreserved(beforePuller, InstanceCensus::Take(*b2), "puller after overflow catch-up");
    const auto pullerOverflowTable = TableOf(*b2, "BatchBOverflowPuller", "Derived", "p8");
    ASSERT_STREQ(overflowTable.c_str(), pullerOverflowTable.c_str());
    Statement overflowRow;
    ASSERT_EQ(BE_SQLITE_OK, overflowRow.Prepare(*b2, SqlPrintfString("SELECT COUNT(*) FROM main.[%s] WHERE Id=?", pullerOverflowTable.c_str()).GetUtf8CP()));
    overflowRow.BindId(1, key.GetInstanceId());
    ASSERT_EQ(BE_SQLITE_ROW, overflowRow.Step());
    EXPECT_EQ(1, overflowRow.GetValueInt(0));

    ECSqlStatement select;
    ASSERT_EQ(ECSqlStatus::Success, select.Prepare(*b2, "SELECT baseProp,p1 FROM bov.Derived WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, select.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, select.Step());
    EXPECT_STREQ("before", select.GetValueText(0));
    EXPECT_EQ(1, select.GetValueInt(1));
    ExpectECTablesIdentical(*b2, *b1, "overflow puller versus importer");
    ExpectPhysicalSchemaIdentical(*b2, *b1, "physical overflow schema after pull");
    VerifyFileIsSound(*b2, "briefcase that only pulled the overflow mapping");
    }

// ---------------------------------------------------------------------------------------
// The second importer references the first importer's schema before either briefcase has pushed.
// The sync db's reference closure, rather than the second briefcase's incomplete local context, must
// determine what both briefcases eventually hold.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, ConcurrentImportsOfSchemasThatReferenceEachOtherConverge)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-concurrent-reference");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);
    auto b3 = hub.CreateBriefcase();

    const auto base = BatchBSharedColumnSchema("BatchBConcurrentBase", "bcbase", "01.00.00", 2);
    const auto dependent = BatchBReferencingSchema("BatchBConcurrentDependent", "bcdep", "01.00.00", "BatchBConcurrentBase", "bcbase");
    ASSERT_EQ(SchemaSync::Status::OK,
              b1->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b1, { base }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    const auto dependentSchemas = LoadSchemas(*b2, { base, dependent });
    ASSERT_TRUE(dependentSchemas.IsValid()) << "the dependent schema could not be read with its unpushed reference in the read context";
    bvector<ECSchemaCP> dependentOnly { dependentSchemas.m_refs[1] };
    ASSERT_EQ(SchemaSync::Status::OK,
              b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), dependentOnly, SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    b1->PullMergePush("first concurrent import reaches the timeline");
    b2->PullMergePush("second concurrent import merges the referenced schema");
    b1->PullMergePush("first briefcase catches up with the dependent schema");
    b3->PullMergePush("bystander catches up with the reference closure");

    for (auto* briefcase : { b1.get(), b2.get(), b3.get() }) {
        EXPECT_TRUE(HasSchema(*briefcase, "BatchBConcurrentBase"));
        EXPECT_TRUE(HasSchema(*briefcase, "BatchBConcurrentDependent"));
        ExpectECTablesIdentical(*briefcase, *b2, "concurrent reference closure");
    }
    ExpectPhysicalSchemaIdentical(*b1, *b2, "physical schema after concurrent reference imports");
    ExpectPhysicalSchemaIdentical(*b3, *b2, "bystander physical schema after concurrent reference imports");
    syncDb.WithReadOnly([&](ECDbR sync) {
        ExpectECTablesIdentical(*b2, sync, "concurrent reference closure versus sync db");
    });
    }

// ---------------------------------------------------------------------------------------
// Import the newer schema first and then ask the sync db to process the older one. The surviving
// mapping must retain the newer property's row rather than treating the second call as a downgrade.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, ImportingTheOlderVersionSecondLeavesTheNewerInPlace)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-older-second");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto v101 = BatchBSharedColumnSchema("BatchBOlderSecond", "bos", "01.00.01", 1);
    const auto v102 = BatchBSharedColumnSchema("BatchBOlderSecond", "bos", "01.00.02", 2);

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, v102, SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    EXPECT_STREQ("1.0.2", VersionOf(*b1, "BatchBOlderSecond").c_str());
    syncDb.WithReadOnly([&](ECDbR sync) { EXPECT_STREQ("1.0.2", VersionOf(sync, "BatchBOlderSecond").c_str()); });

    ASSERT_EQ(SchemaImportResult::OK, ImportSchemas(*b2, { v101 }, SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_STREQ("1.0.2", VersionOf(*b2, "BatchBOlderSecond").c_str());
    EXPECT_FALSE(ColumnOf(*b2, "BatchBOlderSecond", "Derived", "p2").empty())
        << "the older second import removed the property from the newer sync-db schema";

    b1->PullMergePush("push the newer version");
    b2->PullMergePush("pull the newer version after the older import");
    auto b3 = hub.CreateBriefcase();
    EXPECT_STREQ("1.0.2", VersionOf(*b3, "BatchBOlderSecond").c_str());
    EXPECT_FALSE(ColumnOf(*b3, "BatchBOlderSecond", "Derived", "p2").empty());
    ExpectECTablesIdentical(*b2, *b1, "newer schema after older import");
    ExpectECTablesIdentical(*b3, *b1, "fresh briefcase after older import");
    }

// ---------------------------------------------------------------------------------------
// A bystander receives the changeset produced by the local upgrade. It has to reconstruct both the
// moved values and the consolidated physical layout without having run the upgrade itself.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, UpgradeSchemasMovesDataAndABystanderConvergesOnIt)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-upgrade-bystander");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);
    auto bystander = hub.CreateBriefcase();

    const auto initial = BatchBRemapSchema("BatchBUpgradeBystander", "bub", "01.00.00", false);
    const auto hoisted = BatchBRemapSchema("BatchBUpgradeBystander", "bub", "01.00.01", true);
    ASSERT_EQ(SchemaSync::Status::OK,
              b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { initial }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    ECSqlStatement insertA;
    ASSERT_EQ(ECSqlStatus::Success, insertA.Prepare(*b2, "INSERT INTO bub.LeafA(baseProp,filler,movingProp) VALUES('a','f','from A')"));
    ASSERT_EQ(BE_SQLITE_DONE, insertA.Step());
    ECSqlStatement insertB;
    ASSERT_EQ(ECSqlStatus::Success, insertB.Prepare(*b2, "INSERT INTO bub.LeafB(baseProp,movingProp) VALUES('b','from B')"));
    ASSERT_EQ(BE_SQLITE_DONE, insertB.Step());
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    const auto before = InstanceCensus::Take(*b2);
    b2->PullMergePush("publish the initial schema and data");
    bystander->PullMergePush("bystander picks up the initial schema and data");
    b1->PullMergePush("second existing briefcase picks up the initial schema and data");

    ASSERT_EQ(SchemaImportResult::OK,
              ImportSchema(*b2, hoisted, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    ExpectCensusPreserved(before, InstanceCensus::Take(*b2), "upgrader after the local remap");
    b2->PullMergePush("push the local upgrade");
    bystander->PullMergePush("bystander catches up with the local upgrade");
    b1->PullMergePush("second existing briefcase catches up with the local upgrade");

    EXPECT_STREQ("1.0.1", VersionOf(*bystander, "BatchBUpgradeBystander").c_str());
    EXPECT_STREQ("from A", ReadStringProperty(*bystander, "SELECT movingProp FROM bub.LeafA").c_str());
    EXPECT_STREQ("from B", ReadStringProperty(*bystander, "SELECT movingProp FROM bub.LeafB").c_str());
    ExpectECTablesIdentical(*bystander, *b2, "bystander after an upgrade changeset");
    ExpectPhysicalSchemaIdentical(*bystander, *b2, "bystander physical schema after an upgrade changeset");
    VerifySchemaSyncRules(syncDb, { b1.get(), b2.get(), bystander.get() }, "upgrade bystander convergence");
    }

// ---------------------------------------------------------------------------------------
// The abandoned object is a relationship-bearing schema. Overwrite pass 1 must cascade its schema,
// class, property, navigation and map rows before pass 2 restores the briefcase's requested state.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, UpgradeSchemasDropsAnAbandonedSchemaFromTheSyncDb)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-upgrade-schema-cleanup");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto initial = BatchBRemapSchema("BatchBCleanupRemap", "bcrm", "01.00.00", false);
    const auto hoisted = BatchBRemapSchema("BatchBCleanupRemap", "bcrm", "01.00.01", true);
    const auto abandoned = BatchBRelationshipSchema();
    ASSERT_EQ(SchemaSync::Status::OK,
              b1->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b1, { initial }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("publish the initial remap schema");
    b2->PullMergePush("pick up the initial remap schema");

    ASSERT_EQ(SchemaSync::Status::OK,
              b1->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b1, { abandoned }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    syncDb.WithReadOnly([&](ECDbR sync) {
        ASSERT_TRUE(HasSchema(sync, "BatchBCleanupRelationship"));
        EXPECT_TRUE(HasClass(sync, "BatchBCleanupRelationship", "Parent"));
    });
    EXPECT_FALSE(HasSchema(*b2, "BatchBCleanupRelationship"));

    ASSERT_EQ(SchemaSync::Status::OK,
              b2->Schemas().GetSchemaSync().UpgradeSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { hoisted }).Refs(), SchemaManager::SchemaImportOptions::None, nullptr));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_FALSE(HasSchema(sync, "BatchBCleanupRelationship"));
        EXPECT_FALSE(HasClass(sync, "BatchBCleanupRelationship", "Parent"));
        EXPECT_STREQ("1.0.1", VersionOf(sync, "BatchBCleanupRemap").c_str());
        ExpectECTablesIdentical(*b2, sync, "after dropping an abandoned whole schema");
    });

    const auto reloaded = LoadSchemas(*b2, { abandoned });
    ASSERT_TRUE(reloaded.IsValid());
    ASSERT_EQ(SchemaSync::Status::OK,
              b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), reloaded.Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_TRUE(HasSchema(*b2, "BatchBCleanupRelationship"));
    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_TRUE(HasClass(sync, "BatchBCleanupRelationship", "Child"));
        ExpectECTablesIdentical(*b2, sync, "after re-importing the reclaimed schema");
    });
    }

// ---------------------------------------------------------------------------------------
// The briefcase profile is independently ahead of the sync db. Both entry points must reject that
// skew before changing either file, which is the opposite direction from the core guard test.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, EntryPointsRefuseProfileVersionSkewInTheOtherDirection)
    {
    auto shiftProfileVersion = [](ECDbR db, int by) {
        const auto current = db.GetECDbProfileVersion();
        const ProfileVersion shifted(current.GetMajor(), current.GetMinor(), current.GetSub1(), (uint16_t)(current.GetSub2() + by));
        ASSERT_EQ(BE_SQLITE_OK, db.SavePropertyString(PropertySpec("SchemaVersion", "ec_Db"), shifted.ToJson()));
        ASSERT_EQ(BE_SQLITE_OK, db.SaveChanges());
    };

    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-profile-skew-import");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);
    const auto syncSub2 = syncDb.OpenReadOnly()->GetECDbProfileVersion().GetSub2();
    shiftProfileVersion(*b2, 1);
    const auto schemas = LoadSchemas(*b2, { BatchBSharedColumnSchema("BatchBProfileImport", "bpimp", "01.00.00", 2) });
    ASSERT_TRUE(schemas.IsValid());
    EXPECT_EQ(SchemaSync::Status::ERROR_PROFILE_VERSION_MISMATCH,
              b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), schemas.Refs(), SchemaManager::SchemaImportOptions::None));
    EXPECT_FALSE(HasSchema(*b2, "BatchBProfileImport"));
    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_EQ(syncSub2, sync.GetECDbProfileVersion().GetSub2());
        EXPECT_FALSE(HasSchema(sync, "BatchBProfileImport"));
    });
    }

    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-profile-skew-upgrade");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);
    const auto syncSub2 = syncDb.OpenReadOnly()->GetECDbProfileVersion().GetSub2();
    shiftProfileVersion(*b2, 1);
    const auto schemas = LoadSchemas(*b2, { BatchBSharedColumnSchema("BatchBProfileUpgrade", "bpup", "01.00.00", 2) });
    ASSERT_TRUE(schemas.IsValid());
    EXPECT_EQ(SchemaSync::Status::ERROR_PROFILE_VERSION_MISMATCH,
              b2->Schemas().GetSchemaSync().UpgradeSchemas(syncDb.GetSyncDbUri(), schemas.Refs(), SchemaManager::SchemaImportOptions::None, nullptr));
    EXPECT_FALSE(HasSchema(*b2, "BatchBProfileUpgrade"));
    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_EQ(syncSub2, sync.GetECDbProfileVersion().GetSub2());
        EXPECT_FALSE(HasSchema(sync, "BatchBProfileUpgrade"));
    });
    }
    }

// ---------------------------------------------------------------------------------------
// Removing a mapped property is routed through the deletion status rather than the transform status.
// The dynamic schema and raised read version make the same edit legal when the upgrade option is used.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, PlainImportRoutesByDeletionRatherThanTransform)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-deletion-routing");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto initial = BatchBDeletionSchema("01.00.00", true);
    const auto deletion = BatchBDeletionSchema("02.00.00", false);
    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b2, initial, SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_STREQ("1.0.0", VersionOf(*b2, "BatchBDeletionTest").c_str());

    {
    ScopedDisableFailOnAssertion disableFailOnAssertion;
    EXPECT_EQ(SchemaImportResult::ERROR_DATA_DELETION_REQUIRED,
              ImportSchema(*b2, deletion, SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    }
    EXPECT_STREQ("1.0.0", VersionOf(*b2, "BatchBDeletionTest").c_str());
    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_STREQ("1.0.0", VersionOf(sync, "BatchBDeletionTest").c_str());
    });

    ASSERT_EQ(SchemaImportResult::OK,
              ImportSchema(*b2, deletion, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_STREQ("2.0.0", VersionOf(*b2, "BatchBDeletionTest").c_str());
    EXPECT_TRUE(HasClass(*b2, "BatchBDeletionTest", "Pump"));
    EXPECT_TRUE(ColumnOf(*b2, "BatchBDeletionTest", "Pump", "flowRate").empty());
    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_STREQ("2.0.0", VersionOf(sync, "BatchBDeletionTest").c_str());
        EXPECT_TRUE(ColumnOf(sync, "BatchBDeletionTest", "Pump", "flowRate").empty());
        ExpectECTablesIdentical(*b2, sync, "after routing a deletion through the upgrade path");
    });
    }

// ---------------------------------------------------------------------------------------
// A fresh briefcase built from the complete timeline never sees the intermediate physical layout.
// It must nevertheless read the values after the remap and reproduce the consolidated schema.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, ARemapSurvivesIntoABriefcaseBuiltFromTheTimeline)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-remap-timeline");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    const auto initial = BatchBRemapSchema("BatchBRemapTimeline", "brt", "01.00.00", false);
    const auto hoisted = BatchBRemapSchema("BatchBRemapTimeline", "brt", "01.00.01", true);
    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, initial, SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("publish the initial remap schema");
    b2->PullMergePush("pick up the initial remap schema");

    for (auto* briefcase : { b1.get(), b2.get() }) {
        ECSqlStatement insert;
        ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(*briefcase, "INSERT INTO brt.LeafB(baseProp,movingProp) VALUES('b','before the move')"));
        ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
        ASSERT_EQ(BE_SQLITE_OK, briefcase->SaveChanges());
    }
    b2->PullMergePush("publish b2's pre-remap row");
    b1->PullMergePush("publish b1's pre-remap row");

    ASSERT_EQ(SchemaImportResult::OK,
              ImportSchema(*b1, hoisted, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("publish the remap");

    auto timelineBriefcase = hub.CreateBriefcase();
    b2->PullMergePush("pick up the remap");

    EXPECT_STREQ("1.0.1", VersionOf(*timelineBriefcase, "BatchBRemapTimeline").c_str());
    EXPECT_STREQ("before the move", ReadStringProperty(*timelineBriefcase, "SELECT movingProp FROM brt.LeafB").c_str());
    ExpectECTablesIdentical(*timelineBriefcase, *b1, "fresh briefcase built from the remap timeline");
    ExpectPhysicalSchemaIdentical(*timelineBriefcase, *b1, "fresh briefcase physical schema after the remap");
    EXPECT_STREQ("1.0.1", VersionOf(*b2, "BatchBRemapTimeline").c_str());
    EXPECT_STREQ("before the move", ReadStringProperty(*b2, "SELECT movingProp FROM brt.LeafB").c_str());
    VerifySchemaSyncRules(syncDb, { b1.get(), b2.get(), timelineBriefcase.get() }, "remap timeline convergence");
    }


// ---- variants of the tests above, group C ----

namespace {

SchemaItem BatchCRelationshipJoinedSchema() {
    return SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BatchCJoinedRelationship" alias="bcr" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
            <ECEntityClass typeName="Root" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00" />
                </ECCustomAttributes>
                <ECProperty propertyName="rootName" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Parent">
                <BaseClass>Root</BaseClass>
                <ECProperty propertyName="name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Child">
                <BaseClass>Root</BaseClass>
                <ECProperty propertyName="name" typeName="string" />
                <ECNavigationProperty propertyName="Owner" relationshipName="ParentOwnsChild" direction="Backward">
                    <ECCustomAttributes>
                        <ForeignKeyConstraint xmlns="ECDbMap.02.00.00"><OnDeleteAction>SetNull</OnDeleteAction></ForeignKeyConstraint>
                    </ECCustomAttributes>
                </ECNavigationProperty>
            </ECEntityClass>
            <ECEntityClass typeName="Tag">
                <ECProperty propertyName="name" typeName="string" />
            </ECEntityClass>
            <ECRelationshipClass typeName="ParentOwnsChild" strength="referencing" modifier="None">
                <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true"><Class class="Parent" /></Source>
                <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true"><Class class="Child" /></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="ChildHasTags" strength="referencing" modifier="None">
                <Source multiplicity="(0..*)" roleLabel="has" polymorphic="true"><Class class="Child" /></Source>
                <Target multiplicity="(0..*)" roleLabel="belongs to" polymorphic="true"><Class class="Tag" /></Target>
            </ECRelationshipClass>
        </ECSchema>)xml");
}

SchemaItem BatchCJoinedOverflowSchema() {
    Utf8String properties;
    for (int i = 1; i <= 8; ++i)
        properties.append(Utf8PrintfString("<ECProperty propertyName=\"p%d\" typeName=\"int\" />", i));

    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BatchCJoinedOverflow" alias="bco" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
            <ECEntityClass typeName="Base" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow>
                        <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                    <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00" />
                </ECCustomAttributes>
                <ECProperty propertyName="baseProp" typeName="int" />
            </ECEntityClass>
            <ECEntityClass typeName="Sub1">
                <BaseClass>Base</BaseClass>
                %s
            </ECEntityClass>
            <ECEntityClass typeName="Sub2">
                <BaseClass>Base</BaseClass>
                <ECProperty propertyName="sub2Prop" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml", properties.c_str());
    return SchemaItem(xml);
}

SchemaItem BatchCUnitDefinitionsSchema() {
    return SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BatchCUnitDefinitions" alias="bcu" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <UnitSystem typeName="BATCHSYS" displayLabel="Batch system" />
            <Phenomenon typeName="BATCHPHENOMENON" definition="BATCHPHENOMENON" displayLabel="Batch length" />
            <Unit typeName="BATCHSMALL" definition="BATCHSMALL" phenomenon="BATCHPHENOMENON" unitSystem="BATCHSYS" />
            <Unit typeName="BATCHBIG" definition="BATCHSMALL" numerator="100.0" phenomenon="BATCHPHENOMENON" unitSystem="BATCHSYS" />
            <Format typeName="BatchFormat" displayLabel="Batch format" type="Fractional" precision="4"
                    formatTraits="TrailZeroes|KeepSingleZero" decimalSeparator="." thousandSeparator="," uomSeparator=" ">
                <Composite spacer="-" includeZero="True">
                    <Unit label="b">BATCHBIG</Unit>
                    <Unit label="s">BATCHSMALL</Unit>
                </Composite>
            </Format>
            <KindOfQuantity typeName="BatchLength" description="A batch length" persistenceUnit="BATCHBIG" presentationUnits="BatchFormat" relativeError="0.001" />
            <ECEntityClass typeName="Measured">
                <ECProperty propertyName="length" typeName="double" kindOfQuantity="BatchLength" />
            </ECEntityClass>
        </ECSchema>)xml");
}

SchemaItem BatchCUnitConsumerSchema() {
    return SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BatchCUnitConsumer" alias="bcc" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="BatchCUnitDefinitions" version="01.00.00" alias="bcu" />
            <ECEntityClass typeName="Consumer">
                <ECProperty propertyName="length" typeName="double" kindOfQuantity="bcu:BatchLength" />
                <ECProperty propertyName="label" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml");
}

SchemaItem BatchCSiblingSlotSchema(Utf8CP version, int siblingCount) {
    Utf8String siblings;
    if (siblingCount >= 1)
        siblings.append(R"xml(
            <ECEntityClass typeName="First">
                <BaseClass>Root</BaseClass>
                <ECProperty propertyName="firstProp" typeName="int" />
            </ECEntityClass>)xml");
    if (siblingCount >= 2)
        siblings.append(R"xml(
            <ECEntityClass typeName="Second">
                <BaseClass>Root</BaseClass>
                <ECProperty propertyName="secondProp" typeName="double" />
            </ECEntityClass>)xml");
    if (siblingCount >= 3)
        siblings.append(R"xml(
            <ECEntityClass typeName="Third">
                <BaseClass>Root</BaseClass>
                <ECProperty propertyName="thirdProp" typeName="string" />
            </ECEntityClass>)xml");

    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BatchCSlots" alias="bcs" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
            <ECEntityClass typeName="Root" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>32</MaxSharedColumnsBeforeOverflow></ShareColumns>
                </ECCustomAttributes>
            </ECEntityClass>
            %s
        </ECSchema>)xml", version, siblings.c_str());
    return SchemaItem(xml);
}

SchemaItem BatchCTimeStampSchema(Utf8CP version) {
    const Utf8CP timestamp = R"xml(
                <ECCustomAttributes>
                    <ClassHasCurrentTimeStampProperty xmlns="CoreCustomAttributes.01.00.03">
                        <PropertyName>LastMod</PropertyName>
                    </ClassHasCurrentTimeStampProperty>
                </ECCustomAttributes>
                <ECProperty propertyName="LastMod" typeName="dateTime" readOnly="True" />)xml";

    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BatchCTimestamp" alias="bct" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA" />
            <ECEntityClass typeName="Stamped">
                <ECProperty propertyName="name" typeName="string" />
                %s
            </ECEntityClass>
        </ECSchema>)xml", version, timestamp);
    return SchemaItem(xml);
}

SchemaItem BatchCTimelineSchema(Utf8CP version, int propertyCount) {
    Utf8String properties;
    for (int i = 1; i <= propertyCount; ++i)
        properties.append(Utf8PrintfString("<ECProperty propertyName=\"p%d\" typeName=\"int\" />", i));

    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BatchCTimeline" alias="btl" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
            <ECEntityClass typeName="Asset">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow></ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="name" typeName="string" />
                %s
            </ECEntityClass>
        </ECSchema>)xml", version, properties.c_str());
    return SchemaItem(xml);
}

SchemaItem BatchCRelationshipOverflowSchema() {
    return SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BatchCRelOverflow" alias="bcrv" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />
            <ECEntityClass typeName="Parent">
                <ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
                <ECProperty propertyName="name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Child">
                <ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
                <ECProperty propertyName="name" typeName="string" />
                <ECNavigationProperty propertyName="Owner" relationshipName="ParentOwnsChild" direction="Backward">
                    <ECCustomAttributes><ForeignKeyConstraint xmlns="ECDbMap.02.00.00"><OnDeleteAction>Cascade</OnDeleteAction></ForeignKeyConstraint></ECCustomAttributes>
                </ECNavigationProperty>
            </ECEntityClass>
            <ECEntityClass typeName="Tag">
                <ECProperty propertyName="name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Wide">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow></ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="name" typeName="string" />
                <ECProperty propertyName="p1" typeName="int" />
                <ECProperty propertyName="p2" typeName="int" />
                <ECProperty propertyName="p3" typeName="int" />
                <ECProperty propertyName="p4" typeName="int" />
                <ECProperty propertyName="p5" typeName="int" />
                <ECProperty propertyName="p6" typeName="int" />
            </ECEntityClass>
            <ECRelationshipClass typeName="ParentOwnsChild" strength="embedding" modifier="None">
                <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="true"><Class class="Parent" /></Source>
                <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="true"><Class class="Child" /></Target>
            </ECRelationshipClass>
            <ECRelationshipClass typeName="ChildHasTags" strength="referencing" modifier="None">
                <Source multiplicity="(0..*)" roleLabel="has" polymorphic="true"><Class class="Child" /></Source>
                <Target multiplicity="(0..*)" roleLabel="belongs to" polymorphic="true"><Class class="Tag" /></Target>
            </ECRelationshipClass>
        </ECSchema>)xml");
}

SchemaItem BatchCStatementCacheSchema() {
    return SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BatchCStatementCache" alias="bsc" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Machine">
                <ECProperty propertyName="name" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml");
}

SchemaItem BatchCDynamicDeleteSchema(Utf8CP version, bool withDeletedClass, Utf8CP schemaName = "BatchCDelete", Utf8CP alias = "bcd") {
    Utf8String deletedClass;
    if (withDeletedClass)
        deletedClass = R"xml(
            <ECEntityClass typeName="ToDelete">
                <ECProperty propertyName="value" typeName="string" />
            </ECEntityClass>)xml";

    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="%s" alias="%s" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA" />
            <ECCustomAttributes><DynamicSchema xmlns="CoreCustomAttributes.01.00.03" /></ECCustomAttributes>
            <ECEntityClass typeName="Keep">
                <ECProperty propertyName="name" typeName="string" />
            </ECEntityClass>
            %s
        </ECSchema>)xml", schemaName, alias, version, deletedClass.c_str());
    return SchemaItem(xml);
}

SchemaItem BatchCLabelSchema(Utf8CP version, Utf8CP label, Utf8CP schemaName = "BatchCLabel", Utf8CP alias = "bcl") {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="%s" alias="%s" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Existing" displayLabel="%s">
                <ECProperty propertyName="value" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml", schemaName, alias, version, label);
    return SchemaItem(xml);
}

SchemaItem BatchCPropertySchema(Utf8CP version, bool withProperty, Utf8CP propertyLabel) {
    Utf8String property;
    if (withProperty)
        property.Sprintf(R"xml(<ECProperty propertyName="contested" displayLabel="%s" typeName="string" />)xml", propertyLabel);

    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BatchCProperty" alias="bcp" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Existing">
                <ECProperty propertyName="baseValue" typeName="string" />
                %s
            </ECEntityClass>
        </ECSchema>)xml", version, property.c_str());
    return SchemaItem(xml);
}

Utf8String BatchCPropertyDisplayLabelOf(ECDbR db, Utf8CP schemaName, Utf8CP className, Utf8CP propertyName) {
    Statement stmt;
    if (stmt.Prepare(db, R"sql(
        SELECT p.DisplayLabel FROM main.ec_Property p
        JOIN main.ec_Class c ON c.Id = p.ClassId
        JOIN main.ec_Schema s ON s.Id = c.SchemaId
        WHERE s.Name=? AND c.Name=? AND p.Name=?)sql") != BE_SQLITE_OK)
        return "<prepare failed>";
    stmt.BindText(1, schemaName, Statement::MakeCopy::No);
    stmt.BindText(2, className, Statement::MakeCopy::No);
    stmt.BindText(3, propertyName, Statement::MakeCopy::No);
    if (stmt.Step() != BE_SQLITE_ROW)
        return "<no such property>";
    Utf8CP label = stmt.GetValueText(0);
    return label == nullptr ? Utf8String("<null>") : Utf8String(label);
}

void BatchCInsertTimelineRow(TrackedECDb& db, Utf8CP name) {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO btl.Asset(name,p1) VALUES(?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, name, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(2, 1));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
}

} // namespace

// ---------------------------------------------------------------------------------------
// The relationship ends are direct subclasses in this variant, so the foreign-key pass must walk
// the joined hierarchy to find the physical end tables while the sync-db cache is being rebuilt.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, RelationshipsOnAJoinedTableHierarchyConvergeWithNoDeltaToReplay)
    {
    ExpectSchemaConvergesWithNoDeltaToReplay("upstream-batchc-joined-relationship", { BatchCRelationshipJoinedSchema() },
        "joined-table relationship constraints adopted with no DDL to replay",
        [](ECDbR adopted) {
            EXPECT_TRUE(HasPhysicalTable(adopted, "bcr_ChildHasTags")) << "the link table was never built";
            EXPECT_FALSE(ColumnOf(adopted, "BatchCJoinedRelationship", "Child", "Owner.Id").empty()) << "the joined child navigation property was not mapped";
            const auto childDdl = DdlOf(adopted, "bcr_Child");
            EXPECT_TRUE(childDdl.ContainsI("FOREIGN KEY")) << "the joined child navigation foreign key was not derived: " << childDdl.c_str();
            const auto linkDdl = DdlOf(adopted, "bcr_ChildHasTags");
            EXPECT_TRUE(linkDdl.ContainsI("FOREIGN KEY")) << "the link table foreign keys were not derived: " << linkDdl.c_str();
        });
    }

// ---------------------------------------------------------------------------------------
// Joined child and overflow child tables use different foreign-key clauses, so comparing only ec_
// rows would miss a layout that converged while retaining the wrong derived DDL.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, JoinedTablesWithSharedColumnsAndOverflowConvergeWithNoDeltaToReplay)
    {
    ExpectSchemaConvergesWithNoDeltaToReplay("upstream-batchc-joined-overflow", { BatchCJoinedOverflowSchema() },
        "joined-table shared columns and overflow adopted with no DDL to replay",
        [](ECDbR adopted) {
            const auto joinedTable = TableOf(adopted, "BatchCJoinedOverflow", "Sub1", "p1");
            const auto overflowTable = TableOf(adopted, "BatchCJoinedOverflow", "Sub1", "p8");
            EXPECT_FALSE(joinedTable.empty()) << "the joined child property was not mapped";
            EXPECT_FALSE(overflowTable.empty()) << "the overflow property was not mapped";
            EXPECT_STRNE(joinedTable.c_str(), overflowTable.c_str()) << "the joined child properties did not spill to a separate table";
            EXPECT_TRUE(HasPhysicalTable(adopted, joinedTable.c_str()));
            EXPECT_TRUE(HasPhysicalTable(adopted, overflowTable.c_str()));
            const auto joinedDdl = DdlOf(adopted, joinedTable.c_str());
            const auto overflowDdl = DdlOf(adopted, overflowTable.c_str());
            EXPECT_TRUE(joinedDdl.ContainsI("FOREIGN KEY")) << "the joined child foreign key was not derived: " << joinedDdl.c_str();
            EXPECT_TRUE(overflowDdl.ContainsI("FOREIGN KEY")) << "the overflow child foreign key was not derived: " << overflowDdl.c_str();
        });
    }

// ---------------------------------------------------------------------------------------
// The consumer arrives from a different briefcase and closes over a schema whose unit and format
// references use NO ACTION foreign keys, unlike the usual cascade-only metadata references.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, ASchemaReferencingAnotherSchemasUnitsAndFormatsIsAdopted)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-batchc-units-reference");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, BatchCUnitDefinitionsSchema(), SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("add BatchCUnitDefinitions");
    b2->PullMergePush("pick up BatchCUnitDefinitions");

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b2, BatchCUnitConsumerSchema(), SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_TRUE(HasSchema(*b2, "BatchCUnitDefinitions"));
    EXPECT_TRUE(HasSchema(*b2, "BatchCUnitConsumer"));
    EXPECT_FALSE(ColumnOf(*b2, "BatchCUnitConsumer", "Consumer", "length").empty()) << "the cross-schema KindOfQuantity property was not mapped";
    for (auto table : { "ec_UnitSystem", "ec_Phenomenon", "ec_Unit", "ec_Format", "ec_FormatCompositeUnit", "ec_KindOfQuantity" })
        EXPECT_GT(CountRows(*b2, table), 0) << table << " is empty after the referenced schema was adopted";
    EXPECT_TRUE(DdlOf(*b2, "ec_Unit").ContainsI("NO ACTION")) << "the unit foreign key lost its NO ACTION clause";
    EXPECT_TRUE(DdlOf(*b2, "ec_FormatCompositeUnit").ContainsI("NO ACTION")) << "the format-unit foreign key lost its NO ACTION clause";

    b2->PullMergePush("add BatchCUnitConsumer");
    b1->PullMergePush("pick up BatchCUnitConsumer");
    ExpectNoForeignKeyViolations(*b1, "b1 after the cross-schema units import");
    ExpectNoForeignKeyViolations(*b2, "b2 after the cross-schema units import");
    ExpectECTablesIdentical(*b1, *b2, "after a different briefcase adopted the units consumer");
    ExpectPhysicalSchemaIdentical(*b1, *b2, "after a different briefcase adopted the units consumer");
    }

// ---------------------------------------------------------------------------------------
// Three importers extend the same shared-column pool in sequence. Sibling classes may reuse one
// physical slot, so every briefcase must make the same reuse decision.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, ThreeBriefcasesAddingSiblingsAgreeOnEverySlot)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-batchc-three-sibling-slots");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);
    auto b3 = hub.CreateBriefcase();

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, BatchCSiblingSlotSchema("01.00.00", 1), SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("add First");
    b2->PullMergePush("b2 picks up First");
    b3->PullMergePush("b3 picks up First");

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b2, BatchCSiblingSlotSchema("01.00.01", 2), SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add Second");
    b1->PullMergePush("b1 picks up Second");
    b3->PullMergePush("b3 picks up Second");

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b3, BatchCSiblingSlotSchema("01.00.02", 3), SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b3->SaveChanges());
    b3->PullMergePush("add Third");
    b1->PullMergePush("b1 picks up Third");
    b2->PullMergePush("b2 picks up Third");

    for (auto* db : { b1.get(), b2.get(), b3.get() }) {
        const auto firstColumn = ColumnOf(*db, "BatchCSlots", "First", "firstProp");
        const auto secondColumn = ColumnOf(*db, "BatchCSlots", "Second", "secondProp");
        const auto thirdColumn = ColumnOf(*db, "BatchCSlots", "Third", "thirdProp");
        EXPECT_FALSE(firstColumn.empty()) << "First lost its shared-column slot";
        EXPECT_FALSE(secondColumn.empty()) << "Second lost its shared-column slot";
        EXPECT_FALSE(thirdColumn.empty()) << "Third lost its shared-column slot";
        EXPECT_STREQ(firstColumn.c_str(), secondColumn.c_str()) << "First and Second did not reuse the shared slot";
        EXPECT_STREQ(firstColumn.c_str(), thirdColumn.c_str()) << "First and Third did not reuse the shared slot";
    }
    ExpectECTablesIdentical(*b2, *b1, "after three siblings were assigned slots");
    ExpectECTablesIdentical(*b3, *b1, "after three siblings were assigned slots");
    ExpectPhysicalSchemaIdentical(*b2, *b1, "after three siblings were assigned slots");
    ExpectPhysicalSchemaIdentical(*b3, *b1, "after three siblings were assigned slots");
    }

// ---------------------------------------------------------------------------------------
// Four separate schema changesets are interleaved with data changesets. The late briefcase has to
// rebuild the final overflow layout and census every row while replaying the whole sequence.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, ABriefcaseFromTheTimelineConvergesAfterSeveralSchemaChangesets)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-batchc-several-timeline-changesets");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, BatchCTimelineSchema("01.00.00", 1), SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("timeline schema 1");
    b2->PullMergePush("pick up timeline schema 1");

    BatchCInsertTimelineRow(*b1, "row before schema 2");
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("timeline data 1");
    b2->PullMergePush("pick up timeline data 1");

    const auto beforeSchema2 = InstanceCensus::Take(*b2);
    ASSERT_EQ(SchemaSync::Status::OK, b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchCTimelineSchema("01.00.01", 2) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    ExpectCensusPreserved(beforeSchema2, InstanceCensus::Take(*b2), "importer after timeline schema 2");
    b2->PullMergePush("timeline schema 2");
    b1->PullMergePush("pick up timeline schema 2");

    BatchCInsertTimelineRow(*b2, "row before schema 3");
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("timeline data 2");
    b1->PullMergePush("pick up timeline data 2");

    const auto beforeSchema3 = InstanceCensus::Take(*b1);
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b1, { BatchCTimelineSchema("01.00.02", 3) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    ExpectCensusPreserved(beforeSchema3, InstanceCensus::Take(*b1), "importer after timeline schema 3");
    b1->PullMergePush("timeline schema 3");
    b2->PullMergePush("pick up timeline schema 3");

    BatchCInsertTimelineRow(*b1, "row before schema 4");
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("timeline data 3");
    b2->PullMergePush("pick up timeline data 3");

    const auto beforeSchema4 = InstanceCensus::Take(*b2);
    ASSERT_EQ(SchemaSync::Status::OK, b2->Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchCTimelineSchema("01.00.03", 8) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    ExpectCensusPreserved(beforeSchema4, InstanceCensus::Take(*b2), "importer after timeline schema 4");
    b2->PullMergePush("timeline schema 4 with overflow");
    b1->PullMergePush("pick up timeline schema 4");

    BatchCInsertTimelineRow(*b2, "row after schema 4");
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("timeline data 4");
    b1->PullMergePush("pick up timeline data 4");

    const auto expected = InstanceCensus::Take(*b1);
    const auto primaryTable = TableOf(*b1, "BatchCTimeline", "Asset", "p1");
    const auto overflowTable = TableOf(*b1, "BatchCTimeline", "Asset", "p8");
    EXPECT_FALSE(primaryTable.empty());
    EXPECT_FALSE(overflowTable.empty());
    EXPECT_STRNE(primaryTable.c_str(), overflowTable.c_str()) << "the final timeline schema did not create an overflow table";

    auto b3 = hub.CreateBriefcase();
    const auto actual = InstanceCensus::Take(*b3);
    EXPECT_EQ(expected.GetInstanceCount(), actual.GetInstanceCount()) << "the timeline briefcase has a different row count";
    ExpectCensusPreserved(expected, actual, "briefcase built after several schema changesets");
    ExpectECTablesIdentical(*b3, *b1, "briefcase built after several schema changesets");
    ExpectPhysicalSchemaIdentical(*b3, *b1, "briefcase built after several schema changesets");
    VerifyFileIsSound(*b3, "briefcase built after several schema changesets");
    }

// ---------------------------------------------------------------------------------------
// Init has to mirror existing relationship rows, navigation values, and overflow rows while leaving
// the sync db metadata-only. These shapes only exist in the physical briefcase before Init.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, EnablingSchemaSyncOnABriefcaseHoldingRelationshipsAndAnOverflowTable)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-batchc-enable-rel-overflow");
    auto b1 = hub.CreateBriefcase();

    const auto schema = BatchCRelationshipOverflowSchema();
    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, schema));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());

    ECInstanceKey parentKey, childKey, tagKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b1, "INSERT INTO bcrv.Parent(name) VALUES('parent')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(parentKey));
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b1, "INSERT INTO bcrv.Child(name,Owner.Id) VALUES('child',?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, parentKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(childKey));
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b1, "INSERT INTO bcrv.Tag(name) VALUES('tag')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(tagKey));
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b1, "INSERT INTO bcrv.ChildHasTags(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, childKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, childKey.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, tagKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, tagKey.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b1, "INSERT INTO bcrv.Wide(name,p1,p2,p3,p4,p5,p6) VALUES('wide',1,2,3,4,5,6)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    const auto before = InstanceCensus::Take(*b1);
    ASSERT_EQ(5u, before.GetInstanceCount()) << "the pre-Init relationship and overflow rows were not all readable";
    const auto overflowTable = TableOf(*b1, "BatchCRelOverflow", "Wide", "p6");
    ASSERT_FALSE(overflowTable.empty());
    ASSERT_STRNE(TableOf(*b1, "BatchCRelOverflow", "Wide", "p1").c_str(), overflowTable.c_str());
    ASSERT_GT(CountRows(*b1, "bcrv_ChildHasTags"), 0);

    b1->PullMergePush("publish the pre-schema-sync file");
    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "batchc-enable-rel-overflow-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    ExpectCensusPreserved(before, InstanceCensus::Take(*b1), "briefcase after enabling schema sync with relationship and overflow data");
    b1->PullMergePush("enable schema sync on relationship and overflow data");

    auto b2 = hub.CreateBriefcase();
    const auto afterB2 = InstanceCensus::Take(*b2);
    ExpectCensusPreserved(before, afterB2, "briefcase built after schema sync was enabled");
    EXPECT_TRUE(HasPhysicalTable(*b2, overflowTable.c_str()));
    EXPECT_GT(CountRows(*b2, "bcrv_ChildHasTags"), 0);
    EXPECT_FALSE(ColumnOf(*b2, "BatchCRelOverflow", "Child", "Owner.Id").empty());
    ExpectECTablesIdentical(*b1, *b2, "after enabling schema sync on relationship and overflow data");
    ExpectPhysicalSchemaIdentical(*b1, *b2, "after enabling schema sync on relationship and overflow data");
    VerifySchemaSyncRules(syncDb, { b1.get(), b2.get() }, "schema sync enabled on relationship and overflow data");
    }

// ---------------------------------------------------------------------------------------
// The instance reader variant leaves the sqlite statement cache holding a live cursor when Init
// tries to detach the sync db. Init must clear that cache before the detach.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, EnablingSchemaSyncWhileTheStatementCacheHoldsAPreparedStatement)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-batchc-statement-cache");
    auto b1 = hub.CreateBriefcase();

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, BatchCStatementCacheSchema()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    {
    ECSqlStatement insert;
    ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(*b1, "INSERT INTO bsc.Machine(name) VALUES('machine')"));
    ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    const auto before = InstanceCensus::Take(*b1);
    b1->PullMergePush("publish the statement-cache file");

    CachedStatementPtr cachedStatement = b1->GetCachedStatement("SELECT name FROM bsc_Machine");
    ASSERT_TRUE(cachedStatement.IsValid());
    ASSERT_EQ(BE_SQLITE_ROW, cachedStatement->Step());
    cachedStatement = nullptr;

    ASSERT_EQ(SchemaSync::Status::OK, b1->Schemas().GetSchemaSync().Init(syncDb.GetSyncDbUri(), "batchc-statement-cache-container", false));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    ExpectCensusPreserved(before, InstanceCensus::Take(*b1), "briefcase after enabling schema sync with a cached statement");
    b1->PullMergePush("enable schema sync with a cached statement");

    auto b2 = hub.CreateBriefcase();
    ExpectCensusPreserved(before, InstanceCensus::Take(*b2), "briefcase built after cached-statement schema sync was enabled");
    ExpectECTablesIdentical(*b1, *b2, "after enabling schema sync with a cached statement");
    ExpectPhysicalSchemaIdentical(*b1, *b2, "after enabling schema sync with a cached statement");
    VerifySchemaSyncRules(syncDb, { b1.get(), b2.get() }, "schema sync enabled with a cached statement");
    }

// ---------------------------------------------------------------------------------------
// The local upgrade removes an ec_ row, so OverwriteSyncDb must delete the old class from its target
// rather than only adding the briefcase's remaining rows. The Keep row proves unrelated data stays.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, OverwriteSyncDbFollowsARowRemovedOnlyOnTheBriefcase)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-batchc-overwrite-delete");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, BatchCDynamicDeleteSchema("01.00.00", true), SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("add the overwrite-delete schema");
    b2->PullMergePush("pick up the overwrite-delete schema");

    {
    ECSqlStatement insert;
    ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(*b1, "INSERT INTO bcd.Keep(name) VALUES('keep')"));
    ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    b1->PullMergePush("insert the row that must survive");
    b2->PullMergePush("pick up the surviving row");
    const auto before = InstanceCensus::Take(*b1);

    auto& sync = b1->Schemas().GetSchemaSync();
    sync.DisableSchemaSync();
    const auto localResult = ImportSchema(*b1, BatchCDynamicDeleteSchema("02.00.00", false), SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade);
    sync.ReEnableSchemaSync();
    ASSERT_EQ(SchemaImportResult::OK, localResult);
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    ASSERT_FALSE(HasClass(*b1, "BatchCDelete", "ToDelete"));
    ExpectCensusPreserved(before, InstanceCensus::Take(*b1), "briefcase after locally removing an ec_ row");

    ASSERT_EQ(SchemaSync::Status::OK, sync.OverwriteSyncDb(syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    syncDb.WithReadOnly([&](ECDbR syncConn) {
        EXPECT_FALSE(HasClass(syncConn, "BatchCDelete", "ToDelete")) << "the overwrite kept the removed ec_Class row";
        EXPECT_TRUE(HasClass(syncConn, "BatchCDelete", "Keep"));
        ExpectECTablesIdentical(syncConn, *b1, "sync db after mirroring the removed ec_ row");
    });

    b1->PullMergePush("push the overwrite-delete upgrade");
    b2->PullMergePush("pick up the overwrite-delete upgrade");
    EXPECT_FALSE(HasClass(*b2, "BatchCDelete", "ToDelete"));
    ExpectCensusPreserved(before, InstanceCensus::Take(*b2), "briefcase after receiving the overwrite-delete upgrade");
    ExpectECTablesIdentical(*b1, *b2, "after a locally removed ec_ row reached the other briefcase");
    ExpectPhysicalSchemaIdentical(*b1, *b2, "after a locally removed ec_ row reached the other briefcase");
    VerifyFileIsSound(*b1, "briefcase after overwriting the removed ec_ row");
    }

// ---------------------------------------------------------------------------------------
// Both importers carry committed but unpushed data while their schema edits are rebased in reverse
// push order. The data census checks the LocalChangeSet replay alongside schema precedence.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, ConcurrentLabelEditsWhileBothBriefcasesHoldUnpushedData)
    {
    ConcurrentEditScenario scenario("upstream-batchc-label-both-data-unpushed");
    scenario.Start({ BatchCLabelSchema("01.00.00", "before anybody edited it") });

    {
    ECSqlStatement firstInsert;
    ASSERT_EQ(ECSqlStatus::Success, firstInsert.Prepare(*scenario.m_firstImporter, "INSERT INTO bcl.Existing([value]) VALUES('first local row')"));
    ASSERT_EQ(BE_SQLITE_DONE, firstInsert.Step());
    ASSERT_EQ(BE_SQLITE_OK, scenario.m_firstImporter->SaveChanges());
    }
    {
    ECSqlStatement secondInsert;
    ASSERT_EQ(ECSqlStatus::Success, secondInsert.Prepare(*scenario.m_secondImporter, "INSERT INTO bcl.Existing([value]) VALUES('second local row')"));
    ASSERT_EQ(BE_SQLITE_DONE, secondInsert.Step());
    ASSERT_EQ(BE_SQLITE_OK, scenario.m_secondImporter->SaveChanges());
    }
    const auto beforeFirst = InstanceCensus::Take(*scenario.m_firstImporter);
    const auto beforeSecond = InstanceCensus::Take(*scenario.m_secondImporter);

    scenario.ImportConcurrently(BatchCLabelSchema("01.00.01", "relabelled by the first importer"),
                                 BatchCLabelSchema("01.00.02", "relabelled by the second importer"));
    scenario.Exchange(PushOrder::ReverseImportOrder);

    for (auto* db : { scenario.m_firstImporter.get(), scenario.m_secondImporter.get(), scenario.m_bystander.get() }) {
        EXPECT_STREQ("1.0.2", VersionOf(*db, "BatchCLabel").c_str());
        EXPECT_STREQ("relabelled by the second importer", DisplayLabelOf(*db, "BatchCLabel", "Existing").c_str());
        ExpectECTablesIdentical(*db, *scenario.m_firstImporter, "concurrent relabel with both briefcases holding unpushed data");
    }
    scenario.m_syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_STREQ("1.0.2", VersionOf(sync, "BatchCLabel").c_str());
        EXPECT_STREQ("relabelled by the second importer", DisplayLabelOf(sync, "BatchCLabel", "Existing").c_str());
    });
    ExpectCensusPreserved(beforeFirst, InstanceCensus::Take(*scenario.m_firstImporter), "first importer after rebasing its unpushed data");
    ExpectCensusPreserved(beforeSecond, InstanceCensus::Take(*scenario.m_secondImporter), "second importer after rebasing its unpushed data");
    ExpectPhysicalSchemaIdentical(*scenario.m_secondImporter, *scenario.m_firstImporter, "concurrent relabel with both briefcases holding unpushed data");
    }

// ---------------------------------------------------------------------------------------
// The conflict is an inserted property row under an existing ec_Class parent. Reversed replay must
// retain the winning property row instead of replacing the parent and cascading that child away.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, ConcurrentEditsToANewPropertyOnAnExistingClassInReversedPushOrder)
    {
    ConcurrentEditScenario scenario("upstream-batchc-property-reversed");
    scenario.Start({ BatchCPropertySchema("01.00.00", false, "unused") });

    scenario.ImportConcurrently(BatchCPropertySchema("01.00.01", true, "first property label"),
                                 BatchCPropertySchema("01.00.02", true, "second property label"));
    scenario.Exchange(PushOrder::ReverseImportOrder);

    for (auto* db : { scenario.m_firstImporter.get(), scenario.m_secondImporter.get(), scenario.m_bystander.get() }) {
        EXPECT_STREQ("1.0.2", VersionOf(*db, "BatchCProperty").c_str());
        EXPECT_STREQ("second property label", BatchCPropertyDisplayLabelOf(*db, "BatchCProperty", "Existing", "contested").c_str());
        ExpectECTablesIdentical(*db, *scenario.m_firstImporter, "concurrent property insert under an existing class");
        ExpectPhysicalSchemaIdentical(*db, *scenario.m_firstImporter, "concurrent property insert under an existing class");
    }
    }

// ---------------------------------------------------------------------------------------
// A concurrent label edit advances one schema while the later upgrade deletes a class from another.
// The overwrite must remove the deleted rows without restoring the stale label schema.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, AnUpgradeThatDeletesAfterAConcurrentEditDoesNotRollTheSyncDbBack)
    {
    ConcurrentEditScenario scenario("upstream-batchc-upgrade-delete-after-edit");
    scenario.Start({ BatchCLabelSchema("01.00.00", "before anybody edited it", "BatchCUpgradeLabel", "bcul"), BatchCDynamicDeleteSchema("01.00.00", true, "BatchCUpgradeDelete", "bcud") });

    {
    ECSqlStatement insert;
    ASSERT_EQ(ECSqlStatus::Success, insert.Prepare(*scenario.m_firstImporter, "INSERT INTO bcud.Keep(name) VALUES('surviving row')"));
    ASSERT_EQ(BE_SQLITE_DONE, insert.Step());
    ASSERT_EQ(BE_SQLITE_OK, scenario.m_firstImporter->SaveChanges());
    }
    scenario.m_firstImporter->PullMergePush("insert the row that survives the delete");
    scenario.m_secondImporter->PullMergePush("pick up the surviving row");
    scenario.m_bystander->PullMergePush("pick up the surviving row");
    const auto before = InstanceCensus::Take(*scenario.m_firstImporter);

    scenario.ImportConcurrently(BatchCLabelSchema("01.00.01", "relabelled by the first importer", "BatchCUpgradeLabel", "bcul"),
                                 BatchCLabelSchema("01.00.02", "relabelled by the second importer", "BatchCUpgradeLabel", "bcul"));
    scenario.Exchange(PushOrder::ImportOrder);

    auto& upgrader = *scenario.m_firstImporter;
    ASSERT_EQ(SchemaImportResult::OK,
              ImportSchema(upgrader, BatchCDynamicDeleteSchema("02.00.00", false, "BatchCUpgradeDelete", "bcud"), SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade, scenario.m_syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, upgrader.SaveChanges());
    EXPECT_FALSE(HasClass(upgrader, "BatchCUpgradeDelete", "ToDelete"));
    ExpectCensusPreserved(before, InstanceCensus::Take(upgrader), "upgrader after deleting a class following a concurrent edit");

    scenario.m_syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_STREQ("1.0.2", VersionOf(sync, "BatchCUpgradeLabel").c_str()) << "the delete upgrade rolled the label schema back";
        EXPECT_STREQ("relabelled by the second importer", DisplayLabelOf(sync, "BatchCUpgradeLabel", "Existing").c_str()) << "the delete overwrite replaced the winning label";
        EXPECT_FALSE(HasClass(sync, "BatchCUpgradeDelete", "ToDelete")) << "the overwrite kept the deleted class row";
    });

    upgrader.PullMergePush("push the delete upgrade after the concurrent edit");
    scenario.m_secondImporter->PullMergePush("pick up the delete upgrade");
    scenario.m_bystander->PullMergePush("pick up the delete upgrade");
    for (auto* db : { scenario.m_firstImporter.get(), scenario.m_secondImporter.get(), scenario.m_bystander.get() }) {
        EXPECT_STREQ("1.0.2", VersionOf(*db, "BatchCUpgradeLabel").c_str());
        EXPECT_STREQ("relabelled by the second importer", DisplayLabelOf(*db, "BatchCUpgradeLabel", "Existing").c_str());
        EXPECT_FALSE(HasClass(*db, "BatchCUpgradeDelete", "ToDelete"));
        ExpectCensusPreserved(before, InstanceCensus::Take(*db), "briefcase after the delete upgrade and concurrent edit");
        ExpectECTablesIdentical(*db, *scenario.m_firstImporter, "after the delete upgrade and concurrent edit");
    }
    ExpectPhysicalSchemaIdentical(*scenario.m_secondImporter, *scenario.m_firstImporter, "after the delete upgrade and concurrent edit");
    }

// ---------------------------------------------------------------------------------------
// CreateTriggers now runs for every table an import finds already up to date, so it is reached
// again on every later import. It has to leave the one trigger alone rather than fail or duplicate.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, RepeatedImportsLeaveAnExistingTriggerAlone)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-batchc-timestamp-idempotent");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto countTrigger = [](ECDbR db, Utf8CP triggerName) {
        Statement stmt;
        EXPECT_EQ(BE_SQLITE_OK, stmt.Prepare(db, "SELECT COUNT(*) FROM main.sqlite_master WHERE type='trigger' AND name=?"));
        stmt.BindText(1, triggerName, Statement::MakeCopy::No);
        EXPECT_EQ(BE_SQLITE_ROW, stmt.Step());
        return stmt.GetValueInt(0);
    };

    const Utf8CP triggerName = "bct_Stamped_CurrentTimeStamp";
    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, BatchCTimeStampSchema("01.00.00"), SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
    ASSERT_EQ(1, countTrigger(*b1, triggerName)) << "the importer never got the trigger this test is about";
    b1->PullMergePush("add the stamped class");
    b2->PullMergePush("pick up the stamped class");
    ASSERT_EQ(1, countTrigger(*b2, triggerName));

    // Three more imports of unrelated schemas. Stamped is untouched by each, so its table reports
    // itself up to date and the trigger pass runs over it every time.
    for (int round = 1; round <= 3; ++round) {
        const auto schemaName = Utf8PrintfString("BatchCIdempotentFiller%d", round);
        const auto alias = Utf8PrintfString("bcif%d", round);
        Utf8String xml;
        xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="%s" alias="%s" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Filler">
                    <ECProperty propertyName="value" typeName="string" />
                </ECEntityClass>
            </ECSchema>)xml", schemaName.c_str(), alias.c_str());

        ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b1, SchemaItem(xml), SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()))
            << "import round " << round << " failed, which is what a trigger that already exists would do";
        ASSERT_EQ(BE_SQLITE_OK, b1->SaveChanges());
        EXPECT_EQ(1, countTrigger(*b1, triggerName)) << "the trigger changed on the importer in round " << round;

        b1->PullMergePush(Utf8PrintfString("filler round %d", round).c_str());
        b2->PullMergePush(Utf8PrintfString("pick up filler round %d", round).c_str());
        EXPECT_EQ(1, countTrigger(*b2, triggerName)) << "the trigger changed on the receiver in round " << round;
    }

    auto b3 = hub.CreateBriefcase();
    EXPECT_EQ(1, countTrigger(*b3, triggerName)) << "a briefcase built from the whole timeline has no trigger";
    ExpectPhysicalSchemaIdentical(*b2, *b1, "after repeated imports over an existing trigger");
    ExpectPhysicalSchemaIdentical(*b3, *b1, "timeline briefcase after repeated imports over an existing trigger");
    }


// ---- variants of the tests above, group D ----

namespace {

SchemaItem BatchDJoinedTableSchema(Utf8CP version, bool withAddedProperty) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
<ECSchema schemaName="BatchDJoinedTable" alias="bjt" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
<ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
<ECEntityClass typeName="Base" modifier="Abstract">
<ECCustomAttributes>
<ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
<JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00" />
</ECCustomAttributes>
<ECProperty propertyName="baseProp" typeName="int" />
</ECEntityClass>
<ECEntityClass typeName="Sub1">
<BaseClass>Base</BaseClass>
<ECProperty propertyName="sub1Prop" typeName="int" />
%s
</ECEntityClass>
<ECEntityClass typeName="Sub2">
<BaseClass>Base</BaseClass>
<ECProperty propertyName="sub2Prop" typeName="int" />
</ECEntityClass>
</ECSchema>)xml", version, withAddedProperty ? R"xml(<ECProperty propertyName="addedProp" typeName="int" />)xml" : "");
    return SchemaItem(xml);
}

SchemaItem BatchDAssetPumpSchema(Utf8CP schemaName, Utf8CP alias, Utf8CP version, int extraPropertyCount) {
    static const char* const propertyNames[] = { "owner", "pressure", "serial", "spare", "secondSerial", "secondSpare" };
    static const char* const propertyTypes[] = { "string", "double", "string", "int", "string", "int" };
    Utf8String extraProperties;
    for (int i = 0; i < extraPropertyCount; ++i)
        extraProperties.append(SqlPrintfString("<ECProperty propertyName=\"%s\" typeName=\"%s\" />\n", propertyNames[i], propertyTypes[i]).GetUtf8CP());

    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
<ECSchema schemaName="%s" alias="%s" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
<ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
<ECEntityClass typeName="Asset">
<ECCustomAttributes>
<ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
<ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow></ShareColumns>
</ECCustomAttributes>
<ECProperty propertyName="name" typeName="string" />
</ECEntityClass>
<ECEntityClass typeName="Pump">
<BaseClass>Asset</BaseClass>
<ECProperty propertyName="flowRate" typeName="double" />
%s
</ECEntityClass>
</ECSchema>)xml", schemaName, alias, version, extraProperties.c_str());
    return SchemaItem(xml);
}

void BatchDInsertAssetPumpRows(ECDbR db, Utf8CP alias, Utf8CP suffix, int extraPropertyCount) {
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, Utf8PrintfString("INSERT INTO %s.Asset(name) VALUES(?)", alias).c_str()));
    stmt.BindText(1, Utf8PrintfString("asset-%s", suffix).c_str(), IECSqlBinder::MakeCopy::Yes);
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    static const char* const propertyNames[] = { "owner", "pressure", "serial", "spare", "secondSerial", "secondSpare" };
    Utf8String columns("name,flowRate");
    Utf8String values("?,42.5");
    for (int i = 0; i < extraPropertyCount; ++i) {
        columns.append(",");
        columns.append(propertyNames[i]);
        values.append(",?");
    }

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, Utf8PrintfString("INSERT INTO %s.Pump(%s) VALUES(%s)", alias, columns.c_str(), values.c_str()).c_str()));
    stmt.BindText(1, Utf8PrintfString("pump-%s", suffix).c_str(), IECSqlBinder::MakeCopy::Yes);
    int bindIndex = 2;
    for (int i = 0; i < extraPropertyCount; ++i) {
        if (i == 1)
            stmt.BindDouble(bindIndex++, 100.0 + i);
        else if (i == 3 || i == 5)
            stmt.BindInt(bindIndex++, 10 + i);
        else
            stmt.BindText(bindIndex++, Utf8PrintfString("%s-%s", propertyNames[i], suffix).c_str(), IECSqlBinder::MakeCopy::Yes);
    }
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    ASSERT_EQ(BE_SQLITE_OK, db.SaveChanges());
}

SchemaItem BatchDAddedSubclassSchema(Utf8CP version, bool withAddedClass) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
<ECSchema schemaName="BatchDAddedSubclass" alias="bca" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
<ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
<ECEntityClass typeName="Base">
<ECCustomAttributes>
<ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
<ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow></ShareColumns>
</ECCustomAttributes>
<ECProperty propertyName="name" typeName="string" />
<ECProperty propertyName="baseValue" typeName="int" />
</ECEntityClass>
<ECEntityClass typeName="Existing">
<BaseClass>Base</BaseClass>
<ECProperty propertyName="existingValue" typeName="int" />
</ECEntityClass>
%s
</ECSchema>)xml", version, withAddedClass ? R"xml(<ECEntityClass typeName="Added">
<BaseClass>Base</BaseClass>
<ECProperty propertyName="addedValue" typeName="int" />
</ECEntityClass>)xml" : "");
    return SchemaItem(xml);
}

SchemaItem BatchDBasePropertySchema(Utf8CP version, bool withAddedProperty) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
<ECSchema schemaName="BatchDBaseProperty" alias="bpb" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
<ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
<ECEntityClass typeName="Base">
<ECCustomAttributes>
<ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
<ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>8</MaxSharedColumnsBeforeOverflow></ShareColumns>
</ECCustomAttributes>
<ECProperty propertyName="name" typeName="string" />
%s
</ECEntityClass>
<ECEntityClass typeName="Child">
<BaseClass>Base</BaseClass>
<ECProperty propertyName="childValue" typeName="double" />
</ECEntityClass>
</ECSchema>)xml", version, withAddedProperty ? R"xml(<ECProperty propertyName="baseAdded" typeName="string" />)xml" : "");
    return SchemaItem(xml);
}

SchemaItem BatchDTwoMixinsSchema(Utf8CP version, bool withSecondMixin) {
    Utf8String secondMixin;
    if (withSecondMixin)
        secondMixin = R"xml(<ECEntityClass typeName="SecondMixin" modifier="Abstract">
<ECCustomAttributes><IsMixin xmlns="CoreCustomAttributes.01.00.00"><AppliesToEntityClass>Pump</AppliesToEntityClass></IsMixin></ECCustomAttributes>
<ECProperty propertyName="secondAuditCode" typeName="string" />
</ECEntityClass>)xml";

    Utf8String secondBaseClass = withSecondMixin ? R"xml(<BaseClass>SecondMixin</BaseClass>)xml" : "";
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
<ECSchema schemaName="BatchDTwoMixins" alias="btm" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
<ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
<ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA"/>
<ECEntityClass typeName="FirstMixin" modifier="Abstract">
<ECCustomAttributes><IsMixin xmlns="CoreCustomAttributes.01.00.00"><AppliesToEntityClass>Pump</AppliesToEntityClass></IsMixin></ECCustomAttributes>
<ECProperty propertyName="firstAuditCode" typeName="string" />
</ECEntityClass>
%s
<ECEntityClass typeName="Asset">
<ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
<ECProperty propertyName="name" typeName="string" />
</ECEntityClass>
<ECEntityClass typeName="Pump">
<BaseClass>Asset</BaseClass>
<BaseClass>FirstMixin</BaseClass>
%s
<ECProperty propertyName="flowRate" typeName="double" />
</ECEntityClass>
</ECSchema>)xml", version, secondMixin.c_str(), secondBaseClass.c_str());
    return SchemaItem(xml);
}

SchemaItem BatchDLinkRelationshipSchema(Utf8CP version, bool withRelationship) {
    Utf8String relationship;
    if (withRelationship)
        relationship = R"xml(<ECRelationshipClass typeName="ParentHasChildren" strength="referencing" modifier="None">
<Source multiplicity="(0..*)" roleLabel="has" polymorphic="true"><Class class="Parent" /></Source>
<Target multiplicity="(0..*)" roleLabel="is referenced by" polymorphic="true"><Class class="Child" /></Target>
</ECRelationshipClass>)xml";

    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
<ECSchema schemaName="BatchDLinkRelationship" alias="blr" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
<ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
<ECEntityClass typeName="Parent">
<ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
<ECProperty propertyName="name" typeName="string" />
</ECEntityClass>
<ECEntityClass typeName="Child">
<ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
<ECProperty propertyName="name" typeName="string" />
</ECEntityClass>
%s
</ECSchema>)xml", version, relationship.c_str());
    return SchemaItem(xml);
}

SchemaItem BatchDPresentationSchema(Utf8CP version, bool withPresentationMetadata) {
    Utf8String category;
    Utf8String classLabel;
    Utf8String propertyCategory;
    if (withPresentationMetadata) {
        category = R"xml(<PropertyCategory typeName="Presentation" description="Presentation values" displayLabel="Presentation" priority="50" />)xml";
        classLabel = " displayLabel=\"Presented Record\"";
        propertyCategory = " category=\"Presentation\"";
    }

    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
<ECSchema schemaName="BatchDPresentation" alias="bpm" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
%s
<ECEntityClass typeName="Record"%s>
<ECProperty propertyName="value" typeName="string"%s />
</ECEntityClass>
</ECSchema>)xml", version, category.c_str(), classLabel.c_str(), propertyCategory.c_str());
    return SchemaItem(xml);
}

SchemaItem BatchDEnumerationSchema(Utf8CP version, bool isStrict) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
<ECSchema schemaName="BatchDEnumeration" alias="ben" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
<ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
<ECEnumeration typeName="Colour" backingTypeName="int" isStrict="%s">
<ECEnumerator name="Red" value="1" displayLabel="Red" />
<ECEnumerator name="Green" value="2" displayLabel="Green" />
</ECEnumeration>
<ECEntityClass typeName="Paint">
<ECCustomAttributes><ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap></ECCustomAttributes>
<ECProperty propertyName="name" typeName="string" />
<ECProperty propertyName="colour" typeName="Colour" />
</ECEntityClass>
</ECSchema>)xml", version, isStrict ? "true" : "false");
    return SchemaItem(xml);
}

bool BatchDEnumerationIsStrict(ECDbR db) {
    Statement stmt;
    if (stmt.Prepare(db, "SELECT e.IsStrict FROM main.ec_Enumeration e JOIN main.ec_Schema s ON s.Id=e.SchemaId WHERE s.Name='BatchDEnumeration' AND e.Name='Colour'") != BE_SQLITE_OK)
        return false;
    return stmt.Step() == BE_SQLITE_ROW && stmt.GetValueInt(0) != 0;
}

bool BatchDHasPresentationCategory(ECDbR db) {
    Statement stmt;
    if (stmt.Prepare(db, "SELECT 1 FROM main.ec_PropertyCategory pc JOIN main.ec_Schema s ON s.Id=pc.SchemaId WHERE s.Name='BatchDPresentation' AND pc.Name='Presentation'") != BE_SQLITE_OK)
        return false;
    return stmt.Step() == BE_SQLITE_ROW;
}

} // namespace

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// The added property is mapped into a joined child table, while every populated subclass row
// already spans the root table and that child table.
TEST_F(SchemaSyncImportExtendedTests, DataSurvivesPropertiesAddedToAJoinedTableHierarchy)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-census-joined-property");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchDJoinedTableSchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add the joined hierarchy");
    b1->PullMergePush("pick up the joined hierarchy");

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO bjt.Sub1(baseProp,sub1Prop) VALUES(11,101)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO bjt.Sub2(baseProp,sub2Prop) VALUES(22,202)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add joined hierarchy rows");
    b1->PullMergePush("pick up joined hierarchy rows");

    const auto beforeImporter = InstanceCensus::Take(*b2);
    const auto beforePuller = InstanceCensus::Take(*b1);
    ASSERT_EQ(2u, beforeImporter.GetInstanceCount());
    ASSERT_EQ(2u, beforePuller.GetInstanceCount());

    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchDJoinedTableSchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_FALSE(TableOf(*b2, "BatchDJoinedTable", "Sub1", "addedProp").empty());
    ExpectCensusPreserved(beforeImporter, InstanceCensus::Take(*b2), "importer after adding a joined-table child property");

    b2->PullMergePush("add the joined-table child property");
    b1->PullMergePush("pick up the joined-table child property");
    ExpectCensusPreserved(beforePuller, InstanceCensus::Take(*b1), "puller after adding a joined-table child property");
    ExpectPhysicalSchemaIdentical(*b1, *b2, "joined hierarchy after adding a child property");
    VerifyFileIsSound(*b2, "importer after adding a joined-table child property");
    VerifyFileIsSound(*b1, "puller after adding a joined-table child property");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// Each schema changeset is followed by rows before the next schema changeset, so the puller
// rebuilds from ec_ repeatedly while its existing data grows and eventually spills.
TEST_F(SchemaSyncImportExtendedTests, DataSurvivesOnAPullingBriefcaseAcrossSeveralSchemaChangesets)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-census-several-schema-changesets");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchDAssetPumpSchema("BatchDSequentialCensus", "bds", "01.00.00", 0) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add the first sequential schema");
    b1->PullMergePush("pick up the first sequential schema");
    BatchDInsertAssetPumpRows(*b1, "bds", "before-first-change", 0);
    b1->PullMergePush("add rows before the first schema change");
    b2->PullMergePush("pick up rows before the first schema change");

    const auto beforeFirstImporter = InstanceCensus::Take(*b2);
    const auto beforeFirstPuller = InstanceCensus::Take(*b1);
    ASSERT_EQ(2u, beforeFirstPuller.GetInstanceCount());
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchDAssetPumpSchema("BatchDSequentialCensus", "bds", "01.00.01", 1) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    ExpectCensusPreserved(beforeFirstImporter, InstanceCensus::Take(*b2), "importer after the first sequential schema change");
    b2->PullMergePush("push the first sequential schema change");
    b1->PullMergePush("pull the first sequential schema change");
    ExpectCensusPreserved(beforeFirstPuller, InstanceCensus::Take(*b1), "puller after the first sequential schema change");
    BatchDInsertAssetPumpRows(*b1, "bds", "between-first-and-second-change", 1);
    b1->PullMergePush("add rows after the first sequential schema change");
    b2->PullMergePush("pick up rows after the first sequential schema change");

    const auto beforeSecondImporter = InstanceCensus::Take(*b2);
    const auto beforeSecondPuller = InstanceCensus::Take(*b1);
    ASSERT_EQ(4u, beforeSecondPuller.GetInstanceCount());
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchDAssetPumpSchema("BatchDSequentialCensus", "bds", "01.00.02", 2) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    ExpectCensusPreserved(beforeSecondImporter, InstanceCensus::Take(*b2), "importer after the second sequential schema change");
    b2->PullMergePush("push the second sequential schema change");
    b1->PullMergePush("pull the second sequential schema change");
    ExpectCensusPreserved(beforeSecondPuller, InstanceCensus::Take(*b1), "puller after the second sequential schema change");
    BatchDInsertAssetPumpRows(*b1, "bds", "between-second-and-third-change", 2);
    b1->PullMergePush("add rows after the second sequential schema change");
    b2->PullMergePush("pick up more sequential rows");

    const auto beforeThirdImporter = InstanceCensus::Take(*b2);
    const auto beforeThirdPuller = InstanceCensus::Take(*b1);
    ASSERT_EQ(6u, beforeThirdPuller.GetInstanceCount());
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchDAssetPumpSchema("BatchDSequentialCensus", "bds", "01.00.03", 4) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    ExpectCensusPreserved(beforeThirdImporter, InstanceCensus::Take(*b2), "importer after the third sequential schema change");
    b2->PullMergePush("push the third sequential schema change");
    b1->PullMergePush("pull the third sequential schema change");
    ExpectCensusPreserved(beforeThirdPuller, InstanceCensus::Take(*b1), "puller after the third sequential schema change");
    EXPECT_STRNE(TableOf(*b1, "BatchDSequentialCensus", "Pump", "flowRate").c_str(), TableOf(*b1, "BatchDSequentialCensus", "Pump", "spare").c_str());
    ExpectPhysicalSchemaIdentical(*b1, *b2, "puller after several sequential schema changesets");
    VerifyFileIsSound(*b1, "puller after several sequential schema changesets");
    VerifyFileIsSound(*b2, "importer after several sequential schema changesets");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// The class and its first overflow layout arrive in one changeset with rows already requiring
// an overflow row, so the puller has to build both tables before applying those rows.
TEST_F(SchemaSyncImportExtendedTests, DataInsertedInTheSameChangesetAsItsSpilledClassReachesAPullingBriefcase)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-census-same-changeset-spill");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchDAssetPumpSchema("BatchDSpilledCensus", "bsc", "01.00.00", 4) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    BatchDInsertAssetPumpRows(*b2, "bsc", "same-changeset-spill", 4);
    const auto before = InstanceCensus::Take(*b2);
    ASSERT_EQ(2u, before.GetInstanceCount());
    EXPECT_STRNE(TableOf(*b2, "BatchDSpilledCensus", "Pump", "flowRate").c_str(), TableOf(*b2, "BatchDSpilledCensus", "Pump", "spare").c_str());

    b2->PullMergePush("add the spilled class and its rows together");
    b1->PullMergePush("pull the spilled class and its rows together");

    ExpectCensusPreserved(before, InstanceCensus::Take(*b1), "puller after the spilled class and its rows arrived together");
    ExpectPhysicalSchemaIdentical(*b1, *b2, "puller vs importer after a spilled class arrived with rows");
    VerifyFileIsSound(*b1, "puller after a spilled class arrived with rows");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// The new subclass enters the shared-column pool below a base class that already has both a row
// and a sibling mapping, so its slot allocation competes with existing data.
TEST_F(SchemaSyncImportExtendedTests, DataSurvivesASubclassAddedUnderAPopulatedClass)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-census-added-subclass");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchDAddedSubclassSchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add the populated base hierarchy");
    b1->PullMergePush("pick up the populated base hierarchy");

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO bca.Base(name,baseValue) VALUES('base',1)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO bca.Existing(name,baseValue,existingValue) VALUES('existing',2,3)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add rows to the populated base hierarchy");
    b1->PullMergePush("pick up rows in the populated base hierarchy");

    const auto beforeImporter = InstanceCensus::Take(*b2);
    const auto beforePuller = InstanceCensus::Take(*b1);
    ASSERT_EQ(2u, beforeImporter.GetInstanceCount());
    ASSERT_EQ(2u, beforePuller.GetInstanceCount());

    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchDAddedSubclassSchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_TRUE(HasClass(*b2, "BatchDAddedSubclass", "Added"));
    ExpectCensusPreserved(beforeImporter, InstanceCensus::Take(*b2), "importer after adding a subclass under a populated class");

    b2->PullMergePush("add the subclass under the populated class");
    b1->PullMergePush("pick up the subclass under the populated class");
    EXPECT_TRUE(HasClass(*b1, "BatchDAddedSubclass", "Added"));
    ExpectCensusPreserved(beforePuller, InstanceCensus::Take(*b1), "puller after adding a subclass under a populated class");
    ExpectPhysicalSchemaIdentical(*b1, *b2, "populated shared-column pool after adding a subclass");
    VerifyFileIsSound(*b1, "puller after adding a subclass under a populated class");
    VerifyFileIsSound(*b2, "importer after adding a subclass under a populated class");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// Adding the base property remaps the shared-column view for every subclass at once, including
// the populated Child row.
TEST_F(SchemaSyncImportExtendedTests, DataSurvivesAPropertyAddedToTheBaseClassOfAPopulatedSubclass)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-census-base-property");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchDBasePropertySchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add the base and populated subclass");
    b1->PullMergePush("pick up the base and populated subclass");

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO bpb.Base(name) VALUES('base')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO bpb.Child(name,childValue) VALUES('child',42.5)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add rows to the base and subclass");
    b1->PullMergePush("pick up rows to the base and subclass");

    const auto beforeImporter = InstanceCensus::Take(*b2);
    const auto beforePuller = InstanceCensus::Take(*b1);
    ASSERT_EQ(2u, beforeImporter.GetInstanceCount());
    ASSERT_EQ(2u, beforePuller.GetInstanceCount());

    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchDBasePropertySchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_FALSE(ColumnOf(*b2, "BatchDBaseProperty", "Base", "baseAdded").empty());
    ExpectCensusPreserved(beforeImporter, InstanceCensus::Take(*b2), "importer after adding a property to the base class");

    b2->PullMergePush("add the base property");
    b1->PullMergePush("pick up the base property");
    EXPECT_FALSE(ColumnOf(*b1, "BatchDBaseProperty", "Base", "baseAdded").empty());
    ExpectCensusPreserved(beforePuller, InstanceCensus::Take(*b1), "puller after adding a property to the base class");
    ExpectPhysicalSchemaIdentical(*b1, *b2, "populated subclass after adding a base property");
    VerifyFileIsSound(*b1, "puller after adding a base property");
    VerifyFileIsSound(*b2, "importer after adding a base property");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// The second mixin must be appended after the first in ec_ClassHasBaseClasses while the existing
// Pump row keeps its first mixin property.
TEST_F(SchemaSyncImportExtendedTests, DataSurvivesASecondMixinAddedToAClassThatAlreadyHasOne)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-census-second-mixin");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchDTwoMixinsSchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add the first mixin");
    b1->PullMergePush("pick up the first mixin");

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO btm.Pump(name,firstAuditCode,flowRate) VALUES('pump','first',42.5)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add the populated mixed-in class");
    b1->PullMergePush("pick up the populated mixed-in class");

    const auto beforeImporter = InstanceCensus::Take(*b2);
    const auto beforePuller = InstanceCensus::Take(*b1);
    ASSERT_EQ(1u, beforeImporter.GetInstanceCount());
    ASSERT_EQ(1u, beforePuller.GetInstanceCount());

    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchDTwoMixinsSchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_TRUE(HasClass(*b2, "BatchDTwoMixins", "SecondMixin"));
    ExpectCensusPreserved(beforeImporter, InstanceCensus::Take(*b2), "importer after adding a second mixin");

    b2->PullMergePush("add the second mixin");
    b1->PullMergePush("pick up the second mixin");
    EXPECT_TRUE(HasClass(*b1, "BatchDTwoMixins", "SecondMixin"));
    ExpectCensusPreserved(beforePuller, InstanceCensus::Take(*b1), "puller after adding a second mixin");
    ExpectPhysicalSchemaIdentical(*b1, *b2, "populated mixed-in class after adding a second mixin");
    VerifyFileIsSound(*b1, "puller after adding a second mixin");
    VerifyFileIsSound(*b2, "importer after adding a second mixin");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// A link-table relationship adds a physical table and two foreign-key paths; the existing end
// rows must survive even though the relationship has no prior data row.
TEST_F(SchemaSyncImportExtendedTests, DataSurvivesALinkTableRelationshipAddedThroughTheSyncDb)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-census-link-relationship");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchDLinkRelationshipSchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add the relationship endpoints");
    b1->PullMergePush("pick up the relationship endpoints");

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO blr.Parent(name) VALUES('parent')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO blr.Child(name) VALUES('child')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add relationship endpoint rows");
    b1->PullMergePush("pick up relationship endpoint rows");

    const auto beforeImporter = InstanceCensus::Take(*b2);
    const auto beforePuller = InstanceCensus::Take(*b1);
    ASSERT_EQ(2u, beforeImporter.GetInstanceCount());
    ASSERT_EQ(2u, beforePuller.GetInstanceCount());

    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchDLinkRelationshipSchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    ExpectCensusPreserved(beforeImporter, InstanceCensus::Take(*b2), "importer after adding a link-table relationship");
    EXPECT_TRUE(HasPhysicalTable(*b2, "blr_ParentHasChildren"));

    b2->PullMergePush("add the link-table relationship");
    b1->PullMergePush("pick up the link-table relationship");
    ExpectCensusPreserved(beforePuller, InstanceCensus::Take(*b1), "puller after adding a link-table relationship");
    EXPECT_TRUE(HasPhysicalTable(*b1, "blr_ParentHasChildren"));
    ExpectPhysicalSchemaIdentical(*b1, *b2, "endpoints after adding a link-table relationship");
    VerifyFileIsSound(*b1, "puller after adding a link-table relationship");
    VerifyFileIsSound(*b2, "importer after adding a link-table relationship");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// The change only adds presentation metadata - a property category and a class label - so the
// physical schema must stay byte-for-byte unchanged while the populated row remains readable.
TEST_F(SchemaSyncImportExtendedTests, DataSurvivesAPropertyCategoryAndDisplayLabelAddedToAPopulatedClass)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-census-presentation-metadata");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchDPresentationSchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add the presentation test class");
    b1->PullMergePush("pick up the presentation test class");

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO bpm.Record([value]) VALUES('value')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add the populated presentation test class");
    b1->PullMergePush("pick up the populated presentation test class");

    const auto beforeImporter = InstanceCensus::Take(*b2);
    const auto beforePuller = InstanceCensus::Take(*b1);
    const auto recordDdlBefore = DdlOf(*b2, "bpm_Record");
    ASSERT_EQ(1u, beforeImporter.GetInstanceCount());
    ASSERT_EQ(1u, beforePuller.GetInstanceCount());

    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchDPresentationSchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_TRUE(BatchDHasPresentationCategory(*b2));
    EXPECT_STREQ(recordDdlBefore.c_str(), DdlOf(*b2, "bpm_Record").c_str());
    EXPECT_STREQ("Presented Record", DisplayLabelOf(*b2, "BatchDPresentation", "Record").c_str());
    ExpectCensusPreserved(beforeImporter, InstanceCensus::Take(*b2), "importer after adding presentation metadata");

    b2->PullMergePush("add the presentation metadata");
    b1->PullMergePush("pick up the presentation metadata");
    EXPECT_TRUE(BatchDHasPresentationCategory(*b1));
    EXPECT_STREQ(recordDdlBefore.c_str(), DdlOf(*b1, "bpm_Record").c_str());
    EXPECT_STREQ("Presented Record", DisplayLabelOf(*b1, "BatchDPresentation", "Record").c_str());
    ExpectCensusPreserved(beforePuller, InstanceCensus::Take(*b1), "puller after adding presentation metadata");
    ExpectPhysicalSchemaIdentical(*b1, *b2, "physical schema after adding presentation metadata");
    VerifyFileIsSound(*b1, "puller after adding presentation metadata");
    VerifyFileIsSound(*b2, "importer after adding presentation metadata");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// Relaxing IsStrict rewrites the enumeration definition row while a populated property keeps its
// stored enumerator value.
TEST_F(SchemaSyncImportExtendedTests, DataSurvivesAnEnumerationRelaxedFromStrictToNonStrict)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-census-enumeration-relaxed");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchDEnumerationSchema("01.00.00", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add the strict enumeration");
    b1->PullMergePush("pick up the strict enumeration");
    EXPECT_TRUE(BatchDEnumerationIsStrict(*b2));
    EXPECT_TRUE(BatchDEnumerationIsStrict(*b1));

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b2, "INSERT INTO ben.Paint(name,colour) VALUES('paint',1)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add the populated enumeration property");
    b1->PullMergePush("pick up the populated enumeration property");

    const auto beforeImporter = InstanceCensus::Take(*b2);
    const auto beforePuller = InstanceCensus::Take(*b1);
    ASSERT_EQ(1u, beforeImporter.GetInstanceCount());
    ASSERT_EQ(1u, beforePuller.GetInstanceCount());

    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchDEnumerationSchema("01.00.01", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_FALSE(BatchDEnumerationIsStrict(*b2));
    ExpectCensusPreserved(beforeImporter, InstanceCensus::Take(*b2), "importer after relaxing the enumeration");

    b2->PullMergePush("relax the enumeration");
    b1->PullMergePush("pick up the relaxed enumeration");
    EXPECT_FALSE(BatchDEnumerationIsStrict(*b1));
    ExpectCensusPreserved(beforePuller, InstanceCensus::Take(*b1), "puller after relaxing the enumeration");
    ExpectPhysicalSchemaIdentical(*b1, *b2, "physical schema after relaxing the enumeration");
    VerifyFileIsSound(*b1, "puller after relaxing the enumeration");
    VerifyFileIsSound(*b2, "importer after relaxing the enumeration");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// The second import adds properties to an overflow table that already exists and already has rows;
// b1 never imports either schema and exercises the post-data-change catch-up while pulling.
TEST_F(SchemaSyncImportExtendedTests, DataSurvivesASecondSpillOnABriefcaseThatOnlyPulls)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-census-second-spill-puller");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchDAssetPumpSchema("BatchDSecondSpillCensus", "bss", "01.00.00", 0) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add the second-spill base schema");
    b1->PullMergePush("pick up the second-spill base schema");
    BatchDInsertAssetPumpRows(*b1, "bss", "before-first-spill", 0);
    b1->PullMergePush("add rows before the first spill");
    b2->PullMergePush("pick up rows before the first spill");

    const auto beforeFirstImporter = InstanceCensus::Take(*b2);
    const auto beforeFirstPuller = InstanceCensus::Take(*b1);
    ASSERT_EQ(2u, beforeFirstPuller.GetInstanceCount());
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchDAssetPumpSchema("BatchDSecondSpillCensus", "bss", "01.00.01", 4) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_STRNE(TableOf(*b2, "BatchDSecondSpillCensus", "Pump", "flowRate").c_str(), TableOf(*b2, "BatchDSecondSpillCensus", "Pump", "spare").c_str());
    ExpectCensusPreserved(beforeFirstImporter, InstanceCensus::Take(*b2), "importer after the first spill");
    b2->PullMergePush("push the first spill");
    b1->PullMergePush("pull the first spill");
    EXPECT_STRNE(TableOf(*b1, "BatchDSecondSpillCensus", "Pump", "flowRate").c_str(), TableOf(*b1, "BatchDSecondSpillCensus", "Pump", "spare").c_str());
    ExpectCensusPreserved(beforeFirstPuller, InstanceCensus::Take(*b1), "puller after the first spill");

    BatchDInsertAssetPumpRows(*b1, "bss", "between-spills", 4);
    b1->PullMergePush("add rows to the existing overflow table");
    b2->PullMergePush("pick up rows in the existing overflow table");

    const auto beforeSecondImporter = InstanceCensus::Take(*b2);
    const auto beforeSecondPuller = InstanceCensus::Take(*b1);
    ASSERT_EQ(4u, beforeSecondPuller.GetInstanceCount());
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { BatchDAssetPumpSchema("BatchDSecondSpillCensus", "bss", "01.00.02", 6) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_STREQ(TableOf(*b2, "BatchDSecondSpillCensus", "Pump", "spare").c_str(), TableOf(*b2, "BatchDSecondSpillCensus", "Pump", "secondSpare").c_str())
        << "the later spill should reuse the existing overflow table";
    ExpectCensusPreserved(beforeSecondImporter, InstanceCensus::Take(*b2), "importer after the second spill");
    b2->PullMergePush("push the second spill");
    b1->PullMergePush("pull the second spill");
    ExpectCensusPreserved(beforeSecondPuller, InstanceCensus::Take(*b1), "puller after the second spill");
    VerifyFileIsSound(*b1, "puller after the second spill");
    VerifyFileIsSound(*b2, "importer after the second spill");
    }


// ---- variants of the tests above, group E ----

namespace {

SchemaItem BatchECensusSchema(Utf8CP schemaName, Utf8CP alias, Utf8CP version, int extraPropertyCount) {
    Utf8String extraProperties;
    const Utf8CP propertyNames[] = { "owner", "pressure", "serial", "spare" };
    const Utf8CP propertyTypes[] = { "string", "double", "string", "int" };
    for (int i = 0; i < extraPropertyCount; ++i) {
        Utf8String propertyName;
        Utf8CP propertyType;
        if (i < 4) {
            propertyName = propertyNames[i];
            propertyType = propertyTypes[i];
        } else {
            propertyName.Sprintf("spare%d", i - 2);
            propertyType = "int";
        }
        extraProperties.append(Utf8PrintfString("<ECProperty propertyName=\"%s\" typeName=\"%s\" />\n", propertyName.c_str(), propertyType).c_str());
    }

    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="%s" alias="%s" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Asset">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow></ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Pump">
                <BaseClass>Asset</BaseClass>
                <ECProperty propertyName="flowRate" typeName="double" />
                %s
            </ECEntityClass>
        </ECSchema>)xml", schemaName, alias, version, extraProperties.c_str());
    return SchemaItem(xml);
}

SchemaItem BatchEOverflowSchema(Utf8CP schemaName, Utf8CP alias) {
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="%s" alias="%s" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Spill">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow></ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="p1" typeName="int" />
                <ECProperty propertyName="p2" typeName="int" />
                <ECProperty propertyName="p3" typeName="int" />
                <ECProperty propertyName="p4" typeName="int" />
                <ECProperty propertyName="p5" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml", schemaName, alias);
    return SchemaItem(xml);
}

SchemaItem BatchEIdSequenceSchema(Utf8CP version, bool withAddedClass) {
    const Utf8CP addedProperty = withAddedClass ? R"xml(
                <ECProperty propertyName="sequenceLabel" typeName="string" />)xml" : "";
    const Utf8CP addedClass = withAddedClass ? R"xml(
            <ECEntityClass typeName="Added">
                <ECProperty propertyName="name" typeName="string" />
            </ECEntityClass>)xml" : "";
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BatchIdSequenceTest" alias="bis" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Asset">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>4</MaxSharedColumnsBeforeOverflow></ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="name" typeName="string" />
                %s
            </ECEntityClass>
            <ECEntityClass typeName="Pump">
                <BaseClass>Asset</BaseClass>
                <ECProperty propertyName="flowRate" typeName="double" />
            </ECEntityClass>
            %s
        </ECSchema>)xml", version, addedProperty, addedClass);
    return SchemaItem(xml);
}

SchemaItem BatchERelationshipSchema(Utf8CP version, bool withLinkRelationship) {
    const Utf8CP relationship = withLinkRelationship ? R"xml(
            <ECRelationshipClass typeName="ChildHasTags" strength="referencing" modifier="Sealed">
                <Source multiplicity="(0..*)" roleLabel="has" polymorphic="true"><Class class="Child" /></Source>
                <Target multiplicity="(0..*)" roleLabel="belongs to" polymorphic="true"><Class class="Tag" /></Target>
            </ECRelationshipClass>)xml" : "";
    const Utf8CP dynamicSchema = R"xml(
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECCustomAttributes><DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/></ECCustomAttributes>)xml";
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BatchRelationshipDeleteTest" alias="brd" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            %s
            <ECEntityClass typeName="Child">
                <ECProperty propertyName="name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Tag">
                <ECProperty propertyName="name" typeName="string" />
            </ECEntityClass>
            %s
        </ECSchema>)xml", version, dynamicSchema, relationship);
    return SchemaItem(xml);
}

SchemaItem BatchEPropertyDeletionSchema(Utf8CP version, bool withDeletedProperty) {
    const Utf8CP dynamicSchema = R"xml(
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECCustomAttributes><DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/></ECCustomAttributes>)xml";
    const Utf8CP deletedProperty = withDeletedProperty ? R"xml(
                <ECProperty propertyName="flowRate" typeName="double" />)xml" : "";
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BatchPropertyDeleteTest" alias="bpd" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            %s
            <ECEntityClass typeName="Asset">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>TablePerHierarchy</MapStrategy></ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00"><MaxSharedColumnsBeforeOverflow>8</MaxSharedColumnsBeforeOverflow></ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="name" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="Pump">
                <BaseClass>Asset</BaseClass>
                %s
                <ECProperty propertyName="serial" typeName="string" />
                <ECProperty propertyName="label" typeName="string" />
            </ECEntityClass>
        </ECSchema>)xml", version, dynamicSchema, deletedProperty);
    return SchemaItem(xml);
}

SchemaItem BatchEOwnColumnPropertySchema(Utf8CP version, bool withDeletedProperty) {
    const Utf8CP dynamicSchema = R"xml(
            <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA"/>
            <ECCustomAttributes><DynamicSchema xmlns="CoreCustomAttributes.01.00.03"/></ECCustomAttributes>)xml";
    const Utf8CP deletedProperty = withDeletedProperty ? R"xml(
                <ECProperty propertyName="dropMe" typeName="int" />)xml" : "";
    Utf8String xml;
    xml.Sprintf(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="BatchOwnColumnDeleteTest" alias="boc" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            %s
            <ECEntityClass typeName="Record">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00"><MapStrategy>OwnTable</MapStrategy></ClassMap>
                </ECCustomAttributes>
                <ECProperty propertyName="keepMe" typeName="string" />
                %s
                <ECProperty propertyName="alsoKeep" typeName="int" />
            </ECEntityClass>
        </ECSchema>)xml", version, dynamicSchema, deletedProperty);
    return SchemaItem(xml);
}

void BatchEInsertCensusInstances(ECDbR db, Utf8CP alias, Utf8CP suffix) {
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, Utf8PrintfString("INSERT INTO %s.Asset(name) VALUES(?)", alias).c_str()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, Utf8PrintfString("asset-%s", suffix).c_str(), IECSqlBinder::MakeCopy::Yes));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, Utf8PrintfString("INSERT INTO %s.Pump(name,flowRate) VALUES(?,42.5)", alias).c_str()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, Utf8PrintfString("pump-%s", suffix).c_str(), IECSqlBinder::MakeCopy::Yes));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }
    ASSERT_EQ(BE_SQLITE_OK, db.SaveChanges());
}

void BatchEApplyUnpushedUpdatesDeletesAndInserts(ECDbR db, Utf8CP alias) {
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, Utf8PrintfString("UPDATE %s.Pump SET flowRate=84.5 WHERE name='pump-keep'", alias).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, Utf8PrintfString("DELETE FROM %s.Pump WHERE name='pump-delete'", alias).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, Utf8PrintfString("INSERT INTO %s.Asset(name) VALUES('asset-new')", alias).c_str()));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, Utf8PrintfString("INSERT INTO %s.Pump(name,flowRate) VALUES('pump-new',17.25)", alias).c_str()));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }
    ASSERT_EQ(BE_SQLITE_OK, db.SaveChanges());
}

void BatchEInsertAsset(ECDbR db, Utf8CP alias, Utf8CP name, ECInstanceKey& key) {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, Utf8PrintfString("INSERT INTO %s.Asset(name) VALUES(?)", alias).c_str()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, name, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_EQ(BE_SQLITE_OK, db.SaveChanges());
}

void BatchEInsertRelationshipData(ECDbR db) {
    ECInstanceKey childKey;
    ECInstanceKey tagKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO brd.Child(name) VALUES('child')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(childKey));
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO brd.Tag(name) VALUES('tag')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(tagKey));
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO brd.ChildHasTags(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, childKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, childKey.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, tagKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, tagKey.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }
    ASSERT_EQ(BE_SQLITE_OK, db.SaveChanges());
}

void BatchEInsertPropertyData(ECDbR db) {
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO bpd.Asset(name) VALUES('asset')"));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO bpd.Pump(name,flowRate,serial,label) VALUES('pump',42.5,'serial-value','label-value')"));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    }
    ASSERT_EQ(BE_SQLITE_OK, db.SaveChanges());
}

void BatchEInsertOwnColumnData(ECDbR db) {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(db, "INSERT INTO boc.Record(keepMe,dropMe,alsoKeep) VALUES('keep',7,11)"));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    ASSERT_EQ(BE_SQLITE_OK, db.SaveChanges());
}

} // namespace

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// The local transaction updates and deletes existing Pump rows as well as inserting rows. Those
// replay operations must find their pre-existing overflow-backed rows, and the second briefcase
// checks the changeset that the rebase produced rather than only the rebasing file.
TEST_F(SchemaSyncImportExtendedTests, DataSurvivesASpillArrivingOnTopOfUnpushedUpdatesAndDeletes)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-rebase-updates-deletes");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(),
        LoadSchemas(*b2, { BatchECensusSchema("BatchRebaseUpdateDeleteTest", "rbd", "01.00.00", 0) }).Refs(),
        SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add the rebase census schema");
    b1->PullMergePush("pick up the rebase census schema");

    BatchEInsertCensusInstances(*b2, "rbd", "keep");
    BatchEInsertCensusInstances(*b2, "rbd", "delete");
    b2->PullMergePush("add rows to the rebase census");
    b1->PullMergePush("pick up the rebase census rows");

    BatchEApplyUnpushedUpdatesDeletesAndInserts(*b1, "rbd");
    const auto before = InstanceCensus::Take(*b1);
    ASSERT_EQ(5u, before.GetInstanceCount());
    ECSqlStatement updated;
    ASSERT_EQ(ECSqlStatus::Success, updated.Prepare(*b1, "SELECT flowRate FROM rbd.Pump WHERE name='pump-keep'"));
    ASSERT_EQ(BE_SQLITE_ROW, updated.Step());
    EXPECT_DOUBLE_EQ(84.5, updated.GetValueDouble(0));

    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(),
        LoadSchemas(*b2, { BatchECensusSchema("BatchRebaseUpdateDeleteTest", "rbd", "01.00.01", 4) }).Refs(),
        SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("spill the rebase census class");
    EXPECT_STRNE(TableOf(*b2, "BatchRebaseUpdateDeleteTest", "Pump", "flowRate").c_str(),
                 TableOf(*b2, "BatchRebaseUpdateDeleteTest", "Pump", "spare").c_str());

    b1->PullMergePush("rebase the updates, delete and inserts onto the spill");
    b2->PullMergePush("pick up the rebased data changeset");

    ExpectCensusPreserved(before, InstanceCensus::Take(*b1), "the briefcase whose mixed local txn was rebased");
    ExpectCensusPreserved(before, InstanceCensus::Take(*b2), "the briefcase that received the rebased mixed txn");
    ExpectECTablesIdentical(*b2, *b1, "after the mixed local txn reached the second briefcase");
    ExpectPhysicalSchemaIdentical(*b2, *b1, "after the mixed local txn reached the second briefcase");
    VerifyFileIsSound(*b1, "rebased briefcase after mixed updates and deletes");
    VerifyFileIsSound(*b2, "receiver after mixed updates and deletes");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// The local side has one unpushed transaction while three separate incoming schema changesets
// arrive in one pull. Two incoming schemas independently create overflow tables, so catch-up
// cannot be accidentally tied to each individual changeset.
TEST_F(SchemaSyncImportExtendedTests, DataSurvivesSeveralIncomingSchemaChangesetsArrivingOnTopOfUnpushedRows)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-rebase-many-incoming");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(),
        LoadSchemas(*b2, { BatchECensusSchema("BatchManyRebaseTest", "bmr", "01.00.00", 0) }).Refs(),
        SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add the many-incoming census schema");
    b1->PullMergePush("pick up the many-incoming census schema");

    BatchEInsertCensusInstances(*b1, "bmr", "unpushed");
    const auto before = InstanceCensus::Take(*b1);
    ASSERT_EQ(2u, before.GetInstanceCount());

    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(),
        LoadSchemas(*b2, { BatchECensusSchema("BatchManyRebaseTest", "bmr", "01.00.01", 4) }).Refs(),
        SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("first incoming schema change spills the census class");
    EXPECT_STRNE(TableOf(*b2, "BatchManyRebaseTest", "Pump", "flowRate").c_str(),
                 TableOf(*b2, "BatchManyRebaseTest", "Pump", "spare").c_str());

    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(),
        LoadSchemas(*b2, { BatchEOverflowSchema("BatchManySpillTwo", "bm2") }).Refs(),
        SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("second incoming schema change spills another class");
    EXPECT_STRNE(TableOf(*b2, "BatchManySpillTwo", "Spill", "p1").c_str(),
                 TableOf(*b2, "BatchManySpillTwo", "Spill", "p5").c_str());

    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(),
        LoadSchemas(*b2, { BatchEOverflowSchema("BatchManySpillThree", "bm3") }).Refs(),
        SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("third incoming schema change spills another class");
    EXPECT_STRNE(TableOf(*b2, "BatchManySpillThree", "Spill", "p1").c_str(),
                 TableOf(*b2, "BatchManySpillThree", "Spill", "p5").c_str());

    // One pull receives all three incoming changesets, so the rebase has to bracket the whole pull.
    b1->PullMergePush("rebase the unpushed rows across three incoming schema changesets");
    b2->PullMergePush("pick up the rebased rows after all incoming schemas");

    ExpectCensusPreserved(before, InstanceCensus::Take(*b1), "the briefcase rebased across several incoming schemas");
    ExpectCensusPreserved(before, InstanceCensus::Take(*b2), "the receiver of several incoming schemas");
    ExpectECTablesIdentical(*b2, *b1, "after several incoming schema changesets");
    ExpectPhysicalSchemaIdentical(*b2, *b1, "after several incoming schema changesets");
    VerifyFileIsSound(*b1, "rebased briefcase after several incoming schemas");
    VerifyFileIsSound(*b2, "receiver after several incoming schemas");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// The incoming schema adds a class and a property to an existing class while the local transaction
// is reinstated. Both additions stay below the overflow budget, keeping this check separate from spills.
TEST_F(SchemaSyncImportExtendedTests, ARebaseDoesNotReuseInstanceIdsWhenTheIncomingChangesetAlsoAddsAClass)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-rebase-adds-class");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(),
        LoadSchemas(*b2, { BatchEIdSequenceSchema("01.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add the id sequence schema");
    b1->PullMergePush("pick up the id sequence schema");

    ECInstanceKey unpushed;
    BatchEInsertAsset(*b1, "bis", "unpushed", unpushed);

    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(),
        LoadSchemas(*b2, { BatchEIdSequenceSchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add a class while the id sequence row is unpushed");

    b1->PullMergePush("rebase the id sequence row while adding a class");

    ECInstanceKey afterRebase;
    BatchEInsertAsset(*b1, "bis", "after-rebase", afterRebase);
    EXPECT_GT(afterRebase.GetInstanceId().GetValue(), unpushed.GetInstanceId().GetValue())
        << "the rebase reused an instance id while reinstating a local row";

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*b1, "SELECT COUNT(*) FROM bis.Asset"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_EQ(2, stmt.GetValueInt(0));
    b2->PullMergePush("pick up both id sequence rows");
    VerifyFileIsSound(*b1, "briefcase after adding a class during rebase");
    VerifyFileIsSound(*b2, "receiver after adding a class during rebase");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// This removes a relationship whose data is in a link table. The link table's two foreign-key
// endpoints exercise the relationship map deletion path separately from an entity class table.
TEST_F(SchemaSyncImportExtendedTests, DeletingARelationshipClassReportsDataDeletionRequired)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-delete-relationship");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(),
        LoadSchemas(*b2, { BatchERelationshipSchema("01.00.00", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    BatchEInsertRelationshipData(*b2);
    const auto before = InstanceCensus::Take(*b2);
    ASSERT_EQ(1, CountRows(*b2, "brd_ChildHasTags"));

    {
    ScopedDisableFailOnAssertion disableFailOnAssertion;
    EXPECT_EQ(SchemaSync::Status::ERROR_DATA_DELETION_REQUIRED,
        sync2.ImportSchemas(syncDb.GetSyncDbUri(),
            LoadSchemas(*b2, { BatchERelationshipSchema("02.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None))
        << "deleting a populated relationship link table has to be refused";
    }

    ExpectCensusPreserved(before, InstanceCensus::Take(*b2), "after the refused relationship delete");
    EXPECT_TRUE(HasClass(*b2, "BatchRelationshipDeleteTest", "ChildHasTags"))
        << "the refused relationship delete removed the relationship metadata";
    EXPECT_EQ(1, CountRows(*b2, "brd_ChildHasTags")) << "the refused relationship delete removed its link row";
    syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_TRUE(HasClass(sync, "BatchRelationshipDeleteTest", "ChildHasTags"))
            << "the refused relationship delete changed the sync db";
    });
    SchemaSyncTestFixture::ExpectNoForeignKeyViolations(*b2, "after the refused relationship delete");
    VerifyFileIsSound(*b2, "after the refused relationship delete");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// The update path first refuses a shared-column property deletion, then the upgrade path performs
// it and carries the result to a briefcase that only pulls; the surviving properties are read back.
TEST_F(SchemaSyncImportExtendedTests, TheUpgradePathDeletesThePropertyTheUpdatePathRefused)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-delete-property-upgrade");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(),
        LoadSchemas(*b2, { BatchEPropertyDeletionSchema("01.00.00", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add the property deletion schema");
    b1->PullMergePush("pick up the property deletion schema");

    BatchEInsertPropertyData(*b2);
    b2->PullMergePush("add property deletion data");
    b1->PullMergePush("pick up property deletion data");
    const auto beforeImporter = InstanceCensus::Take(*b2);
    const auto beforePuller = InstanceCensus::Take(*b1);
    ASSERT_EQ(2u, beforeImporter.GetInstanceCount());
    ASSERT_EQ(2u, beforePuller.GetInstanceCount());

    {
    ScopedDisableFailOnAssertion disableFailOnAssertion;
    EXPECT_EQ(SchemaSync::Status::ERROR_DATA_DELETION_REQUIRED,
        sync2.ImportSchemas(syncDb.GetSyncDbUri(),
            LoadSchemas(*b2, { BatchEPropertyDeletionSchema("02.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None))
        << "the update path must refuse deleting a populated property";
    }

    ASSERT_EQ(SchemaImportResult::OK,
        ImportSchema(*b2, BatchEPropertyDeletionSchema("02.00.00", false),
                     SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade, syncDb.GetSyncDbUri()))
        << "the upgrade path did not accept the property deletion";
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    EXPECT_TRUE(ColumnOf(*b2, "BatchPropertyDeleteTest", "Pump", "flowRate").empty())
        << "the upgrade retained the deleted property mapping";

    ECSqlStatement remaining;
    ASSERT_EQ(ECSqlStatus::Success, remaining.Prepare(*b2, "SELECT serial,label FROM bpd.Pump"));
    ASSERT_EQ(BE_SQLITE_ROW, remaining.Step());
    EXPECT_STREQ("serial-value", remaining.GetValueText(0));
    EXPECT_STREQ("label-value", remaining.GetValueText(1));

    b2->PullMergePush("push the property deletion upgrade");
    b1->PullMergePush("pick up the property deletion upgrade");

    const std::vector<Utf8String> removedProperties{ "Pump.flowRate" };
    ExpectCensusPreserved(beforeImporter, InstanceCensus::Take(*b2), "importer after the property deletion upgrade", removedProperties);
    ExpectCensusPreserved(beforePuller, InstanceCensus::Take(*b1), "puller after the property deletion upgrade", removedProperties);
    EXPECT_TRUE(ColumnOf(*b1, "BatchPropertyDeleteTest", "Pump", "flowRate").empty())
        << "the pulling briefcase retained the deleted property mapping";
    ECSqlStatement pullerRemaining;
    ASSERT_EQ(ECSqlStatus::Success, pullerRemaining.Prepare(*b1, "SELECT serial,label FROM bpd.Pump"));
    ASSERT_EQ(BE_SQLITE_ROW, pullerRemaining.Step());
    EXPECT_STREQ("serial-value", pullerRemaining.GetValueText(0));
    EXPECT_STREQ("label-value", pullerRemaining.GetValueText(1));
    ExpectECTablesIdentical(*b1, *b2, "after the property deletion upgrade reached the puller");
    ExpectPhysicalSchemaIdentical(*b1, *b2, "after the property deletion upgrade reached the puller");
    VerifyFileIsSound(*b2, "importer after the property deletion upgrade");
    VerifyFileIsSound(*b1, "puller after the property deletion upgrade");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// This property has its own column in an OwnTable class, so the dedicated-column deletion route
// is checked independently of the shared-column UPDATE ... SET NULL route.
TEST_F(SchemaSyncImportExtendedTests, DeletingAPropertyInItsOwnColumnRemainsUnsupported)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-delete-own-column-property");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(),
        LoadSchemas(*b2, { BatchEOwnColumnPropertySchema("01.00.00", true) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    BatchEInsertOwnColumnData(*b2);

    const auto before = InstanceCensus::Take(*b2);
    const auto dropColumn = ColumnOf(*b2, "BatchOwnColumnDeleteTest", "Record", "dropMe");
    const auto keepColumn = ColumnOf(*b2, "BatchOwnColumnDeleteTest", "Record", "keepMe");
    ASSERT_FALSE(dropColumn.empty()) << "the property was not mapped before testing its deletion";
    ASSERT_FALSE(keepColumn.empty()) << "the surviving property was not mapped before testing its deletion";
    EXPECT_STRNE(dropColumn.c_str(), keepColumn.c_str()) << "the test property did not get its own physical column";
    // Two different names prove nothing on their own - two slots out of the shared pool are also two
    // names. Only Kind says which branch of SchemaWriter::DeleteProperty this test reaches, and the
    // two branches report different statuses, so pin it.
    EXPECT_EQ(COLUMN_KIND_DEFAULT, ColumnKindOf(*b2, "BatchOwnColumnDeleteTest", "Record", "dropMe"))
        << "expected a dedicated column for an OwnTable class with no ShareColumns";

    {
    ScopedDisableFailOnAssertion disableFailOnAssertion;
    EXPECT_EQ(SchemaSync::Status::ERROR,
        sync2.ImportSchemas(syncDb.GetSyncDbUri(),
            LoadSchemas(*b2, { BatchEOwnColumnPropertySchema("02.00.00", false) }).Refs(), SchemaManager::SchemaImportOptions::None))
        << "deleting a property mapped to its own column remains unsupported on both paths";
    }

    ExpectCensusPreserved(before, InstanceCensus::Take(*b2), "after the refused own-column property delete");
    EXPECT_STREQ(dropColumn.c_str(), ColumnOf(*b2, "BatchOwnColumnDeleteTest", "Record", "dropMe").c_str())
        << "the refused own-column property delete changed the mapping";
    VerifyFileIsSound(*b2, "after the refused own-column property delete");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// The mirror must patch the ec_ table shape without treating unchanged metadata rows as changed.
// GetModifiedRowCount reads sqlite3_total_changes64; foreign-key cascades count too, so it can be higher than the mirror's per-statement trace. Use a bound that catches a wholesale rewrite instead of an exact count.
TEST_F(SchemaSyncImportExtendedTests, SyncDbFollowsAnECTableColumnWithoutRewritingEveryRow)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-profile-column-differential");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(),
        LoadSchemas(*b2, { BatchECensusSchema("BatchProfileColumnTest", "bpc", "01.00.00", 0) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add the profile-column schema");
    b1->PullMergePush("pick up the profile-column schema");

    BatchEInsertCensusInstances(*b2, "bpc", "profile");
    b2->PullMergePush("add profile-column data");
    b1->PullMergePush("pick up profile-column data");
    const auto beforeImporter = InstanceCensus::Take(*b2);

    ASSERT_EQ(BE_SQLITE_OK, b2->ExecuteDdl("ALTER TABLE main.ec_Property ADD COLUMN SimulatedProfileColumn INTEGER"));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    ASSERT_EQ(SchemaSync::Status::OK, sync2.OverwriteSyncDb(syncDb.GetSyncDbUri()))
        << "the sync db has to be rebuilt after the profile column was added";

    EXPECT_LT(sync2.GetModifiedRowCount(), 100)
        << "the differential mirror modified too many rows and may have rewritten the sync db";
    syncDb.WithReadOnly([&](ECDbR sync) {
        bvector<Utf8String> columns;
        sync.GetColumns(columns, "ec_Property");
        EXPECT_TRUE(std::find(columns.begin(), columns.end(), Utf8String("SimulatedProfileColumn")) != columns.end())
            << "the mirror did not carry the new ec_Property column into the sync db";
    });

    syncDb.WithReadOnly([&](ECDbR sync) { VerifyFileIsSound(sync, "sync db after the differential profile mirror"); });
    ExpectCensusPreserved(beforeImporter, InstanceCensus::Take(*b2), "briefcase after the profile column change");
    VerifyFileIsSound(*b2, "briefcase after the differential profile mirror");
    }

// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
// A briefcase created after the profile upgrade replays the upgrade changeset from the timeline,
// so it must derive the same ec_ rows and physical schema as the briefcase that performed it.
TEST_F(SchemaSyncImportExtendedTests, ProfileUpgradeReachesABriefcaseCreatedFromTheTimelineAfterwards)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-profile-upgrade-timeline");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(),
        LoadSchemas(*b2, { BatchECensusSchema("BatchProfileUpgradeTest", "bpu", "01.00.00", 0) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add the profile-upgrade schema");
    b1->PullMergePush("pick up the profile-upgrade schema");

    BatchEInsertCensusInstances(*b2, "bpu", "profile");
    b2->PullMergePush("add profile-upgrade data");
    b1->PullMergePush("pick up profile-upgrade data");
    const auto beforeImporter = InstanceCensus::Take(*b2);
    const auto beforePuller = InstanceCensus::Take(*b1);

    const auto oldVersion = b2->GetECDbProfileVersion();
    const ProfileVersion upgradedVersion(oldVersion.GetMajor(), oldVersion.GetMinor(), oldVersion.GetSub1(), (uint16_t)(oldVersion.GetSub2() + 1));
    ASSERT_EQ(BE_SQLITE_OK, b2->ExecuteDdl("ALTER TABLE main.ec_Property ADD COLUMN SimulatedProfileColumn INTEGER"));
    ASSERT_EQ(BE_SQLITE_OK, b2->SavePropertyString(PropertySpec("SchemaVersion", "ec_Db"), upgradedVersion.ToJson()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    ASSERT_EQ(SchemaSync::Status::OK, sync2.OverwriteSyncDb(syncDb.GetSyncDbUri()))
        << "the sync db has to be rebuilt after the profile upgrade";
    syncDb.WithReadOnly([&](ECDbR sync) {
        bvector<Utf8String> columns;
        sync.GetColumns(columns, "ec_Property");
        EXPECT_TRUE(std::find(columns.begin(), columns.end(), Utf8String("SimulatedProfileColumn")) != columns.end())
            << "the mirror did not carry the upgraded ec_Property shape";
        EXPECT_EQ(upgradedVersion.GetSub2(), sync.GetECDbProfileVersion().GetSub2())
            << "the sync db kept the old profile version";
    });

    b2->PullMergePush("push the profile upgrade");
    b1->PullMergePush("pick up the profile upgrade");
    auto b3 = hub.CreateBriefcase();

    bvector<Utf8String> columns;
    b3->GetColumns(columns, "ec_Property");
    EXPECT_TRUE(std::find(columns.begin(), columns.end(), Utf8String("SimulatedProfileColumn")) != columns.end())
        << "the timeline briefcase did not replay the widened ec_Property table";
    EXPECT_EQ(upgradedVersion.GetSub2(), b3->GetECDbProfileVersion().GetSub2())
        << "the timeline briefcase kept the old profile version";

    ExpectCensusPreserved(beforeImporter, InstanceCensus::Take(*b2), "profile-upgrade importer");
    ExpectCensusPreserved(beforePuller, InstanceCensus::Take(*b1), "profile-upgrade puller");
    ExpectCensusPreserved(beforeImporter, InstanceCensus::Take(*b3), "profile-upgrade timeline briefcase");
    ExpectECTablesIdentical(*b3, *b2, "timeline briefcase compared with profile-upgrade importer");
    ExpectPhysicalSchemaIdentical(*b3, *b2, "timeline briefcase compared with profile-upgrade importer");
    VerifyFileIsSound(*b2, "profile-upgrade importer");
    VerifyFileIsSound(*b1, "profile-upgrade puller");
    VerifyFileIsSound(*b3, "profile-upgrade timeline briefcase");
    }
// ---------------------------------------------------------------------------------------
// The stamp that says which sync db state a txn was produced against has to sit in the same txn as
// the rows it describes. A rebase replays txn by txn, so a txn holding ec_ rows and no stamp reads
// as dataVer 0 and yields to whatever arrived. MakeChangeset merges local changesets before they
// are pushed, so this is only visible before the push.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, TheDataVersionStampSitsInTheTxnHoldingTheRowsItDescribes)
    {
    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-extended-datever-same-txn");
    std::unique_ptr<TrackedECDb> b1, b2;
    SetupSyncedPair(hub, syncDb, b1, b2);

    auto& sync2 = b2->Schemas().GetSchemaSync();
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(),
        LoadSchemas(*b2, { SharedColumnSchema("01.00.00", 2) }).Refs(), SchemaManager::SchemaImportOptions::None));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    auto* tracker = b2->GetTracker();
    ASSERT_TRUE(tracker != nullptr);
    const auto localChangesets = tracker->GetLocalChangesets();
    ASSERT_FALSE(localChangesets.empty()) << "the import produced no local changeset to inspect";

    int changesetsCarryingRows = 0;
    for (auto const* localChangeset : localChangesets)
        {
        // GetChanges is non-const, and the tracker hands out const pointers, so walk a copy.
        auto copy = localChangeset->Clone();
        ASSERT_TRUE(copy != nullptr);

        bool holdsECRows = false;
        bool holdsStamp = false;
        SchemaSync::DataVer stampedVersion = 0;
        for (auto& change : copy->GetChanges())
            {
            if (change.GetTableName().StartsWithIAscii("ec_"))
                holdsECRows = true;
            if (SchemaSync::IsLocalDbInfoChange(change) && SchemaSync::TryGetDataVersion(stampedVersion, change))
                holdsStamp = true;
            }

        if (!holdsECRows)
            continue;

        ++changesetsCarryingRows;
        EXPECT_TRUE(holdsStamp)
            << "local changeset " << localChangeset->GetIndex() << " (" << localChangeset->GetOperation().c_str()
            << ") carries ec_ rows with no localDbInfo stamp, so a rebase replaying it would read dataVer 0";
        EXPECT_NE(0, stampedVersion)
            << "local changeset " << localChangeset->GetIndex() << " stamped dataVer 0, which never supersedes anything";
        }
    EXPECT_GT(changesetsCarryingRows, 0) << "no local changeset carried ec_ rows, so this test checked nothing";

    // The stamp has to describe the state the rows actually came from. The sync db is not a schema
    // sync client of itself, so its own version is read through SyncDbInfo rather than GetSchemaSync.
    EXPECT_EQ(SchemaSync::SyncDbInfo::From(syncDb.GetSyncDbUri()).GetDataVersion(), sync2.GetInfo().GetDataVersion())
        << "the briefcase and the sync db disagree about the version the briefcase just adopted";

    b2->PullMergePush("push the stamped import");
    b1->PullMergePush("pick up the stamped import");
    ExpectECTablesIdentical(*b1, *b2, "after a stamped import reached the other briefcase");
    }

// ---------------------------------------------------------------------------------------
// The three-briefcase matrix walks every permutation. Beyond three the round count grows
// factorially, so this takes a few fixed orderings instead and spends the budget on width: with
// five briefcases and three schema shapes, two of them necessarily import the same shape at the
// same time, which three briefcases never do.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, FiveBriefcasesConvergeAcrossAFewPushOrderings)
    {
    const int briefcaseCount = 5;

    // In import order, reversed, and odds before evens - enough to separate "converges" from
    // "converges only when the pushes happen to line up with the imports".
    struct { Utf8CP m_label; std::vector<int> m_pushOrder; } const orderings[] = {
        { "in import order",   { 0, 1, 2, 3, 4 } },
        { "reversed",          { 4, 3, 2, 1, 0 } },
        { "odds before evens", { 1, 3, 0, 2, 4 } },
    };

    int round = 0;
    for (auto const& ordering : orderings)
        {
        ++round;
        const Utf8PrintfString context("five briefcases, push %s", ordering.m_label);

        ECDbHub hub;
        SchemaSyncDb syncDb(Utf8PrintfString("upstream-wide-matrix-%d", round).c_str());
        std::unique_ptr<TrackedECDb> seed, unused;
        SetupSyncedPair(hub, syncDb, seed, unused);

        SeedThroughSyncDb(*seed, syncDb, MatrixBaselineSchema(), context.c_str());
        if (CurrentTestHasFailed())
            return;

        std::vector<std::unique_ptr<TrackedECDb>> briefcases;
        briefcases.push_back(std::move(seed));
        for (int i = 1; i < briefcaseCount; ++i)
            briefcases.push_back(hub.CreateBriefcase());

        std::vector<InstanceCensus> before;
        for (size_t i = 0; i < briefcases.size(); ++i)
            {
            InsertMatrixRow(*briefcases[i], Utf8PrintfString("%s: bc%d", context.c_str(), (int)i).c_str());
            before.push_back(InstanceCensus::Take(*briefcases[i]));
            }
        if (CurrentTestHasFailed())
            return;

        const auto moves = MatrixMoves(round);
        for (int i = 0; i < briefcaseCount; ++i)
            {
            auto& bc = *briefcases[i];
            const auto& move = moves[i % (int)moves.size()];
            const auto loaded = LoadSchemas(bc, { move.m_schema });
            ASSERT_TRUE(loaded.IsValid()) << context.c_str() << ": could not load " << move.m_label;
            ASSERT_EQ(SchemaSync::Status::OK,
                      bc.Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), loaded.Refs(), SchemaManager::SchemaImportOptions::None))
                << context.c_str() << ": briefcase " << i << " was refused " << move.m_label;
            ASSERT_EQ(BE_SQLITE_OK, bc.SaveChanges());
            }

        for (int idx : ordering.m_pushOrder)
            ASSERT_EQ(BE_SQLITE_OK, briefcases[idx]->PullMergePush(Utf8PrintfString("%s: briefcase %d", context.c_str(), idx).c_str()))
                << context.c_str() << ": briefcase " << idx << " could not push";
        for (size_t i = 0; i < briefcases.size(); ++i)
            ASSERT_EQ(BE_SQLITE_OK, briefcases[i]->PullMergePush("catch up"))
                << context.c_str() << ": briefcase " << (int)i << " could not catch up";

        std::vector<TrackedECDb*> raw;
        for (auto& bc : briefcases)
            raw.push_back(bc.get());
        ExpectAllConverged(raw, before, context.c_str());

        VerifySchemaSyncRules(syncDb, std::vector<ECDb*>(raw.begin(), raw.end()), context.c_str());

        // A briefcase that saw none of it, built from the finished timeline.
        auto latecomer = hub.CreateBriefcase();
        ExpectECTablesIdentical(*latecomer, *briefcases[0], Utf8PrintfString("%s: latecomer", context.c_str()).c_str());
        ExpectPhysicalSchemaIdentical(*latecomer, *briefcases[0], Utf8PrintfString("%s: latecomer", context.c_str()).c_str());

        if (CurrentTestHasFailed())
            return;
        }
    }

// ---------------------------------------------------------------------------------------
// Every briefcase imports the identical schema at the same time. The sync db decides once and the
// other seven adopt what it decided, so seven of the eight imports are no-ops there - the case
// where there is no delta anywhere for anyone to replay, at width.
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportExtendedTests, EightBriefcasesImportingOneSchemaTogetherConverge)
    {
    const int briefcaseCount = 8;

    ECDbHub hub;
    SchemaSyncDb syncDb("upstream-wide-identical-import");
    std::unique_ptr<TrackedECDb> seed, unused;
    SetupSyncedPair(hub, syncDb, seed, unused);

    // Derived with one property, narrow enough to sit in the shared columns. The eight-way import
    // below takes it to six, past the four-column limit and into overflow.
    SeedThroughSyncDb(*seed, syncDb, SharedColumnSchema("01.00.00", 1), "eight-way identical import");
    if (CurrentTestHasFailed())
        return;

    std::vector<std::unique_ptr<TrackedECDb>> briefcases;
    briefcases.push_back(std::move(seed));
    for (int i = 1; i < briefcaseCount; ++i)
        briefcases.push_back(hub.CreateBriefcase());

    std::vector<InstanceCensus> before;
    for (size_t i = 0; i < briefcases.size(); ++i)
        {
        InsertDerivedRow(*briefcases[i], Utf8PrintfString("bc%d", (int)i).c_str());
        before.push_back(InstanceCensus::Take(*briefcases[i]));
        }
    if (CurrentTestHasFailed())
        return;

    const auto schema = SharedColumnSchema("01.00.01", 6);
    for (int i = 0; i < briefcaseCount; ++i)
        {
        auto& bc = *briefcases[i];
        const auto loaded = LoadSchemas(bc, { schema });
        ASSERT_TRUE(loaded.IsValid());
        ASSERT_EQ(SchemaSync::Status::OK,
                  bc.Schemas().GetSchemaSync().ImportSchemas(syncDb.GetSyncDbUri(), loaded.Refs(), SchemaManager::SchemaImportOptions::None))
            << "briefcase " << i << " was refused a schema the sync db had already decided";
        ASSERT_EQ(BE_SQLITE_OK, bc.SaveChanges());
        }

    // If this stops holding the census below stops testing the overflow catch-up, so say so here
    // rather than let the test quietly weaken.
    ASSERT_STRNE(TableOf(*briefcases[0], "UpstreamTest", "Derived", "p1").c_str(),
                 TableOf(*briefcases[0], "UpstreamTest", "Derived", "p6").c_str())
        << "the widened schema did not spill into overflow";

    for (int i = briefcaseCount - 1; i >= 0; --i)
        ASSERT_EQ(BE_SQLITE_OK, briefcases[i]->PullMergePush(Utf8PrintfString("eight-way identical import: briefcase %d", i).c_str()))
            << "briefcase " << i << " could not push";
    for (size_t i = 0; i < briefcases.size(); ++i)
        ASSERT_EQ(BE_SQLITE_OK, briefcases[i]->PullMergePush("catch up"))
            << "briefcase " << (int)i << " could not catch up";

    std::vector<TrackedECDb*> raw;
    for (auto& bc : briefcases)
        raw.push_back(bc.get());
    ExpectAllConverged(raw, before, "eight briefcases importing one schema together");

    VerifySchemaSyncRules(syncDb, std::vector<ECDb*>(raw.begin(), raw.end()), "eight briefcases importing one schema together");

    // One shared-column pool, one decision: every briefcase has to agree on where each property went.
    for (int i = 1; i < briefcaseCount; ++i)
        {
        for (int prop = 1; prop <= 6; ++prop)
            {
            const Utf8PrintfString accessString("p%d", prop);
            EXPECT_STREQ(ColumnOf(*briefcases[0], "UpstreamTest", "Derived", accessString.c_str()).c_str(),
                         ColumnOf(*briefcases[i], "UpstreamTest", "Derived", accessString.c_str()).c_str())
                << "briefcase " << i << " put " << accessString.c_str() << " in a different column than briefcase 0";
            }
        }
    }
END_ECDBUNITTESTS_NAMESPACE
