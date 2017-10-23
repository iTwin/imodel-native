/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ChangeSummaryTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"

// #define DUMP_CHANGE_SUMMARY 1

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   12/16
//=======================================================================================
struct ChangeSummaryTestFixture : public ECDbTestFixture
    {
    protected:
        void DumpChangeSummary(ChangeSummary const& changeSummary, Utf8CP label);
        bool ChangeSummaryContainsInstance(ECDbCR ecdb, ChangeSummary const& changeSummary, ECInstanceId instanceId, Utf8CP schemaName, Utf8CP className, DbOpcode dbOpcode);
    };

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   12/16
//=======================================================================================
struct TestChangeSet : BeSQLite::ChangeSet
    {
    ConflictResolution _OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) override { BeAssert(false && "Unexpected conflict"); return ConflictResolution::Skip; }
    };

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   12/16
//=======================================================================================
struct TestChangeTracker : BeSQLite::ChangeTracker
    {
    TestChangeTracker(BeSQLite::DbR db) { SetDb(&db); }

    OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) override { return OnCommitStatus::Continue; }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/17
//---------------------------------------------------------------------------------------
void ChangeSummaryTestFixture::DumpChangeSummary(ChangeSummary const& changeSummary, Utf8CP label)
    {
#ifdef DUMP_CHANGE_SUMMARY
    printf("\t%s:\n", label);
    changeSummary.Dump();
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    072015
//---------------------------------------------------------------------------------------
bool ChangeSummaryTestFixture::ChangeSummaryContainsInstance(ECDbCR ecdb, ChangeSummary const& changeSummary, ECInstanceId instanceId, Utf8CP schemaName, Utf8CP className, DbOpcode dbOpcode)
    {
    Utf8String tableName = changeSummary.GetInstancesTableName();
    ECClassId classId = ecdb.Schemas().GetClassId(schemaName, className);

    Utf8PrintfString sql("SELECT NULL FROM %s WHERE ClassId=? AND InstanceId=? AND DbOpcode=?", tableName.c_str());
    CachedStatementPtr statement = ecdb.GetCachedStatement(sql.c_str());
    BeAssert(statement.IsValid());

    statement->BindId(1, classId);
    statement->BindId(2, instanceId);
    statement->BindInt(3, (int) dbOpcode);

    DbResult result = statement->Step();
    return (result == BE_SQLITE_ROW);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/16
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, InvalidSummary)
    {
    ASSERT_EQ(BE_SQLITE_OK, SetupECDb("invalidsummarytest.ecdb"));

    // Test1: Change to be_Prop table - should cause empty change summary without errors
    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    DbResult result = m_ecdb.SavePropertyString(PropertySpec("TestName", "TestNamespace"), "TestValue");
    ASSERT_EQ(BE_SQLITE_OK, result);

    TestChangeSet changeSet;
    result = changeSet.FromChangeTrack(tracker);
    ASSERT_EQ(BE_SQLITE_OK, result);

    ChangeSummary changeSummary(m_ecdb);
    BentleyStatus status = changeSummary.FromChangeSet(changeSet);
    ASSERT_EQ(SUCCESS, status);

    // Test2: Change to ec_ tables - should cause an error creating a change summary
    tracker.Restart();

    ASSERT_EQ(SUCCESS, ImportSchema(SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' alias='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'> "
        "</ECSchema>")));

    changeSet.Free();
    result = changeSet.FromChangeTrack(tracker);
    ASSERT_EQ(BE_SQLITE_OK, result);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/16
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, Overflow_PrimitiveProperties)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowProperties.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?> 
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1"> 
            <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
            <ECEntityClass typeName="Element" modifier="Abstract">
                <ECCustomAttributes>
                    <ClassMap xmlns="ECDbMap.02.00">
                        <MapStrategy>TablePerHierarchy</MapStrategy>
                    </ClassMap>
                    <ShareColumns xmlns="ECDbMap.02.00">
                      <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                    </ShareColumns>
                </ECCustomAttributes>
                <ECProperty propertyName="Code" typeName="string" />
            </ECEntityClass>
            <ECEntityClass typeName="TestElement" modifier="None">
                <BaseClass>Element</BaseClass>
                <ECProperty propertyName="S" typeName="string"/>
                <ECProperty propertyName="I" typeName="int"/>
                <ECProperty propertyName="L" typeName="long"/>
                <ECProperty propertyName="D" typeName="double"/>
                <ECProperty propertyName="DT" typeName="dateTime"/>
                <ECProperty propertyName="B" typeName="boolean"/>
            </ECEntityClass>
        </ECSchema>)xml")));

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ASSERT_EQ(BE_SQLITE_DONE, GetHelper().ExecuteECSql("INSERT INTO ts.TestElement(Code,S,I,L,D,DT,B) VALUES('C1', 'Str', 123, 12345, 23.5453, TIMESTAMP '2013-02-09T12:00:00', true)"));
    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    ASSERT_EQ(BE_SQLITE_OK, changeset.FromChangeTrack(tracker));

    ChangeSummary changeSummary(m_ecdb);
    ASSERT_EQ(SUCCESS, changeSummary.FromChangeSet(changeset));

    /*
    BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
    AccessString;OldValue;NewValue
    0:1;TestSchema:TestElement:60;Insert;No
    B;NULL;1
    Code;NULL;"C1"
    D;NULL;23.5453
    DT;NULL;2.45633e+06
    I;NULL;123
    L;NULL;12345
    S;NULL;"Str"
    */
    DumpChangeSummary(changeSummary, "Overflow_PrimitiveProperties");

    ChangeSummary::InstanceIterator instIter = changeSummary.MakeInstanceIterator();
    EXPECT_EQ(1, instIter.QueryCount());

    ChangeSummary::ValueIterator valIter = instIter.begin().GetInstance().MakeValueIterator();
    EXPECT_EQ(7, valIter.QueryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Maha.Nasir                    1/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, Overflow_StructProperty)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECStructClass typeName='StructProp' modifier='None'>"
        "        <ECProperty propertyName='S' typeName='string' readOnly='false'/>"
        "        <ECProperty propertyName='I' typeName='int'/>"
        "    </ECStructClass>"
        "    <ECEntityClass typeName='TestElement' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECStructProperty propertyName='SP' typeName='StructProp'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code, SP) VALUES ('C1', ?)"));

    IECSqlBinder& Binder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, Binder["S"].BindText("TestVal", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, Binder["I"].BindInt(123));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    auto rc = changeset.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == rc);

    ChangeSummary changeSummary(m_ecdb);
    BentleyStatus status = changeSummary.FromChangeSet(changeset);
    ASSERT_TRUE(SUCCESS == status);

    /*
    Code:"C1"
    SP.S:"TestVal"
    SP.I:123
    */

    ChangeSummary::InstanceIterator instIter = changeSummary.MakeInstanceIterator();
    EXPECT_EQ(1, instIter.QueryCount());

    ChangeSummary::ValueIterator valIter = instIter.begin().GetInstance().MakeValueIterator();
    EXPECT_EQ(3, valIter.QueryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Maha.Nasir                    1/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, Overflow_ArrayProperty)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestElement' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECArrayProperty propertyName='ArrayProp' typeName='double' minOccurs='0' maxOccurs='unbounded'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;
    double Array[] = { 123.3434, 345.223,-532.123 };
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code, ArrayProp) VALUES ('C1', ?)"));

    IECSqlBinder& Binder = stmt.GetBinder(1).AddArrayElement();
    for (size_t i = 0; i < 3; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, Binder.BindDouble(Array[i]));
        }
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    auto rc = changeset.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == rc);

    ChangeSummary changeSummary(m_ecdb);
    BentleyStatus status = changeSummary.FromChangeSet(changeset);
    ASSERT_TRUE(SUCCESS == status);

    /*
    Code:"C1"
    ArrayProp: { 123.3434, 345.223,-532.123 }
    */

    ChangeSummary::InstanceIterator instIter = changeSummary.MakeInstanceIterator();
    EXPECT_EQ(1, instIter.QueryCount());

    ChangeSummary::ValueIterator valIter = instIter.begin().GetInstance().MakeValueIterator();
    EXPECT_EQ(2, valIter.QueryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Maha.Nasir                    1/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, Overflow_ComplexPropertyTypes)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestElement' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECProperty propertyName='P2D' typeName='point2d'/>"
        "        <ECProperty propertyName='P3D' typeName='point3d'/>"
        "        <ECProperty propertyName='BIN' typeName='binary'/>"
        "        <ECProperty propertyName='Geom' typeName='Bentley.Geometry.Common.IGeometry'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;
    std::vector<Utf8Char> bin = { 'H', 'e', 'l','l', 'o' };
    IGeometryPtr geom = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code, P2D, P3D, Bin, Geom) VALUES ('C1', ?, ?, ?, ?)"));

    //Binding Point 2d & 3d
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(1, DPoint2d::From(-21, 22.1)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(2, DPoint3d::From(-12.53, 21.76, -32.22)));
    //Binding Binary blob
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBlob(3, &bin, static_cast<int>(bin.size()), IECSqlBinder::MakeCopy::No));
    //Binding Geometry
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGeometry(4, *geom));

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    auto rc = changeset.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == rc);

    ChangeSummary changeSummary(m_ecdb);
    BentleyStatus status = changeSummary.FromChangeSet(changeset);
    ASSERT_TRUE(SUCCESS == status);

    /*
    Code:"C1"
    P2D.X: -21
    P2D.Y: 22.1
    P3D.X: -12.53
    P3D.Y: 21.76
    P3D.Z: -32.22
    Bin: { 'H', 'e', 'l','l', 'o' }
    */

    ChangeSummary::InstanceIterator instIter = changeSummary.MakeInstanceIterator();
    EXPECT_EQ(1, instIter.QueryCount());

    ChangeSummary::ValueIterator valIter = instIter.begin().GetInstance().MakeValueIterator();
    EXPECT_EQ(8, valIter.QueryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Maha.Nasir                    1/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, Overflow_ArrayOfPoints)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestElement' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECArrayProperty propertyName='arrayOfP2d' typeName='point2d' minOccurs='0' maxOccurs='unbounded'/>"
        "        <ECArrayProperty propertyName='arrayOfP3d' typeName='point3d' minOccurs='0' maxOccurs='unbounded'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;

    DPoint2d ArrayOfP2d[] = { DPoint2d::From(-21, 22.1),DPoint2d::From(-85.34, 35.36),DPoint2d::From(-31.34, 12.35) };
    DPoint3d ArrayOfP3d[] = { DPoint3d::From(-41.33, 41.13, -12.25), DPoint3d::From(-23.37, 53.54, -34.31), DPoint3d::From(-33.41, 11.13, -99.11) };

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code, arrayOfP2d, arrayOfP3d) VALUES ('C1', ?, ?)"));

    //Binding Array of Point2d
    IECSqlBinder& arrayOfP2d = stmt.GetBinder(1);
    for (size_t i = 0; i < 3; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayOfP2d.AddArrayElement().BindPoint2d(ArrayOfP2d[i]));
        }

    //Binding Array of Point3d
    IECSqlBinder& arrayOfP3d = stmt.GetBinder(2);
    for (size_t i = 0; i < 3; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayOfP3d.AddArrayElement().BindPoint3d(ArrayOfP3d[i]));
        }

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    auto rc = changeset.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == rc);

    ChangeSummary changeSummary(m_ecdb);
    BentleyStatus status = changeSummary.FromChangeSet(changeset);
    ASSERT_TRUE(SUCCESS == status);

    ChangeSummary::InstanceIterator instIter = changeSummary.MakeInstanceIterator();
    EXPECT_EQ(1, instIter.QueryCount());

    ChangeSummary::ValueIterator valIter = instIter.begin().GetInstance().MakeValueIterator();
    EXPECT_EQ(3, valIter.QueryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Maha.Nasir                    1/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, Overflow_ArrayOfStructs)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECStructClass typeName='ST2' modifier='None'>"
        "        <ECProperty propertyName='D2' typeName='double' readOnly='false'/>"
        "        <ECProperty propertyName='P3D' typeName='point3d'/>"
        "    </ECStructClass>"
        "    <ECStructClass typeName='ST1' modifier='None'>"
        "        <ECProperty propertyName='D1' typeName='double' readOnly='false'/>"
        "        <ECProperty propertyName='P2D' typeName='point2d'/>"
        "        <ECStructProperty propertyName='ST2P' typeName='ST2'/>"
        "    </ECStructClass>"
        "    <ECEntityClass typeName='TestElement' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECStructArrayProperty propertyName='arrayOfST1' typeName='ST1' minOccurs='0' maxOccurs='unbounded'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;

    //Binding array of Struct
    double ArrayOfST1_D1[] = { 123.3434, 345.223,-532.123 };
    DPoint2d ArrayOfST1_P2D[] = { DPoint2d::From(-21, 22.1),DPoint2d::From(-85.34, 35.36),DPoint2d::From(-31.34, 12.35) };
    double ArrayOfST1_D2[] = { 12.3, -45.72, -31.11 };
    DPoint3d ArrayOfST1_P3D[] = { DPoint3d::From(-12.11, -74.1, 12.3),DPoint3d::From(-12.53, 21.76, -32.22),DPoint3d::From(-41.14, -22.45, -31.16) };

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.TestElement (Code, arrayOfST1) VALUES ('C1', ?)"));

    //Binding Array of Struct
    IECSqlBinder& arrayOfST1 = stmt.GetBinder(1);
    for (size_t i = 0; i < 3; i++)
        {
        IECSqlBinder& elementBinder = arrayOfST1.AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["D1"].BindDouble(ArrayOfST1_D1[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["P2D"].BindPoint2d(ArrayOfST1_P2D[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["ST2P"]["D2"].BindDouble(ArrayOfST1_D2[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["ST2P"]["P3D"].BindPoint3d(ArrayOfST1_P3D[i]));
        }

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    auto rc = changeset.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == rc);

    ChangeSummary changeSummary(m_ecdb);
    BentleyStatus status = changeSummary.FromChangeSet(changeset);
    ASSERT_TRUE(SUCCESS == status);

    ChangeSummary::InstanceIterator instIter = changeSummary.MakeInstanceIterator();
    EXPECT_EQ(1, instIter.QueryCount());

    ChangeSummary::ValueIterator valIter = instIter.begin().GetInstance().MakeValueIterator();
    EXPECT_EQ(2, valIter.QueryCount());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Maha.Nasir                    1/17
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, PropertiesWithRegularColumns)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECStructClass typeName='ST1' modifier='None'>"
        "        <ECProperty propertyName='D1' typeName='double' readOnly='false'/>"
        "        <ECProperty propertyName='I1' typeName='int'/>"
        "    </ECStructClass>"
        "    <ECStructClass typeName='ST2' modifier='None'>"
        "        <ECProperty propertyName='D2' typeName='double' readOnly='false'/>"
        "        <ECProperty propertyName='P3D' typeName='point3d'/>"
        "        <ECStructProperty propertyName='ST2P' typeName='ST1'/>"
        "    </ECStructClass>"
        "    <ECEntityClass typeName='Element' modifier='None'>"
        "        <ECProperty propertyName='S' typeName='string'/>"
        "        <ECProperty propertyName='I' typeName='int'/>"
        "        <ECProperty propertyName='L' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='double'/>"
        "        <ECProperty propertyName='DT' typeName='dateTime'/>"
        "        <ECProperty propertyName='B' typeName='boolean'/>"
        "        <ECProperty propertyName='P2D' typeName='point2d'/>"
        "        <ECProperty propertyName='P3D' typeName='point3d'/>"
        "        <ECProperty propertyName='BIN' typeName='binary'/>"
        "        <ECProperty propertyName='Geom' typeName='Bentley.Geometry.Common.IGeometry'/>"
        "        <ECStructProperty propertyName='StructProp' typeName='ST1'/>"
        "        <ECArrayProperty propertyName='ArrayProp' typeName='double' minOccurs='0' maxOccurs='unbounded'/>"
        "        <ECArrayProperty propertyName='arrayOfP2d' typeName='point2d' minOccurs='0' maxOccurs='unbounded'/>"
        "        <ECArrayProperty propertyName='arrayOfP3d' typeName='point3d' minOccurs='0' maxOccurs='unbounded'/>"
        "        <ECStructArrayProperty propertyName='arrayOfST2' typeName='ST2' minOccurs='0' maxOccurs='unbounded'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    int idx = 1;
    Utf8CP String = "TestVal";
    int Integer = 132;
    int64_t Long = 1235678901;
    double Double = 2341.432;
    DateTime DT = DateTime(DateTime::Kind::Unspecified, 2017, 1, 17, 0, 0);
    bool Boolean = false;
    DPoint2d P2D = DPoint2d::From(22.33, -21.34);
    DPoint3d P3D = DPoint3d::From(12.13, -42.34, -93.12);
    IGeometryPtr geom = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));
    std::vector<Utf8Char> Blob = { 'H', 'e', 'l','l', 'o' };
    double Array[] = { 123.3434, 345.223,-532.123 };
    DPoint2d ArrayOfP2d[] = { DPoint2d::From(-21, 22.1),DPoint2d::From(-85.34, 35.36),DPoint2d::From(-31.34, 12.35) };
    DPoint3d ArrayOfP3d[] = { DPoint3d::From(-41.33, 41.13, -12.25), DPoint3d::From(-23.37, 53.54, -34.31), DPoint3d::From(-33.41, 11.13, -99.11) };
    double ArrayOfST1_D1[] = { 123.3434, 345.223,-532.123 };
    int ArrayOfST1_I1[] = { 012, 456, 789 };
    double ArrayOfST2_D2[] = { 12.3, -45.72, -31.11 };
    DPoint3d ArrayOfST2_P3D[] = { DPoint3d::From(-12.11, -74.1, 12.3),DPoint3d::From(-12.53, 21.76, -32.22),DPoint3d::From(-41.14, -22.45, -31.16) };

    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.Element (S, I, L, D, DT, B, P2D, P3D, BIN, Geom, StructProp, ArrayProp , arrayOfP2d, arrayOfP3d, arrayOfST2) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(idx++, String, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(idx++, Integer));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(idx++, Long));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(idx++, Double));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(idx++, DT));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBoolean(idx++, Boolean));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(idx++, P2D));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(idx++, P3D));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBlob(idx++, &Blob, static_cast<int>(Blob.size()), IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGeometry(idx++, *geom));

    //Binding Struct property
    IECSqlBinder& StructBinder = stmt.GetBinder(idx++);
    ASSERT_EQ(ECSqlStatus::Success, StructBinder["D1"].BindDouble(Double));
    ASSERT_EQ(ECSqlStatus::Success, StructBinder["I1"].BindInt(123));

    //Binding Array property
    IECSqlBinder& ArrayBinder = stmt.GetBinder(idx++).AddArrayElement();
    for (size_t i = 0; i < 3; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, ArrayBinder.BindDouble(Array[i]));
        }

    //Binding Array of Point2d
    IECSqlBinder& arrayOfP2d = stmt.GetBinder(idx++);
    for (size_t i = 0; i < 3; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayOfP2d.AddArrayElement().BindPoint2d(ArrayOfP2d[i]));
        }

    //Binding Array of Point3d
    IECSqlBinder& arrayOfP3d = stmt.GetBinder(idx++);
    for (size_t i = 0; i < 3; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, arrayOfP3d.AddArrayElement().BindPoint3d(ArrayOfP3d[i]));
        }

    //Binding Array of Struct
    IECSqlBinder& arrayOfST2 = stmt.GetBinder(idx++);
    for (size_t i = 0; i < 3; i++)
        {
        IECSqlBinder& elementBinder = arrayOfST2.AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["D2"].BindDouble(ArrayOfST2_D2[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["P3D"].BindPoint3d(ArrayOfST2_P3D[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["ST2P"]["D1"].BindDouble(ArrayOfST1_D1[i]));
        ASSERT_EQ(ECSqlStatus::Success, elementBinder["ST2P"]["I1"].BindInt(ArrayOfST1_I1[i]));
        }

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    auto rc = changeset.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == rc);

    ChangeSummary changeSummary(m_ecdb);
    BentleyStatus status = changeSummary.FromChangeSet(changeset);
    ASSERT_TRUE(SUCCESS == status);

    ChangeSummary::InstanceIterator instIter = changeSummary.MakeInstanceIterator();
    EXPECT_EQ(1, instIter.QueryCount());

    ChangeSummary::ValueIterator valIter = instIter.begin().GetInstance().MakeValueIterator();
    EXPECT_EQ(19, valIter.QueryCount());
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2015
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, RelationshipChangesFromCurrentTransaction)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("RelationshipChangesFromCurrentTransaction.ecdb", SchemaItem::CreateForFile("StartupCompany.02.00.ecschema.xml")));
    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    // Insert Employee - FirstName, LastName
    // Insert Company - Name
    // Insert Hardware - Make (String), Model (String)
    // Insert EmployeeCompany - End Table relationship (Company__trg_11_id)
    // Insert EmployeeHardware - Link Table relationship

    ECSqlStatement statement;
    statement.Prepare(m_ecdb, "INSERT INTO StartupCompany.Employee (FirstName,LastName) VALUES('John','Doe')");
    ECInstanceKey employeeKey;
    DbResult stepStatus = statement.Step(employeeKey);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(m_ecdb, "INSERT INTO StartupCompany.Company (Name) VALUES('AcmeWorks')");
    ECInstanceKey companyKey1;
    stepStatus = statement.Step(companyKey1);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(m_ecdb, "INSERT INTO StartupCompany.Company (Name) VALUES('CmeaWorks')");
    ECInstanceKey companyKey2;
    stepStatus = statement.Step(companyKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(m_ecdb, "INSERT INTO StartupCompany.Hardware (Make,Model) VALUES('Tesla', 'Model-S')");
    ECInstanceKey hardwareKey1;
    stepStatus = statement.Step(hardwareKey1);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(m_ecdb, "INSERT INTO StartupCompany.Hardware (Make,Model) VALUES('Toyota', 'Prius')");
    ECInstanceKey hardwareKey2;
    stepStatus = statement.Step(hardwareKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    TestChangeSet changeSet;
    DbResult result = changeSet.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == result);

    ChangeSummary changeSummary(m_ecdb);
    BentleyStatus status = changeSummary.FromChangeSet(changeSet);
    ASSERT_TRUE(SUCCESS == status);

    DumpChangeSummary(changeSummary, "ChangeSummary after inserting instances");

    /*
        ChangeSummary after inserting instances:
        BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
                AccessString;OldValue;NewValue
        0:1;StartupCompany:Employee:79;Insert;No
                FirstName;NULL;"John"
                LastName;NULL;"Doe"
        0:2;StartupCompany:Company:71;Insert;No
                Name;NULL;"AcmeWorks"
        0:3;StartupCompany:Company:71;Insert;No
                Name;NULL;"CmeaWorks"
        0:4;StartupCompany:Hardware:75;Insert;No
                Make;NULL;"Tesla"
                Model;NULL;"Model-S"
        0:5;StartupCompany:Hardware:75;Insert;No
                Make;NULL;"Toyota"
                Model;NULL;"Prius"
    */
    EXPECT_EQ(5, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(m_ecdb, changeSummary, ECInstanceId(employeeKey.GetInstanceId().GetValueUnchecked()), "StartupCompany", "Employee", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(m_ecdb, changeSummary, ECInstanceId(companyKey1.GetInstanceId().GetValueUnchecked()), "StartupCompany", "Company", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(m_ecdb, changeSummary, ECInstanceId(companyKey2.GetInstanceId().GetValueUnchecked()), "StartupCompany", "Company", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(m_ecdb, changeSummary, ECInstanceId(hardwareKey1.GetInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", DbOpcode::Insert));
    EXPECT_TRUE(ChangeSummaryContainsInstance(m_ecdb, changeSummary, ECInstanceId(hardwareKey2.GetInstanceId().GetValueUnchecked()), "StartupCompany", "Hardware", DbOpcode::Insert));

    m_ecdb.SaveChanges();
    tracker.Restart();

    statement.Finalize();
    statement.Prepare(m_ecdb, "INSERT INTO StartupCompany.EmployeeHardware (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindId(1, employeeKey.GetClassId());
    statement.BindId(2, employeeKey.GetInstanceId());
    statement.BindId(3, hardwareKey1.GetClassId());
    statement.BindId(4, hardwareKey1.GetInstanceId());

    ECInstanceKey employeeHardwareKey;
    stepStatus = statement.Step(employeeHardwareKey);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    changeSummary.Free();
    changeSet.Free();

    result = changeSet.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == result);
    
    status = changeSummary.FromChangeSet(changeSet);
    ASSERT_TRUE(SUCCESS == status);

    DumpChangeSummary(changeSummary, "ChangeSummary after inserting relationships");

    /*
        ChangeSummary after inserting relationships:
        BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
                AccessString;OldValue;NewValue
        0:6;StartupCompany:EmployeeHardware:82;Insert;No
                SourceECClassId;NULL;StartupCompany:Employee:79
                SourceECInstanceId;NULL;0:1
                TargetECClassId;NULL;StartupCompany:Hardware:75
                TargetECInstanceId;NULL;0:4
    */
    EXPECT_EQ(1, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(m_ecdb, changeSummary, ECInstanceId(employeeHardwareKey.GetInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Insert));

    m_ecdb.SaveChanges();
    tracker.Restart();

    /* 
    * Note: ECDb doesn't support updates of relationships directly. Can only delete and re-insert relationships
    */
    statement.Finalize();
    statement.Prepare(m_ecdb, "DELETE FROM StartupCompany.EmployeeHardware WHERE EmployeeHardware.ECInstanceId=?");
    statement.BindId(1, employeeHardwareKey.GetInstanceId());
    stepStatus = statement.Step();
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    statement.Finalize();
    statement.Prepare(m_ecdb, "INSERT INTO StartupCompany.EmployeeHardware (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES(?,?,?,?)");
    statement.BindId(1, employeeKey.GetClassId());
    statement.BindId(2, employeeKey.GetInstanceId());
    statement.BindId(3, hardwareKey2.GetClassId());
    statement.BindId(4, hardwareKey2.GetInstanceId());

    ECInstanceKey employeeHardwareKey2;
    stepStatus = statement.Step(employeeHardwareKey2);
    ASSERT_TRUE(stepStatus == BE_SQLITE_DONE);

    changeSummary.Free();
    changeSet.Free();

    result = changeSet.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == result);

    status = changeSummary.FromChangeSet(changeSet);
    ASSERT_TRUE(SUCCESS == status);

    DumpChangeSummary(changeSummary, "ChangeSummary after updating (deleting and inserting different) relationships");

    /*
        ChangeSummary after updating (deleting and inserting different) relationships:
        BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
                AccessString;OldValue;NewValue
        0:6;StartupCompany:EmployeeHardware:82;Delete;No
                SourceECClassId;StartupCompany:Employee:79;NULL
                SourceECInstanceId;0:1;NULL
                TargetECClassId;StartupCompany:Hardware:75;NULL
                TargetECInstanceId;0:4;NULL
        0:7;StartupCompany:EmployeeHardware:82;Insert;No
                SourceECClassId;NULL;StartupCompany:Employee:79
                SourceECInstanceId;NULL;0:1
                TargetECClassId;NULL;StartupCompany:Hardware:75
                TargetECInstanceId;NULL;0:5
    */
    EXPECT_EQ(2, changeSummary.MakeInstanceIterator().QueryCount());
    EXPECT_TRUE(ChangeSummaryContainsInstance(m_ecdb, changeSummary, ECInstanceId(employeeHardwareKey.GetInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Delete));
    EXPECT_TRUE(ChangeSummaryContainsInstance(m_ecdb, changeSummary, ECInstanceId(employeeHardwareKey2.GetInstanceId().GetValueUnchecked()), "StartupCompany", "EmployeeHardware", DbOpcode::Insert));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/16
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, OverflowTables)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("overflowTables.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='GrandParent' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='A' typeName='string' />"
        "        <ECProperty propertyName='B' typeName='string' />"
        "        <ECProperty propertyName='C' typeName='string' />"
        "        <ECProperty propertyName='D' typeName='string' />"
        "        <ECProperty propertyName='E' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Parent' modifier='None'>"
        "        <BaseClass>GrandParent</BaseClass>"
        "        <ECCustomAttributes>"
        "            <JoinedTablePerDirectSubclass xmlns='ECDbMap.2.0'/>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='F' typeName='string'/>"
        "        <ECProperty propertyName='G' typeName='string'/>"
        "        <ECProperty propertyName='H' typeName='string'/>"
        "        <ECProperty propertyName='I' typeName='string'/>"
        "        <ECProperty propertyName='J' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='Child' modifier='None'>"
        "        <BaseClass>Parent</BaseClass>"
        "        <ECCustomAttributes>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <MaxSharedColumnsBeforeOverflow>2</MaxSharedColumnsBeforeOverflow>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='K' typeName='string'/>"
        "        <ECProperty propertyName='L' typeName='string'/>"
        "        <ECProperty propertyName='M' typeName='string'/>"
        "        <ECProperty propertyName='N' typeName='string'/>"
        "        <ECProperty propertyName='O' typeName='string'/>"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='GrandChild' modifier='None'>"
        "        <BaseClass>Child</BaseClass>"
        "        <ECProperty propertyName='P' typeName='string'/>"
        "        <ECProperty propertyName='Q' typeName='string'/>"
        "        <ECProperty propertyName='R' typeName='string'/>"
        "        <ECProperty propertyName='S' typeName='string'/>"
        "        <ECProperty propertyName='T' typeName='string'/>"
        "    </ECEntityClass>"
        "</ECSchema>")));

    TestChangeTracker tracker(m_ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.GrandChild (A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T) VALUES ('A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T')"));

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    m_ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    auto rc = changeset.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == rc);

    ChangeSummary changeSummary(m_ecdb);
    BentleyStatus status = changeSummary.FromChangeSet(changeset);
    ASSERT_TRUE(SUCCESS == status);

    /*
    BriefcaseId:LocalId;SchemaName:ClassName:ClassId;DbOpcode;Indirect
            AccessString;OldValue;NewValue
    0:1;TestSchema:GrandChild:53;Insert;No
            A;NULL;"A"
            B;NULL;"B"
            C;NULL;"C"
            D;NULL;"D"
            E;NULL;"E"
            F;NULL;"F"
            G;NULL;"G"
            H;NULL;"H"
            I;NULL;"I"
            J;NULL;"J"
            K;NULL;"K"
            L;NULL;"L"
            M;NULL;"M"
            N;NULL;"N"
            O;NULL;"O"
            P;NULL;"P"
            Q;NULL;"Q"
            R;NULL;"R"
            S;NULL;"S"
            T;NULL;"T"
    */
    DumpChangeSummary(changeSummary, "OverflowTables");

    ChangeSummary::InstanceIterator instIter = changeSummary.MakeInstanceIterator();
    EXPECT_EQ(1, instIter.QueryCount());

    ChangeSummary::ValueIterator valIter = instIter.begin().GetInstance().MakeValueIterator();
    EXPECT_EQ(20, valIter.QueryCount());
    }

END_ECDBUNITTESTS_NAMESPACE
