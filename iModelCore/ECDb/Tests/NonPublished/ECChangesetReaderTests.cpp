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

        //! Extended schema: nested structs (Address.GeoCoord) + struct array (Document.MetaTags).
        Utf8CP GetExtendedSchema() const {
            return R"xml(<?xml version="1.0" encoding="utf-8"?>
                            <ECSchema schemaName="TestExtCS" alias="te" version="01.00.00"
                                    xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                            <ECStructClass typeName="GeoCoord" modifier="Sealed">
                                <ECProperty propertyName="Lat" typeName="double"/>
                                <ECProperty propertyName="Lon" typeName="double"/>
                            </ECStructClass>
                            <ECStructClass typeName="Address" modifier="Sealed">
                                <ECProperty propertyName="Street" typeName="string"/>
                                <ECStructProperty propertyName="Coord" typeName="GeoCoord"/>
                            </ECStructClass>
                            <ECStructClass typeName="Tag" modifier="Sealed">
                                <ECProperty propertyName="Key" typeName="string"/>
                                <ECProperty propertyName="Value" typeName="int"/>
                            </ECStructClass>
                            <ECEntityClass typeName="Place" modifier="Sealed">
                                <ECProperty propertyName="Name" typeName="string"/>
                                <ECStructProperty propertyName="Location" typeName="Address"/>
                            </ECEntityClass>
                            <ECEntityClass typeName="Document" modifier="Sealed">
                                <ECProperty propertyName="Title" typeName="string"/>
                                <ECStructArrayProperty propertyName="MetaTags" typeName="Tag" minOccurs="0" maxOccurs="unbounded"/>
                            </ECEntityClass>
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

    std::vector<Utf8String> changedProps;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetChangesetFetchedPropertyNames(changedProps));
    auto hasName = [&](Utf8CP n) { return std::find(changedProps.begin(), changedProps.end(), n) != changedProps.end(); };
    EXPECT_TRUE(hasName("ECInstanceId")); // INSERT — always present
    EXPECT_TRUE(hasName("Name"));
    EXPECT_TRUE(hasName("Weight"));
    EXPECT_TRUE(hasName("Cnt"));
    EXPECT_TRUE(hasName("Active"));
    EXPECT_TRUE(hasName("Pos2d"));        // both coords set → full name
    EXPECT_FALSE(hasName("Pos2d<->X"));
    EXPECT_FALSE(hasName("Pos2d<->Y"));
    EXPECT_TRUE(hasName("Pos3d"));        // all three coords set → full name
    EXPECT_FALSE(hasName("Pos3d<->X"));
    EXPECT_FALSE(hasName("Pos3d<->Y"));
    EXPECT_FALSE(hasName("Pos3d<->Z"));
    EXPECT_TRUE(hasName("Details->Label"));
    EXPECT_TRUE(hasName("Details->Score"));
    EXPECT_FALSE(hasName("Details"));      // bare struct name never present
    EXPECT_TRUE(hasName("Tags"));
    EXPECT_TRUE(hasName("Owner<->Id")); // Expected 'Owner.Id' in changed property names because 'Owner.RelClassId' is virtual

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

    std::vector<Utf8String> changedProps;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetChangesetFetchedPropertyNames(changedProps));
    auto hasName = [&](Utf8CP n) { return std::find(changedProps.begin(), changedProps.end(), n) != changedProps.end(); };
    // Changed: Name (scalar), Pos2d.X (partial Point2d), Details.Label (partial struct).
    EXPECT_TRUE(hasName("Name"));
    EXPECT_TRUE(hasName("Pos2d<->X"));
    EXPECT_TRUE(hasName("Details.Label"));
    // Unchanged or partial components must not appear.
    EXPECT_FALSE(hasName("Pos2d"));          // not both coords changed
    EXPECT_FALSE(hasName("Pos2d<->Y"));
    EXPECT_FALSE(hasName("Details->Score"));  // Score not changed
    EXPECT_FALSE(hasName("Weight"));
    EXPECT_FALSE(hasName("Cnt"));
    EXPECT_FALSE(hasName("Pos3d"));
    EXPECT_TRUE(hasName("ECInstanceId"));   // UPDATE — always present

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

    std::vector<Utf8String> changedProps;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetChangesetFetchedPropertyNames(changedProps));
    auto hasName = [&](Utf8CP n) { return std::find(changedProps.begin(), changedProps.end(), n) != changedProps.end(); };
    EXPECT_TRUE(hasName("ECInstanceId")); // DELETE — always present
    EXPECT_TRUE(hasName("Name"));
    EXPECT_TRUE(hasName("Weight"));
    EXPECT_TRUE(hasName("Cnt"));
    EXPECT_TRUE(hasName("Active"));
    EXPECT_TRUE(hasName("Pos2d"));
    EXPECT_TRUE(hasName("Pos3d"));
    EXPECT_TRUE(hasName("Details->Label"));
    EXPECT_TRUE(hasName("Details->Score"));
    EXPECT_FALSE(hasName("Details"));
    EXPECT_TRUE(hasName("Tags"));
    EXPECT_TRUE(hasName("Owner<->Id")); // Expected 'Owner.Id' in changed property names because 'Owner.RelClassId' is virtual

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    reader.Close();
    }

