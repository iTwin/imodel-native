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
        std::unique_ptr<BeSQLite::ChangeStream>(cs.release()), false, ECChangesetReader::Mode::All_Properties));

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

    std::unordered_set<Utf8String> changedProps;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetChangedPropertyNames(changedProps));
    EXPECT_NE(changedProps.end(), changedProps.find("ECInstanceId")); // INSERT — always present
    EXPECT_NE(changedProps.end(), changedProps.find("Name"));
    EXPECT_NE(changedProps.end(), changedProps.find("Weight"));
    EXPECT_NE(changedProps.end(), changedProps.find("Cnt"));
    EXPECT_NE(changedProps.end(), changedProps.find("Active"));
    EXPECT_NE(changedProps.end(), changedProps.find("Pos2d"));        // both coords set → full name
    EXPECT_EQ(changedProps.end(), changedProps.find("Pos2d.X"));
    EXPECT_EQ(changedProps.end(), changedProps.find("Pos2d.Y"));
    EXPECT_NE(changedProps.end(), changedProps.find("Pos3d"));        // all three coords set → full name
    EXPECT_EQ(changedProps.end(), changedProps.find("Pos3d.X"));
    EXPECT_NE(changedProps.end(), changedProps.find("Details.Label"));
    EXPECT_NE(changedProps.end(), changedProps.find("Details.Score"));
    EXPECT_EQ(changedProps.end(), changedProps.find("Details"));      // bare struct name never present
    EXPECT_NE(changedProps.end(), changedProps.find("Tags"));
    bool hasOwner = changedProps.count("Owner") > 0 || changedProps.count("Owner.Id") > 0;
    EXPECT_TRUE(hasOwner) << "Expected 'Owner' or 'Owner.Id' in changed property names";

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
        std::unique_ptr<BeSQLite::ChangeStream>(cs.release()), false, ECChangesetReader::Mode::All_Properties));

    ASSERT_EQ(reader.Step(), BE_SQLITE_ROW);
    bool isEC = false;
    ASSERT_EQ(BE_SQLITE_OK, reader.IsECTable(isEC));

    DbOpcode opcode;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Update, opcode);
    ASSERT_EQ(5, reader.GetColumnCount(ECChangesetReader::Stage::New));

    // Property 1
    IECSqlValue const& new_v0 = reader.GetValue(ECChangesetReader::Stage::New, 0);
    ECN::ECPropertyCP new_prop0 = new_v0.GetColumnInfo().GetProperty();
    EXPECT_STREQ("ECInstanceId", new_prop0->GetName().c_str());
    ECInstanceId id = new_v0.GetId<ECInstanceId>();
    EXPECT_EQ(widgetKey.GetInstanceId(), id);

    //Property 2
    IECSqlValue const& new_v1 = reader.GetValue(ECChangesetReader::Stage::New, 1);
    ECN::ECPropertyCP new_prop1 = new_v1.GetColumnInfo().GetProperty();
    EXPECT_STREQ("ECClassId", new_prop1->GetName().c_str());
    ECN::ECClassId classId = new_v1.GetId<ECN::ECClassId>();
    EXPECT_EQ(widgetKey.GetClassId(), classId);

    //Property 3
    IECSqlValue const& new_v2 = reader.GetValue(ECChangesetReader::Stage::New, 2);
    ECN::ECPropertyCP new_prop2 = new_v2.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Name", new_prop2->GetName().c_str());
    Utf8CP name = new_v2.GetText();
    EXPECT_STREQ("WidgetB", name);

    //Property 4
    IECSqlValue const& new_v3 = reader.GetValue(ECChangesetReader::Stage::New, 3);
    ECN::ECPropertyCP new_prop3 = new_v3.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Pos2d", new_prop3->GetName().c_str());
    DPoint2d pos2d = new_v3.GetPoint2d();
    EXPECT_DOUBLE_EQ(5.0, pos2d.x) << "Pos2d.X must be the updated value";
    EXPECT_DOUBLE_EQ(2.0, pos2d.y) << "Pos2d.Y must fall back to the pre-update value";

    //Property 5
    IECSqlValue const& new_v4 = reader.GetValue(ECChangesetReader::Stage::New, 4);
    ECN::ECPropertyCP new_prop4 = new_v4.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Details", new_prop4->GetName().c_str());
    Utf8CP label = new_v4["Label"].GetText();
    EXPECT_STREQ("NewLabel", label);

    ASSERT_EQ(5, reader.GetColumnCount(ECChangesetReader::Stage::Old));

    //Property 1
    IECSqlValue const& old_v0 = reader.GetValue(ECChangesetReader::Stage::Old, 0);
    ECN::ECPropertyCP old_prop0 = old_v0.GetColumnInfo().GetProperty();
    EXPECT_STREQ("ECInstanceId", old_prop0->GetName().c_str());
    ECInstanceId oldId = old_v0.GetId<ECInstanceId>();
    EXPECT_EQ(widgetKey.GetInstanceId(), oldId);

    //Property 2
    IECSqlValue const& old_v1 = reader.GetValue(ECChangesetReader::Stage::Old, 1);
    ECN::ECPropertyCP old_prop1 = old_v1.GetColumnInfo().GetProperty();
    EXPECT_STREQ("ECClassId", old_prop1->GetName().c_str());
    ECN::ECClassId oldClassId = old_v1.GetId<ECN::ECClassId>();
    EXPECT_EQ(widgetKey.GetClassId(), oldClassId);

    //Property 3
    IECSqlValue const& old_v2 = reader.GetValue(ECChangesetReader::Stage::Old, 2);
    ECN::ECPropertyCP old_prop2 = old_v2.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Name", old_prop2->GetName().c_str());
    Utf8CP oldName = old_v2.GetText();
    EXPECT_STREQ("WidgetA", oldName);

    //Property 4
    IECSqlValue const& old_v3 = reader.GetValue(ECChangesetReader::Stage::Old, 3);
    ECN::ECPropertyCP old_prop3 = old_v3.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Pos2d", old_prop3->GetName().c_str());
    DPoint2d oldPos2d = old_v3.GetPoint2d();
    EXPECT_DOUBLE_EQ(1.0, oldPos2d.x) << "Old Pos2d.X must be the pre-update value";
    EXPECT_DOUBLE_EQ(2.0, oldPos2d.y) << "Old Pos2d.Y must fall back to the pre-update value";

    //Property 5
    IECSqlValue const& old_v4 = reader.GetValue(ECChangesetReader::Stage::Old, 4);
    ECN::ECPropertyCP old_prop4 = old_v4.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Details", old_prop4->GetName().c_str());
    Utf8CP oldLabel = old_v4["Label"].GetText();
    EXPECT_STREQ("OldLabel", oldLabel);


    Utf8String newKey, oldKey;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetInstanceKey(ECChangesetReader::Stage::New, newKey));
    ASSERT_EQ(BE_SQLITE_OK, reader.GetInstanceKey(ECChangesetReader::Stage::Old, oldKey));
    EXPECT_FALSE(newKey.empty());
    EXPECT_FALSE(oldKey.empty());

    std::unordered_set<Utf8String> changedProps;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetChangedPropertyNames(changedProps));
    // Changed: Name (scalar), Pos2d.X (partial Point2d), Details.Label (partial struct).
    EXPECT_NE(changedProps.end(), changedProps.find("Name"));
    EXPECT_NE(changedProps.end(), changedProps.find("Pos2d.X"));
    EXPECT_NE(changedProps.end(), changedProps.find("Details.Label"));
    // Unchanged or partial components must not appear.
    EXPECT_EQ(changedProps.end(), changedProps.find("Pos2d"));          // not both coords changed
    EXPECT_EQ(changedProps.end(), changedProps.find("Pos2d.Y"));
    EXPECT_EQ(changedProps.end(), changedProps.find("Details.Score"));  // Score not changed
    EXPECT_EQ(changedProps.end(), changedProps.find("Weight"));
    EXPECT_EQ(changedProps.end(), changedProps.find("Cnt"));
    EXPECT_EQ(changedProps.end(), changedProps.find("Pos3d"));
    EXPECT_EQ(changedProps.end(), changedProps.find("ECInstanceId"));   // UPDATE — never present

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());

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

    // Container inserted BEFORE tracking — must not appear in the changeset.
    ECInstanceKey containerKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(containerKey,
        "INSERT INTO ts.Container(Name) VALUES('Box')"));

    // Widget inserted BEFORE tracking — deleted row values appear in Old stage.
    ECInstanceKey widgetKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "INSERT INTO ts.Widget(Name, Weight, Cnt, Active, Pos2d, Pos3d, Details.Label, Details.Score, Tags, Owner) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Doomed", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(2, 2.71));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(3, 7));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBoolean(4, false));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(5, DPoint2d::From(3.0, 4.0)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(6, DPoint3d::From(1.0, 2.0, 3.0)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(7, "MyLabel", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(8, 55));
    IECSqlBinder& tagsBinder = stmt.GetBinder(9);
    ASSERT_EQ(ECSqlStatus::Success, tagsBinder.AddArrayElement().BindText("alpha", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, tagsBinder.AddArrayElement().BindText("beta", IECSqlBinder::MakeCopy::No));
    ECN::ECClassId relClassId = m_ecdb.Schemas().GetClassId("TestReadCS", "ContainerOwnsWidgets");
    ASSERT_TRUE(relClassId.IsValid());
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(10, containerKey.GetInstanceId(), relClassId));
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
        std::unique_ptr<BeSQLite::ChangeStream>(cs.release()), false, ECChangesetReader::Mode::All_Properties));

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());
    bool isEC = false;
    ASSERT_EQ(BE_SQLITE_OK, reader.IsECTable(isEC));

    DbOpcode opcode;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Delete, opcode);

    // New stage must be empty for a delete.
    EXPECT_EQ(0, reader.GetColumnCount(ECChangesetReader::Stage::New));

    EXPECT_EQ(11, reader.GetColumnCount(ECChangesetReader::Stage::Old));

    // Property 1
    IECSqlValue const& v0 = reader.GetValue(ECChangesetReader::Stage::Old, 0);
    EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetInstanceId(), v0.GetId<ECInstanceId>());

    // Property 2
    IECSqlValue const& v1 = reader.GetValue(ECChangesetReader::Stage::Old, 1);
    EXPECT_STREQ("ECClassId", v1.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetClassId(), v1.GetId<ECN::ECClassId>());

    // Property 3
    IECSqlValue const& v2 = reader.GetValue(ECChangesetReader::Stage::Old, 2);
    EXPECT_STREQ("Name", v2.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_STREQ("Doomed", v2.GetText());

    // Property 4
    IECSqlValue const& v3 = reader.GetValue(ECChangesetReader::Stage::Old, 3);
    EXPECT_STREQ("Weight", v3.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_DOUBLE_EQ(2.71, v3.GetDouble());

    // Property 5
    IECSqlValue const& v4 = reader.GetValue(ECChangesetReader::Stage::Old, 4);
    EXPECT_STREQ("Cnt", v4.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(7, v4.GetInt64());

    // Property 6
    IECSqlValue const& v5 = reader.GetValue(ECChangesetReader::Stage::Old, 5);
    EXPECT_STREQ("Active", v5.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_FALSE(v5.GetBoolean());

    // Property 7
    IECSqlValue const& v6 = reader.GetValue(ECChangesetReader::Stage::Old, 6);
    EXPECT_STREQ("Pos2d", v6.GetColumnInfo().GetProperty()->GetName().c_str());
    DPoint2d pos2d = v6.GetPoint2d();
    EXPECT_DOUBLE_EQ(3.0, pos2d.x);
    EXPECT_DOUBLE_EQ(4.0, pos2d.y);

    // Property 8
    IECSqlValue const& v7 = reader.GetValue(ECChangesetReader::Stage::Old, 7);
    EXPECT_STREQ("Pos3d", v7.GetColumnInfo().GetProperty()->GetName().c_str());
    DPoint3d pos3d = v7.GetPoint3d();
    EXPECT_DOUBLE_EQ(1.0, pos3d.x);
    EXPECT_DOUBLE_EQ(2.0, pos3d.y);
    EXPECT_DOUBLE_EQ(3.0, pos3d.z);

    // Property 9
    IECSqlValue const& v8 = reader.GetValue(ECChangesetReader::Stage::Old, 8);
    EXPECT_STREQ("Details", v8.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_STREQ("MyLabel", v8["Label"].GetText());
    EXPECT_EQ(55, v8["Score"].GetInt());

    // Property 10
    IECSqlValue const& v9 = reader.GetValue(ECChangesetReader::Stage::Old, 9);
    EXPECT_STREQ("Tags", v9.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(2, v9.GetArrayLength());
    Utf8CP expectedTags[] = {"alpha", "beta"};
    int idx = 0;
    for (IECSqlValue const& elem : v9.GetArrayIterable())
        {
        EXPECT_STREQ(expectedTags[idx], elem.GetText());
        ++idx;
        }
    EXPECT_EQ(2, idx);

    // Property 11
    IECSqlValue const& v10 = reader.GetValue(ECChangesetReader::Stage::Old, 10);
    EXPECT_STREQ("Owner", v10.GetColumnInfo().GetProperty()->GetName().c_str());
    ECN::ECClassId relId;
    ECInstanceId ownerId = v10.GetNavigation<ECInstanceId>(&relId);
    EXPECT_EQ(containerKey.GetInstanceId(), ownerId);
    EXPECT_TRUE(relId.IsValid());

    Utf8String instanceKey;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetInstanceKey(ECChangesetReader::Stage::Old, instanceKey));
    EXPECT_FALSE(instanceKey.empty());

    std::unordered_set<Utf8String> changedProps;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetChangedPropertyNames(changedProps));
    EXPECT_NE(changedProps.end(), changedProps.find("ECInstanceId")); // DELETE — always present
    EXPECT_NE(changedProps.end(), changedProps.find("Name"));
    EXPECT_NE(changedProps.end(), changedProps.find("Weight"));
    EXPECT_NE(changedProps.end(), changedProps.find("Cnt"));
    EXPECT_NE(changedProps.end(), changedProps.find("Active"));
    EXPECT_NE(changedProps.end(), changedProps.find("Pos2d"));
    EXPECT_NE(changedProps.end(), changedProps.find("Pos3d"));
    EXPECT_NE(changedProps.end(), changedProps.find("Details.Label"));
    EXPECT_NE(changedProps.end(), changedProps.find("Details.Score"));
    EXPECT_EQ(changedProps.end(), changedProps.find("Details"));
    EXPECT_NE(changedProps.end(), changedProps.find("Tags"));
    bool hasOwner = changedProps.count("Owner") > 0 || changedProps.count("Owner.Id") > 0;
    EXPECT_TRUE(hasOwner) << "Expected 'Owner' or 'Owner.Id' in changed property names";

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    reader.Close();
    }

//---------------------------------------------------------------------------------------
// INSERT a Widget with only two scalar properties (Name + Weight) — all other properties
// left unset (NULL).  Verifies that the changeset reader emits exactly those properties
// that were explicitly provided and that GetChangedPropertyNames is consistent.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECChangesetReaderTests, Insert_PartialProperties)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_partial_insert.ecdb", SchemaItem(GetSchema())));

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    // Insert only Name and Weight — all other properties left unset (NULL).
    ECInstanceKey widgetKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "INSERT INTO ts.Widget(Name, Weight) VALUES('Sparse', 2.5)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(widgetKey));
    }

    auto cs = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker));

    ECChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenChangeStream(m_ecdb,
        std::unique_ptr<BeSQLite::ChangeStream>(cs.release()), false,
        ECChangesetReader::Mode::All_Properties));

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());

    DbOpcode opcode;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Insert, opcode);

    // Old stage empty for insert.
    EXPECT_EQ(0, reader.GetColumnCount(ECChangesetReader::Stage::Old));

    // New stage: ECInstanceId, ECClassId, name + weight at minimum.
    int newCount = reader.GetColumnCount(ECChangesetReader::Stage::New);
    EXPECT_GE(newCount, 4);

    // Slots 0 and 1 must always be ECInstanceId and ECClassId.
    IECSqlValue const& v0 = reader.GetValue(ECChangesetReader::Stage::New, 0);
    EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetInstanceId(), v0.GetId<ECInstanceId>());

    IECSqlValue const& v1 = reader.GetValue(ECChangesetReader::Stage::New, 1);
    EXPECT_STREQ("ECClassId", v1.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetClassId(), v1.GetId<ECN::ECClassId>());

    // Find Name and Weight anywhere in the new-stage field list.
    bool foundName = false, foundWeight = false;
    for (int i = 2; i < newCount; ++i)
        {
        IECSqlValue const& v = reader.GetValue(ECChangesetReader::Stage::New, i);
        auto* prop = v.GetColumnInfo().GetProperty();
        if (!prop) continue;
        if (Utf8String(prop->GetName()).EqualsIAscii("Name"))
            {
            EXPECT_STREQ("Sparse", v.GetText());
            foundName = true;
            }
        else if (Utf8String(prop->GetName()).EqualsIAscii("Weight"))
            {
            EXPECT_DOUBLE_EQ(2.5, v.GetDouble());
            foundWeight = true;
            }
        }
    EXPECT_TRUE(foundName)   << "Name must appear in the partial-insert changeset";
    EXPECT_TRUE(foundWeight) << "Weight must appear in the partial-insert changeset";

    // GetChangedPropertyNames must always include the explicitly set properties.
    std::unordered_set<Utf8String> changedProps;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetChangedPropertyNames(changedProps));
    EXPECT_NE(changedProps.end(), changedProps.find("ECInstanceId")); // INSERT — always present
    EXPECT_NE(changedProps.end(), changedProps.find("Name"));
    EXPECT_NE(changedProps.end(), changedProps.find("Weight"));

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    reader.Close();
    }

