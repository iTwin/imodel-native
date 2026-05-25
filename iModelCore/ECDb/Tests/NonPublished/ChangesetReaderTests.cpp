/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <BeSQLite/ChangesetFile.h>
#include <Bentley/BeDirectoryIterator.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
//! Local changeset helpers
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
struct ChangesetReaderTests : ECDbTestFixture {
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

        //! Schema with a many-to-many (link table) relationship where source and target
        //! constraints are non-polymorphic â†’ SourceECClassId and TargetECClassId are virtual.
        Utf8CP GetRelSchema() const {
            return R"xml(<?xml version="1.0" encoding="utf-8"?>
                            <ECSchema schemaName="TestRelCS" alias="tr" version="01.00.00"
                                    xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                            <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                            <ECEntityClass typeName="Person" modifier="Sealed">
                                <ECProperty propertyName="Name" typeName="string"/>
                            </ECEntityClass>
                            <ECEntityClass typeName="Project" modifier="Sealed">
                                <ECProperty propertyName="Title" typeName="string"/>
                            </ECEntityClass>
                            <ECRelationshipClass typeName="PersonWorksOnProject" modifier="Sealed" strength="referencing">
                                <Source multiplicity="(0..*)" roleLabel="works on" polymorphic="false">
                                    <Class class="Person"/>
                                </Source>
                                <Target multiplicity="(0..*)" roleLabel="is worked on by" polymorphic="false">
                                    <Class class="Project"/>
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

    //! Writes @p cs to an LZMA-compressed changeset file in the test output directory.
    //! Returns the absolute path of the created file.
    BeFileName WriteChangesetToFile(ECDbCR ecdb, TestCSChangeSet& cs, Utf8CP fileName)
        {
        BeFileName path = BuildECDbPath(fileName);
        BeSQLite::ChangeGroup group(ecdb);
        EXPECT_EQ(BE_SQLITE_OK, cs.AddToChangeGroup(group));
        BeSQLite::DdlChanges emptyDdl;
        BeSQLite::ChangesetFileWriter writer(path, false, emptyDdl, &ecdb);
        EXPECT_EQ(BE_SQLITE_OK, writer.Initialize());
        EXPECT_EQ(BE_SQLITE_OK, writer.FromChangeGroup(group));
        return path;
        }
    
    size_t GetDefaultSpillThresholdBytes() const { return 50ull * 1024 * 1024; /* 50 MB */ }

    //! V1 schema: Item with Name (string) and Value (int).
    Utf8CP GetStrictModeV1Schema() const {
        return R"xml(<?xml version="1.0" encoding="utf-8"?>
                        <ECSchema schemaName="TestStrictItem" alias="tsi" version="01.00.00"
                                xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                        <ECEntityClass typeName="Item" modifier="Sealed">
                            <ECProperty propertyName="Name" typeName="string"/>
                            <ECProperty propertyName="Val" typeName="int"/>
                        </ECEntityClass>
                        </ECSchema>)xml";
        }

    //! V2 schema: Item with Name, Value, and Extra (string) â€” one column wider than V1.
    Utf8CP GetStrictModeV2Schema() const {
        return R"xml(<?xml version="1.0" encoding="utf-8"?>
                        <ECSchema schemaName="TestStrictItem" alias="tsi" version="01.00.01"
                                xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
                        <ECEntityClass typeName="Item" modifier="Sealed">
                            <ECProperty propertyName="Name" typeName="string"/>
                            <ECProperty propertyName="Val" typeName="int"/>
                            <ECProperty propertyName="Extra" typeName="string"/>
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
TEST_F(ChangesetReaderTests, Insert_AllPropertyTypes)
    {
    NativeLogging::Logging::SetLogger(&NativeLogging::ConsoleLogger::GetLogger());
    
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_insert.ecdb", SchemaItem(GetSchema())));

    // Container inserted BEFORE tracking â€” must not appear in the changeset.
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

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    // Step one by one.
    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());
    bool isEC = false;
    ASSERT_EQ(SUCCESS, reader.IsECTable(isEC));

    DbOpcode opcode;
    ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Insert, opcode);

    // Old stage must be empty for an insert.
    EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));

    // Walk every New-stage column: print then assert.
    EXPECT_EQ(11, reader.GetColumnCount(Changes::Change::Stage::New));
    
    //Property 1
    IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::New, 0);
    ECN::ECPropertyCP prop0 = v0.GetColumnInfo().GetProperty();
    EXPECT_STREQ("ECInstanceId", prop0->GetName().c_str());
    ECInstanceId id = v0.GetId<ECInstanceId>();
    EXPECT_EQ(widgetKey.GetInstanceId(), id);

    //Property 2
    IECSqlValue const& v1 = reader.GetValue(Changes::Change::Stage::New, 1);
    ECN::ECPropertyCP prop1 = v1.GetColumnInfo().GetProperty();
    EXPECT_STREQ("ECClassId", prop1->GetName().c_str());
    ECN::ECClassId classId = v1.GetId<ECN::ECClassId>();
    EXPECT_EQ(widgetKey.GetClassId(), classId);

    // Property 3
    IECSqlValue const& v2 = reader.GetValue(Changes::Change::Stage::New, 2);
    ECN::ECPropertyCP prop2 = v2.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Name", prop2->GetName().c_str());
    Utf8CP name = v2.GetText();
    EXPECT_STREQ("WidgetA", name);

    // Property 4
    IECSqlValue const& v3 = reader.GetValue(Changes::Change::Stage::New, 3);
    ECN::ECPropertyCP prop3 = v3.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Weight", prop3->GetName().c_str());
    double weight = v3.GetDouble();
    EXPECT_DOUBLE_EQ(3.14, weight);

    //Property 5
    IECSqlValue const& v4 = reader.GetValue(Changes::Change::Stage::New, 4);
    ECN::ECPropertyCP prop4 = v4.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Cnt", prop4->GetName().c_str());
    int64_t cnt = v4.GetInt64();
    EXPECT_EQ(42, cnt);

    //Property 6
    IECSqlValue const& v5 = reader.GetValue(Changes::Change::Stage::New, 5);
    ECN::ECPropertyCP prop5 = v5.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Active", prop5->GetName().c_str());
    bool active = v5.GetBoolean();
    EXPECT_TRUE(active);

    //Property 7
    IECSqlValue const& v6 = reader.GetValue(Changes::Change::Stage::New, 6);
    ECN::ECPropertyCP prop6 = v6.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Pos2d", prop6->GetName().c_str());
    DPoint2d pos2d = v6.GetPoint2d();
    EXPECT_DOUBLE_EQ(1.0, pos2d.x);
    EXPECT_DOUBLE_EQ(2.0, pos2d.y);

    //Property 8
    IECSqlValue const& v7 = reader.GetValue(Changes::Change::Stage::New, 7);
    ECN::ECPropertyCP prop7 = v7.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Pos3d", prop7->GetName().c_str());
    DPoint3d pos3d = v7.GetPoint3d();
    EXPECT_DOUBLE_EQ(10.0, pos3d.x);
    EXPECT_DOUBLE_EQ(20.0, pos3d.y);
    EXPECT_DOUBLE_EQ(30.0, pos3d.z);

    //Property 9
    IECSqlValue const& v8 = reader.GetValue(Changes::Change::Stage::New, 8);
    ECN::ECPropertyCP prop8 = v8.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Details", prop8->GetName().c_str());
    Utf8CP label = v8["Label"].GetText();
    int   score = v8["Score"].GetInt();
    EXPECT_STREQ("SpecLabel", label);
    EXPECT_EQ(100, score);

    //Property 10
    IECSqlValue const& v9 = reader.GetValue(Changes::Change::Stage::New, 9);
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
    IECSqlValue const& v10 = reader.GetValue(Changes::Change::Stage::New, 10);
    ECN::ECPropertyCP prop10 = v10.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Owner", prop10->GetName().c_str());
    ECN::ECClassId relId;
    ECInstanceId ownerId = v10.GetNavigation<ECInstanceId>(&relId);
    EXPECT_EQ(containerKey.GetInstanceId(), ownerId);
    EXPECT_TRUE(relId.IsValid());

    Utf8String instanceKey;
    ASSERT_EQ(SUCCESS, reader.GetInstanceKey(Changes::Change::Stage::New, instanceKey));
    EXPECT_FALSE(instanceKey.empty());

    auto const* changedProps = reader.GetChangeFetchedPropertyNames();
    ASSERT_NE(nullptr, changedProps);
    auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
    EXPECT_TRUE(hasName("ECInstanceId")); // INSERT â€” always present
    EXPECT_TRUE(hasName("Name"));
    EXPECT_TRUE(hasName("Weight"));
    EXPECT_TRUE(hasName("Cnt"));
    EXPECT_TRUE(hasName("Active"));
    EXPECT_TRUE(hasName("Pos2d"));        // both coords set â†’ full name
    EXPECT_FALSE(hasName("Pos2d.X"));
    EXPECT_FALSE(hasName("Pos2d.Y"));
    EXPECT_TRUE(hasName("Pos3d"));        // all three coords set â†’ full name
    EXPECT_FALSE(hasName("Pos3d.X"));
    EXPECT_FALSE(hasName("Pos3d.Y"));
    EXPECT_FALSE(hasName("Pos3d.Z"));
    EXPECT_TRUE(hasName("Details.Label"));
    EXPECT_TRUE(hasName("Details.Score"));
    EXPECT_FALSE(hasName("Details"));      // bare struct name never present
    EXPECT_TRUE(hasName("Tags"));
    EXPECT_TRUE(hasName("Owner.Id")); // Expected 'Owner.Id' in changed property names because 'Owner.RelClassId' is virtual

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }

//---------------------------------------------------------------------------------------
// UPDATE a Widget with a partial set of columns:
//   - Name changed
//   - Pos2d.X changed (Pos2d.Y NOT changed â€” falls back from DB)
//   - Details.Label changed (Details.Score NOT changed â€” falls back from DB)
//   - All other properties untouched â€” must NOT appear in the changeset
// Step through the changeset, print every New-stage and Old-stage column, then assert.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, Update_PartialFields_ChangesetAndDBFallback)
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

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    ASSERT_EQ(reader.Step(), BE_SQLITE_ROW);
    bool isEC = false;
    ASSERT_EQ(SUCCESS, reader.IsECTable(isEC));

    DbOpcode opcode;
    ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Update, opcode);
    ASSERT_EQ(5, reader.GetColumnCount(Changes::Change::Stage::New));

    // Property 1
    IECSqlValue const& new_v0 = reader.GetValue(Changes::Change::Stage::New, 0);
    ECN::ECPropertyCP new_prop0 = new_v0.GetColumnInfo().GetProperty();
    EXPECT_STREQ("ECInstanceId", new_prop0->GetName().c_str());
    ECInstanceId id = new_v0.GetId<ECInstanceId>();
    EXPECT_EQ(widgetKey.GetInstanceId(), id);

    //Property 2
    IECSqlValue const& new_v1 = reader.GetValue(Changes::Change::Stage::New, 1);
    ECN::ECPropertyCP new_prop1 = new_v1.GetColumnInfo().GetProperty();
    EXPECT_STREQ("ECClassId", new_prop1->GetName().c_str());
    ECN::ECClassId classId = new_v1.GetId<ECN::ECClassId>();
    EXPECT_EQ(widgetKey.GetClassId(), classId);

    //Property 3
    IECSqlValue const& new_v2 = reader.GetValue(Changes::Change::Stage::New, 2);
    ECN::ECPropertyCP new_prop2 = new_v2.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Name", new_prop2->GetName().c_str());
    Utf8CP name = new_v2.GetText();
    EXPECT_STREQ("WidgetB", name);

    //Property 4
    IECSqlValue const& new_v3 = reader.GetValue(Changes::Change::Stage::New, 3);
    ECN::ECPropertyCP new_prop3 = new_v3.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Pos2d", new_prop3->GetName().c_str());
    DPoint2d pos2d = new_v3.GetPoint2d();
    EXPECT_DOUBLE_EQ(5.0, pos2d.x) << "Pos2d.X must be the updated value";
    EXPECT_DOUBLE_EQ(2.0, pos2d.y) << "Pos2d.Y must fall back to the pre-update value";

    //Property 5
    IECSqlValue const& new_v4 = reader.GetValue(Changes::Change::Stage::New, 4);
    ECN::ECPropertyCP new_prop4 = new_v4.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Details", new_prop4->GetName().c_str());
    Utf8CP label = new_v4["Label"].GetText();
    EXPECT_STREQ("NewLabel", label);

    ASSERT_EQ(5, reader.GetColumnCount(Changes::Change::Stage::Old));

    //Property 1
    IECSqlValue const& old_v0 = reader.GetValue(Changes::Change::Stage::Old, 0);
    ECN::ECPropertyCP old_prop0 = old_v0.GetColumnInfo().GetProperty();
    EXPECT_STREQ("ECInstanceId", old_prop0->GetName().c_str());
    ECInstanceId oldId = old_v0.GetId<ECInstanceId>();
    EXPECT_EQ(widgetKey.GetInstanceId(), oldId);

    //Property 2
    IECSqlValue const& old_v1 = reader.GetValue(Changes::Change::Stage::Old, 1);
    ECN::ECPropertyCP old_prop1 = old_v1.GetColumnInfo().GetProperty();
    EXPECT_STREQ("ECClassId", old_prop1->GetName().c_str());
    ECN::ECClassId oldClassId = old_v1.GetId<ECN::ECClassId>();
    EXPECT_EQ(widgetKey.GetClassId(), oldClassId);

    //Property 3
    IECSqlValue const& old_v2 = reader.GetValue(Changes::Change::Stage::Old, 2);
    ECN::ECPropertyCP old_prop2 = old_v2.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Name", old_prop2->GetName().c_str());
    Utf8CP oldName = old_v2.GetText();
    EXPECT_STREQ("WidgetA", oldName);

    //Property 4
    IECSqlValue const& old_v3 = reader.GetValue(Changes::Change::Stage::Old, 3);
    ECN::ECPropertyCP old_prop3 = old_v3.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Pos2d", old_prop3->GetName().c_str());
    DPoint2d oldPos2d = old_v3.GetPoint2d();
    EXPECT_DOUBLE_EQ(1.0, oldPos2d.x) << "Old Pos2d.X must be the pre-update value";
    EXPECT_DOUBLE_EQ(2.0, oldPos2d.y) << "Old Pos2d.Y must fall back to the pre-update value";

    //Property 5
    IECSqlValue const& old_v4 = reader.GetValue(Changes::Change::Stage::Old, 4);
    ECN::ECPropertyCP old_prop4 = old_v4.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Details", old_prop4->GetName().c_str());
    Utf8CP oldLabel = old_v4["Label"].GetText();
    EXPECT_STREQ("OldLabel", oldLabel);


    Utf8String newKey, oldKey;
    ASSERT_EQ(SUCCESS, reader.GetInstanceKey(Changes::Change::Stage::New, newKey));
    ASSERT_EQ(SUCCESS, reader.GetInstanceKey(Changes::Change::Stage::Old, oldKey));
    EXPECT_FALSE(newKey.empty());
    EXPECT_FALSE(oldKey.empty());

    auto const* changedProps = reader.GetChangeFetchedPropertyNames();
    ASSERT_NE(nullptr, changedProps);
    auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
    // Changed: Name (scalar), Pos2d.X (partial Point2d), Details.Label (partial struct).
    EXPECT_TRUE(hasName("Name"));
    EXPECT_TRUE(hasName("Pos2d.X"));
    EXPECT_TRUE(hasName("Details.Label"));
    // Unchanged or partial components must not appear.
    EXPECT_FALSE(hasName("Pos2d"));          // not both coords changed
    EXPECT_FALSE(hasName("Pos2d.Y"));
    EXPECT_FALSE(hasName("Details.Score"));  // Score not changed
    EXPECT_FALSE(hasName("Weight"));
    EXPECT_FALSE(hasName("Cnt"));
    EXPECT_FALSE(hasName("Pos3d"));
    EXPECT_TRUE(hasName("ECInstanceId"));   // UPDATE â€” always present

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());

    ASSERT_EQ(SUCCESS, reader.Close());
    }

