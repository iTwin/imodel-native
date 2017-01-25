/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ChangeSummary_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "SchemaImportTestFixture.h"
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
        BentleyStatus ImportSchemas(ECDbR ecdb, SchemaItem const& schemaItem);
    };

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   12/16
//=======================================================================================
struct TestChangeSet : BeSQLite::ChangeSet
    {
    ConflictResolution _OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) override
        {
        BeAssert(false && "Unexpected conflict");
        return ConflictResolution::Skip;
        }
    };

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   12/16
//=======================================================================================
struct TestChangeTracker : BeSQLite::ChangeTracker
    {
    TestChangeTracker(BeSQLite::DbR db)
        {
        SetDb(&db);
        }

    OnCommitStatus _OnCommit(bool isCommit, Utf8CP operation) override
        {
        return OnCommitStatus::Continue;
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/17
//---------------------------------------------------------------------------------------
BentleyStatus ChangeSummaryTestFixture::ImportSchemas(ECDbR ecdb, SchemaItem const& schemaItem)
    {
    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    context->AddSchemaLocater(ecdb.GetSchemaLocater());

    for (Utf8StringCR schemaXml : schemaItem.m_schemaXmlList)
        {
        if (SUCCESS != ECDbTestUtility::ReadECSchemaFromString(context, schemaXml.c_str()))
            return ERROR;
        }

    if (SUCCESS != ecdb.Schemas().ImportECSchemas(context->GetCache().GetSchemas()))
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/16
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, InvalidSummary)
    {
    ECDbR ecdb = SetupECDb("invalidsummarytest.ecdb");
    ASSERT_TRUE(ecdb.IsDbOpen());

    // Test1: Change to be_Prop table - should cause empty change summary without errors
    TestChangeTracker tracker(ecdb);
    tracker.EnableTracking(true);

    DbResult result = ecdb.SavePropertyString(PropertySpec("TestName", "TestNamespace"), "TestValue");
    EXPECT_EQ(BE_SQLITE_OK, result);

    TestChangeSet changeSet;
    result = changeSet.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == result);

    ChangeSummary changeSummary(ecdb);
    BentleyStatus status = changeSummary.FromChangeSet(changeSet);
    ASSERT_TRUE(SUCCESS == status);

    // Test2: Change to ec_ tables - should cause an error creating a change summary
    tracker.Restart();

    status = ImportSchemas(ecdb, SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "</ECSchema>"));

    changeSet.Free();
    result = changeSet.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == result);

    changeSummary.Free();
    BeTest::SetFailOnAssert(false);
    status = changeSummary.FromChangeSet(changeSet);
    BeTest::SetFailOnAssert(true);
    ASSERT_TRUE(ERROR == status);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    12/16
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, Overflow_PrimitiveProperties)
    {
    ECDbR ecdb = SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>1</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestElement' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECProperty propertyName='S' typeName='string'/>"
        "        <ECProperty propertyName='I' typeName='int'/>"
        "        <ECProperty propertyName='L' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='double'/>"
        "        <ECProperty propertyName='DT' typeName='dateTime'/>"
        "        <ECProperty propertyName='B' typeName='boolean'/>"
        "    </ECEntityClass>"
        "</ECSchema>"));


    ASSERT_TRUE(ecdb.IsDbOpen());

    TestChangeTracker tracker(ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.TestElement (Code, S, I, L, D, DT, B) VALUES ('C1', 'Str', 123, 12345, 23.5453, TIMESTAMP '2013-02-09T12:00:00', true)"));

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    auto rc = changeset.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == rc);

    ChangeSummary changeSummary(ecdb);
    BentleyStatus status = changeSummary.FromChangeSet(changeset);
    ASSERT_TRUE(SUCCESS == status);

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
#ifdef DUMP_CHANGE_SUMMARY
    changeSummary.Dump();
