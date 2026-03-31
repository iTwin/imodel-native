/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
//! Local changeset helpers (same pattern as other ECDb tests)
// @bsiclass
//=======================================================================================
struct TestCSChangeSet : BeSQLite::ChangeSet
    {
    ConflictResolution _OnConflict(ConflictCause, BeSQLite::Changes::Change) override
        { return ConflictResolution::Skip; }
    };

struct TestCSChangeTracker : BeSQLite::ChangeTracker
    {
    TestCSChangeTracker(BeSQLite::DbR db) { SetDb(&db); }
    OnCommitStatus _OnCommit(bool, Utf8CP) override { return OnCommitStatus::Commit; }
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ECChangesetReaderTests : ECDbTestFixture {
    public: 
        Utf8CP GetSchema() const { 
            return R"xml(<?xml version="1.0" encoding="utf-8"?>
                            <ECSchema schemaName="TestReadCS" alias="ts" version="01.00.00"
                                    xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                            <ECStructClass typeName="Spec" modifier="Sealed">
                                <ECProperty propertyName="Label" typeName="string"/>
                                <ECProperty propertyName="Score" typeName="int"/>
                            </ECStructClass>
                            <ECEntityClass typeName="Container" modifier="Sealed">
                                <ECProperty propertyName="Name" typeName="string"/>
                            </ECEntityClass>
                            <ECEntityClass typeName="Widget">
                                <ECProperty propertyName="Name" typeName="string"/>
                                <ECProperty propertyName="Weight" typeName="double"/>
                                <ECProperty propertyName="Cnt" typeName="long"/>
                                <ECProperty propertyName="Active" typeName="boolean"/>
                                <ECProperty propertyName="Pos2d" typeName="point2d"/>
                                <ECProperty propertyName="Pos3d" typeName="point3d"/>
                                <ECStructProperty propertyName="Details" typeName="Spec"/>
                                <ECArrayProperty propertyName="Tags" typeName="string" minOccurs="0" maxOccurs="unbounded"/>
                                <ECNavigationProperty propertyName="Owner" relationshipName="ContainerOwnsWidgets" direction="Backward"/>
                            </ECEntityClass>
                            <ECRelationshipClass typeName="ContainerOwnsWidgets" modifier="Sealed" strength="referencing">
                                <Source multiplicity="(0..1)" roleLabel="owns" polymorphic="false">
                                <Class class="Container"/>
                                </Source>
                                <Target multiplicity="(0..*)" roleLabel="is owned by" polymorphic="false">
                                <Class class="Widget"/>
                                </Target>
                            </ECRelationshipClass>
                        </ECSchema>)xml"; 
        }
};

