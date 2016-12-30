/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ChangeSummary_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
    {};

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
// @bsimethod                                Ramanujam.Raman                    12/16
//---------------------------------------------------------------------------------------
TEST_F(ChangeSummaryTestFixture, OverflowProperties)
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
        "        <ECProperty propertyName='S' typeName='string'/>"
        "        <ECProperty propertyName='I' typeName='int'/>"
        "        <ECProperty propertyName='L' typeName='long'/>"
        "        <ECProperty propertyName='D' typeName='double'/>"
        "        <ECProperty propertyName='DT' typeName='dateTime'/>"
        "        <ECProperty propertyName='B' typeName='boolean'/>"
        "        <ECProperty propertyName='P2D' typeName='point2d'/>"
        "        <ECProperty propertyName='P3D' typeName='point3d'/>"
        "        <ECStructProperty propertyName='ST1P' typeName='ST1'/>"
        "        <ECArrayProperty propertyName='arrayOfP3d' typeName='point3d' minOccurs='0' maxOccurs='unbounded'/>"
        "        <ECStructArrayProperty propertyName='arrayOfST1' typeName='ST1' minOccurs='0' maxOccurs='unbounded'/>"
        "        <ECProperty propertyName='BIN' typeName='binary'/>"
        "    </ECEntityClass>"
        "</ECSchema>"));


    ASSERT_TRUE(ecdb.IsDbOpen());
    ASSERT_EQ(BentleyStatus::SUCCESS, ecdb.Schemas().CreateECClassViewsInDb());

    TestChangeTracker tracker(ecdb);
    tracker.EnableTracking(true);

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO ts.TestElement (Code, S, I, L, D, DT, B, P2D.X, P2D.Y, P3D.X, P3D.Y, P3D.Z, ST1P.D1, ST1P.P2D.X, ST1P.P2D.Y, ST1P.ST2P.D2, ST1P.ST2P.P3D.X, ST1P.ST2P.P3D.Y, ST1P.ST2P.P3D.Z, arrayOfP3d, arrayOfST1, BIN) "
                                                 "VALUES ('C1', 'Str', 123, 12345, 23.5453, TIMESTAMP '2013-02-09T12:00:00', true, 12.34, 45.45, 56.34, 67.44, 14.44, 22312.34, 34.14, 86.54, 34.23, 23.55, 64.34, 34.45, null, null, null)"));
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
        P2D.X;NULL;12.34
        P2D.Y;NULL;45.45
        P3D.X;NULL;56.34
        P3D.Y;NULL;67.44
        P3D.Z;NULL;14.44
        S;NULL;"Str"
        ST1P.D1;NULL;22312.3
        ST1P.P2D.X;NULL;34.14
        ST1P.P2D.Y;NULL;86.54
        ST1P.ST2P.D2;NULL;34.23
        ST1P.ST2P.P3D.X;NULL;23.55
        ST1P.ST2P.P3D.Y;NULL;64.34
        ST1P.ST2P.P3D.Z;NULL;34.45
    */
#ifdef DUMP_CHANGE_SUMMARY
    changeSummary.Dump();
#endif

    ChangeSummary::InstanceIterator instIter = changeSummary.MakeInstanceIterator();
    EXPECT_EQ(1, instIter.QueryCount());

    ChangeSummary::ValueIterator valIter = instIter.begin().GetInstance().MakeValueIterator();
    EXPECT_EQ(19, valIter.QueryCount());

    // TODO: Extend this test to cover array of points, array of structures and binary blob also!!
    }

END_ECDBUNITTESTS_NAMESPACE