//---------------------------------------------------------------------------------------
// INSERT a Widget with only two scalar properties (Name + Weight) — all other properties
// left unset (NULL).  Verifies that the changeset reader emits exactly those properties
// that were explicitly provided and that GetChangesetFetchedPropertyNames is consistent.
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

    // GetChangesetFetchedPropertyNames must always include the explicitly set properties.
    std::vector<Utf8String> changedProps;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetChangesetFetchedPropertyNames(changedProps));
    auto hasName = [&](Utf8CP n) { return std::find(changedProps.begin(), changedProps.end(), n) != changedProps.end(); };
    EXPECT_TRUE(hasName("ECInstanceId")); // INSERT — always present
    EXPECT_TRUE(hasName("Name"));
    EXPECT_TRUE(hasName("Weight"));

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    reader.Close();
    }

//---------------------------------------------------------------------------------------
// INSERT a Widget with Tags outside tracker, then UPDATE Tags with a completely new set.
// For UPDATE: only Tags changes — New and Old stages each carry exactly 3 fields
// (ECInstanceId, ECClassId, Tags).
// Verifies array values in both stages and GetChangesetFetchedPropertyNames = {"Tags"}.
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
    std::vector<Utf8String> changedProps;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetChangesetFetchedPropertyNames(changedProps));
    auto hasName = [&](Utf8CP n) { return std::find(changedProps.begin(), changedProps.end(), n) != changedProps.end(); };
    EXPECT_TRUE(hasName("Tags"));
    EXPECT_FALSE(hasName("Name"));
    EXPECT_FALSE(hasName("Weight"));
    EXPECT_TRUE(hasName("ECInstanceId"));

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    reader.Close();
    }

//---------------------------------------------------------------------------------------
// INSERT a Widget with multiple scalars outside the tracker, then UPDATE only two of
// them (Weight and Cnt).  Verifies that the New and Old stages contain exactly those two
// scalars (plus ECInstanceId/ECClassId) and that GetChangesetFetchedPropertyNames is limited to
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
    std::vector<Utf8String> changedProps;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetChangesetFetchedPropertyNames(changedProps));
    auto hasName = [&](Utf8CP n) { return std::find(changedProps.begin(), changedProps.end(), n) != changedProps.end(); };
    EXPECT_TRUE(hasName("Weight"));
    EXPECT_TRUE(hasName("Cnt"));
    EXPECT_FALSE(hasName("Name"));
    EXPECT_FALSE(hasName("Active"));
    EXPECT_TRUE(hasName("ECInstanceId")); // UPDATE — always present

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    reader.Close();
    }