//---------------------------------------------------------------------------------------
// INSERT a Widget with every property type populated.
// Open the changeset reader, step through every row, print every column name and value,
// then assert the expected values.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECChangesetReaderTests, Insert_AllPropertyTypes)
    {
    NativeLogging::Logging::SetLogger(&NativeLogging::ConsoleLogger::GetLogger());
    
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_insert.ecdb", SchemaItem(GetSchema())));

    // Container inserted BEFORE tracking — must not appear in the changeset.
    ECInstanceKey containerKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(containerKey,
        "INSERT INTO ts.Container(Name) VALUES('Box')"));

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECInstanceKey widgetKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "INSERT INTO ts.Widget(Name, Weight, Cnt, Active, Pos2d, Pos3d, Details.Label, Details.Score, Tags, Owner) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "WidgetA", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(2, 3.14));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(3, 42));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBoolean(4, true));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(5, DPoint2d::From(1.0, 2.0)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(6, DPoint3d::From(10.0, 20.0, 30.0)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(7, "SpecLabel", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(8, 100));
    IECSqlBinder& tagsBinder = stmt.GetBinder(9);
    ASSERT_EQ(ECSqlStatus::Success, tagsBinder.AddArrayElement().BindText("foo", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, tagsBinder.AddArrayElement().BindText("bar", IECSqlBinder::MakeCopy::No));
    ECN::ECClassId relClassId = m_ecdb.Schemas().GetClassId("TestReadCS", "ContainerOwnsWidgets");
    ASSERT_TRUE(relClassId.IsValid());
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(10, containerKey.GetInstanceId(), relClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(widgetKey));
    }

    auto cs = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker));

    ECChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenChangeStream(m_ecdb,
        std::unique_ptr<BeSQLite::ChangeStream>(cs.release()), false));

    // Step one by one.
    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());
    bool isEC = false;
    ASSERT_EQ(BE_SQLITE_OK, reader.IsECTable(isEC));

    DbOpcode opcode;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Insert, opcode);

    // Old stage must be empty for an insert.
    EXPECT_EQ(0, reader.GetColumnCount(ECChangesetReader::Stage::Old));

    // Walk every New-stage column: print then assert.
    EXPECT_EQ(11, reader.GetColumnCount(ECChangesetReader::Stage::New));
    
    //Property 1
    IECSqlValue const& v0 = reader.GetValue(ECChangesetReader::Stage::New, 0);
    ECN::ECPropertyCP prop0 = v0.GetColumnInfo().GetProperty();
    EXPECT_STREQ("ECInstanceId", prop0->GetName().c_str());
    ECInstanceId id = v0.GetId<ECInstanceId>();
    EXPECT_EQ(widgetKey.GetInstanceId(), id);

    //Property 2
    IECSqlValue const& v1 = reader.GetValue(ECChangesetReader::Stage::New, 1);
    ECN::ECPropertyCP prop1 = v1.GetColumnInfo().GetProperty();
    EXPECT_STREQ("ECClassId", prop1->GetName().c_str());
    ECN::ECClassId classId = v1.GetId<ECN::ECClassId>();
    EXPECT_EQ(widgetKey.GetClassId(), classId);

    // Property 3
    IECSqlValue const& v2 = reader.GetValue(ECChangesetReader::Stage::New, 2);
    ECN::ECPropertyCP prop2 = v2.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Name", prop2->GetName().c_str());
    Utf8CP name = v2.GetText();
    EXPECT_STREQ("WidgetA", name);

    // Property 4
    IECSqlValue const& v3 = reader.GetValue(ECChangesetReader::Stage::New, 3);
    ECN::ECPropertyCP prop3 = v3.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Weight", prop3->GetName().c_str());
    double weight = v3.GetDouble();
    EXPECT_DOUBLE_EQ(3.14, weight);

    //Property 5
    IECSqlValue const& v4 = reader.GetValue(ECChangesetReader::Stage::New, 4);
    ECN::ECPropertyCP prop4 = v4.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Cnt", prop4->GetName().c_str());
    int64_t cnt = v4.GetInt64();
    EXPECT_EQ(42, cnt);

    //Property 6
    IECSqlValue const& v5 = reader.GetValue(ECChangesetReader::Stage::New, 5);
    ECN::ECPropertyCP prop5 = v5.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Active", prop5->GetName().c_str());
    bool active = v5.GetBoolean();
    EXPECT_TRUE(active);

    //Property 7
    IECSqlValue const& v6 = reader.GetValue(ECChangesetReader::Stage::New, 6);
    ECN::ECPropertyCP prop6 = v6.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Pos2d", prop6->GetName().c_str());
    DPoint2d pos2d = v6.GetPoint2d();
    EXPECT_DOUBLE_EQ(1.0, pos2d.x);
    EXPECT_DOUBLE_EQ(2.0, pos2d.y);

    //Property 8
    IECSqlValue const& v7 = reader.GetValue(ECChangesetReader::Stage::New, 7);
    ECN::ECPropertyCP prop7 = v7.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Pos3d", prop7->GetName().c_str());
    DPoint3d pos3d = v7.GetPoint3d();
    EXPECT_DOUBLE_EQ(10.0, pos3d.x);
    EXPECT_DOUBLE_EQ(20.0, pos3d.y);
    EXPECT_DOUBLE_EQ(30.0, pos3d.z);

    //Property 9
    IECSqlValue const& v8 = reader.GetValue(ECChangesetReader::Stage::New, 8);
    ECN::ECPropertyCP prop8 = v8.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Details", prop8->GetName().c_str());
    Utf8CP label = v8["Label"].GetText();
    int   score = v8["Score"].GetInt();
    EXPECT_STREQ("SpecLabel", label);
    EXPECT_EQ(100, score);

    //Property 10
    IECSqlValue const& v9 = reader.GetValue(ECChangesetReader::Stage::New, 9);
    ECN::ECPropertyCP prop9 = v9.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Tags", prop9->GetName().c_str());
    int len = v9.GetArrayLength();
    EXPECT_EQ(2, len);
    Utf8CP expectedTags[] = {"foo", "bar"};
    int idx = 0;
    for (IECSqlValue const& elem : v9.GetArrayIterable())
        {
        Utf8CP tag = elem.GetText();
        EXPECT_STREQ(expectedTags[idx], tag);
        ++idx;
        }
    EXPECT_EQ(2, idx);

    //Property 11
    IECSqlValue const& v10 = reader.GetValue(ECChangesetReader::Stage::New, 10);
    ECN::ECPropertyCP prop10 = v10.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Owner", prop10->GetName().c_str());
    ECN::ECClassId relId;
    ECInstanceId ownerId = v10.GetNavigation<ECInstanceId>(&relId);
    EXPECT_EQ(containerKey.GetInstanceId(), ownerId);
    EXPECT_TRUE(relId.IsValid());

    Utf8String instanceKey;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetInstanceKey(ECChangesetReader::Stage::New, instanceKey));
    EXPECT_FALSE(instanceKey.empty());
    
    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    reader.Close();
    }

