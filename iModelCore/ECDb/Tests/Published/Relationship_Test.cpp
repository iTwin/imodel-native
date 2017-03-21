/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/Relationship_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include "../BackDoor/PublicAPI/BackDoor/ECDb/BackDoor.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE
struct RelationshipMappingTestFixture : DbMappingTestFixture
    {};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, LogicalForeignKeyRelationship)
    {
    SetupECDb("LogicalForeignKeyRelationship.ecdb",
              SchemaItem("Diamond Problem",
                         "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                         "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                         "  <ECCustomAttributeClass typeName='Interface' appliesTo='EntityClass' modifier='Sealed' />"
                         "  <ECEntityClass typeName='PrimaryClassA'>"
                         "      <ECCustomAttributes>"
                         "          <ClassMap xmlns='ECDbMap.02.00'>"
                         "              <MapStrategy>TablePerHierarchy</MapStrategy>"
                         "          </ClassMap>"
                         "          <ShareColumns xmlns='ECDbMap.02.00'>"
                         "              <SharedColumnCount>5</SharedColumnCount>" //
                         "              <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>"
                         "          </ShareColumns>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='P1' typeName='long' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='SecondaryClassA'>"
                         "      <ECCustomAttributes>"
                         "          <ClassMap xmlns='ECDbMap.02.00'>"
                         "              <MapStrategy>TablePerHierarchy</MapStrategy>"
                         "          </ClassMap>"
                         "          <ShareColumns xmlns='ECDbMap.02.00'>"
                         "              <SharedColumnCount>2</SharedColumnCount>" //
                         "              <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>"
                         "          </ShareColumns>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='T1' typeName='long' />"
                         "      <ECNavigationProperty propertyName='PrimaryClassA' relationshipName='PrimaryClassAHasSecondaryClassA' direction='Backward' />"
                         "  </ECEntityClass>"
                         "   <ECRelationshipClass typeName='PrimaryClassAHasSecondaryClassA' strength='Referencing' modifier='Abstract'>"
                         "      <Source cardinality='(0,1)' polymorphic='False'>"
                         "          <Class class ='PrimaryClassA' />"
                         "      </Source>"
                         "      <Target cardinality='(0,N)' polymorphic='False'>"
                         "          <Class class ='SecondaryClassA' />"
                         "      </Target>"
                         "   </ECRelationshipClass>"
                         "   <ECRelationshipClass typeName='PrimaryClassAHasSecondaryClassB' strength='Referencing' modifier='Sealed'>"
                         "       <BaseClass>PrimaryClassAHasSecondaryClassA</BaseClass> "
                         "      <Source cardinality='(0,1)' polymorphic='False'>"
                         "          <Class class ='PrimaryClassA' />"
                         "      </Source>"
                         "      <Target cardinality='(0,N)' polymorphic='False'>"
                         "          <Class class ='SecondaryClassA' />"
                         "      </Target>"
                         "   </ECRelationshipClass>"
                         "</ECSchema>"));

    ASSERT_TRUE(GetECDb().IsDbOpen());
    GetECDb().Schemas().CreateClassViewsInDb();
    GetECDb().SaveChanges();
    ECClassId primaryClassAHasSecondaryClassAId = GetECDb().Schemas().GetClassId("TestSchema", "PrimaryClassAHasSecondaryClassA");
    ECClassId primaryClassAHasSecondaryClassBId = GetECDb().Schemas().GetClassId("TestSchema", "PrimaryClassAHasSecondaryClassB");

    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.PrimaryClassA(ECInstanceId, P1) VALUES(101, 10000)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.PrimaryClassA(ECInstanceId, P1) VALUES(102, 20000)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.PrimaryClassA(ECInstanceId, P1) VALUES(103, 30000)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.PrimaryClassA(ECInstanceId, P1) VALUES(104, 40000)"));

    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), Utf8PrintfString("INSERT INTO ts.SecondaryClassA(ECInstanceId, T1, PrimaryClassA.Id, PrimaryClassA.RelECClassId) VALUES(201, 10000, 101, %ld)", primaryClassAHasSecondaryClassBId.GetValue()).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.SecondaryClassA(ECInstanceId, T1, PrimaryClassA.Id) VALUES(202, 20000, 102)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.SecondaryClassA(ECInstanceId, T1) VALUES(203, 30000)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.SecondaryClassA(ECInstanceId, T1) VALUES(204, 40000)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), Utf8PrintfString("UPDATE ts.SecondaryClassA SET PrimaryClassA.Id = 103, T1=300002, PrimaryClassA.RelECClassId = %ld  WHERE ECInstanceId = 203", primaryClassAHasSecondaryClassBId.GetValue()).c_str()));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.PrimaryClassAHasSecondaryClassB(SourceECInstanceId, TargetECInstanceId) VALUES(104, 204)"));
    GetECDb().SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Affan.Khan                         01/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, LogicalForeignKeyRelationshipMappedToSharedColumn)
    {
    SetupECDb("logicalfk_sharedcol.ecdb",
              SchemaItem("Diamond Problem",
                         "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                         "  <ECSchemaReference name='ECDbMap' version='02.00.00' alias='ecdbmap' />"
                         "  <ECSchemaReference name='CoreCustomAttributes' version='01.00.00' alias='CoreCA'/>"
                         "  <ECEntityClass typeName='Equipment'  modifier='Abstract'>"
                         "      <ECCustomAttributes>"
                         "          <ClassMap xmlns='ECDbMap.02.00'>"
                         "              <MapStrategy>TablePerHierarchy</MapStrategy>"
                         "          </ClassMap>"
                         "          <JoinedTablePerDirectSubclass xmlns='ECDbMap.02.00'/>"
                         "          <ShareColumns xmlns='ECDbMap.02.00'>"
                         "              <SharedColumnCount>10</SharedColumnCount>"
                         "              <ApplyToSubclassesOnly>False</ApplyToSubclassesOnly>"
                         "          </ShareColumns>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='Code' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='IEndPoint' modifier='Abstract'>"
                         "      <ECCustomAttributes>"
                         "          <IsMixin xmlns='CoreCustomAttributes.01.00'>"
                         "              <AppliesToEntityClass>Equipment</AppliesToEntityClass>"
                         "          </IsMixin>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='www' typeName='long' />"
                         "  </ECEntityClass>"
                         "  <ECRelationshipClass typeName='BaseRelationship' strength='holding' strengthDirection='Forward' modifier='Abstract'>"
                         "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='A'>"
                         "         <Class class='Car' />"
                         "     </Source>"
                         "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='B'>"
                         "        <Class class='IEndPoint' />"
                         "     </Target>"
                         "  </ECRelationshipClass>"
                         "  <ECRelationshipClass typeName='CarHasEndPoint' strength='holding' strengthDirection='Forward' modifier='Sealed'>"
                         "      <BaseClass>BaseRelationship</BaseClass>"
                         "      <Source multiplicity='(0..1)' polymorphic='False' roleLabel='A'>"
                         "         <Class class='Car' />"
                         "     </Source>"
                         "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='B'>"
                         "        <Class class='IEndPoint' />"
                         "     </Target>"
                         "  </ECRelationshipClass>"
                         "  <ECEntityClass typeName='Car'>"
                         "      <ECProperty propertyName='Name' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='Engine'>"
                         "      <BaseClass>Equipment</BaseClass>"
                         "      <BaseClass>IEndPoint</BaseClass>"
                         "      <ECProperty propertyName='Volumn' typeName='double' />"
                         "      <ECNavigationProperty propertyName='Car' relationshipName='CarHasEndPoint' direction='Backward' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='Sterring'>"
                         "      <BaseClass>Equipment</BaseClass>"
                         "      <BaseClass>IEndPoint</BaseClass>"
                         "      <ECProperty propertyName='Type' typeName='string' />"
                         "      <ECNavigationProperty propertyName='Car' relationshipName='CarHasEndPoint' direction='Backward' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='Tire'>"
                         "      <BaseClass>Equipment</BaseClass>"
                         "      <ECProperty propertyName='Diameter' typeName='double' />"
                         "  </ECEntityClass>"

                         "</ECSchema>"));
    ASSERT_TRUE(GetECDb().IsDbOpen());
    GetECDb().SaveChanges();

    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.Car            (Name                ) VALUES ('BMW-S')"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.Engine         (Code, www, Volumn,Car.Id,Car.RelECClassId ) VALUES ('CODE-1','www1', 2000.0,1,53 )"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.Sterring       (Code, www, Type,Car.Id,Car.RelECClassId   ) VALUES ('CODE-2','www2', 'S-Type',1,53)"));
    ASSERT_EQ(BE_SQLITE_DONE, ExecuteNonSelectECSql(GetECDb(), "INSERT INTO ts.Tire           (Code, Diameter      ) VALUES ('CODE-3', 15.0)"));


    GetECDb().Schemas().CreateClassViewsInDb();
    GetECDb().SaveChanges();
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT ECInstanceId, ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId FROM ts.CarHasEndPoint"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(2, stmt.GetValueInt64(0));
    ASSERT_EQ(53, stmt.GetValueInt64(1));
    ASSERT_EQ(1, stmt.GetValueInt64(2));
    ASSERT_EQ(51, stmt.GetValueInt64(3));
    ASSERT_EQ(2, stmt.GetValueInt64(4));
    ASSERT_EQ(54, stmt.GetValueInt64(5));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    ASSERT_EQ(3, stmt.GetValueInt64(0));
    ASSERT_EQ(53, stmt.GetValueInt64(1));
    ASSERT_EQ(1, stmt.GetValueInt64(2));
    ASSERT_EQ(51, stmt.GetValueInt64(3));
    ASSERT_EQ(3, stmt.GetValueInt64(4));
    ASSERT_EQ(56, stmt.GetValueInt64(5));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT Car.Id,Car.RelECClassId FROM ts.Engine"));
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "SELECT Car.Id,Car.RelECClassId FROM ts.Sterring"));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     03/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipMappingTestFixture, LogicalForeignKeyRelationshipMappedToUnsharedColumn)
    {
    SetupECDb("logicalfk_unsharedcol.ecdb",
              SchemaItem("Diamond Problem",
                         "<ECSchema schemaName='TestSchema' alias='ts' version='1.0.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.1'>"
                         "  <ECSchemaReference name='ECDbMap' version='02.00.00' alias='ecdbmap' />"
                         "  <ECEntityClass typeName='Model' modifier='None'>"
                         "      <ECProperty propertyName='Name' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='Element'  modifier='None'>"
                         "      <ECCustomAttributes>"
                         "          <ClassMap xmlns='ECDbMap.02.00'>"
                         "              <MapStrategy>TablePerHierarchy</MapStrategy>"
                         "          </ClassMap>"
                         "      </ECCustomAttributes>"
                         "      <ECProperty propertyName='Code' typeName='string' />"
                         "      <ECNavigationProperty propertyName='Model' relationshipName='Rel' direction='Backward' />"
                         "  </ECEntityClass>"
                         "  <ECRelationshipClass typeName='Rel' strength='referencing' strengthDirection='Forward' modifier='Abstract'>"
                         "      <Source multiplicity='(1..1)' polymorphic='False' roleLabel='Model'>"
                         "         <Class class='Model' />"
                         "     </Source>"
                         "      <Target multiplicity='(0..*)' polymorphic='True' roleLabel='Element'>"
                         "        <Class class='Element' />"
                         "     </Target>"
                         "  </ECRelationshipClass>"
                         "</ECSchema>"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    GetECDb().Schemas().CreateClassViewsInDb();
    GetECDb().SaveChanges();

    ECClassCP elementClass = GetECDb().Schemas().GetClass("TestSchema", "Element");
    ASSERT_TRUE(elementClass != nullptr);

    AssertForeignKey(false, GetECDb(), "ts_Element", "ModelId");
    AssertIndexExists(GetECDb(), "x_ts_Element_fk_ts_Rel_target", false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
struct ECDbHoldingRelationshipStrengthTestFixture : DbMappingTestFixture
    {
    protected:
        bool InstanceExists(Utf8CP classExp, ECInstanceKey const& key) const
            {
            Utf8String ecsql;
            ecsql.Sprintf("SELECT NULL FROM %s WHERE ECInstanceId=?", classExp);
            ECSqlStatement stmt;
            EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), ecsql.c_str())) << ecsql.c_str();
            EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId()));

            DbResult stat = stmt.Step();
            EXPECT_TRUE(stat == BE_SQLITE_ROW || stat == BE_SQLITE_DONE);
            return stat == BE_SQLITE_ROW;
            };

        bool RelationshipExists(Utf8CP relClassExp, ECInstanceKey const& sourceKey, ECInstanceKey const& targetKey) const
            {
            Utf8String ecsql;
            ecsql.Sprintf("SELECT NULL FROM %s WHERE SourceECInstanceId=? AND SourceECClassId=? AND TargetECInstanceId=? AND TargetECClassId=?", relClassExp);
            ECSqlStatement stmt;
            EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), ecsql.c_str())) << ecsql.c_str();
            EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(1, sourceKey.GetInstanceId()));
            EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(2, sourceKey.GetClassId()));
            EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(3, targetKey.GetInstanceId()));
            EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(4, targetKey.GetClassId()));

            DbResult stat = stmt.Step();
            EXPECT_TRUE(stat == BE_SQLITE_ROW || stat == BE_SQLITE_DONE);
            return stat == BE_SQLITE_ROW;
            };
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbHoldingRelationshipStrengthTestFixture, OneToOneForward)
    {
    SetupECDb("ecdbrelationshipmappingrules_onetomanyandholding.ecdb",
              SchemaItem("1:N and holding",
                         "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                         "<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                         "  <ECEntityClass typeName='Geometry' >"
                         "    <ECProperty propertyName='Type' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='GeometryPart' >"
                         "    <ECProperty propertyName='Stream' typeName='binary' />"
                         "  </ECEntityClass>"
                         "  <ECRelationshipClass typeName='GeometryHoldsParts' strength='holding' strengthDirection='Forward' modifier='Sealed'>"
                         "     <ECCustomAttributes>"
                         "         <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                         "     </ECCustomAttributes>"
                         "     <Source cardinality='(0,1)' polymorphic='True'>"
                         "         <Class class='Geometry' />"
                         "     </Source>"
                         "    <Target cardinality='(0,1)' polymorphic='True'>"
                         "        <Class class='GeometryPart' />"
                         "     </Target>"
                         "  </ECRelationshipClass>"
                         "</ECSchema>"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    ECInstanceKey geomKey1;
    ECInstanceKey geomKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.Geometry(Type) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Polygon", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey1));
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Solid", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey2));
    }

    ECInstanceKey partKey1;
    ECInstanceKey partKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.GeometryPart(Stream) VALUES(randomblob(4))"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey1));
    stmt.Reset();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey2));
    }

    {
    //Create relationships:
    //Geom-Part
    //1-1
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.GeometryHoldsParts(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, geomKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey1.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, partKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, partKey1.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    }

    GetECDb().SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(GetECDb(), "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey1.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, delGeomStmt.Step());
    delGeomStmt.Reset();
    delGeomStmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Geometry", geomKey1));
    ASSERT_TRUE(InstanceExists("ts.Geometry", geomKey2));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey1));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey2));
    ASSERT_FALSE(RelationshipExists("ts.GeometryHoldsParts", geomKey1, partKey1)) << "ECSQL DELETE deletes affected relationships";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbHoldingRelationshipStrengthTestFixture, OneToOneBackward)
    {
    SetupECDb("ecdbrelationshipmappingrules_onetomanyandholding.ecdb",
              SchemaItem("1:N and holding",
                         "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                         "  <ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
                         "  <ECEntityClass typeName='Geometry' >"
                         "    <ECProperty propertyName='Type' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='GeometryPart' >"
                         "    <ECProperty propertyName='Stream' typeName='binary' />"
                         "  </ECEntityClass>"
                         "  <ECRelationshipClass typeName='PartHeldByGeometry' strength='holding' strengthDirection='Backward' modifier='Sealed'>"
                         "    <ECCustomAttributes>"
                         "        <ForeignKeyConstraint xmlns='ECDbMap.02.00'/>"
                         "    </ECCustomAttributes>"
                         "    <Source cardinality='(0,1)' polymorphic='True'>"
                         "        <Class class='GeometryPart' />"
                         "     </Source>"
                         "     <Target cardinality='(0,1)' polymorphic='True'>"
                         "         <Class class='Geometry' />"
                         "     </Target>"
                         "  </ECRelationshipClass>"
                         "</ECSchema>"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    ECInstanceKey geomKey1;
    ECInstanceKey geomKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.Geometry(Type) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Polygon", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey1));
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Solid", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey2));
    }

    ECInstanceKey partKey1;
    ECInstanceKey partKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.GeometryPart(Stream) VALUES(randomblob(4))"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey1));
    stmt.Reset();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey2));
    }
    {
    //Create relationships:
    //Geom-Part
    //1-1
    GetECDb().SaveChanges();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.PartHeldByGeometry(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, partKey1.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, geomKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, geomKey1.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();
    }

    GetECDb().SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(GetECDb(), "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey1.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, delGeomStmt.Step());
    delGeomStmt.Reset();
    delGeomStmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Geometry", geomKey1));
    ASSERT_TRUE(InstanceExists("ts.Geometry", geomKey2));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey1));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey2));
    ASSERT_FALSE(RelationshipExists("ts.PartHeldByGeometry", partKey1, geomKey1)) << "ECSQL DELETE deletes affected relationships";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbHoldingRelationshipStrengthTestFixture, OneToManyForward)
    {
    SetupECDb("ecdbrelationshipmappingrules_onetomanyandholding.ecdb",
              SchemaItem("1:N and holding",
                         "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                         "  <ECEntityClass typeName='Geometry' >"
                         "    <ECProperty propertyName='Type' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='GeometryPart' >"
                         "    <ECProperty propertyName='Stream' typeName='binary' />"
                         "  </ECEntityClass>"
                         "  <ECRelationshipClass typeName='GeometryHoldsParts' strength='holding' strengthDirection='Forward' modifier='Sealed'>"
                         "     <Source cardinality='(0,N)' polymorphic='True'>"
                         "         <Class class='Geometry' />"
                         "     </Source>"
                         "    <Target cardinality='(0,1)' polymorphic='True'>"
                         "        <Class class='GeometryPart' />"
                         "     </Target>"
                         "  </ECRelationshipClass>"
                         "</ECSchema>"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    ECInstanceKey geomKey1;
    ECInstanceKey geomKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.Geometry(Type) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Polygon", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey1));
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Solid", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey2));
    }

    ECInstanceKey partKey1;
    ECInstanceKey partKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.GeometryPart(Stream) VALUES(randomblob(4))"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey1));
    stmt.Reset();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey2));
    }

    {
    //Create relationships:
    //Geom-Part
    //1-1
    //2-1
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.GeometryHoldsParts(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, geomKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey1.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, partKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, partKey1.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, geomKey2.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey2.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, partKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, partKey1.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    GetECDb().SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(GetECDb(), "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey1.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, delGeomStmt.Step());
    delGeomStmt.Reset();
    delGeomStmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Geometry", geomKey1));
    ASSERT_TRUE(InstanceExists("ts.Geometry", geomKey2));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey1));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey2));
    ASSERT_FALSE(RelationshipExists("ts.GeometryHoldsParts", geomKey1, partKey1)) << "ECSQL DELETE deletes affected relationships";
    ASSERT_TRUE(RelationshipExists("ts.GeometryHoldsParts", geomKey2, partKey1));

    //delete Geom2
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey2.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, delGeomStmt.Step());

    ASSERT_FALSE(InstanceExists("ts.Geometry", geomKey2));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey1)) << "Part 1 is not held anymore, but will only be deleted by Purge";
    ASSERT_FALSE(RelationshipExists("ts.GeometryHoldsParts", geomKey2, partKey1));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbHoldingRelationshipStrengthTestFixture, OneToManyBackward)
    {
    SetupECDb("ecdbrelationshipmappingrules_onetomanyandholding.ecdb",
              SchemaItem("1:N and holding",
                         "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                         "  <ECEntityClass typeName='Geometry' >"
                         "    <ECProperty propertyName='Type' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='GeometryPart' >"
                         "    <ECProperty propertyName='Stream' typeName='binary' />"
                         "  </ECEntityClass>"
                         "  <ECRelationshipClass typeName='PartIsHeldByGeometry' strength='holding' strengthDirection='Backward' modifier='Sealed'>"
                         "    <Source cardinality='(0,1)' polymorphic='True'>"
                         "        <Class class='GeometryPart' />"
                         "     </Source>"
                         "     <Target cardinality='(0,N)' polymorphic='True'>"
                         "         <Class class='Geometry' />"
                         "     </Target>"
                         "  </ECRelationshipClass>"
                         "</ECSchema>"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    ECInstanceKey geomKey1;
    ECInstanceKey geomKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.Geometry(Type) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Polygon", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey1));
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Solid", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey2));
    }

    ECInstanceKey partKey1;
    ECInstanceKey partKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.GeometryPart(Stream) VALUES(randomblob(4))"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey1));
    stmt.Reset();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey2));
    }

    {
    //Create relationships:
    //Part-Geom
    //1-1
    //1-2
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.PartIsHeldByGeometry(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, partKey1.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, geomKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, geomKey1.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, partKey1.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, geomKey2.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, geomKey2.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    GetECDb().SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(GetECDb(), "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey1.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, delGeomStmt.Step());
    delGeomStmt.Reset();
    delGeomStmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Geometry", geomKey1));
    ASSERT_TRUE(InstanceExists("ts.Geometry", geomKey2));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey1));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey2));
    ASSERT_FALSE(RelationshipExists("ts.PartIsHeldByGeometry", partKey1, geomKey1)) << "ECSQL DELETE deletes affected relationships";
    ASSERT_TRUE(RelationshipExists("ts.PartIsHeldByGeometry", partKey1, geomKey2));

    //delete Geom2
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey2.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, delGeomStmt.Step());

    ASSERT_FALSE(InstanceExists("ts.Geometry", geomKey2));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey1)) << "Part 1 is not held anymore, but will only be deleted by Purge";
    ASSERT_FALSE(RelationshipExists("ts.PartIsHeldByGeometry", partKey1, geomKey2));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbHoldingRelationshipStrengthTestFixture, ManyToManyForward)
    {
    SetupECDb("ecdbrelationshipmappingrules_manytomanyandholding.ecdb",
              SchemaItem("N:N and holding",
                         "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                         "  <ECEntityClass typeName='Geometry' >"
                         "    <ECProperty propertyName='Type' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='GeometryPart' >"
                         "    <ECProperty propertyName='Stream' typeName='binary' />"
                         "  </ECEntityClass>"
                         "  <ECRelationshipClass typeName='GeometryHasParts' strength='holding' strengthDirection='Forward' modifier='Sealed'>"
                         "     <Source cardinality='(0,N)' polymorphic='True'>"
                         "         <Class class='Geometry' />"
                         "     </Source>"
                         "    <Target cardinality='(0,N)' polymorphic='True'>"
                         "        <Class class='GeometryPart' />"
                         "     </Target>"
                         "  </ECRelationshipClass>"
                         "</ECSchema>"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    ECInstanceKey geomKey1;
    ECInstanceKey geomKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.Geometry(Type) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Polygon", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey1));
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Solid", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey2));
    }

    ECInstanceKey partKey1;
    ECInstanceKey partKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.GeometryPart(Stream) VALUES(randomblob(4))"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey1));
    stmt.Reset();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey2));
    }

    {
    //Create relationships:
    //Geom-Part
    //1-1
    //1-2
    //2-2
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.GeometryHasParts(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, geomKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey1.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, partKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, partKey1.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, geomKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey1.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, partKey2.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, partKey2.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, geomKey2.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, geomKey2.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, partKey2.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, partKey2.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    GetECDb().SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(GetECDb(), "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey1.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, delGeomStmt.Step());

    ASSERT_FALSE(InstanceExists("ts.Geometry", geomKey1));
    ASSERT_TRUE(InstanceExists("ts.Geometry", geomKey2));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey1));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey2));
    ASSERT_FALSE(RelationshipExists("ts.GeometryHasParts", geomKey1, partKey1));
    ASSERT_FALSE(RelationshipExists("ts.GeometryHasParts", geomKey1, partKey2));
    ASSERT_TRUE(RelationshipExists("ts.GeometryHasParts", geomKey2, partKey2));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbHoldingRelationshipStrengthTestFixture, ManyToManyBackward)
    {
    SetupECDb("ecdbrelationshipmappingrules_manytomanyandholding.ecdb",
              SchemaItem("N:N and holding",
                         "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                         "  <ECEntityClass typeName='Geometry' >"
                         "    <ECProperty propertyName='Type' typeName='string' />"
                         "  </ECEntityClass>"
                         "  <ECEntityClass typeName='GeometryPart' >"
                         "    <ECProperty propertyName='Stream' typeName='binary' />"
                         "  </ECEntityClass>"
                         "  <ECRelationshipClass typeName='PartsHeldByGeometry' strength='holding' strengthDirection='Backward' modifier='Sealed'>"
                         "    <Source cardinality='(0,N)' polymorphic='True'>"
                         "        <Class class='GeometryPart' />"
                         "     </Source>"
                         "     <Target cardinality='(0,N)' polymorphic='True'>"
                         "         <Class class='Geometry' />"
                         "     </Target>"
                         "  </ECRelationshipClass>"
                         "</ECSchema>"));
    ASSERT_TRUE(GetECDb().IsDbOpen());

    ECInstanceKey geomKey1;
    ECInstanceKey geomKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.Geometry(Type) VALUES(?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Polygon", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey1));
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, "Solid", IECSqlBinder::MakeCopy::Yes));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(geomKey2));
    }

    ECInstanceKey partKey1;
    ECInstanceKey partKey2;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.GeometryPart(Stream) VALUES(randomblob(4))"));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey1));
    stmt.Reset();
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(partKey2));
    }

    {
    //Create relationships:
    //Geom-Part
    //1-1
    //1-2
    //2-2
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "INSERT INTO ts.PartsHeldByGeometry(SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId) VALUES(?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, partKey1.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, geomKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, geomKey1.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey2.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, partKey2.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, geomKey1.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, geomKey1.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, partKey2.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(2, partKey2.GetClassId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(3, geomKey2.GetInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(4, geomKey2.GetClassId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    GetECDb().SaveChanges();

    //Delete Geom1
    ECSqlStatement delGeomStmt;
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.Prepare(GetECDb(), "DELETE FROM ts.Geometry WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, delGeomStmt.BindId(1, geomKey1.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, delGeomStmt.Step());

    ASSERT_FALSE(InstanceExists("ts.Geometry", geomKey1));
    ASSERT_TRUE(InstanceExists("ts.Geometry", geomKey2));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey1));
    ASSERT_TRUE(InstanceExists("ts.GeometryPart", partKey2));
    ASSERT_FALSE(RelationshipExists("ts.PartsHeldByGeometry", partKey1, geomKey1));
    ASSERT_FALSE(RelationshipExists("ts.PartsHeldByGeometry", partKey2, geomKey1));
    ASSERT_TRUE(RelationshipExists("ts.PartsHeldByGeometry", partKey2, geomKey2));
    }