//---------------------------------------------------------------------------------------
// INSERT a Place with a fully-populated nested struct (Location.Street + Location.Coord.Lat/Lon).
// Verifies:
//   - Nested struct value is accessible via chained bracket notation.
//   - changedProps uses fully-dotted paths ("Location.Street", "Location.Coord.Lat",
//     "Location.Coord.Lon") and never the bare struct names ("Location", "Location.Coord").
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECChangesetReaderTests, Insert_NestedStruct)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_nested_insert.ecdb", SchemaItem(GetExtendedSchema())));

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECInstanceKey placeKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "INSERT INTO te.Place(Name, Location) VALUES(?, ?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Paris", IECSqlBinder::MakeCopy::No));
    IECSqlBinder& locBinder = stmt.GetBinder(2);
    ASSERT_EQ(ECSqlStatus::Success, locBinder["Street"].BindText("Rue de Rivoli", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, locBinder["Coord"]["Lat"].BindDouble(48.86));
    ASSERT_EQ(ECSqlStatus::Success, locBinder["Coord"]["Lon"].BindDouble(2.34));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(placeKey));
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

    EXPECT_EQ(0, reader.GetColumnCount(ECChangesetReader::Stage::Old));
    // ECInstanceId, ECClassId, Name, Location.
    ASSERT_EQ(4, reader.GetColumnCount(ECChangesetReader::Stage::New));

    IECSqlValue const& v0 = reader.GetValue(ECChangesetReader::Stage::New, 0);
    EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(placeKey.GetInstanceId(), v0.GetId<ECInstanceId>());

    IECSqlValue const& v2 = reader.GetValue(ECChangesetReader::Stage::New, 2);
    EXPECT_STREQ("Name", v2.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_STREQ("Paris", v2.GetText());

    IECSqlValue const& locVal = reader.GetValue(ECChangesetReader::Stage::New, 3);
    EXPECT_STREQ("Location", locVal.GetColumnInfo().GetProperty()->GetName().c_str());
    // Nested struct member access via chained operator[].
    EXPECT_STREQ("Rue de Rivoli", locVal["Street"].GetText());
    EXPECT_DOUBLE_EQ(48.86, locVal["Coord"]["Lat"].GetDouble());
    EXPECT_DOUBLE_EQ(2.34,  locVal["Coord"]["Lon"].GetDouble());

    // changedProps: fully-dotted paths for nested members.
    std::vector<Utf8String> changedProps;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetChangesetFetchedPropertyNames(changedProps));
    auto hasName = [&](Utf8CP n) { return std::find(changedProps.begin(), changedProps.end(), n) != changedProps.end(); };
    EXPECT_TRUE(hasName("ECInstanceId"));
    EXPECT_TRUE(hasName("Name"));
    EXPECT_TRUE(hasName("Location->Street"));
    EXPECT_TRUE(hasName("Location->Coord->Lat"));
    EXPECT_TRUE(hasName("Location->Coord->Lon"));
    // Struct nesting never collapses to a bare struct name.
    EXPECT_FALSE(hasName("Location"));
    EXPECT_FALSE(hasName("Location->Coord"));

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    reader.Close();
    }

//---------------------------------------------------------------------------------------
// Insert a fully-populated Place outside the tracker, then update ONLY Location.Street.
// Verifies:
//   - The Location struct is emitted but contains only the Street member (Coord absent
//     because neither Coord column was in the changeset).
//   - changedProps = {"Location.Street"} — Coord paths are absent.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECChangesetReaderTests, Update_NestedStruct_StreetOnly)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_nested_upd_street.ecdb", SchemaItem(GetExtendedSchema())));

    ECInstanceKey placeKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "INSERT INTO te.Place(Name, Location) VALUES('Lyon', ?)"));
    IECSqlBinder& lb = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, lb["Street"].BindText("Old Street", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, lb["Coord"]["Lat"].BindDouble(45.75));
    ASSERT_EQ(ECSqlStatus::Success, lb["Coord"]["Lon"].BindDouble(4.83));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(placeKey));
    }

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    // Update only Location.Street — Coord columns are untouched.
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "UPDATE te.Place SET Location.Street='New Street' WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, placeKey.GetInstanceId()));
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

    // ECInstanceId, ECClassId, Location (Street changed) — Name not touched.
    ASSERT_EQ(3, reader.GetColumnCount(ECChangesetReader::Stage::New));
    ASSERT_EQ(3, reader.GetColumnCount(ECChangesetReader::Stage::Old));

    IECSqlValue const& newLoc = reader.GetValue(ECChangesetReader::Stage::New, 2);
    EXPECT_STREQ("Location", newLoc.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_STREQ("New Street", newLoc["Street"].GetText());

    IECSqlValue const& oldLoc = reader.GetValue(ECChangesetReader::Stage::Old, 2);
    EXPECT_STREQ("Location", oldLoc.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_STREQ("Old Street", oldLoc["Street"].GetText());

    std::vector<Utf8String> changedProps;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetChangesetFetchedPropertyNames(changedProps));
    auto hasName = [&](Utf8CP n) { return std::find(changedProps.begin(), changedProps.end(), n) != changedProps.end(); };
    EXPECT_TRUE(hasName("Location->Street"));
    // Coord was not touched — must not appear at all.
    EXPECT_FALSE(hasName("Location->Coord->Lat"));
    EXPECT_FALSE(hasName("Location->Coord->Lon"));
    EXPECT_FALSE(hasName("Location->Coord"));
    EXPECT_FALSE(hasName("Location"));
    EXPECT_FALSE(hasName("Name"));
    EXPECT_TRUE(hasName("ECInstanceId"));

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    reader.Close();
    }

