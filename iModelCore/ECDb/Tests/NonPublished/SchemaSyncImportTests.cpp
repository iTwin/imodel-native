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
struct SchemaSyncImportExtendedTests : SchemaSyncImportTestFixture {};

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

// Stands in for the backend's post-merge hook. A merged changeset carries ec_ rows but no DDL.
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
            MaterializeAfterMerge(*other);
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
        MaterializeAfterMerge(*trails);
        leads->PullMergePush("and back, this time with nothing local");
        MaterializeAfterMerge(*leads);
        m_bystander->PullMergePush("a briefcase that imported nothing");
        MaterializeAfterMerge(*m_bystander);
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
    MaterializeAfterMerge(*b2);
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
    MaterializeAfterMerge(*b2);

    ImportThroughSyncDb(*b1, syncDb, { MachinerySchema("01.00.01", true) }, { "Machinery" });
    ImportThroughSyncDb(*b2, syncDb, { TankSchema() }, { "DemoB" });

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
    MaterializeAfterMerge(*b2);
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
    printf("[schemasync-test] p1 -> %s, p8 -> %s\n", primaryTable.c_str(), overflowTable.c_str());
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
    MaterializeAfterMerge(*b2);

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
    MaterializeAfterMerge(*b2);
    b1->PullMergePush("b1 merges tank");
    MaterializeAfterMerge(*b1);

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
    MaterializeAfterMerge(*b2);
    EXPECT_STREQ("1.0.2", VersionOf(*b2, "UpstreamTest").c_str()) << "b2 did not converge on the surviving version";

    // 4) A briefcase built fresh from the timeline holds 1.0.2, with both properties.
    auto b3 = hub.CreateBriefcase();
    MaterializeAfterMerge(*b3);
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
    MaterializeAfterMerge(*b2);

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
    MaterializeAfterMerge(*b2);

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

    for (auto* briefcase : { b1.get(), b2.get() }) {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*briefcase, "INSERT INTO rmp.LeafA(baseProp,filler,movingProp) VALUES('a','f','after the move')"));
        ECInstanceKey key;
        EXPECT_EQ(BE_SQLITE_DONE, stmt.Step(key)) << "writing through the consolidated column failed";
        ASSERT_EQ(BE_SQLITE_OK, briefcase->SaveChanges());
    }
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
    MaterializeAfterMerge(*b2);
    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b2, SiblingSlotSchema("01.00.01", true), SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());

    EXPECT_STREQ(firstColumn.c_str(), ColumnOf(*b2, "SlotTest", "First", "firstProp").c_str())
        << "the property that was already mapped moved";
    EXPECT_STREQ(firstColumn.c_str(), ColumnOf(*b2, "SlotTest", "Second", "secondProp").c_str())
        << "the sibling was given a slot of its own instead of reusing the one next to it";

    b2->PullMergePush("SlotTest 1.0.1");
    b1->PullMergePush("pick up SlotTest 1.0.1");
    MaterializeAfterMerge(*b1);

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
    MaterializeAfterMerge(*b2);
    EXPECT_TRUE(HasTrigger(*b2, triggerName)) << "a briefcase that merged the schema changeset has no trigger";

    auto b3 = hub.CreateBriefcase();
    MaterializeAfterMerge(*b3);
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
    MaterializeAfterMerge(*b2);
    ASSERT_EQ(SchemaImportResult::OK, ImportSchema(*b2, UnitsAndFormatsSchema(), SchemaManager::SchemaImportOptions::None, syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("b2 adds UnitTest");

    b1->PullMergePush("b1 picks up UnitTest");
    MaterializeAfterMerge(*b1);

    auto b3 = hub.CreateBriefcase();
    MaterializeAfterMerge(*b3);

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
    MaterializeAfterMerge(*b1);

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
    MaterializeAfterMerge(*b2);

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
    MaterializeAfterMerge(*b2);
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
    MaterializeAfterMerge(*b1);
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
    MaterializeAfterMerge(*b1);

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
    MaterializeAfterMerge(*b1);

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
    MaterializeAfterMerge(*b1);

    ExpectCensusPreserved(before, InstanceCensus::Take(*b1), "the briefcase whose unpushed rows were rebased");
    VerifyFileIsSound(*b1, "rebased briefcase after the spill");

    // The overflow rows have to be in what b1 pushed. A briefcase that only receives the rebased
    // changeset gets a pure data changeset and runs no catch-up of its own.
    b2->PullMergePush("pick up the rebased rows");
    MaterializeAfterMerge(*b2);
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
    MaterializeAfterMerge(*b1);

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
    MaterializeAfterMerge(*b1);

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
    MaterializeAfterMerge(*b1);

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
    MaterializeAfterMerge(*b1);

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
    MaterializeAfterMerge(*b1);

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
    MaterializeAfterMerge(*b1);
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
    MaterializeAfterMerge(*b1);

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
    MaterializeAfterMerge(*b1);
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
    MaterializeAfterMerge(*b1);

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
    MaterializeAfterMerge(*b1);
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
    MaterializeAfterMerge(*b1);

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
    MaterializeAfterMerge(*b1);
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
    MaterializeAfterMerge(*b1);

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
    MaterializeAfterMerge(*b1);
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
    MaterializeAfterMerge(*b1);

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
    MaterializeAfterMerge(*b1);
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
    MaterializeAfterMerge(*b1);

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
    MaterializeAfterMerge(*b1);
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
    for (size_t i = 0; i < briefcases.size(); ++i) {
        const Utf8PrintfString where("%s: briefcase %d", context, (int)i);
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

            std::vector<std::unique_ptr<TrackedECDb>> briefcases;
            briefcases.push_back(std::move(seed));
            for (int i = 1; i < briefcaseCount; ++i)
                briefcases.push_back(hub.CreateBriefcase());

            // Something to lose, before anybody changes the schema.
            std::vector<InstanceCensus> before;
            for (auto& bc : briefcases)
                before.push_back(InstanceCensus::Take(*bc));

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
                briefcases[idx]->PullMergePush(Utf8PrintfString("%s: briefcase %d", context.c_str(), idx).c_str());
            }

            // Everyone catches up, then materialises what the changesets described.
            for (auto& bc : briefcases) {
                bc->PullMergePush("catch up");
                MaterializeAfterMerge(*bc);
            }

            std::vector<TrackedECDb*> raw;
            for (auto& bc : briefcases)
                raw.push_back(bc.get());
            ExpectAllConverged(raw, before, context.c_str());

            syncDb.WithReadOnly([&](ECDbR sync) { VerifyFileIsSound(sync, context.c_str()); });
            // One broken ordering is enough to look at; the rest would repeat it. HasFailure() is a
            // member of ::testing::Test, so it is available directly in a TEST_F body.
            if (HasFailure())
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
        MaterializeAfterMerge(*b2);
        b3->PullMergePush("b3 catches up");
        MaterializeAfterMerge(*b3);
        b1->PullMergePush("b1 catches up");
        MaterializeAfterMerge(*b1);

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
    MaterializeAfterMerge(*b1);

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
    MaterializeAfterMerge(*b1);

    {
    bvector<Utf8String> columns;
    b1->GetColumns(columns, "ec_Property");
    EXPECT_TRUE(std::find(columns.begin(), columns.end(), Utf8String("SimulatedProfileColumn")) != columns.end())
        << "the other briefcase never got the widened ec_Property";
    }
    EXPECT_EQ(upgradedVersion.GetSub2(), b1->GetECDbProfileVersion().GetSub2())
        << "the other briefcase kept the old EC profile version";

    // Everything still has to work across the upgraded pair, and nobody may have lost data.
    ASSERT_EQ(SchemaSync::Status::OK, sync2.ImportSchemas(syncDb.GetSyncDbUri(), LoadSchemas(*b2, { CensusSchema("01.00.01", true) }).Refs(), SchemaManager::SchemaImportOptions::None))
        << "an ordinary update stopped working after the profile upgrade";
    ASSERT_EQ(BE_SQLITE_OK, b2->SaveChanges());
    b2->PullMergePush("add properties after the profile upgrade");
    b1->PullMergePush("pick up properties after the profile upgrade");
    MaterializeAfterMerge(*b1);

    ExpectCensusPreserved(beforeImporter, InstanceCensus::Take(*b2), "importer across the profile upgrade");
    ExpectCensusPreserved(beforePuller, InstanceCensus::Take(*b1), "puller across the profile upgrade");
    ExpectECTablesIdentical(*b1, *b2, "after the profile upgrade");
    VerifyFileIsSound(*b2, "importer after the profile upgrade");
    VerifyFileIsSound(*b1, "puller after the profile upgrade");
    }

END_ECDBUNITTESTS_NAMESPACE