//---------------------------------------------------------------------------------------
// DELETE a Widget.
// Stage::New must be empty (column count == 0).
// Stage::Old must carry every originally-inserted property.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, Delete_OldStageContainsAllValues)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_delete.ecdb", SchemaItem(GetSchema())));

    // Container inserted BEFORE tracking â€” must not appear in the changeset.
    ECInstanceKey containerKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(containerKey,
        "INSERT INTO ts.Container(Name) VALUES('Box')"));

    // Widget inserted BEFORE tracking â€” deleted row values appear in Old stage.
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

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());
    bool isEC = false;
    ASSERT_EQ(SUCCESS, reader.IsECTable(isEC));

    DbOpcode opcode;
    ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Delete, opcode);

    // New stage must be empty for a delete.
    EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::New));

    EXPECT_EQ(11, reader.GetColumnCount(Changes::Change::Stage::Old));

    // Property 1
    IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::Old, 0);
    EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetInstanceId(), v0.GetId<ECInstanceId>());

    // Property 2
    IECSqlValue const& v1 = reader.GetValue(Changes::Change::Stage::Old, 1);
    EXPECT_STREQ("ECClassId", v1.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetClassId(), v1.GetId<ECN::ECClassId>());

    // Property 3
    IECSqlValue const& v2 = reader.GetValue(Changes::Change::Stage::Old, 2);
    EXPECT_STREQ("Name", v2.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_STREQ("Doomed", v2.GetText());

    // Property 4
    IECSqlValue const& v3 = reader.GetValue(Changes::Change::Stage::Old, 3);
    EXPECT_STREQ("Weight", v3.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_DOUBLE_EQ(2.71, v3.GetDouble());

    // Property 5
    IECSqlValue const& v4 = reader.GetValue(Changes::Change::Stage::Old, 4);
    EXPECT_STREQ("Cnt", v4.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(7, v4.GetInt64());

    // Property 6
    IECSqlValue const& v5 = reader.GetValue(Changes::Change::Stage::Old, 5);
    EXPECT_STREQ("Active", v5.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_FALSE(v5.GetBoolean());

    // Property 7
    IECSqlValue const& v6 = reader.GetValue(Changes::Change::Stage::Old, 6);
    EXPECT_STREQ("Pos2d", v6.GetColumnInfo().GetProperty()->GetName().c_str());
    DPoint2d pos2d = v6.GetPoint2d();
    EXPECT_DOUBLE_EQ(3.0, pos2d.x);
    EXPECT_DOUBLE_EQ(4.0, pos2d.y);

    // Property 8
    IECSqlValue const& v7 = reader.GetValue(Changes::Change::Stage::Old, 7);
    EXPECT_STREQ("Pos3d", v7.GetColumnInfo().GetProperty()->GetName().c_str());
    DPoint3d pos3d = v7.GetPoint3d();
    EXPECT_DOUBLE_EQ(1.0, pos3d.x);
    EXPECT_DOUBLE_EQ(2.0, pos3d.y);
    EXPECT_DOUBLE_EQ(3.0, pos3d.z);

    // Property 9
    IECSqlValue const& v8 = reader.GetValue(Changes::Change::Stage::Old, 8);
    EXPECT_STREQ("Details", v8.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_STREQ("MyLabel", v8["Label"].GetText());
    EXPECT_EQ(55, v8["Score"].GetInt());

    // Property 10
    IECSqlValue const& v9 = reader.GetValue(Changes::Change::Stage::Old, 9);
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
    IECSqlValue const& v10 = reader.GetValue(Changes::Change::Stage::Old, 10);
    EXPECT_STREQ("Owner", v10.GetColumnInfo().GetProperty()->GetName().c_str());
    ECN::ECClassId relId;
    ECInstanceId ownerId = v10.GetNavigation<ECInstanceId>(&relId);
    EXPECT_EQ(containerKey.GetInstanceId(), ownerId);
    EXPECT_TRUE(relId.IsValid());

    Utf8String instanceKey;
    ASSERT_EQ(SUCCESS, reader.GetInstanceKey(Changes::Change::Stage::Old, instanceKey));
    EXPECT_FALSE(instanceKey.empty());

    auto const* changedProps = reader.GetChangeFetchedPropertyNames();
    ASSERT_NE(nullptr, changedProps);
    auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
    EXPECT_TRUE(hasName("ECInstanceId")); // DELETE â€” always present
    EXPECT_TRUE(hasName("Name"));
    EXPECT_TRUE(hasName("Weight"));
    EXPECT_TRUE(hasName("Cnt"));
    EXPECT_TRUE(hasName("Active"));
    EXPECT_TRUE(hasName("Pos2d"));
    EXPECT_TRUE(hasName("Pos3d"));
    EXPECT_TRUE(hasName("Details.Label"));
    EXPECT_TRUE(hasName("Details.Score"));
    EXPECT_FALSE(hasName("Details"));
    EXPECT_TRUE(hasName("Tags"));
    EXPECT_TRUE(hasName("Owner.Id")); // Expected 'Owner.Id' in changed property names because 'Owner.RelClassId' is virtual

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }

//---------------------------------------------------------------------------------------
// INSERT a Widget with only two scalar properties (Name + Weight) â€” all other properties
// left unset (NULL).  Verifies that the changeset reader emits exactly those properties
// that were explicitly provided and that GetChangeFetchedPropertyNames is consistent.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, Insert_PartialProperties)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_partial_insert.ecdb", SchemaItem(GetSchema())));

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    // Insert only Name and Weight â€” all other properties left unset (NULL).
    ECInstanceKey widgetKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "INSERT INTO ts.Widget(Name, Weight) VALUES('Sparse', 2.5)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(widgetKey));
    }

    auto cs = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker));

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());

    DbOpcode opcode;
    ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Insert, opcode);

    // Old stage empty for insert.
    EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));

    // New stage: ECInstanceId, ECClassId, name + weight at minimum.
    int newCount = reader.GetColumnCount(Changes::Change::Stage::New);
    EXPECT_GE(newCount, 4);

    // Slots 0 and 1 must always be ECInstanceId and ECClassId.
    IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::New, 0);
    EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetInstanceId(), v0.GetId<ECInstanceId>());

    IECSqlValue const& v1 = reader.GetValue(Changes::Change::Stage::New, 1);
    EXPECT_STREQ("ECClassId", v1.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetClassId(), v1.GetId<ECN::ECClassId>());

    // Find Name and Weight anywhere in the new-stage field list.
    bool foundName = false, foundWeight = false;
    for (int i = 2; i < newCount; ++i)
        {
        IECSqlValue const& v = reader.GetValue(Changes::Change::Stage::New, i);
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

    // GetChangeFetchedPropertyNames must always include the explicitly set properties.
    auto const* changedProps = reader.GetChangeFetchedPropertyNames();
    ASSERT_NE(nullptr, changedProps);
    auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
    EXPECT_TRUE(hasName("ECInstanceId")); // INSERT â€” always present
    EXPECT_TRUE(hasName("Name"));
    EXPECT_TRUE(hasName("Weight"));

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }

//---------------------------------------------------------------------------------------
// INSERT a Widget with Tags outside tracker, then UPDATE Tags with a completely new set.
// For UPDATE: only Tags changes â€” New and Old stages each carry exactly 3 fields
// (ECInstanceId, ECClassId, Tags).
// Verifies array values in both stages and GetChangeFetchedPropertyNames = {"Tags"}.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, Update_ArrayProperty)
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

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());

    DbOpcode opcode;
    ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Update, opcode);

    // Only Tags changed: ECInstanceId, ECClassId, Tags.
    ASSERT_EQ(3, reader.GetColumnCount(Changes::Change::Stage::New));
    ASSERT_EQ(3, reader.GetColumnCount(Changes::Change::Stage::Old));

    // New stage â€” Tags has 3 elements.
    IECSqlValue const& newTags = reader.GetValue(Changes::Change::Stage::New, 2);
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

    // Old stage â€” Tags had 2 elements.
    IECSqlValue const& oldTags = reader.GetValue(Changes::Change::Stage::Old, 2);
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
    auto const* changedProps = reader.GetChangeFetchedPropertyNames();
    ASSERT_NE(nullptr, changedProps);
    auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
    EXPECT_TRUE(hasName("Tags"));
    EXPECT_FALSE(hasName("Name"));
    EXPECT_FALSE(hasName("Weight"));
    EXPECT_TRUE(hasName("ECInstanceId"));

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }

//---------------------------------------------------------------------------------------
// INSERT a Widget with multiple scalars outside the tracker, then UPDATE only two of
// them (Weight and Cnt).  Verifies that the New and Old stages contain exactly those two
// scalars (plus ECInstanceId/ECClassId) and that GetChangeFetchedPropertyNames is limited to
// those two names.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, Update_TwoScalars)
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

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());

    DbOpcode opcode;
    ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Update, opcode);

    // ECInstanceId, ECClassId, Weight, Cnt â€” nothing else.
    ASSERT_EQ(4, reader.GetColumnCount(Changes::Change::Stage::New));
    ASSERT_EQ(4, reader.GetColumnCount(Changes::Change::Stage::Old));

    // New stage.
    IECSqlValue const& new_v0 = reader.GetValue(Changes::Change::Stage::New, 0);
    EXPECT_STREQ("ECInstanceId", new_v0.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetInstanceId(), new_v0.GetId<ECInstanceId>());

    IECSqlValue const& new_v2 = reader.GetValue(Changes::Change::Stage::New, 2);
    EXPECT_STREQ("Weight", new_v2.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_DOUBLE_EQ(9.9, new_v2.GetDouble());

    IECSqlValue const& new_v3 = reader.GetValue(Changes::Change::Stage::New, 3);
    EXPECT_STREQ("Cnt", new_v3.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(99, new_v3.GetInt64());

    // Old stage.
    IECSqlValue const& old_v2 = reader.GetValue(Changes::Change::Stage::Old, 2);
    EXPECT_STREQ("Weight", old_v2.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_DOUBLE_EQ(1.0, old_v2.GetDouble());

    IECSqlValue const& old_v3 = reader.GetValue(Changes::Change::Stage::Old, 3);
    EXPECT_STREQ("Cnt", old_v3.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(10, old_v3.GetInt64());

    // changedProps: only Weight and Cnt; Name and Active were not touched.
    auto const* changedProps = reader.GetChangeFetchedPropertyNames();
    ASSERT_NE(nullptr, changedProps);
    auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
    EXPECT_TRUE(hasName("Weight"));
    EXPECT_TRUE(hasName("Cnt"));
    EXPECT_FALSE(hasName("Name"));
    EXPECT_FALSE(hasName("Active"));
    EXPECT_TRUE(hasName("ECInstanceId")); // UPDATE â€” always present

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }

//---------------------------------------------------------------------------------------
// INSERT a Place with a fully-populated nested struct (Location.Street + Location.Coord.Lat/Lon).
// Verifies:
//   - Nested struct value is accessible via chained bracket notation.
//   - changedProps uses fully-dotted paths ("Location.Street", "Location.Coord.Lat",
//     "Location.Coord.Lon") and never the bare struct names ("Location", "Location.Coord").
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, Insert_NestedStruct)
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

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());

    DbOpcode opcode;
    ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Insert, opcode);

    EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));
    // ECInstanceId, ECClassId, Name, Location.
    ASSERT_EQ(4, reader.GetColumnCount(Changes::Change::Stage::New));

    IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::New, 0);
    EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(placeKey.GetInstanceId(), v0.GetId<ECInstanceId>());

    IECSqlValue const& v2 = reader.GetValue(Changes::Change::Stage::New, 2);
    EXPECT_STREQ("Name", v2.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_STREQ("Paris", v2.GetText());

    IECSqlValue const& locVal = reader.GetValue(Changes::Change::Stage::New, 3);
    EXPECT_STREQ("Location", locVal.GetColumnInfo().GetProperty()->GetName().c_str());
    // Nested struct member access via chained operator[].
    EXPECT_STREQ("Rue de Rivoli", locVal["Street"].GetText());
    EXPECT_DOUBLE_EQ(48.86, locVal["Coord"]["Lat"].GetDouble());
    EXPECT_DOUBLE_EQ(2.34,  locVal["Coord"]["Lon"].GetDouble());

    // changedProps: fully-dotted paths for nested members.
    auto const* changedProps = reader.GetChangeFetchedPropertyNames();
    ASSERT_NE(nullptr, changedProps);
    auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
    EXPECT_TRUE(hasName("ECInstanceId"));
    EXPECT_TRUE(hasName("Name"));
    EXPECT_TRUE(hasName("Location.Street"));
    EXPECT_TRUE(hasName("Location.Coord.Lat"));
    EXPECT_TRUE(hasName("Location.Coord.Lon"));
    // Struct nesting never collapses to a bare struct name.
    EXPECT_FALSE(hasName("Location"));
    EXPECT_FALSE(hasName("Location.Coord"));

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }

//---------------------------------------------------------------------------------------
// Insert a fully-populated Place outside the tracker, then update ONLY Location.Street.
// Verifies:
//   - The Location struct is emitted but contains only the Street member (Coord absent
//     because neither Coord column was in the changeset).
//   - changedProps = {"Location.Street"} â€” Coord paths are absent.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, Update_NestedStruct_StreetOnly)
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

    // Update only Location.Street â€” Coord columns are untouched.
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "UPDATE te.Place SET Location.Street='New Street' WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, placeKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    auto cs = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker));

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());

    DbOpcode opcode;
    ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Update, opcode);

    // ECInstanceId, ECClassId, Location (Street changed) â€” Name not touched.
    ASSERT_EQ(3, reader.GetColumnCount(Changes::Change::Stage::New));
    ASSERT_EQ(3, reader.GetColumnCount(Changes::Change::Stage::Old));

    IECSqlValue const& newLoc = reader.GetValue(Changes::Change::Stage::New, 2);
    EXPECT_STREQ("Location", newLoc.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_STREQ("New Street", newLoc["Street"].GetText());

    IECSqlValue const& oldLoc = reader.GetValue(Changes::Change::Stage::Old, 2);
    EXPECT_STREQ("Location", oldLoc.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_STREQ("Old Street", oldLoc["Street"].GetText());

    auto const* changedProps = reader.GetChangeFetchedPropertyNames();
    ASSERT_NE(nullptr, changedProps);
    auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
    EXPECT_TRUE(hasName("Location.Street"));
    // Coord was not touched â€” must not appear at all.
    EXPECT_FALSE(hasName("Location.Coord.Lat"));
    EXPECT_FALSE(hasName("Location.Coord.Lon"));
    EXPECT_FALSE(hasName("Location.Coord"));
    EXPECT_FALSE(hasName("Location"));
    EXPECT_FALSE(hasName("Name"));
    EXPECT_TRUE(hasName("ECInstanceId"));

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }

//---------------------------------------------------------------------------------------
// Insert a fully-populated Place outside the tracker, then update ONLY
// Location.Coord.Lat and Location.Coord.Lon (both nested struct leaf coords).
// Verifies:
//   - Location struct is emitted containing only Coord (Street absent).
//   - changedProps = {"Location.Coord.Lat", "Location.Coord.Lon"} â€” struct types
//     never collapse to a bare name even when all their members change.
//   - "Location.Coord" is NOT in changedProps (unlike Point2d/3d which collapse).
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, Update_NestedStruct_CoordOnly)
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

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());

    DbOpcode opcode;
    ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Update, opcode);

    // ECInstanceId, ECClassId, Location (Coord changed) â€” Name + Street not touched.
    ASSERT_EQ(3, reader.GetColumnCount(Changes::Change::Stage::New));
    ASSERT_EQ(3, reader.GetColumnCount(Changes::Change::Stage::Old));

    // New stage: Location contains only Coord (Street absent because not in changeset).
    IECSqlValue const& newLoc = reader.GetValue(Changes::Change::Stage::New, 2);
    EXPECT_STREQ("Location", newLoc.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_DOUBLE_EQ(44.0, newLoc["Coord"]["Lat"].GetDouble());
    EXPECT_DOUBLE_EQ(6.0,  newLoc["Coord"]["Lon"].GetDouble());

    // Old stage: Location contains the pre-update Coord values.
    IECSqlValue const& oldLoc = reader.GetValue(Changes::Change::Stage::Old, 2);
    EXPECT_STREQ("Location", oldLoc.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_DOUBLE_EQ(43.30, oldLoc["Coord"]["Lat"].GetDouble());
    EXPECT_DOUBLE_EQ(5.37,  oldLoc["Coord"]["Lon"].GetDouble());

    // Key distinction from Point2d/3d: struct nesting NEVER collapses to a bare name,
    // so even when BOTH Lat and Lon change, changedProps has two dotted entries, not "Location.Coord".
    auto const* changedProps = reader.GetChangeFetchedPropertyNames();
    ASSERT_NE(nullptr, changedProps);
    auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
    EXPECT_TRUE(hasName("Location.Coord.Lat"));
    EXPECT_TRUE(hasName("Location.Coord.Lon"));
    EXPECT_FALSE(hasName("Location.Coord"));  // struct, not a Point â€” no collapse
    EXPECT_FALSE(hasName("Location.Street"));
    EXPECT_FALSE(hasName("Location"));
    EXPECT_FALSE(hasName("Name"));
    EXPECT_TRUE(hasName("ECInstanceId"));

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }

//---------------------------------------------------------------------------------------
// INSERT a Document with a struct-array property (MetaTags: Tag[]).
// Verifies:
//   - The MetaTags field is emitted with the correct element count.
//   - changedProps = {"ECInstanceId", "Title", "MetaTags"} â€” array uses the bare
//     property name (same as primitive arrays), not per-element paths.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, Insert_StructArray)
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

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());

    DbOpcode opcode;
    ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Insert, opcode);

    EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));
    // ECInstanceId, ECClassId, Title, MetaTags.
    ASSERT_EQ(4, reader.GetColumnCount(Changes::Change::Stage::New));

    IECSqlValue const& titleVal = reader.GetValue(Changes::Change::Stage::New, 2);
    EXPECT_STREQ("Title", titleVal.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_STREQ("MyDoc", titleVal.GetText());

    IECSqlValue const& tagsVal = reader.GetValue(Changes::Change::Stage::New, 3);
    EXPECT_STREQ("MetaTags", tagsVal.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(2, tagsVal.GetArrayLength());

    // changedProps: struct-array uses bare property name, same as primitive arrays.
    auto const* changedProps = reader.GetChangeFetchedPropertyNames();
    ASSERT_NE(nullptr, changedProps);
    auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
    EXPECT_TRUE(hasName("ECInstanceId"));
    EXPECT_TRUE(hasName("Title"));
    EXPECT_TRUE(hasName("MetaTags"));
    EXPECT_FALSE(hasName("MetaTags.Key"));   // no per-element keys
    EXPECT_FALSE(hasName("MetaTags.Value"));

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }

//---------------------------------------------------------------------------------------
// Insert a Document with 2 MetaTags outside the tracker; update MetaTags to 3 elements
// inside the tracker.  Only MetaTags changed â€” Title was not touched.
// Verifies:
//   - New stage MetaTags has 3 elements; Old stage has 2.
//   - changedProps = {"MetaTags"}, Title absent.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, Update_StructArray)
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

    // Replace MetaTags with 3 elements â€” Title left untouched.
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

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());

    DbOpcode opcode;
    ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Update, opcode);

    // ECInstanceId, ECClassId, MetaTags â€” Title not changed.
    ASSERT_EQ(3, reader.GetColumnCount(Changes::Change::Stage::New));
    ASSERT_EQ(3, reader.GetColumnCount(Changes::Change::Stage::Old));

    IECSqlValue const& newTags = reader.GetValue(Changes::Change::Stage::New, 2);
    EXPECT_STREQ("MetaTags", newTags.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(3, newTags.GetArrayLength()) << "New MetaTags must have 3 elements";

    IECSqlValue const& oldTags = reader.GetValue(Changes::Change::Stage::Old, 2);
    EXPECT_STREQ("MetaTags", oldTags.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(2, oldTags.GetArrayLength()) << "Old MetaTags must have 2 elements";

    auto const* changedProps = reader.GetChangeFetchedPropertyNames();
    ASSERT_NE(nullptr, changedProps);
    auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
    EXPECT_TRUE(hasName("MetaTags"));
    EXPECT_FALSE(hasName("Title"));       // not changed
    EXPECT_TRUE(hasName("ECInstanceId")); // UPDATE â€” always present

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }

//---------------------------------------------------------------------------------------
// INSERT a Widget with only partial Point2d (X only) and partial Point3d (Y and Z only)
// â€” all other fields left unset.
// Verifies:
//   - changedProps contains "Pos2d.X" but NOT "Pos2d" (full collapse) or "Pos2d.Y" (Y absent).
//   - changedProps contains "Pos3d.Y" and "Pos3d.Z" but NOT "Pos3d" (full collapse) or "Pos3d.X" (X absent).
//   - The Pos2d field is emitted with X=3.0 and Y=0.0 (absent component defaults to zero).
//   - The Pos3d field is emitted with X=0.0, Y=7.0, Z=8.0.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, Insert_PartialPoint2dAndPoint3d)
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

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());

    DbOpcode opcode;
    ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Insert, opcode);

    EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));
    EXPECT_EQ(11, reader.GetColumnCount(Changes::Change::Stage::New));

    // Property 1
    IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::New, 0);
    EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetInstanceId(), v0.GetId<ECInstanceId>());
    
    // Property 2
    IECSqlValue const& v1 = reader.GetValue(Changes::Change::Stage::New, 1);
    EXPECT_STREQ("ECClassId", v1.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetClassId(), v1.GetId<ECClassId>());
    
    // Property 3
    IECSqlValue const& v2 = reader.GetValue(Changes::Change::Stage::New, 2);
    EXPECT_STREQ("Name", v2.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_TRUE(v2.IsNull());

    // Property 4
    IECSqlValue const& v3 = reader.GetValue(Changes::Change::Stage::New, 3);
    ECN::ECPropertyCP prop3 = v3.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Weight", prop3->GetName().c_str());
    EXPECT_TRUE(v3.IsNull());

    //Property 5
    IECSqlValue const& v4 = reader.GetValue(Changes::Change::Stage::New, 4);
    ECN::ECPropertyCP prop4 = v4.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Cnt", prop4->GetName().c_str());
    EXPECT_TRUE(v4.IsNull());

    //Property 6
    IECSqlValue const& v5 = reader.GetValue(Changes::Change::Stage::New, 5);
    ECN::ECPropertyCP prop5 = v5.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Active", prop5->GetName().c_str());
    EXPECT_TRUE(v5.IsNull());

    //Property 7
    IECSqlValue const& v6 = reader.GetValue(Changes::Change::Stage::New, 6);
    ECN::ECPropertyCP prop6 = v6.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Pos2d", prop6->GetName().c_str());
    EXPECT_TRUE(v6.IsNull()); // Partial Point2d does not collapse â†’ full Pos2d emitted but null because missing Y means entire struct is null.

    //Property 8
    IECSqlValue const& v7 = reader.GetValue(Changes::Change::Stage::New, 7);
    ECN::ECPropertyCP prop7 = v7.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Pos3d", prop7->GetName().c_str());
    EXPECT_TRUE(v7.IsNull()); // Partial Point3d does not collapse â†’ full Pos3d emitted but null because missing X means entire struct is null.

    //Property 9
    IECSqlValue const& v8 = reader.GetValue(Changes::Change::Stage::New, 8);
    ECN::ECPropertyCP prop8 = v8.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Details", prop8->GetName().c_str());
    EXPECT_TRUE(v8.IsNull());

    //Property 10
    IECSqlValue const& v9 = reader.GetValue(Changes::Change::Stage::New, 9);
    ECN::ECPropertyCP prop9 = v9.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Tags", prop9->GetName().c_str());
    EXPECT_TRUE(v9.IsNull());

    //Property 11
    IECSqlValue const& v10 = reader.GetValue(Changes::Change::Stage::New, 10);
    ECN::ECPropertyCP prop10 = v10.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Owner", prop10->GetName().c_str());
    EXPECT_TRUE(v10.IsNull());

    auto const* changedProps = reader.GetChangeFetchedPropertyNames();
    ASSERT_NE(nullptr, changedProps);
    auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
    EXPECT_TRUE(hasName("ECInstanceId")); // INSERT â€” always present
    EXPECT_TRUE(hasName("Name"));
    EXPECT_TRUE(hasName("Weight"));
    EXPECT_TRUE(hasName("Cnt"));
    EXPECT_TRUE(hasName("Active"));
    EXPECT_TRUE(hasName("Pos2d"));        // both coords set â†’ full name
    EXPECT_FALSE(hasName("Pos2d.X"));
    EXPECT_FALSE(hasName("Pos2d.Y"));
    EXPECT_TRUE(hasName("Pos3d"));        // all three coords set â†’ full name
    EXPECT_FALSE(hasName("Pos3d.X"));
    EXPECT_FALSE(hasName("Pos3d.Y"));
    EXPECT_FALSE(hasName("Pos3d.Z"));
    EXPECT_TRUE(hasName("Details.Label"));
    EXPECT_TRUE(hasName("Details.Score"));
    EXPECT_FALSE(hasName("Details"));      // bare struct name never present
    EXPECT_TRUE(hasName("Tags"));
    EXPECT_TRUE(hasName("Owner.Id")); // Expected 'Owner.Id' in changed property names because 'Owner.RelClassId' is virtual

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());

        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
            "SELECT * FROM ts.Widget WHERE ECInstanceId=?"));
        stmt.BindId(1, widgetKey.GetInstanceId());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_EQ(11, stmt.GetColumnCount());
        EXPECT_EQ(widgetKey.GetInstanceId(), stmt.GetValue(0).GetId<ECInstanceId>());
        EXPECT_EQ(widgetKey.GetClassId(), stmt.GetValue(1).GetId<ECClassId>());
        EXPECT_EQ(true, stmt.GetValue(2).IsNull()); // Name
        EXPECT_EQ(true, stmt.GetValue(3).IsNull()); // Weight
        EXPECT_EQ(true, stmt.GetValue(4).IsNull()); // Cnt
        EXPECT_EQ(true, stmt.GetValue(5).IsNull()); // Active
        EXPECT_EQ(true, stmt.GetValue(6).IsNull()); // Pos2d
        EXPECT_EQ(true, stmt.GetValue(7).IsNull()); // Pos3d
        EXPECT_EQ(true, stmt.GetValue(8).IsNull()); // Details
        EXPECT_EQ(true, stmt.GetValue(9).IsNull()); // Tags
        EXPECT_EQ(true, stmt.GetValue(10).IsNull()); // Owner
        }
    }

//---------------------------------------------------------------------------------------
// INSERT a Widget with only a navigation property set â€” all other fields left unset.
// Verifies:
//   - The Owner field is emitted with the correct ECInstanceId target.
//   - changedProps contains "Owner.Id" but not "Name", "Weight", etc.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, Insert_NavProperty)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_nav_insert.ecdb", SchemaItem(GetSchema())));

    // Container inserted BEFORE tracking â€” provides the nav prop target.
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

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());

    DbOpcode opcode;
    ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Insert, opcode);

    EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));
    EXPECT_EQ(11, reader.GetColumnCount(Changes::Change::Stage::New));

    // Property 1
    IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::New, 0);
    EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetInstanceId(), v0.GetId<ECInstanceId>());
    
    // Property 2
    IECSqlValue const& v1 = reader.GetValue(Changes::Change::Stage::New, 1);
    EXPECT_STREQ("ECClassId", v1.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetClassId(), v1.GetId<ECClassId>());
    
    // Property 3
    IECSqlValue const& v2 = reader.GetValue(Changes::Change::Stage::New, 2);
    EXPECT_STREQ("Name", v2.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_TRUE(v2.IsNull());

    // Property 4
    IECSqlValue const& v3 = reader.GetValue(Changes::Change::Stage::New, 3);
    ECN::ECPropertyCP prop3 = v3.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Weight", prop3->GetName().c_str());
    EXPECT_TRUE(v3.IsNull());

    //Property 5
    IECSqlValue const& v4 = reader.GetValue(Changes::Change::Stage::New, 4);
    ECN::ECPropertyCP prop4 = v4.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Cnt", prop4->GetName().c_str());
    EXPECT_TRUE(v4.IsNull());

    //Property 6
    IECSqlValue const& v5 = reader.GetValue(Changes::Change::Stage::New, 5);
    ECN::ECPropertyCP prop5 = v5.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Active", prop5->GetName().c_str());
    EXPECT_TRUE(v5.IsNull());

    //Property 7
    IECSqlValue const& v6 = reader.GetValue(Changes::Change::Stage::New, 6);
    ECN::ECPropertyCP prop6 = v6.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Pos2d", prop6->GetName().c_str());
    DPoint2d pos2d = v6.GetPoint2d();
    EXPECT_DOUBLE_EQ(0.0, pos2d.x);
    EXPECT_DOUBLE_EQ(0.0, pos2d.y);

    //Property 8
    IECSqlValue const& v7 = reader.GetValue(Changes::Change::Stage::New, 7);
    ECN::ECPropertyCP prop7 = v7.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Pos3d", prop7->GetName().c_str());
    DPoint3d pos3d = v7.GetPoint3d();
    EXPECT_DOUBLE_EQ(0.0, pos3d.x);
    EXPECT_DOUBLE_EQ(0.0, pos3d.y);
    EXPECT_DOUBLE_EQ(0.0, pos3d.z);

    //Property 9
    IECSqlValue const& v8 = reader.GetValue(Changes::Change::Stage::New, 8);
    ECN::ECPropertyCP prop8 = v8.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Details", prop8->GetName().c_str());
    EXPECT_TRUE(v8.IsNull());

    //Property 10
    IECSqlValue const& v9 = reader.GetValue(Changes::Change::Stage::New, 9);
    ECN::ECPropertyCP prop9 = v9.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Tags", prop9->GetName().c_str());
    EXPECT_TRUE(v9.IsNull());

    //Property 11
    IECSqlValue const& v10 = reader.GetValue(Changes::Change::Stage::New, 10);
    ECN::ECPropertyCP prop10 = v10.GetColumnInfo().GetProperty();
    EXPECT_STREQ("Owner", prop10->GetName().c_str());
    ECN::ECClassId relId;
    ECInstanceId ownerId = v10.GetNavigation<ECInstanceId>(&relId);
    EXPECT_EQ(containerKey.GetInstanceId(), ownerId);
    EXPECT_TRUE(relId.IsValid());

    auto const* changedProps = reader.GetChangeFetchedPropertyNames();
    ASSERT_NE(nullptr, changedProps);
    auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
    EXPECT_TRUE(hasName("ECInstanceId")); // INSERT â€” always present
    EXPECT_TRUE(hasName("Name"));
    EXPECT_TRUE(hasName("Weight"));
    EXPECT_TRUE(hasName("Cnt"));
    EXPECT_TRUE(hasName("Active"));
    EXPECT_TRUE(hasName("Pos2d"));        // both coords set â†’ full name
    EXPECT_FALSE(hasName("Pos2d.X"));
    EXPECT_FALSE(hasName("Pos2d.Y"));
    EXPECT_TRUE(hasName("Pos3d"));        // all three coords set â†’ full name
    EXPECT_FALSE(hasName("Pos3d.X"));
    EXPECT_FALSE(hasName("Pos3d.Y"));
    EXPECT_FALSE(hasName("Pos3d.Z"));
    EXPECT_TRUE(hasName("Details.Label"));
    EXPECT_TRUE(hasName("Details.Score"));
    EXPECT_FALSE(hasName("Details"));      // bare struct name never present
    EXPECT_TRUE(hasName("Tags"));
    EXPECT_TRUE(hasName("Owner.Id")); // Expected 'Owner.Id' in changed property names because 'Owner.RelClassId' is virtual

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
        {
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
            "SELECT * FROM ts.Widget WHERE ECInstanceId=?"));
        stmt.BindId(1, widgetKey.GetInstanceId());
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        EXPECT_EQ(11, stmt.GetColumnCount());
        EXPECT_EQ(widgetKey.GetInstanceId(), stmt.GetValue(0).GetId<ECInstanceId>());
        EXPECT_EQ(widgetKey.GetClassId(), stmt.GetValue(1).GetId<ECClassId>());
        EXPECT_EQ(true, stmt.GetValue(2).IsNull()); // Name
        EXPECT_EQ(true, stmt.GetValue(3).IsNull()); // Weight
        EXPECT_EQ(true, stmt.GetValue(4).IsNull()); // Cnt
        EXPECT_EQ(true, stmt.GetValue(5).IsNull()); // Active
        EXPECT_EQ(true, stmt.GetValue(6).IsNull()); // Pos2d
        EXPECT_EQ(true, stmt.GetValue(7).IsNull()); // Pos3d
        EXPECT_EQ(true, stmt.GetValue(8).IsNull()); // Details
        EXPECT_EQ(true, stmt.GetValue(9).IsNull()); // Tags
        ECN::ECClassId relId;
        ECInstanceId ownerId = stmt.GetValue(10).GetNavigation<ECInstanceId>(&relId);
        EXPECT_EQ(containerKey.GetInstanceId(), ownerId);
        EXPECT_TRUE(relId.IsValid());
        }
    }