//---------------------------------------------------------------------------------------
// UPDATE a Widget with a partial set of columns:
//   - Name changed
//   - Pos2d.X changed (Pos2d.Y NOT changed — falls back from DB)
//   - Details.Label changed (Details.Score NOT changed — falls back from DB)
//   - All other properties untouched — must NOT appear in the changeset
// Step through the changeset, print every New-stage and Old-stage column, then assert.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECChangesetReaderTests, Update_PartialFields_ChangesetAndDBFallback)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_update.ecdb", SchemaItem(GetSchema())));

    // Baseline insert outside tracking.
    ECInstanceKey widgetKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "INSERT INTO ts.Widget(Name, Weight, Cnt, Active, Pos2d, Pos3d, Details) VALUES ('WidgetA', 3.14, 42, TRUE, ?, ?, ?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(1, DPoint2d::From(1.0, 2.0)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(2, DPoint3d::From(10.0, 20.0, 30.0)));
    IECSqlBinder& detailsBinder = stmt.GetBinder(3);
    ASSERT_EQ(ECSqlStatus::Success, detailsBinder["Label"].BindText("OldLabel", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, detailsBinder["Score"].BindInt(99));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(widgetKey));
    }

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    // Partial update: Name, Pos2d.X only, Details.Label only.
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "UPDATE ts.Widget SET Name='WidgetB', Pos2d.X=5.0, Details.Label='NewLabel' WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, widgetKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    auto cs = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker));

    ECChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenChangeStream(m_ecdb,
        std::unique_ptr<BeSQLite::ChangeStream>(cs.release()), false));

    while (reader.Step() == BE_SQLITE_ROW)
        {
        bool isEC = false;
        ASSERT_EQ(BE_SQLITE_OK, reader.IsECTable(isEC));
        if (!isEC)
            continue;

        DbOpcode opcode;
        ASSERT_EQ(BE_SQLITE_OK, reader.GetOpcode(opcode));
        ASSERT_EQ(DbOpcode::Update, opcode);

        // ---- Print and assert New stage ----
        int newCount = reader.GetColumnCount(ECChangesetReader::Stage::New);
        printf("[Update] New stage: %d columns\n", newCount);

        for (int i = 0; i < newCount; ++i)
            {
            IECSqlValue const& v = reader.GetValue(ECChangesetReader::Stage::New, i);
            ECN::ECPropertyCP prop = v.GetColumnInfo().GetProperty();
            if (prop == nullptr)
                continue;

            if (prop->GetName().EqualsIAscii("ECInstanceId"))
                {
                ECInstanceId id = v.GetId<ECInstanceId>();
                printf("  New[%d] ECInstanceId = %s\n", i, id.ToHexStr().c_str());
                EXPECT_EQ(widgetKey.GetInstanceId(), id);
                }
            else if (prop->GetName().EqualsIAscii("ECClassId"))
                {
                printf("  New[%d] ECClassId = %s\n", i, v.GetId<ECN::ECClassId>().ToHexStr().c_str());
                EXPECT_EQ(widgetKey.GetClassId(), v.GetId<ECN::ECClassId>());
                }
            else if (prop->GetName().EqualsIAscii("Name"))
                {
                printf("  New[%d] Name = %s\n", i, v.GetText());
                EXPECT_STREQ("WidgetB", v.GetText());
                }
            else if (prop->GetName().EqualsIAscii("X"))
                {
                // X is in changeset (updated to 5.0).
                // Y is absent from changeset — falls back to DB (still 2.0).
                double p = v.GetDouble();
                printf("  New[%d] Pos2d.X = (%f)\n", i, p);
                EXPECT_DOUBLE_EQ(5.0, p) << "Pos2d.X must be the updated value";
                }
            else if (prop->GetName().EqualsIAscii("Details"))
                {
                // Label is in changeset; Score is absent — falls back to DB (99).
                Utf8CP label = v.GetText();
                printf("  New[%d] Details.Label = %s\n", i, label);
                EXPECT_STREQ("NewLabel", label);
                }
            else
                {
                // Weight, Count, Active, Pos3d, Tags, Owner were NOT updated.
                printf("  New[%d] %s = %s\n", i, prop->GetName().c_str(), v.GetText());
                }
            }

        // ---- Print and assert Old stage ----
        int oldCount = reader.GetColumnCount(ECChangesetReader::Stage::Old);
        printf("[Update] Old stage: %d columns\n", oldCount);

        for (int i = 0; i < oldCount; ++i)
            {
            IECSqlValue const& v = reader.GetValue(ECChangesetReader::Stage::Old, i);
            ECN::ECPropertyCP prop = v.GetColumnInfo().GetProperty();
            if (prop == nullptr)
                continue;

            if (prop->GetName().EqualsIAscii("ECInstanceId"))
                {
                ECInstanceId id = v.GetId<ECInstanceId>();
                printf("  Old[%d] ECInstanceId = %s\n", i, id.ToHexStr().c_str());
                EXPECT_EQ(widgetKey.GetInstanceId(), id);
                }
            else if (prop->GetName().EqualsIAscii("ECClassId"))
                {
                printf("  Old[%d] ECClassId = %s\n", i, v.GetId<ECN::ECClassId>().ToHexStr().c_str());
                EXPECT_EQ(widgetKey.GetClassId(), v.GetId<ECN::ECClassId>());
                }
            else if (prop->GetName().EqualsIAscii("Name"))
                {
                printf("  Old[%d] Name = %s\n", i, v.GetText());
                EXPECT_STREQ("WidgetA", v.GetText());
                }
            else if (prop->GetName().EqualsIAscii("Pos2d.X"))
                {
                double p = v.GetDouble();
                printf("  Old[%d] Pos2d.X = (%f)\n", i, p);
                EXPECT_DOUBLE_EQ(1.0, p) << "Old Pos2d.X must be the pre-update value";
                }
            else if (prop->GetName().EqualsIAscii("Details.Label"))
                {
                Utf8CP label = v.GetText();
                printf("  Old[%d] Details.Label = %s\n", i, label);
                EXPECT_STREQ("OldLabel", label);
                }
            else
                {
                ADD_FAILURE() << "Unexpected unchanged property in Old stage: "
                              << prop->GetName().c_str();
                }
            }

        Utf8String newKey, oldKey;
        ASSERT_EQ(BE_SQLITE_OK, reader.GetInstanceKey(ECChangesetReader::Stage::New, newKey));
        ASSERT_EQ(BE_SQLITE_OK, reader.GetInstanceKey(ECChangesetReader::Stage::Old, oldKey));
        printf("  InstanceKey(New) = %s\n  InstanceKey(Old) = %s\n",
            newKey.c_str(), oldKey.c_str());
        EXPECT_FALSE(newKey.empty());
        EXPECT_FALSE(oldKey.empty());
        }

    reader.Close();
    }