//---------------------------------------------------------------------------------------
// INSERT a Widget with Tags outside tracker, then UPDATE Tags with a completely new set.
// For UPDATE: only Tags changes — New and Old stages each carry exactly 3 fields
// (ECInstanceId, ECClassId, Tags).
// Verifies array values in both stages and GetChangedPropertyNames = {"Tags"}.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECChangesetReaderTests, Update_ArrayProperty)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_update_array.ecdb", SchemaItem(GetSchema())));

    ECInstanceKey widgetKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "INSERT INTO ts.Widget(Tags) VALUES(?)"));
    IECSqlBinder& b = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, b.AddArrayElement().BindText("old1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, b.AddArrayElement().BindText("old2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(widgetKey));
    }

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    // Update Tags to a different set of values.
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "UPDATE ts.Widget SET Tags=? WHERE ECInstanceId=?"));
    IECSqlBinder& b = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, b.AddArrayElement().BindText("new1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, b.AddArrayElement().BindText("new2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, b.AddArrayElement().BindText("new3", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, widgetKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    auto cs = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker));

    ECChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenChangeStream(m_ecdb,
        std::unique_ptr<BeSQLite::ChangeStream>(cs.release()), false,
        ECChangesetReader::Mode::All_Properties));

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());

    DbOpcode opcode;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Update, opcode);

    // Only Tags changed: ECInstanceId, ECClassId, Tags.
    ASSERT_EQ(3, reader.GetColumnCount(ECChangesetReader::Stage::New));
    ASSERT_EQ(3, reader.GetColumnCount(ECChangesetReader::Stage::Old));

    // New stage — Tags has 3 elements.
    IECSqlValue const& newTags = reader.GetValue(ECChangesetReader::Stage::New, 2);
    EXPECT_STREQ("Tags", newTags.GetColumnInfo().GetProperty()->GetName().c_str());
    ASSERT_EQ(3, newTags.GetArrayLength());
    Utf8CP expectedNew[] = {"new1", "new2", "new3"};
    int idx = 0;
    for (IECSqlValue const& elem : newTags.GetArrayIterable())
        {
        EXPECT_STREQ(expectedNew[idx], elem.GetText());
        ++idx;
        }
    EXPECT_EQ(3, idx);

    // Old stage — Tags had 2 elements.
    IECSqlValue const& oldTags = reader.GetValue(ECChangesetReader::Stage::Old, 2);
    EXPECT_STREQ("Tags", oldTags.GetColumnInfo().GetProperty()->GetName().c_str());
    ASSERT_EQ(2, oldTags.GetArrayLength());
    Utf8CP expectedOld[] = {"old1", "old2"};
    idx = 0;
    for (IECSqlValue const& elem : oldTags.GetArrayIterable())
        {
        EXPECT_STREQ(expectedOld[idx], elem.GetText());
        ++idx;
        }
    EXPECT_EQ(2, idx);

    // changedProps for UPDATE contains only "Tags" (no ECInstanceId).
    std::unordered_set<Utf8String> changedProps;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetChangedPropertyNames(changedProps));
    EXPECT_NE(changedProps.end(), changedProps.find("Tags"));
    EXPECT_EQ(changedProps.end(), changedProps.find("Name"));
    EXPECT_EQ(changedProps.end(), changedProps.find("Weight"));
    EXPECT_EQ(changedProps.end(), changedProps.find("ECInstanceId"));

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    reader.Close();
    }