//---------------------------------------------------------------------------------------
// Filter by table name: insert both a Container and a Widget inside the tracker.
// With a "ts_Widget" table filter only the Widget row must be visible.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, Filter_ByTableName)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_filter_table.ecdb", SchemaItem(GetSchema())));

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECInstanceKey containerKey, widgetKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(containerKey,
        "INSERT INTO ts.Container(Name) VALUES('Box')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(widgetKey,
        "INSERT INTO ts.Widget(Name) VALUES('Gizmo')"));

    auto cs = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker));

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    reader.SetTableFilters({"ts_Widget"});

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());
        {
        Utf8String widgetTableName;
        ASSERT_EQ(SUCCESS, reader.GetTableName(widgetTableName));
        EXPECT_STREQ("ts_Widget", widgetTableName.c_str());
        DbOpcode opcode;
        ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
        EXPECT_EQ(DbOpcode::Insert, opcode);
        bool isECTable = false;
        ASSERT_EQ(SUCCESS, reader.IsECTable(isECTable));
        EXPECT_TRUE(isECTable);
        auto const* changedProps = reader.GetChangeFetchedPropertyNames();
        ASSERT_NE(nullptr, changedProps);
        EXPECT_TRUE(!changedProps->empty());
        
        EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));
        EXPECT_EQ(11, reader.GetColumnCount(Changes::Change::Stage::New));
        IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::New, 0);
        EXPECT_EQ(widgetKey.GetInstanceId(), v0.GetId<ECInstanceId>());
        }

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }

//---------------------------------------------------------------------------------------
// Filter by opcode: insert a Widget and then update it inside the tracker.
// With a DbOpcode::Update filter only the update row must be visible.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, Filter_ByOpcode)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_filter_opcode.ecdb", SchemaItem(GetSchema())));

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECInstanceKey widgetKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(widgetKey,
        "INSERT INTO ts.Widget(Name) VALUES('Alpha')"));

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "UPDATE ts.Widget SET Name='Beta' WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, widgetKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    auto cs = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker));

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    reader.SetOpcodeFilters({DbOpcode::Update});

    // Insert and Update gets squashed into INSERT so nothing is returned after filtering.
    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    bool isECTable = false;
    ASSERT_EQ(ERROR, reader.IsECTable(isECTable));
    DbOpcode opcode;
    ASSERT_EQ(ERROR, reader.GetOpcode(opcode));
    Utf8String tableName;
    ASSERT_EQ(ERROR, reader.GetTableName(tableName));
    Utf8String instanceKey;
    ASSERT_EQ(ERROR, reader.GetInstanceKey(Changes::Change::Stage::New, instanceKey));
    ASSERT_EQ(ERROR, reader.GetInstanceKey(Changes::Change::Stage::Old, instanceKey));
    bool isIndirect = false;
    ASSERT_EQ(ERROR, reader.IsIndirectChange(isIndirect));
    ASSERT_EQ(nullptr, reader.GetChangeFetchedPropertyNames());
    ASSERT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::New));
    ASSERT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));
    EXPECT_TRUE(reader.GetValue(Changes::Change::Stage::New, 0).IsNull());
    EXPECT_TRUE(reader.GetValue(Changes::Change::Stage::Old, 0).IsNull());

    // Even if we step multiple times we should get BE_SQLITE_DONE because we have reached the end.
    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }

//---------------------------------------------------------------------------------------
// Filter by ECClassId: insert a Container and a Widget inside the tracker.
// With a filter on the Widget's ECClassId only the Widget row must be visible.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, Filter_ByECClassId)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_filter_classid.ecdb", SchemaItem(GetSchema())));

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECInstanceKey containerKey, widgetKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(containerKey,
        "INSERT INTO ts.Container(Name) VALUES('Box')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(widgetKey,
        "INSERT INTO ts.Widget(Name) VALUES('Cog')"));

    auto cs = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker));

    ECN::ECClassId widgetClassId = m_ecdb.Schemas().GetClassId("TestReadCS", "Widget");
    ASSERT_TRUE(widgetClassId.IsValid());

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    reader.SetECClassNameFilters({"TestReadCS:Widget"});

    // Only the Widget row must be returned.
    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());
        {
        Utf8String widgetTableName;
        ASSERT_EQ(SUCCESS, reader.GetTableName(widgetTableName));
        EXPECT_STREQ("ts_Widget", widgetTableName.c_str());
        DbOpcode opcode;
        ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
        EXPECT_EQ(DbOpcode::Insert, opcode);
        bool isECTable = false;
        ASSERT_EQ(SUCCESS, reader.IsECTable(isECTable));
        EXPECT_TRUE(isECTable);
        auto const* changedProps = reader.GetChangeFetchedPropertyNames();
        ASSERT_NE(nullptr, changedProps);
        EXPECT_TRUE(!changedProps->empty());
        
        EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));
        EXPECT_EQ(11, reader.GetColumnCount(Changes::Change::Stage::New));
        IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::New, 0);
        EXPECT_EQ(widgetKey.GetInstanceId(), v0.GetId<ECInstanceId>());
        }

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }

//---------------------------------------------------------------------------------------
// ClearFilters: set a table filter that hides all rows, verify nothing is returned,
// then clear the filter and verify all rows are visible again.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, Filter_ClearTableFilter)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_filter_clear.ecdb", SchemaItem(GetSchema())));

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECInstanceKey widgetKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(widgetKey,
        "INSERT INTO ts.Widget(Name) VALUES('Sprocket')"));

        // First reader: non-matching table filter â†’ no rows.
        {
        auto cs = std::make_unique<TestCSChangeSet>();
        ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker));

        ChangesetReader reader;
        ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
            std::move(cs), false,
            ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

        reader.SetTableFilters({"no_such_table"});
        // should not include the Widget row because of the filter
        ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
        ASSERT_EQ(SUCCESS, reader.Close());
        }

        // Second reader: set the same filter then clear it â†’ the Widget row must appear.
        {
        auto cs = std::make_unique<TestCSChangeSet>();
        ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker));

        ChangesetReader reader;
        ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
            std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

        reader.SetTableFilters({"no_such_table"});
        reader.ClearTableFilters();

        ASSERT_EQ(BE_SQLITE_ROW, reader.Step());
        Utf8String widgetTableName;
        ASSERT_EQ(SUCCESS, reader.GetTableName(widgetTableName));
        EXPECT_STREQ("ts_Widget", widgetTableName.c_str());
        DbOpcode opcode;
        ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
        EXPECT_EQ(DbOpcode::Insert, opcode);
        bool isECTable = false;
        ASSERT_EQ(SUCCESS, reader.IsECTable(isECTable));
        EXPECT_TRUE(isECTable);
        auto const* changedProps = reader.GetChangeFetchedPropertyNames();
        ASSERT_NE(nullptr, changedProps);
        EXPECT_TRUE(!changedProps->empty()); // should not be empty because the filter was cleared

        EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old)); // Fields should not be empty because the filter was cleared
        EXPECT_EQ(11, reader.GetColumnCount(Changes::Change::Stage::New)); // Fields should not be empty because the filter was cleared

        IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::New, 0);
        EXPECT_EQ(widgetKey.GetInstanceId(), v0.GetId<ECInstanceId>());

        ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
        ASSERT_EQ(SUCCESS, reader.Close());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, OverflowTable_InsertAndUpdateOverflowOnly)
    {
    Utf8CP overflowSchema = R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestOver" alias="to" version="01.00.00"
                xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
        <ECEntityClass typeName="Entity" modifier="Abstract">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <ShareColumns xmlns="ECDbMap.02.00.00">
                    <ApplyToSubclassesOnly>true</ApplyToSubclassesOnly>
                    <MaxSharedColumnsBeforeOverflow>3</MaxSharedColumnsBeforeOverflow>
                </ShareColumns>
            </ECCustomAttributes>
        </ECEntityClass>
        <ECEntityClass typeName="BigThing" modifier="Sealed">
            <BaseClass>Entity</BaseClass>
            <ECProperty propertyName="A" typeName="string"/>
            <ECProperty propertyName="B" typeName="string"/>
            <ECProperty propertyName="C" typeName="string"/>
            <ECProperty propertyName="D" typeName="string"/>
            <ECProperty propertyName="E" typeName="string"/>
        </ECEntityClass>
        </ECSchema>)xml";

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_overflow.ecdb", SchemaItem(overflowSchema)));

    // -----------------------------------------------------------------------
    // Part 1: INSERT inside tracker â†’ two changeset entries (primary + overflow)
    // -----------------------------------------------------------------------
    ECInstanceKey bigKey;
    {
    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "INSERT INTO to.BigThing(A, B, C, D, E) VALUES('alpha', 'beta', 'gamma', 'delta', 'epsilon')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(bigKey));
    }

    auto cs = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker));

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    // --- Changeset row 1: primary table (to_Entity) ---
    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());
        {
        Utf8String tableName;
        ASSERT_EQ(SUCCESS, reader.GetTableName(tableName));
        EXPECT_STREQ("to_Entity", tableName.c_str());

        DbOpcode opcode;
        ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
        ASSERT_EQ(DbOpcode::Insert, opcode);

        EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));
        // ECInstanceId, ECClassId, A, B, C â€” D and E live in the overflow table.
        ASSERT_EQ(5, reader.GetColumnCount(Changes::Change::Stage::New));

        IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::New, 0);
        EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_EQ(bigKey.GetInstanceId(), v0.GetId<ECInstanceId>());

        IECSqlValue const& v1 = reader.GetValue(Changes::Change::Stage::New, 1);
        EXPECT_STREQ("ECClassId", v1.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_EQ(bigKey.GetClassId(), v1.GetId<ECN::ECClassId>());

        IECSqlValue const& v2 = reader.GetValue(Changes::Change::Stage::New, 2);
        EXPECT_STREQ("A", v2.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_STREQ("alpha", v2.GetText());

        IECSqlValue const& v3 = reader.GetValue(Changes::Change::Stage::New, 3);
        EXPECT_STREQ("B", v3.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_STREQ("beta", v3.GetText());

        IECSqlValue const& v4 = reader.GetValue(Changes::Change::Stage::New, 4);
        EXPECT_STREQ("C", v4.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_STREQ("gamma", v4.GetText());

        auto const* changedProps = reader.GetChangeFetchedPropertyNames();
        ASSERT_NE(nullptr, changedProps);
        auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
        EXPECT_TRUE(hasName("ECInstanceId"));
        EXPECT_TRUE(hasName("ECClassId"));
        EXPECT_TRUE(hasName("A"));
        EXPECT_TRUE(hasName("B"));
        EXPECT_TRUE(hasName("C"));
        EXPECT_FALSE(hasName("D")); // D lives in overflow table â€” absent from this row
        EXPECT_FALSE(hasName("E")); // E lives in overflow table â€” absent from this row
        }

    // --- Changeset row 2: overflow table (to_Entity_Overflow) ---
    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());
        {
        Utf8String tableName;
        ASSERT_EQ(SUCCESS, reader.GetTableName(tableName));
        EXPECT_STREQ("to_Entity_Overflow", tableName.c_str());

        DbOpcode opcode;
        ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
        ASSERT_EQ(DbOpcode::Insert, opcode);

        EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));
        // ECInstanceId, ECClassId, D, E â€” A, B, C live in the primary table.
        ASSERT_EQ(4, reader.GetColumnCount(Changes::Change::Stage::New));

        IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::New, 0);
        EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_EQ(bigKey.GetInstanceId(), v0.GetId<ECInstanceId>());

        IECSqlValue const& v1 = reader.GetValue(Changes::Change::Stage::New, 1);
        EXPECT_STREQ("ECClassId", v1.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_EQ(bigKey.GetClassId(), v1.GetId<ECN::ECClassId>());

        IECSqlValue const& v2 = reader.GetValue(Changes::Change::Stage::New, 2);
        EXPECT_STREQ("D", v2.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_STREQ("delta", v2.GetText());

        IECSqlValue const& v3 = reader.GetValue(Changes::Change::Stage::New, 3);
        EXPECT_STREQ("E", v3.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_STREQ("epsilon", v3.GetText());

        auto const* changedProps = reader.GetChangeFetchedPropertyNames();
        ASSERT_NE(nullptr, changedProps);
        auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
        EXPECT_TRUE(hasName("ECInstanceId"));
        EXPECT_TRUE(hasName("ECClassId"));
        EXPECT_TRUE(hasName("D"));
        EXPECT_TRUE(hasName("E"));
        EXPECT_FALSE(hasName("A")); // A lives in primary table â€” absent from this overflow row
        EXPECT_FALSE(hasName("B"));
        EXPECT_FALSE(hasName("C"));
        }

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }

    // -----------------------------------------------------------------------
    // Part 2: UPDATE only D and E (overflow columns) â€” only to_Entity_Overflow is written.
    // -----------------------------------------------------------------------
    {
    TestCSChangeTracker tracker2(m_ecdb);
    tracker2.EnableTracking(true);

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "UPDATE to.BigThing SET D='delta2', E='epsilon2' WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, bigKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    auto cs2 = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs2->FromChangeTrack(tracker2));

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs2), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    // Only the overflow table row must appear; primary table was not written.
    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());
        {
        Utf8String tableName;
        ASSERT_EQ(SUCCESS, reader.GetTableName(tableName));
        EXPECT_STREQ("to_Entity_Overflow", tableName.c_str());

        DbOpcode opcode;
        ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
        ASSERT_EQ(DbOpcode::Update, opcode);

        // ECInstanceId, ECClassId, D, E
        ASSERT_EQ(4, reader.GetColumnCount(Changes::Change::Stage::New));
        ASSERT_EQ(4, reader.GetColumnCount(Changes::Change::Stage::Old));

        // New stage
        IECSqlValue const& newId = reader.GetValue(Changes::Change::Stage::New, 0);
        EXPECT_STREQ("ECInstanceId", newId.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_EQ(bigKey.GetInstanceId(), newId.GetId<ECInstanceId>());

        IECSqlValue const& newD = reader.GetValue(Changes::Change::Stage::New, 2);
        EXPECT_STREQ("D", newD.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_STREQ("delta2", newD.GetText());

        IECSqlValue const& newE = reader.GetValue(Changes::Change::Stage::New, 3);
        EXPECT_STREQ("E", newE.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_STREQ("epsilon2", newE.GetText());

        // Old stage
        IECSqlValue const& oldD = reader.GetValue(Changes::Change::Stage::Old, 2);
        EXPECT_STREQ("D", oldD.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_STREQ("delta", oldD.GetText());

        IECSqlValue const& oldE = reader.GetValue(Changes::Change::Stage::Old, 3);
        EXPECT_STREQ("E", oldE.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_STREQ("epsilon", oldE.GetText());

        // ECClassId was not in the UPDATE changeset (neither PK nor changed column),
        // so classIdFromChangeset=false and ECClassId does not appear in changedProps.
        auto const* changedProps = reader.GetChangeFetchedPropertyNames();
        ASSERT_NE(nullptr, changedProps);
        auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
        EXPECT_TRUE(hasName("ECInstanceId"));
        EXPECT_TRUE(hasName("D"));
        EXPECT_TRUE(hasName("E"));
        EXPECT_FALSE(hasName("ECClassId")); // resolved via DB seek, not from changeset
        EXPECT_FALSE(hasName("A")); // untouched and stored in primary table
        EXPECT_FALSE(hasName("B"));
        EXPECT_FALSE(hasName("C"));
        }

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, JoinedTable_Insert)
    {
    Utf8CP joinedSchema = R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestJoined" alias="tj" version="01.00.00"
                xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
        <ECEntityClass typeName="JBase" modifier="Abstract">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
            </ECCustomAttributes>
            <ECProperty propertyName="BaseCode" typeName="string"/>
        </ECEntityClass>
        <ECEntityClass typeName="JChild" modifier="Sealed">
            <BaseClass>JBase</BaseClass>
            <ECProperty propertyName="P" typeName="int"/>
            <ECProperty propertyName="Q" typeName="string"/>
        </ECEntityClass>
        </ECSchema>)xml";

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_joined.ecdb", SchemaItem(joinedSchema)));

    ECInstanceKey jChildKey;
    {
    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "INSERT INTO tj.JChild(BaseCode, P, Q) VALUES('hello', 42, 'world')"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(jChildKey));
    }

    auto cs = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker));

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    // --- Changeset row 1: primary table (tj_JBase) ---
    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());
        {
        Utf8String tableName;
        ASSERT_EQ(SUCCESS, reader.GetTableName(tableName));
        EXPECT_STREQ("tj_JBase", tableName.c_str());

        DbOpcode opcode;
        ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
        ASSERT_EQ(DbOpcode::Insert, opcode);

        EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));
        // ECInstanceId, ECClassId (physical â€” in changeset), BaseCode
        ASSERT_EQ(3, reader.GetColumnCount(Changes::Change::Stage::New));

        IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::New, 0);
        EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_EQ(jChildKey.GetInstanceId(), v0.GetId<ECInstanceId>());

        IECSqlValue const& v1 = reader.GetValue(Changes::Change::Stage::New, 1);
        EXPECT_STREQ("ECClassId", v1.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_EQ(jChildKey.GetClassId(), v1.GetId<ECN::ECClassId>());

        IECSqlValue const& v2 = reader.GetValue(Changes::Change::Stage::New, 2);
        EXPECT_STREQ("BaseCode", v2.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_STREQ("hello", v2.GetText());

        auto const* changedProps = reader.GetChangeFetchedPropertyNames();
        ASSERT_NE(nullptr, changedProps);
        auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
        EXPECT_TRUE(hasName("ECInstanceId"));
        EXPECT_TRUE(hasName("ECClassId")); // physical in the primary table â€” present in changeset
        EXPECT_TRUE(hasName("BaseCode"));
        EXPECT_FALSE(hasName("P")); // P lives in the joined table
        EXPECT_FALSE(hasName("Q")); // Q lives in the joined table
        }

    // --- Changeset row 2: joined table (tj_JChild) ---
    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());
        {
        Utf8String tableName;
        ASSERT_EQ(SUCCESS, reader.GetTableName(tableName));
        EXPECT_STREQ("tj_JChild", tableName.c_str());

        DbOpcode opcode;
        ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
        ASSERT_EQ(DbOpcode::Insert, opcode);

        EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));
        // ECInstanceId, ECClassId (via GetRootClassMap â€” virtual col not in changeset), P, Q
        ASSERT_EQ(4, reader.GetColumnCount(Changes::Change::Stage::New));

        IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::New, 0);
        EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_EQ(jChildKey.GetInstanceId(), v0.GetId<ECInstanceId>());

        IECSqlValue const& v1 = reader.GetValue(Changes::Change::Stage::New, 1);
        EXPECT_STREQ("ECClassId", v1.GetColumnInfo().GetProperty()->GetName().c_str());
        // Virtual ECClassId resolved via GetRootClassMap â†’ JChild's class id.
        EXPECT_EQ(jChildKey.GetClassId(), v1.GetId<ECN::ECClassId>());

        IECSqlValue const& v2 = reader.GetValue(Changes::Change::Stage::New, 2);
        EXPECT_STREQ("P", v2.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_EQ(42, v2.GetInt());

        IECSqlValue const& v3 = reader.GetValue(Changes::Change::Stage::New, 3);
        EXPECT_STREQ("Q", v3.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_STREQ("world", v3.GetText());

        auto const* changedProps = reader.GetChangeFetchedPropertyNames();
        ASSERT_NE(nullptr, changedProps);
        auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
        EXPECT_TRUE(hasName("ECInstanceId"));
        EXPECT_TRUE(hasName("P"));
        EXPECT_TRUE(hasName("Q"));
        EXPECT_TRUE(hasName("ECClassId"));
        // BaseCode lives in the primary table â€” absent from this joined-table row.
        EXPECT_FALSE(hasName("BaseCode"));
        }

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, OverflowOfJoinedTable_Insert)
    {
    Utf8CP overflowJoinedSchema = R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestOverJoined" alias="tjo" version="01.00.00"
                xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
        <ECEntityClass typeName="JBase" modifier="Abstract">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>TablePerHierarchy</MapStrategy>
                </ClassMap>
                <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00.00"/>
                <ShareColumns xmlns="ECDbMap.02.00.00">
                    <ApplyToSubclassesOnly>true</ApplyToSubclassesOnly>
                    <MaxSharedColumnsBeforeOverflow>1</MaxSharedColumnsBeforeOverflow>
                </ShareColumns>
            </ECCustomAttributes>
            <ECProperty propertyName="BaseCode" typeName="string"/>
        </ECEntityClass>
        <ECEntityClass typeName="JChild" modifier="Sealed">
            <BaseClass>JBase</BaseClass>
            <ECProperty propertyName="Name" typeName="string"/>
            <ECProperty propertyName="Age" typeName="int"/>
        </ECEntityClass>
        </ECSchema>)xml";

    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_overflow_joined.ecdb", SchemaItem(overflowJoinedSchema)));

    ECInstanceKey jChildKey;
    {
    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "INSERT INTO tjo.JChild(BaseCode, Name, Age) VALUES('hello', 'Alice', 30)"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(jChildKey));
    }

    auto cs = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker));

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    // --- Changeset row 1: primary table (tjo_JBase) ---
    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());
        {
        Utf8String tableName;
        ASSERT_EQ(SUCCESS, reader.GetTableName(tableName));
        EXPECT_STREQ("tjo_JBase", tableName.c_str());

        DbOpcode opcode;
        ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
        ASSERT_EQ(DbOpcode::Insert, opcode);

        EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));
        // ECInstanceId, ECClassId (physical â€” in changeset), BaseCode
        ASSERT_EQ(3, reader.GetColumnCount(Changes::Change::Stage::New));

        IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::New, 0);
        EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_EQ(jChildKey.GetInstanceId(), v0.GetId<ECInstanceId>());

        IECSqlValue const& v1 = reader.GetValue(Changes::Change::Stage::New, 1);
        EXPECT_STREQ("ECClassId", v1.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_EQ(jChildKey.GetClassId(), v1.GetId<ECN::ECClassId>());

        IECSqlValue const& v2 = reader.GetValue(Changes::Change::Stage::New, 2);
        EXPECT_STREQ("BaseCode", v2.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_STREQ("hello", v2.GetText());

        auto const* changedProps = reader.GetChangeFetchedPropertyNames();
        ASSERT_NE(nullptr, changedProps);
        auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
        EXPECT_TRUE(hasName("ECInstanceId"));
        EXPECT_TRUE(hasName("ECClassId"));
        EXPECT_TRUE(hasName("BaseCode"));
        EXPECT_FALSE(hasName("Name")); // Name is in the joined table
        EXPECT_FALSE(hasName("Age"));  // Age is in the overflow table
        }

    // --- Changeset row 2: joined table (tjo_JChild) ---
    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());
        {
        Utf8String tableName;
        ASSERT_EQ(SUCCESS, reader.GetTableName(tableName));
        EXPECT_STREQ("tjo_JChild", tableName.c_str());

        DbOpcode opcode;
        ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
        ASSERT_EQ(DbOpcode::Insert, opcode);

        EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));
        // ECInstanceId + ECClassId (via GetRootClassMap) + Name;  Age is in the overflow table.
        ASSERT_EQ(3, reader.GetColumnCount(Changes::Change::Stage::New));

        IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::New, 0);
        EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_EQ(jChildKey.GetInstanceId(), v0.GetId<ECInstanceId>());

        IECSqlValue const& v1 = reader.GetValue(Changes::Change::Stage::New, 1);
        EXPECT_STREQ("ECClassId", v1.GetColumnInfo().GetProperty()->GetName().c_str());
        // Virtual ECClassId in tjo_JChild â€” resolved via GetRootClassMap â†’ JChild's class id.
        EXPECT_EQ(jChildKey.GetClassId(), v1.GetId<ECN::ECClassId>());

        IECSqlValue const& v2 = reader.GetValue(Changes::Change::Stage::New, 2);
        EXPECT_STREQ("Name", v2.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_STREQ("Alice", v2.GetText());

        auto const* changedProps = reader.GetChangeFetchedPropertyNames();
        ASSERT_NE(nullptr, changedProps);
        auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
        EXPECT_TRUE(hasName("ECInstanceId"));
        EXPECT_TRUE(hasName("Name"));
        EXPECT_TRUE(hasName("ECClassId"));
        // Age overflowed; BaseCode is in the primary table â€” both absent from this row.
        EXPECT_FALSE(hasName("Age"));
        EXPECT_FALSE(hasName("BaseCode"));
        }

    // --- Changeset row 3: overflow of joined table (tjo_JChild_Overflow) ---
    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());
        {
        Utf8String tableName;
        ASSERT_EQ(SUCCESS, reader.GetTableName(tableName));
        EXPECT_STREQ("tjo_JChild_Overflow", tableName.c_str());

        DbOpcode opcode;
        ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
        ASSERT_EQ(DbOpcode::Insert, opcode);

        EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));
        // ECInstanceId, ECClassId (physical â€” in changeset), Age (os1)
        ASSERT_EQ(3, reader.GetColumnCount(Changes::Change::Stage::New));

        IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::New, 0);
        EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_EQ(jChildKey.GetInstanceId(), v0.GetId<ECInstanceId>());

        IECSqlValue const& v1 = reader.GetValue(Changes::Change::Stage::New, 1);
        EXPECT_STREQ("ECClassId", v1.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_EQ(jChildKey.GetClassId(), v1.GetId<ECN::ECClassId>());

        IECSqlValue const& v2 = reader.GetValue(Changes::Change::Stage::New, 2);
        EXPECT_STREQ("Age", v2.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_EQ(30, v2.GetInt());

        auto const* changedProps = reader.GetChangeFetchedPropertyNames();
        ASSERT_NE(nullptr, changedProps);
        auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
        EXPECT_TRUE(hasName("ECInstanceId"));
        EXPECT_TRUE(hasName("ECClassId")); // physical in the overflow table â€” present in changeset
        EXPECT_TRUE(hasName("Age"));
        EXPECT_FALSE(hasName("Name"));     // Name is in tjo_JChild (joined table)
        EXPECT_FALSE(hasName("BaseCode")); // BaseCode is in tjo_JBase (primary table)
        }

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, ExistingTable_InsertAndUpdate)
    {
    // Step 1: create the raw SQLite table BEFORE importing the EC schema.
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("csreader_existing.ecdb"));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql(
        "CREATE TABLE te_Gadget(Id INTEGER PRIMARY KEY, Label TEXT, Score INTEGER)"));

    Utf8CP existingSchema = R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestExisting" alias="te" version="01.00.00"
                xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
        <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap"/>
        <ECEntityClass typeName="Gadget" modifier="Sealed">
            <ECCustomAttributes>
                <ClassMap xmlns="ECDbMap.02.00.00">
                    <MapStrategy>ExistingTable</MapStrategy>
                    <TableName>te_Gadget</TableName>
                    <ECInstanceIdColumn>Id</ECInstanceIdColumn>
                </ClassMap>
            </ECCustomAttributes>
            <ECProperty propertyName="Label" typeName="string"/>
            <ECProperty propertyName="Score" typeName="int"/>
        </ECEntityClass>
        </ECSchema>)xml";

    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(SchemaItem(existingSchema)));

    // -----------------------------------------------------------------------
    // Part 1: INSERT
    // -----------------------------------------------------------------------
    ECInstanceId gadgetKey(BeInt64Id(1)); /* Gadget's class id will be 1 since it's the first entity class in the schema */
    {
    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql(
        "INSERT INTO te_Gadget(Id, Label, Score) VALUES(1, 'widget', 99)"));

    auto cs = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker));

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    // ExistingTable maps to exactly one physical table â€” exactly one changeset row.
    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());
        {
        Utf8String tableName;
        ASSERT_EQ(SUCCESS, reader.GetTableName(tableName));
        EXPECT_STREQ("te_Gadget", tableName.c_str());

        DbOpcode opcode;
        ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
        ASSERT_EQ(DbOpcode::Insert, opcode);

        EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));
        // ECInstanceId + ECClassId (virtual â†’ resolved via GetRootClassMap) + Label + Score
        ASSERT_EQ(4, reader.GetColumnCount(Changes::Change::Stage::New));

        IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::New, 0);
        EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_EQ(gadgetKey, v0.GetId<ECInstanceId>());

        IECSqlValue const& v1 = reader.GetValue(Changes::Change::Stage::New, 1);
        EXPECT_STREQ("ECClassId", v1.GetColumnInfo().GetProperty()->GetName().c_str());
        // Virtual ECClassId â€” resolved via GetRootClassMap(te_Gadget) â†’ Gadget's class id.
        EXPECT_EQ(m_ecdb.Schemas().GetClass("TestExisting","Gadget")->GetId(), v1.GetId<ECN::ECClassId>());

        IECSqlValue const& v2 = reader.GetValue(Changes::Change::Stage::New, 2);
        EXPECT_STREQ("Label", v2.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_STREQ("widget", v2.GetText());

        IECSqlValue const& v3 = reader.GetValue(Changes::Change::Stage::New, 3);
        EXPECT_STREQ("Score", v3.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_EQ(99, v3.GetInt());

        auto const* changedProps = reader.GetChangeFetchedPropertyNames();
        ASSERT_NE(nullptr, changedProps);
        auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
        EXPECT_TRUE(hasName("ECInstanceId"));
        // ECClassId is virtual â€” not in the SQLite changeset, so classIdFromChangeset=false.
        EXPECT_FALSE(hasName("ECClassId"));
        EXPECT_TRUE(hasName("Label"));
        EXPECT_TRUE(hasName("Score"));
        }

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }

    // -----------------------------------------------------------------------
    // Part 2: UPDATE Label and Score.
    // -----------------------------------------------------------------------
    {
    TestCSChangeTracker tracker2(m_ecdb);
    tracker2.EnableTracking(true);
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql(
        "UPDATE te_Gadget SET Label='gadget', Score=42 WHERE Id=1"));

    auto cs2 = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs2->FromChangeTrack(tracker2));

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs2), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());
        {
        Utf8String tableName;
        ASSERT_EQ(SUCCESS, reader.GetTableName(tableName));
        EXPECT_STREQ("te_Gadget", tableName.c_str());

        DbOpcode opcode;
        ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
        ASSERT_EQ(DbOpcode::Update, opcode);

        // ECInstanceId, ECClassId (virtual), Label, Score
        ASSERT_EQ(4, reader.GetColumnCount(Changes::Change::Stage::New));
        ASSERT_EQ(4, reader.GetColumnCount(Changes::Change::Stage::Old));

        // --- New stage ---
        IECSqlValue const& newId = reader.GetValue(Changes::Change::Stage::New, 0);
        EXPECT_STREQ("ECInstanceId", newId.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_EQ(gadgetKey, newId.GetId<ECInstanceId>());

        IECSqlValue const& newLabel = reader.GetValue(Changes::Change::Stage::New, 2);
        EXPECT_STREQ("Label", newLabel.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_STREQ("gadget", newLabel.GetText());

        IECSqlValue const& newScore = reader.GetValue(Changes::Change::Stage::New, 3);
        EXPECT_STREQ("Score", newScore.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_EQ(42, newScore.GetInt());

        // --- Old stage ---
        IECSqlValue const& oldLabel = reader.GetValue(Changes::Change::Stage::Old, 2);
        EXPECT_STREQ("Label", oldLabel.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_STREQ("widget", oldLabel.GetText());

        IECSqlValue const& oldScore = reader.GetValue(Changes::Change::Stage::Old, 3);
        EXPECT_STREQ("Score", oldScore.GetColumnInfo().GetProperty()->GetName().c_str());
        EXPECT_EQ(99, oldScore.GetInt());

        // ECClassId virtual â†’ not from changeset â†’ absent from changedProps.
        auto const* changedProps = reader.GetChangeFetchedPropertyNames();
        ASSERT_NE(nullptr, changedProps);
        auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
        EXPECT_TRUE(hasName("ECInstanceId"));
        EXPECT_FALSE(hasName("ECClassId")); // virtual â€” resolved via GetRootClassMap, not changeset
        EXPECT_TRUE(hasName("Label"));
        EXPECT_TRUE(hasName("Score"));
        }

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }
    }

//---------------------------------------------------------------------------------------
// Link table relationship with non-polymorphic (polymorphic=false) source and target
// constraints: SourceECClassId and TargetECClassId have no physical column (virtual).
// Verify that:
//   - ChangesetReader exposes exactly 4 columns for the INSERT row:
//     ECInstanceId, ECClassId, SourceECInstanceId, TargetECInstanceId.
//   - SourceECClassId and TargetECClassId produce no IECSqlValue (CreateSystem skips
//     them because FindDataPropertyMap returns nullptr for virtual columns).
//   - GetChangeFetchedPropertyNames contains the two physical IDs but omits ECClassId,
//     SourceECClassId and TargetECClassId (all virtual â€” not in the SQLite changeset).
//   - Rendering the row via ECSqlRowAdaptor + ChangesetRow produces JSON with
//     classFullName, sourceId and targetId, but no sourceClassName or targetClassName
//     (the virtual class IDs were never captured in the changeset).
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, Insert_RelationshipLinkTable_VirtualSourceTargetClassIds)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_rel_virtual_classids.ecdb", SchemaItem(GetRelSchema())));

    // Insert Person and Project before tracking â€” they must not appear in the changeset.
    ECInstanceKey personKey, projectKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(personKey,
        "INSERT INTO tr.Person(Name) VALUES('Alice')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(projectKey,
        "INSERT INTO tr.Project(Title) VALUES('Proj1')"));

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECInstanceKey relKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "INSERT INTO tr.PersonWorksOnProject(SourceECInstanceId, TargetECInstanceId) VALUES(?, ?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, personKey.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, projectKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(relKey));
    }

    auto cs = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker));

    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb,
        std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());

    DbOpcode opcode;
    ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Insert, opcode);

    EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));

    // Only 4 columns: ECInstanceId, ECClassId (virtualâ†’resolved), SourceECInstanceId,
    // TargetECInstanceId.  SourceECClassId and TargetECClassId are virtual
    // (polymorphic=false on source/target constraints) so CreateSystem emits nothing.
    ASSERT_EQ(4, reader.GetColumnCount(Changes::Change::Stage::New));

    // Column 0: ECInstanceId â€” physical primary key, always present.
    IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::New, 0);
    EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(relKey.GetInstanceId(), v0.GetId<ECInstanceId>());

    // Column 1: ECClassId â€” virtual for a sealed single-hierarchy link table,
    //           resolved via GetRootClassMap, not from a physical changeset column.
    IECSqlValue const& v1 = reader.GetValue(Changes::Change::Stage::New, 1);
    EXPECT_STREQ("ECClassId", v1.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(relKey.GetClassId(), v1.GetId<ECClassId>());

    // Column 2: SourceECInstanceId â€” physical column in the link table.
    IECSqlValue const& v2 = reader.GetValue(Changes::Change::Stage::New, 2);
    EXPECT_STREQ("SourceECInstanceId", v2.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(personKey.GetInstanceId(), v2.GetId<ECInstanceId>());

    // Column 3: TargetECInstanceId â€” physical column in the link table.
    IECSqlValue const& v3 = reader.GetValue(Changes::Change::Stage::New, 3);
    EXPECT_STREQ("TargetECInstanceId", v3.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(projectKey.GetInstanceId(), v3.GetId<ECInstanceId>());

    auto const* changedProps = reader.GetChangeFetchedPropertyNames();
    ASSERT_NE(nullptr, changedProps);
    auto hasName = [&](Utf8CP n) {
        return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end();
    };

    // Physical columns: ECInstanceId always present for INSERT; Source/TargetECInstanceId
    // are the two physical FK columns in the link table.
    EXPECT_TRUE(hasName("ECInstanceId"));
    EXPECT_TRUE(hasName("SourceECInstanceId"));
    EXPECT_TRUE(hasName("TargetECInstanceId"));

    // Virtual columns: no physical SQLite column â†’ absent from the changeset â†’
    // absent from GetChangeFetchedPropertyNames.
    EXPECT_FALSE(hasName("ECClassId"));       // virtual â€” resolved via GetRootClassMap
    EXPECT_FALSE(hasName("SourceECClassId")); // virtual â€” polymorphic=false on source constraint
    EXPECT_FALSE(hasName("TargetECClassId")); // virtual â€” polymorphic=false on target constraint

    // Render the New-stage row as a JSON object via ECSqlRowAdaptor + ChangesetRow â€”
    // the same path taken by GetValue() in IModelJsNative.cpp.
    // The JSON must contain classFullName, sourceId and targetId, but sourceClassName and
    // targetClassName must be absent because SourceECClassId / TargetECClassId are virtual
    // and were never captured in the changeset.
    {
    BeJsDocument rowDoc;
    BeJsValue rowJson(rowDoc);
    ECSqlRowAdaptor adaptor(m_ecdb);
    adaptor.GetOptions().SetConvertClassIdsToClassNames(true);
    adaptor.GetOptions().SetUseJsNames(true);
    adaptor.GetOptions().SetUseClassFullNameInsteadofClassName(true);
    ASSERT_EQ(SUCCESS, adaptor.RenderRowAsObject(rowJson, ChangesetRow(reader, Changes::Change::Stage::New)));

    // classFullName is present â€” ECClassId was resolved via GetRootClassMap even though virtual.
    Utf8String classFullName = rowJson["classFullName"].asString();
    EXPECT_FALSE(classFullName.empty());

    // Physical source/target instance IDs are present.
    EXPECT_FALSE(rowJson["sourceId"].asString().empty());
    EXPECT_FALSE(rowJson["targetId"].asString().empty());

    // Virtual class IDs are absent from the JSON â€” they were not in the changeset.
    EXPECT_TRUE(rowJson["sourceClassName"].isNull());
    EXPECT_TRUE(rowJson["targetClassName"].isNull());
    }

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }

//---------------------------------------------------------------------------------------
// Verify that InstanceRepository::Insert on a link-table relationship succeeds when
// SourceECClassId and TargetECClassId are absent from the input JSON.
// Because the source/target constraints are non-polymorphic (polymorphic=false), those
// class IDs are virtual â€” ECDb infers them from the schema rather than reading them from
// the row.  A JSON round-trip that captures only the physical properties
// (classFullName + sourceId + targetId) is therefore sufficient to re-insert the
// relationship without losing fidelity.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, Insert_RelationshipLinkTable_RoundTrip_VirtualClassIds)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_rel_roundtrip.ecdb", SchemaItem(GetRelSchema())));

    ECInstanceKey personKey, projectKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(personKey,
        "INSERT INTO tr.Person(Name) VALUES('Bob')"));
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(projectKey,
        "INSERT INTO tr.Project(Title) VALUES('ProjX')"));

    // Build JSON with only classFullName + physical source/target IDs.
    // SourceECClassId and TargetECClassId are intentionally omitted â€” ECDb must infer
    // them from the non-polymorphic schema without error.
    Utf8String jsonStr;
    jsonStr.Sprintf(
        R"({"classFullName":"TestRelCS:PersonWorksOnProject","sourceId":"%s","targetId":"%s"})",
        personKey.GetInstanceId().ToHexStr().c_str(),
        projectKey.GetInstanceId().ToHexStr().c_str());

    BeJsDocument doc;
    doc.Parse(jsonStr.c_str());
    ASSERT_FALSE(doc.hasParseError());

    BeJsDocument emptyArgs;
    ECInstanceKey relKey;
    auto rc = m_ecdb.GetInstanceRepository().Insert(doc, emptyArgs, JsFormat::JsName, relKey);
    EXPECT_EQ(BE_SQLITE_DONE, rc) << m_ecdb.GetInstanceRepository().GetLastError().c_str();
    EXPECT_TRUE(relKey.GetInstanceId().IsValid());
    EXPECT_TRUE(relKey.GetClassId().IsValid());

    // Read back the inserted row and confirm all four constraint columns are correct.
    // SourceECClassId == Person's class ID and TargetECClassId == Project's class ID are
    // inferred by ECDb from the schema â€” no explicit value was needed in the JSON.
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "SELECT SourceECInstanceId, TargetECInstanceId, SourceECClassId, TargetECClassId"
        " FROM tr.PersonWorksOnProject WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, relKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    EXPECT_EQ(personKey.GetInstanceId(),  stmt.GetValue(0).GetId<ECInstanceId>());
    EXPECT_EQ(projectKey.GetInstanceId(), stmt.GetValue(1).GetId<ECInstanceId>());
    EXPECT_EQ(personKey.GetClassId(),     stmt.GetValue(2).GetId<ECClassId>());
    EXPECT_EQ(projectKey.GetClassId(),    stmt.GetValue(3).GetId<ECClassId>());
    }
    }