//=======================================================================================    
// @bsiclass                                   Muhammad Hassan                     05/15
//=======================================================================================    
struct RelationshipsAndSharedTablesTestFixture : DbMappingTestFixture
    {
    protected:
        static Utf8CP const SCHEMA_XML;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP const RelationshipsAndSharedTablesTestFixture::SCHEMA_XML =
"<?xml version='1.0' encoding='utf-8'?>"
"<ECSchema schemaName='test' nameSpacePrefix='t' version='1.0' description='Schema covers all the cases in which base class is OwnTable(Polymorphic)' displayLabel='Table Per Hierarchy' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
"<ECSchemaReference name='ECDbMap' version='02.00' prefix='ecdbmap' />"
"<ECEntityClass typeName='Base'>"
"<ECCustomAttributes>"
"        <ClassMap xmlns='ECDbMap.02.00'>"
"                <MapStrategy>TablePerHierarchy</MapStrategy>"
"</ClassMap>"
"</ECCustomAttributes>"
"<ECProperty propertyName='P0' typeName='string' />"
"</ECEntityClass>"
"<ECEntityClass typeName='ClassA' >"
"<BaseClass>Base</BaseClass>"
"<ECProperty propertyName='P1' typeName='string' />"
"</ECEntityClass>"
"<ECEntityClass typeName='ClassB' >"
"<BaseClass>ClassA</BaseClass>"
"<ECProperty propertyName='P2' typeName='string' />"
"</ECEntityClass>"
"<ECRelationshipClass typeName='BaseOwnsBase' strength='referencing' strengthDirection='forward' modifier='Abstract'>"
"<ECCustomAttributes>"
"        <ClassMap xmlns='ECDbMap.02.00'>"
"                <MapStrategy>TablePerHierarchy</MapStrategy>"
"</ClassMap>"
"</ECCustomAttributes>"
"<Source cardinality='(0,N)' polymorphic='True'>"
"<Class class='Base' />"
"</Source>"
"<Target cardinality='(0,N)' polymorphic='True'>"
"<Class class='Base' />"
"</Target>"
"</ECRelationshipClass>"
"<ECRelationshipClass typeName='BaseHasClassA' strength='referencing' strengthDirection='forward' modifier='Sealed'>"
"<BaseClass>BaseOwnsBase</BaseClass>"
"<Source cardinality='(0,1)' polymorphic='True'>"
"<Class class='Base' />"
"</Source>"
"<Target cardinality='(0,1)' polymorphic='True'>"
"<Class class='ClassA' />"
"</Target>"
"</ECRelationshipClass>"
"<ECRelationshipClass typeName='BaseHasClassB' strength='referencing' strengthDirection='forward' modifier='Sealed'>"
"<BaseClass>BaseOwnsBase</BaseClass>"
"<Source cardinality='(0,1)' polymorphic='True'>"
"<Class class='Base' />"
"</Source>"
"<Target cardinality='(0,1)' polymorphic='True'>"
"<Class class='ClassB' />"
"</Target>"
"</ECRelationshipClass>"
"</ECSchema>";

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipsAndSharedTablesTestFixture, UniqueIndexesSupportFor1to1Relationship)
    {
    SchemaItem testItem(SCHEMA_XML, true);
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "RelationshipsAndSharedTables.ecdb");
    ASSERT_FALSE(asserted);

    BeSQLite::Statement stmt;
    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.Prepare(ecdb, "SELECT Id from ec_Class where ec_Class.Name = 'BaseHasClassA'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, stmt.Step());
    ECClassId classId = stmt.GetValueId<ECClassId>(0);
    stmt.Finalize();

    //verify that entry in the ec_Index table exists for relationship table BaseHasClassA
    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.Prepare(ecdb, "SELECT Name, IsUnique from ec_Index where ClassId = ?"));
    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.BindId(1, classId));
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        {
        ASSERT_EQ(1, stmt.GetValueInt(1)) << "Index value for 1:1 Relationship is not Unique";
        Utf8String indexName = stmt.GetValueText(0);
        ASSERT_TRUE(indexName == "idx_ECRel_Source_Unique_t_BaseOwnsBase" || "idx_ECRel_Target_Unique_t_BaseOwnsBase");
        }
    stmt.Finalize();

    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.Prepare(ecdb, "SELECT Id from ec_Class where ec_Class.Name = 'BaseHasClassB'"));
    ASSERT_EQ(DbResult::BE_SQLITE_ROW, stmt.Step());
    classId = stmt.GetValueId<ECClassId>(0);
    stmt.Finalize();

    //verify that entry in ec_Index table also exists for relationship table BaseHasClassB
    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.Prepare(ecdb, "SELECT Name, IsUnique from ec_Index where ClassId = ?"));
    ASSERT_EQ(DbResult::BE_SQLITE_OK, stmt.BindId(1, classId));
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        {
        ASSERT_EQ(1, stmt.GetValueInt(1)) << "Index value for 1:1 Relationship is not Unique";
        Utf8String indexName = stmt.GetValueText(0);
        ASSERT_TRUE(indexName == "uix_unique_t_BaseHasClassB_Source" || "uix_unique_t_BaseHasClassB_Target");
        }
    stmt.Finalize();

    ecdb.CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Muhammad Hassan                     05/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipsAndSharedTablesTestFixture, InstanceDeletionFromPolymorphicRelationships)
    {
    SchemaItem testItem(SCHEMA_XML, true);
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "RelationshipsAndSharedTables.ecdb");
    ASSERT_FALSE(asserted);

    ASSERT_TRUE(ecdb.TableExists("t_BaseOwnsBase"));
    ASSERT_FALSE(ecdb.TableExists("t_BaseHasClassA"));
    ASSERT_FALSE(ecdb.TableExists("t_BaseHasClassB"));

    ECSchemaCP schema = ecdb.Schemas().GetSchema("test", true);
    ASSERT_TRUE(schema != nullptr) << "Couldn't locate test schema";

    ECClassCP baseClass = schema->GetClassCP("Base");
    ASSERT_TRUE(baseClass != nullptr) << "Couldn't locate class Base from schema";
    ECClassCP classA = schema->GetClassCP("ClassA");
    ASSERT_TRUE(classA != nullptr) << "Couldn't locate classA from Schema";
    ECClassCP classB = schema->GetClassCP("ClassB");
    ASSERT_TRUE(classB != nullptr) << "Couldn't locate classB from Schema";

    //Insert Instances for class Base
    ECN::StandaloneECInstancePtr baseInstance1 = baseClass->GetDefaultStandaloneEnabler()->CreateInstance();
    ECN::StandaloneECInstancePtr baseInstance2 = baseClass->GetDefaultStandaloneEnabler()->CreateInstance();

    baseInstance1->SetValue("P0", ECValue("string1"));
    baseInstance2->SetValue("P0", ECValue("string2"));

    ECInstanceInserter inserter(ecdb, *baseClass, nullptr);
    ASSERT_TRUE(inserter.IsValid());

    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(*baseInstance1, true));
    ASSERT_EQ(BE_SQLITE_OK, inserter.Insert(*baseInstance2, true));

    //Insert Instances for ClassA
    ECN::StandaloneECInstancePtr classAInstance1 = classA->GetDefaultStandaloneEnabler()->CreateInstance();
    ECN::StandaloneECInstancePtr classAInstance2 = classA->GetDefaultStandaloneEnabler()->CreateInstance();

    classAInstance1->SetValue("P1", ECValue("string1"));
    classAInstance2->SetValue("P1", ECValue("string2"));

    ECInstanceInserter classAinserter(ecdb, *classA, nullptr);
    ASSERT_TRUE(classAinserter.IsValid());

    ASSERT_EQ(BE_SQLITE_OK, classAinserter.Insert(*classAInstance1, true));
    ASSERT_EQ(BE_SQLITE_OK, classAinserter.Insert(*classAInstance2, true));

    //Insert Instances for ClassB
    ECN::StandaloneECInstancePtr classBInstance1 = classB->GetDefaultStandaloneEnabler()->CreateInstance();
    ECN::StandaloneECInstancePtr classBInstance2 = classB->GetDefaultStandaloneEnabler()->CreateInstance();

    classBInstance1->SetValue("P2", ECValue("string1"));
    classBInstance2->SetValue("P2", ECValue("string2"));

    ECInstanceInserter classBinserter(ecdb, *classB, nullptr);
    ASSERT_TRUE(classBinserter.IsValid());

    ASSERT_EQ(BE_SQLITE_OK, classBinserter.Insert(*classBInstance1, true));
    ASSERT_EQ(BE_SQLITE_OK, classBinserter.Insert(*classBInstance2, true));

    //Get Relationship Classes
    ECRelationshipClassCP baseHasClassAClass = schema->GetClassCP("BaseHasClassA")->GetRelationshipClassCP();
    ASSERT_TRUE(baseHasClassAClass != nullptr);
    ECRelationshipClassCP baseHasClassBClass = schema->GetClassCP("BaseHasClassB")->GetRelationshipClassCP();
    ASSERT_TRUE(baseHasClassBClass != nullptr);

    {//Insert Instances for Relationship TPHhasClassA
    ECN::StandaloneECRelationshipInstancePtr relationshipInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*baseHasClassAClass)->CreateRelationshipInstance();
    ECInstanceInserter relationshipinserter(ecdb, *baseHasClassAClass, nullptr);
    ASSERT_TRUE(relationshipinserter.IsValid());

    {//Inserting 1st Instance
    relationshipInstance->SetSource(baseInstance1.get());
    relationshipInstance->SetTarget(classAInstance1.get());
    relationshipInstance->SetInstanceId("source->target");
    ASSERT_EQ(BE_SQLITE_OK, relationshipinserter.Insert(*relationshipInstance));
    }
    {//Inserting 2nd Instance
    relationshipInstance->SetSource(baseInstance2.get());
    relationshipInstance->SetTarget(classAInstance2.get());
    relationshipInstance->SetInstanceId("source->target");
    ASSERT_EQ(BE_SQLITE_OK, relationshipinserter.Insert(*relationshipInstance));
    }
    }

    {//Insert Instances for Relationship TPHhasClassB
    ECN::StandaloneECRelationshipInstancePtr relationshipInstance = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*baseHasClassBClass)->CreateRelationshipInstance();
    ECInstanceInserter relationshipinserter(ecdb, *baseHasClassBClass, nullptr);
    ASSERT_TRUE(relationshipinserter.IsValid());

    {//Inserting 1st Instance
    relationshipInstance->SetSource(baseInstance1.get());
    relationshipInstance->SetTarget(classBInstance1.get());
    relationshipInstance->SetInstanceId("source->target");
    ASSERT_EQ(BE_SQLITE_OK, relationshipinserter.Insert(*relationshipInstance));
    }
    {//Inserting 2nd Instance
    relationshipInstance->SetSource(baseInstance2.get());
    relationshipInstance->SetTarget(classBInstance2.get());
    relationshipInstance->SetInstanceId("source->target");
    ASSERT_EQ(BE_SQLITE_OK, relationshipinserter.Insert(*relationshipInstance));
    }
    }
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT COUNT(*) FROM t.Base"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(6, stmt.GetValueInt(0));
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(4, stmt.GetValueInt(0));
    stmt.Finalize();

    //Deletes the instances of BaseOwnsBase class..
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "DELETE FROM ONLY t.BaseOwnsBase"));
    ASSERT_TRUE(BE_SQLITE_DONE == stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(4, stmt.GetValueInt(0));
    stmt.Finalize();

    //Deletes the instances of BaseHasClassA class..
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "DELETE FROM ONLY t.BaseHasClassA"));
    ASSERT_TRUE(BE_SQLITE_DONE == stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(2, stmt.GetValueInt(0));
    stmt.Finalize();

    //Deletes the instances of BaseHasClassB class..
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "DELETE FROM ONLY t.BaseHasClassB"));
    ASSERT_TRUE(BE_SQLITE_DONE == stmt.Step());
    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT COUNT(*) FROM t.BaseOwnsBase"));
    ASSERT_TRUE(BE_SQLITE_ROW == stmt.Step());
    EXPECT_EQ(0, stmt.GetValueInt(0));
    stmt.Finalize();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Muhammad Hassan                  07/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipsAndSharedTablesTestFixture, RetrieveConstraintClassInstanceBeforeAfterInsertingRelationshipInstance)
    {
    SchemaItem testItem(SCHEMA_XML, true);
    ECDb ecdb;
    bool asserted = false;
    AssertSchemaImport(ecdb, asserted, testItem, "RelationshipsAndSharedTables.ecdb");
    ASSERT_FALSE(asserted);


    ASSERT_TRUE(ecdb.TableExists("t_BaseOwnsBase"));
    ASSERT_FALSE(ecdb.TableExists("t_BaseHasClassA"));
    ASSERT_FALSE(ecdb.TableExists("t_BaseHasClassB"));

    ECSqlStatement insertStatement;
    ECInstanceKey TPHKey1;
    ECInstanceKey TPHKey2;
    ASSERT_EQ(ECSqlStatus::Success, insertStatement.Prepare(ecdb, "INSERT INTO t.Base (P0) VALUES ('string1')"));
    ASSERT_EQ(BE_SQLITE_DONE, insertStatement.Step(TPHKey1));
    ASSERT_TRUE(TPHKey1.IsValid());
    insertStatement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, insertStatement.Prepare(ecdb, "INSERT INTO t.Base (P0) VALUES ('string2')"));
    ASSERT_EQ(BE_SQLITE_DONE, insertStatement.Step(TPHKey2));
    ASSERT_TRUE(TPHKey2.IsValid());
    insertStatement.Finalize();

    ECInstanceKey classAKey1;
    ECInstanceKey classAKey2;
    ASSERT_EQ(ECSqlStatus::Success, insertStatement.Prepare(ecdb, "INSERT INTO t.ClassA (P1) VALUES ('string1')"));
    ASSERT_EQ(BE_SQLITE_DONE, insertStatement.Step(classAKey1));
    ASSERT_TRUE(classAKey1.IsValid());
    insertStatement.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, insertStatement.Prepare(ecdb, "INSERT INTO t.ClassA (P1) VALUES ('string2')"));
    ASSERT_EQ(BE_SQLITE_DONE, insertStatement.Step(classAKey2));
    ASSERT_TRUE(classAKey2.IsValid());
    insertStatement.Finalize();

    //retrieve ECInstance from Db before inserting Relationship Instance, based on ECInstanceId, verify ECInstance is valid
    ECSqlStatement selectStmt;
    ASSERT_EQ(ECSqlStatus::Success, selectStmt.Prepare(ecdb, "SELECT * FROM t.Base WHERE ECInstanceId = ?"));
    selectStmt.BindId(1, TPHKey1.GetInstanceId());
    ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
    ECInstanceECSqlSelectAdapter TPHadapter(selectStmt);
    IECInstancePtr readInstance = TPHadapter.GetInstance();
    ASSERT_TRUE(readInstance.IsValid());
    selectStmt.Finalize();

    ECSqlStatement relationStmt;
    ASSERT_EQ(relationStmt.Prepare(ecdb, "INSERT INTO t.BaseHasClassA (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)"), ECSqlStatus::Success);
    relationStmt.BindId(1, TPHKey1.GetInstanceId());
    relationStmt.BindId(2, TPHKey1.GetClassId());
    relationStmt.BindId(3, classAKey1.GetInstanceId());
    relationStmt.BindId(4, classAKey1.GetClassId());
    ASSERT_EQ(BE_SQLITE_DONE, relationStmt.Step());
    relationStmt.Finalize();

    //try to insert Duplicate relationship step() should return error
    ASSERT_EQ(relationStmt.Prepare(ecdb, "INSERT INTO t.BaseHasClassA (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES (?, ?, ?, ?)"), ECSqlStatus::Success);
    relationStmt.BindId(1, TPHKey1.GetInstanceId());
    relationStmt.BindId(2, TPHKey1.GetClassId());
    relationStmt.BindId(3, classAKey1.GetInstanceId());
    relationStmt.BindId(4, classAKey1.GetClassId());
    ASSERT_TRUE((BE_SQLITE_CONSTRAINT_BASE & relationStmt.Step()) == BE_SQLITE_CONSTRAINT_BASE);
    relationStmt.Finalize();

    //retrieve ECInstance from Db After Inserting Relationship Instance, based on ECInstanceId, verify ECInstance is valid
    ASSERT_EQ(ECSqlStatus::Success, selectStmt.Prepare(ecdb, "SELECT * FROM t.ClassA WHERE ECInstanceId = ?"));
    selectStmt.BindId(1, classAKey1.GetInstanceId());
    ASSERT_EQ(BE_SQLITE_ROW, selectStmt.Step());
    ECInstanceECSqlSelectAdapter ClassAadapter(selectStmt);
    readInstance = ClassAadapter.GetInstance();
    ASSERT_TRUE(readInstance.IsValid());
    selectStmt.Finalize();

    ecdb.CloseDb();
    }