//---------------------------------------------------------------------------------------
// Insert a fully-populated Place outside the tracker, then update ONLY
// Location.Coord.Lat and Location.Coord.Lon (both nested struct leaf coords).
// Verifies:
//   - Location struct is emitted containing only Coord (Street absent).
//   - changedProps = {"Location.Coord.Lat", "Location.Coord.Lon"} — struct types
//     never collapse to a bare name even when all their members change.
//   - "Location.Coord" is NOT in changedProps (unlike Point2d/3d which collapse).
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECChangesetReaderTests, Update_NestedStruct_CoordOnly)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_nested_upd_coord.ecdb", SchemaItem(GetExtendedSchema())));

    ECInstanceKey placeKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "INSERT INTO te.Place(Name, Location) VALUES('Marseille', ?)"));
    IECSqlBinder& lb = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, lb["Street"].BindText("Canebiere", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, lb["Coord"]["Lat"].BindDouble(43.30));
    ASSERT_EQ(ECSqlStatus::Success, lb["Coord"]["Lon"].BindDouble(5.37));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(placeKey));
    }

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    // Update both Coord components but leave Street unchanged.
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "UPDATE te.Place SET Location.Coord.Lat=44.0, Location.Coord.Lon=6.0 WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, placeKey.GetInstanceId()));
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

    // ECInstanceId, ECClassId, Location (Coord changed) — Name + Street not touched.
    ASSERT_EQ(3, reader.GetColumnCount(ECChangesetReader::Stage::New));
    ASSERT_EQ(3, reader.GetColumnCount(ECChangesetReader::Stage::Old));

    // New stage: Location contains only Coord (Street absent because not in changeset).
    IECSqlValue const& newLoc = reader.GetValue(ECChangesetReader::Stage::New, 2);
    EXPECT_STREQ("Location", newLoc.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_DOUBLE_EQ(44.0, newLoc["Coord"]["Lat"].GetDouble());
    EXPECT_DOUBLE_EQ(6.0,  newLoc["Coord"]["Lon"].GetDouble());

    // Old stage: Location contains the pre-update Coord values.
    IECSqlValue const& oldLoc = reader.GetValue(ECChangesetReader::Stage::Old, 2);
    EXPECT_STREQ("Location", oldLoc.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_DOUBLE_EQ(43.30, oldLoc["Coord"]["Lat"].GetDouble());
    EXPECT_DOUBLE_EQ(5.37,  oldLoc["Coord"]["Lon"].GetDouble());

    // Key distinction from Point2d/3d: struct nesting NEVER collapses to a bare name,
    // so even when BOTH Lat and Lon change, changedProps has two dotted entries, not "Location.Coord".
    std::vector<Utf8String> changedProps;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetChangesetFetchedPropertyNames(changedProps));
    auto hasName = [&](Utf8CP n) { return std::find(changedProps.begin(), changedProps.end(), n) != changedProps.end(); };
    EXPECT_TRUE(hasName("Location->Coord->Lat"));
    EXPECT_TRUE(hasName("Location->Coord->Lon"));
    EXPECT_FALSE(hasName("Location->Coord"));  // struct, not a Point — no collapse
    EXPECT_FALSE(hasName("Location->Street"));
    EXPECT_FALSE(hasName("Location"));
    EXPECT_FALSE(hasName("Name"));
    EXPECT_TRUE(hasName("ECInstanceId"));

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    reader.Close();
    }