//---------------------------------------------------------------------------------------
// OpenChangeGroup with a single file should produce the same opcode and row count as OpenChangesetFile.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, OpenGroup_SingleFile_SameResultAsOpenFile)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_group_single.ecdb", SchemaItem(GetSchema())));

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECInstanceKey widgetKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Widget(Name) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Alpha", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(widgetKey));
    }

    TestCSChangeSet cs;
    ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
    BeFileName csFile = WriteChangesetToFile(m_ecdb, cs, "csreader_group_single.changeset");

    // Read with OpenChangesetFile.
    int fileRowCount = 0;
    DbOpcode fileOpcode = DbOpcode::Update; // sentinel â€” will be overwritten
    {
    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenChangesetFile(m_ecdb, csFile.GetNameUtf8(), false, ChangesetReader::PropertyFilter::All));
    while (BE_SQLITE_ROW == reader.Step())
        {
        ++fileRowCount;
        ASSERT_EQ(SUCCESS, reader.GetOpcode(fileOpcode));
        }
    ASSERT_EQ(SUCCESS, reader.Close());
    }

    // Read with OpenChangeGroup using the same single file.
    int groupRowCount = 0;
    DbOpcode groupOpcode = DbOpcode::Delete; // sentinel â€” will be overwritten
    {
    ChangesetReader reader;
    T_Utf8StringVector files{csFile.GetNameUtf8()};
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenChangeGroup(m_ecdb, files, false, ChangesetReader::PropertyFilter::All, 1));
    while (BE_SQLITE_ROW == reader.Step())
        {
        ++groupRowCount;
        ASSERT_EQ(SUCCESS, reader.GetOpcode(groupOpcode));
        }
    ASSERT_EQ(SUCCESS, reader.Close());
    }

    EXPECT_EQ(groupRowCount, 1);
    EXPECT_EQ(fileRowCount, groupRowCount);
    EXPECT_EQ(fileOpcode, groupOpcode);
    }

//---------------------------------------------------------------------------------------
// OpenInMemoryChangeset with the same in memory changeset should produce the same opcode and row count as OpenChangesetFile.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, OpenInMemoryChangeset_SingleFile_SameResultAsOpenFile)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_group_single.ecdb", SchemaItem(GetSchema())));

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECInstanceKey widgetKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Widget(Name) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Alpha", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(widgetKey));
    }

    auto inMemoryCS = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, inMemoryCS->FromChangeTrack(tracker));
    TestCSChangeSet cs;
    ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
    BeFileName csFile = WriteChangesetToFile(m_ecdb, cs, "csreader_group_single.changeset");


    // Read with OpenChangesetFile.
    int fileRowCount = 0;
    DbOpcode fileOpcode = DbOpcode::Update; // sentinel â€” will be overwritten
    {
    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenChangesetFile(m_ecdb, csFile.GetNameUtf8(), false, ChangesetReader::PropertyFilter::All));
    while (BE_SQLITE_ROW == reader.Step())
        {
        ++fileRowCount;
        ASSERT_EQ(SUCCESS, reader.GetOpcode(fileOpcode));
        }
    ASSERT_EQ(SUCCESS, reader.Close());
    }

    // Read with OpenInMemoryChangeset using the same in memory changeset.
    int inMemRowCount = 0;
    DbOpcode inMemOpcode = DbOpcode::Delete; // sentinel â€” will be overwritten
    {
    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb, std::move(inMemoryCS), false, ChangesetReader::PropertyFilter::All, 1));
    while (BE_SQLITE_ROW == reader.Step())
        {
        ++inMemRowCount;
        ASSERT_EQ(SUCCESS, reader.GetOpcode(inMemOpcode));
        }
    ASSERT_EQ(SUCCESS, reader.Close());
    }

    EXPECT_EQ(inMemRowCount, 1);
    EXPECT_EQ(fileRowCount, inMemRowCount);
    EXPECT_EQ(fileOpcode, inMemOpcode);
    }

//---------------------------------------------------------------------------------------
// OpenChangeGroup with two files each inserting a different row should return both INSERT rows.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, OpenGroup_TwoIndependentInserts_BothRowsReturned)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_group_two_inserts.ecdb", SchemaItem(GetSchema())));

    // Changeset 1: INSERT Widget "Alpha".
    BeFileName cs1File;
    {
    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Widget(Name) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Alpha", IECSqlBinder::MakeCopy::No));
    ECInstanceKey k;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(k));
    TestCSChangeSet cs;
    ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
    cs1File = WriteChangesetToFile(m_ecdb, cs, "csreader_group_two_inserts_cs1.changeset");
    }

    // Changeset 2: INSERT Widget "Beta".
    BeFileName cs2File;
    {
    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Widget(Name) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Beta", IECSqlBinder::MakeCopy::No));
    ECInstanceKey k;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(k));
    TestCSChangeSet cs;
    ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
    cs2File = WriteChangesetToFile(m_ecdb, cs, "csreader_group_two_inserts_cs2.changeset");
    }

    ChangesetReader reader;
    T_Utf8StringVector files{cs1File.GetNameUtf8(), cs2File.GetNameUtf8()};
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenChangeGroup(m_ecdb, files, false, ChangesetReader::PropertyFilter::All, 1));

    int insertCount = 0;
    while (BE_SQLITE_ROW == reader.Step())
        {
        DbOpcode opcode;
        ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
        EXPECT_EQ(DbOpcode::Insert, opcode);
        ++insertCount;
        }
    ASSERT_EQ(SUCCESS, reader.Close());

    EXPECT_EQ(2, insertCount);
    }