//---------------------------------------------------------------------------------------
// DELETE a Widget.
// Stage::New must be empty (column count == 0).
// Stage::Old must carry every originally-inserted property.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECChangesetReaderTests, Delete_OldStageContainsAllValues)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_delete.ecdb", SchemaItem(GetSchema())));

    // Insert outside tracking.
    ECInstanceKey widgetKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "INSERT INTO ts.Widget(Name, Weight, Cnt, Active, Pos2d, Pos3d, Details) VALUES ('Doomed', 2.71, 7, FALSE, ?, ?, ?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(1, DPoint2d::From(3.0, 4.0)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(2, DPoint3d::From(1.0, 2.0, 3.0)));
    IECSqlBinder& detailsBinder = stmt.GetBinder(3);
    ASSERT_EQ(ECSqlStatus::Success, detailsBinder["Label"].BindText("MyLabel", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, detailsBinder["Score"].BindInt(55));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(widgetKey));
    }

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "DELETE FROM ts.Widget WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, widgetKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    auto cs = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker));

    ECChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenChangeStream(m_ecdb,
        std::unique_ptr<BeSQLite::ChangeStream>(cs.release()), false));

    while (reader.Step() == BE_SQLITE_ROW)
        {
        bool isEC = false;
        ASSERT_EQ(BE_SQLITE_OK, reader.IsECTable(isEC));
        if (!isEC)
            continue;

        DbOpcode opcode;
        ASSERT_EQ(BE_SQLITE_OK, reader.GetOpcode(opcode));
        ASSERT_EQ(DbOpcode::Delete, opcode);

        // New stage must be empty for a delete.
        EXPECT_EQ(0, reader.GetColumnCount(ECChangesetReader::Stage::New));

        int oldCount = reader.GetColumnCount(ECChangesetReader::Stage::Old);
        printf("[Delete] Old stage: %d columns\n", oldCount);

        for (int i = 0; i < oldCount; ++i)
            {
            IECSqlValue const& v = reader.GetValue(ECChangesetReader::Stage::Old, i);
            ECN::ECPropertyCP prop = v.GetColumnInfo().GetProperty();
            if (prop == nullptr)
                continue;

            if (prop->GetName().EqualsIAscii("ECInstanceId"))
                {
                ECInstanceId id = v.GetId<ECInstanceId>();
                printf("  Old[%d] ECInstanceId = %s\n", i, id.ToHexStr().c_str());
                EXPECT_EQ(widgetKey.GetInstanceId(), id);
                }
            else if (prop->GetName().EqualsIAscii("ECClassId"))
                {
                ECN::ECClassId classId = v.GetId<ECN::ECClassId>();
                printf("  Old[%d] ECClassId = %s\n", i, classId.ToHexStr().c_str());
                EXPECT_EQ(widgetKey.GetClassId(), classId);
                }
            else if (prop->GetName().EqualsIAscii("Name"))
                {
                printf("  Old[%d] Name = %s\n", i, v.GetText());
                EXPECT_STREQ("Doomed", v.GetText());
                }
            else if (prop->GetName().EqualsIAscii("Weight"))
                {
                printf("  Old[%d] Weight = %f\n", i, v.GetDouble());
                EXPECT_DOUBLE_EQ(2.71, v.GetDouble());
                }
            else if (prop->GetName().EqualsIAscii("Count"))
                {
                printf("  Old[%d] Count = %lld\n", i, (long long)v.GetInt64());
                EXPECT_EQ(7, v.GetInt64());
                }
            else if (prop->GetName().EqualsIAscii("Active"))
                {
                printf("  Old[%d] Active = %d\n", i, (int)v.GetBoolean());
                EXPECT_FALSE(v.GetBoolean());
                }
            else if (prop->GetName().EqualsIAscii("Pos2d"))
                {
                DPoint2d p = v.GetPoint2d();
                printf("  Old[%d] Pos2d = (%f, %f)\n", i, p.x, p.y);
                EXPECT_DOUBLE_EQ(3.0, p.x);
                EXPECT_DOUBLE_EQ(4.0, p.y);
                }
            else if (prop->GetName().EqualsIAscii("Pos3d"))
                {
                DPoint3d p = v.GetPoint3d();
                printf("  Old[%d] Pos3d = (%f, %f, %f)\n", i, p.x, p.y, p.z);
                EXPECT_DOUBLE_EQ(1.0, p.x);
                EXPECT_DOUBLE_EQ(2.0, p.y);
                EXPECT_DOUBLE_EQ(3.0, p.z);
                }
            else if (prop->GetName().EqualsIAscii("Details"))
                {
                Utf8CP label = v["Label"].GetText();
                int   score = v["Score"].GetInt();
                printf("  Old[%d] Details = { Label=%s, Score=%d }\n", i, label, score);
                EXPECT_STREQ("MyLabel", label);
                EXPECT_EQ(55, score);
                }
            else
                {
                printf("  Old[%d] %s (not checked)\n", i, prop->GetName().c_str());
                }
            }

        Utf8String instanceKey;
        ASSERT_EQ(BE_SQLITE_OK, reader.GetInstanceKey(ECChangesetReader::Stage::Old, instanceKey));
        printf("  InstanceKey(Old) = %s\n", instanceKey.c_str());
        EXPECT_FALSE(instanceKey.empty());
        }

    reader.Close();
    }