//=======================================================================================    
// @bsiclass                                   Muhammad Hassan                     05/15
//=======================================================================================    
struct ReferentialIntegrityTestFixture : DbMappingTestFixture
    {
    private:
        void VerifyRelationshipInsertionIntegrity(ECDbCR ecdb, Utf8CP relationshipClass, std::vector<ECInstanceKey> const& sourceKeys, std::vector<ECInstanceKey>const& targetKeys, std::vector<DbResult> const& expected, size_t& rowInserted) const;
        size_t GetRelationshipInstanceCount(ECDbCR ecdb, Utf8CP relationshipClass) const;

    protected:
        void ExecuteRelationshipInsertionIntegrityTest(ECDbR ecdb, bool allowDuplicateRelationships, bool allowForeignKeyConstraint, bool schemaImportExpectedToSucceed) const;
    };

/*---------------------------------------------------------------------------------**//**
                                                                                      * @bsimethod                              Muhammad Hassan                         04/15
                                                                                      +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ReferentialIntegrityTestFixture, ForeignKeyConstraint_EnforceReferentialIntegrity)
    {
    ECDbR ecdb = SetupECDb("ForeignKeyConstraint_EnforceReferentialIntegrity.ecdb");
    ExecuteRelationshipInsertionIntegrityTest(ecdb, false, true, true);
    //when AllowDuplicate is turned of, OneFooHasManyGoo will also be mapped as endtable therefore ReferentialIntegrityCheck will be performed for it, so there will be two rows in the ForeignKey table
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasOneGoo"));
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasManyGoo"));
    }

/*---------------------------------------------------------------------------------**//**
                                                                                      * @bsimethod                              Muhammad Hassan                         04/15
                                                                                      +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ReferentialIntegrityTestFixture, ForeignKeyConstraint_EnforceReferentialIntegrityCheck_AllowDuplicateRelation)
    {
    ECDbR ecdb = SetupECDb("ForeignKeyConstraint_EnforceReferentialIntegrityCheck_AllowDuplicateRelation.ecdb");
    ExecuteRelationshipInsertionIntegrityTest(ecdb, true, true, true);
    //when AllowDuplicate is turned on, OneFooHasManyGoo will also be mapped as endtable therefore there will be only one row in the ForeignKey table
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasOneGoo"));
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasManyGoo"));
    }

/*---------------------------------------------------------------------------------**//**
                                                                                      * @bsimethod                                   Affan.Khan                         02/15
                                                                                      +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ReferentialIntegrityTestFixture, DoNotAllowDuplicateRelationships)
    {
    ECDbR ecdb = SetupECDb("RelationshipCardinalityTest.ecdb");
    ExecuteRelationshipInsertionIntegrityTest(ecdb, false, true, true);
    ASSERT_TRUE(ecdb.TableExists("ts_Foo"));
    ASSERT_TRUE(ecdb.TableExists("ts_Goo"));
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasOneGoo"));
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasManyGoo"));
    ASSERT_TRUE(ecdb.TableExists("ts_ManyFooHasManyGoo"));
    }

/*---------------------------------------------------------------------------------**//**
                                                                                      * @bsimethod                                   Affan.Khan                         02/15
                                                                                      +---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ReferentialIntegrityTestFixture, AllowDuplicateRelationships)
    {
    ECDbR ecdb = SetupECDb("RelationshipCardinalityTest_AllowDuplicateRelationships.ecdb");
    ExecuteRelationshipInsertionIntegrityTest(ecdb, true, true, true);
    ASSERT_TRUE(ecdb.TableExists("ts_Foo"));
    ASSERT_TRUE(ecdb.TableExists("ts_Goo"));
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasOneGoo"));
    ASSERT_FALSE(ecdb.TableExists("ts_OneFooHasManyGoo"));
    ASSERT_TRUE(ecdb.TableExists("ts_ManyFooHasManyGoo"));
    }

/*---------------------------------------------------------------------------------**//**
                                                                                      * @bsimethod                                   Affan.Khan                         02/15
                                                                                      +---------------+---------------+---------------+---------------+---------------+------*/