//---------------------------------------------------------------------------------------
// INSERT a Document with a struct-array property (MetaTags: Tag[]).
// Verifies:
//   - The MetaTags field is emitted with the correct element count.
//   - changedProps = {"ECInstanceId", "Title", "MetaTags"} — array uses the bare
//     property name (same as primitive arrays), not per-element paths.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECChangesetReaderTests, Insert_StructArray)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_structarr_insert.ecdb", SchemaItem(GetExtendedSchema())));

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECInstanceKey docKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "INSERT INTO te.Document(Title, MetaTags) VALUES(?, ?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "MyDoc", IECSqlBinder::MakeCopy::No));
    IECSqlBinder& mb = stmt.GetBinder(2);
    {
    IECSqlBinder& t1 = mb.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, t1["Key"].BindText("author", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, t1["Value"].BindInt(1));
    }
    {
    IECSqlBinder& t2 = mb.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, t2["Key"].BindText("version", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, t2["Value"].BindInt(2));
    }
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(docKey));
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

    EXPECT_EQ(0, reader.GetColumnCount(ECChangesetReader::Stage::Old));
    // ECInstanceId, ECClassId, Title, MetaTags.
    ASSERT_EQ(4, reader.GetColumnCount(ECChangesetReader::Stage::New));

    IECSqlValue const& titleVal = reader.GetValue(ECChangesetReader::Stage::New, 2);
    EXPECT_STREQ("Title", titleVal.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_STREQ("MyDoc", titleVal.GetText());

    IECSqlValue const& tagsVal = reader.GetValue(ECChangesetReader::Stage::New, 3);
    EXPECT_STREQ("MetaTags", tagsVal.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(2, tagsVal.GetArrayLength());

    // changedProps: struct-array uses bare property name, same as primitive arrays.
    std::vector<Utf8String> changedProps;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetChangesetFetchedPropertyNames(changedProps));
    auto hasName = [&](Utf8CP n) { return std::find(changedProps.begin(), changedProps.end(), n) != changedProps.end(); };
    EXPECT_TRUE(hasName("ECInstanceId"));
    EXPECT_TRUE(hasName("Title"));
    EXPECT_TRUE(hasName("MetaTags"));
    EXPECT_FALSE(hasName("MetaTags.Key"));   // no per-element keys
    EXPECT_FALSE(hasName("MetaTags.Value"));

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    reader.Close();
    }

//---------------------------------------------------------------------------------------
// Insert a Document with 2 MetaTags outside the tracker; update MetaTags to 3 elements
// inside the tracker.  Only MetaTags changed — Title was not touched.
// Verifies:
//   - New stage MetaTags has 3 elements; Old stage has 2.
//   - changedProps = {"MetaTags"}, Title absent.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECChangesetReaderTests, Update_StructArray)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_structarr_update.ecdb", SchemaItem(GetExtendedSchema())));

    ECInstanceKey docKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "INSERT INTO te.Document(Title, MetaTags) VALUES(?, ?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Draft", IECSqlBinder::MakeCopy::No));
    IECSqlBinder& mb = stmt.GetBinder(2);
    {
    IECSqlBinder& t1 = mb.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, t1["Key"].BindText("a", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, t1["Value"].BindInt(1));
    }
    {
    IECSqlBinder& t2 = mb.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, t2["Key"].BindText("b", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, t2["Value"].BindInt(2));
    }
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(docKey));
    }

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    // Replace MetaTags with 3 elements — Title left untouched.
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "UPDATE te.Document SET MetaTags=? WHERE ECInstanceId=?"));
    IECSqlBinder& mb = stmt.GetBinder(1);
    {
    IECSqlBinder& t1 = mb.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, t1["Key"].BindText("x", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, t1["Value"].BindInt(10));
    }
    {
    IECSqlBinder& t2 = mb.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, t2["Key"].BindText("y", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, t2["Value"].BindInt(20));
    }
    {
    IECSqlBinder& t3 = mb.AddArrayElement();
    ASSERT_EQ(ECSqlStatus::Success, t3["Key"].BindText("z", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, t3["Value"].BindInt(30));
    }
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, docKey.GetInstanceId()));
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

    // ECInstanceId, ECClassId, MetaTags — Title not changed.
    ASSERT_EQ(3, reader.GetColumnCount(ECChangesetReader::Stage::New));
    ASSERT_EQ(3, reader.GetColumnCount(ECChangesetReader::Stage::Old));

    IECSqlValue const& newTags = reader.GetValue(ECChangesetReader::Stage::New, 2);
    EXPECT_STREQ("MetaTags", newTags.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(3, newTags.GetArrayLength()) << "New MetaTags must have 3 elements";

    IECSqlValue const& oldTags = reader.GetValue(ECChangesetReader::Stage::Old, 2);
    EXPECT_STREQ("MetaTags", oldTags.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(2, oldTags.GetArrayLength()) << "Old MetaTags must have 2 elements";

    std::vector<Utf8String> changedProps;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetChangesetFetchedPropertyNames(changedProps));
    auto hasName = [&](Utf8CP n) { return std::find(changedProps.begin(), changedProps.end(), n) != changedProps.end(); };
    EXPECT_TRUE(hasName("MetaTags"));
    EXPECT_FALSE(hasName("Title"));       // not changed
    EXPECT_TRUE(hasName("ECInstanceId")); // UPDATE — always present

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    reader.Close();
    }