//---------------------------------------------------------------------------------------
// INSERT a Widget with multiple scalars outside the tracker, then UPDATE only two of
// them (Weight and Cnt).  Verifies that the New and Old stages contain exactly those two
// scalars (plus ECInstanceId/ECClassId) and that GetChangedPropertyNames is limited to
// those two names.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECChangesetReaderTests, Update_TwoScalars)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_update_scalars.ecdb", SchemaItem(GetSchema())));

    // Full insert outside tracking.
    ECInstanceKey widgetKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "INSERT INTO ts.Widget(Name, Weight, Cnt, Active) VALUES('ScalarWidget', 1.0, 10, TRUE)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(widgetKey));
    }

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    // Update only Weight and Cnt.
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "UPDATE ts.Widget SET Weight=9.9, Cnt=99 WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, widgetKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    auto cs = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker));

    ECChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenChangeStream(m_ecdb,
        std::unique_ptr<BeSQLite::ChangeStream>(cs.release()), false,
        ECChangesetReader::Mode::All_Properties));

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());

    DbOpcode opcode;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Update, opcode);

    // ECInstanceId, ECClassId, Weight, Cnt — nothing else.
    ASSERT_EQ(4, reader.GetColumnCount(ECChangesetReader::Stage::New));
    ASSERT_EQ(4, reader.GetColumnCount(ECChangesetReader::Stage::Old));

    // New stage.
    IECSqlValue const& new_v0 = reader.GetValue(ECChangesetReader::Stage::New, 0);
    EXPECT_STREQ("ECInstanceId", new_v0.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetInstanceId(), new_v0.GetId<ECInstanceId>());

    IECSqlValue const& new_v2 = reader.GetValue(ECChangesetReader::Stage::New, 2);
    EXPECT_STREQ("Weight", new_v2.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_DOUBLE_EQ(9.9, new_v2.GetDouble());

    IECSqlValue const& new_v3 = reader.GetValue(ECChangesetReader::Stage::New, 3);
    EXPECT_STREQ("Cnt", new_v3.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(99, new_v3.GetInt64());

    // Old stage.
    IECSqlValue const& old_v2 = reader.GetValue(ECChangesetReader::Stage::Old, 2);
    EXPECT_STREQ("Weight", old_v2.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_DOUBLE_EQ(1.0, old_v2.GetDouble());

    IECSqlValue const& old_v3 = reader.GetValue(ECChangesetReader::Stage::Old, 3);
    EXPECT_STREQ("Cnt", old_v3.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(10, old_v3.GetInt64());

    // changedProps: only Weight and Cnt; Name and Active were not touched.
    std::unordered_set<Utf8String> changedProps;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetChangedPropertyNames(changedProps));
    EXPECT_NE(changedProps.end(), changedProps.find("Weight"));
    EXPECT_NE(changedProps.end(), changedProps.find("Cnt"));
    EXPECT_EQ(changedProps.end(), changedProps.find("Name"));
    EXPECT_EQ(changedProps.end(), changedProps.find("Active"));
    EXPECT_EQ(changedProps.end(), changedProps.find("ECInstanceId")); // UPDATE — never present

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    reader.Close();
    }

END_ECDBUNITTESTS_NAMESPACE