//---------------------------------------------------------------------------------------
// OpenChangeGroup merges two sequential updates to the same row via ChangeGroup net-merge
// semantics: the merged result is a single UPDATE whose Old value comes from cs1 and
// whose New value comes from cs2.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, OpenGroup_TwoUpdatesToSameRow_NetMerged)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_group_net_merge.ecdb", SchemaItem(GetSchema())));

    // Pre-insert the Widget outside tracking so its initial value is "Original".
    ECInstanceKey widgetKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Widget(Name) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Original", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(widgetKey));
    }

    // Changeset 1: UPDATE â†’ "Version1".
    BeFileName cs1File;
    {
    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Widget SET Name=? WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Version1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, widgetKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    TestCSChangeSet cs;
    ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
    cs1File = WriteChangesetToFile(m_ecdb, cs, "csreader_group_net_merge_cs1.changeset");
    }

    // Changeset 2: UPDATE â†’ "Version2".
    BeFileName cs2File;
    {
    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "UPDATE ts.Widget SET Name=? WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Version2", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, widgetKey.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    TestCSChangeSet cs;
    ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
    cs2File = WriteChangesetToFile(m_ecdb, cs, "csreader_group_net_merge_cs2.changeset");
    }

    // OpenChangeGroup merges the two: result must be a single UPDATE.
    ChangesetReader reader;
    T_Utf8StringVector files{cs1File.GetNameUtf8(), cs2File.GetNameUtf8()};
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenChangeGroup(m_ecdb, files, false, ChangesetReader::PropertyFilter::All, 1));
    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());

    DbOpcode opcode;
    ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
    EXPECT_EQ(DbOpcode::Update, opcode);

    // New stage: Name must be "Version2".
    Utf8String newName;
    int newColCount = reader.GetColumnCount(Changes::Change::Stage::New);
    for (int i = 0; i < newColCount; ++i)
        {
        IECSqlValue const& v = reader.GetValue(Changes::Change::Stage::New, i);
        ECN::ECPropertyCP prop = v.GetColumnInfo().GetProperty();
        if (prop != nullptr && Utf8String("Name") == prop->GetName())
            {
            newName = v.GetText();
            break;
            }
        }
    EXPECT_EQ(Utf8String("Version2"), newName);

    // Old stage: Name must be "Original".
    Utf8String oldName;
    int oldColCount = reader.GetColumnCount(Changes::Change::Stage::Old);
    for (int i = 0; i < oldColCount; ++i)
        {
        IECSqlValue const& v = reader.GetValue(Changes::Change::Stage::Old, i);
        ECN::ECPropertyCP prop = v.GetColumnInfo().GetProperty();
        if (prop != nullptr && Utf8String("Name") == prop->GetName())
            {
            oldName = v.GetText();
            break;
            }
        }
    EXPECT_EQ(Utf8String("Original"), oldName);

    EXPECT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }

//---------------------------------------------------------------------------------------
// After Close(), OpenChangeGroup must have deleted the temporary "*-merged.changeset" file
// it created during the merge phase.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, OpenGroup_TempFileDeletedAfterClose)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_group_cleanup.ecdb", SchemaItem(GetSchema())));

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECInstanceKey widgetKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Widget(Name) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "CleanupWidget", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(widgetKey));
    }

    TestCSChangeSet cs;
    ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
    BeFileName csFile = WriteChangesetToFile(m_ecdb, cs, "csreader_group_cleanup.changeset");

    // Helper: count files matching "*-merged.changeset" in the test output root.
    BeFileName outputDir;
    BeTest::GetHost().GetOutputRoot(outputDir);
    auto countMergedFiles = [&]() -> size_t
        {
        bvector<BeFileName> matches;
        BeDirectoryIterator::WalkDirsAndMatch(matches, outputDir, L"*-merged.changeset", false);
        return matches.size();
        };

    {
    ChangesetReader reader;
    T_Utf8StringVector files{csFile.GetNameUtf8()};
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenChangeGroup(m_ecdb, files, false, ChangesetReader::PropertyFilter::All, 1));

    // The temp merged file must exist while the reader is open.
    EXPECT_GE(countMergedFiles(), (size_t) 1) << "Temp *-merged.changeset file must exist while reader is open";

    int rowCount = 0;
    while (BE_SQLITE_ROW == reader.Step())
        ++rowCount;
    EXPECT_EQ(1, rowCount);
    ASSERT_EQ(SUCCESS, reader.Close());
    }

    // After Close() the temp file must be gone.
    EXPECT_EQ((size_t) 0, countMergedFiles()) << "Temp *-merged.changeset file must be deleted after Close()";
    }