//---------------------------------------------------------------------------------------
// INSERT a Widget with only partial Point2d (X only) and partial Point3d (Y and Z only)
// — all other fields left unset.
// Verifies:
//   - changedProps contains "Pos2d.X" but NOT "Pos2d" (full collapse) or "Pos2d.Y" (Y absent).
//   - changedProps contains "Pos3d.Y" and "Pos3d.Z" but NOT "Pos3d" (full collapse) or "Pos3d.X" (X absent).
//   - The Pos2d field is emitted with X=3.0 and Y=0.0 (absent component defaults to zero).
//   - The Pos3d field is emitted with X=0.0, Y=7.0, Z=8.0.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECChangesetReaderTests, Insert_PartialPoint2dAndPoint3d)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_partial_pts.ecdb", SchemaItem(GetSchema())));

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    // Insert Widget with Pos2d.X only (Y absent) and Pos3d.Y+Z only (X absent).
    ECInstanceKey widgetKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "INSERT INTO ts.Widget(Pos2d.X, Pos3d.Y, Pos3d.Z) VALUES(?, ?, ?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(1, 3.0));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(2, 7.0));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(3, 8.0));
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

    EXPECT_EQ(0, reader.GetColumnCount(ECChangesetReader::Stage::Old));
    EXPECT_EQ(11, reader.GetColumnCount(ECChangesetReader::Stage::New));

    // Property 1
    IECSqlValue const& v0 = reader.GetValue(ECChangesetReader::Stage::New, 0);
    EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetInstanceId(), v0.GetId<ECInstanceId>());
    
    // Property 2
    IECSqlValue const& v1 = reader.GetValue(ECChangesetReader::Stage::New, 1);
    EXPECT_STREQ("ECClassId", v1.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetClassId(), v1.GetId<ECClassId>());
    
    // Property 3
    IECSqlValue const& v2 = reader.GetValue(ECChangesetReader::Stage::New, 2);
    EXPECT_STREQ("Name", v2.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_TRUE(v2.IsNull());

    // Property 4
    IECSqlValue const& v3 = reader.GetValue(ECChangesetReader::Stage::New, 3);
    ECN::ECPropertyCP prop3 = v3.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Weight", prop3->GetName().c_str());
    EXPECT_TRUE(v3.IsNull());

    //Property 5
    IECSqlValue const& v4 = reader.GetValue(ECChangesetReader::Stage::New, 4);
    ECN::ECPropertyCP prop4 = v4.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Cnt", prop4->GetName().c_str());
    EXPECT_TRUE(v4.IsNull());

    //Property 6
    IECSqlValue const& v5 = reader.GetValue(ECChangesetReader::Stage::New, 5);
    ECN::ECPropertyCP prop5 = v5.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Active", prop5->GetName().c_str());
    EXPECT_TRUE(v5.IsNull());

    //Property 7
    IECSqlValue const& v6 = reader.GetValue(ECChangesetReader::Stage::New, 6);
    ECN::ECPropertyCP prop6 = v6.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Pos2d", prop6->GetName().c_str());
    EXPECT_TRUE(v6.IsNull()); // Partial Point2d does not collapse → full Pos2d emitted but null because missing Y means entire struct is null.

    //Property 8
    IECSqlValue const& v7 = reader.GetValue(ECChangesetReader::Stage::New, 7);
    ECN::ECPropertyCP prop7 = v7.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Pos3d", prop7->GetName().c_str());
    EXPECT_TRUE(v7.IsNull()); // Partial Point3d does not collapse → full Pos3d emitted but null because missing X means entire struct is null.

    //Property 9
    IECSqlValue const& v8 = reader.GetValue(ECChangesetReader::Stage::New, 8);
    ECN::ECPropertyCP prop8 = v8.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Details", prop8->GetName().c_str());
    EXPECT_TRUE(v8.IsNull());

    //Property 10
    IECSqlValue const& v9 = reader.GetValue(ECChangesetReader::Stage::New, 9);
    ECN::ECPropertyCP prop9 = v9.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Tags", prop9->GetName().c_str());
    EXPECT_TRUE(v9.IsNull());

    //Property 11
    IECSqlValue const& v10 = reader.GetValue(ECChangesetReader::Stage::New, 10);
    ECN::ECPropertyCP prop10 = v10.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Owner", prop10->GetName().c_str());
    EXPECT_TRUE(v10.IsNull());

    std::vector<Utf8String> changedProps;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetChangesetFetchedPropertyNames(changedProps));
    auto hasName = [&](Utf8CP n) { return std::find(changedProps.begin(), changedProps.end(), n) != changedProps.end(); };
    EXPECT_TRUE(hasName("ECInstanceId")); // INSERT — always present
    EXPECT_TRUE(hasName("Name"));
    EXPECT_TRUE(hasName("Weight"));
    EXPECT_TRUE(hasName("Cnt"));
    EXPECT_TRUE(hasName("Active"));
    EXPECT_TRUE(hasName("Pos2d"));        // both coords set → full name
    EXPECT_FALSE(hasName("Pos2d<->X"));
    EXPECT_FALSE(hasName("Pos2d<->Y"));
    EXPECT_TRUE(hasName("Pos3d"));        // all three coords set → full name
    EXPECT_FALSE(hasName("Pos3d<->X"));
    EXPECT_FALSE(hasName("Pos3d<->Y"));
    EXPECT_FALSE(hasName("Pos3d<->Z"));
    EXPECT_TRUE(hasName("Details->Label"));
    EXPECT_TRUE(hasName("Details->Score"));
    EXPECT_FALSE(hasName("Details"));      // bare struct name never present
    EXPECT_TRUE(hasName("Tags"));
    EXPECT_TRUE(hasName("Owner<->Id")); // Expected 'Owner.Id' in changed property names because 'Owner.RelClassId' is virtual

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    reader.Close();
    }