void ReferentialIntegrityTestFixture::VerifyRelationshipInsertionIntegrity(ECDbCR ecdb, Utf8CP relationshipClass, std::vector<ECInstanceKey> const& sourceKeys, std::vector<ECInstanceKey>const& targetKeys, std::vector<DbResult> const& expected, size_t& rowInserted) const
    {
    ECSqlStatement stmt;
    auto sql = SqlPrintfString("INSERT INTO %s (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(:sECInstanceId,:sECClassId,:tECInstanceId,:tECClassId)", relationshipClass);
    ASSERT_EQ(stmt.Prepare(ecdb, sql.GetUtf8CP()), ECSqlStatus::Success);
    ASSERT_EQ(expected.size(), sourceKeys.size() * targetKeys.size());

    const int sECInstanceId = stmt.GetParameterIndex("sECInstanceId");
    const int sECClassId = stmt.GetParameterIndex("sECClassId");
    const int tECInstanceId = stmt.GetParameterIndex("tECInstanceId");
    const int tECClassId = stmt.GetParameterIndex("tECClassId");

    int n = 0;
    for (auto& fooKey : sourceKeys)
        {
        for (auto& gooKey : targetKeys)
            {
            stmt.Reset();
            ASSERT_EQ(ECSqlStatus::Success, stmt.ClearBindings());
            stmt.BindId(sECInstanceId, fooKey.GetInstanceId());
            stmt.BindId(sECClassId, fooKey.GetClassId());
            stmt.BindId(tECInstanceId, gooKey.GetInstanceId());
            stmt.BindId(tECClassId, gooKey.GetClassId());
            if (expected[n] != BE_SQLITE_DONE)
                ASSERT_NE(BE_SQLITE_DONE, stmt.Step());
            else
                {
                ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
                rowInserted++;
                }
            n = n + 1;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
                                                                                      * @bsimethod                                   Affan.Khan                         02/15
                                                                                      +---------------+---------------+---------------+---------------+---------------+------*/
size_t ReferentialIntegrityTestFixture::GetRelationshipInstanceCount(ECDbCR ecdb, Utf8CP relationshipClass) const
    {
    ECSqlStatement stmt;
    auto sql = SqlPrintfString("SELECT COUNT(*) FROM ONLY ts.Foo JOIN ts.Goo USING %s", relationshipClass);
    if (stmt.Prepare(ecdb, sql.GetUtf8CP()) == ECSqlStatus::Success)
        {
        if (stmt.Step() == BE_SQLITE_ROW)
            return static_cast<size_t>(stmt.GetValueInt(0));
        }

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
                                                                                      * @bsimethod                                   Affan.Khan                         02/15
                                                                                      +---------------+---------------+---------------+---------------+---------------+------*/
void ReferentialIntegrityTestFixture::ExecuteRelationshipInsertionIntegrityTest(ECDbR ecdb, bool allowDuplicateRelationships, bool allowForeignKeyConstraint, bool schemaImportExpectedToSucceed) const
    {
    ECSchemaPtr testSchema;
    ECEntityClassP foo = nullptr, goo = nullptr;
    ECRelationshipClassP oneFooHasOneGoo = nullptr, oneFooHasManyGoo = nullptr, manyFooHasManyGoo = nullptr;
    PrimitiveECPropertyP prim;

    auto readContext = ECSchemaReadContext::CreateContext();
    readContext->AddSchemaLocater(ecdb.GetSchemaLocater());
    auto ecdbmapKey = SchemaKey("ECDbMap", 2, 0);
    auto ecdbmapSchema = readContext->LocateSchema(ecdbmapKey, SchemaMatchType::LatestWriteCompatible);
    ASSERT_TRUE(ecdbmapSchema.IsValid());

    ECSchema::CreateSchema(testSchema, "TestSchema", "ts", 1, 0, 0);
    ASSERT_TRUE(testSchema.IsValid());

    testSchema->AddReferencedSchema(*ecdbmapSchema);

    testSchema->CreateEntityClass(foo, "Foo");
    testSchema->CreateEntityClass(goo, "Goo");

    testSchema->CreateRelationshipClass(oneFooHasOneGoo, "OneFooHasOneGoo");
    oneFooHasOneGoo->SetClassModifier(ECClassModifier::Sealed);
    testSchema->CreateRelationshipClass(oneFooHasManyGoo, "OneFooHasManyGoo");
    oneFooHasManyGoo->SetClassModifier(ECClassModifier::Sealed);
    testSchema->CreateRelationshipClass(manyFooHasManyGoo, "ManyFooHasManyGoo");
    manyFooHasManyGoo->SetClassModifier(ECClassModifier::Sealed);
    ASSERT_TRUE(foo != nullptr);
    ASSERT_TRUE(foo != nullptr);
    ASSERT_TRUE(oneFooHasOneGoo != nullptr);
    ASSERT_TRUE(oneFooHasManyGoo != nullptr);
    ASSERT_TRUE(manyFooHasManyGoo != nullptr);

    prim = nullptr;
    foo->CreatePrimitiveProperty(prim, "fooProp");
    prim->SetType(PrimitiveType::PRIMITIVETYPE_String);
    ASSERT_TRUE(prim != nullptr);

    prim = nullptr;
    goo->CreatePrimitiveProperty(prim, "gooProp");
    prim->SetType(PrimitiveType::PRIMITIVETYPE_String);
    ASSERT_TRUE(prim != nullptr);

    oneFooHasOneGoo->GetSource().AddClass(*foo);
    oneFooHasOneGoo->GetSource().SetRoleLabel("OneFooHasOneGoo");
    oneFooHasOneGoo->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    oneFooHasOneGoo->GetTarget().AddClass(*goo);
    oneFooHasOneGoo->GetTarget().SetRoleLabel("OneFooHasOneGoo (Reversed)");
    oneFooHasOneGoo->GetTarget().SetMultiplicity(RelationshipMultiplicity::ZeroOne());

    oneFooHasManyGoo->GetSource().AddClass(*foo);
    oneFooHasManyGoo->GetSource().SetRoleLabel("OneFooHasManyGoo");
    oneFooHasManyGoo->GetSource().SetMultiplicity(RelationshipMultiplicity::ZeroOne());
    oneFooHasManyGoo->GetTarget().AddClass(*goo);
    oneFooHasManyGoo->GetTarget().SetRoleLabel("OneFooHasManyGoo (Reversed)");
    oneFooHasManyGoo->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneMany());

    manyFooHasManyGoo->GetSource().AddClass(*foo);
    manyFooHasManyGoo->GetSource().SetRoleLabel("ManyFooHasManyGoo");
    manyFooHasManyGoo->GetSource().SetMultiplicity(RelationshipMultiplicity::OneMany());
    manyFooHasManyGoo->GetTarget().AddClass(*goo);
    manyFooHasManyGoo->GetTarget().SetRoleLabel("ManyFooHasManyGoo (Reversed)");
    manyFooHasManyGoo->GetTarget().SetMultiplicity(RelationshipMultiplicity::OneMany());
    BackDoor::ECObjects::ECSchemaReadContext::AddSchema(*readContext, *testSchema);

    if (allowDuplicateRelationships)
        {
        auto caInstClass = ecdbmapSchema->GetClassCP("LinkTableRelationshipMap");
        ASSERT_TRUE(caInstClass != nullptr);
        auto caInst = caInstClass->GetDefaultStandaloneEnabler()->CreateInstance();
        ASSERT_TRUE(caInst != nullptr);
        ASSERT_TRUE(caInst->SetValue("AllowDuplicateRelationships", ECValue(true)) == ECObjectsStatus::Success);
        ASSERT_TRUE(manyFooHasManyGoo->SetCustomAttribute(*caInst) == ECObjectsStatus::Success);
        }

    if (allowForeignKeyConstraint)
        {
        auto fkMapClass = ecdbmapSchema->GetClassCP("ForeignKeyConstraint");
        ASSERT_TRUE(fkMapClass != nullptr);
        auto caInst = fkMapClass->GetDefaultStandaloneEnabler()->CreateInstance();
        ASSERT_TRUE(caInst != nullptr);
        ASSERT_TRUE(oneFooHasOneGoo->SetCustomAttribute(*caInst) == ECObjectsStatus::Success);
        ASSERT_TRUE(oneFooHasManyGoo->SetCustomAttribute(*caInst) == ECObjectsStatus::Success);
        }

    if (schemaImportExpectedToSucceed)
        ASSERT_EQ(SUCCESS, ecdb.Schemas().ImportSchemas(readContext->GetCache().GetSchemas()));
    else
        {
        ASSERT_EQ(ERROR, ecdb.Schemas().ImportSchemas(readContext->GetCache().GetSchemas()));
        return;
        }

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;

    ECSqlStatement fooStmt;
    ASSERT_EQ(fooStmt.Prepare(ecdb, "INSERT INTO ts.Foo(fooProp) VALUES(?)"), ECSqlStatus::Success);
    for (auto i = 0; i < maxFooInstances; i++)
        {
        ECInstanceKey out;
        ASSERT_EQ(fooStmt.Reset(), ECSqlStatus::Success);
        ASSERT_EQ(fooStmt.ClearBindings(), ECSqlStatus::Success);
        ASSERT_EQ(fooStmt.BindText(1, SqlPrintfString("foo_%d", i), IECSqlBinder::MakeCopy::Yes), ECSqlStatus::Success);
        ASSERT_EQ(fooStmt.Step(out), BE_SQLITE_DONE);
        fooKeys.push_back(out);
        }

    ECSqlStatement gooStmt;
    ASSERT_EQ(gooStmt.Prepare(ecdb, "INSERT INTO ts.Goo(gooProp) VALUES(?)"), ECSqlStatus::Success);
    for (auto i = 0; i < maxGooInstances; i++)
        {
        ECInstanceKey out;
        ASSERT_EQ(gooStmt.Reset(), ECSqlStatus::Success);
        ASSERT_EQ(gooStmt.ClearBindings(), ECSqlStatus::Success);
        ASSERT_EQ(gooStmt.BindText(1, SqlPrintfString("goo_%d", i), IECSqlBinder::MakeCopy::Yes), ECSqlStatus::Success);
        ASSERT_EQ(gooStmt.Step(out), BE_SQLITE_DONE);
        gooKeys.push_back(out);
        }

    //Compute what are the right valid permutation
    std::vector<DbResult> oneFooHasOneGooResult;
    std::vector<DbResult> oneFooHasManyGooResult;
    std::vector<DbResult> manyFooHasManyGooResult;
    std::vector<DbResult> reinsertResultError;
    std::vector<DbResult> reinsertResultDone;
    for (auto f = 0; f < maxFooInstances; f++)
        {
        for (auto g = 0; g < maxGooInstances; g++)
            {
            //1:1 is not effected with AllowDuplicateRelationships
            if (f == g)
                oneFooHasOneGooResult.push_back(BE_SQLITE_DONE);
            else
                oneFooHasOneGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);

            //1:N is effected with AllowDuplicateRelationships
            if (f == 0)
                oneFooHasManyGooResult.push_back(BE_SQLITE_DONE);
            else
                oneFooHasManyGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);

            manyFooHasManyGooResult.push_back(BE_SQLITE_DONE);
            reinsertResultError.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);
            reinsertResultDone.push_back(BE_SQLITE_DONE);
            }
        }
    //1:1--------------------------------
    size_t count_OneFooHasOneGoo = 0;
    VerifyRelationshipInsertionIntegrity(ecdb, "ts.OneFooHasOneGoo", fooKeys, gooKeys, oneFooHasOneGooResult, count_OneFooHasOneGoo);
    VerifyRelationshipInsertionIntegrity(ecdb, "ts.OneFooHasOneGoo", fooKeys, gooKeys, reinsertResultError, count_OneFooHasOneGoo);

    MapStrategyInfo mapStrategy;
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, oneFooHasOneGoo->GetId()));
    ASSERT_EQ((int) MapStrategyInfo::Strategy::ForeignKeyRelationshipInTargetTable, (int) mapStrategy.m_strategy);
    ASSERT_EQ(count_OneFooHasOneGoo, GetRelationshipInstanceCount(ecdb, "ts.OneFooHasOneGoo"));

    //1:N--------------------------------
    size_t count_OneFooHasManyGoo = 0;
    VerifyRelationshipInsertionIntegrity(ecdb, "ts.OneFooHasManyGoo", fooKeys, gooKeys, oneFooHasManyGooResult, count_OneFooHasManyGoo);

    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, oneFooHasManyGoo->GetId()));
    ASSERT_EQ((int) MapStrategyInfo::Strategy::ForeignKeyRelationshipInTargetTable, (int) mapStrategy.m_strategy);
    ASSERT_EQ(count_OneFooHasManyGoo, GetRelationshipInstanceCount(ecdb, "ts.OneFooHasManyGoo"));

    //N:N--------------------------------
    size_t count_ManyFooHasManyGoo = 0;
    VerifyRelationshipInsertionIntegrity(ecdb, "ts.ManyFooHasManyGoo", fooKeys, gooKeys, manyFooHasManyGooResult, count_ManyFooHasManyGoo);
    if (allowDuplicateRelationships)
        VerifyRelationshipInsertionIntegrity(ecdb, "ts.ManyFooHasManyGoo", fooKeys, gooKeys, reinsertResultDone, count_ManyFooHasManyGoo);
    else
        VerifyRelationshipInsertionIntegrity(ecdb, "ts.ManyFooHasManyGoo", fooKeys, gooKeys, reinsertResultError, count_ManyFooHasManyGoo);

    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, ecdb, manyFooHasManyGoo->GetId()));

    ASSERT_EQ((int) MapStrategyInfo::Strategy::OwnTable, (int) mapStrategy.m_strategy);
    ASSERT_TRUE(mapStrategy.m_tphInfo.IsUnset());
    ASSERT_EQ(count_ManyFooHasManyGoo, GetRelationshipInstanceCount(ecdb, "ts.ManyFooHasManyGoo"));
    }



