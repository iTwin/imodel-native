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

    auto fooClass = m_ecdb.Schemas().GetClass("TS", "Foo");
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
    auto fooClass = m_ecdb.Schemas().GetClass("TS", "Foo");
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

    auto fooClass = m_ecdb.Schemas().GetClass("TS", "Foo");
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

END_ECDBUNITTESTS_NAMESPACE