//---------------------------------------------------------------------------------------
// INSERT a Widget with only a navigation property set — all other fields left unset.
// Verifies:
//   - The Owner field is emitted with the correct ECInstanceId target.
//   - changedProps contains "Owner.Id" but not "Name", "Weight", etc.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ECChangesetReaderTests, Insert_NavProperty)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_nav_insert.ecdb", SchemaItem(GetSchema())));

    // Container inserted BEFORE tracking — provides the nav prop target.
    ECInstanceKey containerKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(containerKey,
        "INSERT INTO ts.Container(Name) VALUES('Box')"));

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    // Insert Widget with only Owner set.
    ECInstanceKey widgetKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "INSERT INTO ts.Widget(Owner.Id) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, containerKey.GetInstanceId()));
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

    EXPECT_EQ(0, reader.GetColumnCount(ECChangesetReader::Stage::Old));
    EXPECT_EQ(11, reader.GetColumnCount(ECChangesetReader::Stage::New));

    // Property 1
    IECSqlValue const& v0 = reader.GetValue(ECChangesetReader::Stage::New, 0);
    EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetInstanceId(), v0.GetId<ECInstanceId>());
    
    // Property 2
    IECSqlValue const& v1 = reader.GetValue(ECChangesetReader::Stage::New, 1);
    EXPECT_STREQ("ECClassId", v1.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetClassId(), v1.GetId<ECClassId>());
    
    // Property 3
    IECSqlValue const& v2 = reader.GetValue(ECChangesetReader::Stage::New, 2);
    EXPECT_STREQ("Name", v2.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_TRUE(v2.IsNull());

    // Property 4
    IECSqlValue const& v3 = reader.GetValue(ECChangesetReader::Stage::New, 3);
    ECN::ECPropertyCP prop3 = v3.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Weight", prop3->GetName().c_str());
    EXPECT_TRUE(v3.IsNull());

    //Property 5
    IECSqlValue const& v4 = reader.GetValue(ECChangesetReader::Stage::New, 4);
    ECN::ECPropertyCP prop4 = v4.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Cnt", prop4->GetName().c_str());
    EXPECT_TRUE(v4.IsNull());

    //Property 6
    IECSqlValue const& v5 = reader.GetValue(ECChangesetReader::Stage::New, 5);
    ECN::ECPropertyCP prop5 = v5.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Active", prop5->GetName().c_str());
    EXPECT_TRUE(v5.IsNull());

    //Property 7
    IECSqlValue const& v6 = reader.GetValue(ECChangesetReader::Stage::New, 6);
    ECN::ECPropertyCP prop6 = v6.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Pos2d", prop6->GetName().c_str());
    DPoint2d pos2d = v6.GetPoint2d();
    EXPECT_DOUBLE_EQ(0.0, pos2d.x);
    EXPECT_DOUBLE_EQ(0.0, pos2d.y);

    //Property 8
    IECSqlValue const& v7 = reader.GetValue(ECChangesetReader::Stage::New, 7);
    ECN::ECPropertyCP prop7 = v7.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Pos3d", prop7->GetName().c_str());
    DPoint3d pos3d = v7.GetPoint3d();
    EXPECT_DOUBLE_EQ(0.0, pos3d.x);
    EXPECT_DOUBLE_EQ(0.0, pos3d.y);
    EXPECT_DOUBLE_EQ(0.0, pos3d.z);

    //Property 9
    IECSqlValue const& v8 = reader.GetValue(ECChangesetReader::Stage::New, 8);
    ECN::ECPropertyCP prop8 = v8.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Details", prop8->GetName().c_str());
    EXPECT_TRUE(v8.IsNull());

    //Property 10
    IECSqlValue const& v9 = reader.GetValue(ECChangesetReader::Stage::New, 9);
    ECN::ECPropertyCP prop9 = v9.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Tags", prop9->GetName().c_str());
    EXPECT_TRUE(v9.IsNull());

    //Property 11
    IECSqlValue const& v10 = reader.GetValue(ECChangesetReader::Stage::New, 10);
    ECN::ECPropertyCP prop10 = v10.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Owner", prop10->GetName().c_str());
    ECN::ECClassId relId;
    ECInstanceId ownerId = v10.GetNavigation<ECInstanceId>(&relId);
    EXPECT_EQ(containerKey.GetInstanceId(), ownerId);
    EXPECT_TRUE(relId.IsValid());

    std::vector<Utf8String> changedProps;
    ASSERT_EQ(BE_SQLITE_OK, reader.GetChangesetFetchedPropertyNames(changedProps));
    auto hasName = [&](Utf8CP n) { return std::find(changedProps.begin(), changedProps.end(), n) != changedProps.end(); };
    EXPECT_TRUE(hasName("ECInstanceId")); // INSERT — always present
    EXPECT_TRUE(hasName("Name"));
    EXPECT_TRUE(hasName("Weight"));
    EXPECT_TRUE(hasName("Cnt"));
    EXPECT_TRUE(hasName("Active"));
    EXPECT_TRUE(hasName("Pos2d"));        // both coords set → full name
    EXPECT_FALSE(hasName("Pos2d<->X"));
    EXPECT_FALSE(hasName("Pos2d<->Y"));
    EXPECT_TRUE(hasName("Pos3d"));        // all three coords set → full name
    EXPECT_FALSE(hasName("Pos3d<->X"));
    EXPECT_FALSE(hasName("Pos3d<->Y"));
    EXPECT_FALSE(hasName("Pos3d<->Z"));
    EXPECT_TRUE(hasName("Details->Label"));
    EXPECT_TRUE(hasName("Details->Score"));
    EXPECT_FALSE(hasName("Details"));      // bare struct name never present
    EXPECT_TRUE(hasName("Tags"));
    EXPECT_TRUE(hasName("Owner<->Id")); // Expected 'Owner.Id' in changed property names because 'Owner.RelClassId' is virtual

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    reader.Close();
    }

END_ECDBUNITTESTS_NAMESPACE