//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  10/2014
//+---------------+---------------+---------------+---------------+---------------+------
struct RelationshipStrengthTestFixture : ECDbTestFixture
    {
    protected:
    //---------------------------------------------------------------------------------------
    //                                               Muhammad Hassan                  10/2014
    //+---------------+---------------+---------------+---------------+---------------+------
    ECInstanceKey InsertPerson(Utf8CP firstName, Utf8CP lastName)
        {
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO RelationshipStrengthTest.Person(FirstName,LastName) VALUES('%s','%s')", firstName, lastName);
        ECSqlStatement stmt;
        if (stmt.Prepare(GetECDb(), ecsql.c_str()) != ECSqlStatus::Success)
            return ECInstanceKey();

        ECInstanceKey key;
        stmt.Step(key);
        return key;
        }

    //---------------------------------------------------------------------------------------
    //                                               Muhammad Hassan                  10/2014
    //+---------------+---------------+---------------+---------------+---------------+------
    ECInstanceKey InsertRelationship(Utf8CP relationshipClassECSqlName, ECInstanceKey const& source, ECInstanceKey const& target)
        {
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO %s(SourceECInstanceId, TargetECInstanceId) VALUES(%s,%s)", relationshipClassECSqlName, source.GetInstanceId().ToString().c_str(),
                      target.GetInstanceId().ToString().c_str());
        ECSqlStatement stmt;
        if (stmt.Prepare(GetECDb(), ecsql.c_str()) != ECSqlStatus::Success)
            return ECInstanceKey();

        ECInstanceKey key;
        stmt.Step(key);
        return key;
        }

    //---------------------------------------------------------------------------------------
    //                                               Muhammad Hassan                  10/2014
    //+---------------+---------------+---------------+---------------+---------------+------
    BentleyStatus DeleteInstance(ECInstanceKey const& key)
        {
        ECClassCP ecClass = GetECDb().Schemas().GetClass(key.GetClassId());
        if (ecClass == nullptr)
            return ERROR;

        Utf8String ecsql;
        ecsql.Sprintf("DELETE FROM %s WHERE ECInstanceId=%s", ecClass->GetECSqlName().c_str(), key.GetInstanceId().ToString().c_str());
        ECSqlStatement stmt;
        if (stmt.Prepare(GetECDb(), ecsql.c_str()) != ECSqlStatus::Success)
            return ERROR;

        return BE_SQLITE_DONE == stmt.Step() ? SUCCESS : ERROR;
        }

    //---------------------------------------------------------------------------------------
    //                                               Muhammad Hassan                  10/2014
    //+---------------+---------------+---------------+---------------+---------------+------
    bool HasInstance(ECInstanceKey const& key)
        {
        ECClassCP ecClass = GetECDb().Schemas().GetClass(key.GetClassId());
        if (ecClass == nullptr)
            return false;

        Utf8String ecsql;
        ecsql.Sprintf("SELECT NULL FROM ONLY %s WHERE ECInstanceId=%s",
                      ecClass->GetECSqlName().c_str(), key.GetInstanceId().ToString().c_str());

        ECSqlStatement statement;
        if (ECSqlStatus::Success != statement.Prepare(GetECDb(), ecsql.c_str()))
            return false;

        return statement.Step() == BE_SQLITE_ROW;
        }
    };



//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  10/2014
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipStrengthTestFixture, BackwardEmbedding)
    {
    ECDbR ecdb = SetupECDb("BackwardRelationshipStrengthTest.ecdb", BeFileName(L"RelationshipStrengthTest.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());
    /*
    *                                           SingleParent
    *                                                 |
    *                                                 | ChildrenHasSingleParent (Backward EMBEDDING)
    *         ________________________________________|______________________________________
    *        |                                        |                                      |
    *      Child1                                   Child2                                 Child3
    */
    ECInstanceKey child1 = InsertPerson("First", "Child");
    ECInstanceKey child2 = InsertPerson("Second", "Child");
    ECInstanceKey child3 = InsertPerson("Third", "Child");
    ECInstanceKey singleParent = InsertPerson("Only", "singleParent");

    //Backward Embedding relationship (SingleParent -> Child1, Child2)
    ECInstanceKey child1HasSingleParent = InsertRelationship("RelationshipStrengthTest.ChildrenHasSingleParent", child1, singleParent);
    ECInstanceKey child2HasSingleParent = InsertRelationship("RelationshipStrengthTest.ChildrenHasSingleParent", child2, singleParent);
    ECInstanceKey child3HasSingleParent = InsertRelationship("RelationshipStrengthTest.ChildrenHasSingleParent", child3, singleParent);

    /*
    * Test 1: Delete Child1
    * Validate child1HasSingleParent,  child1 have been deleted
    * Validate singleParent, child2HasSingleParent, child3HasSingleParent, child2, child3 are still there
    */
    DeleteInstance(child1);
    ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(child1));
    ASSERT_FALSE(HasInstance(child1HasSingleParent));

    ASSERT_TRUE(HasInstance(singleParent));
    ASSERT_TRUE(HasInstance(child2HasSingleParent));
    ASSERT_TRUE(HasInstance(child3HasSingleParent));
    ASSERT_TRUE(HasInstance(child2));
    ASSERT_TRUE(HasInstance(child3));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RelationshipStrengthTestFixture, RelationshipTest)
    {
    ECDbR ecdb = SetupECDb("RelationshipStrengthTest.ecdb", BeFileName(L"RelationshipStrengthTest.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    /*
     *          Create the following relationship hierarchy
     *
     * GrandParent1  <- ParentHasSpouse (REFERENCING) -> GrandParent2
     *     |__________________________________________________|
     *                             |
     *                             | ManyParentsHaveChildren (REFERENCING)
     *                             |
     *                         SingleParent
     *                             |
     *                             | SingleParentHasChildren (EMBEDDING)
     *      _______________________|__________________________
     *     |                                                  |
     *   Child1                                             Child2
     *
     */

    ECInstanceKey grandParent1 = InsertPerson("First", "GrandParent");
    ECInstanceKey grandParent2 = InsertPerson("Second", "GrandParent");
    ECInstanceKey singleParent = InsertPerson("Only", "SingleParent");
    ECInstanceKey child1 = InsertPerson("First", "Child");
    ECInstanceKey child2 = InsertPerson("Second", "Child");

    // Referencing relationship (GrandParent1, GrandParent2 -> SingleParent)
    ECInstanceKey grandParent1HasSingleParent = InsertRelationship("RelationshipStrengthTest.ManyParentsHaveChildren", grandParent1, singleParent);
    ECInstanceKey grandParent2HasSingleParent = InsertRelationship("RelationshipStrengthTest.ManyParentsHaveChildren", grandParent2, singleParent);

    // Embedding relationship (SingleParent -> Child1, Child2)
    ECInstanceKey singleParentHasChild1 = InsertRelationship("RelationshipStrengthTest.SingleParentHasChildren", singleParent, child1);
    ECInstanceKey singleParentHasChild2 = InsertRelationship("RelationshipStrengthTest.SingleParentHasChildren", singleParent, child2);

    // Referencing relationship (GrandParent1 <-> GrandParent2)
    ECInstanceKey grandParent1HasSpouse = InsertRelationship("RelationshipStrengthTest.ParentHasSpouse", grandParent1, grandParent2);
    ECInstanceKey grandParent2HasSpouse = InsertRelationship("RelationshipStrengthTest.ParentHasSpouse", grandParent2, grandParent1);

    ecdb.SaveChanges();

    //Verify instances before deletion
    ASSERT_TRUE(HasInstance(grandParent1HasSpouse));
    ASSERT_TRUE(HasInstance(grandParent2HasSpouse));

    /*
    * Test 1: Delete GrandParent1
    * Validate grandParent1HasSpouse, grandParent2HasSpouse, grandParent1HasSingleParent have been deleted (orphaned relationships)
    * Validate singleParent is still around (referencing relationship with one parent remaining)
    */
    DeleteInstance(grandParent1);
    ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(grandParent1));
    ASSERT_FALSE(HasInstance(grandParent1HasSpouse));
    ASSERT_FALSE(HasInstance(grandParent2HasSpouse));
    ASSERT_FALSE(HasInstance(grandParent1HasSingleParent));

    ASSERT_TRUE(HasInstance(singleParent));

    /*
    * Test 2: Delete GrandParent2
    * Validate grandParent2HasSingleParent has been deleted (orphaned relationship), *Validate singeParent has been deleted (held instance with no parents remaining)
    */
    DeleteInstance(grandParent2);
    ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(grandParent2));
    ASSERT_FALSE(HasInstance(grandParent2HasSingleParent));

    ASSERT_TRUE(HasInstance(singleParent));
    ASSERT_TRUE(HasInstance(singleParentHasChild1));
    ASSERT_TRUE(HasInstance(singleParentHasChild2));
    ASSERT_TRUE(HasInstance(child1));
    ASSERT_TRUE(HasInstance(child2));
    }

//---------------------------------------------------------------------------------------
//                                               Muhammad Hassan                  10/2014
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(RelationshipStrengthTestFixture, BackwardHoldingForwardEmbedding)
    {
    ECDbR ecdb = SetupECDb("BackwardRelationshipStrengthTest.ecdb", BeFileName(L"RelationshipStrengthTest.01.00.ecschema.xml"));
    ASSERT_TRUE(ecdb.IsDbOpen());

    /*
    *          Create the following relationship hierarchy
    *
    * GrandParent1  <- ParentHasSpouse (REFERENCING) -> GrandParent2
    *     |__________________________________________________|
    *                             |
    *                             | ChildrenHaveManyParents.( Backward REFERENCING)
    *                             |
    *                         SingleParent
    *                             |
    *                             | SingleParentHasChildren.( Forward EMBEDDING)
    *      _______________________|__________________________
    *     |                                                  |
    *   Child1                                             Child2
    *
    */
    ECInstanceKey child1 = InsertPerson("First", "Child");
    ECInstanceKey child2 = InsertPerson("Second", "Child");
    ECInstanceKey singleParent = InsertPerson("Only", "singleParent");
    ECInstanceKey grandParent1 = InsertPerson("First", "GrandParent");
    ECInstanceKey grandParent2 = InsertPerson("Second", "GrandParent");

    // Backward referencing relationship (GrandParent1, GrandParent2 <- SingleParent)
    ECInstanceKey singleParentHasGrandParent1 = InsertRelationship("RelationshipStrengthTest.ChildrenHaveManyParents", singleParent, grandParent1);
    ECInstanceKey singleParentHasGrandParent2 = InsertRelationship("RelationshipStrengthTest.ChildrenHaveManyParents", singleParent, grandParent2);

    //Forward Embedding relationship (SingleParent -> Child1, Child2)
    ECInstanceKey singleParentHasChild1 = InsertRelationship("RelationshipStrengthTest.SingleParentHasChildren", singleParent, child1);
    ECInstanceKey singleParentHasChild2 = InsertRelationship("RelationshipStrengthTest.SingleParentHasChildren", singleParent, child2);

    //Backward Referencing relationship (GrandParent1 <-> GrandParent2)
    ECInstanceKey grandParent1HasSpouse = InsertRelationship("RelationshipStrengthTest.ParentHasSpouse_backward", grandParent1, grandParent2);
    ECInstanceKey grandParent2HasSpouse = InsertRelationship("RelationshipStrengthTest.ParentHasSpouse_backward", grandParent2, grandParent1);

    ecdb.SaveChanges();

    //Validate Instance exists before deletion
    ASSERT_TRUE(HasInstance(singleParentHasChild1));

    /*
    * Test 1: Delete Child1
    * Validate Child1 and singleParentHasChild1 have been deleted
    * Validate Child2 is Still there
    */
    DeleteInstance(child1);
    ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(child1));
    ASSERT_FALSE(HasInstance(singleParentHasChild1));

    ASSERT_TRUE(HasInstance(child2));

    /*
    * Test 2: Delete Child2
    * Validate Child2 and singleParentHasChild2 have been deleted
    * Validate singleParent is still around (relationship grand parents remaining)
    */
    DeleteInstance(child2);
    ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(child2));
    ASSERT_FALSE(HasInstance(singleParentHasChild2));

    ASSERT_TRUE(HasInstance(singleParent));

    /*
    * Test 3: Delete GrandParent1
    * Validate GrandParent1, grandParent1HasSpouse, grandParent2HasSpouse, singleParentHasGrandParent1 have been deleted
    * Validate singleParent is still around (referencing relationship with one parent remaining)
    */
    DeleteInstance(grandParent1);
    ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(grandParent1));
    ASSERT_FALSE(HasInstance(grandParent1HasSpouse));
    ASSERT_FALSE(HasInstance(grandParent2HasSpouse));
    ASSERT_FALSE(HasInstance(singleParentHasGrandParent1));

    ASSERT_TRUE(HasInstance(singleParent));

    /*
    * Test 4: Delete GrandParent2
    * Validate GrandParent2, singleParentHasGrandParent2 have been deleted, * Single parent has been deleted too as no parent exists anymore
    */
    DeleteInstance(grandParent2);
    ecdb.SaveChanges();

    ASSERT_FALSE(HasInstance(grandParent2));
    ASSERT_FALSE(HasInstance(singleParentHasGrandParent2));
    ASSERT_TRUE(HasInstance(singleParent));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                       Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
