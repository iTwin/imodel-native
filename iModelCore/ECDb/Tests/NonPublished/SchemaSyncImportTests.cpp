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

// SchemaSyncTest.cpp covers what the sync db does with a schema once it is there. This file covers
// how it gets there: the two steps of an import, the entry points that drive them, and whether two
// briefcases end up with the same ec_ rows and the same physical schema.
struct SchemaSyncImportTestFixture : SchemaSyncTestFixture {};

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
// last briefcase to import wins there. The timeline resolves them by push order: a rebase skips an
// ec_ row whose incoming counterpart is already present, on both ConflictCause::Data and
// ConflictCause::Conflict, so the first briefcase to push wins. The two agree only when push order
// is the reverse of import order - which nothing enforces, since an update takes only a shared lock.
//
// The two reversed-order tests below are the case where they agree. The two matching-order tests
// and the stale-overwrite test are the case where they do not, and are disabled: they characterise
// the gap rather than a fix.
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

// disabled failing tests by design until we resolve the concurrent edits racing scenario
#if 0
// ---------------------------------------------------------------------------------------
// @bsitest
// +---------------+---------------+---------------+---------------+---------------+------
TEST_F(SchemaSyncImportTestFixture, ConcurrentLabelEditsInPushOrderMatchingImportOrder)
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
TEST_F(SchemaSyncImportTestFixture, ConcurrentEditsToANewClassInPushOrderMatchingImportOrder)
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
TEST_F(SchemaSyncImportTestFixture, StaleBriefcaseWritesItsOlderSchemaBackIntoTheSyncDb)
    {
    ConcurrentEditScenario scenario("upstream-stale-overwrite");
    scenario.Start({ MetadataOnlySchema("01.00.00", "before anybody edited it"), RemapSchema("01.00.00", false) });

    scenario.ImportConcurrently(MetadataOnlySchema("01.00.01", "relabelled by the first importer"),
                                MetadataOnlySchema("01.00.02", "relabelled by the second importer"));
    // Pushing in import order is what leaves the first importer holding 1.0.1 after the exchange.
    scenario.Exchange(PushOrder::ImportOrder);

    // Read rather than asserted, so this test still says something once the exchange is fixed.
    auto& stale = *scenario.m_firstImporter;
    const auto staleLabel = DisplayLabelOf(stale, "LabelTest", "Existing");

    // Any upgrade will do; this one moves data, so it runs on the briefcase.
    ASSERT_EQ(SchemaImportResult::OK,
              ImportSchema(stale, RemapSchema("01.00.01", true), SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade, scenario.m_syncDb.GetSyncDbUri()));
    ASSERT_EQ(BE_SQLITE_OK, stale.SaveChanges());

    scenario.m_syncDb.WithReadOnly([&](ECDbR sync) {
        EXPECT_STREQ("1.0.2", VersionOf(sync, "LabelTest").c_str())
            << "an upgrade run from a briefcase that never received 1.0.2 rolled the sync db back to " << VersionOf(sync, "LabelTest").c_str();
        EXPECT_STREQ("relabelled by the second importer", DisplayLabelOf(sync, "LabelTest", "Existing").c_str())
            << "the overwrite replaced the sync db's label with the stale one the briefcase held (" << staleLabel.c_str() << ")";
    });
    }
#endif

END_ECDBUNITTESTS_NAMESPACE