#endif

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
    ECDbR ecdb = SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>1</SharedColumnCount>"
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
        "</ECSchema>"));


    ASSERT_TRUE(ecdb.IsDbOpen());

    TestChangeTracker tracker(ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.TestElement (Code, SP) VALUES ('C1', ?)"));

    IECSqlBinder& Binder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, Binder["S"].BindText("TestVal", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, Binder["I"].BindInt(123));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    auto rc = changeset.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == rc);

    ChangeSummary changeSummary(ecdb);
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
    ECDbR ecdb = SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>1</SharedColumnCount>"
        "              <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>"
        "            </ShareColumns>"
        "        </ECCustomAttributes>"
        "        <ECProperty propertyName='Code' typeName='string' />"
        "    </ECEntityClass>"
        "    <ECEntityClass typeName='TestElement' modifier='None'>"
        "        <BaseClass>Element</BaseClass>"
        "        <ECArrayProperty propertyName='ArrayProp' typeName='double' minOccurs='0' maxOccurs='unbounded'/>"
        "    </ECEntityClass>"
        "</ECSchema>"));


    ASSERT_TRUE(ecdb.IsDbOpen());

    TestChangeTracker tracker(ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;
    double Array[] = { 123.3434, 345.223,-532.123 };
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.TestElement (Code, ArrayProp) VALUES ('C1', ?)"));

    IECSqlBinder& Binder = stmt.GetBinder(1).AddArrayElement();
    for (size_t i = 0; i < 3; i++)
        {
        ASSERT_EQ(ECSqlStatus::Success, Binder.BindDouble(Array[i]));
        }
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    auto rc = changeset.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == rc);

    ChangeSummary changeSummary(ecdb);
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
    ECDbR ecdb = SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>1</SharedColumnCount>"
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
        "</ECSchema>"));


    ASSERT_TRUE(ecdb.IsDbOpen());

    TestChangeTracker tracker(ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;
    std::vector<Utf8Char> bin = { 'H', 'e', 'l','l', 'o' };
    IGeometryPtr geom = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, 1.0, 1.0, 1.0)));

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.TestElement (Code, P2D, P3D, Bin, Geom) VALUES ('C1', ?, ?, ?, ?)"));

    //Binding Point 2d & 3d
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(1, DPoint2d::From(-21, 22.1)));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(2, DPoint3d::From(-12.53, 21.76, -32.22)));
    //Binding Binary blob
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBlob(3, &bin, static_cast<int>(bin.size()), IECSqlBinder::MakeCopy::No));
    //Binding Geometry
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGeometry(4, *geom));

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    auto rc = changeset.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == rc);

    ChangeSummary changeSummary(ecdb);
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
    ECDbR ecdb = SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>1</SharedColumnCount>"
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
        "</ECSchema>"));


    ASSERT_TRUE(ecdb.IsDbOpen());

    TestChangeTracker tracker(ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;

    DPoint2d ArrayOfP2d[] = { DPoint2d::From(-21, 22.1),DPoint2d::From(-85.34, 35.36),DPoint2d::From(-31.34, 12.35) };
    DPoint3d ArrayOfP3d[] = { DPoint3d::From(-41.33, 41.13, -12.25), DPoint3d::From(-23.37, 53.54, -34.31), DPoint3d::From(-33.41, 11.13, -99.11) };

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.TestElement (Code, arrayOfP2d, arrayOfP3d) VALUES ('C1', ?, ?)"));

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
    ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    auto rc = changeset.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == rc);

    ChangeSummary changeSummary(ecdb);
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
    ECDbR ecdb = SetupECDb("overflowProperties.ecdb", SchemaItem(
        "<?xml version='1.0' encoding='utf-8'?> "
        "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'> "
        "    <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
        "    <ECEntityClass typeName='Element' modifier='Abstract'>"
        "        <ECCustomAttributes>"
        "            <ClassMap xmlns='ECDbMap.02.00'>"
        "                <MapStrategy>TablePerHierarchy</MapStrategy>"
        "            </ClassMap>"
        "            <ShareColumns xmlns='ECDbMap.02.00'>"
        "              <SharedColumnCount>1</SharedColumnCount>"
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
        "</ECSchema>"));


    ASSERT_TRUE(ecdb.IsDbOpen());

    TestChangeTracker tracker(ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;

    //Binding array of Struct
    double ArrayOfST1_D1[] = { 123.3434, 345.223,-532.123 };
    DPoint2d ArrayOfST1_P2D[] = { DPoint2d::From(-21, 22.1),DPoint2d::From(-85.34, 35.36),DPoint2d::From(-31.34, 12.35) };
    double ArrayOfST1_D2[] = { 12.3, -45.72, -31.11 };
    DPoint3d ArrayOfST1_P3D[] = { DPoint3d::From(-12.11, -74.1, 12.3),DPoint3d::From(-12.53, 21.76, -32.22),DPoint3d::From(-41.14, -22.45, -31.16) };

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.TestElement (Code, arrayOfST1) VALUES ('C1', ?)"));

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

    ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    auto rc = changeset.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == rc);

    ChangeSummary changeSummary(ecdb);
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
    ECDbR ecdb = SetupECDb("overflowProperties.ecdb", SchemaItem(
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
        "</ECSchema>"));


    ASSERT_TRUE(ecdb.IsDbOpen());

    TestChangeTracker tracker(ecdb);
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

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.Element (S, I, L, D, DT, B, P2D, P3D, BIN, Geom, StructProp, ArrayProp , arrayOfP2d, arrayOfP3d, arrayOfST2) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"));
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
    ecdb.SaveChanges();

    ASSERT_TRUE(tracker.HasChanges());

    TestChangeSet changeset;
    auto rc = changeset.FromChangeTrack(tracker);
    ASSERT_TRUE(BE_SQLITE_OK == rc);

    ChangeSummary changeSummary(ecdb);
    BentleyStatus status = changeSummary.FromChangeSet(changeset);
    ASSERT_TRUE(SUCCESS == status);

    ChangeSummary::InstanceIterator instIter = changeSummary.MakeInstanceIterator();
    EXPECT_EQ(1, instIter.QueryCount());

    ChangeSummary::ValueIterator valIter = instIter.begin().GetInstance().MakeValueIterator();
    EXPECT_EQ(19, valIter.QueryCount());
    }

END_ECDBUNITTESTS_NAMESPACE