struct ECDbRelationshipsIntegrityTests : DbMappingTestFixture
    {
    enum class Multiplicity
        {
        ZeroOne,
        ZeroMany,
        OneOne,
        OneMany
        };

    enum class Direction
        {
        Forward,
        Backward
        };

    private:
        ECEntityClassP GetEntityClass(Utf8CP className);

        RelationshipMultiplicityCR GetClassMultiplicity(Multiplicity classMultiplicity);

        ECRelatedInstanceDirection GetRelationDirection(Direction direction);

    protected:
        ECSchemaPtr testSchema = nullptr;

        void CreateSchema(Utf8CP schemaName, Utf8CP schemaNamePrefix);

        //Adding a Class automatically adds a Property of Type string with Name "SqlPrintfString ("%sProp", className)" to the class.
        void AddEntityClass(Utf8CP className);

        void AddRelationShipClass(Multiplicity SourceClassMultiplicity, Multiplicity targetClassMultiplicity, StrengthType strengthType, Direction direction, Utf8CP relationshipClassName, Utf8CP sourceClass, Utf8CP targetClass, bool isPolymorphic);

        void AssertSchemaImport(bool isSchemaImportExpectedToSucceed);

        void InsertEntityClassInstances(Utf8CP className, Utf8CP propName, int numberOfInstances, std::vector<ECInstanceKey>& classKeys);

        void InsertRelationshipInstances(Utf8CP relationshipClass, std::vector<ECInstanceKey> const& sourceKeys, std::vector<ECInstanceKey>const& targetKeys, std::vector<DbResult> const& expected, size_t& rowInserted) const;

        size_t GetInsertedRelationshipsCount(Utf8CP relationshipClass) const;

        ECClassId GetRelationShipClassId(Utf8CP className);

        bool InstanceExists(Utf8CP classExp, ECInstanceKey const& key) const;

        bool RelationshipExists(Utf8CP relClassExp, ECInstanceKey const& sourceKey, ECInstanceKey const& targetKey) const;
    };

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
ECEntityClassP ECDbRelationshipsIntegrityTests::GetEntityClass(Utf8CP className)
    {
    return testSchema->GetClassP(className)->GetEntityClassP();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
RelationshipMultiplicityCR ECDbRelationshipsIntegrityTests::GetClassMultiplicity(Multiplicity classMultiplicity)
    {
    if (classMultiplicity == Multiplicity::ZeroOne)
        return RelationshipMultiplicity::ZeroOne();
    else if (classMultiplicity == Multiplicity::ZeroMany)
        return RelationshipMultiplicity::ZeroMany();
    else if (classMultiplicity == Multiplicity::OneOne)
        return RelationshipMultiplicity::OneOne();
    else
        return RelationshipMultiplicity::OneMany();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
ECRelatedInstanceDirection ECDbRelationshipsIntegrityTests::GetRelationDirection(Direction direction)
    {
    if (direction == Direction::Forward)
        return ECRelatedInstanceDirection::Forward;
    else
        return ECRelatedInstanceDirection::Backward;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbRelationshipsIntegrityTests::CreateSchema(Utf8CP schemaName, Utf8CP schemaNameAlias)
    {
    ECSchema::CreateSchema(testSchema, schemaName, schemaNameAlias, 1, 0, 0);

    ASSERT_TRUE(testSchema.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbRelationshipsIntegrityTests::AssertSchemaImport(bool isSchemaImportExpectedToSucceed)
    {

    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    EXPECT_EQ(ECObjectsStatus::Success, readContext->AddSchema(*testSchema));
    if (isSchemaImportExpectedToSucceed)
        {
        EXPECT_EQ(SUCCESS, m_ecdb.Schemas().ImportSchemas(readContext->GetCache().GetSchemas()));
        }
    else
        {
        EXPECT_EQ(ERROR, m_ecdb.Schemas().ImportSchemas(readContext->GetCache().GetSchemas()));
        }
    m_ecdb.SaveChanges();
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbRelationshipsIntegrityTests::AddEntityClass(Utf8CP className)
    {
    ECEntityClassP testClass = nullptr;
    EXPECT_EQ(ECObjectsStatus::Success, testSchema->CreateEntityClass(testClass, className));
    PrimitiveECPropertyP prim = nullptr;
    EXPECT_EQ(ECObjectsStatus::Success, testClass->CreatePrimitiveProperty(prim, SqlPrintfString("%sProp", className).GetUtf8CP()));
    EXPECT_EQ(ECObjectsStatus::Success, prim->SetType(PrimitiveType::PRIMITIVETYPE_String));
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbRelationshipsIntegrityTests::AddRelationShipClass(Multiplicity SourceClassMultiplicity, Multiplicity targetClassMultiplicity, StrengthType strengthType, Direction direction, Utf8CP relationshipClassName, Utf8CP sourceClass, Utf8CP targetClass, bool isPolymorphic)
    {
    ECRelationshipClassP testRelationshipClass = nullptr;
    EXPECT_EQ(ECObjectsStatus::Success, testSchema->CreateRelationshipClass(testRelationshipClass, relationshipClassName));
    testRelationshipClass->SetClassModifier(ECClassModifier::Sealed);
    EXPECT_EQ(ECObjectsStatus::Success, testRelationshipClass->SetStrength(strengthType));
    EXPECT_EQ(ECObjectsStatus::Success, testRelationshipClass->SetStrengthDirection(GetRelationDirection(direction)));

    //Set Relstionship Source Class and Cardinality
    EXPECT_EQ(ECObjectsStatus::Success, testRelationshipClass->GetSource().AddClass(*GetEntityClass(sourceClass)));
    EXPECT_EQ(ECObjectsStatus::Success, testRelationshipClass->GetSource().SetIsPolymorphic(true));
    EXPECT_EQ(ECObjectsStatus::Success, testRelationshipClass->GetSource().SetRoleLabel(testRelationshipClass->GetName().c_str()));
    EXPECT_EQ(ECObjectsStatus::Success, testRelationshipClass->GetSource().SetMultiplicity(GetClassMultiplicity(SourceClassMultiplicity)));

    //Set Relstionship Target Class and Cardinality
    EXPECT_EQ(ECObjectsStatus::Success, testRelationshipClass->GetTarget().AddClass(*GetEntityClass(targetClass)));
    EXPECT_EQ(ECObjectsStatus::Success, testRelationshipClass->GetTarget().SetIsPolymorphic(true));
    Utf8String reversedRoleLabel = testRelationshipClass->GetName() + " (Reversed)";
    EXPECT_EQ(ECObjectsStatus::Success, testRelationshipClass->GetTarget().SetRoleLabel(reversedRoleLabel.c_str()));
    EXPECT_EQ(ECObjectsStatus::Success, testRelationshipClass->GetTarget().SetMultiplicity(GetClassMultiplicity(targetClassMultiplicity)));

    if (SourceClassMultiplicity == Multiplicity::OneOne
        || SourceClassMultiplicity == Multiplicity::ZeroOne
        || targetClassMultiplicity == Multiplicity::OneOne
        || targetClassMultiplicity == Multiplicity::ZeroOne
        )
        {
        ECN::SchemaKey schemaKey = ECN::SchemaKey("ECDbMap", 2, 0, 0);
        ECSchemaCP ecdbMap = testSchema->FindSchema(schemaKey, SchemaMatchType::Exact);
        ECClassCP foreignKeyConstraintCA = ecdbMap->GetClassCP("ForeignKeyConstraint");
        IECInstancePtr  foreignKeyConstraintInstance = foreignKeyConstraintCA->GetDefaultStandaloneEnabler()->CreateInstance();
        testRelationshipClass->SetCustomAttribute(*foreignKeyConstraintInstance);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbRelationshipsIntegrityTests::InsertEntityClassInstances(Utf8CP className, Utf8CP propName, int numberOfInstances, std::vector<ECInstanceKey>& classKeys)
    {
    ECSqlStatement stmt;

    SqlPrintfString insertECSql = SqlPrintfString("INSERT INTO %s(%s) VALUES(?)", GetEntityClass(className)->GetECSqlName().c_str(), propName);

    ASSERT_EQ(stmt.Prepare(m_ecdb, insertECSql.GetUtf8CP()), ECSqlStatus::Success);
    for (int i = 0; i < numberOfInstances; i++)
        {
        ECInstanceKey key;
        SqlPrintfString textVal = SqlPrintfString("%s_%d", className, i);
        ASSERT_EQ(stmt.Reset(), ECSqlStatus::Success);
        ASSERT_EQ(stmt.ClearBindings(), ECSqlStatus::Success);
        ASSERT_EQ(stmt.BindText(1, textVal.GetUtf8CP(), IECSqlBinder::MakeCopy::No), ECSqlStatus::Success);
        ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
        classKeys.push_back(key);
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
void ECDbRelationshipsIntegrityTests::InsertRelationshipInstances(Utf8CP relationshipClass, std::vector<ECInstanceKey> const& sourceKeys, std::vector<ECInstanceKey>const& targetKeys, std::vector<DbResult> const& expected, size_t& rowInserted) const
    {
    ECSqlStatement stmt;
    SqlPrintfString sql = SqlPrintfString("INSERT INTO %s (SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId) VALUES(?,?,?,?)", relationshipClass);

    ASSERT_EQ(stmt.Prepare(m_ecdb, sql.GetUtf8CP()), ECSqlStatus::Success);
    ASSERT_EQ(expected.size(), sourceKeys.size() * targetKeys.size());
    int n = 0;
    for (auto& sourceKey : sourceKeys)
        {
        for (auto& targetKey : targetKeys)
            {
            stmt.Reset();
            ASSERT_EQ(ECSqlStatus::Success, stmt.ClearBindings());
            stmt.BindId(1, sourceKey.GetInstanceId());
            stmt.BindId(2, sourceKey.GetClassId());
            stmt.BindId(3, targetKey.GetInstanceId());
            stmt.BindId(4, targetKey.GetClassId());

            if (expected[n] != BE_SQLITE_DONE)
                ASSERT_NE(BE_SQLITE_DONE, stmt.Step());
            else
                {
                ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
                rowInserted++;
                }

            n = n + 1;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
size_t ECDbRelationshipsIntegrityTests::GetInsertedRelationshipsCount(Utf8CP relationshipClass) const
    {
    ECSqlStatement stmt;
    auto sql = SqlPrintfString("SELECT COUNT(*) FROM ONLY ts.Foo JOIN ts.Goo USING %s", relationshipClass);
    if (stmt.Prepare(m_ecdb, sql.GetUtf8CP()) == ECSqlStatus::Success)
        {
        if (stmt.Step() == BE_SQLITE_ROW)
            return static_cast<size_t>(stmt.GetValueInt(0));
        }

    return 0;
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
ECClassId ECDbRelationshipsIntegrityTests::GetRelationShipClassId(Utf8CP className)
    {
    return testSchema->GetClassP(className)->GetRelationshipClassP()->GetId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbRelationshipsIntegrityTests::InstanceExists(Utf8CP classExp, ECInstanceKey const& key) const
    {
    Utf8String ecsql;
    ecsql.Sprintf("SELECT NULL FROM %s WHERE ECInstanceId=?", classExp);
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), ecsql.c_str())) << ecsql.c_str();
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(1, key.GetInstanceId()));

    DbResult stat = stmt.Step();
    EXPECT_TRUE(stat == BE_SQLITE_ROW || stat == BE_SQLITE_DONE);
    return stat == BE_SQLITE_ROW;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  02/16
//+---------------+---------------+---------------+---------------+---------------+------
bool ECDbRelationshipsIntegrityTests::RelationshipExists(Utf8CP relClassExp, ECInstanceKey const& sourceKey, ECInstanceKey const& targetKey) const
    {
    Utf8String ecsql;
    ecsql.Sprintf("SELECT NULL FROM %s WHERE SourceECInstanceId=? AND SourceECClassId=? AND TargetECInstanceId=? AND TargetECClassId=?", relClassExp);
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), ecsql.c_str())) << ecsql.c_str();
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(1, sourceKey.GetInstanceId()));
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(2, sourceKey.GetClassId()));
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(3, targetKey.GetInstanceId()));
    EXPECT_EQ(ECSqlStatus::Success, stmt.BindId(4, targetKey.GetClassId()));

    DbResult stat = stmt.Step();
    EXPECT_TRUE(stat == BE_SQLITE_ROW || stat == BE_SQLITE_DONE);
    return stat == BE_SQLITE_ROW;
    };

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbRelationshipsIntegrityTests, ForwardEmbeddingRelationshipsTest)
    {
    SetupECDb("forwardEmbeddingRelationshipsTest.ecdb");
    ASSERT_TRUE(GetECDb().IsDbOpen());

    CreateSchema("testSchema", "ts");
    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    readContext->AddSchemaLocater(GetECDb().GetSchemaLocater());
    ECN::SchemaKey schemaKey = ECN::SchemaKey("ECDbMap", 2, 0, 0);
    ECSchemaPtr ecdbMap = ECSchema::LocateSchema(schemaKey, *readContext);
    readContext->RemoveSchemaLocater(GetECDb().GetSchemaLocater());
    ASSERT_TRUE(ecdbMap.IsValid());
    testSchema->AddReferencedSchema(*ecdbMap);

    AddEntityClass("Foo");
    AddEntityClass("Goo");
    AddRelationShipClass(Multiplicity::ZeroOne, Multiplicity::OneOne, StrengthType::Embedding, Direction::Forward, "FooOwnsGoo", "Foo", "Goo", true);
    AddRelationShipClass(Multiplicity::ZeroOne, Multiplicity::OneMany, StrengthType::Embedding, Direction::Forward, "FooOwnsManyGoo", "Foo", "Goo", true);
    AssertSchemaImport(true);

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;
    InsertEntityClassInstances("Foo", "FooProp", maxFooInstances, fooKeys);
    InsertEntityClassInstances("Goo", "GooProp", maxGooInstances, gooKeys);

    //Compute what are the right valid permutation
    std::vector<DbResult> FooOwnsGooResult;
    std::vector<DbResult> FooOwnsManyGooResult;

    for (auto f = 0; f < maxFooInstances; f++)
        {
        for (auto g = 0; g < maxGooInstances; g++)
            {
            //Source(0,1), Target(1,1)
            if (f == g)
                FooOwnsGooResult.push_back(BE_SQLITE_DONE);
            else
                FooOwnsGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(0,1), Target(1,N)
            if (f == 0)
                FooOwnsManyGooResult.push_back(BE_SQLITE_DONE);
            else
                FooOwnsManyGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);
            }
        }

    //1-1............................
    MapStrategyInfo mapStrategy;
    size_t count_FooOwnsGoo = 0;
    {
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, m_ecdb, GetRelationShipClassId("FooOwnsGoo")));
    ASSERT_EQ((int) MapStrategyInfo::Strategy::ForeignKeyRelationshipInTargetTable, (int) mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooOwnsGoo", fooKeys, gooKeys, FooOwnsGooResult, count_FooOwnsGoo);
    }
    ASSERT_EQ(count_FooOwnsGoo, GetInsertedRelationshipsCount("ts.FooOwnsGoo"));

    //1-N............................
    size_t count_FooOwnsManyGoo = 0;
    {
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, m_ecdb, GetRelationShipClassId("FooOwnsManyGoo")));
    ASSERT_EQ((int) MapStrategyInfo::Strategy::ForeignKeyRelationshipInTargetTable, (int) mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooOwnsManyGoo", fooKeys, gooKeys, FooOwnsManyGooResult, count_FooOwnsManyGoo);
    }
    ASSERT_EQ(count_FooOwnsManyGoo, GetInsertedRelationshipsCount("ts.FooOwnsManyGoo"));

    //Delete fooKeys[0]
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "DELETE FROM ts.Foo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKeys[0].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[0]));

    //fooKeys[1] and fooKeys[2] which are in same table have no impact
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[2]));

    //fooKeys[0] embedds gooKeys, so all gooKeys will get deleted.
    ASSERT_FALSE(InstanceExists("ts.Goo", gooKeys[0]));
    ASSERT_FALSE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_FALSE(InstanceExists("ts.Goo", gooKeys[2]));

    //As a result all the relationships will also get deleted.
    ASSERT_FALSE(RelationshipExists("ts.FooOwnsGoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooOwnsGoo", fooKeys[1], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.FooOwnsGoo", fooKeys[2], gooKeys[2]));

    ASSERT_FALSE(RelationshipExists("ts.FooOwnsManyGoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooOwnsManyGoo", fooKeys[0], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.FooOwnsManyGoo", fooKeys[0], gooKeys[2]));
    }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbRelationshipsIntegrityTests, BackwardEmbeddingRelationshipsTest)
    {
    SetupECDb("backwardEmbeddingRelationshipTest.ecdb");
    ASSERT_TRUE(GetECDb().IsDbOpen());

    CreateSchema("testSchema", "ts");
    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    readContext->AddSchemaLocater(GetECDb().GetSchemaLocater());
    ECN::SchemaKey schemaKey = ECN::SchemaKey("ECDbMap", 2, 0, 0);
    ECSchemaPtr ecdbMap = ECSchema::LocateSchema(schemaKey, *readContext);
    readContext->RemoveSchemaLocater(GetECDb().GetSchemaLocater());
    ASSERT_TRUE(ecdbMap.IsValid());
    testSchema->AddReferencedSchema(*ecdbMap);

    AddEntityClass("Foo");
    AddEntityClass("Goo");
    AddRelationShipClass(Multiplicity::ZeroOne, Multiplicity::ZeroOne, StrengthType::Embedding, Direction::Backward, "FooOwnedByGoo", "Foo", "Goo", true);
    AddRelationShipClass(Multiplicity::OneMany, Multiplicity::ZeroOne, StrengthType::Embedding, Direction::Backward, "FooOwnedByManyGoo", "Foo", "Goo", true);
    AssertSchemaImport(true);

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;
    InsertEntityClassInstances("Foo", "FooProp", maxFooInstances, fooKeys);
    InsertEntityClassInstances("Goo", "GooProp", maxGooInstances, gooKeys);

    //Compute what are the right valid permutation
    std::vector<DbResult> FooOwnedByGooResult;
    std::vector<DbResult> FooOwnedByManyGooResult;

    for (auto f = 0; f < maxFooInstances; f++)
        {
        for (auto g = 0; g < maxGooInstances; g++)
            {
            //Source(1,1), Target(0,1)
            if (f == g)
                FooOwnedByGooResult.push_back(BE_SQLITE_DONE);
            else
                FooOwnedByGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(1,N), Target(0,1)
            if (g == 0)
                FooOwnedByManyGooResult.push_back(BE_SQLITE_DONE);
            else
                FooOwnedByManyGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);
            }
        }

    //1-1...........................
    MapStrategyInfo mapStrategy;
    size_t count_FooOwnedByGoo = 0;
    {
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, m_ecdb, GetRelationShipClassId("FooOwnedByGoo")));
    ASSERT_EQ(MapStrategyInfo::Strategy::ForeignKeyRelationshipInSourceTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooOwnedByGoo", fooKeys, gooKeys, FooOwnedByGooResult, count_FooOwnedByGoo);
    }
    ASSERT_EQ(count_FooOwnedByGoo, GetInsertedRelationshipsCount("ts.FooOwnedByGoo"));

    //1-N..........................
    size_t count_FooOwnedByManyGoo = 0;
    {
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, m_ecdb, GetRelationShipClassId("FooOwnedByManyGoo")));
    ASSERT_EQ(MapStrategyInfo::Strategy::ForeignKeyRelationshipInSourceTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooOwnedByManyGoo", fooKeys, gooKeys, FooOwnedByManyGooResult, count_FooOwnedByManyGoo);
    }
    ASSERT_EQ(count_FooOwnedByManyGoo, GetInsertedRelationshipsCount("ts.FooOwnedByManyGoo"));

    //Delete gooKeys[0]
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "DELETE FROM ts.Goo WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, gooKeys[0].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    ASSERT_FALSE(InstanceExists("ts.Goo", gooKeys[0]));

    //gooKeys[1] and gooKeys[2] will have no impact
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));

    //fooKeys[0] embedds fooKeys, so all fooKeys will get deleted.
    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[0]));
    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[2]));

    //As a result all the relationships will also get deleted.
    ASSERT_FALSE(RelationshipExists("ts.FooOwnedByGoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooOwnedByGoo", fooKeys[1], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.FooOwnedByGoo", fooKeys[2], gooKeys[2]));

    ASSERT_FALSE(RelationshipExists("ts.FooOwnedByManyGoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooOwnedByManyGoo", fooKeys[0], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.FooOwnedByManyGoo", fooKeys[0], gooKeys[2]));
    }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbRelationshipsIntegrityTests, ForwardReferencingRelationshipsTest)
    {
    SetupECDb("forwardReferencingRelationshipsTest.ecdb");
    ASSERT_TRUE(GetECDb().IsDbOpen());

    CreateSchema("testSchema", "ts");
    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    readContext->AddSchemaLocater(GetECDb().GetSchemaLocater());
    ECN::SchemaKey schemaKey = ECN::SchemaKey("ECDbMap", 2, 0, 0);
    ECSchemaPtr ecdbMap = ECSchema::LocateSchema(schemaKey, *readContext);
    readContext->RemoveSchemaLocater(GetECDb().GetSchemaLocater());
    ASSERT_TRUE(ecdbMap.IsValid());
    testSchema->AddReferencedSchema(*ecdbMap);

    AddEntityClass("Foo");
    AddEntityClass("Goo");
    AddRelationShipClass(Multiplicity::ZeroOne, Multiplicity::ZeroOne, StrengthType::Referencing, Direction::Forward, "FooHasGoo", "Foo", "Goo", true);
    AddRelationShipClass(Multiplicity::ZeroOne, Multiplicity::ZeroMany, StrengthType::Referencing, Direction::Forward, "FooHasManyGoo", "Foo", "Goo", true);
    AddRelationShipClass(Multiplicity::ZeroMany, Multiplicity::ZeroMany, StrengthType::Referencing, Direction::Forward, "ManyFoohaveManyGoo", "Foo", "Goo", true);
    AssertSchemaImport(true);

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;
    InsertEntityClassInstances("Foo", "FooProp", maxFooInstances, fooKeys);
    InsertEntityClassInstances("Goo", "GooProp", maxGooInstances, gooKeys);

    //Compute what are the right valid permutation
    std::vector<DbResult> FooOwnsGooResult;
    std::vector<DbResult> FooOwnsManyGooResult;
    std::vector<DbResult> ManyFooOwnManyGooResult;

    for (auto f = 0; f < maxFooInstances; f++)
        {
        for (auto g = 0; g < maxGooInstances; g++)
            {
            //Source(0,1), Target(0,1)
            if (f == g)
                FooOwnsGooResult.push_back(BE_SQLITE_DONE);
            else
                FooOwnsGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(0,1), Target(0,N)
            if (f == 0)
                FooOwnsManyGooResult.push_back(BE_SQLITE_DONE);
            else
                FooOwnsManyGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(0,N), Target(0,N)
            ManyFooOwnManyGooResult.push_back(BE_SQLITE_DONE);
            }
        }

    //1-1............................
    MapStrategyInfo mapStrategy;
    size_t count_FooHasGoo = 0;
    {
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, m_ecdb, GetRelationShipClassId("FooHasGoo")));
    ASSERT_EQ(MapStrategyInfo::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooHasGoo", fooKeys, gooKeys, FooOwnsGooResult, count_FooHasGoo);
    }
    ASSERT_EQ(count_FooHasGoo, GetInsertedRelationshipsCount("ts.FooHasGoo"));

    //1-N...........................
    size_t count_FooHasManyGoo = 0;
    {
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, m_ecdb, GetRelationShipClassId("FooHasManyGoo")));
    ASSERT_EQ(MapStrategyInfo::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooHasManyGoo", fooKeys, gooKeys, FooOwnsManyGooResult, count_FooHasManyGoo);
    }
    ASSERT_EQ(count_FooHasManyGoo, GetInsertedRelationshipsCount("ts.FooHasManyGoo"));

    //N-N...........................
    size_t count_ManyFoohaveManyGoo = 0;
    {
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, m_ecdb, GetRelationShipClassId("ManyFoohaveManyGoo")));
    ASSERT_EQ(MapStrategyInfo::Strategy::OwnTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.ManyFoohaveManyGoo", fooKeys, gooKeys, ManyFooOwnManyGooResult, count_ManyFoohaveManyGoo);
    }
    ASSERT_EQ(count_ManyFoohaveManyGoo, GetInsertedRelationshipsCount("ts.ManyFoohaveManyGoo"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "DELETE FROM ts.Foo WHERE ECInstanceId=?"));
    //Delete fooKeys[0]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKeys[0].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[1]));//fooKeys[1] and fooKeys[2] which are in same table have no impact
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooHasGoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooHasManyGoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooHasManyGoo", fooKeys[0], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.FooHasManyGoo", fooKeys[0], gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.ManyFoohaveManyGoo", fooKeys[0], gooKeys[0]));

    ASSERT_TRUE(RelationshipExists("ts.FooHasGoo", fooKeys[1], gooKeys[1]));
    ASSERT_TRUE(RelationshipExists("ts.ManyFoohaveManyGoo", fooKeys[1], gooKeys[1]));
    }

    //Delete fooKeys[1]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKeys[1].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[2])); //fooKeys[2] which is in same table has no impact
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.FooHasGoo", fooKeys[1], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.ManyFoohaveManyGoo", fooKeys[1], gooKeys[1]));

    ASSERT_TRUE(RelationshipExists("ts.FooHasGoo", fooKeys[2], gooKeys[2]));
    ASSERT_TRUE(RelationshipExists("ts.ManyFoohaveManyGoo", fooKeys[2], gooKeys[2]));
    }

    //Delete fooKeys[2]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKeys[2].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.FooHasGoo", fooKeys[2], gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.ManyFoohaveManyGoo", fooKeys[2], gooKeys[2]));

    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[0]));//all gooKeys must exist, refered instances never get deleted.
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbRelationshipsIntegrityTests, BackwardReferencingRelationshipsTest)
    {
    SetupECDb("backwardReferencingRelationshipsTest.ecdb");
    ASSERT_TRUE(GetECDb().IsDbOpen());

    CreateSchema("testSchema", "ts");
    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    readContext->AddSchemaLocater(GetECDb().GetSchemaLocater());
    ECN::SchemaKey schemaKey = ECN::SchemaKey("ECDbMap", 2, 0, 0);
    ECSchemaPtr ecdbMap = ECSchema::LocateSchema(schemaKey, *readContext);
    readContext->RemoveSchemaLocater(GetECDb().GetSchemaLocater());
    ASSERT_TRUE(ecdbMap.IsValid());
    testSchema->AddReferencedSchema(*ecdbMap);

    AddEntityClass("Foo");
    AddEntityClass("Goo");
    AddRelationShipClass(Multiplicity::ZeroOne, Multiplicity::ZeroOne, StrengthType::Referencing, Direction::Backward, "GooHasFoo", "Foo", "Goo", true);
    AddRelationShipClass(Multiplicity::ZeroMany, Multiplicity::ZeroOne, StrengthType::Referencing, Direction::Backward, "GooHasManyFoo", "Foo", "Goo", true);
    AddRelationShipClass(Multiplicity::ZeroMany, Multiplicity::ZeroMany, StrengthType::Referencing, Direction::Backward, "ManyGooHaveManyFoo", "Foo", "Goo", true);
    AssertSchemaImport(true);

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;
    InsertEntityClassInstances("Foo", "FooProp", maxFooInstances, fooKeys);
    InsertEntityClassInstances("Goo", "GooProp", maxGooInstances, gooKeys);

    //Compute what are the right valid permutation
    std::vector<DbResult> GooOwnsFooResult;
    std::vector<DbResult> GooOwnsManyFooResult;
    std::vector<DbResult> ManyGooOwnManyFooResult;

    for (auto f = 0; f < maxFooInstances; f++)
        {
        for (auto g = 0; g < maxGooInstances; g++)
            {
            //Source(0,1), Target(0,1)
            if (f == g)
                GooOwnsFooResult.push_back(BE_SQLITE_DONE);
            else
                GooOwnsFooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(0,N), Target(0,1)
            if (g == 0)
                GooOwnsManyFooResult.push_back(BE_SQLITE_DONE);
            else
                GooOwnsManyFooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(0,N), Target(0,N)
            ManyGooOwnManyFooResult.push_back(BE_SQLITE_DONE);
            }
        }

    //1-1............................
    MapStrategyInfo mapStrategy;
    size_t count_GooHasFoo = 0;
    {
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, m_ecdb, GetRelationShipClassId("GooHasFoo")));
    ASSERT_EQ(MapStrategyInfo::Strategy::ForeignKeyRelationshipInSourceTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.GooHasFoo", fooKeys, gooKeys, GooOwnsFooResult, count_GooHasFoo);
    }
    ASSERT_EQ(count_GooHasFoo, GetInsertedRelationshipsCount("ts.GooHasFoo"));

    //1-N...........................
    size_t count_GooHasManyFoo = 0;
    {
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, m_ecdb, GetRelationShipClassId("GooHasManyFoo")));
    ASSERT_EQ(MapStrategyInfo::Strategy::ForeignKeyRelationshipInSourceTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.GooHasManyFoo", fooKeys, gooKeys, GooOwnsManyFooResult, count_GooHasManyFoo);
    }
    ASSERT_EQ(count_GooHasManyFoo, GetInsertedRelationshipsCount("ts.GooHasManyFoo"));

    //N-N...........................
    size_t count_ManyGooHaveManyFoo = 0;
    {
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, m_ecdb, GetRelationShipClassId("ManyGooHaveManyFoo")));
    ASSERT_EQ(MapStrategyInfo::Strategy::OwnTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.ManyGooHaveManyFoo", fooKeys, gooKeys, ManyGooOwnManyFooResult, count_ManyGooHaveManyFoo);
    }
    ASSERT_EQ(count_ManyGooHaveManyFoo, GetInsertedRelationshipsCount("ts.ManyGooHaveManyFoo"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "DELETE FROM ts.Goo WHERE ECInstanceId=?"));
    //Delete gooKeys[0]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, gooKeys[0].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Goo", gooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.GooHasFoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.GooHasManyFoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.GooHasManyFoo", fooKeys[0], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.GooHasManyFoo", fooKeys[0], gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.ManyGooHaveManyFoo", fooKeys[0], gooKeys[0]));

    ASSERT_TRUE(RelationshipExists("ts.GooHasFoo", fooKeys[1], gooKeys[1]));
    ASSERT_TRUE(RelationshipExists("ts.ManyGooHaveManyFoo", fooKeys[1], gooKeys[1]));
    }

    //Delete gooKeys[1]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, gooKeys[1].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.GooHasFoo", fooKeys[1], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.ManyGooHaveManyFoo", fooKeys[1], gooKeys[1]));

    ASSERT_TRUE(RelationshipExists("ts.GooHasFoo", fooKeys[2], gooKeys[2]));
    ASSERT_TRUE(RelationshipExists("ts.ManyGooHaveManyFoo", fooKeys[2], gooKeys[2]));
    }

    //Delete gooKeys[2]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, gooKeys[2].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.GooHasFoo", fooKeys[2], gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.ManyGooHaveManyFoo", fooKeys[2], gooKeys[2]));

    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[0]));//all fooKeys must exist, refered instances never get deleted.
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[2]));
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Muhammad Hassan                  05/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbRelationshipsIntegrityTests, ForwardHoldingOneToOne)
    {
    SetupECDb("forwardHoldingRelationshipsTest.ecdb");
    ASSERT_TRUE(GetECDb().IsDbOpen());

    CreateSchema("testSchema", "ts");
    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    readContext->AddSchemaLocater(GetECDb().GetSchemaLocater());
    ECN::SchemaKey schemaKey = ECN::SchemaKey("ECDbMap", 2, 0, 0);
    ECSchemaPtr ecdbMap = ECSchema::LocateSchema(schemaKey, *readContext);
    readContext->RemoveSchemaLocater(GetECDb().GetSchemaLocater());
    ASSERT_TRUE(ecdbMap.IsValid());
    testSchema->AddReferencedSchema(*ecdbMap);

    AddEntityClass("Foo");
    AddEntityClass("Goo");
    AddRelationShipClass(Multiplicity::ZeroOne, Multiplicity::OneOne, StrengthType::Holding, Direction::Forward, "FooHoldsGoo", "Foo", "Goo", true);
    AssertSchemaImport(true);

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;
    InsertEntityClassInstances("Foo", "FooProp", maxFooInstances, fooKeys);
    InsertEntityClassInstances("Goo", "GooProp", maxGooInstances, gooKeys);

    //Compute what are the right valid permutation
    std::vector<DbResult> FooHoldsGooResult;

    for (auto f = 0; f < maxFooInstances; f++)
        {
        for (auto g = 0; g < maxGooInstances; g++)
            {
            //Source(1,1), Target(1,1)
            if (f == g)
                FooHoldsGooResult.push_back(BE_SQLITE_DONE);
            else
                FooHoldsGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);
            }
        }

    //1-1............................
    MapStrategyInfo mapStrategy;
    size_t count_FooHoldsGoo = 0;
    {
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, m_ecdb, GetRelationShipClassId("FooHoldsGoo")));
    ASSERT_EQ(MapStrategyInfo::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooHoldsGoo", fooKeys, gooKeys, FooHoldsGooResult, count_FooHoldsGoo);
    }
    ASSERT_EQ(count_FooHoldsGoo, GetInsertedRelationshipsCount("ts.FooHoldsGoo"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "DELETE FROM ts.Foo WHERE ECInstanceId=?"));

    //Delete fooKeys[0]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKeys[0].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.FooHoldsGoo", fooKeys[0], gooKeys[0]));
    }

    //Delete fooKeys[1]
    {
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKeys[1].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.FooHoldsGoo", fooKeys[1], gooKeys[1]));
    }

    //Delete fooKeys[2]
    {
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKeys[2].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.FooHoldsGoo", fooKeys[2], gooKeys[2]));
    }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbRelationshipsIntegrityTests, ForwardHoldingOneToMany)
    {
    SetupECDb("forwardHoldingOneToOneRelationshipsTest.ecdb");
    ASSERT_TRUE(GetECDb().IsDbOpen());

    CreateSchema("testSchema", "ts");
    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    readContext->AddSchemaLocater(GetECDb().GetSchemaLocater());
    ECN::SchemaKey schemaKey = ECN::SchemaKey("ECDbMap", 2, 0, 0);
    ECSchemaPtr ecdbMap = ECSchema::LocateSchema(schemaKey, *readContext);
    readContext->RemoveSchemaLocater(GetECDb().GetSchemaLocater());
    ASSERT_TRUE(ecdbMap.IsValid());
    testSchema->AddReferencedSchema(*ecdbMap);

    AddEntityClass("Foo");
    AddEntityClass("Goo");
    AddRelationShipClass(Multiplicity::ZeroOne, Multiplicity::OneMany, StrengthType::Holding, Direction::Forward, "FooHoldManyGoo", "Foo", "Goo", true);
    AssertSchemaImport(true);

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;
    InsertEntityClassInstances("Foo", "FooProp", maxFooInstances, fooKeys);
    InsertEntityClassInstances("Goo", "GooProp", maxGooInstances, gooKeys);

    //Compute what are the right valid permutation
    std::vector<DbResult> FooHoldsManyGooResult;

    for (auto f = 0; f < maxFooInstances; f++)
        {
        for (auto g = 0; g < maxGooInstances; g++)
            {
            //Source(1,1), Target(1,N)
            if (f == 0)
                FooHoldsManyGooResult.push_back(BE_SQLITE_DONE);
            else
                FooHoldsManyGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);
            }
        }

    //1-N............................
    MapStrategyInfo mapStrategy;
    size_t count_FooHoldManyGoo = 0;
    {
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, m_ecdb, GetRelationShipClassId("FooHoldManyGoo")));
    ASSERT_EQ(MapStrategyInfo::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooHoldManyGoo", fooKeys, gooKeys, FooHoldsManyGooResult, count_FooHoldManyGoo);
    }
    ASSERT_EQ(count_FooHoldManyGoo, GetInsertedRelationshipsCount("ts.FooHoldManyGoo"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "DELETE FROM ts.Foo WHERE ECInstanceId=?"));
    //Delete fooKeys[0]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKeys[0].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.FooHoldManyGoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooHoldManyGoo", fooKeys[0], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.FooHoldManyGoo", fooKeys[0], gooKeys[2]));
    }

    //Delete fooKeys[1]
    {
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKeys[1].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    }

    //Delete fooKeys[2]
    {
    stmt.Reset();
    stmt.ClearBindings();
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKeys[2].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbRelationshipsIntegrityTests, ForwardHoldingRelationshipsTest)
    {
    SetupECDb("forwardHoldingRelationshipsTest.ecdb");
    ASSERT_TRUE(GetECDb().IsDbOpen());

    CreateSchema("testSchema", "ts");
    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    readContext->AddSchemaLocater(GetECDb().GetSchemaLocater());
    ECN::SchemaKey schemaKey = ECN::SchemaKey("ECDbMap", 2, 0, 0);
    ECSchemaPtr ecdbMap = ECSchema::LocateSchema(schemaKey, *readContext);
    readContext->RemoveSchemaLocater(GetECDb().GetSchemaLocater());
    ASSERT_TRUE(ecdbMap.IsValid());
    testSchema->AddReferencedSchema(*ecdbMap);

    AddEntityClass("Foo");
    AddEntityClass("Goo");
    AddRelationShipClass(Multiplicity::ZeroOne, Multiplicity::OneOne, StrengthType::Holding, Direction::Forward, "FooHoldsGoo", "Foo", "Goo", true);
    AddRelationShipClass(Multiplicity::ZeroOne, Multiplicity::OneMany, StrengthType::Holding, Direction::Forward, "FooHoldManyGoo", "Foo", "Goo", true);
    AddRelationShipClass(Multiplicity::OneMany, Multiplicity::OneMany, StrengthType::Holding, Direction::Forward, "ManyFooHoldManyGoo", "Foo", "Goo", true);
    AssertSchemaImport(true);

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;
    InsertEntityClassInstances("Foo", "FooProp", maxFooInstances, fooKeys);
    InsertEntityClassInstances("Goo", "GooProp", maxGooInstances, gooKeys);

    //Compute what are the right valid permutation
    std::vector<DbResult> FooHoldsGooResult;
    std::vector<DbResult> FooHoldsManyGooResult;
    std::vector<DbResult> ManyFooHoldManyGooResult;

    for (auto f = 0; f < maxFooInstances; f++)
        {
        for (auto g = 0; g < maxGooInstances; g++)
            {
            //Source(1,1), Target(1,1)
            if (f == g)
                FooHoldsGooResult.push_back(BE_SQLITE_DONE);
            else
                FooHoldsGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(1,1), Target(1,N)
            if (f == 0)
                FooHoldsManyGooResult.push_back(BE_SQLITE_DONE);
            else
                FooHoldsManyGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(1,N), Target(1,N)
            ManyFooHoldManyGooResult.push_back(BE_SQLITE_DONE);
            }
        }

    //1-1............................
    MapStrategyInfo mapStrategy;
    size_t count_FooHoldsGoo = 0;
    {
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, m_ecdb, GetRelationShipClassId("FooHoldsGoo")));
    ASSERT_EQ(MapStrategyInfo::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooHoldsGoo", fooKeys, gooKeys, FooHoldsGooResult, count_FooHoldsGoo);
    }
    ASSERT_EQ(count_FooHoldsGoo, GetInsertedRelationshipsCount("ts.FooHoldsGoo"));

    //1-N............................
    size_t count_FooHoldManyGoo = 0;
    {
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, m_ecdb, GetRelationShipClassId("FooHoldManyGoo")));
    ASSERT_EQ(MapStrategyInfo::Strategy::ForeignKeyRelationshipInTargetTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooHoldManyGoo", fooKeys, gooKeys, FooHoldsManyGooResult, count_FooHoldManyGoo);
    }
    ASSERT_EQ(count_FooHoldManyGoo, GetInsertedRelationshipsCount("ts.FooHoldManyGoo"));

    //N-N...........................
    size_t count_ManyFooHoldManyGoo = 0;
    {
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, m_ecdb, GetRelationShipClassId("ManyFooHoldManyGoo")));
    ASSERT_EQ(MapStrategyInfo::Strategy::OwnTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.ManyFooHoldManyGoo", fooKeys, gooKeys, ManyFooHoldManyGooResult, count_ManyFooHoldManyGoo);
    }
    ASSERT_EQ(count_ManyFooHoldManyGoo, GetInsertedRelationshipsCount("ts.ManyFooHoldManyGoo"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "DELETE FROM ts.Foo WHERE ECInstanceId=?"));
    //Delete fooKeys[0]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKeys[0].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.FooHoldsGoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooHoldManyGoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooHoldManyGoo", fooKeys[0], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.FooHoldManyGoo", fooKeys[0], gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.ManyFooHoldManyGoo", fooKeys[0], gooKeys[0]));
    }

    //Delete fooKeys[1]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKeys[1].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.FooHoldsGoo", fooKeys[1], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.ManyFooHoldManyGoo", fooKeys[1], gooKeys[1]));
    }

    //Delete fooKeys[2]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, fooKeys[2].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Foo", fooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.FooHoldsGoo", fooKeys[2], gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.ManyFooHoldManyGoo", fooKeys[2], gooKeys[2]));
    }
    }

