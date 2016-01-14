/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECSqlNavigationProps_Tests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlStatementTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_ECDBUNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 01/16
//+---------------+---------------+---------------+---------------+---------------+------
struct ECSqlNavigationPropertyTestFixture : ECDbTestFixture
    {
    protected:
        void AssertPrepare(Utf8CP ecsql, bool expectedToSucceed, Utf8CP assertMessage) const
            {
            ECSqlStatement stmt;
            ECSqlStatus stat = stmt.Prepare(GetECDb(), ecsql);
            if (expectedToSucceed)
                ASSERT_EQ(ECSqlStatus::Success, stat) << assertMessage << " - ECSQL: " << ecsql;
            else
                ASSERT_EQ(ECSqlStatus::InvalidECSql, stat) << assertMessage << " - ECSQL: " << ecsql;
            }
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 01/16
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, ECSqlSupport)
    {
    SetupECDb("ecsqlnavpropsupport.ecdb",
              SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                         "<ECSchema schemaName='TestSchema' nameSpacePrefix='ts' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                         "    <ECEntityClass typeName='A'>"
                         "        <ECProperty propertyName='PA' typeName='int' />"
                         "    </ECEntityClass>"
                         "    <ECEntityClass typeName='B'>"
                         "        <ECProperty propertyName='PB' typeName='int' />"
                         "        <ECNavigationProperty propertyName='AParent' relationshipName='AHasB' direction='Backward' />"
                         "    </ECEntityClass>"
                         "    <ECEntityClass typeName='C'>"
                         "        <ECProperty propertyName='PC' typeName='int' />"
                         "    </ECEntityClass>"
                         "    <ECEntityClass typeName='D'>"
                         "        <ECProperty propertyName='PD' typeName='int' />"
                         "        <ECNavigationProperty propertyName='CParent' relationshipName='CHasDLinkTable' direction='Backward' />"
                         "    </ECEntityClass>"
                         "   <ECRelationshipClass typeName='AHasB' strength='Embedding'>"
                         "      <Source cardinality='(0,1)' polymorphic='False'>"
                         "          <Class class ='A' />"
                         "      </Source>"
                         "      <Target cardinality='(0,N)' polymorphic='False'>"
                         "          <Class class ='B' />"
                         "      </Target>"
                         "   </ECRelationshipClass>"
                         "   <ECRelationshipClass typeName='CHasDLinkTable' strength='Referencing'>"
                         "      <Source cardinality='(0,1)' polymorphic='False'>"
                         "          <Class class ='C' />"
                         "      </Source>"
                         "      <Target cardinality='(0,N)' polymorphic='False'>"
                         "          <Class class ='D' />"
                         "      </Target>"
                         "      <ECProperty propertyName='P1' typeName='int' />"
                         "    </ECRelationshipClass>"
                         "</ECSchema>"));

    ASSERT_TRUE(GetECDb().IsDbOpen());

    AssertPrepare("SELECT PB FROM ts.B WHERE ECInstanceId=?", true, "");
    AssertPrepare("SELECT PB, AParent FROM ts.B WHERE ECInstanceId=?", true, "");
    AssertPrepare("SELECT * FROM ts.B WHERE ECInstanceId=?", true, "");

    AssertPrepare("SELECT PD FROM ts.D WHERE ECInstanceId=?", true, "");
    AssertPrepare("SELECT PD, CParent FROM ts.D WHERE ECInstanceId=?", false, "NavProp with link table relationship is not supported.");
    AssertPrepare("SELECT * FROM ts.D WHERE ECInstanceId=?", false, "NavProp with link table relationship is not supported.");

    AssertPrepare("INSERT INTO ts.B (PB,AParent) VALUES(123,?)", true, "NavProp with single related instance is expected to be supported.");
    AssertPrepare("INSERT INTO ts.D (PD,CParent) VALUES(123,?)", false, "NavProp with link table relationship is not supported.");

    AssertPrepare("UPDATE ONLY ts.B SET AParent=?", false, "Updating NavProp is not supported.");
    AssertPrepare("UPDATE ONLY ts.D SET CParent=?", false, "Updating NavProp is not supported.");
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, SingleInstanceNavProp_ForeignKeyMapping)
    {
    const int rowCount = 3;
    ECDbR ecdb = SetupECDb("ecsqlnavpropsupport.ecdb",
              SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                         "<ECSchema schemaName='TestSchema' nameSpacePrefix='np' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                         "<ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                         "    <ECEntityClass typeName='DgnModel'>"
                         "        <ECProperty propertyName='Name' typeName='string' />"
                         "    </ECEntityClass>"
                         "    <ECEntityClass typeName='DgnElement'>"
                         "        <ECProperty propertyName='Code' typeName='string' />"
                         "        <ECNavigationProperty propertyName='Model' relationshipName='ParentHasChildren' direction='Backward' />"
                         "    </ECEntityClass>"
                         "   <ECRelationshipClass typeName='ParentHasChildren' strength='Referencing'>"
                         "      <Source cardinality='(0,1)' polymorphic='False'>"
                         "          <Class class ='DgnModel' />"
                         "      </Source>"
                         "      <Target cardinality='(0,N)' polymorphic='False'>"
                         "          <Class class ='DgnElement' />"
                         "      </Target>"
                         "   </ECRelationshipClass>"
                         "</ECSchema>"), rowCount);

    ASSERT_TRUE(ecdb.IsDbOpen());
    ASSERT_EQ(SUCCESS, ecdb.Schemas().CreateECClassViewsInDb());

    ECInstanceKey modelKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT GetECClassId(), ECInstanceId FROM np.DgnModel LIMIT 1"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    modelKey = ECInstanceKey(stmt.GetValueInt64(0), stmt.GetValueId<ECInstanceId>(1));
    ASSERT_TRUE(modelKey.IsValid());
    }

    ECInstanceKey elementKey;
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "INSERT INTO np.DgnElement(Model,Code) VALUES(?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetECInstanceId()));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, "TestCode-1", IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(elementKey));

    ecdb.SaveChanges();
    }

    //verify relationship was inserted
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT SourceECInstanceId,SourceECClassId FROM np.ParentHasChildren WHERE TargetECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(modelKey.GetECInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(modelKey.GetECClassId(), stmt.GetValueInt64(1));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT TargetECInstanceId,TargetECClassId FROM np.ParentHasChildren WHERE SourceECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementKey.GetECInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(elementKey.GetECClassId(), stmt.GetValueInt64(1));
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    }

    //select from class with navprops
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT Code FROM np.DgnElement WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT Code, Model FROM np.DgnElement WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(modelKey.GetECInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(1).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());

    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT * FROM np.DgnElement WHERE ECInstanceId=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, elementKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    int navPropIx = -1;
    for (int i = 0; i < stmt.GetColumnCount(); i++)
        {
        if (stmt.GetColumnInfo(i).GetProperty()->GetIsNavigation())
            {
            navPropIx = i;
            break;
            }
        }
    ASSERT_TRUE(navPropIx >= 0);
    ASSERT_EQ(modelKey.GetECInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(navPropIx).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //Nav prop in where clause
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT ECInstanceId FROM np.DgnElement WHERE Model=?"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, modelKey.GetECInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementKey.GetECInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //with literal values
    Utf8String ecsql;
    ecsql.Sprintf("SELECT ECInstanceId FROM np.DgnElement WHERE Model=%lld", modelKey.GetECInstanceId().GetValue());
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, ecsql.c_str()));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(elementKey.GetECInstanceId().GetValue(), stmt.GetValueId<ECInstanceId>(0).GetValue());
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
    stmt.Finalize();

    //Nav prop in order by clause
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT * FROM np.DgnElement ORDER BY Model"));

    int actualRowCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        actualRowCount++;
        }
    //one element was inserted after the setup, therefore rowCount+1 is the expected value
    ASSERT_EQ(rowCount+1, actualRowCount) << stmt.GetECSql();
    stmt.Finalize();

    //Nav prop in order by clause
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT count(*), Model FROM np.DgnElement GROUP BY Model"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());

    stmt.Finalize();
    }
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                 12/15
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(ECSqlNavigationPropertyTestFixture, SingleInstanceNavProp_ForeignKeyMappingWithJoinedTable)
    {
    ECDbR ecdb = SetupECDb("ecsqlnavpropsupport_joinedtable.ecdb",
                           SchemaItem("<?xml version='1.0' encoding='utf-8'?>"
                                      "<ECSchema schemaName='TestSchema' nameSpacePrefix='np' version='1.0' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.3.0'>"
                                      "<ECSchemaReference name='ECDbMap' version='01.00' prefix='ecdbmap' />"
                                      "    <ECEntityClass typeName='DgnCategory'>"
                                      "        <ECProperty propertyName='Name' typeName='string' />"
                                      "    </ECEntityClass>"
                                      "    <ECEntityClass typeName='GeometrySource' modifier='Abstract'>"
                                      "        <ECProperty propertyName='Geometry' typeName='binary' />"
                                      "        <ECNavigationProperty propertyName='Category' relationshipName='GeometryIsInsCategory' direction='Forward' />"
                                      "    </ECEntityClass>"
                                      "    <ECEntityClass typeName='GeometrySource3d' modifier='Abstract'>"
                                      "       <BaseClass>GeometrySource</BaseClass>"
                                      "    </ECEntityClass>"
                                      "    <ECEntityClass typeName='Element' modifier='Abstract'>"
                                      "     <ECCustomAttributes>"
                                      "         <ClassMap xmlns='ECDbMap.01.00'>"
                                      "             <MapStrategy>"
                                      "                 <Strategy>SharedTable</Strategy>"
                                      "                 <AppliesToSubclasses>True</AppliesToSubclasses>"
                                      "                 <Options>JoinedTablePerDirectSubclass</Options>"
                                      "             </MapStrategy>"
                                      "         </ClassMap>"
                                      "     </ECCustomAttributes>"
                                      "      <ECProperty propertyName='Code' typeName='string' />"
                                      "    </ECEntityClass>"
                                      "    <ECEntityClass typeName='SpatialElement' modifier='Abstract'>"
                                      "       <BaseClass>Element</BaseClass>"
                                      "       <BaseClass>GeometrySource3d</BaseClass>"
                                      "    </ECEntityClass>"
                                      "    <ECEntityClass typeName='PhysicalElement'>"
                                      "     <ECCustomAttributes>"
                                      "         <ClassMap xmlns='ECDbMap.01.00'>"
                                      "             <MapStrategy>"
                                      "                 <Options>SharedColumnsForSubclasses</Options>"
                                      "             </MapStrategy>"
                                      "         </ClassMap>"
                                      "     </ECCustomAttributes>"
                                      "       <BaseClass>SpatialElement</BaseClass>"
                                      "    </ECEntityClass>"
                                      "    <ECEntityClass typeName='FooElement'>"
                                      "       <BaseClass>PhysicalElement</BaseClass>"
                                      "        <ECProperty propertyName='Diameter' typeName='double' />"
                                      "    </ECEntityClass>"
                                      "    <ECEntityClass typeName='SystemElement' modifier='Abstract'>"
                                      "       <BaseClass>Element</BaseClass>"
                                      "    </ECEntityClass>"
                                      "    <ECEntityClass typeName='DictionaryElement'>"
                                      "       <BaseClass>Element</BaseClass>"
                                      "    </ECEntityClass>"
                                      "   <ECRelationshipClass typeName='GeometryIsInsCategory' strength='Referencing'>"
                                      "     <ECCustomAttributes>"
                                      "         <ForeignKeyRelationshipMap xmlns='ECDbMap.01.00'>"
                                      "              <End>Source</End>"
                                      "              <ForeignKeyColumn>CategoryId</ForeignKeyColumn>"
                                      "         </ForeignKeyRelationshipMap>"
                                      "     </ECCustomAttributes>"
                                      "      <Source cardinality='(0,N)' polymorphic='True'>"
                                      "          <Class class ='GeometrySource' />"
                                      "      </Source>"
                                      "      <Target cardinality='(1,1)' polymorphic='False'>"
                                      "          <Class class ='DgnCategory' />"
                                      "      </Target>"
                                      "   </ECRelationshipClass>"
                                      "</ECSchema>"), 3);

    ASSERT_TRUE(ecdb.IsDbOpen());
    }

END_ECDBUNITTESTS_NAMESPACE