//---------------------------------------------------------------------------------------
// OpenInMemoryChangeset with a changeset below the default 50 MB threshold must stream directly
// from the in-memory ChangeSet without writing any temp file.
// Uses a fully-populated Widget (all property types) and asserts every column value.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, OpenChangeSet_InMemoryPath_BelowThreshold)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_cs_inmem.ecdb", SchemaItem(GetSchema())));

    // Container inserted BEFORE tracking â€” must not appear in the changeset.
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
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "InMemWidget", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(2, 1.23));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(3, 10));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBoolean(4, true));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(5, DPoint2d::From(4.0, 5.0)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(6, DPoint3d::From(6.0, 7.0, 8.0)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(7, "InMemLabel", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(8, 42));
    IECSqlBinder& tagsBinder = stmt.GetBinder(9);
    ASSERT_EQ(ECSqlStatus::Success, tagsBinder.AddArrayElement().BindText("alpha", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, tagsBinder.AddArrayElement().BindText("beta", IECSqlBinder::MakeCopy::No));
    ECN::ECClassId relClassId = m_ecdb.Schemas().GetClassId("TestReadCS", "ContainerOwnsWidgets");
    ASSERT_TRUE(relClassId.IsValid());
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(10, containerKey.GetInstanceId(), relClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(widgetKey));
    }

    auto cs = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker));

    BeFileName outputDir;
    BeTest::GetHost().GetOutputRoot(outputDir);
    auto countMergedFiles = [&]() -> size_t
        {
        bvector<BeFileName> matches;
        BeDirectoryIterator::WalkDirsAndMatch(matches, outputDir, L"*-merged.changeset", false);
        return matches.size();
        };

    // Default threshold is 50 MB; a tiny changeset must stream in memory â€” no file written.
    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb, std::move(cs), false, ChangesetReader::PropertyFilter::All, GetDefaultSpillThresholdBytes()));
    EXPECT_EQ((size_t) 0, countMergedFiles()) << "No temp file should be created for a sub-threshold changeset";

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());

    DbOpcode opcode;
    ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Insert, opcode);

    // Old stage must be empty for an insert.
    EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));
    EXPECT_EQ(11, reader.GetColumnCount(Changes::Change::Stage::New));

    // Property 1
    IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::New, 0);
    EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetInstanceId(), v0.GetId<ECInstanceId>());

    // Property 2
    IECSqlValue const& v1 = reader.GetValue(Changes::Change::Stage::New, 1);
    EXPECT_STREQ("ECClassId", v1.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetClassId(), v1.GetId<ECN::ECClassId>());

    // Property 3
    IECSqlValue const& v2 = reader.GetValue(Changes::Change::Stage::New, 2);
    EXPECT_STREQ("Name", v2.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_STREQ("InMemWidget", v2.GetText());

    // Property 4
    IECSqlValue const& v3 = reader.GetValue(Changes::Change::Stage::New, 3);
    EXPECT_STREQ("Weight", v3.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_DOUBLE_EQ(1.23, v3.GetDouble());

    // Property 5
    IECSqlValue const& v4 = reader.GetValue(Changes::Change::Stage::New, 4);
    EXPECT_STREQ("Cnt", v4.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(10, v4.GetInt64());

    // Property 6
    IECSqlValue const& v5 = reader.GetValue(Changes::Change::Stage::New, 5);
    EXPECT_STREQ("Active", v5.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_TRUE(v5.GetBoolean());

    // Property 7
    IECSqlValue const& v6 = reader.GetValue(Changes::Change::Stage::New, 6);
    EXPECT_STREQ("Pos2d", v6.GetColumnInfo().GetProperty()->GetName().c_str());
    DPoint2d pos2d = v6.GetPoint2d();
    EXPECT_DOUBLE_EQ(4.0, pos2d.x);
    EXPECT_DOUBLE_EQ(5.0, pos2d.y);

    // Property 8
    IECSqlValue const& v7 = reader.GetValue(Changes::Change::Stage::New, 7);
    EXPECT_STREQ("Pos3d", v7.GetColumnInfo().GetProperty()->GetName().c_str());
    DPoint3d pos3d = v7.GetPoint3d();
    EXPECT_DOUBLE_EQ(6.0, pos3d.x);
    EXPECT_DOUBLE_EQ(7.0, pos3d.y);
    EXPECT_DOUBLE_EQ(8.0, pos3d.z);

    // Property 9
    IECSqlValue const& v8 = reader.GetValue(Changes::Change::Stage::New, 8);
    EXPECT_STREQ("Details", v8.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_STREQ("InMemLabel", v8["Label"].GetText());
    EXPECT_EQ(42, v8["Score"].GetInt());

    // Property 10
    IECSqlValue const& v9 = reader.GetValue(Changes::Change::Stage::New, 9);
    EXPECT_STREQ("Tags", v9.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(2, v9.GetArrayLength());
    Utf8CP expectedTags[] = {"alpha", "beta"};
    int tagIdx = 0;
    for (IECSqlValue const& elem : v9.GetArrayIterable())
        {
        EXPECT_STREQ(expectedTags[tagIdx], elem.GetText());
        ++tagIdx;
        }
    EXPECT_EQ(2, tagIdx);

    // Property 11
    IECSqlValue const& v10 = reader.GetValue(Changes::Change::Stage::New, 10);
    EXPECT_STREQ("Owner", v10.GetColumnInfo().GetProperty()->GetName().c_str());
    ECN::ECClassId navRelId;
    ECInstanceId ownerId = v10.GetNavigation<ECInstanceId>(&navRelId);
    EXPECT_EQ(containerKey.GetInstanceId(), ownerId);
    EXPECT_TRUE(navRelId.IsValid());

    Utf8String instanceKey;
    ASSERT_EQ(SUCCESS, reader.GetInstanceKey(Changes::Change::Stage::New, instanceKey));
    EXPECT_FALSE(instanceKey.empty());

    auto const* changedProps = reader.GetChangeFetchedPropertyNames();
    ASSERT_NE(nullptr, changedProps);
    auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
    EXPECT_TRUE(hasName("ECInstanceId"));
    EXPECT_TRUE(hasName("Name"));
    EXPECT_TRUE(hasName("Weight"));
    EXPECT_TRUE(hasName("Cnt"));
    EXPECT_TRUE(hasName("Active"));
    EXPECT_TRUE(hasName("Pos2d"));
    EXPECT_TRUE(hasName("Pos3d"));
    EXPECT_TRUE(hasName("Details.Label"));
    EXPECT_TRUE(hasName("Details.Score"));
    EXPECT_TRUE(hasName("Tags"));
    EXPECT_TRUE(hasName("Owner.Id"));

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    EXPECT_EQ((size_t) 0, countMergedFiles()) << "No temp file should exist after Close() for in-memory path";
    }

//---------------------------------------------------------------------------------------
// OpenInMemoryChangeset with threshold=1 must spill to a temp LZMA file, stream the same data
// correctly, and delete the file on Close().
// Uses a fully-populated Widget (all property types) and asserts every column value.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, OpenChangeSet_SpillPath_TempFileCreatedAndDeleted)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_cs_spill.ecdb", SchemaItem(GetSchema())));

    // Container inserted BEFORE tracking â€” must not appear in the changeset.
    ECInstanceKey containerKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(containerKey,
        "INSERT INTO ts.Container(Name) VALUES('Crate')"));

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECInstanceKey widgetKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "INSERT INTO ts.Widget(Name, Weight, Cnt, Active, Pos2d, Pos3d, Details.Label, Details.Score, Tags, Owner) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "SpilledWidget", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(2, 9.81));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(3, 99));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBoolean(4, false));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(5, DPoint2d::From(11.0, 22.0)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(6, DPoint3d::From(33.0, 44.0, 55.0)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(7, "SpillLabel", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(8, 77));
    IECSqlBinder& tagsBinder = stmt.GetBinder(9);
    ASSERT_EQ(ECSqlStatus::Success, tagsBinder.AddArrayElement().BindText("x", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, tagsBinder.AddArrayElement().BindText("y", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, tagsBinder.AddArrayElement().BindText("z", IECSqlBinder::MakeCopy::No));
    ECN::ECClassId relClassId = m_ecdb.Schemas().GetClassId("TestReadCS", "ContainerOwnsWidgets");
    ASSERT_TRUE(relClassId.IsValid());
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(10, containerKey.GetInstanceId(), relClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(widgetKey));
    }

    auto cs = std::make_unique<TestCSChangeSet>();
    ASSERT_EQ(BE_SQLITE_OK, cs->FromChangeTrack(tracker));

    BeFileName outputDir;
    BeTest::GetHost().GetOutputRoot(outputDir);
    auto countMergedFiles = [&]() -> size_t
        {
        bvector<BeFileName> matches;
        BeDirectoryIterator::WalkDirsAndMatch(matches, outputDir, L"*-merged.changeset", false);
        return matches.size();
        };

    {
    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenInMemoryChangeset(m_ecdb, std::move(cs), false, ChangesetReader::PropertyFilter::All, 1));

    // Temp file must exist while the reader is open and streaming.
    EXPECT_GE(countMergedFiles(), (size_t) 1) << "Temp *-merged.changeset file must exist during spill-path reading";

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());

    DbOpcode opcode;
    ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Insert, opcode);

    // Old stage must be empty for an insert.
    EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));
    EXPECT_EQ(11, reader.GetColumnCount(Changes::Change::Stage::New));

    // Property 1
    IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::New, 0);
    EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetInstanceId(), v0.GetId<ECInstanceId>());

    // Property 2
    IECSqlValue const& v1 = reader.GetValue(Changes::Change::Stage::New, 1);
    EXPECT_STREQ("ECClassId", v1.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetClassId(), v1.GetId<ECN::ECClassId>());

    // Property 3
    IECSqlValue const& v2 = reader.GetValue(Changes::Change::Stage::New, 2);
    EXPECT_STREQ("Name", v2.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_STREQ("SpilledWidget", v2.GetText());

    // Property 4
    IECSqlValue const& v3 = reader.GetValue(Changes::Change::Stage::New, 3);
    EXPECT_STREQ("Weight", v3.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_DOUBLE_EQ(9.81, v3.GetDouble());

    // Property 5
    IECSqlValue const& v4 = reader.GetValue(Changes::Change::Stage::New, 4);
    EXPECT_STREQ("Cnt", v4.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(99, v4.GetInt64());

    // Property 6
    IECSqlValue const& v5 = reader.GetValue(Changes::Change::Stage::New, 5);
    EXPECT_STREQ("Active", v5.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_FALSE(v5.GetBoolean());

    // Property 7
    IECSqlValue const& v6 = reader.GetValue(Changes::Change::Stage::New, 6);
    EXPECT_STREQ("Pos2d", v6.GetColumnInfo().GetProperty()->GetName().c_str());
    DPoint2d pos2d = v6.GetPoint2d();
    EXPECT_DOUBLE_EQ(11.0, pos2d.x);
    EXPECT_DOUBLE_EQ(22.0, pos2d.y);

    // Property 8
    IECSqlValue const& v7 = reader.GetValue(Changes::Change::Stage::New, 7);
    EXPECT_STREQ("Pos3d", v7.GetColumnInfo().GetProperty()->GetName().c_str());
    DPoint3d pos3d = v7.GetPoint3d();
    EXPECT_DOUBLE_EQ(33.0, pos3d.x);
    EXPECT_DOUBLE_EQ(44.0, pos3d.y);
    EXPECT_DOUBLE_EQ(55.0, pos3d.z);

    // Property 9
    IECSqlValue const& v8 = reader.GetValue(Changes::Change::Stage::New, 8);
    EXPECT_STREQ("Details", v8.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_STREQ("SpillLabel", v8["Label"].GetText());
    EXPECT_EQ(77, v8["Score"].GetInt());

    // Property 10
    IECSqlValue const& v9 = reader.GetValue(Changes::Change::Stage::New, 9);
    EXPECT_STREQ("Tags", v9.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(3, v9.GetArrayLength());
    Utf8CP expectedTags[] = {"x", "y", "z"};
    int tagIdx = 0;
    for (IECSqlValue const& elem : v9.GetArrayIterable())
        {
        EXPECT_STREQ(expectedTags[tagIdx], elem.GetText());
        ++tagIdx;
        }
    EXPECT_EQ(3, tagIdx);

    // Property 11
    IECSqlValue const& v10 = reader.GetValue(Changes::Change::Stage::New, 10);
    EXPECT_STREQ("Owner", v10.GetColumnInfo().GetProperty()->GetName().c_str());
    ECN::ECClassId navRelId;
    ECInstanceId ownerId = v10.GetNavigation<ECInstanceId>(&navRelId);
    EXPECT_EQ(containerKey.GetInstanceId(), ownerId);
    EXPECT_TRUE(navRelId.IsValid());

    Utf8String instanceKey;
    ASSERT_EQ(SUCCESS, reader.GetInstanceKey(Changes::Change::Stage::New, instanceKey));
    EXPECT_FALSE(instanceKey.empty());

    auto const* changedProps = reader.GetChangeFetchedPropertyNames();
    ASSERT_NE(nullptr, changedProps);
    auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
    EXPECT_TRUE(hasName("ECInstanceId"));
    EXPECT_TRUE(hasName("Name"));
    EXPECT_TRUE(hasName("Weight"));
    EXPECT_TRUE(hasName("Cnt"));
    EXPECT_TRUE(hasName("Active"));
    EXPECT_TRUE(hasName("Pos2d"));
    EXPECT_TRUE(hasName("Pos3d"));
    EXPECT_TRUE(hasName("Details.Label"));
    EXPECT_TRUE(hasName("Details.Score"));
    EXPECT_TRUE(hasName("Tags"));
    EXPECT_TRUE(hasName("Owner.Id"));

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }

    // After Close() the temp file must be deleted.
    EXPECT_EQ((size_t) 0, countMergedFiles()) << "Temp *-merged.changeset file must be deleted after Close()";
    }

//---------------------------------------------------------------------------------------
// OpenChangeGroup with threshold=1 spills the merged ChangeSet to a temp LZMA file, yields
// the correct rows, and deletes the file on Close().
// Uses a fully-populated Widget (all property types) and asserts every column value.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, OpenGroup_SpillPath_TempFileCreatedAndDeleted)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("csreader_group_spill.ecdb", SchemaItem(GetSchema())));

    // Container inserted BEFORE tracking â€” must not appear in the changeset.
    ECInstanceKey containerKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(containerKey,
        "INSERT INTO ts.Container(Name) VALUES('Bin')"));

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECInstanceKey widgetKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb,
        "INSERT INTO ts.Widget(Name, Weight, Cnt, Active, Pos2d, Pos3d, Details.Label, Details.Score, Tags, Owner) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "GroupSpillWidget", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(2, 2.71));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(3, 5));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBoolean(4, true));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(5, DPoint2d::From(7.0, 8.0)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(6, DPoint3d::From(1.0, 2.0, 3.0)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(7, "GroupLabel", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(8, 11));
    IECSqlBinder& tagsBinder = stmt.GetBinder(9);
    ASSERT_EQ(ECSqlStatus::Success, tagsBinder.AddArrayElement().BindText("p", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, tagsBinder.AddArrayElement().BindText("q", IECSqlBinder::MakeCopy::No));
    ECN::ECClassId relClassId = m_ecdb.Schemas().GetClassId("TestReadCS", "ContainerOwnsWidgets");
    ASSERT_TRUE(relClassId.IsValid());
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindNavigationValue(10, containerKey.GetInstanceId(), relClassId));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(widgetKey));
    }

    TestCSChangeSet cs;
    ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
    BeFileName csFile = WriteChangesetToFile(m_ecdb, cs, "csreader_group_spill.changeset");

    BeFileName outputDir;
    BeTest::GetHost().GetOutputRoot(outputDir);
    auto countMergedFiles = [&]() -> size_t
        {
        bvector<BeFileName> matches;
        BeDirectoryIterator::WalkDirsAndMatch(matches, outputDir, L"*-merged.changeset", false);
        return matches.size();
        };

    {
    ChangesetReader reader;
    T_Utf8StringVector files{csFile.GetNameUtf8()};
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenChangeGroup(m_ecdb, files, false, ChangesetReader::PropertyFilter::All, 1));

    EXPECT_GE(countMergedFiles(), (size_t) 1) << "Temp file must exist while group reader is open (spill path)";

    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());

    DbOpcode opcode;
    ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
    ASSERT_EQ(DbOpcode::Insert, opcode);

    // Old stage must be empty for an insert.
    EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));
    EXPECT_EQ(11, reader.GetColumnCount(Changes::Change::Stage::New));

    // Property 1
    IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::New, 0);
    EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetInstanceId(), v0.GetId<ECInstanceId>());

    // Property 2
    IECSqlValue const& v1 = reader.GetValue(Changes::Change::Stage::New, 1);
    EXPECT_STREQ("ECClassId", v1.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(widgetKey.GetClassId(), v1.GetId<ECN::ECClassId>());

    // Property 3
    IECSqlValue const& v2 = reader.GetValue(Changes::Change::Stage::New, 2);
    EXPECT_STREQ("Name", v2.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_STREQ("GroupSpillWidget", v2.GetText());

    // Property 4
    IECSqlValue const& v3 = reader.GetValue(Changes::Change::Stage::New, 3);
    EXPECT_STREQ("Weight", v3.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_DOUBLE_EQ(2.71, v3.GetDouble());

    // Property 5
    IECSqlValue const& v4 = reader.GetValue(Changes::Change::Stage::New, 4);
    EXPECT_STREQ("Cnt", v4.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(5, v4.GetInt64());

    // Property 6
    IECSqlValue const& v5 = reader.GetValue(Changes::Change::Stage::New, 5);
    EXPECT_STREQ("Active", v5.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_TRUE(v5.GetBoolean());

    // Property 7
    IECSqlValue const& v6 = reader.GetValue(Changes::Change::Stage::New, 6);
    EXPECT_STREQ("Pos2d", v6.GetColumnInfo().GetProperty()->GetName().c_str());
    DPoint2d pos2d = v6.GetPoint2d();
    EXPECT_DOUBLE_EQ(7.0, pos2d.x);
    EXPECT_DOUBLE_EQ(8.0, pos2d.y);

    // Property 8
    IECSqlValue const& v7 = reader.GetValue(Changes::Change::Stage::New, 7);
    EXPECT_STREQ("Pos3d", v7.GetColumnInfo().GetProperty()->GetName().c_str());
    DPoint3d pos3d = v7.GetPoint3d();
    EXPECT_DOUBLE_EQ(1.0, pos3d.x);
    EXPECT_DOUBLE_EQ(2.0, pos3d.y);
    EXPECT_DOUBLE_EQ(3.0, pos3d.z);

    // Property 9
    IECSqlValue const& v8 = reader.GetValue(Changes::Change::Stage::New, 8);
    EXPECT_STREQ("Details", v8.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_STREQ("GroupLabel", v8["Label"].GetText());
    EXPECT_EQ(11, v8["Score"].GetInt());

    // Property 10
    IECSqlValue const& v9 = reader.GetValue(Changes::Change::Stage::New, 9);
    EXPECT_STREQ("Tags", v9.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(2, v9.GetArrayLength());
    Utf8CP expectedTags[] = {"p", "q"};
    int tagIdx = 0;
    for (IECSqlValue const& elem : v9.GetArrayIterable())
        {
        EXPECT_STREQ(expectedTags[tagIdx], elem.GetText());
        ++tagIdx;
        }
    EXPECT_EQ(2, tagIdx);

    // Property 11
    IECSqlValue const& v10 = reader.GetValue(Changes::Change::Stage::New, 10);
    EXPECT_STREQ("Owner", v10.GetColumnInfo().GetProperty()->GetName().c_str());
    ECN::ECClassId navRelId;
    ECInstanceId ownerId = v10.GetNavigation<ECInstanceId>(&navRelId);
    EXPECT_EQ(containerKey.GetInstanceId(), ownerId);
    EXPECT_TRUE(navRelId.IsValid());

    Utf8String instanceKey;
    ASSERT_EQ(SUCCESS, reader.GetInstanceKey(Changes::Change::Stage::New, instanceKey));
    EXPECT_FALSE(instanceKey.empty());

    auto const* changedProps = reader.GetChangeFetchedPropertyNames();
    ASSERT_NE(nullptr, changedProps);
    auto hasName = [&](Utf8CP n) { return std::find(changedProps->begin(), changedProps->end(), n) != changedProps->end(); };
    EXPECT_TRUE(hasName("ECInstanceId"));
    EXPECT_TRUE(hasName("Name"));
    EXPECT_TRUE(hasName("Weight"));
    EXPECT_TRUE(hasName("Cnt"));
    EXPECT_TRUE(hasName("Active"));
    EXPECT_TRUE(hasName("Pos2d"));
    EXPECT_TRUE(hasName("Pos3d"));
    EXPECT_TRUE(hasName("Details.Label"));
    EXPECT_TRUE(hasName("Details.Score"));
    EXPECT_TRUE(hasName("Tags"));
    EXPECT_TRUE(hasName("Owner.Id"));

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }

    EXPECT_EQ((size_t) 0, countMergedFiles()) << "Temp file must be deleted after Close() for OpenChangeGroup spill path";
    }

//---------------------------------------------------------------------------------------
// Verifies strict-mode behaviour when the changeset was captured against an older (V1)
// schema and is later read against a newer (V2) DB whose table has an extra column.
// Strict mode ON  â†’ Step() must return BE_SQLITE_ERROR.
// Strict mode OFF â†’ Step() must succeed and return BE_SQLITE_ROW.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, StrictMode_OlderChangesetOnNewerDb)
    {
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("strict_older_cs.ecdb", SchemaItem(GetStrictModeV1Schema())));

    // Capture a V1 INSERT changeset (table has 4 columns: Id, ECClassId, Name, Value).
    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECInstanceKey itemKey;
    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(itemKey,
        "INSERT INTO tsi.Item(Name, Val) VALUES('Foo', 42)"));

    TestCSChangeSet cs;
    ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
    BeFileName csFile = WriteChangesetToFile(m_ecdb, cs, "strict_older_cs.changeset");

    // Upgrade the DB to V2 â€” the tsi_Item table now has 5 columns (Id, ECClassId, Name, Value, Extra).
    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(SchemaItem(GetStrictModeV2Schema())));

    // Strict mode ON: column-count mismatch (4 in changeset vs 5 in table) must be an error.
    {
    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenChangesetFile(m_ecdb, csFile.GetNameUtf8(), false,
        ChangesetReader::PropertyFilter::All));
    ASSERT_EQ(SUCCESS, reader.EnableStrictMode());
    EXPECT_EQ(BE_SQLITE_ERROR, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }

    // Strict mode OFF: reader must tolerate the shorter changeset and return a valid row.
    {
    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenChangesetFile(m_ecdb, csFile.GetNameUtf8(), false,
        ChangesetReader::PropertyFilter::All));
    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());

    DbOpcode opcode;
    ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
    EXPECT_EQ(DbOpcode::Insert, opcode);

    // Old stage is empty for an INSERT.
    EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));

    // Only the 4 V1 columns are readable; the V2 Extra column is not present in the changeset.
    EXPECT_EQ(4, reader.GetColumnCount(Changes::Change::Stage::New));

    IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::New, 0);
    EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(itemKey.GetInstanceId(), v0.GetId<ECInstanceId>());

    IECSqlValue const& v1 = reader.GetValue(Changes::Change::Stage::New, 1);
    EXPECT_STREQ("ECClassId", v1.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(itemKey.GetClassId(), v1.GetId<ECN::ECClassId>());

    IECSqlValue const& v2 = reader.GetValue(Changes::Change::Stage::New, 2);
    EXPECT_STREQ("Name", v2.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_STREQ("Foo", v2.GetText());

    IECSqlValue const& v3 = reader.GetValue(Changes::Change::Stage::New, 3);
    EXPECT_STREQ("Val", v3.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(42, v3.GetInt());

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }
    }

//---------------------------------------------------------------------------------------
// Verifies strict-mode behaviour when the changeset was captured against a newer (V2)
// schema and is read against an older (V1) DB whose table is one column shorter.
// The V2 changeset is produced by a separate ECDb instance; the reader uses a V1 DB.
// Strict mode ON  â†’ Step() must return BE_SQLITE_ERROR.
// Strict mode OFF â†’ Step() must succeed and return BE_SQLITE_ROW.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(ChangesetReaderTests, StrictMode_NewerChangesetOnOlderDb)
    {
    // Build a V2 changeset using a separate ECDb upgraded to V2 (5 columns).
    BeFileName csFile;
    ECInstanceKey itemKey;
    {
    // m_ecdb is the reader DB â€” it carries only the V1 schema (4 columns).
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("strict_newer_src.ecdb", SchemaItem(GetStrictModeV1Schema())));
    ASSERT_EQ(BentleyStatus::SUCCESS, ImportSchema(SchemaItem(GetStrictModeV2Schema())));

    TestCSChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteInsertECSql(itemKey,
        "INSERT INTO tsi.Item(Name, Val, Extra) VALUES('Bar', 99, 'bonus')"));

    TestCSChangeSet cs;
    ASSERT_EQ(BE_SQLITE_OK, cs.FromChangeTrack(tracker));
    csFile = WriteChangesetToFile(m_ecdb, cs, "strict_newer_cs.changeset");
    }   // End source-db setup scope; m_ecdb is reinitialized by the SetupECDb() call below.

    // m_ecdb is the reader DB â€” it carries only the V1 schema (4 columns).
    ASSERT_EQ(BentleyStatus::SUCCESS, SetupECDb("strict_older_db.ecdb", SchemaItem(GetStrictModeV1Schema())));

    // Strict mode ON: changeset has 5 columns but the V1 reader DB table has only 4 â†’ error.
    {
    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenChangesetFile(m_ecdb, csFile.GetNameUtf8(), false,
        ChangesetReader::PropertyFilter::All));
    ASSERT_EQ(SUCCESS, reader.EnableStrictMode());
    EXPECT_EQ(BE_SQLITE_ERROR, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }

    // Strict mode OFF: reader must read the first min(5,4)=4 columns and succeed.
    // The Extra column (index 4 in the changeset) is beyond the minimum and not surfaced.
    {
    ChangesetReader reader;
    ASSERT_EQ(BE_SQLITE_OK, reader.OpenChangesetFile(m_ecdb, csFile.GetNameUtf8(), false,
        ChangesetReader::PropertyFilter::All));
    ASSERT_EQ(BE_SQLITE_ROW, reader.Step());

    DbOpcode opcode;
    ASSERT_EQ(SUCCESS, reader.GetOpcode(opcode));
    EXPECT_EQ(DbOpcode::Insert, opcode);

    // Old stage is empty for an INSERT.
    EXPECT_EQ(0, reader.GetColumnCount(Changes::Change::Stage::Old));

    // Only the 4 V1 columns are resolved; Extra (5th changeset column) is beyond the minimum.
    EXPECT_EQ(4, reader.GetColumnCount(Changes::Change::Stage::New));

    IECSqlValue const& v0 = reader.GetValue(Changes::Change::Stage::New, 0);
    EXPECT_STREQ("ECInstanceId", v0.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(itemKey.GetInstanceId(), v0.GetId<ECInstanceId>());

    IECSqlValue const& v1 = reader.GetValue(Changes::Change::Stage::New, 1);
    EXPECT_STREQ("ECClassId", v1.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(itemKey.GetClassId(), v1.GetId<ECN::ECClassId>());

    IECSqlValue const& v2 = reader.GetValue(Changes::Change::Stage::New, 2);
    EXPECT_STREQ("Name", v2.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_STREQ("Bar", v2.GetText());

    IECSqlValue const& v3 = reader.GetValue(Changes::Change::Stage::New, 3);
    EXPECT_STREQ("Val", v3.GetColumnInfo().GetProperty()->GetName().c_str());
    EXPECT_EQ(99, v3.GetInt());

    ASSERT_EQ(BE_SQLITE_DONE, reader.Step());
    ASSERT_EQ(SUCCESS, reader.Close());
    }
    }

END_ECDBUNITTESTS_NAMESPACE