//---------------------------------------------------------------------------------------
// @bsiMethod                                      Muhammad Hassan                  01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECDbRelationshipsIntegrityTests, BackwardHoldingRelationshipsTest)
    {
    SetupECDb("backwardHoldingRelationshipsTest.ecdb");
    ASSERT_TRUE(GetECDb().IsDbOpen());

    CreateSchema("testSchema", "ts");
    ECSchemaReadContextPtr readContext = ECSchemaReadContext::CreateContext();
    readContext->AddSchemaLocater(GetECDb().GetSchemaLocater());
    ECN::SchemaKey schemaKey = ECN::SchemaKey("ECDbMap", 2, 0, 0);
    ECSchemaPtr ecdbMap = ECSchema::LocateSchema(schemaKey, *readContext);
    readContext->RemoveSchemaLocater(GetECDb().GetSchemaLocater());
    ASSERT_TRUE(ecdbMap.IsValid());
    testSchema->AddReferencedSchema(*ecdbMap);

    AddEntityClass("Foo");
    AddEntityClass("Goo");
    AddRelationShipClass(Multiplicity::ZeroOne, Multiplicity::ZeroOne, StrengthType::Holding, Direction::Backward, "FooHeldByGoo", "Foo", "Goo", true);
    AddRelationShipClass(Multiplicity::OneMany, Multiplicity::ZeroOne, StrengthType::Holding, Direction::Backward, "FooHeldByManyGoo", "Foo", "Goo", true);
    AddRelationShipClass(Multiplicity::OneMany, Multiplicity::OneMany, StrengthType::Holding, Direction::Backward, "ManyFooHeldByManyGoo", "Foo", "Goo", true);
    AssertSchemaImport(true);

    std::vector<ECInstanceKey> fooKeys, gooKeys;
    const int maxFooInstances = 3;
    const int maxGooInstances = 3;
    InsertEntityClassInstances("Foo", "FooProp", maxFooInstances, fooKeys);
    InsertEntityClassInstances("Goo", "GooProp", maxGooInstances, gooKeys);

    //Compute what are the right valid permutation
    std::vector<DbResult> fooHeldByGooResult;
    std::vector<DbResult> fooHeldByManyGooResult;
    std::vector<DbResult> manyFooHeldByManyGoo;

    for (auto f = 0; f < maxFooInstances; f++)
        {
        for (auto g = 0; g < maxGooInstances; g++)
            {
            //Source(1,1), Target(1,1)
            if (f == g)
                fooHeldByGooResult.push_back(BE_SQLITE_DONE);
            else
                fooHeldByGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(1,N), Target(1,1)
            if (g == 0)
                fooHeldByManyGooResult.push_back(BE_SQLITE_DONE);
            else
                fooHeldByManyGooResult.push_back(BE_SQLITE_CONSTRAINT_UNIQUE);

            //Source(1,N), Target(1,N)
            manyFooHeldByManyGoo.push_back(BE_SQLITE_DONE);
            }
        }

    //1-1............................
    MapStrategyInfo mapStrategy;
    size_t count_FooHeldByGoo = 0;
    {
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, m_ecdb, GetRelationShipClassId("FooHeldByGoo")));
    ASSERT_EQ(MapStrategyInfo::Strategy::ForeignKeyRelationshipInSourceTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooHeldByGoo", fooKeys, gooKeys, fooHeldByGooResult, count_FooHeldByGoo);
    }
    ASSERT_EQ(count_FooHeldByGoo, GetInsertedRelationshipsCount("ts.FooHeldByGoo"));

    //1-N...........................
    size_t count_FooHeldByManyGoo = 0;
    {
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, m_ecdb, GetRelationShipClassId("FooHeldByManyGoo")));
    ASSERT_EQ(MapStrategyInfo::Strategy::ForeignKeyRelationshipInSourceTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.FooHeldByManyGoo", fooKeys, gooKeys, fooHeldByManyGooResult, count_FooHeldByManyGoo);
    }
    ASSERT_EQ(count_FooHeldByManyGoo, GetInsertedRelationshipsCount("ts.FooHeldByManyGoo"));

    //N-N...........................
    size_t count_ManyFooHeldByManyGoo = 0;
    {
    ASSERT_TRUE(TryGetMapStrategyInfo(mapStrategy, m_ecdb, GetRelationShipClassId("ManyFooHeldByManyGoo")));
    ASSERT_EQ(MapStrategyInfo::Strategy::OwnTable, mapStrategy.m_strategy);

    InsertRelationshipInstances("ts.ManyFooHeldByManyGoo", fooKeys, gooKeys, manyFooHeldByManyGoo, count_ManyFooHeldByManyGoo);
    }
    ASSERT_EQ(count_ManyFooHeldByManyGoo, GetInsertedRelationshipsCount("ts.ManyFooHeldByManyGoo"));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(GetECDb(), "DELETE FROM ts.Goo WHERE ECInstanceId=?"));
    //Delete gooKeys[0]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, gooKeys[0].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Goo", gooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.FooHeldByGoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooHeldByManyGoo", fooKeys[0], gooKeys[0]));
    ASSERT_FALSE(RelationshipExists("ts.FooHeldByManyGoo", fooKeys[0], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.FooHeldByManyGoo", fooKeys[0], gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.ManyFooHeldByManyGoo", fooKeys[0], gooKeys[0]));
    }

    //Delete gooKeys[1]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, gooKeys[1].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Goo", gooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.FooHeldByGoo", fooKeys[1], gooKeys[1]));
    ASSERT_FALSE(RelationshipExists("ts.ManyFooHeldByManyGoo", fooKeys[1], gooKeys[1]));
    }

    //Delete gooKeys[2]
    {
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, gooKeys[2].GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Reset();
    stmt.ClearBindings();

    ASSERT_FALSE(InstanceExists("ts.Goo", gooKeys[2]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[0]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[1]));
    ASSERT_TRUE(InstanceExists("ts.Foo", fooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.FooHeldByGoo", fooKeys[2], gooKeys[2]));
    ASSERT_FALSE(RelationshipExists("ts.ManyFooHeldByManyGoo", fooKeys[2], gooKeys[2]));
    }
    }

END_ECDBUNITTESTS_NAMESPACE