//---------------------------------------------------------------------------------------
// Three sequential changesets: INSERT -> UPDATE -> DELETE.
// Verifies that FromChangeTrack resets the tracker between captures so each
// changeset contains only its own operation.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECChangesetReaderTests, InsertUpdateDelete_SequentialChangesets)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_seq.ecdb", SchemaItem(GetSchema())));

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    // ============================================================
    // Changeset 1: INSERT
    // ============================================================
    ECInstanceKey widgetKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "INSERT INTO ts.Widget(Name, Cnt) VALUES ('Seq1', 10)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(widgetKey));
    }

    {
    auto cs = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker)); // capture and reset

    ECChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenChangeStream(m_ecdb,
        std::unique_ptr<BeSQLite::ChangeStream>(cs.release()), false));

    while (reader.Step() == BE_SQLITE_ROW)
        {
        bool isEC = false;
        ASSERT_EQ(BE_SQLITE_OK, reader.IsECTable(isEC));
        if (!isEC)
            continue;

        DbOpcode op;
        ASSERT_EQ(BE_SQLITE_OK, reader.GetOpcode(op));
        ASSERT_EQ(DbOpcode::Insert, op);
        EXPECT_EQ(0, reader.GetColumnCount(ECChangesetReader::Stage::Old));

        int newCount = reader.GetColumnCount(ECChangesetReader::Stage::New);
        printf("[Seq Insert] New cols: %d\n", newCount);

        for (int i = 0; i < newCount; ++i)
            {
            IECSqlValue const& v = reader.GetValue(ECChangesetReader::Stage::New, i);
            ECN::ECPropertyCP prop = v.GetColumnInfo().GetProperty();
            if (prop == nullptr)
                continue;
            if (prop->GetName().EqualsIAscii("Name"))
                {
                printf("  Name = %s\n", v.GetText());
                EXPECT_STREQ("Seq1", v.GetText());
                }
            else if (prop->GetName().EqualsIAscii("Count"))
                {
                printf("  Count = %lld\n", (long long)v.GetInt64());
                EXPECT_EQ(10, v.GetInt64());
                }
            else
                {
                printf("  %s (not checked)\n", prop->GetName().c_str());
                }
            }
        }

    reader.Close();
    }

    // ============================================================
    // Changeset 2: UPDATE (only Name)
    // ============================================================
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "UPDATE ts.Widget SET Name='Seq2' WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, widgetKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    {
    auto cs = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker)); // capture and reset

    ECChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenChangeStream(m_ecdb,
        std::unique_ptr<BeSQLite::ChangeStream>(cs.release()), false));

    while (reader.Step() == BE_SQLITE_ROW)
        {
        bool isEC = false;
        ASSERT_EQ(BE_SQLITE_OK, reader.IsECTable(isEC));
        if (!isEC)
            continue;

        DbOpcode op;
        ASSERT_EQ(BE_SQLITE_OK, reader.GetOpcode(op));
        ASSERT_EQ(DbOpcode::Update, op);

        int newCount = reader.GetColumnCount(ECChangesetReader::Stage::New);
        int oldCount = reader.GetColumnCount(ECChangesetReader::Stage::Old);
        printf("[Seq Update] New cols: %d  Old cols: %d\n", newCount, oldCount);

        for (int i = 0; i < newCount; ++i)
            {
            IECSqlValue const& v = reader.GetValue(ECChangesetReader::Stage::New, i);
            ECN::ECPropertyCP prop = v.GetColumnInfo().GetProperty();
            if (prop == nullptr)
                continue;
            if (prop->GetName().EqualsIAscii("Name"))
                {
                printf("  New Name = %s\n", v.GetText());
                EXPECT_STREQ("Seq2", v.GetText());
                }
            else if (!prop->GetName().EqualsIAscii("ECInstanceId") &&
                     !prop->GetName().EqualsIAscii("ECClassId"))
                {
                // Count was not updated — must not appear in the changeset.
                ADD_FAILURE() << "Unexpected property in Update New stage: "
                              << prop->GetName().c_str();
                }
            }

        for (int i = 0; i < oldCount; ++i)
            {
            IECSqlValue const& v = reader.GetValue(ECChangesetReader::Stage::Old, i);
            ECN::ECPropertyCP prop = v.GetColumnInfo().GetProperty();
            if (prop == nullptr)
                continue;
            if (prop->GetName().EqualsIAscii("Name"))
                {
                printf("  Old Name = %s\n", v.GetText());
                EXPECT_STREQ("Seq1", v.GetText());
                }
            else if (!prop->GetName().EqualsIAscii("ECInstanceId") &&
                     !prop->GetName().EqualsIAscii("ECClassId"))
                {
                ADD_FAILURE() << "Unexpected property in Update Old stage: "
                              << prop->GetName().c_str();
                }
            }
        }

    reader.Close();
    }

    // ============================================================
    // Changeset 3: DELETE
    // ============================================================
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "DELETE FROM ts.Widget WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, widgetKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    {
    auto cs = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker)); // capture and reset

    ECChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenChangeStream(m_ecdb,
        std::unique_ptr<BeSQLite::ChangeStream>(cs.release()), false));

    while (reader.Step() == BE_SQLITE_ROW)
        {
        bool isEC = false;
        ASSERT_EQ(BE_SQLITE_OK, reader.IsECTable(isEC));
        if (!isEC)
            continue;

        DbOpcode op;
        ASSERT_EQ(BE_SQLITE_OK, reader.GetOpcode(op));
        ASSERT_EQ(DbOpcode::Delete, op);
        EXPECT_EQ(0, reader.GetColumnCount(ECChangesetReader::Stage::New));

        int oldCount = reader.GetColumnCount(ECChangesetReader::Stage::Old);
        printf("[Seq Delete] Old cols: %d\n", oldCount);

        for (int i = 0; i < oldCount; ++i)
            {
            IECSqlValue const& v = reader.GetValue(ECChangesetReader::Stage::Old, i);
            ECN::ECPropertyCP prop = v.GetColumnInfo().GetProperty();
            if (prop == nullptr)
                continue;
            if (prop->GetName().EqualsIAscii("Name"))
                {
                // Must be "Seq2" — the value as of the last update.
                printf("  Old Name = %s\n", v.GetText());
                EXPECT_STREQ("Seq2", v.GetText());
                }
            else
                {
                printf("  Old %s (not checked)\n", prop->GetName().c_str());
                }
            }
        }

    reader.Close();
    }
    }

END_ECDBUNITTESTS_NAMESPACE
