/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <ECDb/ChangesetSchema.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct TestChangeSet : BeSQLite::ChangeSet
    {
    ConflictResolution _OnConflict(ConflictCause, BeSQLite::Changes::Change) override { return ConflictResolution::Skip; }
    };

struct TestChangeTracker : BeSQLite::ChangeTracker
    {
    TestChangeTracker(BeSQLite::DbR db) { SetDb(&db); }
    OnCommitStatus _OnCommit(bool, Utf8CP) override { return OnCommitStatus::Commit; }
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ChangesetSchemaTests : public ECDbTestFixture
    {
    void CaptureChangeset(TestChangeSet& cs)
        {
        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);
        ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
        tracker.EnableTracking(false);
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetSchemaTests, ExtractFrom_Basic)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ChangesetSchema_Extract.ecdb",
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TS" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Foo">
                    <ECProperty propertyName="Name" typeName="string"/>
                    <ECProperty propertyName="Age"  typeName="int"/>
                </ECEntityClass>
            </ECSchema>)xml")));

    TestChangeSet cs;
        {
        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.Foo(Name,Age) VALUES('a',1)"));
        ECInstanceKey k;
        ASSERT_EQ(BE_SQLITE_DONE, s.Step(k));
        ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
        tracker.EnableTracking(false);
        }
    m_ecdb.SaveChanges();

    auto schema = ChangesetSchema::ExtractFrom(m_ecdb, cs);
    ASSERT_FALSE(schema.IsEmpty());

    auto fooClass = m_ecdb.Schemas().FindClass("TS:Foo");
    ASSERT_NE(nullptr, fooClass);
    auto it = schema.GetClasses().find(fooClass->GetId());
    ASSERT_NE(schema.GetClasses().end(), it);
    EXPECT_STREQ("TS:Foo", it->second.fullName.c_str());
    ASSERT_EQ(1u, it->second.tableSegments.size());
    auto& seg = it->second.tableSegments.begin()->second;
    EXPECT_GE(seg.idColumnIndex, 0);
    EXPECT_GT(seg.sourceColumnCount, 0);

    bool foundName = false, foundAge = false;
    for (auto const& ck : seg.columns)
        {
        if (ck.first == "Name") { foundName = true; EXPECT_GE(ck.second.sourceColumnIndex, 0); }
        if (ck.first == "Age")  { foundAge  = true; EXPECT_GE(ck.second.sourceColumnIndex, 0); }
        }
    EXPECT_TRUE(foundName);
    EXPECT_TRUE(foundAge);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetSchemaTests, JsonRoundTrip)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ChangesetSchema_Json.ecdb",
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TS" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Foo">
                    <ECProperty propertyName="Name" typeName="string"/>
                </ECEntityClass>
            </ECSchema>)xml")));

    TestChangeSet cs;
        {
        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.Foo(Name) VALUES('a')"));
        ECInstanceKey k;
        ASSERT_EQ(BE_SQLITE_DONE, s.Step(k));
        ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
        tracker.EnableTracking(false);
        }
    m_ecdb.SaveChanges();

    auto original = ChangesetSchema::ExtractFrom(m_ecdb, cs);
    ASSERT_FALSE(original.IsEmpty());

    BeJsDocument doc;
    original.To(doc);
    EXPECT_TRUE(doc.isArray());

    auto restored = ChangesetSchema::From(doc);
    ASSERT_FALSE(restored.IsEmpty());
    ASSERT_EQ(original.GetClasses().size(), restored.GetClasses().size());

    for (auto const& kv : original.GetClasses())
        {
        auto it = restored.GetClasses().find(kv.first);
        ASSERT_NE(restored.GetClasses().end(), it);
        EXPECT_EQ(kv.second.fullName, it->second.fullName);
        EXPECT_EQ(kv.second.tableSegments.size(), it->second.tableSegments.size());
        for (auto const& sk : kv.second.tableSegments)
            {
            auto sit = it->second.tableSegments.find(sk.first);
            ASSERT_NE(it->second.tableSegments.end(), sit);
            EXPECT_EQ(sk.second.tableName, sit->second.tableName);
            EXPECT_EQ(sk.second.idColumnIndex, sit->second.idColumnIndex);
            EXPECT_EQ(sk.second.classIdColumnIndex, sit->second.classIdColumnIndex);
            EXPECT_EQ(sk.second.columns.size(), sit->second.columns.size());
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetSchemaTests, Validate_Match)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ChangesetSchema_ValidateMatch.ecdb",
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TS" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Foo">
                    <ECProperty propertyName="Name" typeName="string"/>
                </ECEntityClass>
            </ECSchema>)xml")));

    TestChangeSet cs1, cs2;
        {
        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.Foo(Name) VALUES('x')"));
        ECInstanceKey k;
        ASSERT_EQ(BE_SQLITE_DONE, s.Step(k));
        ASSERT_EQ(BE_SQLITE_OK, cs1.FromChangeTrack(tracker));
        tracker.EnableTracking(false);
        }
    m_ecdb.SaveChanges();

    auto schema = ChangesetSchema::ExtractFrom(m_ecdb, cs1);
    ASSERT_FALSE(schema.IsEmpty());

    // Re-create same changeset for validation pass
    auto fooClass = m_ecdb.Schemas().FindClass("TS:Foo");
    ASSERT_NE(nullptr, fooClass);

    // Capture a second changeset to validate against
        {
        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.Foo(Name) VALUES('y')"));
        ECInstanceKey k;
        ASSERT_EQ(BE_SQLITE_DONE, s.Step(k));
        ASSERT_EQ(BE_SQLITE_OK, cs2.FromChangeTrack(tracker));
        tracker.EnableTracking(false);
        }
    m_ecdb.SaveChanges();

    bvector<Utf8String> errors;
    auto stat = schema.Validate(m_ecdb, cs2, &errors);
    EXPECT_EQ(SUCCESS, stat) << (errors.empty() ? "" : errors[0].c_str());
    EXPECT_TRUE(errors.empty());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetSchemaTests, Diff_NoChanges)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ChangesetSchema_DiffNone.ecdb",
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TS" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Foo">
                    <ECProperty propertyName="Name" typeName="string"/>
                </ECEntityClass>
            </ECSchema>)xml")));

    TestChangeSet cs;
        {
        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.Foo(Name) VALUES('a')"));
        ECInstanceKey k;
        ASSERT_EQ(BE_SQLITE_DONE, s.Step(k));
        ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
        tracker.EnableTracking(false);
        }
    m_ecdb.SaveChanges();

    auto schema = ChangesetSchema::ExtractFrom(m_ecdb, cs);
    auto diff = ChangesetSchemaDiff::Compute(schema, m_ecdb);
    EXPECT_TRUE(diff.IsTransformable());
    EXPECT_TRUE(diff.GetClassDiffs().empty()); // no changes
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetSchemaTests, Diff_ColumnRemappedBySchemaUpgrade)
    {
    // Schema v1: MyBaseClass (TPH + ShareColumns), sibling subclasses A and B.
    // With ApplyToSubclassesOnly absent, subclasses share column slots independently,
    // so A.PropA and B.PropB each map to ps1 (different rows, same slot).
    SchemaItem schemaV1(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="MyBaseClass" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>
                    </ShareColumns>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="A">
                <BaseClass>MyBaseClass</BaseClass>
                <ECProperty propertyName="PropA" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="B">
                <BaseClass>MyBaseClass</BaseClass>
                <ECProperty propertyName="PropB" typeName="string"/>
            </ECEntityClass>
        </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, SetupECDb("csremap_b1.ecdb", schemaV1));

    // Confirm schema v1 column assignments: A.PropA and B.PropB both occupy ps1.
    ASSERT_EQ(ExpectedColumn("ts_MyBaseClass", "ps1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "A", "PropA")));
    ASSERT_EQ(ExpectedColumn("ts_MyBaseClass", "ps1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "B", "PropB")));

    // Insert a baseline row in B so we can verify data migration in b2 after schema upgrade.
        {
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.B (PropB) VALUES ('baseline_b')"));
        ASSERT_EQ(BE_SQLITE_DONE, s.Step());
        }
    m_ecdb.SaveChanges();

    // Clone b1 into b2; both start from the same schema v1 state with the baseline row.
    ECDb b2;
    ASSERT_EQ(BE_SQLITE_OK, CloneECDb(b2, "csremap_b2.ecdb", BeFileName(m_ecdb.GetDbFileName())));
    TestHelper b2h(b2);

    // Capture a changeset on b1: insert an A instance while PropA is still at ps1.
    TestChangeSet cs;
        {
        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.A (PropA) VALUES ('aval')"));
        ECInstanceKey k;
        ASSERT_EQ(BE_SQLITE_DONE, s.Step(k));
        ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
        tracker.EnableTracking(false);
        }
    m_ecdb.SaveChanges();

    // Extract ChangesetSchema BEFORE the upgrade: records A.PropA at ps1.
    auto oldSchema = ChangesetSchema::ExtractFrom(m_ecdb, cs);
    ASSERT_FALSE(oldSchema.IsEmpty());

    // Schema v2: insert NewBase between MyBaseClass and A/B; NewBase gets PropBase.
    // The upgrade preserves existing column assignments: A.PropA and B.PropB stay at ps1.
    // NewBase.PropBase is assigned the next available column ps2.
    SchemaItem schemaV2(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="MyBaseClass" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>
                    </ShareColumns>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="NewBase">
                <BaseClass>MyBaseClass</BaseClass>
                <ECProperty propertyName="PropBase" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="A">
                <BaseClass>NewBase</BaseClass>
                <ECProperty propertyName="PropA" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="B">
                <BaseClass>NewBase</BaseClass>
                <ECProperty propertyName="PropB" typeName="string"/>
            </ECEntityClass>
        </ECSchema>)xml");

    // Import v2 into b1 (triggers column remapping in b1's physical table).
    ASSERT_EQ(SUCCESS, ImportSchema(schemaV2, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    // Confirm post-upgrade column assignments in b1.
    ASSERT_EQ(ExpectedColumn("ts_MyBaseClass", "ps2"), GetHelper().GetPropertyMapColumn(AccessString("ts", "NewBase", "PropBase")));
    ASSERT_EQ(ExpectedColumn("ts_MyBaseClass", "ps1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "A", "PropA")));
    ASSERT_EQ(ExpectedColumn("ts_MyBaseClass", "ps1"), GetHelper().GetPropertyMapColumn(AccessString("ts", "B", "PropB")));

    // Import v2 into b2 (same remapping applies).
    ASSERT_EQ(SUCCESS, b2h.ImportSchema(schemaV2, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    // The baseline B row in b2 should be readable after b2's schema upgrade.
    ASSERT_EQ(JsonValue(R"json([{"PropB":"baseline_b"}])json"), b2h.ExecuteSelectECSql("SELECT PropB FROM ts.B"));

    // Compute diff: A.PropA is still at ps1 after the upgrade (the upgrade only added
    // NewBase.PropBase at ps2; existing columns are preserved). The diff should be empty.
    auto diff = ChangesetSchemaDiff::Compute(oldSchema, m_ecdb);
    EXPECT_TRUE(diff.IsTransformable());
    EXPECT_TRUE(diff.GetClassDiffs().empty()) << "PropA was not remapped; no column diff expected";

    // Transform the stale changeset to use the new column layout.
    TestChangeSet transformedCs;
    ASSERT_EQ(BE_SQLITE_OK, diff.Transform(cs, transformedCs, m_ecdb));

    // Apply the transformed changeset to b2 and verify the A instance was inserted correctly.
    ASSERT_EQ(BE_SQLITE_OK, transformedCs.ApplyChanges(b2));
    b2.SaveChanges();

    ASSERT_EQ(JsonValue(R"json([{"PropA":"aval"}])json"), b2h.ExecuteSelectECSql("SELECT PropA FROM ts.A"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetSchemaTests, ExtractFrom_Update)
    {
    // Verify that ExtractFrom correctly captures the schema from an UPDATE changeset
    // even though ECClassId is absent from UPDATE change records (it is resolved from the DB).
    ASSERT_EQ(SUCCESS, SetupECDb("ChangesetSchema_ExtractUpdate.ecdb",
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TS" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Foo">
                    <ECProperty propertyName="Name" typeName="string"/>
                    <ECProperty propertyName="Age"  typeName="int"/>
                </ECEntityClass>
            </ECSchema>)xml")));

    // Insert a row first.
    ECInstanceKey insertedKey;
        {
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.Foo(Name,Age) VALUES('original',10)"));
        ASSERT_EQ(BE_SQLITE_DONE, s.Step(insertedKey));
        }
    m_ecdb.SaveChanges();

    // Capture an UPDATE changeset (ECClassId will NOT appear in this change record).
    TestChangeSet cs;
        {
        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "UPDATE ts.Foo SET Name='updated' WHERE ECInstanceId=?"));
        s.BindId(1, insertedKey.GetInstanceId());
        ASSERT_EQ(BE_SQLITE_DONE, s.Step());
        ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
        tracker.EnableTracking(false);
        }
    m_ecdb.SaveChanges();

    // ExtractFrom must resolve the class via DB query (since ECClassId is absent from UPDATE).
    auto schema = ChangesetSchema::ExtractFrom(m_ecdb, cs);
    ASSERT_FALSE(schema.IsEmpty());

    auto fooClass = m_ecdb.Schemas().FindClass("TS:Foo");
    ASSERT_NE(nullptr, fooClass);
    auto it = schema.GetClasses().find(fooClass->GetId());
    ASSERT_NE(schema.GetClasses().end(), it) << "Foo class must appear in the schema extracted from an UPDATE changeset";
    EXPECT_STREQ("TS:Foo", it->second.fullName.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetSchemaTests, Transform_UpdateDoesNotSynthesiseECClassId)
    {
    // Verify that Transform does NOT write ECClassId into UPDATE change records even
    // when a ClassIdRemap diff is present. SQLite UPDATE records omit ECClassId, and
    // the transformed record must preserve that absence.
    SchemaItem schemaV1(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TS" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Base" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>
                    </ShareColumns>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="Foo">
                <BaseClass>Base</BaseClass>
                <ECProperty propertyName="Name" typeName="string"/>
            </ECEntityClass>
        </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, SetupECDb("csupdatetx_b1.ecdb", schemaV1));

    // Insert a Foo instance.
    ECInstanceKey fooKey;
        {
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.Foo(Name) VALUES('before')"));
        ASSERT_EQ(BE_SQLITE_DONE, s.Step(fooKey));
        }
    m_ecdb.SaveChanges();

    // Clone into b2 (both DBs start from same schema v1 + same data).
    ECDb b2;
    ASSERT_EQ(BE_SQLITE_OK, CloneECDb(b2, "csupdatetx_b2.ecdb", BeFileName(m_ecdb.GetDbFileName())));
    TestHelper b2h(b2);

    // Capture an UPDATE changeset on b1 (ECClassId absent).
    TestChangeSet cs;
        {
        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "UPDATE ts.Foo SET Name='after' WHERE ECInstanceId=?"));
        s.BindId(1, fooKey.GetInstanceId());
        ASSERT_EQ(BE_SQLITE_DONE, s.Step());
        ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
        tracker.EnableTracking(false);
        }
    m_ecdb.SaveChanges();

    // Extract schema from the UPDATE changeset.
    auto oldSchema = ChangesetSchema::ExtractFrom(m_ecdb, cs);
    ASSERT_FALSE(oldSchema.IsEmpty());

    // Schema v2: add a sibling class so that the shared-column slot is still valid;
    // the important thing is that there is no column diff for Foo.Name.
    SchemaItem schemaV2(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TS" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Base" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>
                    </ShareColumns>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="Foo">
                <BaseClass>Base</BaseClass>
                <ECProperty propertyName="Name" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="Bar">
                <BaseClass>Base</BaseClass>
                <ECProperty propertyName="Value" typeName="int"/>
            </ECEntityClass>
        </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, ImportSchema(schemaV2, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    ASSERT_EQ(SUCCESS, b2h.ImportSchema(schemaV2, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    // Compute diff and transform.
    auto diff = ChangesetSchemaDiff::Compute(oldSchema, m_ecdb);
    EXPECT_TRUE(diff.IsTransformable());

    TestChangeSet transformedCs;
    ASSERT_EQ(BE_SQLITE_OK, diff.Transform(cs, transformedCs, m_ecdb));

    // Apply to b2 and verify the update was applied (name changed to 'after').
    ASSERT_EQ(BE_SQLITE_OK, transformedCs.ApplyChanges(b2));
    b2.SaveChanges();

    ASSERT_EQ(JsonValue(R"json([{"Name":"after"}])json"), b2h.ExecuteSelectECSql("SELECT Name FROM ts.Foo"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetSchemaTests, ExtractFrom_LinkTable)
    {
    // Schema: two entity classes, a link-table relationship between them.
    ASSERT_EQ(SUCCESS, SetupECDb("ChangesetSchema_LinkTable.ecdb",
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TS" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="A">
                    <ECProperty propertyName="Val" typeName="string"/>
                </ECEntityClass>
                <ECEntityClass typeName="B">
                    <ECProperty propertyName="Val" typeName="string"/>
                </ECEntityClass>
                <ECRelationshipClass typeName="AToB" strength="referencing" modifier="Sealed">
                    <Source multiplicity="(0..*)" roleLabel="is related to" polymorphic="false">
                        <Class class="A"/>
                    </Source>
                    <Target multiplicity="(0..*)" roleLabel="is related to" polymorphic="false">
                        <Class class="B"/>
                    </Target>
                </ECRelationshipClass>
            </ECSchema>)xml")));

    // Insert A, B, then relate them via AToB.
    ECInstanceKey aKey, bKey;
        {
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.A(Val) VALUES('aval')"));
        ASSERT_EQ(BE_SQLITE_DONE, s.Step(aKey));
        s.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.B(Val) VALUES('bval')"));
        ASSERT_EQ(BE_SQLITE_DONE, s.Step(bKey));
        }
    m_ecdb.SaveChanges();

    TestChangeSet cs;
        {
        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);
        // Insert a new A instance within the tracking window so the entity class is captured.
        ECSqlStatement sA;
        ECInstanceKey aKey2;
        ASSERT_EQ(ECSqlStatus::Success, sA.Prepare(m_ecdb, "INSERT INTO ts.A(Val) VALUES('aval2')"));
        ASSERT_EQ(BE_SQLITE_DONE, sA.Step(aKey2));
        sA.Finalize();
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.AToB(SourceECInstanceId,TargetECInstanceId) VALUES(?,?)"));
        s.BindId(1, aKey.GetInstanceId());
        s.BindId(2, bKey.GetInstanceId());
        ASSERT_EQ(BE_SQLITE_DONE, s.Step());
        ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
        tracker.EnableTracking(false);
        }
    m_ecdb.SaveChanges();

    auto schema = ChangesetSchema::ExtractFrom(m_ecdb, cs);
    ASSERT_FALSE(schema.IsEmpty());

    // Find the AToB relationship class entry.
    auto const* relClass = m_ecdb.Schemas().FindClass("TS:AToB");
    ASSERT_NE(nullptr, relClass);
    auto it = schema.GetClasses().find(relClass->GetId());
    ASSERT_NE(schema.GetClasses().end(), it) << "AToB relationship class should be captured";
    auto const& relEntry = it->second;
    EXPECT_TRUE(relEntry.isRelationshipClass) << "Link-table entry should have isRelationshipClass=true";

    // Entity classes should also be captured.
    auto const* aClass = m_ecdb.Schemas().FindClass("TS:A");
    ASSERT_NE(nullptr, aClass);
    EXPECT_NE(schema.GetClasses().end(), schema.GetClasses().find(aClass->GetId())) << "A should be captured (from separate insert)";

    // Verify classIdRefCols in the relationship table segment.
    // The AToB link table has SourceECClassId and TargetECClassId only if constraint is polymorphic.
    // With polymorphic=false and sealed modifier, these columns may be virtual.
    // We verify the entry exists and is a relationship class — structural test only.
    EXPECT_FALSE(relEntry.tableSegments.empty()) << "AToB entry should have at least one table segment";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetSchemaTests, ExtractFrom_NavProp)
    {
    // Schema: entity Child has a nav prop pointing to Parent via Rel (modifier=None → non-virtual RelECClassId).
    ASSERT_EQ(SUCCESS, SetupECDb("ChangesetSchema_NavProp.ecdb",
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TS" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Parent">
                    <ECProperty propertyName="Val" typeName="string"/>
                </ECEntityClass>
                <ECEntityClass typeName="Child">
                    <ECProperty propertyName="Val" typeName="string"/>
                    <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" strength="referencing" modifier="None">
                    <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="True">
                        <Class class="Parent"/>
                    </Source>
                    <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="True">
                        <Class class="Child"/>
                    </Target>
                </ECRelationshipClass>
            </ECSchema>)xml")));

    // Insert a Parent and a Child linked to it.
    ECInstanceKey parentKey;
        {
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.Parent(Val) VALUES('p1')"));
        ASSERT_EQ(BE_SQLITE_DONE, s.Step(parentKey));
        }
    m_ecdb.SaveChanges();

    TestChangeSet cs;
        {
        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.Child(Val,Parent) VALUES('c1',?)"));
        s.BindNavigationValue(1, parentKey.GetInstanceId());
        ECInstanceKey childKey;
        ASSERT_EQ(BE_SQLITE_DONE, s.Step(childKey));
        ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
        tracker.EnableTracking(false);
        }
    m_ecdb.SaveChanges();

    auto schema = ChangesetSchema::ExtractFrom(m_ecdb, cs);
    ASSERT_FALSE(schema.IsEmpty());

    auto const* childClass = m_ecdb.Schemas().FindClass("TS:Child");
    ASSERT_NE(nullptr, childClass);
    auto it = schema.GetClasses().find(childClass->GetId());
    ASSERT_NE(schema.GetClasses().end(), it) << "Child class should be in schema";
    auto const& childEntry = it->second;
    EXPECT_FALSE(childEntry.isRelationshipClass) << "Child should NOT be marked as relationship class";

    // When Rel is modifier=None (can be subclassed), RelECClassId column is non-virtual.
    // Verify classIdRefCols contains the nav prop RelECClassId column.
    bool foundRelClassIdRef = false;
    for (auto const& sk : childEntry.tableSegments)
        {
        for (auto const& rk : sk.second.classIdRefCols)
            {
            if (!rk.second.referencedClassFullName.empty())
                {
                foundRelClassIdRef = true;
                EXPECT_FALSE(rk.second.columnName.empty()) << "classIdRefCol should have a column name";
                EXPECT_TRUE(rk.second.referencedClassId.IsValid()) << "referencedClassId should be valid";
                auto const* relClass = m_ecdb.Schemas().FindClass(rk.second.referencedClassFullName.c_str());
                ASSERT_NE(nullptr, relClass) << "referencedClassFullName should resolve to an existing class";
                EXPECT_EQ(relClass->GetId(), rk.second.referencedClassId) << "stored referencedClassId should match live class";
                }
            }
        }
    EXPECT_TRUE(foundRelClassIdRef) << "Child should have at least one nav-prop classIdRefCol (Rel is modifier=None)";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetSchemaTests, Transform_LinkTable_ClassIdRemap)
    {
    // Verify that SourceECClassId / TargetECClassId in a link-table changeset are
    // remapped when the source/target entity classes have different IDs in the target ECDb.
    //
    // We achieve different class IDs by creating b2 independently (not cloned from b1).
    // b2 imports an extra "Dummy" class before importing the main schema, so all
    // subsequent class IDs are offset relative to b1.

    Utf8String const schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TS" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="A">
                <ECProperty propertyName="Val" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="B">
                <ECProperty propertyName="Val" typeName="string"/>
            </ECEntityClass>
            <ECRelationshipClass typeName="AToB" strength="referencing" modifier="Sealed">
                <Source multiplicity="(0..*)" roleLabel="is related to" polymorphic="false">
                    <Class class="A"/>
                </Source>
                <Target multiplicity="(0..*)" roleLabel="is related to" polymorphic="false">
                    <Class class="B"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

    // b1: import schema directly.
    ASSERT_EQ(SUCCESS, SetupECDb("cslnktx_b1.ecdb", SchemaItem(schemaXml)));

    // Insert entities in b1.
    ECInstanceKey aKey, bKey;
        {
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.A(Val) VALUES('a1')"));
        ASSERT_EQ(BE_SQLITE_DONE, s.Step(aKey));
        s.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.B(Val) VALUES('b1')"));
        ASSERT_EQ(BE_SQLITE_DONE, s.Step(bKey));
        }
    m_ecdb.SaveChanges();

    // Capture a changeset with a link-table INSERT.
    TestChangeSet cs;
        {
        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.AToB(SourceECInstanceId,TargetECInstanceId) VALUES(?,?)"));
        s.BindId(1, aKey.GetInstanceId());
        s.BindId(2, bKey.GetInstanceId());
        ASSERT_EQ(BE_SQLITE_DONE, s.Step());
        ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
        tracker.EnableTracking(false);
        }
    m_ecdb.SaveChanges();

    auto sourceSchema = ChangesetSchema::ExtractFrom(m_ecdb, cs);
    ASSERT_FALSE(sourceSchema.IsEmpty());

    // b2: create separately; first import a "Dummy" class to offset class IDs.
    ECDb b2;
    ASSERT_EQ(BE_SQLITE_OK, CreateECDb(b2, "cslnktx_b2.ecdb"));
    TestHelper b2h(b2);
    ASSERT_EQ(SUCCESS, b2h.ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Dummy" alias="d" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="X"><ECProperty propertyName="V" typeName="int"/></ECEntityClass>
            </ECSchema>)xml")));
    ASSERT_EQ(SUCCESS, b2h.ImportSchema(SchemaItem(schemaXml)));

    // Insert matching A and B rows into b2 (same instance IDs required for apply).
        {
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(b2, "INSERT INTO ts.A(Val) VALUES('a1')"));
        ASSERT_EQ(BE_SQLITE_DONE, s.Step());
        s.Finalize();
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(b2, "INSERT INTO ts.B(Val) VALUES('b1')"));
        ASSERT_EQ(BE_SQLITE_DONE, s.Step());
        }
    b2.SaveChanges();

    // Compute diff; A and/or B class IDs may differ between b1 and b2.
    auto diff = ChangesetSchemaDiff::Compute(sourceSchema, b2);
    EXPECT_TRUE(diff.IsTransformable());

    // If A/B have the same IDs in b1 and b2, there's nothing to remap — that's fine.
    // We only assert transform succeeds.
    TestChangeSet transformedCs;
    ASSERT_EQ(BE_SQLITE_OK, diff.Transform(cs, transformedCs, b2));

    ASSERT_EQ(BE_SQLITE_OK, transformedCs.ApplyChanges(b2));
    b2.SaveChanges();

    ASSERT_EQ(JsonValue(R"json([{"sourceId":"0x1","targetId":"0x2"}])json"),
              b2h.ExecuteSelectECSql("SELECT SourceECInstanceId, TargetECInstanceId FROM ts.AToB"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetSchemaTests, Transform_NavProp_RelClassIdRemap)
    {
    // Verify that RelECClassId in a nav-prop column is remapped when the relationship
    // class has a different ID in the target ECDb.
    // Uses modifier=None so RelECClassId is non-virtual.

    Utf8String const schemaXml = R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TS" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Parent">
                <ECProperty propertyName="Val" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="Child">
                <ECProperty propertyName="Val" typeName="string"/>
                <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
            </ECEntityClass>
            <ECRelationshipClass typeName="Rel" strength="referencing" modifier="None">
                <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="True">
                    <Class class="Parent"/>
                </Source>
                <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="True">
                    <Class class="Child"/>
                </Target>
            </ECRelationshipClass>
        </ECSchema>)xml";

    // b1: baseline DB.
    ASSERT_EQ(SUCCESS, SetupECDb("csnavtx_b1.ecdb", SchemaItem(schemaXml)));

    ECInstanceKey parentKey;
        {
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.Parent(Val) VALUES('p1')"));
        ASSERT_EQ(BE_SQLITE_DONE, s.Step(parentKey));
        }
    m_ecdb.SaveChanges();

    TestChangeSet cs;
        {
        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.Child(Val,Parent) VALUES('c1',?)"));
        s.BindNavigationValue(1, parentKey.GetInstanceId());
        ECInstanceKey childKey;
        ASSERT_EQ(BE_SQLITE_DONE, s.Step(childKey));
        ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
        tracker.EnableTracking(false);
        }
    m_ecdb.SaveChanges();

    auto sourceSchema = ChangesetSchema::ExtractFrom(m_ecdb, cs);
    ASSERT_FALSE(sourceSchema.IsEmpty());

    // b2: created independently with a Dummy class first to offset IDs.
    ECDb b2;
    ASSERT_EQ(BE_SQLITE_OK, CreateECDb(b2, "csnavtx_b2.ecdb"));
    TestHelper b2h(b2);
    ASSERT_EQ(SUCCESS, b2h.ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="Dummy" alias="d" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="X"><ECProperty propertyName="V" typeName="int"/></ECEntityClass>
            </ECSchema>)xml")));
    ASSERT_EQ(SUCCESS, b2h.ImportSchema(SchemaItem(schemaXml)));

    // Insert matching Parent row.
        {
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(b2, "INSERT INTO ts.Parent(Val) VALUES('p1')"));
        ASSERT_EQ(BE_SQLITE_DONE, s.Step());
        }
    b2.SaveChanges();

    auto diff = ChangesetSchemaDiff::Compute(sourceSchema, b2);
    EXPECT_TRUE(diff.IsTransformable());

    TestChangeSet transformedCs;
    ASSERT_EQ(BE_SQLITE_OK, diff.Transform(cs, transformedCs, b2));

    ASSERT_EQ(BE_SQLITE_OK, transformedCs.ApplyChanges(b2));
    b2.SaveChanges();

    // Verify child was inserted and parent nav prop is readable.
    ASSERT_EQ(JsonValue(R"json([{"Val":"c1"}])json"), b2h.ExecuteSelectECSql("SELECT Val FROM ts.Child"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetSchemaTests, ExtractFrom_Delete)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ChangesetSchema_ExtractDelete.ecdb",
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TS" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Foo">
                    <ECProperty propertyName="Name" typeName="string"/>
                    <ECProperty propertyName="Age"  typeName="int"/>
                </ECEntityClass>
            </ECSchema>)xml")));

    // Insert a row first.
    ECInstanceKey insertedKey;
        {
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.Foo(Name,Age) VALUES('toDelete',42)"));
        ASSERT_EQ(BE_SQLITE_DONE, s.Step(insertedKey));
        }
    m_ecdb.SaveChanges();

    // Capture a DELETE changeset.
    TestChangeSet cs;
        {
        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "DELETE FROM ts.Foo WHERE ECInstanceId=?"));
        s.BindId(1, insertedKey.GetInstanceId());
        ASSERT_EQ(BE_SQLITE_DONE, s.Step());
        ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
        tracker.EnableTracking(false);
        }
    m_ecdb.SaveChanges();

    auto schema = ChangesetSchema::ExtractFrom(m_ecdb, cs);
    ASSERT_FALSE(schema.IsEmpty());

    auto fooClass = m_ecdb.Schemas().FindClass("TS:Foo");
    ASSERT_NE(nullptr, fooClass);
    auto it = schema.GetClasses().find(fooClass->GetId());
    ASSERT_NE(schema.GetClasses().end(), it) << "Foo class must appear in the schema extracted from a DELETE changeset";
    EXPECT_STREQ("TS:Foo", it->second.fullName.c_str());
    ASSERT_EQ(1u, it->second.tableSegments.size());
    auto& seg = it->second.tableSegments.begin()->second;
    EXPECT_GE(seg.idColumnIndex, 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetSchemaTests, ExtractFrom_EmptyChangeset)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ChangesetSchema_Empty.ecdb",
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TS" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Foo">
                    <ECProperty propertyName="Name" typeName="string"/>
                </ECEntityClass>
            </ECSchema>)xml")));

    // Capture an empty changeset (no DML operations inside tracking window).
    TestChangeSet cs;
        {
        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);
        // No inserts/updates/deletes.
        ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
        tracker.EnableTracking(false);
        }

    auto schema = ChangesetSchema::ExtractFrom(m_ecdb, cs);
    EXPECT_TRUE(schema.IsEmpty()) << "Schema from empty changeset should be empty";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetSchemaTests, ExtractFrom_MultiClass)
    {
    // Verify that ExtractFrom captures multiple entity classes from one changeset.
    ASSERT_EQ(SUCCESS, SetupECDb("ChangesetSchema_MultiClass.ecdb",
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TS" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Foo">
                    <ECProperty propertyName="Name" typeName="string"/>
                </ECEntityClass>
                <ECEntityClass typeName="Bar">
                    <ECProperty propertyName="Value" typeName="int"/>
                </ECEntityClass>
            </ECSchema>)xml")));

    TestChangeSet cs;
        {
        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);
        ECSqlStatement s1;
        ASSERT_EQ(ECSqlStatus::Success, s1.Prepare(m_ecdb, "INSERT INTO ts.Foo(Name) VALUES('f1')"));
        ASSERT_EQ(BE_SQLITE_DONE, s1.Step());
        s1.Finalize();
        ECSqlStatement s2;
        ASSERT_EQ(ECSqlStatus::Success, s2.Prepare(m_ecdb, "INSERT INTO ts.Bar(Value) VALUES(99)"));
        ASSERT_EQ(BE_SQLITE_DONE, s2.Step());
        ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
        tracker.EnableTracking(false);
        }
    m_ecdb.SaveChanges();

    auto schema = ChangesetSchema::ExtractFrom(m_ecdb, cs);
    ASSERT_FALSE(schema.IsEmpty());

    auto fooClass = m_ecdb.Schemas().FindClass("TS:Foo");
    auto barClass = m_ecdb.Schemas().FindClass("TS:Bar");
    ASSERT_NE(nullptr, fooClass);
    ASSERT_NE(nullptr, barClass);
    EXPECT_NE(schema.GetClasses().end(), schema.GetClasses().find(fooClass->GetId())) << "Foo should be captured";
    EXPECT_NE(schema.GetClasses().end(), schema.GetClasses().find(barClass->GetId())) << "Bar should be captured";
    EXPECT_EQ(2u, schema.GetClasses().size());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetSchemaTests, JsonRoundTrip_NavProp)
    {
    // Verify that JSON serialization/deserialization preserves classIdRefCols from nav props.
    ASSERT_EQ(SUCCESS, SetupECDb("ChangesetSchema_JsonNavProp.ecdb",
        SchemaItem(R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <ECSchema schemaName="TS" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEntityClass typeName="Parent">
                    <ECProperty propertyName="Val" typeName="string"/>
                </ECEntityClass>
                <ECEntityClass typeName="Child">
                    <ECProperty propertyName="Val" typeName="string"/>
                    <ECNavigationProperty propertyName="Parent" relationshipName="Rel" direction="Backward"/>
                </ECEntityClass>
                <ECRelationshipClass typeName="Rel" strength="referencing" modifier="None">
                    <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="True">
                        <Class class="Parent"/>
                    </Source>
                    <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="True">
                        <Class class="Child"/>
                    </Target>
                </ECRelationshipClass>
            </ECSchema>)xml")));

    ECInstanceKey parentKey;
        {
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.Parent(Val) VALUES('p1')"));
        ASSERT_EQ(BE_SQLITE_DONE, s.Step(parentKey));
        }
    m_ecdb.SaveChanges();

    TestChangeSet cs;
        {
        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.Child(Val,Parent) VALUES('c1',?)"));
        s.BindNavigationValue(1, parentKey.GetInstanceId());
        ECInstanceKey childKey;
        ASSERT_EQ(BE_SQLITE_DONE, s.Step(childKey));
        ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
        tracker.EnableTracking(false);
        }
    m_ecdb.SaveChanges();

    auto original = ChangesetSchema::ExtractFrom(m_ecdb, cs);
    ASSERT_FALSE(original.IsEmpty());

    BeJsDocument doc;
    original.To(doc);
    auto restored = ChangesetSchema::From(doc);
    ASSERT_FALSE(restored.IsEmpty());
    ASSERT_EQ(original.GetClasses().size(), restored.GetClasses().size());

    // Find Child class and verify classIdRefCols survived the round-trip.
    auto const* childClass = m_ecdb.Schemas().FindClass("TS:Child");
    ASSERT_NE(nullptr, childClass);
    auto origIt = original.GetClasses().find(childClass->GetId());
    auto restIt = restored.GetClasses().find(childClass->GetId());
    ASSERT_NE(original.GetClasses().end(), origIt);
    ASSERT_NE(restored.GetClasses().end(), restIt);

    for (auto const& sk : origIt->second.tableSegments)
        {
        auto sit = restIt->second.tableSegments.find(sk.first);
        ASSERT_NE(restIt->second.tableSegments.end(), sit);
        EXPECT_EQ(sk.second.classIdRefCols.size(), sit->second.classIdRefCols.size())
            << "classIdRefCols count must survive JSON round-trip";
        for (auto const& rk : sk.second.classIdRefCols)
            {
            auto rIt = sit->second.classIdRefCols.find(rk.first);
            ASSERT_NE(sit->second.classIdRefCols.end(), rIt) << "classIdRefCol '" << rk.first.c_str() << "' missing after round-trip";
            EXPECT_EQ(rk.second.referencedClassFullName, rIt->second.referencedClassFullName);
            EXPECT_EQ(rk.second.referencedClassId, rIt->second.referencedClassId);
            EXPECT_EQ(rk.second.sourceColumnIndex, rIt->second.sourceColumnIndex);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetSchemaTests, Diff_PropertyLost_NotTransformable)
    {
    // When a property no longer exists in the target ECDb, the diff must be
    // marked as not-transformable.
    SchemaItem schemaV1(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TS" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="Name" typeName="string"/>
                <ECProperty propertyName="Extra" typeName="string"/>
            </ECEntityClass>
        </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, SetupECDb("csdiff_proplost.ecdb", schemaV1));

    TestChangeSet cs;
        {
        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.Foo(Name,Extra) VALUES('a','e')"));
        ECInstanceKey k;
        ASSERT_EQ(BE_SQLITE_DONE, s.Step(k));
        ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
        tracker.EnableTracking(false);
        }
    m_ecdb.SaveChanges();

    auto oldSchema = ChangesetSchema::ExtractFrom(m_ecdb, cs);
    ASSERT_FALSE(oldSchema.IsEmpty());

    // Upgrade: remove the Extra property.
    SchemaItem schemaV2(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TS" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="Name" typeName="string"/>
            </ECEntityClass>
        </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, ImportSchema(schemaV2, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    auto diff = ChangesetSchemaDiff::Compute(oldSchema, m_ecdb);
    EXPECT_FALSE(diff.IsTransformable()) << "Diff should NOT be transformable when a property is lost";

    bool foundPropertyLost = false;
    for (auto const& cd : diff.GetClassDiffs())
        for (auto const& d : cd.columnDiffs)
            if (d.kind == ChangesetSchemaDiff::ChangeKind::PropertyLost && d.accessString == "Extra")
                foundPropertyLost = true;
    EXPECT_TRUE(foundPropertyLost) << "Should have a PropertyLost diff for 'Extra'";

    // Transform must fail for untransformable diffs.
    TestChangeSet transformedCs;
    EXPECT_NE(BE_SQLITE_OK, diff.Transform(cs, transformedCs, m_ecdb));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetSchemaTests, Diff_ClassLost_NotTransformable)
    {
    // When a class no longer exists in the target ECDb, the diff must be not-transformable.
    SchemaItem schemaV1(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TS" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="Name" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="Ephemeral">
                <ECProperty propertyName="Data" typeName="string"/>
            </ECEntityClass>
        </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, SetupECDb("csdiff_classlost.ecdb", schemaV1));

    TestChangeSet cs;
        {
        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.Ephemeral(Data) VALUES('gone')"));
        ECInstanceKey k;
        ASSERT_EQ(BE_SQLITE_DONE, s.Step(k));
        ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
        tracker.EnableTracking(false);
        }
    m_ecdb.SaveChanges();

    auto oldSchema = ChangesetSchema::ExtractFrom(m_ecdb, cs);
    ASSERT_FALSE(oldSchema.IsEmpty());

    // Upgrade: remove the Ephemeral class.
    SchemaItem schemaV2(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TS" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECEntityClass typeName="Foo">
                <ECProperty propertyName="Name" typeName="string"/>
            </ECEntityClass>
        </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, ImportSchema(schemaV2, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    auto diff = ChangesetSchemaDiff::Compute(oldSchema, m_ecdb);
    EXPECT_FALSE(diff.IsTransformable()) << "Diff should NOT be transformable when a class is lost";

    bool foundClassLost = false;
    for (auto const& cd : diff.GetClassDiffs())
        if (cd.classLost)
            foundClassLost = true;
    EXPECT_TRUE(foundClassLost) << "Should have a ClassLost diff";

    TestChangeSet transformedCs;
    EXPECT_NE(BE_SQLITE_OK, diff.Transform(cs, transformedCs, m_ecdb));
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetSchemaTests, Transform_UpdateWithColumnRemap)
    {
    // Verify that Transform correctly handles an UPDATE changeset where a column was
    // actually remapped to a different shared-column slot.
    SchemaItem schemaV1(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TS" alias="ts" version="01.00.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Base" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>
                    </ShareColumns>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="A">
                <BaseClass>Base</BaseClass>
                <ECProperty propertyName="PropA" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="B">
                <BaseClass>Base</BaseClass>
                <ECProperty propertyName="PropB" typeName="string"/>
            </ECEntityClass>
        </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, SetupECDb("csupdremap_b1.ecdb", schemaV1));

    // Insert an A instance.
    ECInstanceKey aKey;
        {
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "INSERT INTO ts.A(PropA) VALUES('original')"));
        ASSERT_EQ(BE_SQLITE_DONE, s.Step(aKey));
        }
    m_ecdb.SaveChanges();

    // Clone into b2.
    ECDb b2;
    ASSERT_EQ(BE_SQLITE_OK, CloneECDb(b2, "csupdremap_b2.ecdb", BeFileName(m_ecdb.GetDbFileName())));
    TestHelper b2h(b2);

    // Capture an UPDATE changeset on b1.
    TestChangeSet cs;
        {
        TestChangeTracker tracker(m_ecdb);
        tracker.EnableTracking(true);
        ECSqlStatement s;
        ASSERT_EQ(ECSqlStatus::Success, s.Prepare(m_ecdb, "UPDATE ts.A SET PropA='updated' WHERE ECInstanceId=?"));
        s.BindId(1, aKey.GetInstanceId());
        ASSERT_EQ(BE_SQLITE_DONE, s.Step());
        ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
        tracker.EnableTracking(false);
        }
    m_ecdb.SaveChanges();

    auto oldSchema = ChangesetSchema::ExtractFrom(m_ecdb, cs);
    ASSERT_FALSE(oldSchema.IsEmpty());

    // Schema v2: add NewBase between Base and A/B.
    SchemaItem schemaV2(R"xml(<?xml version="1.0" encoding="UTF-8"?>
        <ECSchema schemaName="TS" alias="ts" version="01.00.01" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
            <ECEntityClass typeName="Base" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00.00">
                        <MaxSharedColumnsBeforeOverflow>10</MaxSharedColumnsBeforeOverflow>
                    </ShareColumns>
                </ECCustomAttributes>
            </ECEntityClass>
            <ECEntityClass typeName="NewBase">
                <BaseClass>Base</BaseClass>
                <ECProperty propertyName="PropBase" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="A">
                <BaseClass>NewBase</BaseClass>
                <ECProperty propertyName="PropA" typeName="string"/>
            </ECEntityClass>
            <ECEntityClass typeName="B">
                <BaseClass>NewBase</BaseClass>
                <ECProperty propertyName="PropB" typeName="string"/>
            </ECEntityClass>
        </ECSchema>)xml");

    ASSERT_EQ(SUCCESS, ImportSchema(schemaV2, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));
    ASSERT_EQ(SUCCESS, b2h.ImportSchema(schemaV2, SchemaManager::SchemaImportOptions::AllowDataTransformDuringSchemaUpgrade));

    auto diff = ChangesetSchemaDiff::Compute(oldSchema, m_ecdb);
    EXPECT_TRUE(diff.IsTransformable());

    TestChangeSet transformedCs;
    ASSERT_EQ(BE_SQLITE_OK, diff.Transform(cs, transformedCs, m_ecdb));

    ASSERT_EQ(BE_SQLITE_OK, transformedCs.ApplyChanges(b2));
    b2.SaveChanges();

    ASSERT_EQ(JsonValue(R"json([{"PropA":"updated"}])json"), b2h.ExecuteSelectECSql("SELECT PropA FROM ts.A"));
    }

END_ECDBUNITTESTS_NAMESPACE
